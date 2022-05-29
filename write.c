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
	cFILE* file = cfopen("/tmp/cfile", NULL, "w");
	set_compressor(&compressor, file);
	char* buf = "Some text to be written to cfile";
	cfseek(file, 0, SEEK_END);
	size_t res = cfwrite((void*)buf, 1, strlen(buf), file);
	printf("%u\n", res);
	cfclose(file);
}
