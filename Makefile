build: main.o compressedio.o
	gcc -lz -g  main.o compressedio.o -o exec.bin
%.o: %.c
	gcc -c -g $<
clean: 
	rm *.o exec.bin
