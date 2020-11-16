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

typedef struct cBLOCK {
    void* DATA;
    size_t _SIZE;
    size_t _CSIZE;
    size_t _FPOS;
    struct cBLOCK* _NEXT_BLOCK;
} cBLOCK;

/*
 * EVERY block of compressed data should store his position in compressed file. 
 * FPOS is block position in bytes
 */

typedef  struct cBLOCKS {
    cBLOCK* _HEAD;
    cBLOCK* _CURR_BLOCK;
    size_t _BLOCKS_AMOUNT;
} cBLOCKS;

    
typedef struct cFILE {
    FILE* _FILE;
    const char*  _MODE;
    cBLOCKS* _BLOCKS;            
} cFILE;
//BLOCKS is a list of compressed data blocks containing in FILE;

#define ARW "a+b"
#define R "r+b"
/*MODES:
 *ARW  
 *R
 */

int cfopen (const char* CFILENAME, const char* FILE_MODE, cFILE* CFILE );
//opens CFILENAME as compressed file in FILE_MODE mode
//return 0 if succseeded


int cfclose (cFILE* cFILE);
//closes CFILE, writes buffer if it's possible and frees allocated memory.
//return 0 if succseeded


int loadCurBlockData ( cFILE* CFILE);
//loads _CURR_BLOCK from a compressed file
//return 0 if succseeded

int insertBlock (void* DATA, size_t DATA_SIZE, cFILE* CFILE);
//Inserts new block with DATA after _CURR_BLOCK and updates other blocks service data 
//returns 0 of succseeded

int removeCurrBlock (cFILE* CFILE);
//removes curr block and updates other blocks service data
//return 0 if succseeded

int writeBlocks(cFILE* CFILE);
//writes all blocks data to the disks
//returns 0 if succseeded

int mergeBlocks (size_t start, size_t end, cFILE* CFILE);
//merges blocks from start to end - 1, and updates blocks service data

#ifdef __cplusplus
}
#endif
#endif
//END OF COMPRESSEDIO_LIB_H

