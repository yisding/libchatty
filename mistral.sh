#!/bin/zsh

for i in {1..10}
do
    OPENAI_API_BASE="https://api.mistral.ai/v1" build/chatty mistral-large-2407
done
