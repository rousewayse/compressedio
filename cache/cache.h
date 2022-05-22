#ifndef _COMPRESSEDIO_CACHE_LIB
#define _COMPRESSEDIO_CACHE_LIB

typedef struct QNode {
	cBLOCK* block;
	int in_lower;
	struct QNode* next;
    struct QNode* prev;
} QNode;

typedef struct Queue{
	size_t size;
	size_t filled_count;
	QNode* front;
   	QNode* rear;
} Queue;

typedef struct cCACHE{
	//Hash* hash;
	Queue* lower;
   	Queue* upper;
} cCACHE;

#endif
