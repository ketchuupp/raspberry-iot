#pragma once

#include <cstdint>
#include <optional>

namespace SensorHub::Components {

// Structure to hold the sensor readings
struct BME280Data {
    double temperature_celsius;
    double humidity_percent;
    double pressure_hpa;
};

// Basic BME280 registers (check datasheet for your specific version!)
namespace BME280 {
    constexpr uint8_t DEFAULT_ADDRESS = 0x76; // Or 0x77
    constexpr uint8_t REG_CHIP_ID = 0xD0;
    constexpr uint8_t REG_CTRL_HUM = 0xF2;
    constexpr uint8_t REG_CTRL_MEAS = 0xF4;
    constexpr uint8_t REG_CONFIG = 0xF5;
    constexpr uint8_t REG_CALIB_DT1_LSB = 0x88; // Start of T, P calibration data
    constexpr uint8_t REG_CALIB_DH1 = 0xA1;     // H1 calibration data
    constexpr uint8_t REG_CALIB_DH2_LSB = 0xE1; // Start of H2-H6 calibration data
    constexpr uint8_t REG_PRESS_MSB = 0xF7;    // Start of measurement data (P, T, H)

    constexpr uint8_t CHIP_ID_VALUE = 0x60; // Expected Chip ID value for BME280

    // Operating modes (example settings - adjust as needed!)
    // Humidity, Pressure, Temp Oversampling x1; Normal mode; IIR filter off; Standby 1000ms
    constexpr uint8_t CTRL_HUM_OS_1 = 0x01; // Oversampling x1 Humidity
    // Bits 7,6,5: temp OS; Bits 4,3,2: press OS; Bits 1,0: mode
    constexpr uint8_t CTRL_MEAS_SETTINGS = (0b001 << 5) | (0b001 << 2) | 0b11; // T_OS=1, P_OS=1, Mode=Normal(11)
    // Bits 7,6,5: t_sb; Bits 4,3,2: filter; Bit 0: spi3w_en (0 for I2C)
    constexpr uint8_t CONFIG_SETTINGS = (0b101 << 5) | (0b000 << 2) | 0; // t_sb=1000ms(101), filter=off(000)
} // namespace BME280

} // namespace SensorHub::Components