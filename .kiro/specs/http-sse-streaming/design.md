# Design Document

## Overview

This design adds HTTP Server-Sent Events (SSE) streaming support to libchatty, enabling real-time token-by-token response streaming from LLM providers. The implementation will extend the existing `chatty_chat()` function with a new streaming variant that processes SSE data chunks and invokes user-provided callbacks for each token received.

The design maintains libchatty's performance-first philosophy by using efficient streaming parsing and minimal memory allocation while providing a simple callback-based API for handling streaming responses.

## Architecture

### Streaming API Design
The streaming functionality will be implemented as a new function `chatty_chat_stream()` that follows the same pattern as the existing `chatty_chat()` function but accepts an additional callback parameter for handling streaming chunks.

### SSE Processing Flow
1. **Request Modification**: Add `"stream": true` to the JSON payload
2. **Response Handling**: Use libcurl's write callback to process SSE chunks as they arrive
3. **Chunk Parsing**: Parse each `data: ` line containing JSON deltas
4. **Token Extraction**: Extract content from delta and invoke user callback
5. **Stream Completion**: Handle `data: [DONE]` to signal completion

### Memory Management
The streaming implementation will use a small fixed-size buffer for SSE line processing to avoid dynamic memory allocation during streaming, maintaining performance characteristics.

## Components and Interfaces

### New Public API

#### Streaming Callback Type
```c
typedef enum chatty_StreamStatus {
    CHATTY_STREAM_CHUNK,     // Partial content received
    CHATTY_STREAM_DONE,      // Stream completed successfully
    CHATTY_STREAM_ERROR      // Stream encountered an error
} chatty_StreamStatus;

typedef int (*chatty_StreamCallback)(const char *content, chatty_StreamStatus status, void *user_data);
```

#### Streaming Function
```c
enum chatty_ERROR chatty_chat_stream(
    int msgc, 
    chatty_Message msgv[], 
    chatty_Options options, 
    chatty_StreamCallback callback, 
    void *user_data
);
```

### Core Implementation Changes

#### chatty.h Additions
- Add streaming callback typedef and status enum
- Add `chatty_chat_stream()` function declaration
- Maintain backward compatibility with existing API

#### chatty.c Modifications

##### Streaming Write Callback
A new libcurl write callback function `chatty_write_stream()` that:
- Processes incoming data line by line
- Identifies SSE `data: ` lines
- Parses JSON deltas from each chunk
- Extracts content and invokes user callback
- Handles `data: [DONE]` completion signal

##### JSON Payload Modification
Extend `chatty_to_json_string()` to accept a streaming flag:
```c
char *chatty_to_json_string(int msgc, chatty_Message msgv[], chatty_Options options, bool stream);
```

##### Stream Processing Logic
```c
typedef struct chatty_StreamContext {
    chatty_StreamCallback callback;
    void *user_data;
    char line_buffer[4096];  // Fixed buffer for line processing
    size_t buffer_pos;
    bool error_occurred;
} chatty_StreamContext;
```

## Data Models

### SSE Response Format
OpenAI-compatible streaming responses use this format:
```
data: {"id":"chatcmpl-123","object":"chat.completion.chunk","created":1677652288,"model":"gpt-3.5-turbo","choices":[{"index":0,"delta":{"content":"Hello"},"finish_reason":null}]}

data: {"id":"chatcmpl-123","object":"chat.completion.chunk","created":1677652288,"model":"gpt-3.5-turbo","choices":[{"index":0,"delta":{"content":" world"},"finish_reason":null}]}

data: [DONE]
```

### Delta JSON Structure
Each streaming chunk contains a delta with partial content:
```json
{
  "choices": [{
    "index": 0,
    "delta": {
      "content": "partial text"
    },
    "finish_reason": null
  }]
}
```

### Request Modification
The streaming request adds a single field to the existing JSON:
```json
{
  "model": "gpt-3.5-turbo",
  "messages": [...],
  "stream": true
}
```

## Error Handling

### New Error Codes
Add streaming-specific error codes to the existing enum:
```c
enum chatty_ERROR {
    // ... existing codes ...
    CHATTY_STREAM_CALLBACK_ERROR,  // Callback returned error
    CHATTY_STREAM_PARSE_ERROR,     // SSE parsing failed
};
```

### Error Scenarios
1. **Callback Errors**: If user callback returns non-zero, stop streaming and return error
2. **SSE Parse Errors**: Malformed SSE data triggers `CHATTY_STREAM_PARSE_ERROR`
3. **Network Errors**: Existing network error handling applies
4. **JSON Parse Errors**: Delta parsing failures trigger appropriate error codes

### Graceful Degradation
If a provider doesn't support streaming:
- The request will succeed but return a single non-streaming response
- The callback will be invoked once with the complete content
- No special handling required - existing error codes apply

## Testing Strategy

### Unit Testing
1. **SSE Parsing**: Test line-by-line SSE data processing
2. **JSON Delta Parsing**: Verify correct content extraction from deltas
3. **Buffer Management**: Test line buffer overflow and edge cases
4. **Callback Invocation**: Verify correct callback parameters and timing

### Integration Testing
1. **Provider Compatibility**: Test streaming with all supported providers
2. **Error Handling**: Test network failures, malformed responses
3. **Memory Management**: Verify no memory leaks during streaming
4. **Performance**: Measure streaming overhead vs non-streaming

### Manual Testing
1. **Real-time Display**: Test with a simple CLI that displays tokens as received
2. **Long Responses**: Test with prompts that generate lengthy responses
3. **Error Recovery**: Test behavior when streams are interrupted

## Implementation Details

### SSE Line Processing
The streaming write callback will:
1. Append incoming data to a line buffer
2. Process complete lines (ending with `\n`)
3. Skip empty lines and non-data lines
4. Parse `data: ` lines as JSON
5. Extract content from delta and invoke callback

### Buffer Management
- Use a fixed 4KB line buffer to avoid dynamic allocation
- Handle line overflow by treating as parse error
- Reset buffer position after processing each complete line

### Callback Safety
- Validate callback pointer before invocation
- Handle callback errors by stopping stream and returning error code
- Pass user_data pointer through to callback for context

### Performance Considerations
- Minimal memory allocation during streaming
- Efficient line parsing without string copying
- Early termination on callback errors
- Reuse existing HTTP connection handling

## Backward Compatibility

### Existing API Unchanged
- All existing functions maintain identical signatures
- No changes to existing data structures
- No changes to existing error codes (only additions)

### Migration Path
- Applications can adopt streaming incrementally
- Non-streaming code continues to work unchanged
- Streaming can be enabled per-request basis

## Provider Support

### Universal Compatibility
The streaming implementation works with any OpenAI-compatible provider that supports:
- `"stream": true` parameter in requests
- SSE response format with JSON deltas
- Standard `data: [DONE]` completion signal

### Tested Providers
All existing providers support streaming:
- OpenAI (GPT models)
- Groq (Llama, Mixtral models)
- Fireworks AI (various models)
- Mistral AI (Mistral models)
- Hyperbolic (various models)
- DeepSeek (DeepSeek models)
- Moonshot AI (Moonshot models)

### Fallback Behavior
If a provider doesn't support streaming, the request will either:
1. Return a single non-streaming response (callback invoked once)
2. Return an HTTP error (handled by existing error handling)