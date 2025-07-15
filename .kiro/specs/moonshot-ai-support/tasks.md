# Implementation Plan

- [x] 1. Add moonshot.ai provider detection to chatty.c
  - Modify the provider detection logic in `chatty_chat()` function to include moonshot.ai URL matching
  - Add the moonshot.ai base URL check and set `MOONSHOT_API_KEY` as the key environment variable
  - Ensure the new detection follows the same pattern as existing providers
  - _Requirements: 1.1, 1.2, 3.1, 3.2_

- [x] 2. Create moonshot.sh benchmark script
  - Write a new shell script following the established pattern of other provider scripts
  - Set the correct `OPENAI_API_BASE` environment variable to moonshot.ai's API endpoint
  - Configure the script to run 10 benchmark iterations using the `kimi-k2-instruct` model
  - Use zsh shebang and follow the same structure as groq.sh, fireworks.sh, etc.
  - _Requirements: 2.1, 2.2, 2.3_

- [x] 3. Test moonshot.ai integration functionality
  - Write a simple test to verify the provider detection works correctly
  - Test that the correct API key environment variable is selected when moonshot.ai base URL is used
  - Verify that the HTTP request is constructed properly with the moonshot.ai endpoint
  - _Requirements: 1.1, 1.2, 1.3, 3.1_

- [x] 4. Validate error handling with moonshot.ai
  - Test error scenarios like missing API key, invalid API key, and network failures
  - Ensure existing error codes are returned appropriately for moonshot.ai requests
  - Verify that error messages are consistent with other providers
  - _Requirements: 1.4, 3.3_