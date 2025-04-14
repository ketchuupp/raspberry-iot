#pragma once

#include "Interfaces/ii2c_bus.h" // Include the interface
#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <iostream> // For stub messages

// Forward declare sensor defs if needed for dummy data generation
namespace SensorHub::Components { namespace BME280_STUB {
    constexpr uint8_t REG_CHIP_ID = 0xD0;
    constexpr uint8_t CHIP_ID_VALUE = 0x60;
    constexpr uint8_t DEFAULT_ADDRESS = 0x76;
    constexpr uint8_t REG_CALIB_DT1_LSB = 0x88;
    constexpr uint8_t REG_CALIB_DH1 = 0xA1;
    constexpr uint8_t REG_CALIB_DH2_LSB = 0xE1;
    constexpr uint8_t REG_PRESS_MSB = 0xF7;
}} // namespace SensorHub::Components

namespace SensorHub::Components {

// Inherit from the interface
class StubI2C_Manager : public SensorHub::Interfaces::II2C_Bus {
public:
    explicit StubI2C_Manager(std::string bus_device_path); // Still take path for consistency
    ~StubI2C_Manager() override = default;

    // Override interface methods with stub implementations
    bool writeByteData(uint8_t device_address, uint8_t reg, uint8_t value) override;
    std::optional<uint8_t> readByteData(uint8_t device_address, uint8_t reg) override;
    std::optional<std::vector<uint8_t>> readBlockData(uint8_t device_address, uint8_t start_reg, size_t count) override;
    bool writeBlockData(uint8_t device_address, uint8_t start_reg, const std::vector<uint8_t>& data) override;
    bool probeDevice(uint8_t device_address) override;
    const std::string& getBusPath() const override;

    // Delete copy/assignment
    StubI2C_Manager(const StubI2C_Manager&) = delete;
    StubI2C_Manager& operator=(const StubI2C_Manager&) = delete;
    // Allow move semantics
    StubI2C_Manager(StubI2C_Manager&&) noexcept = default;
    StubI2C_Manager& operator=(StubI2C_Manager&&) noexcept = default;

private:
    std::string bus_path_;
    // No file descriptor or Linux-specific members needed
};

} // namespace SensorHub::Components