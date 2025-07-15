# Project Structure

## Core Library Files
- `chatty.h` - Public API header with enums, structs, and function declarations
- `chatty.c` - Main library implementation with HTTP client and JSON handling
- `cJSON.h/cJSON.c` - Embedded JSON parser (third-party, minimal modifications)
- `main.c` - Simple CLI executable demonstrating library usage

## Build Configuration
- `CMakeLists.txt` - Primary build configuration
- `CMakePresets.json` - Build presets (default uses Ninja)
- `vcpkg.json` - Package dependencies (curl with HTTP/2 and OpenSSL)
- `vcpkg-configuration.json` - vcpkg configuration

## Benchmark Scripts
- `loop.sh` - Generic benchmark runner (10 iterations)
- `[provider].sh` - Provider-specific benchmark scripts (groq.sh, fireworks.sh, etc.)
- `rice*.sh` - Additional benchmark variations

## Comparison Projects
- `langchain/` - Python LangChain benchmark comparison
- `llamaindex/` - Python LlamaIndex benchmark comparison  
- `lits/` - TypeScript LlamaIndex benchmark comparison

## Organization Principles
- **Flat structure**: Core files at root level for simplicity
- **Minimal dependencies**: Only essential external deps (libcurl)
- **Self-contained**: JSON parser embedded to avoid extra dependencies
- **Benchmark-focused**: Multiple scripts for performance validation
- **Provider agnostic**: Single codebase supports multiple LLM providers

## Key Conventions
- Library builds as both static lib (`libchatty`) and CLI executable (`chatty`)
- Shell scripts use zsh shebang (`#!/bin/zsh`)
- All benchmark scripts run 10 iterations by default
- Provider selection via `OPENAI_API_BASE` environment variable