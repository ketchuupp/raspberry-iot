#!/usr/bin/env bash

# Script to build and run Unit Tests for the Native macOS target
# Place this script in the project root directory.

# Exit immediately if a command exits with a non-zero status.
set -e

echo "*** Building and Running Unit Tests for Native macOS Target ***"

# --- Configuration ---
PROJECT_ROOT="." # Project root is the current directory
MACOS_BUILD_DIR="$PROJECT_ROOT/build/macos_build" # Use the same build dir as build_macos.sh

# --- Main Logic ---

# 1. Configure CMake for native macOS build (ensures tests are enabled)
echo "Configuring CMake for native macOS (ensuring tests are enabled)..."
cmake -B "$MACOS_BUILD_DIR" \
      -S "$PROJECT_ROOT" \
      -DCMAKE_BUILD_TYPE=Debug \
      -G Ninja # Use Ninja generator (ensure ninja is installed: brew install ninja)

# 2. Build the project (specifically targets tests if possible, otherwise all)
echo "Building tests..."
# Building 'all' ensures test executables are up-to-date
cmake --build "$MACOS_BUILD_DIR"
# Alternatively, to build only test targets (might be faster if code didn't change):
# cmake --build "$MACOS_BUILD_DIR" --target test # Might need specific test target names

# 3. Run tests using CTest
echo "Running tests via CTest..."
# Run ctest from within the build directory
cd "$MACOS_BUILD_DIR"
ctest --verbose # Use --verbose for detailed output
TEST_EXIT_CODE=$? # Capture CTest exit code
cd "$PROJECT_ROOT" # Go back to project root

# 4. Report results
if [ $TEST_EXIT_CODE -eq 0 ]; then
    echo "*** All tests passed! ***"
else
    echo "*** Some tests FAILED! (Exit code: $TEST_EXIT_CODE) ***"
fi

exit $TEST_EXIT_CODE # Exit with the CTest result