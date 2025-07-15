# Design Document

## Overview

This design adds moonshot.ai support to libchatty by extending the existing provider detection mechanism. Moonshot.ai provides an OpenAI-compatible API at `https://api.moonshot.ai/v1`, making integration straightforward using the established pattern of automatic API key detection based on base URL matching.

The implementation will follow the same architectural approach as existing providers (Groq, Fireworks, Mistral, etc.) to maintain consistency and performance characteristics.

## Architecture

### Provider Detection Pattern
The existing `chatty_chat()` function uses string matching on the `OPENAI_API_BASE` environment variable to determine which API key environment variable to use. This pattern will be extended to include moonshot.ai detection.

### API Compatibility
Moonshot.ai uses the OpenAI-compatible chat completions endpoint:
- **Base URL**: `https://api.moonshot.ai/v1`
- **Endpoint**: `/chat/completions` 
- **Authentication**: Bearer token via `Authorization` header
- **Request/Response Format**: Standard OpenAI chat completions format

## Components and Interfaces

### Core Library Changes

#### chatty.c Modifications
The main change will be in the `chatty_chat()` function's provider detection logic:

```c
// Existing provider detection code...
else if (strstr(base, "https://api.llama.com") == base)
{
    key_env = "LLAMA_API_KEY";
}
// NEW: Add moonshot.ai detection
else if (strstr(base, "https://api.moonshot.ai") == base)
{
    key_env = "MOONSHOT_API_KEY";
}
```

This follows the exact same pattern as existing providers:
1. Check if the base URL starts with the moonshot.ai API base
2. Set the appropriate environment variable name for API key lookup
3. The rest of the flow remains unchanged

#### No API Changes Required
- No changes to `chatty.h` public interface
- No changes to data structures or function signatures
- No changes to JSON request/response handling (OpenAI-compatible)

### Benchmark Script

#### moonshot.sh Implementation
A new shell script following the established pattern:

```bash
#!/bin/zsh

for i in {1..10}
do
    OPENAI_API_BASE="https://api.moonshot.ai/v1" build/chatty [default-model]
done
```

The script will:
- Set the correct API base URL for moonshot.ai
- Run 10 benchmark iterations (consistent with other provider scripts)
- Use a sensible default model (to be determined based on moonshot.ai's available models)

## Data Models

### Environment Variables
- **MOONSHOT_API_KEY**: API key for moonshot.ai authentication
- **OPENAI_API_BASE**: Set to "https://api.moonshot.ai/v1" to use moonshot.ai

### Request/Response Format
No changes required - moonshot.ai uses standard OpenAI chat completions format:
- Same JSON request structure
- Same JSON response structure  
- Same message roles (system, user, assistant, tool)
- Same optional parameters (temperature, top_p)

## Error Handling

### Existing Error Handling Applies
The current error handling in `chatty_chat()` will work without modification:
- **CHATTY_INVALID_KEY**: When `MOONSHOT_API_KEY` is not set
- **CHATTY_CURL_NETWORK_ERROR**: For HTTP errors or network issues
- **CHATTY_JSON_PARSE_ERROR**: For malformed API responses
- **CHATTY_MEMORY_ERROR**: For memory allocation failures

### Provider-Specific Considerations
- Moonshot.ai API errors will be handled through the existing HTTP status code checking
- JSON response parsing remains the same due to OpenAI compatibility
- No special error handling required for moonshot.ai-specific scenarios

## Testing Strategy

### Manual Testing
1. **Environment Setup**: Set `MOONSHOT_API_KEY` environment variable
2. **Basic Functionality**: Test chat completion with moonshot.ai models
3. **Error Scenarios**: Test with invalid API key, network issues
4. **Benchmark Testing**: Run `moonshot.sh` script for performance validation

### Integration Testing
1. **Provider Detection**: Verify correct API key environment variable is used
2. **URL Construction**: Ensure correct endpoint URL is built
3. **Request Format**: Confirm JSON payload matches OpenAI format
4. **Response Parsing**: Validate response parsing works with moonshot.ai responses

### Performance Testing
1. **Benchmark Comparison**: Compare performance with other providers using respective scripts
2. **Memory Usage**: Ensure no memory leaks or increased usage
3. **Latency**: Measure request/response times

### Compatibility Testing
1. **Model Support**: Test with various moonshot.ai model names
2. **Parameter Support**: Test temperature and top_p parameters
3. **Message Types**: Test different message roles and content types

## Implementation Notes

### Minimal Code Changes
The design prioritizes minimal changes to maintain the library's simplicity and performance:
- Single line addition for provider detection
- No changes to core data structures
- No changes to JSON handling logic
- No changes to HTTP client configuration

### Consistency with Existing Providers
The implementation will be indistinguishable from other providers in terms of:
- API usage patterns
- Performance characteristics
- Error handling behavior
- Memory usage patterns

### Default Model Selection
The benchmark script will need a default model name. Common moonshot.ai models include:
- `moonshot-v1-8k`
- `moonshot-v1-32k` 
- `moonshot-v1-128k`

The script will use `moonshot-v1-8k` as the default for consistency with other providers using smaller context models for benchmarking.