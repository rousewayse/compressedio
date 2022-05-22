#ifndef _COMPRESSEDIO_BLOCKS_LIB
#define _COMPRESSEDIO_BLOCKS_LIB

#define BLOCK_META_SIZE 3*sizeof(size_t)

typedef struct cBLOCK{
	size_t size;
	size_t csize;
	size_t fpos;
	size_t mapped_fpos;
	void* data;
	char data_changed;
	QNode* cache_node;
} cBLOCK;

typedef struct cFBLOCK{
	size_t size;
	size_t fpos;
} cFBLOCK;

#endif
