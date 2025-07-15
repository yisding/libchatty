#!/bin/zsh

for i in {1..10}
do
    OPENAI_API_BASE="https://api.moonshot.ai/v1" build/chatty kimi-k2-instruct
done