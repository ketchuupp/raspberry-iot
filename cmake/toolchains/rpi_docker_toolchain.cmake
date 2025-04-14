# CMake Toolchain file for RPi using downloaded ARM GCC Toolchain inside Docker

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# --- Compilers ---
# Use paths inside the downloaded toolchain (match ENV vars set in Dockerfile)
if(NOT DEFINED ENV{CC} OR NOT DEFINED ENV{CXX})
    message(FATAL_ERROR "CC and CXX environment variables not set!")
endif()
set(CMAKE_C_COMPILER   $ENV{CC})
set(CMAKE_CXX_COMPILER $ENV{CXX})
message(STATUS "Docker Toolchain: Using C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Docker Toolchain: Using C++ Compiler: ${CMAKE_CXX_COMPILER}")

# --- Sysroot ---
# The sysroot is usually relative to the compiler within the toolchain structure
# Setting CMAKE_SYSROOT might not even be strictly necessary if the compiler finds it implicitly
# If needed, set it based on the toolchain layout (check after unpacking)
# set(CMAKE_SYSROOT "$ENV{RPI_SYSROOT}") # e.g., /opt/arm-gcc-11/arm-none-linux-gnueabihf/libc
# For self-contained toolchains, often just letting the compiler handle it is best.
# Let's try commenting out CMAKE_SYSROOT setting first.
# if(DEFINED ENV{RPI_SYSROOT} AND EXISTS "$ENV{RPI_SYSROOT}")
#    set(CMAKE_SYSROOT "$ENV{RPI_SYSROOT}")
#    message(STATUS "Docker Toolchain: Using Sysroot: ${CMAKE_SYSROOT}")
#    set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
# else()
#    message(WARNING "Sysroot environment variable RPI_SYSROOT not set or path invalid. Relying on compiler defaults.")
# endif()


# --- CMake Find Behavior ---
# These might need adjustment depending on whether CMAKE_SYSROOT is set
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY) # Search only sysroot/toolchain paths first
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Remove explicit linker flags, let the self-contained toolchain manage its paths
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_SYSROOT}/lib" CACHE STRING "Linker flags" FORCE)


# --- Application Specific Flags ---
set(BUILD_TARGET_RPI TRUE CACHE BOOL "Explicitly building for Raspberry Pi target")
message(STATUS "Docker Toolchain: Setting BUILD_TARGET_RPI=${BUILD_TARGET_RPI}")