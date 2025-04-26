#pragma once

#include "Interfaces/ii2c_bus.h"
#include "SensorBME280/bme280_sensor.h" // Include necessary sensor headers
#include "NetworkMQTT/mqtt_publisher.h"
#include <nlohmann/json_fwd.hpp> // Forward declare json for header
#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <chrono>

namespace SensorHub::App {

/**
 * @brief Configuration structure for a single sensor.
 * Designed to be extensible for different sensor types.
 */
struct SensorConfig {
    std::string type;
    bool enabled = false;
    std::string publish_topic_suffix;
    std::chrono::seconds publish_interval{10}; // Default interval

    // --- I2C Specific ---
    std::string i2c_bus;
    uint8_t i2c_address = 0; // Store parsed address

    // --- Add fields for other sensor types later ---
};

/**
 * @brief Main application class encapsulating sensor reading and MQTT publishing.
 * Loads configuration from a JSON file.
 */
class App {
public:
    /**
     * @brief Constructor. Loads config and initializes components.
     * @param config_path Path to the JSON configuration file.
     * @throws std::runtime_error on configuration loading or initialization failure.
     */
    explicit App(const std::string& config_path = "config.json"); // Default path

    /**
     * @brief Destructor. Handles cleanup.
     */
    ~App();

    /**
     * @brief Runs the main application loop.
     * Sets up signal handling and periodically reads sensors and publishes data
     * until a shutdown signal is received.
     * @return 0 on graceful shutdown, 1 on error during loop setup.
     */
    int run();

    // Delete copy/assignment/move as App manages unique resources
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

private:
    /**
     * @brief Loads configuration from the specified JSON file.
     * @param config_path Path to the JSON configuration file.
     * @return Parsed nlohmann::json object.
     * @throws std::runtime_error if file cannot be read or parsed.
     */
    nlohmann::json loadConfig(const std::string& config_path);

    /**
     * @brief Initializes MQTT client based on loaded configuration.
     * @param config The loaded JSON configuration object.
     * @throws std::runtime_error on component initialization failure.
     */
    void initMqtt(const nlohmann::json& config);

    /**
     * @brief Performs one cycle of reading sensor data and publishing via MQTT.
     */
    void processSensors();

    /**
     * @brief Static signal handler function to request shutdown.
     * @param signum Signal number received.
     */
    static void signalHandler(int signum);

    // --- Loaded Configuration ---
    std::string mqtt_broker_address_;
    std::string mqtt_client_id_base_;
    std::string mqtt_topic_base_;
    std::chrono::seconds global_publish_interval_{10}; // <<< ADDED Declaration

    // --- Active Components ---
    std::string platform_name_;
    std::string mqtt_client_id_;
    // Sensor instances built by SensorBuilder
    std::vector<std::unique_ptr<SensorHub::Interfaces::ISensor>> sensors_; // <<< ADDED Declaration
    std::unique_ptr<SensorHub::Components::MqttPublisher> mqtt_client_;

    // --- Sensor Timing ---
    // Map sensor pointer to its next publish time point
    std::map<SensorHub::Interfaces::ISensor*, std::chrono::steady_clock::time_point> next_publish_times_; // <<< ADDED Declaration

    // Static flag for signal handling
    static std::atomic<bool> shutdown_requested_;
};

} // namespace SensorHub::App
