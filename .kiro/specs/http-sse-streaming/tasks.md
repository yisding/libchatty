# Implementation Plan

- [x] 1. Add streaming API definitions to chatty.h
  - Define `chatty_StreamStatus` enum with CHUNK, DONE, and ERROR values
  - Define `chatty_StreamCallback` function pointer type for handling streaming chunks
  - Add `chatty_chat_stream()` function declaration with callback parameter
  - Add new error codes `CHATTY_STREAM_CALLBACK_ERROR` and `CHATTY_STREAM_PARSE_ERROR` to enum
  - _Requirements: 2.1, 2.2_

- [x] 2. Implement streaming context and buffer management
  - Create `chatty_StreamContext` struct to hold callback, user_data, line buffer, and state
  - Implement fixed-size line buffer (4KB) for efficient SSE line processing
  - Add buffer position tracking and overflow detection logic
  - _Requirements: 4.1, 4.2, 4.4_

- [x] 3. Create SSE parsing and processing functions
  - Implement `chatty_write_stream()` libcurl write callback for processing SSE chunks
  - Add line-by-line processing logic to handle incoming SSE data
  - Implement SSE `data: ` line detection and parsing
  - Add logic to handle `data: [DONE]` completion signal
  - _Requirements: 1.2, 1.3, 1.4_

- [x] 4. Implement JSON delta parsing for streaming chunks
  - Create function to parse JSON delta from SSE data lines
  - Extract content from `choices[0].delta.content` field
  - Handle missing or null content fields gracefully
  - Add error handling for malformed JSON deltas
  - _Requirements: 1.2, 1.3_

- [x] 5. Modify JSON payload generation to support streaming
  - Extend `chatty_to_json_string()` function to accept streaming boolean parameter
  - Add `"stream": true` field to JSON payload when streaming is enabled
  - Maintain backward compatibility with existing non-streaming calls
  - _Requirements: 1.1, 3.3_

- [x] 6. Implement main streaming function chatty_chat_stream()
  - Create `chatty_chat_stream()` function following same pattern as `chatty_chat()`
  - Set up streaming context with user callback and data
  - Configure libcurl to use streaming write callback
  - Handle provider detection and authentication same as non-streaming
  - _Requirements: 2.1, 2.2, 3.1, 3.3_

- [x] 7. Add callback invocation and error handling
  - Implement callback invocation with content and status parameters
  - Add callback error handling - stop streaming if callback returns non-zero
  - Handle streaming-specific errors and map to appropriate error codes
  - Ensure proper cleanup on streaming errors
  - _Requirements: 2.2, 2.4, 1.5_

- [x] 8. Update error handling and add new error strings
  - Add cases for new streaming error codes in `chatty_error_string()` function
  - Ensure streaming errors are properly propagated and cleaned up
  - Test error scenarios like network failures during streaming
  - _Requirements: 1.5_

- [x] 9. Create streaming test and validation
  - Write a simple test program that uses `chatty_chat_stream()` with a callback
  - Test callback receives partial content chunks and completion signal
  - Verify streaming works with different providers (OpenAI, Groq, etc.)
  - Test error scenarios like invalid callbacks and network failures
  - _Requirements: 1.1, 1.2, 1.3, 2.1, 3.1_

- [x] 10. Add streaming example to main.c or create separate example
  - Create example usage showing how to use streaming API
  - Demonstrate real-time token display using the streaming callback
  - Show proper error handling and cleanup
  - Include example of user_data usage for context passing
  - _Requirements: 2.1, 2.2_

- [x] 11. Integrate streaming support into main.c CLI
  - Add command-line flag (--stream or -s) to enable streaming mode
  - Implement streaming callback that prints tokens in real-time
  - Maintain backward compatibility with existing non-streaming behavior
  - Add usage information showing streaming option
  - Test streaming mode with different providers and models
  - _Requirements: 2.1, 2.2, 3.1_
  
- [x] 12. Refactor shared logic between chatty_chat and chatty_chat_stream
  - Extract common HTTP setup logic into shared helper functions
  - Create shared function for provider detection and authentication
  - Extract common libcurl configuration into reusable functions
  - Consolidate error handling and cleanup logic
  - Maintain identical behavior while reducing code duplication
  - _Requirements: 4.2, 4.3_