build: main.o compressedio.o
	gcc -lz main.o compressedio.o -o exec.bin
%.o: %.c
	gcc -c $<
clean: 
	rm *.o exec.bin
