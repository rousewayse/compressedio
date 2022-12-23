/*! \file compressedio.c */

#include "compressedio.h"
#include <string.h>
#include <stdio.h>

/*! \brief Structure representing a compressed/uncompressed data block.
 *
 *  Block's uncompressed data is a slice of uncompressed file starting at \ref mapped_fpos
 */
struct cBLOCK{
	size_t size;//!< \brief Uncompressed block's data size less or equal than base_block_size.
	size_t csize;//!< \brief Compressed data size in bytes
	size_t fpos;//!< \brief Compressed block position stored in storage file
	size_t mapped_fpos;//!< \brief Uncompressed file position asociated with this block's uncompressed data.
	void* data; //!< \brief Pointer to blocks uncompressed data.
				//!
				//! NULL value means that this block's data is not loaded yet.
				//! \warning \ref data is initialized by library inner functions and should never be modified by user directly
	//flag showing if blocks data were modified and block is needed to be rewritten to storage file
	char data_modified;//!< \brief Uncompressed block's data size less or equal than \ref base_block_size.
	//Pointer to cache node, NULL if block is not cached
	struct cQNode* cache_node;//!< \brief Pointer to cache node.
							  //!
							  //!< NULL value means that this block is not predent in \ref cCACHE

};

/*! \brief Structure representing a free block.
 */
struct cFBLOCK{
	size_t size;//!< \brief Size of free block.
	size_t fpos;//!< \brief Position of free block in storage file.
};

/*! \brief Structure representing a free blocks table. 
 */
struct cFBTABLE{
	size_t filled_count;
	size_t size;
	size_t allocated_size;
	size_t fpos;
	struct cFBLOCK** fblocks;
};

/*! \brief Structure representing a blocks table.
 */
struct cBTABLE{
	size_t filled_count;
	size_t size;
	size_t allocated_size;
	size_t fpos;
	struct cBLOCK** blocks;
};


enum cCONTAINER_TYPE{
	cBLOCK_SORTED_ARRAY,
	cBTREE,
	cMIXED,
};

struct cBTREE_NODE{
	char is_leaf;
	size_t keys_count;
	size_t* keys;
	struct cBTREE_NODE** childrens;
	struct cBLOCK* block;
};
struct cBTREE{
	struct cBTREE_NODE* root;
};


/*!\ingroup cfile 
 * \brief Structure representing a file blocks in runtime.*/
struct cBLOCKS_CONTAINER{
	enum cCONTAINER_TYPE container_type;
	//pointer to container instance 
	void* container;
	struct cBTABLE* btable;
	struct CBTREE* btree;
};

struct cQNode {
	struct cBLOCK* block;
	char in_lower;
	struct QNode* next;
	struct QNode* prev;
};


struct cQueue{
	size_t size;
	size_t filled_count;
	struct cQNode* front;
	struct cQNode* rear;
};

/*! \brief Structure representing a LRU blocks cache.
 */
struct cCACHE {
	struct cQueue* lower;//!< \brief Pointer to lower cache queue.
	struct cQueue* upper;//!< \brief Pointer to upper cache queue.
};


// Structure representing compressed file (stream) service information
struct cFILE_HEADER{
	//Position of compressed file header in storage file (archive)
	size_t cfile_header_fpos;
	//Position of blocks table of file (stream) in storage file (archive)
	size_t blocks_table_fpos;
	//Pointer to blocks table instance
	struct cBTABLE* blocks_table;
};
/*! \brief Structure representing a [compressed files](\ref cFILE) in [storage](\ref cSTORAGE).*/
struct cFILES{
	size_t files_count;
	//struct cFILE_INFO* files_infoes;
	//...
};

/*!\ingroup cstorage 
 * \brief Structure representing a storage header with information about storage file.
 */
struct cSTORAGE_HEADER{
	size_t fblocks_table_fpos;//!< \brief Position of free blocks table in storage file. 
	size_t config_fpos;//!< \brief Position of [configuration structure](\ref cCONFIG) in storage file.
	size_t served_fpos;
	size_t cdata_size;//!< \brief Size of all compressed blocks.
	size_t free_size;//!< \brief Size of all free blocks. 

	size_t fully_filled_blocks_count;//!< \brief Amount of fully filled uncompressed blocks.
	size_t partially_filled_blocks_count;//!< \brief Amount of partially filled uncompressed blocks.	
	struct CFBTALBE* free_block_table;//!< \brief Pointer to [free blocks table](\ref cFBTABLE) instance.
	struct cFILES* files;//!< \brief Pointer to [files](\ref cFILES) stored in storage file.
};

/*!\ingroup cstorage 
 * \brief Structure representing a opened storage file.*/
struct cSTORAGE{
	FILE* storage_file;//!< \brief Instance of stdio.h opened stream.
	char* filename;//!< \brief File name of a storage.
	struct cSTORAGE_HEADER* storage_header;//!< \brief Pointer to [storage header](\ref cSTORAGE_HEADER) structure instance.

	struct cCACHE* cache;//!< \brief Pointer to [blocks cache](\ref cCACHE) instance.
	struct cCONFIG* storage_cfg;//!< \brief Pointer to [storage configuration](\ref cCONFIG) instance.	
	void* COMPRESSION_BUFFER;//!< \brief Pointer to compression/decompression buffer
							 // \warning Memory under this pointer is allocated and freed by means of the library inner functions.	
	struct cFILES* stored_files;//!< \brief Pointer to [cFILES](\ref cFILES) instance.  
	//Pointer to free blocks table instance
	struct cFBTABLE* free_blocks_table;//!< \brief Pointer to [free blocks table](\ref cFBTABLE) instance
};

/*!\ingroup cfile 
 * \brief Structure representing a opened file (stream) in srotage.*/
struct cFILE{
	
	char* stream_name;//!< \brief File (stream) stored in [storage](\ref cSTORAGE) C-string name.
	int stream_mode;//!< \brief File mode, i.e. mode passed to [cfopen](\ref cfopen) function.
	struct cSTORAGE* storage;//!< \brief  Pointer to [storage](\ref cSTORAGE) instance.	
	size_t stream_pos;//!< \brief Current file (stream) position.	
	struct cBLOCKS_CONTAINER* blocks_container;//!< \brief Pointer to structure instance representing [file compressed blocks](\ref cBLOCKS_CONTAINER).
	struct cFILE_HEADER* stream_header;//!< \brief Pointer to [compressed file header](\ref cFILE_HEADER) instance.
};


