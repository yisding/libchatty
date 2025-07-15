# Design Document

## Overview

The interactive/multiturn mode extends the existing chatty CLI application to support continuous conversations with LLM providers. The design maintains the library's performance characteristics while adding conversation state management and an interactive user interface. The implementation will modify the main.c file to detect when to enter interactive mode and manage conversation history using the existing chatty library API.

## Architecture

### High-Level Flow

```
User Input → Mode Detection → Notcurses Init → Interactive Loop → Conversation Management → API Calls → Response Display
```

### Mode Detection Logic

The application will determine the mode based on command-line arguments:

1. **Standard Mode**: `./chatty model "message"` - Single request/response (existing behavior)
2. **Interactive Mode**: `./chatty` or `./chatty model` - Enter interactive mode immediately
3. **Initial + Interactive Mode**: `./chatty -i model "initial message"` - Send initial message then enter interactive mode

### Interactive Loop Architecture with Notcurses

```
┌─────────────────────────────────────────┐
│           Interactive Mode              │
├─────────────────────────────────────────┤
│ 1. Initialize notcurses context         │
│ 2. Create conversation display area     │
│ 3. Create input area                    │
│ 4. Read user input via notcurses        │
│ 5. Check for exit commands              │
│ 6. Add user message to history          │
│ 7. Display "thinking..." indicator      │
│ 8. Call chatty API with full history    │
│ 9. Display response in conversation     │
│ 10. Add assistant response to history   │
│ 11. Loop back to step 4                 │
└─────────────────────────────────────────┘
```

### Notcurses Integration Benefits

- **Proper terminal handling**: Automatic terminal state management
- **Rich text display**: Support for colors, formatting, and scrolling
- **Input handling**: Robust keyboard input with history and editing
- **Responsive UI**: Real-time updates during streaming responses
- **Cross-platform**: Works consistently across different terminals

## Components and Interfaces

### New Data Structures

```c
typedef struct {
    chatty_Message *messages;
    int count;
    int capacity;
} ConversationHistory;

#ifdef HAVE_NOTCURSES
typedef struct {
    struct notcurses *nc;
    struct ncplane *conversation_plane;
    struct ncplane *input_plane;
    struct ncplane *status_plane;
    int conversation_height;
    int input_height;
} InteractiveUI;
#endif
```

### New Functions

```c
// Conversation history management
void init_conversation_history(ConversationHistory *history);
int add_message_to_history(ConversationHistory *history, enum chatty_Role role, const char *message);
void cleanup_conversation_history(ConversationHistory *history);

// Interactive UI with notcurses
int init_interactive_ui(InteractiveUI *ui);
void cleanup_interactive_ui(InteractiveUI *ui);
void display_conversation(InteractiveUI *ui, ConversationHistory *history);
void display_status(InteractiveUI *ui, const char *status);
char* read_user_input_nc(InteractiveUI *ui);

// Interactive mode main loop
int run_interactive_mode(chatty_Options options, bool streaming_mode, const char *initial_message);

// Utility functions
bool is_exit_command(const char *input);
void format_message_display(char *buffer, size_t buffer_size, enum chatty_Role role, const char *message);
```

### Modified Functions

The `main()` function will be extended to:
- Parse the new `-i` flag
- Detect when to enter interactive mode
- Call the appropriate mode function

### Signal Handling

```c
#include <signal.h>

// Global flag for graceful shutdown
volatile sig_atomic_t should_exit = 0;

// Signal handler for Ctrl+C
void signal_handler(int sig);
```

## Data Models

### ConversationHistory Structure

- **messages**: Dynamic array of `chatty_Message` structs
- **count**: Current number of messages in history
- **capacity**: Current allocated capacity for messages array
- **Growth strategy**: Double capacity when full, starting with initial capacity of 10

### Memory Management Strategy

- Use `realloc()` for dynamic array growth
- Free all message content when cleaning up
- Implement reasonable limits (e.g., max 1000 messages) to prevent unbounded growth
- Each user message and assistant response is stored as a separate `chatty_Message`

## Error Handling

### Input Handling Errors

- **Empty input**: Continue loop, don't send empty messages
- **Memory allocation failures**: Display error and exit gracefully
- **Signal interruption**: Clean up resources and exit

### API Errors

- **Network errors**: Display error message and continue conversation
- **Authentication errors**: Display error and exit (can't continue without valid auth)
- **Rate limiting**: Display error message and continue (user can retry)

### Graceful Degradation

- If conversation history becomes too large, implement truncation strategy
- If memory allocation fails for history expansion, continue with current capacity
- If streaming fails, fall back to non-streaming mode for that request

## Testing Strategy

### Unit Testing Approach

1. **Conversation History Management**
   - Test initialization, growth, and cleanup
   - Test memory allocation edge cases
   - Test message addition and retrieval

2. **Input Processing**
   - Test exit command detection
   - Test empty input handling
   - Test multiline input scenarios

3. **Mode Detection**
   - Test argument parsing for different modes
   - Test flag combinations
   - Test invalid argument handling

### Integration Testing

1. **End-to-End Interactive Flow**
   - Start interactive mode and conduct multi-turn conversation
   - Test with different providers
   - Test streaming and non-streaming modes

2. **Signal Handling**
   - Test Ctrl+C graceful shutdown
   - Test Ctrl+D handling
   - Test `/quit` command

3. **Memory Management**
   - Test long conversations for memory leaks
   - Test conversation history limits
   - Test cleanup on various exit scenarios

### Performance Testing

1. **Response Time Validation**
   - Ensure interactive mode maintains sub-second response times
   - Test with growing conversation history
   - Compare performance with single-shot mode

2. **Memory Usage**
   - Monitor memory growth with conversation length
   - Test memory cleanup on exit
   - Validate reasonable memory bounds

## Dependencies

### New Build Dependencies

- **notcurses**: Terminal UI library for rich interactive interface (optional)
- **Homebrew integration**: Use notcurses from Homebrew installation on macOS
- **CMake updates**: Optional notcurses detection and linking with fallback support
- **Cross-platform compatibility**: Build system works with or without notcurses

### Notcurses Features Used (when available)

- **Terminal management**: Automatic terminal state save/restore
- **Plane system**: Separate planes for conversation, input, and status
- **Input handling**: Keyboard input with line editing capabilities
- **Color support**: Rich color and styling for user vs assistant messages
- **Scrolling**: Smooth scrolling for long conversations
- **Unicode support**: Full Unicode character rendering

### Fallback Behavior (when notcurses unavailable)

- **Simple terminal mode**: Basic printf/scanf interaction
- **Clear prompts**: Text-based "You:" and "Assistant:" indicators
- **Error messaging**: Inform user that rich interactive mode is unavailable
- **Full functionality**: All core features work without interactive UI

## Implementation Details

### User Interface Design with Notcurses

```
┌─────────────────────────────────────────────────────────────┐
│ Chatty Interactive Mode - gpt-4o                           │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│ You: Hello, how are you?                                    │
│                                                             │
│ Assistant: I'm doing well, thank you for asking! How can   │
│ I help you today?                                           │
│                                                             │
│ You: What's the weather like?                               │
│                                                             │
│ Assistant: I don't have access to real-time weather data,  │
│ but I'd be happy to help you find weather information or   │
│ discuss weather-related topics. What would you like to     │
│ know?                                                       │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│ Status: Ready                                               │
├─────────────────────────────────────────────────────────────┤
│ You: _                                                      │
└─────────────────────────────────────────────────────────────┘
```

### Visual Elements

- **Header bar**: Shows current model and mode
- **Conversation area**: Scrollable history with color-coded messages
- **Status bar**: Shows current state (Ready, Thinking, Error)
- **Input area**: Single-line input with cursor and editing support
- **Color scheme**: Different colors for user/assistant messages

### Command Line Interface

- **Existing behavior preserved**: All current command-line options continue to work
- **New `-i` flag**: Enables initial message + interactive mode
- **Backward compatibility**: No breaking changes to existing usage patterns

### Streaming Integration

- Interactive mode supports both streaming and non-streaming modes
- Use existing `stream_callback` function for real-time display
- Streaming state is preserved throughout the interactive session

### Provider Compatibility

- All existing provider configurations work unchanged
- Environment variable detection remains the same
- API key validation happens once at startup

### Exit Mechanisms

1. **`/quit` command**: Clean text-based exit
2. **Ctrl+C (SIGINT)**: Immediate graceful shutdown with cleanup
3. **Ctrl+D (EOF)**: Handle end-of-file gracefully, require double press for safety
4. **Empty input on EOF**: Treat as exit signal

### History Management Strategy

- **Initial capacity**: 10 messages
- **Growth factor**: 2x when capacity exceeded
- **Maximum limit**: 1000 messages (configurable)
- **Truncation strategy**: Remove oldest messages when limit reached
- **Memory efficiency**: Only store essential message data