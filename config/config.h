#ifndef _COMPRESSEDIO_CONFIG_LIB
#define _COMPRESSEDIO_CONFIG_LIB

#define DEFAULT_BLOCK_SIZE 16384
#define DEFAULT_PACKING_MODE CSEALING_MODE
#define DEFAULT_TO_SPARSE 1
#define DEFAULT_FLASH_METHOD CDUMMY_FLASH
#define DEFAULT_REPACKING_TRESHOLD 25.0

typedef struct cCONFIG {
	size_t block_size; 
	char  packing_mode; // sealing or gapping
 	int to_sparse; // fill unused space zero or not
	double sealing_treshold; // treshold for repacking blocks
	char packing_method; // strategy of sealing blocks with new tmp file or inside file
	char flash_method; // strategy of writting new blocks to file: first suitable, optimized (min suitable gap in the file), at the end of the file
	size_t cache_size;
} cCONFIG;

#endif
