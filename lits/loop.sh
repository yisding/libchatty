#!/bin/zsh

for i in {1..10}
do
    NODE_ENV=production node index.mjs
done
