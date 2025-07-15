# libchatty Streaming API

libchatty supports real-time streaming responses using HTTP Server-Sent Events (SSE), allowing you to display partial responses as they arrive from LLM providers.

## Quick Start

```c
#include "chatty.h"

// Define a callback to handle streaming chunks
int my_callback(const char *content, chatty_StreamStatus status, void *user_data) {
    switch (status) {
        case CHATTY_STREAM_CHUNK:
            if (content) printf("%s", content);
            break;
        case CHATTY_STREAM_DONE:
            printf("\n[Complete]\n");
            break;
        case CHATTY_STREAM_ERROR:
            printf("\n[Error]\n");
            return 1; // Stop streaming
    }
    return 0; // Continue streaming
}

int main() {
    chatty_Message messages[] = {{CHATTY_USER, "Hello, world!"}};
    chatty_Options options = {.model = "gpt-4o"};
    
    // Stream the response
    enum chatty_ERROR error = chatty_chat_stream(1, messages, options, my_callback, NULL);
    
    if (error != CHATTY_SUCCESS) {
        fprintf(stderr, "Error: %s\n", chatty_error_string(error));
        return 1;
    }
    return 0;
}
```

## API Reference

### chatty_chat_stream()

```c
enum chatty_ERROR chatty_chat_stream(
    int msgc, 
    chatty_Message msgv[], 
    chatty_Options options, 
    chatty_StreamCallback callback, 
    void *user_data
);
```

Streams a chat completion response, invoking the callback for each token received.

**Parameters:**
- `msgc` - Number of messages
- `msgv` - Array of messages
- `options` - Chat options (model, temperature, etc.)
- `callback` - Function to handle streaming chunks
- `user_data` - Optional pointer passed to callback

**Returns:** `chatty_ERROR` code

### chatty_StreamCallback

```c
typedef int (*chatty_StreamCallback)(const char *content, chatty_StreamStatus status, void *user_data);
```

Callback function type for handling streaming responses.

**Parameters:**
- `content` - Partial text content (NULL for non-chunk statuses)
- `status` - Stream status (CHUNK, DONE, or ERROR)
- `user_data` - User-provided context pointer

**Returns:** 0 to continue streaming, non-zero to stop

### chatty_StreamStatus

```c
typedef enum {
    CHATTY_STREAM_CHUNK,     // Partial content received
    CHATTY_STREAM_DONE,      // Stream completed successfully  
    CHATTY_STREAM_ERROR      // Stream encountered an error
} chatty_StreamStatus;
```

## Error Handling

New streaming-specific error codes:

- `CHATTY_STREAM_CALLBACK_ERROR` - Callback returned error
- `CHATTY_STREAM_PARSE_ERROR` - Failed to parse streaming response

## Provider Support

Streaming works with all supported providers:
- OpenAI (GPT models)
- Groq (Llama, Mixtral models)
- Fireworks AI
- Mistral AI
- Hyperbolic
- DeepSeek
- Moonshot AI

## Examples

See `stream_test.c` for a complete example with statistics tracking.

Run the streaming test:
```bash
./stream_test gpt-4o "Write a short story"
```

## Performance

Streaming maintains libchatty's performance characteristics:
- Fixed 4KB buffer for SSE processing
- Minimal memory allocation
- Efficient line-by-line parsing
- No performance impact on non-streaming calls