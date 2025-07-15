# Requirements Document

## Introduction

This feature adds support for moonshot.ai as a new LLM provider to libchatty. Moonshot.ai is a Chinese AI company that provides OpenAI-compatible chat completion APIs. The integration should follow the same pattern as existing providers (Groq, Fireworks, Mistral, etc.) by supporting automatic API key detection and base URL configuration while maintaining the library's performance-first philosophy.

## Requirements

### Requirement 1

**User Story:** As a developer using libchatty, I want to make API calls to moonshot.ai models, so that I can leverage their LLM capabilities in my high-performance applications.

#### Acceptance Criteria

1. WHEN the user sets `OPENAI_API_BASE` to "https://api.moonshot.ai/v1" THEN the system SHALL automatically detect and use the `MOONSHOT_API_KEY` environment variable
2. WHEN the user provides a moonshot.ai model name THEN the system SHALL successfully make chat completion requests to the moonshot.ai API
3. WHEN the API call is successful THEN the system SHALL parse and return the response in the same format as other providers
4. WHEN the API call fails THEN the system SHALL provide appropriate error handling consistent with other providers

### Requirement 2

**User Story:** As a developer, I want a convenient shell script for moonshot.ai benchmarking, so that I can easily test performance and compare with other providers.

#### Acceptance Criteria

1. WHEN the user runs the moonshot.sh script THEN the system SHALL execute 10 benchmark iterations using a moonshot.ai model
2. WHEN the script runs THEN it SHALL automatically set the correct API base URL for moonshot.ai
3. WHEN the script completes THEN it SHALL display timing results consistent with other provider scripts
4. IF no model is specified THEN the script SHALL use a sensible default moonshot.ai model

### Requirement 3

**User Story:** As a developer, I want moonshot.ai integration to follow the same patterns as existing providers, so that the API remains consistent and maintainable.

#### Acceptance Criteria

1. WHEN moonshot.ai support is added THEN it SHALL use the same automatic API key detection pattern as other providers
2. WHEN moonshot.ai is used THEN it SHALL not require any changes to the core chatty API functions
3. WHEN moonshot.ai is integrated THEN it SHALL maintain the same performance characteristics as other providers
4. WHEN moonshot.ai is added THEN the documentation SHALL be updated to reflect the new provider support