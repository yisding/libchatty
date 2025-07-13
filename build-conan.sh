#!/bin/bash

# Build with Conan 2.0
set -e

echo "Building libchatty with Conan 2.0..."

# Check if Conan is installed
if ! command -v conan &> /dev/null; then
    echo "Error: Conan is not installed. Please install Conan 2.0 first:"
    echo "pip install conan"
    exit 1
fi

# Detect default profile if not exists
if [ ! -f ~/.conan2/profiles/default ]; then
    echo "Creating default Conan profile..."
    conan profile detect --force
fi

# Create build directory
mkdir -p build-conan
cd build-conan

# Install dependencies
echo "Installing dependencies with Conan..."
conan install .. --output-folder=. --build=missing

# Configure and build
echo "Configuring with CMake..."
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

echo "Building..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build complete! Executable: build-conan/chatty"
