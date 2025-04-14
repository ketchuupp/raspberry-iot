#pragma once

#include "Interfaces/ii2c_bus.h" // The interface to mock
#include "gmock/gmock.h"        // GoogleMock framework
#include <vector>
#include <optional>
#include <string>
#include <cstdint>

namespace SensorHub::Tests {

// Mock class inheriting from the II2C_Bus interface
class MockI2C_Bus : public SensorHub::Interfaces::II2C_Bus {
public:
    // Use MOCK_METHOD to mock each virtual function from the interface
    // Format: MOCK_METHOD(ReturnType, MethodName, (ArgType1, ArgType2...), (override));
    // Use Const() qualifier for const methods.

    MOCK_METHOD(bool, writeByteData, (uint8_t device_address, uint8_t reg, uint8_t value), (override));
    MOCK_METHOD(std::optional<uint8_t>, readByteData, (uint8_t device_address, uint8_t reg), (override));
    MOCK_METHOD(std::optional<std::vector<uint8_t>>, readBlockData, (uint8_t device_address, uint8_t start_reg, size_t count), (override));
    MOCK_METHOD(bool, writeBlockData, (uint8_t device_address, uint8_t start_reg, const std::vector<uint8_t>& data), (override));
    MOCK_METHOD(bool, probeDevice, (uint8_t device_address), (override));
    MOCK_METHOD(const std::string&, getBusPath, (), (const, override));

    // Provide a default implementation for getBusPath for convenience in tests
    // You can override this with EXPECT_CALL(...).WillRepeatedly(...) if needed.
    MockI2C_Bus() {
        ON_CALL(*this, getBusPath).WillByDefault(testing::ReturnRef(mock_bus_path_));
    }

private:
    // Member to back the default implementation of getBusPath()
    std::string mock_bus_path_ = "mock_bus";
};

} // namespace SensorHub::Tests
