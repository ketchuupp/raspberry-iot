#pragma once

#include "bme280_defs.h"
#include "Interfaces/ii2c_bus.h" // Include the INTERFACE
#include <string>
#include <cstdint>
#include <vector>
#include <optional>

namespace SensorHub::Components {

class BME280_Sensor {
public:
    /**
     * @brief Constructor: Uses an II2C_Bus interface to communicate.
     * @param manager A reference to an object implementing the II2C_Bus interface.
     * @param sensor_address The 7-bit I2C address of the BME280 sensor.
     * @throws std::runtime_error if initialization fails.
     */
    BME280_Sensor(SensorHub::Interfaces::II2C_Bus& manager, // Use the INTERFACE type
                   uint8_t sensor_address = BME280::DEFAULT_ADDRESS);

    /**
     * @brief Destructor.
     */
    ~BME280_Sensor() = default;

    /**
     * @brief Reads the current temperature, humidity, and pressure data.
     * @return std::optional<BME280Data> containing the data if successful,
     * std::nullopt otherwise.
     */
    std::optional<BME280Data> readData();

    // Delete copy constructor and assignment operator
    BME280_Sensor(const BME280_Sensor&) = delete;
    BME280_Sensor& operator=(const BME280_Sensor&) = delete;
    // Allow move semantics (default is likely sufficient)
    BME280_Sensor(BME280_Sensor&&) noexcept = default;
    BME280_Sensor& operator=(BME280_Sensor&&) noexcept = delete;


private:
    // --- Private Helper Methods ---
    bool checkDevice();
    bool readCalibrationData();
    bool configureSensor();
    std::optional<std::vector<uint8_t>> readRawMeasurementData();

    // --- Compensation Calculations ---
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

    // --- Member Variables ---
    SensorHub::Interfaces::II2C_Bus& i2c_bus_ref_; // Store INTERFACE reference
    uint8_t address_;                             // Sensor's specific I2C address
    bool initialized_ = false;                    // Flag indicating successful initialization
};

} // namespace SensorHub::Components