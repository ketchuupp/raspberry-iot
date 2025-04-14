#include "StubI2C_Manager/stub_i2c_manager.h"
#include "SensorBME280/bme280_defs.h"
#include <iostream>
#include <iomanip> // For std::hex
#include <vector>
#include <numeric> // For std::iota

namespace SensorHub::Components {

I2C_Manager::I2C_Manager(std::string bus_device_path)
    : bus_path_(std::move(bus_device_path)) {
    std::cout << "I2C_Manager: Initialized for bus path '" << bus_path_
              << "' (No actual device opened)." << std::endl;
}

bool I2C_Manager::writeByteData(uint8_t device_address, uint8_t reg, uint8_t value) {
    std::cout << "[Stub I2C]: Write Byte 0x" << std::hex << static_cast<int>(value)
              << " to Addr 0x" << static_cast<int>(device_address)
              << " Reg 0x" << static_cast<int>(reg) << std::dec << std::endl;
    // Pretend success
    return true;
}

std::optional<uint8_t> I2C_Manager::readByteData(uint8_t device_address, uint8_t reg) {
    std::cout << "[Stub I2C]: Read Byte from Addr 0x" << std::hex << static_cast<int>(device_address)
              << " Reg 0x" << static_cast<int>(reg) << std::dec << std::endl;

    // Return specific dummy values for known registers if needed for testing
    if (device_address == BME280::DEFAULT_ADDRESS) {
        if (reg == BME280::REG_CHIP_ID) {
            return BME280::CHIP_ID_VALUE; // Return correct Chip ID
        }
        if (reg == BME280::REG_CALIB_DH1) {
             return 0x7F; // Dummy H1 value
        }
    }
    // Default dummy value
    return 0xAB;
}

std::optional<std::vector<uint8_t>> I2C_Manager::readBlockData(uint8_t device_address, uint8_t start_reg, size_t count) {
    std::cout << "[Stub I2C]: Read Block (" << count << " bytes) from Addr 0x" << std::hex << static_cast<int>(device_address)
              << " Reg 0x" << static_cast<int>(start_reg) << std::dec << std::endl;

    // Return dummy data matching the expected structure for BME280 calibration/measurement
    if (device_address == BME280::DEFAULT_ADDRESS) {
        if (start_reg == BME280::REG_CALIB_DT1_LSB && count == 24) {
             // Return somewhat realistic-looking dummy calibration data for T/P block 1
             std::vector<uint8_t> dummy_calib_tp(24);
             // Fill with some pattern, e.g., sequence + offset
             std::iota(dummy_calib_tp.begin(), dummy_calib_tp.end(), static_cast<uint8_t>(0x10));
             return dummy_calib_tp;
        }
         if (start_reg == BME280::REG_CALIB_DH2_LSB && count == 7) {
              // Return dummy calibration data for H block 2
              std::vector<uint8_t> dummy_calib_h(7);
              std::iota(dummy_calib_h.begin(), dummy_calib_h.end(), static_cast<uint8_t>(0xE0));
              return dummy_calib_h;
         }
         if (start_reg == BME280::REG_PRESS_MSB && count == 8) {
             // Return dummy measurement data (P, T, H raw bytes)
             // Example values corresponding roughly to 50000 (P), 40000 (T), 30000 (H) raw counts
              return std::vector<uint8_t>{0x50, 0x10, 0x00, // Pressure high-ish
                                          0x6A, 0xBC, 0xD0, // Temp mid-range
                                          0x7F, 0x80};      // Humidity mid-range
         }
    }

    // Default dummy block if no specific case matches
    std::vector<uint8_t> default_dummy(count);
    std::iota(default_dummy.begin(), default_dummy.end(), static_cast<uint8_t>(0x55));
    return default_dummy;
}

bool I2C_Manager::writeBlockData(uint8_t device_address, uint8_t start_reg, const std::vector<uint8_t>& data) {
     std::cout << "[Stub I2C]: Write Block (" << data.size() << " bytes) to Addr 0x" << std::hex << static_cast<int>(device_address)
              << " Reg 0x" << static_cast<int>(start_reg) << std::dec << std::endl;
     // Pretend success
     return true;
}

bool I2C_Manager::probeDevice(uint8_t device_address) {
     std::cout << "[Stub I2C]: Probe Addr 0x" << std::hex << static_cast<int>(device_address) << std::dec << std::endl;
     // Pretend the default BME280 address exists for testing initialization
     if (device_address == BME280::DEFAULT_ADDRESS) {
         std::cout << "[Stub I2C]: Acknowledged Addr 0x" << std::hex << static_cast<int>(device_address) << std::dec << std::endl;
         return true;
     }
     std::cout << "[Stub I2C]: No device acknowledged at Addr 0x" << std::hex << static_cast<int>(device_address) << std::dec << std::endl;
     return false; // Assume others don't exist
}

const std::string& I2C_Manager::getBusPath() const {
     return bus_path_;
 }

} // namespace SensorHub::Components