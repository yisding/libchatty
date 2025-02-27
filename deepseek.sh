#!/bin/zsh

for i in {1..10}
do
    OPENAI_API_BASE="https://api.deepseek.com" build/chatty deepseek-chat
done
