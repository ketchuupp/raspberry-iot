# CompileOptions.cmake

set(CMAKE_CXX_STANDARD 23 CACHE STRING "the c++ standard to use for this project")
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, etc.)" FORCE)
endif()

# --- Define common flags for GCC as a CMake LIST ---
# Separate flags with spaces or newlines INSIDE the set() command.
set(GCC_COMMON_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Werror # Treat warnings as errors
    # -Wconversion
    -Wunused-variable
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wduplicated-cond
    -Wlogical-op
    -fstack-protector-strong
    -fno-omit-frame-pointer
    -fPIC
)

# --- Define common flags for Clang as a CMake LIST ---
# Separate flags with spaces or newlines INSIDE the set() command.
set(CLANG_COMMON_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Werror # Treat warnings as errors
    # -Wconversion # TODO fix code and enable
    # -Wunused-variable # Often covered well by -Wall in Clang
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    # -Wduplicated-cond
    # -Wlogical-op
    -fstack-protector-strong
    -fno-omit-frame-pointer
    -fPIC
    # -Wdocumentation   # Example Clang-specific flag
)

# --- Initialize Base Global Flags ---
set(CMAKE_C_FLAGS_DEBUG "-g" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O3" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "-g" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3" CACHE STRING "" FORCE)

# --- Default propagation variables ---

# Set DEFAULT_COMPILE_OPTIONS based on the HOST compiler (for tests)
set(DEFAULT_COMPILE_OPTIONS "") # Initialize empty LIST
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "[Host Build] Compiler is GCC - Setting DEFAULT_COMPILE_OPTIONS to GCC flags list")
    set(DEFAULT_COMPILE_OPTIONS ${GCC_COMMON_FLAGS})
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin" AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "[Host Build] Compiler is Clang/AppleClang - Setting DEFAULT_COMPILE_OPTIONS to Clang flags list")
    set(DEFAULT_COMPILE_OPTIONS ${CLANG_COMMON_FLAGS})
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU") # Fallback GCC on other hosts
     message(STATUS "[Host Build] Compiler is GCC (non-Linux) - Setting DEFAULT_COMPILE_OPTIONS to GCC flags list")
     set(DEFAULT_COMPILE_OPTIONS ${GCC_COMMON_FLAGS})
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # Fallback Clang on other hosts
     message(STATUS "[Host Build] Compiler is Clang (non-macOS) - Setting DEFAULT_COMPILE_OPTIONS to Clang flags list")
     set(DEFAULT_COMPILE_OPTIONS ${CLANG_COMMON_FLAGS})
else()
    message(WARNING "[Host Build] Host Compiler ID ${CMAKE_CXX_COMPILER_ID} not recognized - DEFAULT_COMPILE_OPTIONS left empty.")
endif()

# Other default variables
set(DEFAULT_INCLUDE_DIRECTORIES "")
set(DEFAULT_LIBRARIES "")
set(DEFAULT_COMPILE_DEFINITIONS "") # Initialize empty list
set(DEFAULT_LINKER_OPTIONS "")


# --- Final Output Message ---
message("-----------------------------------------------------------")
message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message(" Host C Compiler ID = ${CMAKE_C_COMPILER_ID}") # Renamed for clarity
message(" Host C++ Compiler ID = ${CMAKE_CXX_COMPILER_ID}") # Renamed for clarity
message(" C Debug     = ${CMAKE_C_FLAGS_DEBUG}")
message(" C Release   = ${CMAKE_C_FLAGS_RELEASE}")
message(" C++ Debug   = ${CMAKE_CXX_FLAGS_DEBUG}")
message(" C++ Release = ${CMAKE_CXX_FLAGS_RELEASE}")
message(" DEFAULT_COMPILE_OPTIONS = ${DEFAULT_COMPILE_OPTIONS}") # For host tests
message(" DEFAULT_COMPILE_DEFINITIONS = ${DEFAULT_COMPILE_DEFINITIONS}")
message("-----------------------------------------------------------")