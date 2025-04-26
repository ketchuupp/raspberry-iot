#pragma once

#include "bme280_defs.h"
#include "Interfaces/isensor.h"   // <<< Inherit from ISensor
#include "Interfaces/ii2c_bus.h"
#include "Interfaces/sensor_config.h" // <<< Include SensorConfig
#include <string>
#include <cstdint>
#include <vector>
#include <optional>
#include <memory> // For std::unique_ptr

namespace SensorHub::Components {

// Inherit from ISensor
class BME280_Sensor : public SensorHub::Interfaces::ISensor {
public:
    /**
     * @brief Factory method to create a BME280 sensor instance.
     * @param config The sensor configuration parsed from JSON.
     * @param i2c_bus Reference to the I2C bus manager for communication.
     * @return std::unique_ptr<ISensor> to the created sensor, or nullptr on failure.
     */
    static std::unique_ptr<SensorHub::Interfaces::ISensor> create(
        const SensorHub::Interfaces::SensorConfig& config,
        std::shared_ptr<SensorHub::Interfaces::II2C_Bus> i2c_bus);

    /**
     * @brief Constructor (now protected/private, use create factory).
     * Initializes the sensor using specific config and I2C bus.
     * @param config The sensor configuration.
     * @param i2c_bus Reference to the I2C bus manager.
     * @throws std::runtime_error if initialization fails.
     */
    BME280_Sensor(const SensorHub::Interfaces::SensorConfig& config,
        std::shared_ptr<SensorHub::Interfaces::II2C_Bus> i2c_bus); // Takes config struct

    ~BME280_Sensor() override = default;

    // --- ISensor Interface Implementation ---
    std::string getType() const override;
    bool isEnabled() const override;
    std::chrono::seconds getPublishInterval() const override;
    std::string getTopicSuffix() const override;
    nlohmann::json readDataJson() override; // <<< Changed return type

    // Delete copy/move operations
    BME280_Sensor(const BME280_Sensor&) = delete;
    BME280_Sensor& operator=(const BME280_Sensor&) = delete;
    BME280_Sensor(BME280_Sensor&&) = delete;
    BME280_Sensor& operator=(BME280_Sensor&&) = delete;

private:
    // Original method to read structured data (now private helper)
    std::optional<BME280Data> readDataInternal();

    // Helper methods (remain private)
    bool checkDevice();
    bool readCalibrationData();
    bool configureSensor();
    std::optional<std::vector<uint8_t>> readRawMeasurementData();

    // Compensation Calculations (remain private)
    BME280Data compensate(int32_t adc_T, int32_t adc_P, int32_t adc_H);
    double compensate_T(int32_t adc_T);
    double compensate_P(int32_t adc_P);
    double compensate_H(int32_t adc_H);

    // --- Calibration Data Storage ---
    // Structure to store calibration data read from the sensor
    struct CalibrationData {
        uint16_t dig_T1 = 0;
        int16_t dig_T2 = 0, dig_T3 = 0;
        uint16_t dig_P1 = 0;
        int16_t dig_P2 = 0, dig_P3 = 0, dig_P4 = 0, dig_P5 = 0, dig_P6 = 0, dig_P7 = 0, dig_P8 = 0, dig_P9 = 0;
        uint8_t dig_H1 = 0;
        int16_t dig_H2 = 0;
        uint8_t dig_H3 = 0;
        int16_t dig_H4 = 0, dig_H5 = 0;
        int8_t dig_H6 = 0;
    } calib_data_;

    // Intermediate temperature value needed for P and H compensation
    int32_t t_fine_ = 0;

    // Member Variables
    std::shared_ptr<SensorHub::Interfaces::II2C_Bus> i2c_bus_sptr_;
    SensorHub::Interfaces::SensorConfig config_; // Store the config
    bool initialized_ = false;
};

} // namespace SensorHub::Components
