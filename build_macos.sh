#!/usr/bin/env bash

# Script to build the Native macOS target
# Place this script in the project root directory.

# Exit immediately if a command exits with a non-zero status.
set -e

echo "*** Building for Native macOS Target ***"

# --- Configuration ---
PROJECT_ROOT="." # Project root is the current directory
MACOS_BUILD_DIR="$PROJECT_ROOT/build/macos_build"

# --- Main Logic ---

# 1. Create build directories
echo "Ensuring build directory exists: $MACOS_BUILD_DIR"
mkdir -p "$MACOS_BUILD_DIR"

# 2. Configure CMake for native macOS build
#    No toolchain file needed - BUILD_TARGET_RPI will be OFF/default
#    Tests will be enabled automatically by CMakeLists.txt logic
echo "Configuring CMake for native macOS (Debug build)..."
cmake -B "$MACOS_BUILD_DIR" \
      -S "$PROJECT_ROOT" \
      -DCMAKE_BUILD_TYPE=Debug \
      -G Ninja # Use Ninja generator (ensure ninja is installed: brew install ninja)

# 3. Build the project (including tests)
echo "Building project for native macOS..."
cmake --build "$MACOS_BUILD_DIR"

echo "*** Native macOS build process finished successfully! ***"
echo "Build artifacts are in: $MACOS_BUILD_DIR"
# Example: Executable might be at $MACOS_BUILD_DIR/Application/myApp
# Example: Tests might be at $MACOS_BUILD_DIR/Components/<Component>/Test-<Component>