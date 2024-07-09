#!/bin/zsh

#export LDFLAGS="-L/opt/homebrew/opt/curl/lib"
#export CPPFLAGS="-I/opt/homebrew/opt/curl/include"
#export PKG_CONFIG_PATH="/opt/homebrew/opt/curl/lib/pkgconfig"

clang -c chatty.c cJSON.c
ar rcs libchatty.a chatty.o cJSON.o
clang -L. -lcurl -lchatty main.c -o chatty

# If you get an error it's because you forgot to brew install curl
# Or maybe you're using a different laptop.