add_subdirectory(Interfaces)

# Conditionally add the platform-specific I2C implementation based on the flag
# BUILD_TARGET_RPI should be set to TRUE via the toolchain file for RPi builds
if(BUILD_TARGET_RPI)
    message(STATUS "Target is Raspberry Pi (BUILD_TARGET_RPI=TRUE). Adding Linux I2C Manager.")
    add_subdirectory(LinuxI2C_Manager)
else()
    message(STATUS "Target is NOT Raspberry Pi (BUILD_TARGET_RPI is FALSE or Undefined). Adding Stub I2C Manager.")
    add_subdirectory(StubI2C_Manager)
endif()

# add_subdirectory(Foo)
add_subdirectory(SensorBME280)
add_subdirectory(NetworkMQTT)