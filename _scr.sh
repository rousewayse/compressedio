#!/bin/bash

export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
gcc -lz  -fpic -c -g compressedio.c -o compressedio.o && \
    gcc -lz  -shared -g -o libcompressedio.so compressedio.o && \
    gcc -lz -g -L. main.c -lcompressedio -o main
    ./main 
