#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <optional>

namespace SensorHub::Interfaces {

/**
 * @brief Abstract interface for interacting with an I2C bus.
 * Defines platform-independent I2C operations.
 */
class II2C_Bus {
public:
    virtual ~II2C_Bus() = default; // Virtual destructor is crucial!

    /**
     * @brief Writes a single byte to a specific register on a device.
     * @param device_address The 7-bit I2C address of the target device.
     * @param reg The register address to write to.
     * @param value The byte value to write.
     * @return True on success, false on failure.
     */
    virtual bool writeByteData(uint8_t device_address, uint8_t reg, uint8_t value) = 0;

    /**
     * @brief Reads a single byte from a specific register on a device.
     * @param device_address The 7-bit I2C address of the target device.
     * @param reg The register address to read from.
     * @return std::optional<uint8_t> containing the read byte on success, std::nullopt on failure.
     */
    virtual std::optional<uint8_t> readByteData(uint8_t device_address, uint8_t reg) = 0;

    /**
     * @brief Reads a block of bytes from consecutive registers on a device.
     * @param device_address The 7-bit I2C address of the target device.
     * @param start_reg The starting register address to read from.
     * @param count The number of bytes to read.
     * @return std::optional<std::vector<uint8_t>> containing the read bytes on success, std::nullopt on failure.
     */
    virtual std::optional<std::vector<uint8_t>> readBlockData(uint8_t device_address, uint8_t start_reg, size_t count) = 0;

    /**
     * @brief Writes a block of bytes to consecutive registers on a device.
     * @param device_address The 7-bit I2C address of the target device.
     * @param start_reg The starting register address to write to.
     * @param data The vector of bytes to write.
     * @return True on success, false on failure.
     */
    virtual bool writeBlockData(uint8_t device_address, uint8_t start_reg, const std::vector<uint8_t>& data) = 0;

     /**
      * @brief Probes an address to see if a device acknowledges.
      * @param device_address The 7-bit I2C address to probe.
      * @return True if a device acknowledges at the address, false otherwise or on error.
      */
     virtual bool probeDevice(uint8_t device_address) = 0;

     /**
      * @brief Gets the bus path this manager was initialized with.
      * @return The path string (e.g., "/dev/i2c-1" or "stub").
      */
     virtual const std::string& getBusPath() const = 0;
};

} // namespace SensorHub::Interfaces