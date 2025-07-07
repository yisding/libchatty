#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "chatty.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Forward declarations of functions not in header
char *chatty_to_json_string(int msgc, chatty_Message msgv[], chatty_Options options);
cJSON *chatty_role_to_json(enum chatty_Role role);
enum chatty_Role chatty_role_from_json(cJSON *role);

void test_chatty_input_validation(void **state) {
    (void) state; /* unused */
    
    chatty_Message msg = {CHATTY_USER, "test message"};
    chatty_Options opts = {.model = "gpt-4"};
    chatty_Message response;
    
    // Test NULL response pointer
    enum chatty_ERROR result = chatty_chat(1, &msg, opts, NULL);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
    
    // Test NULL messages array
    result = chatty_chat(1, NULL, opts, &response);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
    
    // Test zero message count
    result = chatty_chat(0, &msg, opts, &response);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
    
    // Test NULL model
    chatty_Options bad_opts = {.model = NULL};
    result = chatty_chat(1, &msg, bad_opts, &response);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
    
    // Test empty model
    bad_opts.model = "";
    result = chatty_chat(1, &msg, bad_opts, &response);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
    
    // Test invalid temperature
    bad_opts.model = "gpt-4";
    bad_opts.has_temperature = true;
    bad_opts.temperature = -1.0;
    result = chatty_chat(1, &msg, bad_opts, &response);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
    
    bad_opts.temperature = 3.0;
    result = chatty_chat(1, &msg, bad_opts, &response);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
    
    // Test invalid top_p
    bad_opts.has_temperature = false;
    bad_opts.has_top_p = true;
    bad_opts.top_p = -0.1;
    result = chatty_chat(1, &msg, bad_opts, &response);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
    
    bad_opts.top_p = 1.1;
    result = chatty_chat(1, &msg, bad_opts, &response);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
    
    // Test NULL message content
    chatty_Message bad_msg = {CHATTY_USER, NULL};
    result = chatty_chat(1, &bad_msg, opts, &response);
    assert_int_equal(CHATTY_INVALID_OPTIONS, result);
}

void test_chatty_json_generation(void **state) {
    (void) state; /* unused */
    
    chatty_Message messages[2] = {
        {CHATTY_SYSTEM, "You are a helpful assistant."},
        {CHATTY_USER, "Hello, world!"}
    };
    
    chatty_Options opts = {
        .model = "gpt-4",
        .has_temperature = true,
        .temperature = 0.7,
        .has_top_p = true,
        .top_p = 0.9
    };
    
    char *json = chatty_to_json_string(2, messages, opts);
    assert_non_null(json);
    
    // Debug: print the JSON to see what we got
    printf("Generated JSON: %s\n", json);
    
    // Basic checks that the JSON contains expected elements
    // Note: cJSON_Print may use different spacing, so we check for key parts
    assert_non_null(strstr(json, "gpt-4"));
    assert_non_null(strstr(json, "model"));
    assert_non_null(strstr(json, "temperature"));
    assert_non_null(strstr(json, "top_p"));
    assert_non_null(strstr(json, "system"));
    assert_non_null(strstr(json, "user"));
    assert_non_null(strstr(json, "You are a helpful assistant."));
    assert_non_null(strstr(json, "Hello, world!"));
    
    free(json);
    
    // Test with NULL model
    opts.model = NULL;
    json = chatty_to_json_string(2, messages, opts);
    assert_null(json);
}

void test_chatty_role_conversion(void **state) {
    (void) state; /* unused */
    
    // Test role to JSON conversion
    cJSON *role_json = chatty_role_to_json(CHATTY_SYSTEM);
    assert_non_null(role_json);
    assert_string_equal("system", role_json->valuestring);
    
    role_json = chatty_role_to_json(CHATTY_USER);
    assert_non_null(role_json);
    assert_string_equal("user", role_json->valuestring);
    
    role_json = chatty_role_to_json(CHATTY_ASSISTANT);
    assert_non_null(role_json);
    assert_string_equal("assistant", role_json->valuestring);
    
    role_json = chatty_role_to_json(CHATTY_TOOL);
    assert_non_null(role_json);
    assert_string_equal("tool", role_json->valuestring);
    
    // Test JSON to role conversion
    cJSON *system_json = cJSON_CreateString("system");
    assert_int_equal(CHATTY_SYSTEM, chatty_role_from_json(system_json));
    cJSON_Delete(system_json);
    
    cJSON *user_json = cJSON_CreateString("user");
    assert_int_equal(CHATTY_USER, chatty_role_from_json(user_json));
    cJSON_Delete(user_json);
    
    cJSON *assistant_json = cJSON_CreateString("assistant");
    assert_int_equal(CHATTY_ASSISTANT, chatty_role_from_json(assistant_json));
    cJSON_Delete(assistant_json);
    
    cJSON *tool_json = cJSON_CreateString("tool");
    assert_int_equal(CHATTY_TOOL, chatty_role_from_json(tool_json));
    cJSON_Delete(tool_json);
    
    // Test invalid role
    cJSON *invalid_json = cJSON_CreateString("invalid");
    assert_int_equal((int)(-1), (int)chatty_role_from_json(invalid_json));
    cJSON_Delete(invalid_json);
    
    // Test NULL JSON
    assert_int_equal((int)(-1), (int)chatty_role_from_json(NULL));
}

void test_chatty_error_strings(void **state) {
    (void) state; /* unused */
    
    assert_string_equal("Success", chatty_error_string(CHATTY_SUCCESS));
    assert_string_equal("Invalid or missing API key", chatty_error_string(CHATTY_INVALID_KEY));
    assert_string_equal("Invalid options provided", chatty_error_string(CHATTY_INVALID_OPTIONS));
    assert_string_equal("Failed to initialize curl", chatty_error_string(CHATTY_CURL_INIT_ERROR));
    assert_string_equal("Network error or non-200 HTTP response", chatty_error_string(CHATTY_CURL_NETWORK_ERROR));
    assert_string_equal("Failed to parse JSON response", chatty_error_string(CHATTY_JSON_PARSE_ERROR));
    assert_string_equal("Memory allocation failure", chatty_error_string(CHATTY_MEMORY_ERROR));
    assert_string_equal("Unknown error", chatty_error_string(999)); // Invalid error code
}
