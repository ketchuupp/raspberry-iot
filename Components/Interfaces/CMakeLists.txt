set(componentName Interfaces)

set(include_path_public "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(include_path_private "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Define source file for the helper function
# set(source_files ${source_path}/sensor_config.cpp)

# Define public header files
set(include_files_public
    ${include_path_public}/Interfaces/ii2c_bus.h
    ${include_path_public}/Interfaces/isensor.h
    ${include_path_public}/Interfaces/sensor_config.h
)

# Create an INTERFACE library as it mainly provides headers and potentially
# shared types/helpers, but doesn't necessarily produce a linkable library itself.
# If sensor_config.cpp grows, could change to STATIC/SHARED library.
add_library(${componentName} INTERFACE)

# Add source file to the interface library (makes it build if library type changes)
# target_sources(${componentName} INTERFACE ${source_files})

# Specify public include directories
target_include_directories(${componentName} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include> # Install path relative to CMAKE_INSTALL_PREFIX
)

# Specify public dependencies (e.g., nlohmann_json needed for headers)
target_link_libraries(${componentName} INTERFACE nlohmann_json::nlohmann_json)

# Installation for headers (if needed separately from components)
# install(FILES ${include_files_public} DESTINATION include/Interfaces)
# Better: Install via components that use them or install the INTERFACE target includes
# install(TARGETS ${componentName} EXPORT InterfacesTargets)
# install(EXPORT InterfacesTargets
#   NAMESPACE SensorHub::
#   DESTINATION lib/cmake/SensorHub/Interfaces
# )
# Install headers associated with the INTERFACE library
install(DIRECTORY include/Interfaces DESTINATION include)

