#include "chatty.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CURL_NO_OLDIES
#include "cJSON.h"
#include <curl/curl.h>

// Some lines taken from https://curl.se/libcurl/c/getinmemory.html
struct chatty_Memory {
  char *memory;
  size_t size;
};

typedef struct chatty_StreamContext {
  chatty_StreamCallback callback;
  void *user_data;
  char line_buffer[4096]; /* Fixed buffer for line processing */
  size_t buffer_pos;
  bool error_occurred;
} chatty_StreamContext;

typedef struct chatty_RequestContext {
  char *base_url;
  char *api_key;
  char *chat_url;
  char *bearer_header;
  bool free_base_url;
} chatty_RequestContext;

static size_t chatty_write_memory(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  struct chatty_Memory *mem = (struct chatty_Memory *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if (!ptr) {
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

static size_t chatty_write_stream(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
  size_t realsize = size * nmemb;
  chatty_StreamContext *ctx = (chatty_StreamContext *)userp;

  if (ctx->error_occurred) {
    return 0; // Stop processing if error occurred
  }

  char *data = (char *)contents;

  for (size_t i = 0; i < realsize; i++) {
    char c = data[i];

    // Add character to line buffer
    if (ctx->buffer_pos < sizeof(ctx->line_buffer) - 1) {
      ctx->line_buffer[ctx->buffer_pos++] = c;
    } else {
      // Buffer overflow - treat as parse error
      ctx->error_occurred = true;
      return 0;
    }

    // Process complete line when we hit newline
    if (c == '\n') {
      ctx->line_buffer[ctx->buffer_pos] = '\0';

      // Check if this is a data line
      if (strncmp(ctx->line_buffer, "data: ", 6) == 0) {
        char *json_data = ctx->line_buffer + 6;

        // Handle completion signal
        if (strcmp(json_data, "[DONE]\n") == 0 ||
            strcmp(json_data, "[DONE]") == 0) {
          if (ctx->callback(NULL, CHATTY_STREAM_DONE, ctx->user_data) != 0) {
            ctx->error_occurred = true;
            return 0;
          }
        } else {
          // Parse JSON delta and extract content
          cJSON *delta_json = cJSON_Parse(json_data);
          if (delta_json != NULL) {
            cJSON *choices =
                cJSON_GetObjectItemCaseSensitive(delta_json, "choices");
            if (choices != NULL && cJSON_GetArraySize(choices) > 0) {
              cJSON *choice = cJSON_GetArrayItem(choices, 0);
              if (choice != NULL) {
                cJSON *delta =
                    cJSON_GetObjectItemCaseSensitive(choice, "delta");
                if (delta != NULL) {
                  cJSON *content =
                      cJSON_GetObjectItemCaseSensitive(delta, "content");
                  if (content != NULL && cJSON_IsString(content)) {
                    // Invoke callback with content
                    if (ctx->callback(content->valuestring, CHATTY_STREAM_CHUNK,
                                      ctx->user_data) != 0) {
                      ctx->error_occurred = true;
                      cJSON_Delete(delta_json);
                      return 0;
                    }
                  }
                }
              }
            }
            cJSON_Delete(delta_json);
          }
        }
      }

      // Reset buffer for next line
      ctx->buffer_pos = 0;
    }
  }

  return realsize;
}

cJSON *chatty_role_to_json(enum chatty_Role role) {
  switch (role) {
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

enum chatty_Role chatty_role_from_json(cJSON *role) {
  if (cJSON_IsString(role)) {
    if (strcmp(role->valuestring, "system") == 0) {
      return CHATTY_SYSTEM;
    } else if (strcmp(role->valuestring, "user") == 0) {
      return CHATTY_USER;
    } else if (strcmp(role->valuestring, "assistant") == 0) {
      return CHATTY_ASSISTANT;
    } else if (strcmp(role->valuestring, "tool") == 0) {
      return CHATTY_TOOL;
    }
  }

  return -1;
}

/* Validate input parameters common to both chat functions */
static enum chatty_ERROR chatty_validate_input(int msgc, chatty_Message msgv[],
                                               chatty_Options options) {
  if (msgc <= 0 || msgv == NULL) {
    return CHATTY_INVALID_OPTIONS;
  }

  if (options.model == NULL || strlen(options.model) == 0) {
    return CHATTY_INVALID_OPTIONS;
  }

  // Validate temperature range
  if (options.has_temperature &&
      (options.temperature < 0.0 || options.temperature > 2.0)) {
    return CHATTY_INVALID_OPTIONS;
  }

  // Validate top_p range
  if (options.has_top_p && (options.top_p < 0.0 || options.top_p > 1.0)) {
    return CHATTY_INVALID_OPTIONS;
  }

  // Validate messages
  for (int i = 0; i < msgc; i++) {
    if (msgv[i].message == NULL) {
      return CHATTY_INVALID_OPTIONS;
    }
  }

  return CHATTY_SUCCESS;
}

/* Initialize request context with provider detection and authentication */
static enum chatty_ERROR
chatty_init_request_context(chatty_RequestContext *ctx) {
  ctx->base_url = curl_getenv("OPENAI_API_BASE");
  ctx->free_base_url = true;
  if (ctx->base_url == NULL) {
    ctx->base_url = "https://api.openai.com/v1";
    ctx->free_base_url = false;
  }

  char *key_env = "OPENAI_API_KEY";
  if (strstr(ctx->base_url, "https://api.groq.com") == ctx->base_url) {
    key_env = "GROQ_API_KEY";
  } else if (strstr(ctx->base_url, "https://api.fireworks.ai") ==
             ctx->base_url) {
    key_env = "FIREWORKS_API_KEY";
  } else if (strstr(ctx->base_url, "https://api.mistral.ai") == ctx->base_url) {
    key_env = "MISTRAL_API_KEY";
  } else if (strstr(ctx->base_url, "https://api.hyperbolic.xyz") ==
             ctx->base_url) {
    key_env = "HYPERBOLIC_API_KEY";
  } else if (strstr(ctx->base_url, "https://api.deepseek.com") ==
             ctx->base_url) {
    key_env = "DEEPSEEK_API_KEY";
  } else if (strstr(ctx->base_url, "https://api.llama.com") == ctx->base_url) {
    key_env = "LLAMA_API_KEY";
  } else if (strstr(ctx->base_url, "https://api.moonshot.ai") ==
             ctx->base_url) {
    key_env = "MOONSHOT_API_KEY";
  }

  ctx->api_key = curl_getenv(key_env);
  if (ctx->api_key == NULL) {
    if (ctx->free_base_url) {
      curl_free(ctx->base_url);
    }
    return CHATTY_INVALID_KEY;
  }

  // Build chat URL
  size_t chat_url_len = strlen(ctx->base_url) + strlen("/chat/completions") + 1;
  ctx->chat_url = malloc(chat_url_len);
  if (ctx->chat_url == NULL) {
    if (ctx->free_base_url) {
      curl_free(ctx->base_url);
    }
    curl_free(ctx->api_key);
    return CHATTY_MEMORY_ERROR;
  }
  snprintf(ctx->chat_url, chat_url_len, "%s/chat/completions", ctx->base_url);

  // Build bearer header
  size_t header_len =
      strlen("Authorization: Bearer ") + strlen(ctx->api_key) + 1;
  ctx->bearer_header = malloc(header_len);
  if (ctx->bearer_header == NULL) {
    if (ctx->free_base_url) {
      curl_free(ctx->base_url);
    }
    curl_free(ctx->api_key);
    free(ctx->chat_url);
    return CHATTY_MEMORY_ERROR;
  }
  snprintf(ctx->bearer_header, header_len, "Authorization: Bearer %s",
           ctx->api_key);

  return CHATTY_SUCCESS;
}

/* Clean up request context resources */
static void chatty_cleanup_request_context(chatty_RequestContext *ctx) {
  if (ctx->free_base_url && ctx->base_url) {
    curl_free(ctx->base_url);
  }
  if (ctx->api_key) {
    curl_free(ctx->api_key);
  }
  if (ctx->chat_url) {
    free(ctx->chat_url);
  }
  if (ctx->bearer_header) {
    free(ctx->bearer_header);
  }
}

/* Initialize and configure CURL handle with common settings */
static enum chatty_ERROR chatty_setup_curl(CURL **curl,
                                           chatty_RequestContext *ctx,
                                           const char *payload,
                                           bool streaming) {
  curl_global_init(CURL_GLOBAL_ALL);

  *curl = curl_easy_init();
  if (!*curl) {
    curl_global_cleanup();
    return CHATTY_CURL_INIT_ERROR;
  }

  curl_easy_setopt(*curl, CURLOPT_USERAGENT, "libchatty/1.0");
  curl_easy_setopt(*curl, CURLOPT_URL, ctx->chat_url);
  curl_easy_setopt(*curl, CURLOPT_POSTFIELDS, payload);

  return CHATTY_SUCCESS;
}

/* Create HTTP headers for the request */
static struct curl_slist *chatty_create_headers(chatty_RequestContext *ctx,
                                                bool streaming) {
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");
  if (streaming) {
    headers = curl_slist_append(headers, "Accept: text/event-stream");
  } else {
    headers = curl_slist_append(headers, "Accept: application/json");
  }
  headers = curl_slist_append(headers, ctx->bearer_header);
  return headers;
}

/* Allocates string that needs to be freed */
char *chatty_to_json_string(int msgc, chatty_Message msgv[],
                            chatty_Options options, bool stream) {
  if (options.model == NULL) {
    return NULL;
  }

  cJSON *json = cJSON_CreateObject();
  cJSON *messages = cJSON_CreateArray();

  for (int i = 0; i < msgc; i++) {
    cJSON *message = cJSON_CreateObject();
    cJSON_AddItemToObjectCS(message, "role", chatty_role_to_json(msgv[i].role));
    cJSON_AddItemToObjectCS(message, "content",
                            cJSON_CreateString(msgv[i].message));
    cJSON_AddItemToArray(messages, message);
  }

  cJSON_AddItemToObjectCS(json, "messages", messages);
  cJSON_AddItemToObjectCS(json, "model", cJSON_CreateString(options.model));
  if (options.has_temperature) {
    cJSON_AddItemToObjectCS(json, "temperature",
                            cJSON_CreateNumber(options.temperature));
  }
  if (options.has_top_p) {
    cJSON_AddItemToObjectCS(json, "top_p", cJSON_CreateNumber(options.top_p));
  }
  if (stream) {
    cJSON_AddItemToObjectCS(json, "stream", cJSON_CreateBool(true));
  }

  char *json_string = cJSON_Print(json);
  cJSON_Delete(json);
  return json_string;
}

/* A non-zero return value indicates an error.
   response will contain the response chat message.
   You'll need to free response.message yourself. */
enum chatty_ERROR chatty_chat(int msgc, chatty_Message msgv[],
                              chatty_Options options,
                              chatty_Message *response) {
  // Input validation
  if (response == NULL) {
    return CHATTY_INVALID_OPTIONS;
  }

  enum chatty_ERROR error = chatty_validate_input(msgc, msgv, options);
  if (error != CHATTY_SUCCESS) {
    return error;
  }

  // Initialize request context
  chatty_RequestContext ctx;
  error = chatty_init_request_context(&ctx);
  if (error != CHATTY_SUCCESS) {
    return error;
  }

  // Generate JSON payload
  char *payload = chatty_to_json_string(msgc, msgv, options, false);
  if (payload == NULL) {
    chatty_cleanup_request_context(&ctx);
    return CHATTY_INVALID_OPTIONS;
  }

  // Set up CURL
  CURL *curl;
  error = chatty_setup_curl(&curl, &ctx, payload, false);
  if (error != CHATTY_SUCCESS) {
    free(payload);
    chatty_cleanup_request_context(&ctx);
    return error;
  }

  // Set up memory buffer for response
  struct chatty_Memory chunk;
  chunk.memory = malloc(1);
  if (chunk.memory == NULL) {
    free(payload);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    chatty_cleanup_request_context(&ctx);
    return CHATTY_MEMORY_ERROR;
  }
  chunk.size = 0;

  // Configure CURL for non-streaming
  struct curl_slist *headers = chatty_create_headers(&ctx, false);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, chatty_write_memory);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

  // Perform request
  CURLcode res = curl_easy_perform(curl);
  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  // Cleanup request resources
  free(payload);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  chatty_cleanup_request_context(&ctx);

  if (res != CURLE_OK || http_code != 200) {
    free(chunk.memory);
    return CHATTY_CURL_NETWORK_ERROR;
  }

  bool json_parse_fail = false;
  cJSON *response_json = cJSON_Parse(chunk.memory);
  if (response_json == NULL) {
    json_parse_fail = true;
    goto parse_end;
  }
  cJSON *choices = cJSON_GetObjectItemCaseSensitive(response_json, "choices");
  if (choices == NULL || cJSON_GetArraySize(choices) == 0) {
    json_parse_fail = true;
    goto parse_end;
  }
  cJSON *choice = cJSON_GetArrayItem(choices, 0);
  if (choice == NULL) {
    json_parse_fail = true;
    goto parse_end;
  }
  cJSON *message = cJSON_GetObjectItemCaseSensitive(choice, "message");
  if (message == NULL) {
    json_parse_fail = true;
    goto parse_end;
  }
  cJSON *role = cJSON_GetObjectItemCaseSensitive(message, "role");
  if (role == NULL) {
    json_parse_fail = true;
    goto parse_end;
  }
  enum chatty_Role role_enum = chatty_role_from_json(role);
  if (role_enum == -1) {
    json_parse_fail = true;
    goto parse_end;
  }
  cJSON *content = cJSON_GetObjectItemCaseSensitive(message, "content");
  if (content == NULL) {
    json_parse_fail = true;
    goto parse_end;
  }

parse_end:
  if (!json_parse_fail) {
    response->role = role_enum;
    response->message = strdup(content->valuestring);
    if (response->message == NULL) {
      // cleanup
      cJSON_Delete(response_json);
      free(chunk.memory);
      return CHATTY_MEMORY_ERROR;
    }
  }

  // cleanup
  cJSON_Delete(response_json); // Works even if response_json is NULL
  free(chunk.memory);

  if (json_parse_fail) {
    return CHATTY_JSON_PARSE_ERROR;
  }
  return CHATTY_SUCCESS;
}

enum chatty_ERROR chatty_chat_stream(int msgc, chatty_Message msgv[],
                                     chatty_Options options,
                                     chatty_StreamCallback callback,
                                     void *user_data) {
  // Input validation
  if (callback == NULL) {
    return CHATTY_INVALID_OPTIONS;
  }

  enum chatty_ERROR error = chatty_validate_input(msgc, msgv, options);
  if (error != CHATTY_SUCCESS) {
    return error;
  }

  // Initialize request context
  chatty_RequestContext ctx;
  error = chatty_init_request_context(&ctx);
  if (error != CHATTY_SUCCESS) {
    return error;
  }

  // Generate JSON payload
  char *payload = chatty_to_json_string(msgc, msgv, options, true);
  if (payload == NULL) {
    chatty_cleanup_request_context(&ctx);
    return CHATTY_INVALID_OPTIONS;
  }

  // Set up CURL
  CURL *curl;
  error = chatty_setup_curl(&curl, &ctx, payload, true);
  if (error != CHATTY_SUCCESS) {
    free(payload);
    chatty_cleanup_request_context(&ctx);
    return error;
  }

  // Set up streaming context
  chatty_StreamContext stream_ctx;
  stream_ctx.callback = callback;
  stream_ctx.user_data = user_data;
  stream_ctx.buffer_pos = 0;
  stream_ctx.error_occurred = false;

  // Configure CURL for streaming
  struct curl_slist *headers = chatty_create_headers(&ctx, true);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, chatty_write_stream);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&stream_ctx);

  // Perform request
  CURLcode res = curl_easy_perform(curl);
  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  // Cleanup request resources
  free(payload);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  chatty_cleanup_request_context(&ctx);

  if (res != CURLE_OK || http_code != 200) {
    return CHATTY_CURL_NETWORK_ERROR;
  }

  if (stream_ctx.error_occurred) {
    return CHATTY_STREAM_CALLBACK_ERROR;
  }

  return CHATTY_SUCCESS;
}

const char *chatty_error_string(enum chatty_ERROR error) {
  switch (error) {
  case CHATTY_SUCCESS:
    return "Success";
  case CHATTY_INVALID_KEY:
    return "Invalid or missing API key";
  case CHATTY_INVALID_OPTIONS:
    return "Invalid options provided";
  case CHATTY_CURL_INIT_ERROR:
    return "Failed to initialize curl";
  case CHATTY_CURL_NETWORK_ERROR:
    return "Network error or non-200 HTTP response";
  case CHATTY_JSON_PARSE_ERROR:
    return "Failed to parse JSON response";
  case CHATTY_MEMORY_ERROR:
    return "Memory allocation failure";
  case CHATTY_STREAM_CALLBACK_ERROR:
    return "Stream callback returned error";
  case CHATTY_STREAM_PARSE_ERROR:
    return "Failed to parse streaming response";
  default:
    return "Unknown error";
  }
}