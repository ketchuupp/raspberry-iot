#pragma once

#include <cstdint>

namespace SensorHub::Components {
namespace LPS25HB {

    // Default I2C Address (Check datasheet/board, can be 0x5C or 0x5D depending on SDO/SA0 pin)
    constexpr uint8_t DEFAULT_ADDRESS = 0x5D;
    constexpr uint8_t ALT_ADDRESS = 0x5C;

    // Register Addresses
    constexpr uint8_t WHO_AM_I       = 0x0F; // Expected value: 0xBD
    constexpr uint8_t CTRL_REG1      = 0x20;
    constexpr uint8_t CTRL_REG2      = 0x21;
    // ... other control registers if needed ...
    constexpr uint8_t PRESS_OUT_XL   = 0x28; // Pressure LSB
    constexpr uint8_t PRESS_OUT_L    = 0x29; // Pressure Mid
    constexpr uint8_t PRESS_OUT_H    = 0x2A; // Pressure MSB
    constexpr uint8_t TEMP_OUT_L     = 0x2B; // Temperature LSB
    constexpr uint8_t TEMP_OUT_H     = 0x2C; // Temperature MSB

    // Bit masks/values for CTRL_REG1
    constexpr uint8_t PD_POWER_UP    = 0x80; // Bit 7: Power Down Control (1=active)
    constexpr uint8_t ODR_25HZ       = 0x40; // Bits 6-4: Output Data Rate 25Hz (100)
    constexpr uint8_t ODR_12_5HZ     = 0x30; // Bits 6-4: Output Data Rate 12.5Hz (011)
    constexpr uint8_t ODR_7HZ        = 0x20; // Bits 6-4: Output Data Rate 7Hz (010)
    constexpr uint8_t ODR_1HZ        = 0x10; // Bits 6-4: Output Data Rate 1Hz (001)
    constexpr uint8_t ODR_ONE_SHOT   = 0x00; // Bits 6-4: One-shot mode (000)
    constexpr uint8_t BDU_ENABLE     = 0x04; // Bit 2: Block Data Update (1=enable)

    // Auto-increment bit for multi-byte reads (optional, depends on I2C manager implementation)
    constexpr uint8_t AUTO_INCREMENT = 0x80;

} // namespace LPS25HB
} // namespace SensorHub::Components
