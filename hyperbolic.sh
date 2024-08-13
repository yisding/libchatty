#!/bin/zsh

for i in {1..10}
do
    OPENAI_API_BASE="https://api.hyperbolic.xyz/v1" build/chatty meta-llama/Meta-Llama-3.1-405B-Instruct
done
