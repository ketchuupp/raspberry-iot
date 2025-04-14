#!/usr/bin/env bash

# Script to build the Raspberry Pi target using Docker.
# Automatically builds the Docker image if it doesn't exist.
# Place this script in the project root directory.

# Exit immediately if a command exits with a non-zero status.
set -e

echo "*** Building for Raspberry Pi Target using Docker ***"
echo "*** Script execution time: $(date) ***" # Added timestamp for context

# --- Configuration ---
PROJECT_ROOT="." # Project root is the current directory
RPI_BUILD_DIR="$PROJECT_ROOT/build/rpi_build"
TOOLCHAIN_FILE_REL_PATH="cmake/toolchains/rpi_docker_toolchain.cmake" # Relative to project root
DOCKER_IMAGE_NAME="myapp-rpi-builder" # Match the name used in 'docker build'

# --- Main Logic ---

# 1. Check if Docker image exists, build if not.
echo "Checking for Docker image: $DOCKER_IMAGE_NAME..."
# Use low-level inspect command with format to check existence quietly
if ! docker image inspect "$DOCKER_IMAGE_NAME" > /dev/null 2>&1; then
    echo "Docker image '$DOCKER_IMAGE_NAME' not found. Building it now..."
    echo "This might take several minutes, especially the first time..."
    # Build the image using the Dockerfile in the project root
    docker build -t "$DOCKER_IMAGE_NAME" "$PROJECT_ROOT"
    echo "Docker image '$DOCKER_IMAGE_NAME' built successfully."
else
    echo "Docker image '$DOCKER_IMAGE_NAME' found."
fi

# 2. Ensure host build directory exists (Docker will map into it)
echo "Ensuring build directory exists: $RPI_BUILD_DIR"
mkdir -p "$RPI_BUILD_DIR"

# 3. Run the build process inside Docker
echo "Running CMake configuration and build inside container..."
# Use $(pwd) for host path to ensure absolute path mounting
docker run --rm \
       -v "$(pwd):/work" \
       "$DOCKER_IMAGE_NAME" \
       bash -c "\
           set -e && \
           echo '*** Configuring CMake inside container...' && \
           # Run cmake configure step
           cmake \
               -B /work/build/rpi_build \
               -S /work \
               -DCMAKE_TOOLCHAIN_FILE=/work/${TOOLCHAIN_FILE_REL_PATH} \
               -DCMAKE_BUILD_TYPE=Debug \
               -G Ninja && \
           echo '*** Building project inside container...' && \
           # Run build step
           cmake --build /work/build/rpi_build \
       "

echo "*** Raspberry Pi build process finished successfully! ***"
echo "Build artifacts are in: $RPI_BUILD_DIR"
echo "(Executable needs copying to RPi for execution)"