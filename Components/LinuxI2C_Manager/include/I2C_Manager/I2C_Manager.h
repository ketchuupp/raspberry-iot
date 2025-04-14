#pragma once

#include "Interfaces/ii2c_bus.h" // Include the interface
#include <string>
#include <mutex>
#include <vector>
#include <optional>
#include <cstdint>

namespace SensorHub::Components {

// Inherit from the interface
class LinuxI2C_Manager : public SensorHub::Interfaces::II2C_Bus {
public:
    explicit LinuxI2C_Manager(std::string bus_device_path);
    ~LinuxI2C_Manager() override; // Override virtual destructor

    // Override interface methods
    bool writeByteData(uint8_t device_address, uint8_t reg, uint8_t value) override;
    std::optional<uint8_t> readByteData(uint8_t device_address, uint8_t reg) override;
    std::optional<std::vector<uint8_t>> readBlockData(uint8_t device_address, uint8_t start_reg, size_t count) override;
    bool writeBlockData(uint8_t device_address, uint8_t start_reg, const std::vector<uint8_t>& data) override;
    bool probeDevice(uint8_t device_address) override;
    const std::string& getBusPath() const override;


    // Delete copy/assignment
    LinuxI2C_Manager(const LinuxI2C_Manager&) = delete;
    LinuxI2C_Manager& operator=(const LinuxI2C_Manager&) = delete;
    // Allow move semantics
    LinuxI2C_Manager(LinuxI2C_Manager&&) noexcept;
    LinuxI2C_Manager& operator=(LinuxI2C_Manager&&) noexcept;

private:
    bool setActiveDevice(uint8_t device_address); // Keep this helper

    std::string bus_path_;
    int fd_ = -1;
    uint8_t current_address_ = 0;
    std::mutex bus_mutex_; // Protect access to fd_ and current_address_
};

} // namespace SensorHub::Components