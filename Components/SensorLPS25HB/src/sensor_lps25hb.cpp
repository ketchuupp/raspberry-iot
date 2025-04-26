#include "SensorLPS25HB/sensor_lps25hb.h"
#include <nlohmann/json.hpp> // Use full json header here
#include <stdexcept>
#include <iostream>
#include <vector>
#include <thread> // For sleep
#include <chrono> // For sleep
#include <iomanip> // For logging hex

using namespace SensorHub::Interfaces;
using json = nlohmann::json;
using namespace std::chrono_literals;

namespace SensorHub::Components {

// --- Factory Method ---
std::unique_ptr<ISensor> SensorLPS25HB::create(
    const SensorConfig& config,
    std::shared_ptr<II2C_Bus> i2c_bus)
{
    if (config.type != "LPS25HB") {
        return nullptr;
    }
    try {
        auto sensor_ptr = std::unique_ptr<ISensor>(new SensorLPS25HB(config, i2c_bus));
        return sensor_ptr;
    } catch (const std::exception& e) {
        std::cerr << "LPS25HB Error: Failed to create sensor instance: " << e.what() << std::endl;
        return nullptr;
    }
}

// --- Constructor ---
SensorLPS25HB::SensorLPS25HB(const SensorConfig& config,
                             std::shared_ptr<II2C_Bus> i2c_bus)
    : i2c_bus_sptr_(std::move(i2c_bus)), // Store the shared_ptr
      config_(config)
{
    if (!i2c_bus_sptr_) {
        throw std::runtime_error("LPS25HB: Invalid I2C bus manager provided.");
    }
    if (!config_.enabled) {
        throw std::runtime_error("LPS25HB: Attempted to initialize a disabled sensor.");
    }
    // Basic config validation
    if (config_.i2c_address == 0) {
         throw std::runtime_error("LPS25HB: Invalid I2C address (0) specified in configuration.");
    }

    try {
        std::cout << "LPS25HB: Initializing sensor at address 0x"
                  << std::hex << static_cast<int>(config_.i2c_address) << std::dec
                  << " on bus " << i2c_bus_sptr_->getBusPath() << std::endl;

        if (!checkDevice()) {
            throw std::runtime_error("Device ID check failed (WHO_AM_I).");
        }
        if (!configureSensor()) {
            throw std::runtime_error("Failed to configure sensor.");
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("LPS25HB Sensor Initialization Error (Addr 0x" +
                                 std::to_string(config_.i2c_address) + "): " + e.what());
    }

    initialized_ = true;
    std::cout << "LPS25HB Sensor initialized successfully (Addr 0x"
              << std::hex << static_cast<int>(config_.i2c_address) << std::dec << ")" << std::endl;
}

// --- Private Helper Methods ---

bool SensorLPS25HB::checkDevice() {
    auto who_am_i = i2c_bus_sptr_->readByteData(config_.i2c_address, LPS25HB::WHO_AM_I);
    if (!who_am_i) {
        std::cerr << "LPS25HB Error: Failed to read WHO_AM_I register." << std::endl;
        return false;
    }
    if (who_am_i.value() != 0xBD) {
         std::cerr << "LPS25HB Error: Unexpected WHO_AM_I value: 0x"
                   << std::hex << static_cast<int>(who_am_i.value()) << std::dec
                   << " (Expected 0xBD)" << std::endl;
        return false;
    }
    std::cout << "LPS25HB: WHO_AM_I check passed (0xBD)." << std::endl;
    return true;
}

bool SensorLPS25HB::configureSensor() {
    // Example: Power up, set 25Hz ODR, enable Block Data Update
    uint8_t ctrl_reg1_value = LPS25HB::PD_POWER_UP | LPS25HB::ODR_25HZ | LPS25HB::BDU_ENABLE;
    std::cout << "LPS25HB: Writing 0x" << std::hex << static_cast<int>(ctrl_reg1_value)
              << std::dec << " to CTRL_REG1 (0x20)..." << std::endl;

    if (!i2c_bus_sptr_->writeByteData(config_.i2c_address, LPS25HB::CTRL_REG1, ctrl_reg1_value)) {
        std::cerr << "LPS25HB Error: Failed to write CTRL_REG1." << std::endl;
        return false;
    }
    // Add short delay after configuration? Check datasheet.
    std::this_thread::sleep_for(5ms);
    return true;
}

std::optional<double> SensorLPS25HB::readPressure() {
    // Read 3 bytes starting from PRESS_OUT_XL (0x28)
    // Use auto-increment address if supported by manager/device (0x28 | 0x80 = 0xA8)
    // Otherwise, read registers individually. Assuming manager handles block read correctly.
    auto raw_bytes_opt = i2c_bus_sptr_->readBlockData(config_.i2c_address, LPS25HB::PRESS_OUT_XL | LPS25HB::AUTO_INCREMENT, 3);
    if (!raw_bytes_opt || raw_bytes_opt.value().size() != 3) {
        std::cerr << "LPS25HB Error: Failed to read pressure data block." << std::endl;
        return std::nullopt;
    }

    const auto& raw = raw_bytes_opt.value();
    // Combine bytes into a 24-bit value (check datasheet for order XL, L, H)
    // Value is twos complement.
    int32_t raw_pressure = static_cast<int32_t>( (static_cast<uint32_t>(raw[2]) << 16) |
                                                 (static_cast<uint32_t>(raw[1]) << 8)  |
                                                 (static_cast<uint32_t>(raw[0])) );

    // Sign extend if necessary (check if MSB of raw[2] is set)
    if (raw[2] & 0x80) {
         // raw_pressure |= 0xFF000000; // Or equivalent sign extension logic if int32_t isn't enough
         // Simpler way for standard integer sizes:
         raw_pressure = static_cast<int32_t>(static_cast<int>(raw_pressure << 8) / 256);
    }

    // Convert raw value to hPa (divide by 4096)
    double pressure_hpa = static_cast<double>(raw_pressure) / 4096.0;

    return pressure_hpa;
}

std::optional<double> SensorLPS25HB::readTemperature() {
    // Read 2 bytes starting from TEMP_OUT_L (0x2B)
    // Use auto-increment address (0x2B | 0x80 = 0xAB)
    auto raw_bytes_opt = i2c_bus_sptr_->readBlockData(config_.i2c_address, LPS25HB::TEMP_OUT_L | LPS25HB::AUTO_INCREMENT, 2);
     if (!raw_bytes_opt || raw_bytes_opt.value().size() != 2) {
        std::cerr << "LPS25HB Error: Failed to read temperature data block." << std::endl;
        return std::nullopt;
    }
    const auto& raw = raw_bytes_opt.value();
    // Combine bytes into 16-bit signed value (twos complement) - LSB first
    int16_t raw_temp = static_cast<int16_t>( (static_cast<uint16_t>(raw[1]) << 8) | raw[0] );

    // Convert raw value to Celsius (Formula: 42.5 + (raw / 480))
    double temperature_c = 42.5 + (static_cast<double>(raw_temp) / 480.0);

    return temperature_c;
}


// --- ISensor Interface Method Implementations ---

std::string SensorLPS25HB::getType() const { return config_.type; }
bool SensorLPS25HB::isEnabled() const { return config_.enabled; }
std::chrono::seconds SensorLPS25HB::getPublishInterval() const { return config_.publish_interval; }
std::string SensorLPS25HB::getTopicSuffix() const { return config_.publish_topic_suffix; }

nlohmann::json SensorLPS25HB::readDataJson() {
    json result = json::object();
    if (!initialized_) {
        result["error"] = "Sensor not initialized";
        return result;
    }

    std::optional<double> pressure = readPressure();
    std::optional<double> temperature = readTemperature();

    if (pressure.has_value()) {
        result["pressure_hpa"] = pressure.value();
    } else {
        result["pressure_error"] = "Read failed";
    }

    if (temperature.has_value()) {
        result["temperature_celsius"] = temperature.value();
    } else {
        result["temperature_error"] = "Read failed";
    }

    // Add overall error if neither reading worked
    if (!pressure.has_value() && !temperature.has_value()) {
         result["error"] = "Failed to read pressure and temperature";
    }

    return result;
}

} // namespace SensorHub::Components
