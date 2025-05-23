# -----------------------------------------------------------------------------
# Component name (Application)
# -----------------------------------------------------------------------------
set(applicationName ${APPLICATION_NAME})

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------
set(include_path_public "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(include_path_private "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(include_files_public
    ${include_path_public}/App/app.h
    )

set(source_files
    ${source_path}/app.cpp
    ${source_path}/main.cpp
    )

# -----------------------------------------------------------------------------
# Create application executable
# -----------------------------------------------------------------------------
add_executable(${applicationName}
    ${source_files}
)

# -----------------------------------------------------------------------------
# Include directories (Private for the application build)
# -----------------------------------------------------------------------------
target_include_directories(${applicationName} 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -----------------------------------------------------------------------------
# Dependencies to other libraries (Link Components and Platform I2C Impl)
# -----------------------------------------------------------------------------
target_link_libraries(${applicationName} 
    PRIVATE
    ${DEFAULT_LIBRARIES}

    SensorBME280
    NetworkMQTT
    SensorBuilder
    nlohmann_json::nlohmann_json
    LinuxI2C_Manager
    # Add other component library targets here
)

# -----------------------------------------------------------------------------
# Compile definitions (Inherited + App Specific)
# -----------------------------------------------------------------------------
target_compile_definitions(${applicationName}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_DEFINITIONS}

    INTERFACE

    )

# -----------------------------------------------------------------------------
# Compile options (Inherited + App Specific)
# -----------------------------------------------------------------------------
target_compile_options(${applicationName}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
    )
