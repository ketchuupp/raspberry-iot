cmake_minimum_required(VERSION 3.14)

# Set the project name and version
project(MyProject VERSION 1.0)

# Set the application name as a CMake variable
set(APPLICATION_NAME "MyApp")

# Compile settings and options
include(CompileOptions.cmake)

# Set output directories
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIRECTORY}/Binaries/Bin")
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIRECTORY}/Binaries/Lib")
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIRECTORY}/Binaries/Lib")

# Enable testing
enable_testing()

# Project modules
add_subdirectory(Externals)
add_subdirectory(Components)
add_subdirectory(Application)


