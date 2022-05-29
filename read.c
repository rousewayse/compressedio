#include <stdio.h>
#include<stdlib.h>
#include"compressedio.h"
#include <zlib.h>
void main(){
	//filling compressor
	cCOMPRESSOR compressor;
	compressor.compress = compress;
	compressor.uncompress = uncompress;
	compressor.compressBound = compressBound;
	// opening file
	cFILE* file = cfopen("/tmp/cfile", NULL, "r+");
	set_compressor(&compressor, file);
	char* buf = malloc(1000);
	size_t res = cfread((void*)buf, 1, 1000, file);
	printf("%s\n", buf);
	cfclose(file);
	free(buf);
}
