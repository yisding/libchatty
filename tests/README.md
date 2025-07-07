# libchatty Testing

This directory contains unit tests for the libchatty library using the CMocka testing framework.

## Test Structure

- `test_main.c` - Main test runner
- `test_chatty.c` - Core functionality tests (input validation, JSON generation, role conversion, error strings)
- `test_memory_failures.c` - Memory allocation failure testing

## Building and Running Tests

### Prerequisites

- CMake 3.10+
- vcpkg (for dependency management)
- A C99-compatible compiler

### Build

```bash
# From the project root
cmake --build build
```

### Run Tests

```bash
# Run all tests
ctest --test-dir build

# Run with verbose output
ctest --test-dir build --verbose

# Run specific test
ctest --test-dir build -R unit
ctest --test-dir build -R memory

# Run tests directly
./build/tests/chatty_tests                    # All tests
./build/tests/chatty_tests --memory-failures  # Memory failure tests only
```

## Test Categories

### Unit Tests
- **Input Validation**: Tests parameter validation and error handling
- **JSON Generation**: Tests message serialization to JSON format
- **Role Conversion**: Tests enum ↔ JSON string conversion
- **Error Strings**: Tests error code to string mapping

### Memory Failure Tests
- Tests graceful handling when memory allocation fails
- Verifies proper cleanup in error conditions
- Currently basic implementation - for comprehensive testing, consider:
  - LD_PRELOAD with custom malloc
  - Valgrind with failure injection
  - More sophisticated mocking frameworks

## Adding New Tests

1. Add test function to appropriate `.c` file
2. Add function declaration to `test_main.c`
3. Add `RUN_TEST()` call in main function
4. Rebuild and run

## Test Framework

Uses CMocka for C unit testing:
- `assert_non_null()` - Verify pointer is not NULL
- `assert_null()` - Verify pointer is NULL
- `assert_int_equal()` - Compare integers
- `assert_string_equal()` - Compare strings
- `print_message()` - Output test messages

## Notes

- Tests expect no API keys to be set (will get `CHATTY_INVALID_KEY`)
- Memory failure testing is basic - real allocation failure testing requires more sophisticated tools
- All tests should pass in a clean environment
