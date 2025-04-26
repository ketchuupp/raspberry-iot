#pragma once

#include "Interfaces/isensor.h"
#include "Interfaces/ii2c_bus.h"
#include <nlohmann/json_fwd.hpp> // Forward declare json
#include <vector>
#include <memory> // For shared_ptr
#include <string>
#include <map> // For managing bus managers

// Forward declare concrete manager types used by builder
namespace SensorHub::Components { class LinuxI2C_Manager; }

namespace SensorHub::Builder {

/**
 * @brief Responsible for creating sensor instances based on configuration.
 * Manages underlying communication bus managers (e.g., I2C).
 */
class SensorBuilder {
public:
    SensorBuilder();
    ~SensorBuilder(); // Needed for unique_ptr to incomplete types (pimpl idiom without pimpl)

    /**
     * @brief Builds a list of enabled sensor instances from JSON configuration.
     * @param sensor_configs_json The "sensors" array from the main config JSON.
     * @return Vector of unique_ptrs to ISensor instances.
     * @throws std::runtime_error on parsing or sensor creation errors.
     */
    std::vector<std::unique_ptr<SensorHub::Interfaces::ISensor>> buildSensors(
        const nlohmann::json& sensor_configs_json);

    // Delete copy/move operations
    SensorBuilder(const SensorBuilder&) = delete;
    SensorBuilder& operator=(const SensorBuilder&) = delete;
    SensorBuilder(SensorBuilder&&) = delete;
    SensorBuilder& operator=(SensorBuilder&&) = delete;

private:
    /**
     * @brief Gets or creates an I2C bus manager for the given bus path.
     * @param bus_path The device path (e.g., "/dev/i2c-1").
     * @return Reference to the II2C_Bus manager.
     * @throws std::runtime_error if manager creation fails.
     */
    std::shared_ptr<SensorHub::Interfaces::II2C_Bus> getI2CManager(const std::string& bus_path);

    // Map to store and reuse I2C bus managers (key = bus path)
    // Use unique_ptr for ownership management
    std::map<std::string, std::shared_ptr<SensorHub::Interfaces::II2C_Bus>> i2c_managers_;

    // Add maps for other bus types (SPI, 1-Wire) here later if needed
    // std::map<std::string, std::unique_ptr<ISpiBus>> spi_managers_;
};

} // namespace SensorHub::Builder
