#pragma once

#include <stdbool.h>

enum chatty_Role {
    CHATTY_SYSTEM,
    CHATTY_USER,
    CHATTY_ASSISTANT,
    CHATTY_TOOL,
};

enum chatty_ERROR {
    CHATTY_SUCCESS = 0,
    CHATTY_INVALID_KEY,
    CHATTY_INVALID_OPTIONS,
    CHATTY_CURL_INIT_ERROR,
    CHATTY_CURL_NETWORK_ERROR,
    CHATTY_JSON_PARSE_ERROR,
};

typedef struct chatty_Message {
    enum chatty_Role role;
    char* message;
} chatty_Message;

/* model is required. Should be 0 initialized using memset. */
typedef struct chatty_Options {
    char* model;
    bool has_temperature;
    double temperature;
    bool has_top_p;
    double top_p;
} chatty_Options;

enum chatty_ERROR chatty_chat(int msgc, chatty_Message msgv[], chatty_Options options, chatty_Message* response);