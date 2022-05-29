# compressedio

A compression library wrapper with transparent compression and random access. 

## building 
````
gcc -fpic -c compressedio.c -o compressedio.o
gcc -shared compressedio.o -o libcompressedio.so

export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
gcc -L. -lcompressedio main.c -o exec
````
 



