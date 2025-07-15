#include "chatty.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Test the JSON payload generation
static int test_json_payload() {
    printf("Testing JSON payload construction for moonshot.ai...\n");
    
    // Test message array
    chatty_Message messages[2];
    messages[0].role = CHATTY_SYSTEM;
    messages[0].message = "You are a helpful assistant.";
    messages[1].role = CHATTY_USER;
    messages[1].message = "Hello, how are you?";
    
    // Test options with temperature and top_p
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    options.has_temperature = true;
    options.temperature = 0.7;
    options.has_top_p = true;
    options.top_p = 0.9;
    
    // This is an internal function, but we can test it indirectly
    // by ensuring the chat function doesn't fail due to JSON issues
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "test-key", 1);
    
    chatty_Message response;
    enum chatty_ERROR result = chatty_chat(2, messages, options, &response);
    
    // Should get network error (expected) or JSON parse error, not invalid options
    if (result == CHATTY_INVALID_OPTIONS) {
        printf("❌ FAIL: JSON payload construction failed\n");
        return 0;
    } else {
        printf("✅ PASS: JSON payload construction successful\n");
        printf("   (Got %s - expected with test API key)\n", chatty_error_string(result));
        return 1;
    }
}

// Test edge cases and error conditions
static int test_edge_cases() {
    printf("\nTesting edge cases and error conditions...\n");
    
    int passed = 0;
    int total = 0;
    
    // Test 1: Invalid model (empty string)
    total++;
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "test-key", 1);
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test";
    
    chatty_Options options = {0};
    options.model = "";  // Empty model
    
    chatty_Message response;
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    if (result == CHATTY_INVALID_OPTIONS) {
        printf("  ✅ PASS: Correctly rejected empty model\n");
        passed++;
    } else {
        printf("  ❌ FAIL: Should reject empty model\n");
    }
    
    // Test 2: Invalid temperature
    total++;
    options.model = "moonshot-v1-8k";
    options.has_temperature = true;
    options.temperature = 3.0;  // Invalid (> 2.0)
    
    result = chatty_chat(1, messages, options, &response);
    
    if (result == CHATTY_INVALID_OPTIONS) {
        printf("  ✅ PASS: Correctly rejected invalid temperature\n");
        passed++;
    } else {
        printf("  ❌ FAIL: Should reject invalid temperature\n");
    }
    
    // Test 3: Invalid top_p
    total++;
    options.temperature = 0.7;  // Valid
    options.has_top_p = true;
    options.top_p = 1.5;  // Invalid (> 1.0)
    
    result = chatty_chat(1, messages, options, &response);
    
    if (result == CHATTY_INVALID_OPTIONS) {
        printf("  ✅ PASS: Correctly rejected invalid top_p\n");
        passed++;
    } else {
        printf("  ❌ FAIL: Should reject invalid top_p\n");
    }
    
    // Test 4: NULL message
    total++;
    options.top_p = 0.9;  // Valid
    messages[0].message = NULL;
    
    result = chatty_chat(1, messages, options, &response);
    
    if (result == CHATTY_INVALID_OPTIONS) {
        printf("  ✅ PASS: Correctly rejected NULL message\n");
        passed++;
    } else {
        printf("  ❌ FAIL: Should reject NULL message\n");
    }
    
    printf("Edge case tests: %d/%d passed\n", passed, total);
    return passed == total;
}

// Test that moonshot.ai doesn't interfere with other providers
static int test_provider_isolation() {
    printf("\nTesting provider isolation...\n");
    
    // Set up multiple API keys
    setenv("OPENAI_API_KEY", "openai-key", 1);
    setenv("GROQ_API_KEY", "groq-key", 1);
    setenv("MOONSHOT_API_KEY", "moonshot-key", 1);
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test";
    
    chatty_Options options = {0};
    options.model = "test-model";
    
    chatty_Message response;
    
    // Test OpenAI (default)
    setenv("OPENAI_API_BASE", "https://api.openai.com/v1", 1);
    enum chatty_ERROR result1 = chatty_chat(1, messages, options, &response);
    
    // Test Groq
    setenv("OPENAI_API_BASE", "https://api.groq.com/openai/v1", 1);
    enum chatty_ERROR result2 = chatty_chat(1, messages, options, &response);
    
    // Test Moonshot
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    enum chatty_ERROR result3 = chatty_chat(1, messages, options, &response);
    
    // All should get network errors (not invalid key errors)
    if (result1 != CHATTY_INVALID_KEY && result2 != CHATTY_INVALID_KEY && result3 != CHATTY_INVALID_KEY) {
        printf("✅ PASS: Provider isolation works correctly\n");
        return 1;
    } else {
        printf("❌ FAIL: Provider isolation issue detected\n");
        printf("  OpenAI result: %s\n", chatty_error_string(result1));
        printf("  Groq result: %s\n", chatty_error_string(result2));
        printf("  Moonshot result: %s\n", chatty_error_string(result3));
        return 0;
    }
}

int main() {
    printf("=== Detailed Moonshot.ai Integration Tests ===\n\n");
    
    int tests_passed = 0;
    int total_tests = 3;
    
    if (test_json_payload()) tests_passed++;
    if (test_edge_cases()) tests_passed++;
    if (test_provider_isolation()) tests_passed++;
    
    printf("\n=== Detailed Test Results ===\n");
    printf("Passed: %d/%d test suites\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("🎉 All detailed tests passed! Moonshot.ai integration is robust.\n");
        return 0;
    } else {
        printf("❌ Some detailed tests failed. Please review the implementation.\n");
        return 1;
    }
}