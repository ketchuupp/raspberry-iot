#include "SensorBME280/bme280_sensor.h"
#include <nlohmann/json.hpp> // Include json library
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cmath>
#include <chrono>
#include <thread>
#include <iomanip>

namespace SensorHub::Components {

// --- Factory Method Implementation ---
std::unique_ptr<SensorHub::Interfaces::ISensor> BME280_Sensor::create(
    const SensorHub::Interfaces::SensorConfig& config,
    std::shared_ptr<SensorHub::Interfaces::II2C_Bus> i2c_bus)
{
    // Check if type matches (although Builder should handle this)
    if (config.type != "BME280") {
        return nullptr;
    }
    try {
        // Use 'new' because make_unique cannot access private/protected constructor easily
        // Wrap immediately in unique_ptr for safety
        auto sensor_ptr = std::unique_ptr<ISensor>(new BME280_Sensor(config, i2c_bus));
        return sensor_ptr;
    } catch (const std::exception& e) {
        std::cerr << "BME280 Error: Failed to create sensor instance: " << e.what() << std::endl;
        return nullptr; // Return null on creation failure
    }
}


// --- Constructor Implementation ---
// Takes SensorConfig and II2C_Bus reference
BME280_Sensor::BME280_Sensor(const SensorHub::Interfaces::SensorConfig& config,
    std::shared_ptr<SensorHub::Interfaces::II2C_Bus> i2c_bus)
    : i2c_bus_sptr_(std::move(i2c_bus)),
      config_(config) // Store the configuration
{
    if (!config_.enabled) {
         throw std::runtime_error("BME280: Attempted to initialize a disabled sensor.");
    }

    try {
        std::cout << "BME280: Initializing sensor at address 0x"
                  << std::hex << static_cast<int>(config_.i2c_address) << std::dec
                  << " on bus " << config_.i2c_bus << std::endl;

        if (!checkDevice()) { // Uses config_.i2c_address internally now
            throw std::runtime_error("Device ID check failed.");
        }
        if (!readCalibrationData()) { // Uses config_.i2c_address internally now
            throw std::runtime_error("Failed to read calibration data.");
        }
        if (!configureSensor()) { // Uses config_.i2c_address internally now
            throw std::runtime_error("Failed to configure sensor.");
        }
    } catch (const std::runtime_error& e) {
        // Provide more context in the chained exception
        throw std::runtime_error("BME280 Sensor Initialization Error (Addr 0x" +
                                 std::to_string(config_.i2c_address) + "): " + e.what());
    }

    initialized_ = true;
    std::cout << "BME280 Sensor initialized successfully (Addr 0x"
              << std::hex << static_cast<int>(config_.i2c_address) << std::dec << ")" << std::endl;
}

// --- ISensor Interface Method Implementations ---

std::string BME280_Sensor::getType() const {
    return config_.type; // Return type from stored config
}

bool BME280_Sensor::isEnabled() const {
    return config_.enabled; // Return status from stored config
}

std::chrono::seconds BME280_Sensor::getPublishInterval() const {
    return config_.publish_interval; // Return interval from stored config
}

std::string BME280_Sensor::getTopicSuffix() const {
    return config_.publish_topic_suffix; // Return suffix from stored config
}

nlohmann::json BME280_Sensor::readDataJson() {
    auto data_opt = readDataInternal(); // Call the original read logic
    nlohmann::json result = nlohmann::json::object(); // Start with empty object

    if (data_opt) {
        // Populate JSON object if read was successful
        result["temperature_celsius"] = data_opt.value().temperature_celsius;
        result["humidity_percent"] = data_opt.value().humidity_percent;
        result["pressure_hpa"] = data_opt.value().pressure_hpa;
    } else {
        // Optionally add an error field or just return empty
        // result["error"] = "Read failed";
    }
    return result;
}

// --- Original readData renamed to readDataInternal ---
std::optional<BME280Data> BME280_Sensor::readDataInternal() {
    if (!initialized_) {
        std::cerr << "BME280 Error: Sensor read attempt before successful initialization." << std::endl;
        return std::nullopt;
    }

    auto raw_data_opt = readRawMeasurementData(); // Uses i2c_bus_ref_ and config_.i2c_address
    if (!raw_data_opt) {
        return std::nullopt;
    }

    const auto& raw_data = *raw_data_opt;
    int32_t adc_P = (static_cast<int32_t>(raw_data[0]) << 12) | (static_cast<int32_t>(raw_data[1]) << 4) | (static_cast<int32_t>(raw_data[2]) >> 4);
    int32_t adc_T = (static_cast<int32_t>(raw_data[3]) << 12) | (static_cast<int32_t>(raw_data[4]) << 4) | (static_cast<int32_t>(raw_data[5]) >> 4);
    int32_t adc_H = (static_cast<int32_t>(raw_data[6]) << 8) | static_cast<int32_t>(raw_data[7]);

    if (adc_T == 0x80000 || adc_P == 0x80000 || adc_H == 0x8000) {
         std::cerr << "BME280 Warning: Invalid raw data read (0x80000/0x8000) for addr 0x"
                   << std::hex << static_cast<int>(config_.i2c_address) << std::dec << std::endl;
         return std::nullopt;
    }

    return compensate(adc_T, adc_P, adc_H);
}


// --- Private Helper Method Implementations (now use config_.i2c_address) ---

bool BME280_Sensor::checkDevice() {
    auto chip_id_opt = i2c_bus_sptr_->readByteData(config_.i2c_address, BME280::REG_CHIP_ID);
    if (chip_id_opt && *chip_id_opt == BME280::CHIP_ID_VALUE) {
        return true;
    }
     if(chip_id_opt) {
         std::cerr << "BME280 Error: Unexpected Chip ID: 0x" << std::hex << static_cast<int>(*chip_id_opt)
                   << " (Expected 0x" << static_cast<int>(BME280::CHIP_ID_VALUE) << ") at addr 0x" << static_cast<int>(config_.i2c_address) << std::dec << std::endl;
     } else {
         std::cerr << "BME280 Error: Failed to read Chip ID via I2C bus interface for addr 0x" << std::hex << static_cast<int>(config_.i2c_address) << std::dec << std::endl;
     }
    return false;
}

bool BME280_Sensor::configureSensor() {
    if (!i2c_bus_sptr_->writeByteData(config_.i2c_address, BME280::REG_CTRL_HUM, BME280::CTRL_HUM_OS_1)) return false;
    if (!i2c_bus_sptr_->writeByteData(config_.i2c_address, BME280::REG_CONFIG, BME280::CONFIG_SETTINGS)) return false;
    if (!i2c_bus_sptr_->writeByteData(config_.i2c_address, BME280::REG_CTRL_MEAS, BME280::CTRL_MEAS_SETTINGS)) return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    return true;
}

 bool BME280_Sensor::readCalibrationData() {
     auto calib_tp_opt = i2c_bus_sptr_->readBlockData(config_.i2c_address, BME280::REG_CALIB_DT1_LSB, 24);
     if (!calib_tp_opt || calib_tp_opt->size() != 24) {
         std::cerr << "BME280 Error: Failed to read T/P calibration data (block 1) for addr 0x" << std::hex << static_cast<int>(config_.i2c_address) << std::dec << std::endl;
         return false;
     }
     const auto& calib_tp = *calib_tp_opt;

     auto calib_h1_opt = i2c_bus_sptr_->readByteData(config_.i2c_address, BME280::REG_CALIB_DH1);
     if (!calib_h1_opt) {
          std::cerr << "BME280 Error: Failed to read H1 calibration data for addr 0x" << std::hex << static_cast<int>(config_.i2c_address) << std::dec << std::endl;
         return false;
     }

     auto calib_h26_opt = i2c_bus_sptr_->readBlockData(config_.i2c_address, BME280::REG_CALIB_DH2_LSB, 7);
     if (!calib_h26_opt || calib_h26_opt->size() != 7) {
          std::cerr << "BME280 Error: Failed to read H2-H6 calibration data (block 2) for addr 0x" << std::hex << static_cast<int>(config_.i2c_address) << std::dec << std::endl;
          return false;
      }
     const auto& calib_h26 = *calib_h26_opt;

     // Parse calibration data (logic unchanged)
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

     return true;
 }

 std::optional<std::vector<uint8_t>> BME280_Sensor::readRawMeasurementData() {
     return i2c_bus_sptr_->readBlockData(config_.i2c_address, BME280::REG_PRESS_MSB, 8);
 }

// --- Compensation Implementations (Unchanged) ---
double BME280_Sensor::compensate_T(int32_t adc_T) { /* ... same ... */
    double var1 = (static_cast<double>(adc_T) / 16384.0 - static_cast<double>(calib_data_.dig_T1) / 1024.0) * static_cast<double>(calib_data_.dig_T2);
    double var2 = ((static_cast<double>(adc_T) / 131072.0 - static_cast<double>(calib_data_.dig_T1) / 8192.0) *
                   (static_cast<double>(adc_T) / 131072.0 - static_cast<double>(calib_data_.dig_T1) / 8192.0)) * static_cast<double>(calib_data_.dig_T3);
    t_fine_ = static_cast<int32_t>(var1 + var2);
    double T = (var1 + var2) / 5120.0;
    return T;
}
double BME280_Sensor::compensate_P(int32_t adc_P) { /* ... same ... */
    double var1 = static_cast<double>(t_fine_) / 2.0 - 64000.0;
    double var2 = var1 * var1 * static_cast<double>(calib_data_.dig_P6) / 32768.0;
    var2 = var2 + var1 * static_cast<double>(calib_data_.dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (static_cast<double>(calib_data_.dig_P4) * 65536.0);
    var1 = (static_cast<double>(calib_data_.dig_P3) * var1 * var1 / 524288.0 + static_cast<double>(calib_data_.dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * static_cast<double>(calib_data_.dig_P1);
    if (var1 == 0.0) { return 0.0; }
    double p = 1048576.0 - static_cast<double>(adc_P);
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = static_cast<double>(calib_data_.dig_P9) * p * p / 2147483648.0;
    var2 = p * static_cast<double>(calib_data_.dig_P8) / 32768.0;
    p = p + (var1 + var2 + static_cast<double>(calib_data_.dig_P7)) / 16.0;
    return p;
 }
double BME280_Sensor::compensate_H(int32_t adc_H) { /* ... same ... */
    double var_H = (static_cast<double>(t_fine_) - 76800.0);
    if (var_H == 0.0) { return 0.0; }
    var_H = (static_cast<double>(adc_H) - (static_cast<double>(calib_data_.dig_H4) * 64.0 + static_cast<double>(calib_data_.dig_H5) / 16384.0 * var_H)) *
            (static_cast<double>(calib_data_.dig_H2) / 65536.0 * (1.0 + static_cast<double>(calib_data_.dig_H6) / 67108864.0 * var_H *
            (1.0 + static_cast<double>(calib_data_.dig_H3) / 67108864.0 * var_H)));
    var_H = var_H * (1.0 - static_cast<double>(calib_data_.dig_H1) * var_H / 524288.0);
    if (var_H > 100.0) var_H = 100.0;
    else if (var_H < 0.0) var_H = 0.0;
    return var_H;
 }
BME280Data BME280_Sensor::compensate(int32_t adc_T, int32_t adc_P, int32_t adc_H) { /* ... same ... */
     BME280Data data;
     data.temperature_celsius = compensate_T(adc_T);
     data.pressure_hpa = compensate_P(adc_P) / 100.0;
     data.humidity_percent = compensate_H(adc_H);
     return data;
 }

} // namespace SensorHub::Components
