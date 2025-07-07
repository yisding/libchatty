#pragma once

#include <stdbool.h>

enum chatty_Role
{
    CHATTY_SYSTEM,
    CHATTY_USER,
    CHATTY_ASSISTANT,
    CHATTY_TOOL,
};

enum chatty_ERROR
{
    CHATTY_SUCCESS = 0,
    CHATTY_INVALID_KEY,
    CHATTY_INVALID_OPTIONS,
    CHATTY_CURL_INIT_ERROR,
    CHATTY_CURL_NETWORK_ERROR,
    CHATTY_JSON_PARSE_ERROR,
};

typedef struct chatty_Message
{
    enum chatty_Role role;
    char *message;
} chatty_Message;

/* model is required. Should be 0 initialized using memset. */
typedef struct chatty_Options
{
    char *model;
    bool has_temperature; /* We need a has_temperature boolean because 0 is a valid temperature. */
    double temperature;
    bool has_top_p; /* Same for top_p, 0 is also a valid top_p */
    double top_p;
} chatty_Options;

enum chatty_ERROR chatty_chat(int msgc, chatty_Message msgv[], chatty_Options options, chatty_Message *response);

/* Get string representation of error code */
const char *chatty_error_string(enum chatty_ERROR error);