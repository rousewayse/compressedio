#ifndef _COMPRESSEDIO_COMPRESSOR_LIB
#define _COMPRESSEDIO_COMPRESSOR_LIB

typedef struct cCOMPRESSOR{
    // int compress (void* dst, size_t* dst_len, void* source, size_t source_len)
	int (*compress)(void*, size_t*, void*, size_t);
	// int uncompress (void* dst, size_t dst_len, void* source, size_t source_len)
	int (*uncompress)(void*, size_t*, void*, size_t);
	// size_t compressBound (size_t source_len)
	size_t (*compressBound)(size_t);
} cCOMPRESSOR;



#endif
