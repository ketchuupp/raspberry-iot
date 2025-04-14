# -----------------------------------------------------------------------------
# Component name (Application)
# -----------------------------------------------------------------------------
# Use variable from root CMakeLists.txt if defined (e.g., APPLICATION_NAME)
# Or define locally if preferred:
set(applicationName myApp)

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(source_files
    ${source_path}/main.cpp
    # Add other application-specific source files here if any
)

# -----------------------------------------------------------------------------
# Create application executable
# -----------------------------------------------------------------------------
add_executable(${applicationName}
    ${source_files}
)

# -----------------------------------------------------------------------------
# Define Platform Macro for C++ Code
# -----------------------------------------------------------------------------
if(BUILD_TARGET_RPI)
    set(TARGET_PLATFORM_DEFINITION PLATFORM_LINUX_RPI) # More specific now
else()
    set(TARGET_PLATFORM_DEFINITION PLATFORM_STUB) # All non-RPi builds use stub
endif()
target_compile_definitions(${applicationName} PRIVATE ${TARGET_PLATFORM_DEFINITION})



# -----------------------------------------------------------------------------
# Include directories (Private for the application build)
# -----------------------------------------------------------------------------
target_include_directories(${applicationName} 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# -----------------------------------------------------------------------------
# Dependencies to other libraries (Link Components and Platform I2C Impl)
# -----------------------------------------------------------------------------
target_link_libraries(${applicationName} 
    PRIVATE
    ${DEFAULT_LIBRARIES}

    SensorBME280
    NetworkMQTT
    nlohmann_json::nlohmann_json
    # Add other component library targets here
)

# Conditionally link the CONCRETE I2C implementation library
if(BUILD_TARGET_RPI)
    target_link_libraries(${applicationName} PRIVATE LinuxI2C_Manager)
else() # Fallback for ALL other systems
    target_link_libraries(${applicationName} PRIVATE StubI2C_Manager)
endif()

# Conditionally link the *concrete* I2C implementation library target
# This ensures the symbols needed by SensorBME280 (which uses the II2C_Bus interface) are resolved
# if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
#     target_link_libraries(${applicationName} PRIVATE LinuxI2C_Manager)
# elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
#     target_link_libraries(${applicationName} PRIVATE StubI2C_Manager)
# else() # Fallback for other systems
#     target_link_libraries(${applicationName} PRIVATE StubI2C_Manager)
# endif()


# -----------------------------------------------------------------------------
# Compile definitions (Inherited + App Specific)
# -----------------------------------------------------------------------------
target_compile_definitions(${applicationName}
    PRIVATE
        # Application-specific compile definitions if any (e.g., -DMYAPP_FEATURE)
    PUBLIC
        ${DEFAULT_COMPILE_DEFINITIONS}
    INTERFACE
        # Definitions needed by targets linking this *if* it were a library
    )

# -----------------------------------------------------------------------------
# Compile options (Inherited + App Specific)
# -----------------------------------------------------------------------------
target_compile_options(${applicationName}
    PRIVATE
        # Application-specific compile options if any (e.g., -Wno-...)
    PUBLIC
        # Inherit common compile options set in CompileOptions.cmake
        ${DEFAULT_COMPILE_OPTIONS}
    INTERFACE
        # Compile options needed by targets linking this *if* it were a library
    )

# -----------------------------------------------------------------------------
# Unit tests (Application executable usually doesn't have its own tests here)
# Tests are typically defined within each component's Test.cmake file.
# -----------------------------------------------------------------------------
# include(Test.cmake)