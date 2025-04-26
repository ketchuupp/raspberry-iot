#pragma once

#include "lps25hb_defs.h"          // Include sensor definitions
#include "Interfaces/isensor.h"   // Inherit from ISensor
#include "Interfaces/ii2c_bus.h"  // Depends on I2C Bus interface
#include "Interfaces/sensor_config.h" // Use SensorConfig
#include <string>
#include <chrono>
#include <memory> // For std::unique_ptr, std::shared_ptr
#include <nlohmann/json_fwd.hpp>

namespace SensorHub::Components {

/**
 * @brief Implementation of ISensor for an LPS25HB Pressure/Temperature sensor.
 * Communicates via I2C.
 */
class SensorLPS25HB : public SensorHub::Interfaces::ISensor {
public:
    /**
     * @brief Factory method to create an LPS25HB sensor instance.
     * @param config The sensor configuration parsed from JSON.
     * @param i2c_bus Shared pointer to the I2C bus manager for communication.
     * @return std::unique_ptr<ISensor> to the created sensor, or nullptr on failure.
     */
    static std::unique_ptr<SensorHub::Interfaces::ISensor> create(
        const SensorHub::Interfaces::SensorConfig& config,
        std::shared_ptr<SensorHub::Interfaces::II2C_Bus> i2c_bus); // Takes shared_ptr

    /**
     * @brief Constructor (protected, use create factory).
     * Initializes the sensor using specific config and I2C bus.
     * @param config The sensor configuration.
     * @param i2c_bus Shared pointer to the I2C bus manager.
     * @throws std::runtime_error if initialization fails.
     */
    SensorLPS25HB(const SensorHub::Interfaces::SensorConfig& config,
                  std::shared_ptr<SensorHub::Interfaces::II2C_Bus> i2c_bus); // Takes shared_ptr

    ~SensorLPS25HB() override = default;

    // --- ISensor Interface Implementation ---
    std::string getType() const override;
    bool isEnabled() const override;
    std::chrono::seconds getPublishInterval() const override;
    std::string getTopicSuffix() const override;
    nlohmann::json readDataJson() override;

    // Delete copy/move operations
    SensorLPS25HB(const SensorLPS25HB&) = delete;
    SensorLPS25HB& operator=(const SensorLPS25HB&) = delete;
    SensorLPS25HB(SensorLPS25HB&&) = delete;
    SensorLPS25HB& operator=(SensorLPS25HB&&) = delete;

private:
    // Helper methods
    bool checkDevice();
    bool configureSensor();
    std::optional<double> readPressure();   // Returns hPa
    std::optional<double> readTemperature(); // Returns Celsius

    // Member Variables
    std::shared_ptr<SensorHub::Interfaces::II2C_Bus> i2c_bus_sptr_; // Store shared_ptr
    SensorHub::Interfaces::SensorConfig config_; // Store the config
    bool initialized_ = false;
};

} // namespace SensorHub::Components
