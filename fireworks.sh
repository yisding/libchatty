#!/bin/zsh

for i in {1..10}
do
    OPENAI_API_BASE="https://api.fireworks.ai/inference/v1" ./chatty accounts/fireworks/models/llama-v3p1-405b-instruct
done
