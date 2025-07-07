#!/bin/zsh

for i in {1..10}
do
    OPENAI_API_BASE="https://api.llama.com/v1" build/chatty Llama-4-Maverick-17B-128E-Instruct-FP8
done
