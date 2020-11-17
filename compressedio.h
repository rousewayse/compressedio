/*
 This library implements compressed input/output library. All operations are provided with decompressed data blocks if they are loaded from file; So user should check whether block  loaded and load it if nedeed.
*/

#ifndef _COMPRESSEDIO_LIB_H
#define COMPRESSEDIO_LIB_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#define DEFAULT_BLOCK_SIZE 16384
#define CBURN 1 
//write file before closing 
#define NOT_CBURN 0
//do not write file before closing - just close
#define FILE_MODE "r+b"

typedef struct cBLOCK {
    void* DATA;
    uLongf _SIZE;
    uLongf _CSIZE;
    uLongf _FPOS;
    struct cBLOCK* _NEXT_BLOCK;
} cBLOCK;

/*
 * EVERY block of compressed data should store his position in compressed file. 
 * FPOS is block position in bytes
 */

typedef  struct cBLOCKS {
    cBLOCK* _HEAD;
    cBLOCK* _CURR_BLOCK;
} cBLOCKS;

    
typedef struct cFILE {
    FILE* _FILE;
    const char*  _MODE;
    cBLOCKS* _BLOCKS;            
} cFILE;
//BLOCKS is a list of compressed data blocks containing in FILE;

int cfopen (const char* CFILENAME, cFILE** CFILE );
//opens CFILENAME as compressed file in FILE_MODE mode

int cfclose (cFILE* cFILE, int IF_CBURN);
//closes CFILE, writes buffer if it's possible and frees allocated memory.

int loadCurrBlockData ( cFILE* CFILE);
//loads _CURR_BLOCK from a compressed file

int insertBlock (void* DATA, size_t DATA_SIZE, cFILE* CFILE);
//Inserts new block with DATA after _CURR_BLOCK and do not updates other blocks service data 

int removeCurrBlock (cFILE* CFILE);
//removes curr block and updates other blocks service data

int writeBlocks(cFILE* CFILE);
//writes all blocks data to the disks

int moveNextBlock (cFILE* CFILE);

int movePrevBlock (cFILE* CFILE);

void deleteBlocks(cFILE* CFILE);

int   dropCurrBlockData(cFILE* CFILE);

int  getCurrBlockData(cFILE* CFILE, void** DATA);

int  crewind(cFILE* CFILE);

int convertFile(const char*  filename);

#define CFILE_ALREADY_OPENED -1
#define FOPEN_ERROR -2
#define FOPEN_OK 0
//MODE_ERROR - FILE_MODE has wrong value
//CFILE_NOT_NULL - user may be tried to open already initialized CFILE
//FOPEN_ERROR - error happend while open file
//FOPEN_OK - no errors
#define CFILE_IS_NULL -1
#define FCLOSE_OK 0
#define CBRUN_ERROR -2
//CFILE_IS_NULL - user tried to close alredy closed or not initialized CFILE
//FCLOSE_OK - no errors
//CBURN_ERROR - bad IF_CBRUN value
#define BLOCK_NOT_EMPTY_ERROR -3
#define BLOCK_LOAD_OK 0
#define BLOCK_LOAD_ERROR -2 
#define NEW_BLOCK_LOAD_ERROR 1
#define NULL_BLOCK_ERROR -1
#define BLOCK_DATA_NULL_ERROR 2
//NEW_BLOCK_LOAD_ERROR - user tried to load block that is not in file, probably user has inserted this block
//BLOCK_NOT_EMPTY error occurs when user tries loading data in alreaddy loaded block
//LOAD_OK means that loading block data is done without errors
#define CFILE_ERROR -1
#define INSERT_OK 0
#define REMOVE_OK 0
#define WRITE_BLOCKS_ERROR -1
#define WRITE_BLOCKS_OK 0
#define CANNOT_MOVE -1

#ifdef __cplusplus
}
#endif
#endif
//END OF COMPRESSEDIO_LIB_H

