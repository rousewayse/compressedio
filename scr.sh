#!/bin/bash

export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
gcc -fpic -c compressedio.c -o compressedio.o && \
    gcc -shared -o libcompressedio.so compressedio.o && \
    gcc -L. main.c -lcompressedio
    ./main 


