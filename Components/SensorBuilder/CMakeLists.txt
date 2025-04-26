# -----------------------------------------------------------------------------
# Component name
# -----------------------------------------------------------------------------
set(componentName SensorBuilder)
set(componentLib ${componentName})

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------
set(include_path_public "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(include_path_private "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(include_files_public
    ${include_path_public}/${componentName}/sensor_builder.h
    )

set(include_files_private
    )

set(source_files
    ${source_path}/sensor_builder.cpp
    )

# -----------------------------------------------------------------------------
# Create library
# -----------------------------------------------------------------------------
add_library(${componentLib}
    ${include_files_public}
    ${include_files_private}
    ${source_files}
    )

# -----------------------------------------------------------------------------
# Include direcories
# -----------------------------------------------------------------------------
target_include_directories(${componentLib} 
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src

    PUBLIC
    ${DEFAULT_INCLUDE_DIRECTORIES}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:Include>

    INTERFACE
)

# -----------------------------------------------------------------------------
# Dependencies to other libraries
# -----------------------------------------------------------------------------
target_link_libraries(${componentLib}
    PRIVATE
    SensorBME280
    SensorDummy
    SensorLPS25HB
    LinuxI2C_Manager

    PUBLIC
    ${DEFAULT_LIBRARIES}
    Interfaces
    nlohmann_json::nlohmann_json


    INTERFACE
)

# -----------------------------------------------------------------------------
# Compile definitions
# -----------------------------------------------------------------------------
target_compile_definitions(${componentLib}
    PRIVATE
    
    PUBLIC
    ${DEFAULT_COMPILE_DEFINITIONS}

    INTERFACE
    )

# -----------------------------------------------------------------------------
# Compile options
# -----------------------------------------------------------------------------
target_compile_options(${componentLib}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
)

# -----------------------------------------------------------------------------
# Deployment
# -----------------------------------------------------------------------------

install(TARGETS ${componentLib}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    )

install(DIRECTORY include/ DESTINATION include)

# -----------------------------------------------------------------------------
# Unit tests
# -----------------------------------------------------------------------------
# include(Test.cmake)
