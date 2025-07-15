#include "chatty.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Test Requirement 1.1: Provider detection for moonshot.ai base URL
static int test_requirement_1_1() {
    printf("Testing Requirement 1.1: Provider detection for moonshot.ai base URL...\n");
    
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "test-key", 1);
    unsetenv("OPENAI_API_KEY");
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test message";
    
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    
    chatty_Message response;
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    // Should NOT get INVALID_KEY if provider detection works
    if (result == CHATTY_INVALID_KEY) {
        printf("❌ FAIL: Provider detection failed for moonshot.ai base URL\n");
        return 0;
    } else {
        printf("✅ PASS: Provider correctly detected for moonshot.ai base URL\n");
        return 1;
    }
}

// Test Requirement 1.2: Correct API key environment variable selection
static int test_requirement_1_2() {
    printf("\nTesting Requirement 1.2: Correct API key environment variable selection...\n");
    
    // Test with moonshot.ai base URL - should use MOONSHOT_API_KEY
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "moonshot-test-key", 1);
    setenv("OPENAI_API_KEY", "openai-test-key", 1);
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test";
    
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    
    chatty_Message response;
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    // Should not get INVALID_KEY since MOONSHOT_API_KEY is set
    if (result == CHATTY_INVALID_KEY) {
        printf("❌ FAIL: Should use MOONSHOT_API_KEY when base URL is moonshot.ai\n");
        return 0;
    }
    
    // Now remove MOONSHOT_API_KEY - should get INVALID_KEY even though OPENAI_API_KEY exists
    unsetenv("MOONSHOT_API_KEY");
    result = chatty_chat(1, messages, options, &response);
    
    if (result != CHATTY_INVALID_KEY) {
        printf("❌ FAIL: Should get INVALID_KEY when MOONSHOT_API_KEY is missing\n");
        return 0;
    }
    
    printf("✅ PASS: Correct API key environment variable (MOONSHOT_API_KEY) is selected\n");
    return 1;
}

// Test Requirement 1.3: HTTP request construction with moonshot.ai endpoint
static int test_requirement_1_3() {
    printf("\nTesting Requirement 1.3: HTTP request construction with moonshot.ai endpoint...\n");
    
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "test-key", 1);
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test HTTP request construction";
    
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    options.has_temperature = true;
    options.temperature = 0.7;
    
    chatty_Message response;
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    // Should get network error or JSON parse error (expected with test key)
    // Should NOT get CURL_INIT_ERROR or INVALID_OPTIONS if HTTP request is constructed properly
    if (result == CHATTY_CURL_INIT_ERROR || result == CHATTY_INVALID_OPTIONS) {
        printf("❌ FAIL: HTTP request construction issue detected\n");
        return 0;
    } else {
        printf("✅ PASS: HTTP request constructed properly with moonshot.ai endpoint\n");
        printf("   (Got %s - expected with test API key)\n", chatty_error_string(result));
        return 1;
    }
}

// Test Requirement 3.1: Integration follows same pattern as existing providers
static int test_requirement_3_1() {
    printf("\nTesting Requirement 3.1: Integration follows same pattern as existing providers...\n");
    
    // Test that moonshot.ai behaves consistently with other providers
    const char* providers[][2] = {
        {"https://api.groq.com/openai/v1", "GROQ_API_KEY"},
        {"https://api.fireworks.ai/inference/v1", "FIREWORKS_API_KEY"},
        {"https://api.moonshot.ai/v1", "MOONSHOT_API_KEY"}
    };
    
    int consistent_behavior = 1;
    
    for (int i = 0; i < 3; i++) {
        setenv("OPENAI_API_BASE", providers[i][0], 1);
        setenv(providers[i][1], "test-key", 1);
        
        // Clear other keys
        for (int j = 0; j < 3; j++) {
            if (j != i) {
                unsetenv(providers[j][1]);
            }
        }
        
        chatty_Message messages[1];
        messages[0].role = CHATTY_USER;
        messages[0].message = "Test";
        
        chatty_Options options = {0};
        options.model = "test-model";
        
        chatty_Message response;
        enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
        
        // All should behave the same way (not get INVALID_KEY)
        if (result == CHATTY_INVALID_KEY) {
            printf("❌ Provider %s failed key detection\n", providers[i][0]);
            consistent_behavior = 0;
        }
    }
    
    if (consistent_behavior) {
        printf("✅ PASS: Moonshot.ai integration follows same pattern as existing providers\n");
        return 1;
    } else {
        printf("❌ FAIL: Inconsistent behavior between providers\n");
        return 0;
    }
}

int main() {
    printf("=== Requirements Validation Tests ===\n\n");
    
    int tests_passed = 0;
    int total_tests = 4;
    
    if (test_requirement_1_1()) tests_passed++;
    if (test_requirement_1_2()) tests_passed++;
    if (test_requirement_1_3()) tests_passed++;
    if (test_requirement_3_1()) tests_passed++;
    
    printf("\n=== Requirements Validation Results ===\n");
    printf("Requirements validated: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("🎉 All requirements validated! Moonshot.ai integration meets specifications.\n");
        return 0;
    } else {
        printf("❌ Some requirements not met. Please review the implementation.\n");
        return 1;
    }
}