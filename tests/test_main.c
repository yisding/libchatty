#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

// Test function declarations
void test_chatty_input_validation(void **state);
void test_chatty_json_generation(void **state);
void test_chatty_role_conversion(void **state);
void test_chatty_error_strings(void **state);
void test_chatty_memory_failures(void **state);

int main(int argc, char* argv[]) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_chatty_input_validation),
        cmocka_unit_test(test_chatty_json_generation),
        cmocka_unit_test(test_chatty_role_conversion),
        cmocka_unit_test(test_chatty_error_strings),
    };
    
    const struct CMUnitTest memory_tests[] = {
        cmocka_unit_test(test_chatty_memory_failures),
    };
    
    if (argc > 1 && strcmp(argv[1], "--memory-failures") == 0) {
        // Run only memory failure tests
        return cmocka_run_group_tests(memory_tests, NULL, NULL);
    } else {
        // Run all tests
        int result1 = cmocka_run_group_tests(tests, NULL, NULL);
        int result2 = cmocka_run_group_tests(memory_tests, NULL, NULL);
        return result1 + result2;
    }
}
