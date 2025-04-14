# -----------------------------------------------------------------------------
# Component name
# -----------------------------------------------------------------------------
set(componentName LinuxI2C_Manager)
set(componentLib ${componentName})

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------
set(include_path_public "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(include_path_private "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(include_files_public
    ${include_path_public}/${componentName}/linux_i2c_manager.h
    )

set(include_files_private
    )

set(source_files
    ${source_path}/linux_i2c_manager.cpp
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
    ${I2C_INCLUDE_DIRS}
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
    # ${I2C_LIBRARIES}     # Link libi2c from pkg-config
    Interfaces

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

install(FILES ${include_files_public} DESTINATION include/I2C_Manager) # Install header correctly

# -----------------------------------------------------------------------------
# Unit tests
# -----------------------------------------------------------------------------
# include(Test.cmake)
