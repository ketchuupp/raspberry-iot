#include <iostream>
#include <string>
#include <chrono>   // For time literals and sleeping
#include <thread>   // For sleeping
#include <csignal>  // For signal handling
#include <atomic>   // For shutdown flag
#include <iomanip>  // For std::put_time, std::setprecision
#include <memory>   // For std::unique_ptr
#include <sstream>  // For std::ostringstream

// Include Interface and Component headers
#include "Interfaces/ii2c_bus.h"     // The I2C interface
#include "SensorBME280/bme280_sensor.h" // Uses II2C_Bus
#include "NetworkMQTT/mqtt_publisher.h"  // MQTT component

// Include JSON library (make sure FetchContent is set up in root CMakeLists.txt)
#include <nlohmann/json.hpp>


// --- Conditional Includes and Type Aliases based on CMake Definition ---
#if defined(PLATFORM_LINUX_RPI) // Check for the RPi specific flag
    #include "LinuxI2C_Manager/linux_i2c_manager.h" // Include Linux implementation
    const std::string PLATFORM_NAME = "Linux RPi"; // Updated platform name
#elif defined(PLATFORM_STUB) // All other platforms use the stub
    #include "StubI2C_Manager/stub_i2c_manager.h"  // Include Stub implementation
    const std::string PLATFORM_NAME = "Stub Platform"; // Updated platform name
#else
    #error "Target platform definition not set correctly by CMake! Define PLATFORM_LINUX_RPI or PLATFORM_STUB."
#endif
// --- End Conditional Includes ---

// Use nlohmann::json for convenience
using json = nlohmann::json;

// Use namespaces for components and interfaces
using namespace SensorHub::Components;
using namespace SensorHub::Interfaces;
using namespace std::chrono_literals; // For time suffixes like 1s, 500ms

// --- Configuration (Replace with a proper config mechanism later) ---
const std::string I2C_BUS_PATH = "/dev/i2c-1"; // Path used by Linux impl, informational for Stub
const uint8_t BME280_ADDRESS = BME280::DEFAULT_ADDRESS; // Or 0x77 if needed

const std::string MQTT_BROKER_ADDRESS = "tcp://localhost:1883"; // Use tcp:// for C++ Paho client
const std::string MQTT_CLIENT_ID = "rpi_sensor_hub_" + PLATFORM_NAME; // Platform-specific ID
const std::string MQTT_TOPIC_BME280 = "rpisensor/data/bme280"; // Topic for BME280 data

const auto PUBLISH_INTERVAL = 10s; // Publish data every 10 seconds

// --- Signal Handling for Graceful Shutdown ---
std::atomic<bool> shutdown_flag = false;

void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received." << std::endl;
    shutdown_flag.store(true);
}

// --- Helper to get current timestamp in ISO 8601 format (UTC) ---
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto itt = std::chrono::system_clock::to_time_t(now);
    std::ostringstream ss;
    // Use gmtime_r for thread safety if this were called from multiple threads
    ss << std::put_time(std::gmtime(&itt), "%FT%TZ");
    return ss.str();
}


int main() {
    std::cout << "Starting Raspberry Pi Sensor Hub Application (" << PLATFORM_NAME << " build)..." << std::endl;

    // Register signal handlers for SIGINT (Ctrl+C) and SIGTERM (kill)
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // --- Initialization ---
    // Use unique_ptr for automatic resource management
    std::unique_ptr<II2C_Bus> i2c_manager;       // Pointer to the I2C interface
    std::unique_ptr<BME280_Sensor> sensor;       // Pointer to the BME280 sensor component
    std::unique_ptr<MqttPublisher> mqtt_client;  // Pointer to the MQTT publisher component

    try {
        // 1. Initialize the appropriate I2C Manager based on the platform
        std::cout << "Initializing I2C Manager (" << PLATFORM_NAME << ")..." << std::endl;
        i2c_manager = std::make_unique<SensorHub::Components::I2C_Manager>(I2C_BUS_PATH);

        // 2. Probe for the sensor using the manager interface (optional but recommended)
        std::cout << "Probing for BME280 at address 0x" << std::hex << static_cast<int>(BME280_ADDRESS) << std::dec << "..." << std::endl;
        if (!i2c_manager->probeDevice(BME280_ADDRESS)) {
             // Specific error message based on platform
             #if defined(PLATFORM_LINUX)
                 throw std::runtime_error("BME280 sensor not found/acknowledged on I2C bus " + I2C_BUS_PATH);
             #else
                 throw std::runtime_error("BME280 sensor not found (Stub did not report device).");
             #endif
        }
        std::cout << "BME280 acknowledged." << std::endl;

        // 3. Initialize the BME280 Sensor component, passing the I2C manager interface
        std::cout << "Initializing BME280 sensor component..." << std::endl;
        sensor = std::make_unique<BME280_Sensor>(*i2c_manager, BME280_ADDRESS);
        auto sensor_data_opt = sensor->readData();
        const auto& data = *sensor_data_opt;
            std::cout << "Read Sensor Data: "
                      << "T=" << std::fixed << std::setprecision(2) << data.temperature_celsius << "C, "
                      << "H=" << std::fixed << std::setprecision(2) << data.humidity_percent << "%, "
                      << "P=" << std::fixed << std::setprecision(2) << data.pressure_hpa << " hPa"
                      << std::endl;

        // 4. Initialize the MQTT Client component
        std::cout << "Initializing MQTT client for broker " << MQTT_BROKER_ADDRESS << "..." << std::endl;
        mqtt_client = std::make_unique<MqttPublisher>(MQTT_BROKER_ADDRESS, MQTT_CLIENT_ID);

    } catch (const std::exception& e) {
        std::cerr << "Initialization failed: " << e.what() << std::endl;
        return 1; // Exit if initialization fails
    }

    // --- Connect MQTT Client ---
    // Attempt to connect; the MqttPublisher might handle retries internally if configured.
    if (!mqtt_client->connect()) {
         std::cerr << "Failed to connect to MQTT broker initially. Exiting. Check broker status and address." << std::endl;
         // Depending on MqttPublisher's retry logic, it might connect later,
         // but for this example, we exit if the initial connect fails.
         return 1;
    }

    std::cout << "Initialization complete. Entering main sensor reading loop..." << std::endl;

    // --- Main Loop ---
    while (!shutdown_flag.load()) { // Continue until SIGINT/SIGTERM is received
        // Read sensor data using the sensor component (which uses the I2C manager internally)
        auto sensor_data_opt = sensor->readData();

        if (sensor_data_opt) {
            // Data read successfully
            const auto& data = *sensor_data_opt;
            std::cout << "Read Sensor Data: "
                      << "T=" << std::fixed << std::setprecision(2) << data.temperature_celsius << "C, "
                      << "H=" << std::fixed << std::setprecision(2) << data.humidity_percent << "%, "
                      << "P=" << std::fixed << std::setprecision(2) << data.pressure_hpa << " hPa"
                      << std::endl;

            // Create JSON payload using nlohmann::json
            json payload_json;
            payload_json["timestamp"] = getCurrentTimestamp(); // Add timestamp
            payload_json["temperature_celsius"] = data.temperature_celsius;
            payload_json["humidity_percent"] = data.humidity_percent;
            payload_json["pressure_hpa"] = data.pressure_hpa;
            payload_json["platform"] = PLATFORM_NAME; // Indicate build type

            std::string payload_str = payload_json.dump(); // Serialize JSON to string
            // std::cout << payload_json;
            // std::string payload_str{};

            // Publish data via MQTT if connected
            if (mqtt_client->isConnected()) {
                 if(!mqtt_client->publish(MQTT_TOPIC_BME280, payload_str)) {
                      // Log publish error, maybe implement queuing for later retry
                      std::cerr << "Failed to publish data to MQTT." << std::endl;
                 }
                 // Optional: Log successful publish
                 // else { std::cout << "Published data to MQTT topic: " << MQTT_TOPIC_BME280 << std::endl; }
            } else {
                 std::cerr << "MQTT client disconnected. Cannot publish data." << std::endl;
                 // The publisher might be attempting to reconnect automatically based on its internal logic.
            }

        } else {
            // Failed to read sensor data
            std::cerr << "Failed to read data from BME280 sensor this cycle." << std::endl;
            // Consider adding a small delay here to avoid spamming errors if sensor fails repeatedly
            std::this_thread::sleep_for(1s);
        }

        // Wait before the next reading/publishing cycle
        // Sleep in smaller intervals to check the shutdown flag more frequently
        auto wake_up_time = std::chrono::steady_clock::now() + PUBLISH_INTERVAL;
        while (std::chrono::steady_clock::now() < wake_up_time && !shutdown_flag.load()) {
            std::this_thread::sleep_for(100ms);
        }
    }

    // --- Shutdown Sequence ---
    std::cout << "Shutdown signal received. Cleaning up resources..." << std::endl;

    // Disconnect MQTT client (its destructor will also attempt this)
    if (mqtt_client) {
        mqtt_client->disconnect();
    }

    // Sensor and I2C manager cleanup happens automatically via unique_ptr destructors

    std::cout << "Application finished gracefully." << std::endl;
    return 0;
}