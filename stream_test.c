#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chatty.h"

// Context structure to demonstrate user_data usage
typedef struct {
    int chunk_count;
    size_t total_chars;
} StreamContext;

// Streaming callback that prints chunks and tracks statistics
int stream_callback(const char *content, chatty_StreamStatus status, void *user_data)
{
    StreamContext *ctx = (StreamContext *)user_data;
    
    switch (status)
    {
    case CHATTY_STREAM_CHUNK:
        if (content != NULL)
        {
            printf("%s", content);
            fflush(stdout); // Ensure immediate output
            
            // Update statistics if context provided
            if (ctx != NULL)
            {
                ctx->chunk_count++;
                ctx->total_chars += strlen(content);
            }
        }
        break;
    case CHATTY_STREAM_DONE:
        printf("\n");
        if (ctx != NULL)
        {
            printf("[Stream completed - %d chunks, %zu characters]\n", 
                   ctx->chunk_count, ctx->total_chars);
        }
        else
        {
            printf("[Stream completed]\n");
        }
        break;
    case CHATTY_STREAM_ERROR:
        printf("\n[Stream error]\n");
        return 1; // Signal error to stop streaming
    }
    return 0; // Success - continue streaming
}

int main(int argc, char *argv[])
{
    chatty_Message messages[1];

    messages[0].role = CHATTY_USER;

    if (argc >= 3)
    {
        messages[0].message = argv[2];
    }
    else
    {
        messages[0].message = "Write a short poem about programming in C.";
    }

    chatty_Options options = {0};

    if (argc >= 2)
    {
        options.model = argv[1];
    }
    else
    {
        options.model = "gpt-4o";
    }

    // Initialize streaming context for statistics tracking
    StreamContext ctx = {0, 0};

    printf("=== libchatty Streaming Example ===\n");
    printf("Model: %s\n", options.model);
    printf("Prompt: %s\n", messages[0].message);
    printf("Response: ");
    fflush(stdout);

    // Call streaming function with context for statistics
    enum chatty_ERROR error = chatty_chat_stream(1, messages, options, stream_callback, &ctx);

    if (error != CHATTY_SUCCESS)
    {
        fprintf(stderr, "\nError: %s\n", chatty_error_string(error));
        return 1;
    }

    return 0;
}