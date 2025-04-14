# CompileOptions.cmake

set(CMAKE_CXX_STANDARD 23 CACHE STRING "the c++ standard to use for this project")
set(CMAKE_CXX_STANDARD_REQUIRED OUTPUT_NAME)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(GCC_COMMON_FLAGS
    # -Wall
    # -Wextra
    # -Wpedantic
    # -Werror
    # -Wconversion
    # -Wunused-variable
    # -Wshadow
    # -Wnon-virtual-dtor
    # -Wold-style-cast
    # -Wduplicated-cond
    # -Wlogical-op
    # -fstack-protector-strong
    # -fno-omit-frame-pointer
    # -fPIC
)

set(CMAKE_CXX_FLAGS_DEBUG ${GCC_COMMON_FLAGS} -g)
set(CMAKE_CXX_FLAGS_RELEASE ${GCC_COMMON_FLAGS} -O3)

set(CMAKE_C_FLAGS_DEBUG ${GCC_COMMON_FLAGS} -g)
set(CMAKE_C_FLAGS_RELEASE ${GCC_COMMON_FLAGS} -O3)


# Include directories
set(DEFAULT_INCLUDE_DIRECTORIES)

# Libraries
set(DEFAULT_LIBRARIESs)

# Compile definitions
set(DEFAULT_COMPILE_DEFINITIONS)

# Compile options
set(DEFAULT_COMPILE_OPTIONS)

# Linker options
set(DEFAULT_LINKER_OPTIONS)


message("-----------------------------------------------------------")
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message(" C Debug     = ${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_DEBUG}")
message(" C Release   = ${CMAKE_C_FLAGS}   ${CMAKE_C_FLAGS_RELEASE}")
message(" C++ Debug   = ${CMAKE_CXX_FLAGS}   ${CMAKE_CXX_FLAGS_DEBUG}")
message(" C++ Release = ${CMAKE_CXX_FLAGS}   ${CMAKE_CXX_FLAGS_RELEASE}")
message("-----------------------------------------------------------")