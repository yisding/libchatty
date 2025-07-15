#!/bin/zsh

echo "=== libchatty Streaming Test ==="
echo

# Test 1: Basic streaming functionality
echo "Test 1: Basic streaming with OpenAI (requires OPENAI_API_KEY)"
if [[ -n "$OPENAI_API_KEY" ]]; then
    echo "Running streaming test..."
    ./stream_test gpt-4o-mini "Count from 1 to 5, one number per line."
    echo
else
    echo "Skipped - OPENAI_API_KEY not set"
    echo
fi

# Test 2: Test with Groq if available
echo "Test 2: Streaming with Groq (requires GROQ_API_KEY)"
if [[ -n "$GROQ_API_KEY" ]]; then
    echo "Running Groq streaming test..."
    OPENAI_API_BASE="https://api.groq.com/openai/v1" ./stream_test llama-3.1-8b-instant "Write a haiku about code."
    echo
else
    echo "Skipped - GROQ_API_KEY not set"
    echo
fi

# Test 3: Error handling test (invalid API key)
echo "Test 3: Error handling with invalid API key"
OPENAI_API_KEY="invalid_key" ./stream_test gpt-4o-mini "Hello"
echo

echo "=== Streaming tests completed ==="