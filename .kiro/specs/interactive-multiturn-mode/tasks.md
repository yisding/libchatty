# Implementation Plan

- [x] 1. Add optional notcurses dependency to build system
  - Modify CMakeLists.txt to optionally detect notcurses via pkg-config and Homebrew
  - Add HAVE_NOTCURSES compile definition when notcurses is available
  - Implement fallback behavior when notcurses is not found
  - Test that the project builds successfully with and without notcurses
  - _Requirements: 4.1, 4.2, 4.3, 6.1, 6.2, 6.4_

- [x] 2. Implement conversation history data structure
  - Create ConversationHistory struct with dynamic array management
  - Implement init_conversation_history() function
  - Implement add_message_to_history() with dynamic resizing
  - Implement cleanup_conversation_history() function
  - Write unit tests for conversation history management
  - _Requirements: 2.1, 2.2, 2.3, 2.4, 5.3_

- [x] 3. Add command-line argument parsing for interactive mode
  - Extend main() function to parse -i flag for initial + interactive mode
  - Add logic to detect when to enter interactive mode (no message argument)
  - Preserve all existing command-line behavior for backward compatibility
  - Add help text for new interactive mode options
  - _Requirements: 1.1, 1.2, 4.3_

- [x] 4. Implement basic notcurses UI initialization
  - Create InteractiveUI struct to hold notcurses context and planes
  - Implement init_interactive_ui() to set up notcurses context
  - Create separate planes for conversation, status, and input areas
  - Implement cleanup_interactive_ui() for proper resource cleanup
  - Add basic error handling for notcurses initialization failures
  - _Requirements: 3.1, 3.5_

- [x] 5. Implement conversation display functionality
  - Create display_conversation() function to render message history
  - Implement message formatting with role indicators (You:, Assistant:)
  - Add color coding for different message types
  - Implement scrolling for long conversations
  - Test display with sample conversation data
  - _Requirements: 3.2, 3.3_

- [x] 6. Implement user input handling with notcurses
  - Create read_user_input_nc() function using notcurses input handling
  - Implement basic line editing capabilities (backspace, cursor movement)
  - Add input validation and empty input handling
  - Implement is_exit_command() to detect /quit, Ctrl+C, Ctrl+D
  - Test input handling with various edge cases
  - _Requirements: 1.3, 1.6, 3.2_

- [x] 7. Implement status display and feedback
  - Create display_status() function for status bar updates
  - Add "Ready", "Thinking...", and error status indicators
  - Implement visual feedback during API request processing
  - Add proper status updates throughout the interaction flow
  - _Requirements: 3.4, 3.5_

- [x] 8. Create main interactive mode loop
  - Implement run_interactive_mode() function with main conversation loop
  - Integrate conversation history management with UI display
  - Add proper error handling that allows conversation to continue
  - Implement graceful shutdown on exit commands
  - Test basic interactive flow without API calls
  - _Requirements: 1.3, 1.4, 1.5, 3.5_

- [x] 9. Integrate chatty API calls with interactive mode
  - Modify interactive loop to call chatty_chat() with full conversation history
  - Add assistant responses to conversation history after API calls
  - Implement proper error handling for API failures that doesn't exit
  - Test with different LLM providers to ensure compatibility
  - _Requirements: 2.3, 2.4, 4.1, 4.2, 4.3_

- [x] 10. Add streaming support to interactive mode
  - Integrate chatty_chat_stream() with interactive UI
  - Implement real-time response display during streaming
  - Update conversation display as streaming chunks arrive
  - Add proper status updates during streaming ("Receiving response...")
  - Test streaming mode with different providers
  - _Requirements: 4.1, 4.2, 4.3, 5.1_

- [x] 11. Implement signal handling for graceful shutdown
  - Add signal handler for SIGINT (Ctrl+C) to set exit flag
  - Implement proper cleanup sequence when signals are received
  - Test graceful shutdown preserves terminal state
  - Ensure all memory is freed on signal-based exit
  - _Requirements: 1.6, 3.5_

- [x] 12. Add conversation history limits and memory management
  - Implement maximum conversation history limit (1000 messages)
  - Add truncation strategy when limit is reached (remove oldest messages)
  - Implement memory usage monitoring and bounds checking
  - Test memory management with very long conversations
  - _Requirements: 2.5, 5.2, 5.3_

- [x] 13. Implement initial message + interactive mode (-i flag)
  - Add logic to handle -i flag with initial message
  - Send initial message and display response before entering interactive loop
  - Ensure initial message is added to conversation history
  - Test -i flag with different providers and streaming modes
  - _Requirements: 1.2, 2.1, 2.2_

- [x] 14. Add comprehensive error handling and recovery
  - Implement error handling for network failures that continues conversation
  - Add proper error messages for authentication failures
  - Handle rate limiting errors gracefully
  - Test error recovery scenarios with different failure types
  - _Requirements: 3.5, 4.4_

- [x] 15. Integration testing and performance validation
  - Test complete interactive mode flow with multiple providers
  - Validate performance requirements (sub-second responses)
  - Test memory usage with long conversations
  - Verify backward compatibility with existing CLI usage
  - Test all exit mechanisms (commands, signals, EOF)
  - _Requirements: 5.1, 5.2, 5.3, 4.4_