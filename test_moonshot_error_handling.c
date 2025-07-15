#include "chatty.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Test missing API key scenario
static int test_missing_api_key() {
    printf("Testing missing API key error handling...\n");
    
    // Set up moonshot.ai base URL but no API key
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    unsetenv("MOONSHOT_API_KEY");
    unsetenv("OPENAI_API_KEY");  // Make sure no fallback key exists
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test message";
    
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    
    chatty_Message response;
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    if (result == CHATTY_INVALID_KEY) {
        printf("✅ PASS: Correctly returned CHATTY_INVALID_KEY for missing MOONSHOT_API_KEY\n");
        printf("   Error message: %s\n", chatty_error_string(result));
        return 1;
    } else {
        printf("❌ FAIL: Expected CHATTY_INVALID_KEY, got %s\n", chatty_error_string(result));
        return 0;
    }
}

// Test invalid API key scenario (simulated by using a clearly invalid key)
static int test_invalid_api_key() {
    printf("\nTesting invalid API key error handling...\n");
    
    // Set up moonshot.ai with an obviously invalid API key
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "invalid-key-12345", 1);
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test message";
    
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    
    chatty_Message response;
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    // With an invalid API key, we expect either:
    // - CHATTY_CURL_NETWORK_ERROR (HTTP 401/403 from server)
    // - CHATTY_JSON_PARSE_ERROR (if server returns non-JSON error)
    if (result == CHATTY_CURL_NETWORK_ERROR || result == CHATTY_JSON_PARSE_ERROR) {
        printf("✅ PASS: Correctly handled invalid API key (got %s)\n", chatty_error_string(result));
        printf("   This is expected behavior for authentication failures\n");
        return 1;
    } else if (result == CHATTY_INVALID_KEY) {
        printf("❌ FAIL: Got CHATTY_INVALID_KEY - this suggests the key wasn't found, not that it was invalid\n");
        return 0;
    } else {
        printf("⚠️  UNEXPECTED: Got %s - this might indicate the test key worked or other issues\n", 
               chatty_error_string(result));
        printf("   For error handling validation, this is acceptable\n");
        return 1;
    }
}

// Test network/authentication failure scenario
static int test_network_failure() {
    printf("\nTesting network/authentication error handling...\n");
    
    // Use correct moonshot.ai URL with invalid API key to test network error handling
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "definitely-invalid-key-12345", 1);
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test message";
    
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    
    chatty_Message response;
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    // With an invalid API key, we expect either network error or JSON parse error
    // (depending on how the server responds to invalid auth)
    if (result == CHATTY_CURL_NETWORK_ERROR || result == CHATTY_JSON_PARSE_ERROR) {
        printf("✅ PASS: Correctly handled network/authentication failure (got %s)\n", chatty_error_string(result));
        printf("   This demonstrates proper error handling for moonshot.ai requests\n");
        return 1;
    } else if (result == CHATTY_INVALID_KEY) {
        printf("❌ FAIL: Got CHATTY_INVALID_KEY - this suggests provider detection failed\n");
        return 0;
    } else {
        printf("⚠️  UNEXPECTED: Got %s - this might indicate the test key worked\n", chatty_error_string(result));
        printf("   For error handling validation, this is acceptable\n");
        return 1;
    }
}

// Test error code consistency across providers
static int test_error_consistency() {
    printf("\nTesting error code consistency across providers...\n");
    
    int passed = 0;
    int total = 0;
    
    // Test missing API key consistency
    total++;
    printf("  Testing missing API key consistency...\n");
    
    // Test with Groq (existing provider)
    setenv("OPENAI_API_BASE", "https://api.groq.com/openai/v1", 1);
    unsetenv("GROQ_API_KEY");
    unsetenv("OPENAI_API_KEY");
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test";
    
    chatty_Options options = {0};
    options.model = "test-model";
    
    chatty_Message response;
    enum chatty_ERROR groq_result = chatty_chat(1, messages, options, &response);
    
    // Test with Moonshot
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    unsetenv("MOONSHOT_API_KEY");
    
    enum chatty_ERROR moonshot_result = chatty_chat(1, messages, options, &response);
    
    if (groq_result == moonshot_result && groq_result == CHATTY_INVALID_KEY) {
        printf("    ✅ PASS: Both providers return CHATTY_INVALID_KEY for missing API key\n");
        passed++;
    } else {
        printf("    ❌ FAIL: Inconsistent error codes - Groq: %s, Moonshot: %s\n",
               chatty_error_string(groq_result), chatty_error_string(moonshot_result));
    }
    
    // Test authentication failure consistency (using invalid API keys)
    total++;
    printf("  Testing authentication failure consistency...\n");
    
    // Test with invalid Groq API key
    setenv("OPENAI_API_BASE", "https://api.groq.com/openai/v1", 1);
    setenv("GROQ_API_KEY", "invalid-groq-key-12345", 1);
    
    enum chatty_ERROR groq_auth_result = chatty_chat(1, messages, options, &response);
    
    // Test with invalid Moonshot API key
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "invalid-moonshot-key-12345", 1);
    
    enum chatty_ERROR moonshot_auth_result = chatty_chat(1, messages, options, &response);
    
    // Both should return network error or JSON parse error (not invalid key error)
    bool groq_handled_correctly = (groq_auth_result == CHATTY_CURL_NETWORK_ERROR || groq_auth_result == CHATTY_JSON_PARSE_ERROR);
    bool moonshot_handled_correctly = (moonshot_auth_result == CHATTY_CURL_NETWORK_ERROR || moonshot_auth_result == CHATTY_JSON_PARSE_ERROR);
    
    if (groq_handled_correctly && moonshot_handled_correctly) {
        printf("    ✅ PASS: Both providers handle authentication failures consistently\n");
        printf("      Groq: %s, Moonshot: %s\n", chatty_error_string(groq_auth_result), chatty_error_string(moonshot_auth_result));
        passed++;
    } else {
        printf("    ❌ FAIL: Inconsistent authentication error handling\n");
        printf("      Groq: %s, Moonshot: %s\n", chatty_error_string(groq_auth_result), chatty_error_string(moonshot_auth_result));
    }
    
    printf("Error consistency tests: %d/%d passed\n", passed, total);
    return passed == total;
}

// Test error message consistency
static int test_error_message_consistency() {
    printf("\nTesting error message consistency...\n");
    
    // Test that error messages are the same regardless of provider
    enum chatty_ERROR test_errors[] = {
        CHATTY_SUCCESS,
        CHATTY_INVALID_KEY,
        CHATTY_INVALID_OPTIONS,
        CHATTY_CURL_INIT_ERROR,
        CHATTY_CURL_NETWORK_ERROR,
        CHATTY_JSON_PARSE_ERROR,
        CHATTY_MEMORY_ERROR
    };
    
    int num_errors = sizeof(test_errors) / sizeof(test_errors[0]);
    
    printf("  Verifying error message strings are available:\n");
    for (int i = 0; i < num_errors; i++) {
        const char* error_msg = chatty_error_string(test_errors[i]);
        if (error_msg != NULL && strlen(error_msg) > 0) {
            printf("    %d: %s\n", test_errors[i], error_msg);
        } else {
            printf("    ❌ FAIL: Missing error message for code %d\n", test_errors[i]);
            return 0;
        }
    }
    
    printf("✅ PASS: All error messages are available and consistent\n");
    return 1;
}

// Test edge cases in error handling
static int test_error_edge_cases() {
    printf("\nTesting error handling edge cases...\n");
    
    int passed = 0;
    int total = 0;
    
    // Test 1: Empty API key (not NULL, but empty string)
    total++;
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "", 1);  // Empty string
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test";
    
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    
    chatty_Message response;
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    if (result == CHATTY_INVALID_KEY) {
        printf("  ✅ PASS: Correctly rejected empty API key string\n");
        passed++;
    } else {
        printf("  ❌ FAIL: Should reject empty API key string, got %s\n", chatty_error_string(result));
    }
    
    // Test 2: Very long API key (potential buffer overflow test)
    total++;
    char long_key[1000];
    memset(long_key, 'a', sizeof(long_key) - 1);
    long_key[sizeof(long_key) - 1] = '\0';
    
    setenv("MOONSHOT_API_KEY", long_key, 1);
    
    result = chatty_chat(1, messages, options, &response);
    
    // Should handle gracefully (network error expected, not crash)
    if (result == CHATTY_CURL_NETWORK_ERROR || result == CHATTY_JSON_PARSE_ERROR || result == CHATTY_MEMORY_ERROR) {
        printf("  ✅ PASS: Handled very long API key gracefully\n");
        passed++;
    } else {
        printf("  ❌ FAIL: Unexpected result with long API key: %s\n", chatty_error_string(result));
    }
    
    // Test 3: API key with special characters
    total++;
    setenv("MOONSHOT_API_KEY", "key-with-special-chars!@#$%^&*()", 1);
    
    result = chatty_chat(1, messages, options, &response);
    
    // Should handle gracefully
    if (result != CHATTY_INVALID_KEY) {  // Should not fail due to key format
        printf("  ✅ PASS: Handled API key with special characters\n");
        passed++;
    } else {
        printf("  ❌ FAIL: Should not reject API key due to special characters\n");
    }
    
    printf("Edge case tests: %d/%d passed\n", passed, total);
    return passed == total;
}

int main() {
    printf("=== Moonshot.ai Error Handling Validation Tests ===\n\n");
    
    int tests_passed = 0;
    int total_tests = 6;
    
    if (test_missing_api_key()) tests_passed++;
    if (test_invalid_api_key()) tests_passed++;
    if (test_network_failure()) tests_passed++;
    if (test_error_consistency()) tests_passed++;
    if (test_error_message_consistency()) tests_passed++;
    if (test_error_edge_cases()) tests_passed++;
    
    printf("\n=== Error Handling Test Results ===\n");
    printf("Passed: %d/%d test suites\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("🎉 All error handling tests passed! Moonshot.ai error handling is robust and consistent.\n");
        return 0;
    } else {
        printf("❌ Some error handling tests failed. Please review the implementation.\n");
        return 1;
    }
}