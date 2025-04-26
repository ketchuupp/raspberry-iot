#include "SensorDummy/sensor_dummy.h"
#include <nlohmann/json.hpp> // Use full json header here
#include <stdexcept>
#include <iostream>
#include <chrono>

using namespace SensorHub::Interfaces;
using json = nlohmann::json;

namespace SensorHub::Components {

// --- Factory Method ---
std::unique_ptr<ISensor> SensorDummy::create(
    const SensorConfig& config
    /*, dependencies... */)
{
    // Check if the type matches what this factory creates
    if (config.type != "Dummy") {
        return nullptr; // Not meant for this factory
    }
    try {
        // Create and return the sensor instance
        auto sensor_ptr = std::unique_ptr<ISensor>(new SensorDummy(config));
        return sensor_ptr;
    } catch (const std::exception& e) {
        std::cerr << "DummySensor Error: Failed to create instance: " << e.what() << std::endl;
        return nullptr;
    }
}

// --- Constructor ---
SensorDummy::SensorDummy(const SensorConfig& config)
    : config_(config)
{
    if (!config_.enabled) {
        throw std::runtime_error("DummySensor: Attempted to initialize a disabled sensor.");
    }
    // Add any dummy-specific validation from config if needed
    // e.g., check for a required "dummy_parameter" in the json

    initialized_ = true; // Assume success for dummy
    std::cout << "Dummy Sensor initialized successfully (Suffix: "
              << config_.publish_topic_suffix << ")" << std::endl;
}

// --- ISensor Interface Method Implementations ---

std::string SensorDummy::getType() const {
    return config_.type;
}

bool SensorDummy::isEnabled() const {
    return config_.enabled;
}

std::chrono::seconds SensorDummy::getPublishInterval() const {
    // Return specific interval from config, or a default if not set/parsed
    return config_.publish_interval;
}

std::string SensorDummy::getTopicSuffix() const {
    return config_.publish_topic_suffix;
}

// --- Data Reading ---
// Returns a simple JSON object with dummy data
nlohmann::json SensorDummy::readDataJson() {
    json result = json::object();
    if (!initialized_) {
        result["error"] = "Sensor not initialized";
        return result;
    }

    std::cout << "[Dummy Sensor]: Reading data..." << std::endl;

    // Simulate some changing data
    counter_++;
    result["counter"] = counter_;
    result["status"] = "OK";
    result["random_value"] = (rand() % 1000) / 10.0; // Example random data

    return result;
}

} // namespace SensorHub::Components
