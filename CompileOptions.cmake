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
    -Wduplicated-cond # <<< GCC specific
    -Wlogical-op      # <<< GCC specific
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
    # -Wduplicated-cond # <<< REMOVED (Not standard Clang)
    # -Wlogical-op      # <<< REMOVED (Not standard Clang)
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
set(DEFAULT_COMPILE_OPTIONS "") # Initialize empty LIST
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Compiler is GCC - Setting DEFAULT_COMPILE_OPTIONS to GCC flags list")
    set(DEFAULT_COMPILE_OPTIONS ${GCC_COMMON_FLAGS}) # Assign the LIST
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang") # Catches Clang and AppleClang
    message(STATUS "Compiler is Clang/AppleClang - Setting DEFAULT_COMPILE_OPTIONS to Clang flags list")
    set(DEFAULT_COMPILE_OPTIONS ${CLANG_COMMON_FLAGS}) # Assign the LIST
else()
    message(WARNING "Compiler ID ${CMAKE_CXX_COMPILER_ID} not recognized - DEFAULT_COMPILE_OPTIONS left empty.")
endif()

# Other default variables
set(DEFAULT_INCLUDE_DIRECTORIES "")
set(DEFAULT_LIBRARIES "")
set(DEFAULT_COMPILE_DEFINITIONS "")
# DEFAULT_COMPILE_OPTIONS is set above
set(DEFAULT_LINKER_OPTIONS "")

# --- Final Output Message ---
# (message block remains the same)
message("-----------------------------------------------------------")
# ...
message(" DEFAULT_COMPILE_OPTIONS = ${DEFAULT_COMPILE_OPTIONS}")
message("-----------------------------------------------------------")