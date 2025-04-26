#pragma once

#include "Interfaces/sensor_config.h" // Include the config struct definition
#include <nlohmann/json.hpp>          // Include json for return type
#include <string>
#include <chrono>
#include <memory> // For std::unique_ptr

namespace SensorHub::Interfaces {

/**
 * @brief Abstract interface for all sensor types.
 */
class ISensor {
public:
    virtual ~ISensor() = default;

    /**
     * @brief Gets the type identifier string for the sensor.
     * @return Sensor type (e.g., "BME280").
     */
    virtual std::string getType() const = 0;

    /**
     * @brief Checks if the sensor instance is configured as enabled.
     * @return True if enabled, false otherwise.
     */
    virtual bool isEnabled() const = 0;

    /**
     * @brief Gets the configured publish interval for this sensor.
     * @return Publish interval.
     */
    virtual std::chrono::seconds getPublishInterval() const = 0;

    /**
     * @brief Gets the MQTT topic suffix specific to this sensor instance.
     * @return Topic suffix string.
     */
    virtual std::string getTopicSuffix() const = 0;

    /**
     * @brief Reads the current data from the sensor.
     * @return A nlohmann::json object containing the sensor-specific data payload.
     * Returns an empty json object `{}` or includes an "error" key on failure.
     */
    virtual nlohmann::json readDataJson() = 0;
};

} // namespace SensorHub::Interfaces
