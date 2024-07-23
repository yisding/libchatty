#!/bin/zsh

for i in {1..10}
do
    OPENAI_API_BASE="https://api.groq.com/openai/v1" ./chatty llama-3.1-8b-instant
done
