#include "chatty.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CURL_NO_OLDIES
#include <curl/curl.h>
#include "cJSON.h"

// Some lines taken from https://curl.se/libcurl/c/getinmemory.html
struct chatty_Memory
{
    char *memory;
    size_t size;
};

static size_t chatty_write_memory(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct chatty_Memory *mem = (struct chatty_Memory *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (!ptr)
    {
        // out of memory!
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

cJSON *chatty_role_to_json(enum chatty_Role role)
{
    switch (role)
    {
    case CHATTY_SYSTEM:
        return cJSON_CreateStringReference("system");
    case CHATTY_USER:
        return cJSON_CreateStringReference("user");
    case CHATTY_ASSISTANT:
        return cJSON_CreateStringReference("assistant");
    case CHATTY_TOOL:
        return cJSON_CreateStringReference("tool");
    }
}

enum chatty_Role chatty_role_from_json(cJSON *role)
{
    if (cJSON_IsString(role))
    {
        if (strcmp(role->valuestring, "system") == 0)
        {
            return CHATTY_SYSTEM;
        }
        else if (strcmp(role->valuestring, "user") == 0)
        {
            return CHATTY_USER;
        }
        else if (strcmp(role->valuestring, "assistant") == 0)
        {
            return CHATTY_ASSISTANT;
        }
        else if (strcmp(role->valuestring, "tool") == 0)
        {
            return CHATTY_TOOL;
        }
    }

    return -1;
}

/* Allocates string that needs to be freed */
char *chatty_to_json_string(int msgc, chatty_Message msgv[], chatty_Options options)
{
    if (options.model == NULL)
    {
        return NULL;
    }

    cJSON *json = cJSON_CreateObject();
    cJSON *messages = cJSON_CreateArray();

    for (int i = 0; i < msgc; i++)
    {
        cJSON *message = cJSON_CreateObject();
        cJSON_AddItemToObjectCS(message, "role", chatty_role_to_json(msgv[i].role));
        cJSON_AddItemToObjectCS(message, "content", cJSON_CreateString(msgv[i].message));
        cJSON_AddItemToArray(messages, message);
    }

    cJSON_AddItemToObjectCS(json, "messages", messages);
    cJSON_AddItemToObjectCS(json, "model", cJSON_CreateString(options.model));
    if (options.has_temperature)
    {
        cJSON_AddItemToObjectCS(json, "temperature", cJSON_CreateNumber(options.temperature));
    }
    if (options.has_top_p)
    {
        cJSON_AddItemToObjectCS(json, "top_p", cJSON_CreateNumber(options.top_p));
    }

    char *json_string = cJSON_Print(json);
    cJSON_Delete(json);
    return json_string;
}

/* A non-zero return value indicates an error.
   response will contain the response chat message.
   You'll need to free response.message yourself. */
enum chatty_ERROR chatty_chat(int msgc, chatty_Message msgv[], chatty_Options options, chatty_Message *response)
{
    char *base = curl_getenv("OPENAI_API_BASE");
    bool free_base = true;
    if (base == NULL)
    {
        base = "https://api.openai.com/v1";
        free_base = false;
    }

    char *key_env = "OPENAI_API_KEY";
    if (strncmp(base, "https://api.groq.com", strlen("https://api.groq.com")) == 0)
    {
        key_env = "GROQ_API_KEY";
    }
    else if (strncmp(base, "https://api.fireworks.ai", strlen("https://api.fireworks.ai")) == 0)
    {
        key_env = "FIREWORKS_API_KEY";
    }
    else if (strncmp(base, "https://api.mistral.ai", strlen("https://api.mistral.ai")) == 0)
    {
        key_env = "MISTRAL_API_KEY";
    }
    else if (strncmp(base, "https://api.hyperbolic.xyz", strlen("https://api.hyperbolic.xyz")) == 0)
    {
        key_env = "HYPERBOLIC_API_KEY";
    }

    char *key = curl_getenv(key_env);
    if (key == NULL)
    {
        return CHATTY_INVALID_KEY;
    }

    size_t chat_url_len = strlen(base) + strlen("/chat/completions") + 1;
    char *chat_url = malloc(chat_url_len);
    snprintf(chat_url, chat_url_len, "%s/chat/completions", base);

    if (free_base)
    {
        curl_free(base);
    }

    size_t header_len = strlen("Authorization: Bearer ") + strlen(key) + 1;
    char *bearer_header = malloc(header_len);
    snprintf(bearer_header, header_len, "Authorization: Bearer %s", key);
    curl_free(key);

    // In windows, this inits the winsock stuff, although tbh I haven't tested this on Windows.
    curl_global_init(CURL_GLOBAL_ALL);

    CURL *curl = curl_easy_init();
    if (!curl)
    {
        curl_global_cleanup();
        free(bearer_header);
        free(chat_url);
        return CHATTY_CURL_INIT_ERROR;
    }

    char *payload = chatty_to_json_string(msgc, msgv, options);

    if (payload == NULL)
    {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        free(chat_url);
        free(bearer_header);
        return CHATTY_INVALID_OPTIONS;
    }

    struct chatty_Memory chunk;

    chunk.memory = malloc(1); // grown as needed by the realloc above
    chunk.size = 0;           // no data at this point

    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libchatty/1.0");
    curl_easy_setopt(curl, CURLOPT_URL, chat_url);
    free(chat_url);
    curl_easy_setopt(curl, CURLOPT_PROXY_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, bearer_header);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, chatty_write_memory);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(curl);

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    free(payload);
    curl_slist_free_all(headers);

    if (res != CURLE_OK || http_code != 200)
    {
        free(chunk.memory);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        free(bearer_header);
        return CHATTY_CURL_NETWORK_ERROR;
    }

    bool json_parse_fail = false;
    cJSON *response_json = cJSON_Parse(chunk.memory);
    if (response_json == NULL)
    {
        json_parse_fail = true;
        goto parse_end;
    }
    cJSON *choices = cJSON_GetObjectItemCaseSensitive(response_json, "choices");
    if (choices == NULL || cJSON_GetArraySize(choices) == 0)
    {
        json_parse_fail = true;
        goto parse_end;
    }
    cJSON *choice = cJSON_GetArrayItem(choices, 0);
    if (choice == NULL)
    {
        json_parse_fail = true;
        goto parse_end;
    }
    cJSON *message = cJSON_GetObjectItemCaseSensitive(choice, "message");
    if (message == NULL)
    {
        json_parse_fail = true;
        goto parse_end;
    }
    cJSON *role = cJSON_GetObjectItemCaseSensitive(message, "role");
    if (role == NULL)
    {
        json_parse_fail = true;
        goto parse_end;
    }
    enum chatty_Role role_enum = chatty_role_from_json(role);
    if (role_enum == -1)
    {
        json_parse_fail = true;
        goto parse_end;
    }
    cJSON *content = cJSON_GetObjectItemCaseSensitive(message, "content");
    if (content == NULL)
    {
        json_parse_fail = true;
        goto parse_end;
    }

parse_end:
    if (!json_parse_fail)
    {
        response->role = role_enum;
        response->message = strdup(content->valuestring);
    }

    // cleanup
    cJSON_Delete(response_json); // Works even if response_json is NULL
    free(chunk.memory);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    free(bearer_header);

    if (json_parse_fail)
    {
        return CHATTY_JSON_PARSE_ERROR;
    }
    return CHATTY_SUCCESS;
}