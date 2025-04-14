#pragma once

#include "Interfaces/ii2c_bus.h" // Include the interface
#include <string>
#include <vector>
#include <optional>
#include <cstdint>
#include <iostream> // For stub messages

namespace SensorHub::Components {

// Inherit from the interface
class I2C_Manager : public SensorHub::Interfaces::II2C_Bus {
public:
    explicit I2C_Manager(std::string bus_device_path); // Still take path for consistency
    ~I2C_Manager() override = default;

    // Override interface methods with stub implementations
    bool writeByteData(uint8_t device_address, uint8_t reg, uint8_t value) override;
    std::optional<uint8_t> readByteData(uint8_t device_address, uint8_t reg) override;
    std::optional<std::vector<uint8_t>> readBlockData(uint8_t device_address, uint8_t start_reg, size_t count) override;
    bool writeBlockData(uint8_t device_address, uint8_t start_reg, const std::vector<uint8_t>& data) override;
    bool probeDevice(uint8_t device_address) override;
    const std::string& getBusPath() const override;

    // Delete copy/assignment
    I2C_Manager(const I2C_Manager&) = delete;
    I2C_Manager& operator=(const I2C_Manager&) = delete;
    // Allow move semantics
    I2C_Manager(I2C_Manager&&) noexcept = default;
    I2C_Manager& operator=(I2C_Manager&&) noexcept = default;

private:
    std::string bus_path_;
    // No file descriptor or Linux-specific members needed
};

} // namespace SensorHub::Components