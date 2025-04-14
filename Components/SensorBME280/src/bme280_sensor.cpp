#include "SensorBME280/bme280_sensor.h"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <chrono>
#include <thread>
#include <iomanip>

namespace SensorHub::Components {

// --- Constructor Implementation ---
BME280_Sensor::BME280_Sensor(SensorHub::Interfaces::II2C_Bus& manager, uint8_t sensor_address)
    : i2c_bus_ref_(manager), // Initialize the interface reference
      address_(sensor_address)
{
    // Initialization uses the interface reference
    try {
        std::cout << "BME280: Checking device presence..." << std::endl;
        if (!checkDevice()) {
            throw std::runtime_error("BME280: Device ID check failed.");
        }
        std::cout << "BME280: Reading calibration data..." << std::endl;
        if (!readCalibrationData()) {
            throw std::runtime_error("BME280: Failed to read calibration data.");
        }
         std::cout << "BME280: Configuring sensor..." << std::endl;
        if (!configureSensor()) {
            throw std::runtime_error("BME280: Failed to configure sensor.");
        }
    } catch (const std::runtime_error& e) {
        // Log the specific error during construction
        std::cerr << "BME280 Sensor Initialization Error: " << e.what() << std::endl;
        throw; // Re-throw to signal construction failure
    }

    initialized_ = true;
    std::cout << "BME280 Sensor initialized successfully using II2C_Bus interface"
              << " (Bus: '" << i2c_bus_ref_.getBusPath() << "')" // Example using interface method
              << " at addr 0x" << std::hex << static_cast<int>(address_) << std::dec << std::endl;
}

// Destructor is now defaulted

// --- Public Method Implementation ---
std::optional<BME280Data> BME280_Sensor::readData() {
    if (!initialized_) {
        std::cerr << "BME280 Error: Sensor read attempt before successful initialization." << std::endl;
        return std::nullopt;
    }

    // Read raw data using the I2C bus interface
    auto raw_data_opt = readRawMeasurementData();
    if (!raw_data_opt) {
         // Error already logged by readRawMeasurementData or the I2C manager it calls
         // std::cerr << "BME280 Error: Failed to read raw measurement data via manager." << std::endl;
        return std::nullopt;
    }

    // Parse raw data
    const auto& raw_data = *raw_data_opt;
    int32_t adc_P = (static_cast<int32_t>(raw_data[0]) << 12) | (static_cast<int32_t>(raw_data[1]) << 4) | (static_cast<int32_t>(raw_data[2]) >> 4);
    int32_t adc_T = (static_cast<int32_t>(raw_data[3]) << 12) | (static_cast<int32_t>(raw_data[4]) << 4) | (static_cast<int32_t>(raw_data[5]) >> 4);
    int32_t adc_H = (static_cast<int32_t>(raw_data[6]) << 8) | static_cast<int32_t>(raw_data[7]);

    // Check for invalid placeholder values (sensor busy or error)
    if (adc_T == 0x80000 || adc_P == 0x80000 || adc_H == 0x8000) {
         std::cerr << "BME280 Warning: Invalid raw data read (0x80000/0x8000) - sensor might still be measuring or improperly configured." << std::endl;
         return std::nullopt;
    }

    // Compensate and return data
    return compensate(adc_T, adc_P, adc_H);
}


// --- Private Method Implementations (Using i2c_bus_ref_) ---

bool BME280_Sensor::checkDevice() {
    // Use the interface reference to call the I2C operation
    auto chip_id_opt = i2c_bus_ref_.readByteData(address_, BME280::REG_CHIP_ID);
    if (chip_id_opt && *chip_id_opt == BME280::CHIP_ID_VALUE) {
        return true; // Chip ID matches
    }
    // Log error if read failed or ID doesn't match
     if(chip_id_opt) {
         std::cerr << "BME280 Error: Unexpected Chip ID: 0x" << std::hex << static_cast<int>(*chip_id_opt)
                   << " (Expected 0x" << static_cast<int>(BME280::CHIP_ID_VALUE) << ") at addr 0x" << static_cast<int>(address_) << std::dec << std::endl;
     } else {
         std::cerr << "BME280 Error: Failed to read Chip ID via I2C bus interface for addr 0x" << std::hex << static_cast<int>(address_) << std::dec << std::endl;
     }
    return false;
}

bool BME280_Sensor::configureSensor() {
    // Write configuration registers using the interface reference
    if (!i2c_bus_ref_.writeByteData(address_, BME280::REG_CTRL_HUM, BME280::CTRL_HUM_OS_1)) return false;
    if (!i2c_bus_ref_.writeByteData(address_, BME280::REG_CONFIG, BME280::CONFIG_SETTINGS)) return false;
    // Write CTRL_MEAS last to activate settings
    if (!i2c_bus_ref_.writeByteData(address_, BME280::REG_CTRL_MEAS, BME280::CTRL_MEAS_SETTINGS)) return false;

    // Short delay for settings to apply
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return true;
}

 bool BME280_Sensor::readCalibrationData() {
      // Read calibration data blocks using the interface reference
     auto calib_tp_opt = i2c_bus_ref_.readBlockData(address_, BME280::REG_CALIB_DT1_LSB, 24);
     if (!calib_tp_opt || calib_tp_opt->size() != 24) {
         std::cerr << "BME280 Error: Failed to read T/P calibration data (block 1) via interface." << std::endl;
         return false;
     }
     const auto& calib_tp = *calib_tp_opt;

     auto calib_h1_opt = i2c_bus_ref_.readByteData(address_, BME280::REG_CALIB_DH1);
     if (!calib_h1_opt) {
         std::cerr << "BME280 Error: Failed to read H1 calibration data via interface." << std::endl;
         return false;
     }

     auto calib_h26_opt = i2c_bus_ref_.readBlockData(address_, BME280::REG_CALIB_DH2_LSB, 7);
     if (!calib_h26_opt || calib_h26_opt->size() != 7) {
          std::cerr << "BME280 Error: Failed to read H2-H6 calibration data (block 2) via interface." << std::endl;
          return false;
      }
     const auto& calib_h26 = *calib_h26_opt;

     // --- Parse and store the calibration coefficients (logic unchanged) ---
     calib_data_.dig_T1 = (static_cast<uint16_t>(calib_tp[1]) << 8) | calib_tp[0];
     calib_data_.dig_T2 = (static_cast<int16_t>(calib_tp[3]) << 8) | calib_tp[2];
     calib_data_.dig_T3 = (static_cast<int16_t>(calib_tp[5]) << 8) | calib_tp[4];
     calib_data_.dig_P1 = (static_cast<uint16_t>(calib_tp[7]) << 8) | calib_tp[6];
     calib_data_.dig_P2 = (static_cast<int16_t>(calib_tp[9]) << 8) | calib_tp[8];
     calib_data_.dig_P3 = (static_cast<int16_t>(calib_tp[11]) << 8) | calib_tp[10];
     calib_data_.dig_P4 = (static_cast<int16_t>(calib_tp[13]) << 8) | calib_tp[12];
     calib_data_.dig_P5 = (static_cast<int16_t>(calib_tp[15]) << 8) | calib_tp[14];
     calib_data_.dig_P6 = (static_cast<int16_t>(calib_tp[17]) << 8) | calib_tp[16];
     calib_data_.dig_P7 = (static_cast<int16_t>(calib_tp[19]) << 8) | calib_tp[18];
     calib_data_.dig_P8 = (static_cast<int16_t>(calib_tp[21]) << 8) | calib_tp[20];
     calib_data_.dig_P9 = (static_cast<int16_t>(calib_tp[23]) << 8) | calib_tp[22];
     calib_data_.dig_H1 = *calib_h1_opt;
     calib_data_.dig_H2 = (static_cast<int16_t>(calib_h26[1]) << 8) | calib_h26[0];
     calib_data_.dig_H3 = calib_h26[2];
     calib_data_.dig_H4 = (static_cast<int16_t>(calib_h26[3]) << 4) | (calib_h26[4] & 0x0F);
     calib_data_.dig_H5 = (static_cast<int16_t>(calib_h26[5]) << 4) | (calib_h26[4] >> 4);
     calib_data_.dig_H6 = static_cast<int8_t>(calib_h26[6]);

     return true; // Successfully read and parsed all calibration data
 }

 std::optional<std::vector<uint8_t>> BME280_Sensor::readRawMeasurementData() {
     // Use the interface reference to read the 8-byte measurement block
     return i2c_bus_ref_.readBlockData(address_, BME280::REG_PRESS_MSB, 8);
 }

 // --- Compensation Implementations (These remain logically unchanged) ---
 double BME280_Sensor::compensate_T(int32_t adc_T) {
     double var1 = (static_cast<double>(adc_T) / 16384.0 - static_cast<double>(calib_data_.dig_T1) / 1024.0) * static_cast<double>(calib_data_.dig_T2);
     double var2 = ((static_cast<double>(adc_T) / 131072.0 - static_cast<double>(calib_data_.dig_T1) / 8192.0) *
                    (static_cast<double>(adc_T) / 131072.0 - static_cast<double>(calib_data_.dig_T1) / 8192.0)) * static_cast<double>(calib_data_.dig_T3);
     t_fine_ = static_cast<int32_t>(var1 + var2);
     double T = (var1 + var2) / 5120.0;
     // Add temperature limits check if desired
     // if (T < -40.0) T = -40.0;
     // if (T > 85.0) T = 85.0;
     return T;
 }
 double BME280_Sensor::compensate_P(int32_t adc_P) {
     double var1 = static_cast<double>(t_fine_) / 2.0 - 64000.0;
     double var2 = var1 * var1 * static_cast<double>(calib_data_.dig_P6) / 32768.0;
     var2 = var2 + var1 * static_cast<double>(calib_data_.dig_P5) * 2.0;
     var2 = (var2 / 4.0) + (static_cast<double>(calib_data_.dig_P4) * 65536.0);
     var1 = (static_cast<double>(calib_data_.dig_P3) * var1 * var1 / 524288.0 + static_cast<double>(calib_data_.dig_P2) * var1) / 524288.0;
     var1 = (1.0 + var1 / 32768.0) * static_cast<double>(calib_data_.dig_P1);
     if (var1 == 0.0) { return 0.0; } // Avoid division by zero
     double p = 1048576.0 - static_cast<double>(adc_P);
     p = (p - (var2 / 4096.0)) * 6250.0 / var1;
     var1 = static_cast<double>(calib_data_.dig_P9) * p * p / 2147483648.0;
     var2 = p * static_cast<double>(calib_data_.dig_P8) / 32768.0;
     p = p + (var1 + var2 + static_cast<double>(calib_data_.dig_P7)) / 16.0;
     // Add pressure limits check if desired (e.g., 30000 Pa to 110000 Pa)
     // if (p < 30000.0) p = 30000.0;
     // if (p > 110000.0) p = 110000.0;
      return p; // Pressure in Pascal
  }
 double BME280_Sensor::compensate_H(int32_t adc_H) {
     double var_H = (static_cast<double>(t_fine_) - 76800.0);
     // Check for zero to avoid potential division issues depending on formula variant
     if (var_H == 0.0) { return 0.0; } // Or handle as per specific datasheet guidance
     var_H = (static_cast<double>(adc_H) - (static_cast<double>(calib_data_.dig_H4) * 64.0 + static_cast<double>(calib_data_.dig_H5) / 16384.0 * var_H)) *
             (static_cast<double>(calib_data_.dig_H2) / 65536.0 * (1.0 + static_cast<double>(calib_data_.dig_H6) / 67108864.0 * var_H *
             (1.0 + static_cast<double>(calib_data_.dig_H3) / 67108864.0 * var_H)));
     var_H = var_H * (1.0 - static_cast<double>(calib_data_.dig_H1) * var_H / 524288.0);
     // Clamp humidity to the valid range [0, 100] %RH
     if (var_H > 100.0) var_H = 100.0;
     else if (var_H < 0.0) var_H = 0.0;
     return var_H; // Humidity in %RH
  }
 BME280Data BME280_Sensor::compensate(int32_t adc_T, int32_t adc_P, int32_t adc_H) {
      BME280Data data;
      // Compensate temperature first, as t_fine is needed for others
      data.temperature_celsius = compensate_T(adc_T);
      // Compensate pressure and convert Pa to hPa
      data.pressure_hpa = compensate_P(adc_P) / 100.0;
      // Compensate humidity
      data.humidity_percent = compensate_H(adc_H);
      return data;
  }

} // namespace SensorHub::Components