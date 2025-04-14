
# -----------------------------------------------------------------------------
# Test application name
# -----------------------------------------------------------------------------
set(IOTest Test-${componentName})

# -----------------------------------------------------------------------------
# Create test executable
# -----------------------------------------------------------------------------
add_executable(${IOTest}
    Test/Test.cpp
    Test/Test-SensorBME280.cpp)
    
# -----------------------------------------------------------------------------
# Include directories
# -----------------------------------------------------------------------------
target_include_directories(${IOTest} 
    # SYSTEM
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${include_files_private}
    ${CMAKE_SOURCE_DIR}/Components/Interfaces/include
    # Include path for Mocks (adjust path as needed)
    ${CMAKE_SOURCE_DIR}/Tests/Mocks

    PUBLIC
    ${DEFAULT_INCLUDE_DIRECTORIES}

    INTERFACE
)

# -----------------------------------------------------------------------------
# Dependencies to other libraries
# -----------------------------------------------------------------------------
target_link_libraries(${IOTest}
    PRIVATE

    PUBLIC
    ${componentLib}
    Interfaces
    gtest
    gmock
    ${DEFAULT_LIBRARIES}


    INTERFACE
)

# -----------------------------------------------------------------------------
# Compile definitions
# -----------------------------------------------------------------------------
target_compile_definitions(${IOTest}
    PRIVATE
    
    PUBLIC
    ${DEFAULT_COMPILE_DEFINITIONS}

    INTERFACE
    )

# -----------------------------------------------------------------------------
# Compile options
# -----------------------------------------------------------------------------
target_compile_options(${IOTest}
    PRIVATE
    -O0

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
)

# -----------------------------------------------------------------------------
# Add test
# -----------------------------------------------------------------------------
# add_test(NAME ${IOTest} COMMAND $<TARGET_FILE:${IOTest}>)
add_test(NAME ${IOTest} COMMAND ${IOTest})