#include "App/app.h"

// Include SensorBuilder only if NOT using mocks
#ifndef BUILD_WITH_MOCKS
#include "SensorBuilder/sensor_builder.h"
#endif
// Include MockSensor only IF using mocks
#ifdef BUILD_WITH_MOCKS
#include "MockSensor.h" // Assumes Tests/Mocks is in include path
#endif

#include "Interfaces/sensor_config.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <csignal>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cstdlib>

// No longer need conditional includes for I2C managers here
// No longer need BME280 specific headers here (rely on ISensor)

using json = nlohmann::json;
using namespace SensorHub::Components;
using namespace SensorHub::Interfaces;
#ifdef BUILD_WITH_MOCKS
    using namespace SensorHub::Tests; // Include Tests namespace for MockSensor
#else
    using namespace SensorHub::Builder;
#endif
using namespace std::chrono_literals;

namespace SensorHub::App {

// Initialize static member
std::atomic<bool> App::shutdown_requested_ = false;

// Static Signal Handler
void App::signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Requesting shutdown..." << std::endl;
    shutdown_requested_.store(true);
}

// Helper: Get Timestamp (Free Function)
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&itt), "%FT%TZ");
    return ss.str();
}


// --- Load Configuration Method ---
// Now just parses the file and returns the json object
json App::loadConfig(const std::string& config_path) {
    std::cout << "Loading configuration from: " << config_path << std::endl;
    std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        throw std::runtime_error("Failed to open configuration file: " + config_path);
    }

    json config;
    try {
        config = json::parse(config_file);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse configuration file '" + config_path + "': " + e.what());
    }
    config_file.close();
    std::cout << "Configuration loaded successfully." << std::endl;
    return config;
}

// --- Initialize MQTT Client ---
// Separated from sensor init
void App::initMqtt(const nlohmann::json& config) {
     #if defined(PLATFORM_LINUX_RPI)
        platform_name_ = "Linux_RPi";
    #elif defined(PLATFORM_STUB)
        platform_name_ = "Stub_Platform";
    #endif
     std::cout << "Platform detected: " << platform_name_ << std::endl;

     try {
        const auto& mqtt_config = config.at("mqtt");
        mqtt_broker_address_ = mqtt_config.at("broker_address").get<std::string>();
        mqtt_client_id_base_ = mqtt_config.at("client_id_base").get<std::string>();
        mqtt_topic_base_ = mqtt_config.at("topic_base").get<std::string>();
        // Load global interval (or keep default)
        global_publish_interval_ = std::chrono::seconds(config.value("global_publish_interval_sec", 10));


        mqtt_client_id_ = mqtt_client_id_base_ + "_" + platform_name_;
        std::replace(mqtt_client_id_.begin(), mqtt_client_id_.end(), ' ', '_');

        std::cout << "Initializing MQTT client for broker " << mqtt_broker_address_ << " with ID " << mqtt_client_id_ << "..." << std::endl;
        mqtt_client_ = std::make_unique<MqttPublisher>(mqtt_broker_address_, mqtt_client_id_);
        std::cout << "MQTT client initialized." << std::endl;
        std::cout << "Global publish interval: " << global_publish_interval_.count() << "s" << std::endl;

    } catch (const json::out_of_range& e) { throw std::runtime_error("Missing required MQTT configuration key: " + std::string(e.what())); }
      catch (const json::type_error& e)   { throw std::runtime_error("Incorrect type for MQTT configuration key: " + std::string(e.what())); }
      catch (const std::exception& e)     { throw std::runtime_error("MQTT Initialization failed: " + std::string(e.what())); }
}


// --- Constructor ---
App::App(const std::string& config_path) {
    std::cout << "Constructing App..." << std::endl;
    try {
        // Load the entire config
        json config = loadConfig(config_path);

        // Initialize MQTT client first (needs mqtt section)
        initMqtt(config);

    
        // --- Build with Real Sensors via SensorBuilder ---
        std::cout << "Initializing with SensorBuilder (BUILD_WITH_MOCKS not defined)..." << std::endl;
        SensorBuilder builder; // Create builder locally
        sensors_ = builder.buildSensors(config.at("sensors")); // Use builder

        if (sensors_.empty()) {
             std::cerr << "Warning: No sensors were successfully created by the builder." << std::endl;
        }
        // Initialize sensor timing map
        auto now = std::chrono::steady_clock::now();
        for(const auto& sensor : sensors_) {
            next_publish_times_[sensor.get()] = now; // Schedule immediate first read
        }

    } catch (const std::exception& e) {
        throw std::runtime_error("Application construction failed: " + std::string(e.what()));
    }
    std::cout << "App construction complete." << std::endl;
}

// --- Destructor ---
App::~App() { 
    std::cout << "Destroying App..." << std::endl;
    // Disconnect MQTT client if connected
    if (mqtt_client_ && mqtt_client_->isConnected()) {
        mqtt_client_->disconnect();
    }
    // unique_ptrs for sensors_ and mqtt_client_ handle their own cleanup
    std::cout << "Application cleanup complete." << std::endl;
 }

// --- Process Sensors Cycle ---
void App::processSensors() {
    if (!mqtt_client_) return; // Should not happen if constructor succeeded

    auto now = std::chrono::steady_clock::now();

    // Iterate through all created sensors
    for (const auto& sensor : sensors_) {
        if (!sensor->isEnabled()) continue; // Skip disabled sensors

        // Check if it's time to publish for this sensor
        auto& next_pub_time = next_publish_times_[sensor.get()];
        if (now >= next_pub_time) {

            // Read sensor data into a JSON object
            json sensor_payload = sensor->readDataJson();

            if (!sensor_payload.is_null() && !sensor_payload.empty() && !sensor_payload.contains("error")) {
                // Create the final JSON payload to publish
                json final_payload = sensor_payload; // Copy sensor data
                final_payload["timestamp"] = getCurrentTimestamp();
                final_payload["platform"] = platform_name_;
                final_payload["sensor_type"] = sensor->getType();
                final_payload["topic_suffix"] = sensor->getTopicSuffix();

                std::string payload_str = final_payload.dump();
                std::string full_topic = mqtt_topic_base_ + "/" + sensor->getTopicSuffix();

                std::cout << "Publishing to " << full_topic << ": " << payload_str << std::endl;

                // Publish data via MQTT if connected
                if (mqtt_client_->isConnected()) {
                     if(!mqtt_client_->publish(full_topic, payload_str)) {
                          std::cerr << "Failed to publish data to MQTT topic: " << full_topic << std::endl;
                     }
                } else {
                     std::cerr << "MQTT client disconnected. Cannot publish data for " << full_topic << "." << std::endl;
                     // Attempt to reconnect if disconnected (might be better handled centrally)
                     // std::cout << "Attempting MQTT reconnect..." << std::endl;
                     // mqtt_client_->connect();
                }
            } else {
                 std::cerr << "Failed to read valid data from sensor type '" << sensor->getType()
                           << "' with suffix '" << sensor->getTopicSuffix() << "'." << std::endl;
                 if(sensor_payload.contains("error")) {
                     std::cerr << "  Error reported: " << sensor_payload.at("error").get<std::string>() << std::endl;
                 }
            }

             // Schedule next publish time for this sensor
             next_pub_time = now + sensor->getPublishInterval();

        } // end if time to publish
    } // end for loop sensors_

    // Reconnect MQTT if needed (central check)
    if (!mqtt_client_->isConnected()) {
        std::cout << "Attempting MQTT reconnect..." << std::endl;
        mqtt_client_->connect();
    }
}

// --- Main Run Method ---
int App::run() {
    std::cout << "Starting application run loop..." << std::endl;
    signal(SIGINT, App::signalHandler);
    signal(SIGTERM, App::signalHandler);

    if (!mqtt_client_) {
         std::cerr << "Critical Error: MQTT Client not initialized before run loop." << std::endl;
         return 1;
    }
    if (!mqtt_client_->isConnected()) {
         std::cerr << "Warning: Failed to connect to MQTT broker initially. Will retry in loop." << std::endl;
    }

    // Main loop now just checks time and processes sensors
    while (!shutdown_requested_.load()) {
        processSensors();
        // Sleep for a short duration to avoid busy-waiting
        // The actual publish interval is handled within processSensors
        std::this_thread::sleep_for(100ms);
    }

    std::cout << "Shutdown requested. Exiting run loop." << std::endl;
    return 0;
}

} // namespace SensorHub::App
