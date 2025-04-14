# Raspberry Pi Sensor Hub (MQTT Publisher)

## Overview

This project implements a C++ application designed to run on a Raspberry Pi (or natively on macOS using a stub for testing). It reads data from a BME280 sensor (temperature, humidity, pressure) via I2C and publishes this data to an MQTT broker.

The project utilizes modern C++ (C++23), CMake for building, and supports cross-compilation for Raspberry Pi (ARM Linux) from a macOS host using Docker. Dependencies like Paho MQTT, GoogleTest, and nlohmann/json are managed via CMake's FetchContent.

An accompanying HTML/JavaScript dashboard (`dashboard.html` - *Note: You need to save this separately*) can subscribe to the MQTT broker to display the sensor data in real-time.

## Features

* Reads Temperature, Humidity, and Pressure from BME280 sensor.
* Publishes data to a configurable MQTT topic in JSON format.
* Abstracted I2C interface with implementations for:
    * Linux (`ioctl`-based) for Raspberry Pi.
    * Stub (for testing/running on non-Linux hosts like macOS).
* Cross-compilation support for Raspberry Pi (arm-linux-gnueabihf) using Docker.
* Native macOS build support (uses I2C Stub).
* Dependencies managed via CMake FetchContent.
* Unit testing framework (GoogleTest) configured for native builds.
* Simple build scripts for different targets.

## Prerequisites

* **Common:**
    * CMake (version 3.14 or higher)
    * Git
    * A C++23 compliant compiler (GCC 14+ or recent Clang/AppleClang recommended)
    * Ninja build system (optional, but used by the build scripts): `brew install ninja` on macOS.
* **For Raspberry Pi Cross-Compilation (using Docker):**
    * Docker Desktop (or Docker Engine on Linux) installed and running.
* **For Running the Application:**
    * An MQTT Broker (like Mosquitto) accessible by the application and dashboard.
        * Must be configured to listen on TCP (default 1883) for the C++ app.
        * Must be configured to listen on WebSockets (e.g., 9001) for the HTML dashboard.
        * Install on macOS: `brew install mosquitto` (See [Setup Guide](#setup-guide) below).
* **Hardware (for RPi target):**
    * Raspberry Pi
    * BME280 Sensor connected via I2C.

## Setup

1.  **Clone the Repository:**
    ```bash
    git clone https://github.com/ketchuupp/raspberry-iot
    cd raspberry-iot
    ```
2.  **Dependencies:** External libraries (Paho MQTT C/C++, GoogleTest, nlohmann/json) are automatically downloaded and built by CMake using FetchContent during the configuration step. No manual installation is needed.

## Building the Application

Use the provided scripts from the project root directory:

* **For Raspberry Pi (Cross-Compile using Docker):**
    1.  Build the Docker image (only needs to be done once or if `Dockerfile` changes):
        ```bash
        docker build -t myapp-rpi-builder .
        ```
    2.  Run the build script:
        ```bash
        ./build_rpi.sh
        ```
    3.  Output: The ARM Linux executable will be located in `build/rpi_build/Application/myApp`.

* **For Native macOS (Uses I2C Stub):**
    1.  Run the build script:
        ```bash
        ./build_macos.sh
        ```
    2.  Output: The native macOS executable will be located in `build/macos_build/Application/myApp`.

## Running

1.  **Setup MQTT Broker:**
    * Install Mosquitto (if not already done): `brew install mosquitto`
    * Configure it for TCP (1883) and WebSockets (e.g., 9001). Edit `/opt/homebrew/etc/mosquitto/mosquitto.conf` (or add a file in `conf.d/`) with:
        ```conf
        listener 1883
        protocol mqtt

        listener 9001
        protocol websockets

        allow_anonymous true # For easy testing
        ```
    * Start/Restart the broker: `brew services restart mosquitto`
    * Verify it's running: `brew services list`

2.  **Run the C++ Application:**
    * **On macOS:** Open a terminal, navigate to the project root, and run:
        ```bash
        ./build/macos_build/Application/myApp
        ```
        *(This will use the Stub I2C Manager and publish dummy data)*
    * **On Raspberry Pi:**
        * Copy the cross-compiled executable (`build/rpi_build/Application/myApp`) from your Mac to the Raspberry Pi (e.g., using `scp`).
        * On the Raspberry Pi terminal, navigate to where you copied the file and run it:
            ```bash
            ./myApp
            ```
            *(Ensure the MQTT broker is accessible from the RPi - e.g., use the Mac's IP address instead of `localhost` in `main.cpp` if running the broker on the Mac, or run the broker on the RPi itself)*.

3.  **Run the HTML Dashboard:**
    * Save the HTML code (provided previously) as `dashboard.html`.
    * Ensure the `brokerUrl` inside the dashboard's JavaScript points to your broker's WebSocket listener (e.g., `ws://localhost:9001` if running the broker and browser on the same Mac).
    * Open the `dashboard.html` file in your web browser.

## Running Unit Tests (macOS Only)

Unit tests are configured only for the native macOS build.

1.  Run the test script from the project root:
    ```bash
    ./run_tests.sh
    ```
    This will configure, build (if needed), and run CTest, reporting results.

## Configuration

Currently, several parameters are hardcoded in `Application/src/main.cpp`:

* `I2C_BUS_PATH`: Path for the Linux I2C device.
* `BME280_ADDRESS`: I2C address of the sensor.
* `MQTT_BROKER_ADDRESS`: Address (including `tcp://` prefix) and port of the MQTT broker.
* `MQTT_CLIENT_ID`: Base client ID for MQTT connection.
* `MQTT_TOPIC_BME280`: Topic used for publishing data.
* `PUBLISH_INTERVAL`: Frequency of data publishing.

Future improvements could involve reading these from a configuration file or environment variables.
