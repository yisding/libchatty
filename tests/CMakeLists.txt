# CMocka Test Framework
find_package(cmocka CONFIG REQUIRED)

# Create test executable
add_executable(chatty_tests
    test_main.c
    test_chatty.c
    test_memory_failures.c
    ../chatty.c
    ../cJSON.c
)

# Link libraries
target_link_libraries(chatty_tests 
    PRIVATE 
    cmocka::cmocka
    ${CMAKE_DL_LIBS}
)

# Include directories
target_include_directories(chatty_tests PRIVATE 
    ..
)

# Find and link curl
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
target_link_libraries(chatty_tests PRIVATE ${CURL_LIBRARIES})
target_include_directories(chatty_tests PRIVATE ${CURL_INCLUDE_DIRS})

# Add compile definitions for testing
target_compile_definitions(chatty_tests PRIVATE 
    CHATTY_TESTING=1
    CURL_NO_OLDIES=1
)

# Register tests with CTest
add_test(NAME chatty_unit_tests COMMAND chatty_tests)
add_test(NAME chatty_memory_tests COMMAND chatty_tests --memory-failures)

# Set test properties
set_tests_properties(chatty_unit_tests PROPERTIES
    TIMEOUT 30
)
set_tests_properties(chatty_memory_tests PROPERTIES
    TIMEOUT 30
)
