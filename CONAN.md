# Building with Conan 2.0

This project now supports both vcpkg and Conan 2.0 as package managers.

## Prerequisites

- CMake 3.10 or higher
- C99-compatible compiler
- Either Conan 2.0 or vcpkg installed

## Quick Start with Conan

```bash
# Use the provided build script
./build-conan.sh
```

## Manual Conan Build

```bash
# Install Conan 2.0 if you haven't already
pip install conan

# Install dependencies
mkdir build-conan && cd build-conan
conan install .. --output-folder=. --build=missing

# Configure and build
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake
make -j$(nproc)

# The executable will be at: build-conan/chatty
```

## Package Management Detection

The CMake configuration automatically detects which package manager to use:

1. **Conan**: If `conan_toolchain.cmake` is found in the build directory
2. **vcpkg**: Falls back to vcpkg if no Conan toolchain is detected
3. **Manual**: If neither is available, tries to find system libcurl

You can also force Conan usage with:
```bash
cmake .. -DUSE_CONAN=ON
```

## Conan Recipe

The project includes a `conanfile.py` that can be used to build libchatty as a Conan package:

```bash
conan create . --build=missing
```

## Dependencies

- **libcurl**: HTTP client library with HTTP/2 and SSL/TLS support
- **cJSON**: JSON parsing (included in source)

The Conan build uses libcurl 8.10.1 with the following features enabled:
- zlib compression
- Secure Transport (macOS)
- HTTP/2 support via built-in implementation
- All standard protocols (HTTP, HTTPS, FTP, etc.)
