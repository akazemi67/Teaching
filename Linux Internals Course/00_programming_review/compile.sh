#!/bin/bash

filename=$1

echo "Compiling $filename"
gcc -m32 -ggdb -mpreferred-stack-boundary=2 -fno-stack-protector -no-pie $filename.c -o $filename.elf


