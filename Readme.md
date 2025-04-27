# Raspberry Pi Sensor Hub (MQTT Publisher)

## Overview

This project implements a C++ application designed to run on a Raspberry Pi. It reads data from configured sensors (e.g., BME280 via I2C) based on a `config.json` file and publishes this data to an MQTT broker.

The project utilizes modern C++ (C++23), CMake for building, and supports cross-compilation for Raspberry Pi (ARM Linux) from a macOS/Linux host using Docker. Dependencies like Paho MQTT and nlohmann/json are managed via CMake's FetchContent. The main application logic is encapsulated within the `App` class.

An accompanying HTML/JavaScript dashboard (`dashboard.html`) can subscribe to the MQTT broker to display the sensor data in real-time.

## Features

* Reads data from multiple sensor types based on `config.json`.
* Currently supports:
    * BME280 (Temperature, Humidity, Pressure) via I2C.
    * LPS25HB (Temperature, Pressure) via I2C.
    * Dummy (Generates example data, useful as a template/test).
* Publishes data to configurable MQTT topics in JSON format.
* Abstracted sensor interface (`ISensor`).
* Sensor instantiation handled by `SensorBuilder`.
* Abstracted I2C interface (`II2C_Bus`) with implementation for Linux (`ioctl`).
* Cross-compilation support for Raspberry Pi (arm-linux-gnueabihf) using Docker.
* Dependencies managed via CMake FetchContent.
* Simple build script (`build.sh`) for the target application.

## Prerequisites

* **Development Host (e.g., macOS, Linux):**
    * CMake (version 3.14 or higher)
    * Git
    * Docker Desktop (or Docker Engine on Linux) installed and running.
    * Ninja build system (optional, but used by build script): `brew install ninja` on macOS, `sudo apt install ninja-build` on Debian/Ubuntu.
* **For Running the Application:**
    * An MQTT Broker (like Mosquitto) accessible by the application.
        * Must be configured to listen on TCP (default 1883).
        * Install on RPi: `sudo apt install mosquitto mosquitto-clients`
        * Install on macOS: `brew install mosquitto`
    * (Optional) HTML Dashboard requires broker configured for WebSockets.
* **Hardware (for RPi target):**
    * Raspberry Pi running Raspberry Pi OS (or compatible Linux).
    * Configured sensors connected to the appropriate pins (e.g., BME280 via I2C).

## Setup on Raspberry Pi (Hardware Interfaces)

Before running the application on the Raspberry Pi, you **must** enable the necessary hardware interfaces for the sensors you configure in `config.json`:

1.  **Enable I2C (for BME280):**
    * Run `sudo raspi-config`.
    * Navigate to `Interface Options` -> `I2C`.
    * Select `<Yes>` to enable.
    * Finish and **Reboot**.
    * Verify your user is in the `i2c` group (`groups $(whoami)`). If not, add with `sudo usermod -aG i2c $USER` and log out/in.


## Building the Application (for RPi via Docker)

Use the provided script from the project root directory on your development machine (e.g., macOS):

1.  **Build the Docker image** (only needs to be done once or if `Dockerfile` changes):
    ```bash
    docker build -t myapp-rpi-builder .
    ```
2.  **Run the build script:**
    ```bash
    ./build.sh
    ```
3.  **Output:** The ARM Linux executable will be located in `build/Application/Sensor_tester`.

## Running on Raspberry Pi

1.  **Copy Files:**
    * Copy the compiled executable (`build/Application/Sensor_tester`) from your development machine to the Raspberry Pi (e.g., using `scp`).
    * Copy the `config.json` file to the same directory on the Raspberry Pi where you place the executable. Ensure `config.json` contains the desired sensor configurations.
2.  **Ensure Permissions:** On the RPi, make the executable runnable: `chmod +x ./Sensor_tester`
3.  **Run:** Execute the application: `./Sensor_tester` (or `./Sensor_tester path/to/config.json` if it's elsewhere).
    *(Ensure the MQTT broker is running and accessible from the RPi. Update the broker address in `config.json` if necessary).*

## Configuration (`config.json`)

The application loads settings from `config.json`. See the example file for structure. Key fields:

* `mqtt`: Contains `broker_address` (e.g., "tcp://192.168.1.10:1883"), `client_id_base`, `topic_base`.
* `global_publish_interval_sec`: Optional integer interval (default 10s) used if sensor-specific interval isn't set.
* `sensors`: An array of sensor objects. Each object needs:
    * `type`: String identifier (e.g., "BME280", "Dummy"). Must match the type handled in `SensorBuilder`.
    * `enabled`: `true` or `false`.
    * `publish_topic_suffix`: String appended to `mqtt.topic_base`.
    * `publish_interval_sec`: Optional integer interval for this specific sensor.
    * Type-specific fields (e.g., `i2c_bus`, `i2c_address` for BME280).



