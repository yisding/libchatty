#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chatty.h"

#ifdef HAVE_NOTCURSES
#include <notcurses/notcurses.h>
#endif

// Conversation history data structure
typedef struct {
  chatty_Message *messages;
  int count;
  int capacity;
} ConversationHistory;

// Initialize conversation history with initial capacity
void init_conversation_history(ConversationHistory *history) {
  history->messages = malloc(10 * sizeof(chatty_Message));
  history->count = 0;
  history->capacity = 10;

  if (history->messages == NULL) {
    fprintf(stderr, "Failed to allocate memory for conversation history\n");
    exit(1);
  }
}

// Add message to conversation history with dynamic resizing
int add_message_to_history(ConversationHistory *history, enum chatty_Role role,
                           const char *message) {
  // Check if we need to resize the array
  if (history->count >= history->capacity) {
    // Check maximum limit (1000 messages)
    if (history->capacity >= 1000) {
      // Remove oldest message to make room
      free(history->messages[0].message);
      memmove(&history->messages[0], &history->messages[1],
              (history->count - 1) * sizeof(chatty_Message));
      history->count--;
    } else {
      // Double the capacity
      int new_capacity = history->capacity * 2;
      if (new_capacity > 1000)
        new_capacity = 1000;

      chatty_Message *new_messages =
          realloc(history->messages, new_capacity * sizeof(chatty_Message));
      if (new_messages == NULL) {
        fprintf(stderr, "Failed to resize conversation history\n");
        return -1;
      }

      history->messages = new_messages;
      history->capacity = new_capacity;
    }
  }

  // Add the new message
  history->messages[history->count].role = role;
  history->messages[history->count].message = strdup(message);

  if (history->messages[history->count].message == NULL) {
    fprintf(stderr, "Failed to allocate memory for message\n");
    return -1;
  }

  history->count++;
  return 0;
}

// Clean up conversation history
void cleanup_conversation_history(ConversationHistory *history) {
  if (history->messages != NULL) {
    for (int i = 0; i < history->count; i++) {
      free(history->messages[i].message);
    }
    free(history->messages);
    history->messages = NULL;
    history->count = 0;
    history->capacity = 0;
  }
}

#ifdef HAVE_NOTCURSES
// Interactive UI data structure
typedef struct {
  struct notcurses *nc;
  struct ncplane *conversation_plane;
  struct ncplane *input_plane;
  struct ncplane *status_plane;
  int conversation_height;
  int input_height;
} InteractiveUI;

// Forward declarations
void cleanup_interactive_ui(InteractiveUI *ui);
void display_conversation(InteractiveUI *ui, ConversationHistory *history);
void format_message_display(char *buffer, size_t buffer_size,
                            enum chatty_Role role, const char *message);
char *read_user_input_nc(InteractiveUI *ui);
bool is_exit_command(const char *input);
void display_status(InteractiveUI *ui, const char *status);
int run_interactive_mode(chatty_Options options, bool streaming_mode,
                         const char *initial_message);

// Streaming context for interactive mode
typedef struct {
  InteractiveUI *ui;
  ConversationHistory *history;
  char *current_response;
  size_t response_capacity;
  size_t response_length;
} InteractiveStreamContext;

// Streaming callback for interactive mode
int interactive_stream_callback(const char *content, chatty_StreamStatus status,
                                void *user_data);

// Streaming callback for interactive mode
int interactive_stream_callback(const char *content, chatty_StreamStatus status,
                                void *user_data) {
  InteractiveStreamContext *ctx = (InteractiveStreamContext *)user_data;

  switch (status) {
  case CHATTY_STREAM_CHUNK:
    if (content && strlen(content) > 0) {
      // Expand response buffer if needed
      size_t content_len = strlen(content);
      if (ctx->response_length + content_len >= ctx->response_capacity) {
        size_t new_capacity = ctx->response_capacity * 2;
        if (new_capacity < ctx->response_length + content_len + 1) {
          new_capacity = ctx->response_length + content_len + 1024;
        }

        char *new_response = realloc(ctx->current_response, new_capacity);
        if (new_response == NULL) {
          return 1; // Error - stop streaming
        }

        ctx->current_response = new_response;
        ctx->response_capacity = new_capacity;
      }

      // Append content to current response
      strcpy(ctx->current_response + ctx->response_length, content);
      ctx->response_length += content_len;

      // Update the conversation display with current partial response
      // First, temporarily add the partial response to history
      if (add_message_to_history(ctx->history, CHATTY_ASSISTANT,
                                 ctx->current_response) == 0) {
        display_conversation(ctx->ui, ctx->history);
        display_status(ctx->ui, "Receiving response...");

        // Remove the temporary message (we'll add the final one when done)
        ctx->history->count--;
        free(ctx->history->messages[ctx->history->count].message);
      }
    }
    break;

  case CHATTY_STREAM_DONE:
    // Streaming is complete - the final response is in current_response
    display_status(ctx->ui, "Ready");
    break;

  case CHATTY_STREAM_ERROR:
    display_status(ctx->ui, "Error: Streaming failed");
    return 1; // Return error to stop streaming
  }

  return 0; // Continue streaming
}

// Initialize interactive UI with notcurses
int init_interactive_ui(InteractiveUI *ui) {
  // Initialize notcurses
  struct notcurses_options opts = {0};
  opts.flags = NCOPTION_SUPPRESS_BANNERS | NCOPTION_NO_WINCH_SIGHANDLER |
               NCOPTION_INHIBIT_SETLOCALE;

  ui->nc = notcurses_init(&opts, NULL);
  if (ui->nc == NULL) {
    fprintf(stderr, "Failed to initialize notcurses\n");
    return -1;
  }

  // Get terminal dimensions
  unsigned rows, cols;
  notcurses_term_dim_yx(ui->nc, &rows, &cols);

  // Calculate plane dimensions
  ui->conversation_height = rows - 3; // Leave room for status and input
  ui->input_height = 1;

  // Create conversation plane (main area)
  struct ncplane_options conv_opts = {.y = 0,
                                      .x = 0,
                                      .rows = ui->conversation_height,
                                      .cols = cols,
                                      .userptr = NULL,
                                      .name = "conversation",
                                      .resizecb = NULL,
                                      .flags = 0,
                                      .margin_b = 0,
                                      .margin_r = 0};
  ui->conversation_plane =
      ncplane_create(notcurses_stdplane(ui->nc), &conv_opts);
  if (ui->conversation_plane == NULL) {
    fprintf(stderr, "Failed to create conversation plane\n");
    notcurses_stop(ui->nc);
    return -1;
  }

  // Create status plane (one line above input)
  struct ncplane_options status_opts = {.y = rows - 2,
                                        .x = 0,
                                        .rows = 1,
                                        .cols = cols,
                                        .userptr = NULL,
                                        .name = "status",
                                        .resizecb = NULL,
                                        .flags = 0,
                                        .margin_b = 0,
                                        .margin_r = 0};
  ui->status_plane = ncplane_create(notcurses_stdplane(ui->nc), &status_opts);
  if (ui->status_plane == NULL) {
    fprintf(stderr, "Failed to create status plane\n");
    ncplane_destroy(ui->conversation_plane);
    notcurses_stop(ui->nc);
    return -1;
  }

  // Create input plane (bottom line)
  struct ncplane_options input_opts = {.y = rows - 1,
                                       .x = 0,
                                       .rows = ui->input_height,
                                       .cols = cols,
                                       .userptr = NULL,
                                       .name = "input",
                                       .resizecb = NULL,
                                       .flags = 0,
                                       .margin_b = 0,
                                       .margin_r = 0};
  ui->input_plane = ncplane_create(notcurses_stdplane(ui->nc), &input_opts);
  if (ui->input_plane == NULL) {
    fprintf(stderr, "Failed to create input plane\n");
    ncplane_destroy(ui->conversation_plane);
    ncplane_destroy(ui->status_plane);
    notcurses_stop(ui->nc);
    return -1;
  }

  // Set up initial display
  ncplane_set_fg_rgb8(ui->status_plane, 0x88, 0x88, 0x88);
  ncplane_putstr(ui->status_plane, "Ready");

  ncplane_putstr(ui->input_plane, "You: ");

  // Render initial state
  if (notcurses_render(ui->nc) != 0) {
    fprintf(stderr, "Failed to render initial UI\n");
    cleanup_interactive_ui(ui);
    return -1;
  }

  return 0;
}

// Clean up interactive UI
void cleanup_interactive_ui(InteractiveUI *ui) {
  if (ui->input_plane) {
    ncplane_destroy(ui->input_plane);
    ui->input_plane = NULL;
  }
  if (ui->status_plane) {
    ncplane_destroy(ui->status_plane);
    ui->status_plane = NULL;
  }
  if (ui->conversation_plane) {
    ncplane_destroy(ui->conversation_plane);
    ui->conversation_plane = NULL;
  }
  if (ui->nc) {
    // Ensure proper terminal state restoration
    notcurses_stop(ui->nc);
    ui->nc = NULL;

    // Additional terminal reset to ensure proper state restoration
    printf("\033[0m"); // Reset all attributes
    fflush(stdout);
  }
}

// Format message for display with role indicator
void format_message_display(char *buffer, size_t buffer_size,
                            enum chatty_Role role, const char *message) {
  const char *role_str;
  switch (role) {
  case CHATTY_USER:
    role_str = "You";
    break;
  case CHATTY_ASSISTANT:
    role_str = "Assistant";
    break;
  case CHATTY_SYSTEM:
    role_str = "System";
    break;
  case CHATTY_TOOL:
    role_str = "Tool";
    break;
  default:
    role_str = "Unknown";
    break;
  }

  snprintf(buffer, buffer_size, "%s: %s", role_str, message);
}

// Display conversation history in the conversation plane
void display_conversation(InteractiveUI *ui, ConversationHistory *history) {
  // Clear the conversation plane
  ncplane_erase(ui->conversation_plane);

  // Get plane dimensions
  unsigned rows, cols;
  ncplane_dim_yx(ui->conversation_plane, &rows, &cols);

  int current_row = 0;

  // Display each message in the conversation history
  for (int i = 0; i < history->count && current_row < (int)rows; i++) {
    char display_buffer[2048];
    format_message_display(display_buffer, sizeof(display_buffer),
                           history->messages[i].role,
                           history->messages[i].message);

    // Set color based on role
    if (history->messages[i].role == CHATTY_USER) {
      ncplane_set_fg_rgb8(ui->conversation_plane, 0x00, 0xAA,
                          0xFF); // Blue for user
    } else if (history->messages[i].role == CHATTY_ASSISTANT) {
      ncplane_set_fg_rgb8(ui->conversation_plane, 0x00, 0xFF,
                          0x00); // Green for assistant
    } else {
      ncplane_set_fg_rgb8(ui->conversation_plane, 0xFF, 0xFF,
                          0xFF); // White for others
    }

    // Move cursor to the current row
    ncplane_cursor_move_yx(ui->conversation_plane, current_row, 0);

    // Handle word wrapping for long messages
    size_t msg_len = strlen(display_buffer);
    size_t pos = 0;

    while (pos < msg_len && current_row < (int)rows) {
      size_t line_len = (msg_len - pos > cols) ? cols : (msg_len - pos);

      // Find a good break point (space) if we're wrapping
      if (line_len == cols && pos + line_len < msg_len) {
        size_t break_pos = line_len;
        while (break_pos > 0 && display_buffer[pos + break_pos] != ' ') {
          break_pos--;
        }
        if (break_pos > 0) {
          line_len = break_pos;
        }
      }

      // Print the line
      ncplane_cursor_move_yx(ui->conversation_plane, current_row, 0);
      for (size_t j = 0; j < line_len; j++) {
        ncplane_putchar(ui->conversation_plane, display_buffer[pos + j]);
      }

      pos += line_len;
      current_row++;

      // Skip spaces at the beginning of wrapped lines
      while (pos < msg_len && display_buffer[pos] == ' ') {
        pos++;
      }
    }

    // Add a blank line between messages if there's room
    if (current_row < (int)rows) {
      current_row++;
    }
  }

  // Reset color to default
  ncplane_set_fg_rgb8(ui->conversation_plane, 0xFF, 0xFF, 0xFF);
}

// Check if input is an exit command
bool is_exit_command(const char *input) {
  if (input == NULL)
    return false;

  // Trim whitespace
  while (*input == ' ' || *input == '\t')
    input++;

  return (strcmp(input, "/quit") == 0);
}

// Read user input with notcurses
char *read_user_input_nc(InteractiveUI *ui) {
  static char input_buffer[1024];
  int buffer_pos = 0;
  ncinput ni;

  // Clear input buffer
  memset(input_buffer, 0, sizeof(input_buffer));

  // Clear input plane and show prompt
  ncplane_erase(ui->input_plane);
  ncplane_putstr(ui->input_plane, "You: ");
  notcurses_render(ui->nc);

  while (1) {
    // Get input from notcurses with proper input structure
    uint32_t key = notcurses_get_blocking(ui->nc, &ni);

    if (key == (uint32_t)-1) {
      // Error or EOF
      return NULL;
    } 
    
    /* Skip key‑release (and other) events so they don’t echo twice */
    if (ni.evtype != NCTYPE_PRESS && ni.evtype != NCTYPE_REPEAT)
        continue;

    if (key == NCKEY_ENTER) {
      // Enter pressed - return the input
      input_buffer[buffer_pos] = '\0';
      return strdup(input_buffer);
    } else if (key == NCKEY_BACKSPACE) {
      // Backspace - remove last character
      if (buffer_pos > 0) {
        buffer_pos--;
        input_buffer[buffer_pos] = '\0';

        // Update display
        ncplane_erase(ui->input_plane);
        ncplane_putstr(ui->input_plane, "You: ");
        ncplane_putstr(ui->input_plane, input_buffer);
        notcurses_render(ui->nc);
      }
    } else if (key == 3) // Ctrl+C
    {
      return NULL;
    } else if (key == 4) // Ctrl+D
    {
      return NULL;
    } else if (key >= 32 && key <= 126 &&
               buffer_pos < (int)sizeof(input_buffer) - 1) {
      // Regular printable character - use the key directly, not ni.utf8
      input_buffer[buffer_pos] = (char)key;
      buffer_pos++;

      // Update display - clear and redraw the entire input line
      ncplane_erase(ui->input_plane);
      ncplane_putstr(ui->input_plane, "You: ");
      ncplane_putstr(ui->input_plane, input_buffer);
      notcurses_render(ui->nc);
    }
  }
}

// Display status message in the status plane
void display_status(InteractiveUI *ui, const char *status) {
  // Clear the status plane
  ncplane_erase(ui->status_plane);

  // Set status color (gray)
  ncplane_set_fg_rgb8(ui->status_plane, 0x88, 0x88, 0x88);

  // Display the status message
  ncplane_putstr(ui->status_plane, status);

  // Render the update
  notcurses_render(ui->nc);
}

// Main interactive mode loop
int run_interactive_mode(chatty_Options options, bool streaming_mode,
                         const char *initial_message) {
  InteractiveUI ui = {0};
  ConversationHistory history = {0};

  // Initialize conversation history
  init_conversation_history(&history);

  // Initialize interactive UI
  if (init_interactive_ui(&ui) != 0) {
    cleanup_conversation_history(&history);
    return -1;
  }

  // Handle initial message if provided
  if (initial_message != NULL) {
    // Add initial user message to history
    if (add_message_to_history(&history, CHATTY_USER, initial_message) != 0) {
      cleanup_interactive_ui(&ui);
      cleanup_conversation_history(&history);
      return -1;
    }

    // Display status
    display_status(&ui, "Thinking...");

    // Send initial message to API and add response to history
    enum chatty_ERROR error;

    if (streaming_mode) {
      // Use streaming mode for initial message
      InteractiveStreamContext stream_ctx = {0};
      stream_ctx.ui = &ui;
      stream_ctx.history = &history;
      stream_ctx.current_response = malloc(1024);
      stream_ctx.response_capacity = 1024;
      stream_ctx.response_length = 0;

      if (stream_ctx.current_response == NULL) {
        cleanup_interactive_ui(&ui);
        cleanup_conversation_history(&history);
        return -1;
      }

      stream_ctx.current_response[0] = '\0';

      error = chatty_chat_stream(history.count, history.messages, options,
                                 interactive_stream_callback, &stream_ctx);

      if (error != CHATTY_SUCCESS) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Error: %s",
                 chatty_error_string(error));
        display_status(&ui, error_msg);

        // Continue with conversation even if API call fails
        if (add_message_to_history(
                &history, CHATTY_ASSISTANT,
                "Sorry, I encountered an error processing your request.") !=
            0) {
          free(stream_ctx.current_response);
          cleanup_interactive_ui(&ui);
          cleanup_conversation_history(&history);
          return -1;
        }
      } else {
        // Add final streaming response to history
        if (add_message_to_history(&history, CHATTY_ASSISTANT,
                                   stream_ctx.current_response) != 0) {
          free(stream_ctx.current_response);
          cleanup_interactive_ui(&ui);
          cleanup_conversation_history(&history);
          return -1;
        }
      }

      free(stream_ctx.current_response);
    } else {
      // Use non-streaming mode for initial message
      chatty_Message response;
      error = chatty_chat(history.count, history.messages, options, &response);

      if (error != CHATTY_SUCCESS) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Error: %s",
                 chatty_error_string(error));
        display_status(&ui, error_msg);

        // Continue with conversation even if API call fails
        if (add_message_to_history(
                &history, CHATTY_ASSISTANT,
                "Sorry, I encountered an error processing your request.") !=
            0) {
          cleanup_interactive_ui(&ui);
          cleanup_conversation_history(&history);
          return -1;
        }
      } else {
        // Add successful response to history
        if (add_message_to_history(&history, CHATTY_ASSISTANT,
                                   response.message) != 0) {
          free(response.message);
          cleanup_interactive_ui(&ui);
          cleanup_conversation_history(&history);
          return -1;
        }
        free(response.message);
      }
    }

    // Update display
    display_conversation(&ui, &history);
    display_status(&ui, "Ready");
  }

  // Main interactive loop
  while (1) {
    // Read user input
    char *user_input = read_user_input_nc(&ui);

    if (user_input == NULL) {
      // EOF or Ctrl+C - exit gracefully
      break;
    }

    // Check for exit commands
    if (is_exit_command(user_input)) {
      free(user_input);
      break;
    }

    // Skip empty input
    if (strlen(user_input) == 0) {
      free(user_input);
      continue;
    }

    // Add user message to history
    if (add_message_to_history(&history, CHATTY_USER, user_input) != 0) {
      free(user_input);
      display_status(&ui, "Error: Failed to add message to history");
      continue;
    }

    // Update display with user message
    display_conversation(&ui, &history);
    display_status(&ui, "Thinking...");

    // Send message to API and add response to history
    enum chatty_ERROR error;

    if (streaming_mode) {
      // Use streaming mode
      InteractiveStreamContext stream_ctx = {0};
      stream_ctx.ui = &ui;
      stream_ctx.history = &history;
      stream_ctx.current_response = malloc(1024);
      stream_ctx.response_capacity = 1024;
      stream_ctx.response_length = 0;

      if (stream_ctx.current_response == NULL) {
        free(user_input);
        display_status(&ui, "Error: Failed to allocate memory for streaming");
        continue;
      }

      stream_ctx.current_response[0] = '\0';

      error = chatty_chat_stream(history.count, history.messages, options,
                                 interactive_stream_callback, &stream_ctx);

      if (error != CHATTY_SUCCESS) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Error: %s",
                 chatty_error_string(error));
        display_status(&ui, error_msg);

        // Continue with conversation even if API call fails
        if (add_message_to_history(
                &history, CHATTY_ASSISTANT,
                "Sorry, I encountered an error processing your request.") !=
            0) {
          free(stream_ctx.current_response);
          free(user_input);
          display_status(&ui, "Error: Failed to add response to history");
          continue;
        }
      } else {
        // Add final streaming response to history
        if (add_message_to_history(&history, CHATTY_ASSISTANT,
                                   stream_ctx.current_response) != 0) {
          free(stream_ctx.current_response);
          free(user_input);
          display_status(&ui, "Error: Failed to add response to history");
          continue;
        }
      }

      free(stream_ctx.current_response);
    } else {
      // Use non-streaming mode
      chatty_Message response;
      error = chatty_chat(history.count, history.messages, options, &response);

      if (error != CHATTY_SUCCESS) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Error: %s",
                 chatty_error_string(error));
        display_status(&ui, error_msg);

        // Continue with conversation even if API call fails
        if (add_message_to_history(
                &history, CHATTY_ASSISTANT,
                "Sorry, I encountered an error processing your request.") !=
            0) {
          free(user_input);
          display_status(&ui, "Error: Failed to add response to history");
          continue;
        }
      } else {
        // Add successful response to history
        if (add_message_to_history(&history, CHATTY_ASSISTANT,
                                   response.message) != 0) {
          free(response.message);
          free(user_input);
          display_status(&ui, "Error: Failed to add response to history");
          continue;
        }
        free(response.message);
      }
    }

    // Update display with response
    display_conversation(&ui, &history);
    display_status(&ui, "Ready");

    free(user_input);
  }

  // Cleanup
  cleanup_interactive_ui(&ui);
  cleanup_conversation_history(&history);

  return 0;
}
#endif

// Streaming callback function that prints tokens in real-time
int stream_callback(const char *content, chatty_StreamStatus status,
                    void *user_data) {
  (void)user_data; // Unused parameter

  switch (status) {
  case CHATTY_STREAM_CHUNK:
    if (content && strlen(content) > 0) {
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

void print_usage(const char *program_name) {
  printf("Usage: %s [OPTIONS] [model] [message]\n", program_name);
  printf("       %s [OPTIONS] [model]              (interactive mode)\n",
         program_name);
  printf("       %s                                (interactive mode with "
         "default model)\n",
         program_name);
  printf("Options:\n");
  printf(
      "  -s, --stream    Enable streaming mode for real-time token display\n");
  printf("  -i, --interactive  Send initial message then enter interactive "
         "mode\n");
  printf("  -h, --help      Show this help message\n");
  printf("\nExamples:\n");
  printf("  %s gpt-4o \"Hello, world!\"                    # Single message\n",
         program_name);
  printf("  %s --stream gpt-4o \"Tell me a story\"          # Single message "
         "with streaming\n",
         program_name);
  printf("  %s gpt-4o                                     # Interactive mode\n",
         program_name);
  printf("  %s                                            # Interactive mode "
         "with default model\n",
         program_name);
  printf("  %s -i gpt-4o \"Hello\" # Initial message then interactive\n",
         program_name);
}

int main(int argc, char *argv[]) {
  bool streaming_mode = false;
  bool interactive_mode = false;
  bool initial_then_interactive = false;
  int arg_offset = 1;

  // Parse command-line arguments
  while (arg_offset < argc && argv[arg_offset][0] == '-') {
    if (strcmp(argv[arg_offset], "-h") == 0 ||
        strcmp(argv[arg_offset], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (strcmp(argv[arg_offset], "-s") == 0 ||
               strcmp(argv[arg_offset], "--stream") == 0) {
      streaming_mode = true;
      arg_offset++;
    } else if (strcmp(argv[arg_offset], "-i") == 0 ||
               strcmp(argv[arg_offset], "--interactive") == 0) {
      initial_then_interactive = true;
      arg_offset++;
    } else {
      fprintf(stderr, "Unknown option: %s\n", argv[arg_offset]);
      print_usage(argv[0]);
      return 1;
    }
  }

  chatty_Options options = {0};
  const char *initial_message = NULL;

  // Parse model argument
  if (arg_offset < argc) {
    options.model = argv[arg_offset];
    arg_offset++;
  } else {
    options.model = "gpt-4o";
  }

  // Parse message argument
  if (arg_offset < argc) {
    initial_message = argv[arg_offset];
  }

  // Determine mode based on arguments
  if (initial_then_interactive && initial_message != NULL) {
    // -i flag with initial message: send message then enter interactive mode
    interactive_mode = true;
  } else if (initial_message == NULL) {
    // No message provided: enter interactive mode
    interactive_mode = true;
  } else if (initial_then_interactive && initial_message == NULL) {
    fprintf(stderr, "Error: -i flag requires an initial message\n");
    print_usage(argv[0]);
    return 1;
  }

  if (interactive_mode) {
#ifdef HAVE_NOTCURSES
    // Run interactive mode with notcurses
    return run_interactive_mode(options, streaming_mode, initial_message);
#else
    // Fallback message when notcurses is not available
    printf("Interactive mode requires notcurses library, which is not "
           "available.\n");
    printf("Please install notcurses and rebuild to use interactive mode.\n");
    return 1;
#endif
  } else {
    // Single message mode (existing behavior)
    chatty_Message messages[1];
    messages[0].role = CHATTY_USER;
    messages[0].message = (char *)initial_message;

    enum chatty_ERROR error;

    if (streaming_mode) {
      // Use streaming mode
      printf("LLM Response (streaming):\n");
      error = chatty_chat_stream(1, messages, options, stream_callback, NULL);

      if (error != CHATTY_SUCCESS) {
        fprintf(stderr, "Streaming error: %s\n", chatty_error_string(error));
        return 1;
      }
    } else {
      // Use traditional non-streaming mode
      chatty_Message response;
      error = chatty_chat(1, messages, options, &response);

      if (error != CHATTY_SUCCESS) {
        fprintf(stderr, "Error: %s\n", chatty_error_string(error));
        return 1;
      }

      printf("LLM Response:\n%s\n", response.message);
      free(response.message);
    }
  }

  return 0;
}