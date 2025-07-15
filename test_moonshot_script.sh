#!/bin/zsh

echo "=== Running Comprehensive Moonshot.ai Test Suite ==="
echo

# Compile all test programs
echo "Compiling test programs..."
gcc -Wall -Wextra -o test_moonshot test_moonshot.c chatty.c cJSON.c -lcurl
gcc -Wall -Wextra -o test_moonshot_detailed test_moonshot_detailed.c chatty.c cJSON.c -lcurl
gcc -Wall -Wextra -o test_moonshot_error_handling test_moonshot_error_handling.c chatty.c cJSON.c -lcurl
gcc -Wall -Wextra -o test_requirements_validation test_requirements_validation.c chatty.c cJSON.c -lcurl

echo "Compilation complete."
echo

# Run all tests
echo "1. Running basic integration tests..."
./test_moonshot
echo

echo "2. Running detailed integration tests..."
./test_moonshot_detailed
echo

echo "3. Running error handling validation tests..."
./test_moonshot_error_handling
echo

echo "4. Running requirements validation tests..."
./test_requirements_validation
echo

echo "=== All Moonshot.ai Tests Complete ==="