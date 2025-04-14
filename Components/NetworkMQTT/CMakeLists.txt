# -----------------------------------------------------------------------------
# Component name
# -----------------------------------------------------------------------------
set(componentName NetworkMQTT)
set(componentLib ${componentName})

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------
set(include_path_public "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(include_path_private "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(include_files_public
    ${include_path_public}/${componentName}/mqtt_publisher.h
    )

set(include_files_private
    )

set(source_files
    ${source_path}/mqtt_publisher.cpp
    )

# Find Threads (still required by Paho C++)
find_package(Threads REQUIRED)

# Find OpenSSL IF Paho C was built with SSL (PAHO_WITH_SSL was set to OFF)
# if(PAHO_WITH_SSL)
#    find_package(OpenSSL REQUIRED)
# endif()

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
    ${PAHO_CPP_INCLUDE_DIR}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:Include>

    INTERFACE
)

# -----------------------------------------------------------------------------
# Dependencies to other libraries
# -----------------------------------------------------------------------------
target_link_libraries(${componentLib}
    PRIVATE

    PUBLIC
    ${DEFAULT_LIBRARIES}
    Threads::Threads
    PahoMqttCpp::paho-mqttpp3

    # Link OpenSSL if needed
    # if(PAHO_WITH_SSL)
    #    OpenSSL::SSL OpenSSL::Crypto
    # endif()

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
install(FILES ${include_files_public} DESTINATION include/NetworkMQTT)

# -----------------------------------------------------------------------------
# Unit tests
# -----------------------------------------------------------------------------
# include(Test.cmake)
