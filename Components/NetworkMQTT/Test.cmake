
# -----------------------------------------------------------------------------
# Test application name
# -----------------------------------------------------------------------------
set(IOTest Test-${componentName})

# -----------------------------------------------------------------------------
# Create test executable
# -----------------------------------------------------------------------------
add_executable(${IOTest}
    Test/Test.cpp
    Test/Test-foo.cpp)
    
# -----------------------------------------------------------------------------
# Include directories
# -----------------------------------------------------------------------------
target_include_directories(${IOTest} 
    # SYSTEM
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${include_files_private}

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