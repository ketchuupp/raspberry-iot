#pragma once

#include <string>
#include <chrono>
#include <cstdint>
#include <nlohmann/json.hpp> // Include full json header now
#include <iostream>          // For warnings in helper

namespace SensorHub::Interfaces {

/**
 * @brief Configuration structure holding common and specific sensor settings.
 * Parsed from the JSON configuration file.
 */
struct SensorConfig {
    std::string type; // e.g., "BME280", "DS18B20"
    bool enabled = false;
    std::string publish_topic_suffix;
    std::chrono::seconds publish_interval{10}; // Default interval

    // --- I2C Specific (Example) ---
    std::string i2c_bus;
    uint8_t i2c_address = 0; // Store parsed address

    // --- GPIO Specific (for DHT11 etc.) ---
    int gpio_pin = -1; // GPIO Pin number (-1 indicates not set)

    // --- Add fields for other sensor types later ---
    // std::string one_wire_id;
    // int gpio_pin;

    /**
     * @brief Parses common fields from a sensor JSON object into the config struct.
     * @param j_sensor The nlohmann::json object representing one sensor config entry.
     * @param config The SensorConfig struct to populate.
     * @return True if the sensor is enabled and common fields parsed successfully, false otherwise.
     */
    static inline bool parseCommon(const nlohmann::json& j_sensor, SensorConfig& config) {
        try {
            config.enabled = j_sensor.value("enabled", false);
            if (!config.enabled) {
                return false; // Don't parse further if not enabled
            }

            config.type = j_sensor.at("type").get<std::string>();
            config.publish_topic_suffix = j_sensor.at("publish_topic_suffix").get<std::string>();
            // Use global interval by default, allow sensor to override if needed later
            config.publish_interval = std::chrono::seconds(j_sensor.value("publish_interval_sec", 10));

            return true; // Common fields parsed successfully
        } catch (const nlohmann::json::out_of_range& e) {
            std::cerr << "SensorBuilder Warning: Missing required common sensor configuration key: " << e.what() << " in " << j_sensor.dump(2) << std::endl;
            return false;
        } catch (const nlohmann::json::type_error& e) {
            std::cerr << "SensorBuilder Warning: Incorrect type for common sensor configuration key: " << e.what() << " in " << j_sensor.dump(2) << std::endl;
            return false;
        }
    }
};

} // namespace SensorHub::Interfaces
