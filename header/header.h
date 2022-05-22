#ifndef _COMPRESSEDIO_HEADER_LIB
#define _COMPRESSEDIO_HEADER_LIB

typedef struct cHEADER{
	size_t block_size;
	size_t blocks_table_fpos;
	size_t fblocks_table_fpos;
	//last byte in file that is managed
	//needed for writing blocks
	size_t last_served_fpos;
	//compressed data size
	size_t cdata_size;
	//free spaces size
	size_t free_size;
} cHEADER;

#endif 
