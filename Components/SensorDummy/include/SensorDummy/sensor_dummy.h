#pragma once

#include "Interfaces/isensor.h"       // Inherit from ISensor
#include "Interfaces/sensor_config.h" // Use SensorConfig
#include <string>
#include <chrono>
#include <memory> // For std::unique_ptr
#include <nlohmann/json_fwd.hpp>

namespace SensorHub::Components {

/**
 * @brief Dummy sensor implementation for demonstration and testing.
 * Implements the ISensor interface but returns fixed/simple data.
 */
class SensorDummy : public SensorHub::Interfaces::ISensor {
public:
    /**
     * @brief Factory method to create a Dummy sensor instance.
     * @param config The sensor configuration parsed from JSON.
     * @param dependencies... (Add any needed dependencies like bus managers here if required)
     * @return std::unique_ptr<ISensor> to the created sensor, or nullptr on failure.
     */
    static std::unique_ptr<SensorHub::Interfaces::ISensor> create(
        const SensorHub::Interfaces::SensorConfig& config
        /* Add other dependencies like bus managers if this sensor needed them */
        );

    /**
     * @brief Constructor (protected, use create factory).
     * @param config The sensor configuration.
     * @throws std::runtime_error if initialization fails.
     */
    explicit SensorDummy(const SensorHub::Interfaces::SensorConfig& config);

    ~SensorDummy() override = default;

    // --- ISensor Interface Implementation ---
    std::string getType() const override;
    bool isEnabled() const override;
    std::chrono::seconds getPublishInterval() const override;
    std::string getTopicSuffix() const override;
    nlohmann::json readDataJson() override;

    // Delete copy/move operations
    SensorDummy(const SensorDummy&) = delete;
    SensorDummy& operator=(const SensorDummy&) = delete;
    SensorDummy(SensorDummy&&) = delete;
    SensorDummy& operator=(SensorDummy&&) = delete;

private:
    // Member Variables
    SensorHub::Interfaces::SensorConfig config_; // Store the config
    bool initialized_ = false;
    int counter_ = 0; // Example internal state for dummy data
};

} // namespace SensorHub::Components
