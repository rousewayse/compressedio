#ifndef _COMPRESSEDIO_TABLE_LIB
#define _COMPRESSEDIO_TABLE_LIB

typedef struct cBTABLE{
	size_t filled_count; 
	size_t size; // amount of block stored in file
	size_t alloc_size; // table size allocated in mem
	size_t fpos; // table position in file
	cBLOCK** blocks; // array of blocks 

} cBTABLE;

typedef struct cFBTABLE{
	size_t filled_count;
	size_t size;
	size_t alloc_size;
	size_t fpos;
	cFBLOCK** blocks;
} cFBTABLE;

#endif
