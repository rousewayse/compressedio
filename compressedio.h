
#ifndef _COMPRESSEDIO_LIB_H
#define _COMPRESSEDIO_LIB_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define CSEALING_MODE 'S'
#define CGAPING_MODE 'G'

typedef struct cBLOCK cBLOCK;
#include "cache/cache.h"
#include "blocks/blocks.h"
#include "table/table.h"
#include "config/config.h"
#include "header/header.h"
#include "compressor/compressor.h"
#include "flash/flash.h"
#include "pos/fpos.h"
#include "read_write/read_write.h"
#include "seal/seal.h"
typedef struct cFILE{
	size_t pos;

	FILE* file;
	char* filename;
	size_t cur_block;
 	cCONFIG* config;
	cCACHE* cache;
	cCOMPRESSOR* compressor;

	cHEADER* header;
	cBTABLE* btable;
	cFBTABLE* fbtable;
//	cBLOCK** cblocks;
//	cBLOCK** cfblocks;
} cFILE;

/**
 *\brief reading function
 *	 this function read data
 *	\param ptr is pointer
 *	\param y the second
 *	\param size is the size
 *	\return returns size_t
 */

size_t cfread(void* ptr, size_t size, size_t nmemb, cFILE* cfile);

size_t cfwrite(void* ptr, size_t size, size_t nmemb, cFILE* cfile);
cFILE* cfopen(const char* filename, cCONFIG* user_config);
int load_block_data(cFILE* cfile, cBLOCK* block);
int set_compressor(cCOMPRESSOR* compressor, cFILE* cfile);
int cfseek(cFILE* cfile, long int offset, int origin);
int cfclose(cFILE* cfile);
cCONFIG* get_default_cconfig();
double overusage_persent(cFILE* cfile);
int convertFile(const char* filename, const char* output_filename, size_t block_size, cCOMPRESSOR* compressor);
int sparse(cFILE* cfile, size_t start, size_t stop);
cBLOCK* get_block(cFILE* cfile, size_t num, int* res);
int cfsync(cFILE* cfile);
#endif
// end of COMPRESSEDIO_H
