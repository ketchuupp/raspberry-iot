#include "SensorBuilder/sensor_builder.h"
#include "Interfaces/sensor_config.h"
#include "SensorBME280/bme280_sensor.h"
#include "SensorDummy/sensor_dummy.h"
#include "SensorLPS25HB/sensor_lps25hb.h"
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>
#include <cstdlib> // For std::strtoul

// Conditional Includes for Concrete I2C Managers
#include "LinuxI2C_Manager/linux_i2c_manager.h"

using namespace SensorHub::Interfaces;
using namespace SensorHub::Components;
using namespace nlohmann;

namespace SensorHub::Builder {

// Helper to parse hex string (could be moved to a common utility)
uint8_t parse_hex_address_builder(const std::string& addr_str) {
     if (addr_str.length() < 3 || addr_str.substr(0, 2) != "0x") {
        throw std::invalid_argument("Invalid hex address format (must be 0xNN): " + addr_str);
    }
    try {
        char* end_ptr = nullptr;
        unsigned long value = std::strtoul(addr_str.c_str(), &end_ptr, 16);
        if (end_ptr == nullptr || *end_ptr != '\0' || value > 0xFF) {
             throw std::invalid_argument("Invalid character or range in hex address: " + addr_str);
        }
        return static_cast<uint8_t>(value);
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to parse hex address '" + addr_str + "': " + e.what());
    }
}

SensorBuilder::SensorBuilder() = default;
// Destructor needs to be defined (even if empty) because unique_ptr needs
// the complete type definition of II2C_Bus at destruction time.
SensorBuilder::~SensorBuilder() = default;

// Gets or creates I2C Manager instance for a specific bus path
std::shared_ptr<II2C_Bus> SensorBuilder::getI2CManager(const std::string& bus_path) {
    auto it = i2c_managers_.find(bus_path);
    if (it == i2c_managers_.end()) {
        std::cout << "SensorBuilder: Creating new I2C Manager for bus: " << bus_path << std::endl;
        // Use make_shared for shared ownership from the start
        std::shared_ptr<II2C_Bus> new_manager = std::make_shared<I2C_Manager>(bus_path);
        it = i2c_managers_.emplace(bus_path, new_manager).first; // Store shared_ptr
    }
    // Return a copy of the shared_ptr
    return it->second;
}

// Builds sensor instances from the JSON config array
std::vector<std::unique_ptr<ISensor>> SensorBuilder::buildSensors(const nlohmann::json& sensor_configs_json)
{
    std::vector<std::unique_ptr<ISensor>> sensors;
    std::cout << "SensorBuilder: Building sensors from configuration..." << std::endl;

    if (!sensor_configs_json.is_array()) {
        throw std::runtime_error("SensorBuilder Error: 'sensors' configuration is not a JSON array.");
    }

    for (const auto& j_sensor : sensor_configs_json) {
        if (!j_sensor.is_object()) {
             std::cerr << "SensorBuilder Warning: Non-object entry in 'sensors' array, skipping." << std::endl;
             continue;
        }

        SensorConfig config;
        // Parse common fields first
        if (!SensorConfig::parseCommon(j_sensor, config)) {
            // Sensor is disabled or basic parsing failed, skip it
            continue;
        }

        std::unique_ptr<ISensor> sensor_ptr = nullptr;

        try {
            // --- Sensor Creation Logic based on Type ---
            if (config.type == "BME280") {
                // Parse BME280 specific fields
                config.i2c_bus = j_sensor.at("i2c_bus").get<std::string>();
                std::string addr_str = j_sensor.at("i2c_address").get<std::string>();
                config.i2c_address = parse_hex_address_builder(addr_str); // Use helper

                // Get or create the required I2C bus manager
                std::shared_ptr<II2C_Bus> i2c_bus_sptr = getI2CManager(config.i2c_bus);

                // Call the static factory method of the sensor implementation
                sensor_ptr = BME280_Sensor::create(config, i2c_bus_sptr);

            }
            else if (config.type == "LPS25HB") {
                config.i2c_bus = j_sensor.at("i2c_bus").get<std::string>();
                std::string addr_str = j_sensor.at("i2c_address").get<std::string>();
                config.i2c_address = parse_hex_address_builder(addr_str);

                std::shared_ptr<II2C_Bus> i2c_bus_sptr = getI2CManager(config.i2c_bus);
                sensor_ptr = SensorLPS25HB::create(config, i2c_bus_sptr); // Call LPS factory
            }
            else if (config.type == "Dummy") {
                // Parse any dummy-specific config fields if needed
                // config.some_dummy_param = j_sensor.at("dummy_param").get<int>();

                // Call the static factory method
                sensor_ptr = SensorDummy::create(config); // Doesn't need extra dependencies currently
            }
            else {
                std::cerr << "SensorBuilder Warning: Unknown sensor type '" << config.type << "' defined in config. Skipping." << std::endl;
            }

            // Add successfully created sensor to the list
            if (sensor_ptr) {
                std::cout << "SensorBuilder: Successfully created sensor instance for type '" << config.type << "' with suffix '" << config.publish_topic_suffix << "'." << std::endl;
                sensors.push_back(std::move(sensor_ptr));
            } else {
                 std::cerr << "SensorBuilder Warning: Failed to create sensor instance for type '" << config.type << "' (config suffix: " << config.publish_topic_suffix << ")." << std::endl;
            }

        } catch (const json::out_of_range& e) {
            std::cerr << "SensorBuilder Warning: Missing required configuration key for sensor type '" << config.type << "': " << e.what() << ". Skipping sensor." << std::endl;
        } catch (const json::type_error& e) {
            std::cerr << "SensorBuilder Warning: Incorrect type for configuration key for sensor type '" << config.type << "': " << e.what() << ". Skipping sensor." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "SensorBuilder Warning: Error processing configuration for sensor type '" << config.type << "': " << e.what() << ". Skipping sensor." << std::endl;
        }

    } // end for loop

    if (sensors.empty()) {
         std::cerr << "SensorBuilder Warning: No sensors were successfully created from the configuration." << std::endl;
    }

    std::cout << "SensorBuilder: Finished building sensors. Created " << sensors.size() << " instances." << std::endl;
    return sensors;
}

} // namespace SensorHub::Builder
