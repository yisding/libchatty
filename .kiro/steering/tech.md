# Technology Stack

## Build System
- **CMake**: Primary build system with CMakeLists.txt
- **vcpkg**: Package manager for C/C++ dependencies
- **Ninja**: Preferred generator (via CMakePresets.json)
- **C99 Standard**: Strict adherence to C99 for maximum compatibility

## Dependencies
- **libcurl**: HTTP client library with HTTP/2 and OpenSSL support
- **cJSON**: Embedded JSON parsing (included in source)

## Common Commands

### Building
```bash
# Configure and build (requires vcpkg)
cmake --preset default
cmake --build build

# Alternative manual configuration
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake ..
make
```

### Running
```bash
# Basic usage
./build/chatty [model] [message]

# With different providers (via environment variables)
OPENAI_API_BASE="https://api.groq.com/openai/v1" ./build/chatty llama-3.1-70b-versatile
OPENAI_API_BASE="https://api.fireworks.ai/inference/v1" ./build/chatty llama-v3p1-405b-instruct

# Benchmarking (10 iterations)
./loop.sh [model]
```

## Compiler Settings
- Wall, Wextra, Wpedantic warnings enabled
- Position Independent Code (PIC) for library
- User Agent: "libchatty/1.0"

## Environment Variables
API keys are automatically detected based on base URL:
- `OPENAI_API_KEY` (default)
- `GROQ_API_KEY`, `FIREWORKS_API_KEY`, `MISTRAL_API_KEY`
- `HYPERBOLIC_API_KEY`, `DEEPSEEK_API_KEY`, `LLAMA_API_KEY`