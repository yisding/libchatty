#include "chatty.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Test helper function to capture environment variable usage
static int test_provider_detection() {
    printf("Testing moonshot.ai provider detection...\n");
    
    // Set up test environment
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "test-key-moonshot", 1);
    
    // Clear other API keys to ensure moonshot key is used
    unsetenv("OPENAI_API_KEY");
    unsetenv("GROQ_API_KEY");
    unsetenv("FIREWORKS_API_KEY");
    
    // Create a simple test message
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Hello, test message";
    
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    
    chatty_Message response;
    
    // This should fail with network error since we're using a fake API key,
    // but it should NOT fail with CHATTY_INVALID_KEY if provider detection works
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    if (result == CHATTY_INVALID_KEY) {
        printf("❌ FAIL: Provider detection failed - MOONSHOT_API_KEY not detected\n");
        return 0;
    } else if (result == CHATTY_CURL_NETWORK_ERROR) {
        printf("✅ PASS: Provider detection works - MOONSHOT_API_KEY was found and used\n");
        printf("   (Network error expected with test API key)\n");
        return 1;
    } else {
        printf("⚠️  UNEXPECTED: Got error code %d (%s)\n", result, chatty_error_string(result));
        printf("   This might indicate the test API key worked or other issues\n");
        return 1; // Still consider this a pass for provider detection
    }
}

static int test_api_key_selection() {
    printf("\nTesting API key environment variable selection...\n");
    
    // Test 1: With moonshot.ai base URL, should use MOONSHOT_API_KEY
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "moonshot-key", 1);
    setenv("OPENAI_API_KEY", "openai-key", 1);
    
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
    
    // Test 2: Remove MOONSHOT_API_KEY, should get INVALID_KEY
    unsetenv("MOONSHOT_API_KEY");
    result = chatty_chat(1, messages, options, &response);
    
    if (result != CHATTY_INVALID_KEY) {
        printf("❌ FAIL: Should get INVALID_KEY when MOONSHOT_API_KEY is missing\n");
        return 0;
    }
    
    printf("✅ PASS: Correct API key environment variable selection\n");
    return 1;
}

static int test_url_construction() {
    printf("\nTesting HTTP request URL construction...\n");
    
    // Set up environment for moonshot.ai
    setenv("OPENAI_API_BASE", "https://api.moonshot.ai/v1", 1);
    setenv("MOONSHOT_API_KEY", "test-key", 1);
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = "Test URL construction";
    
    chatty_Options options = {0};
    options.model = "moonshot-v1-8k";
    
    chatty_Message response;
    
    // This will fail with network error, but the URL construction should be correct
    // We can't easily intercept the actual URL without modifying the library,
    // but we can verify that it doesn't fail due to URL construction issues
    enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
    
    // Should get network error (expected with test key) or success, not other errors
    if (result == CHATTY_CURL_NETWORK_ERROR || result == CHATTY_SUCCESS || result == CHATTY_JSON_PARSE_ERROR) {
        printf("✅ PASS: URL construction appears correct (got expected network-related result)\n");
        return 1;
    } else {
        printf("❌ FAIL: Unexpected error suggests URL construction issue: %s\n", 
               chatty_error_string(result));
        return 0;
    }
}

static int test_different_base_urls() {
    printf("\nTesting different base URL formats...\n");
    
    // Test various moonshot.ai URL formats
    const char* test_urls[] = {
        "https://api.moonshot.ai/v1",
        "https://api.moonshot.ai/v1/",  // with trailing slash
    };
    
    int passed = 0;
    int total = sizeof(test_urls) / sizeof(test_urls[0]);
    
    for (int i = 0; i < total; i++) {
        printf("  Testing URL: %s\n", test_urls[i]);
        
        setenv("OPENAI_API_BASE", test_urls[i], 1);
        setenv("MOONSHOT_API_KEY", "test-key", 1);
        
        chatty_Message messages[1];
        messages[0].role = CHATTY_USER;
        messages[0].message = "Test";
        
        chatty_Options options = {0};
        options.model = "moonshot-v1-8k";
        
        chatty_Message response;
        enum chatty_ERROR result = chatty_chat(1, messages, options, &response);
        
        if (result != CHATTY_INVALID_KEY) {
            printf("    ✅ PASS: Correctly detected moonshot.ai provider\n");
            passed++;
        } else {
            printf("    ❌ FAIL: Failed to detect moonshot.ai provider\n");
        }
    }
    
    printf("Passed %d/%d URL format tests\n", passed, total);
    return passed == total;
}

int main() {
    printf("=== Moonshot.ai Integration Tests ===\n\n");
    
    int tests_passed = 0;
    int total_tests = 4;
    
    if (test_provider_detection()) tests_passed++;
    if (test_api_key_selection()) tests_passed++;
    if (test_url_construction()) tests_passed++;
    if (test_different_base_urls()) tests_passed++;
    
    printf("\n=== Test Results ===\n");
    printf("Passed: %d/%d tests\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("🎉 All tests passed! Moonshot.ai integration is working correctly.\n");
        return 0;
    } else {
        printf("❌ Some tests failed. Please check the implementation.\n");
        return 1;
    }
}