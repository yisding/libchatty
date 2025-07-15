#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "chatty.h"

// Streaming callback function that prints tokens in real-time
int stream_callback(const char *content, chatty_StreamStatus status, void *user_data)
{
    (void)user_data; // Unused parameter
    
    switch (status)
    {
        case CHATTY_STREAM_CHUNK:
            if (content && strlen(content) > 0)
            {
                printf("%s", content);
                fflush(stdout); // Ensure immediate output
            }
            break;
        case CHATTY_STREAM_DONE:
            printf("\n"); // Add newline when stream is complete
            break;
        case CHATTY_STREAM_ERROR:
            fprintf(stderr, "\nStreaming error occurred\n");
            return 1; // Return error to stop streaming
    }
    return 0; // Continue streaming
}

void print_usage(const char *program_name)
{
    printf("Usage: %s [OPTIONS] [model] [message]\n", program_name);
    printf("Options:\n");
    printf("  -s, --stream    Enable streaming mode for real-time token display\n");
    printf("  -h, --help      Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s gpt-4o \"Hello, world!\"\n", program_name);
    printf("  %s --stream gpt-4o \"Tell me a story\"\n", program_name);
    printf("  %s -s llama-3.1-70b-versatile \"Explain quantum computing\"\n", program_name);
}

int main(int argc, char *argv[])
{
    bool streaming_mode = false;
    int arg_offset = 1;
    
    // Parse command-line arguments
    if (argc >= 2)
    {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--stream") == 0)
        {
            streaming_mode = true;
            arg_offset = 2;
        }
    }
    
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;

    // Parse message argument (accounting for streaming flag offset)
    if (argc >= arg_offset + 2)
    {
        messages[0].message = argv[arg_offset + 1];
    }
    else
    {
        messages[0].message = "What is the C++ FQA?";
    }

    chatty_Options options = {0};

    // Parse model argument (accounting for streaming flag offset)
    if (argc >= arg_offset + 1)
    {
        options.model = argv[arg_offset];
    }
    else
    {
        options.model = "gpt-4o";
    }

    enum chatty_ERROR error;
    
    if (streaming_mode)
    {
        // Use streaming mode
        printf("LLM Response (streaming):\n");
        error = chatty_chat_stream(1, messages, options, stream_callback, NULL);
        
        if (error != CHATTY_SUCCESS)
        {
            fprintf(stderr, "Streaming error: %s\n", chatty_error_string(error));
            return 1;
        }
    }
    else
    {
        // Use traditional non-streaming mode
        chatty_Message response;
        error = chatty_chat(1, messages, options, &response);

        if (error != CHATTY_SUCCESS)
        {
            fprintf(stderr, "Error: %s\n", chatty_error_string(error));
            return 1;
        }

        printf("LLM Response:\n%s\n", response.message);
        free(response.message);
    }

    return 0;
}