#!/bin/zsh

clang -c chatty.c cJSON.c
ar rcs libchatty.a chatty.o cJSON.o
clang -L. -lcurl -lchatty main.c -o chatty

# If you get an error it's because you forgot to brew install curl
# Or maybe you're using a different laptop.