# This file handles fetching and making available external libraries

include(FetchContent)
find_package(Git QUIET) # Needed only if PATCH_COMMAND uses git apply

# --- nlohmann/json ---
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG        v3.12.0
)
FetchContent_MakeAvailable(json)


# --- Paho MQTT C++ Library (Now builds bundled C lib) ---
# Set options BEFORE FetchContent_MakeAvailable
set(PAHO_WITH_MQTT_C ON CACHE BOOL "Build bundled Paho C library")
set(PAHO_BUILD_STATIC ON CACHE BOOL "Prefer static Paho C/C++ libs") # Build static libs
set(PAHO_BUILD_SHARED OFF CACHE BOOL "Disable shared Paho C/C++ libs") # Explicitly disable shared
set(PAHO_WITH_SSL OFF CACHE BOOL "Build bundled Paho C without SSL") # Disable SSL for bundled C lib if needed
set(PAHO_BUILD_SAMPLES OFF CACHE BOOL "Do not build Paho C++ samples")
set(PAHO_INSTALL_DOCS OFF CACHE BOOL "Do not install Paho C++ docs")

FetchContent_Declare(
    paho_mqtt_cpp
    GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.cpp.git
    GIT_TAG        v1.5.2
    GIT_SUBMODULES # Fetches default submodules (like paho.mqtt.c)
)
FetchContent_MakeAvailable(paho_mqtt_cpp)

