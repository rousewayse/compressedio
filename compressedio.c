#include "compressedio.h"
cBLOCK* getBlock(FILE* fin){
    cBLOCK* block = malloc(sizeof(cBLOCK));
    fread(&block->_SIZE, sizeof(size_t), 1, fin);
    fread(&block->_CSIZE, sizeof(size_t), 1, fin);
    block->_FPOS=ftell(fin);
    block->DATA = NULL;
    block->_NEXT_BLOCK = NULL;
    fseek(fin, block->_CSIZE, SEEK_CUR);
    return block;
}

cBLOCKS* getBlocks(FILE* fin){
    
    cBLOCKS* blocks = malloc(sizeof(cBLOCKS));
    blocks->_HEAD = getBlock(fin);
    blocks->_CURR_BLOCK = blocks->_HEAD;
    blocks->_BLOCKS_AMOUNT = 1;
        while(!feof(fin)){
        blocks->_CURR_BLOCK->_NEXT_BLOCK = getBlock(fin);
        blocks->_BLOCKS_AMOUNT++;
    }
    return blocks;
}

int cfopen (const char* CFILENAME, const char* FILE_MODE, cFILE* CFILE ){
    if (FILE_MODE != R || FILE_MODE != ARW) {
        printf("Wrong FILE_MODE;\n");
        return -1;
    }
    FILE* fin = fopen (CFILENAME, FILE_MODE);
    if (fin == NULL) {
        printf("Error when open file;\n");
        return -1;
    }
    
    CFILE = malloc(sizeof(cFILE));
    CFILE->_FILE = fin;
    CFILE->_MODE = FILE_MODE;
    CFILE->_BLOCKS = getBlocks(fin);

    return 0;
}

void deleteBlocks(cFILE* CFILE){
    while (CFILE->_BLOCKS->_CURR_BLOCK != NULL){
        cBLOCK* tmp = CFILE->_BLOCKS->_CURR_BLOCK;
        CFILE->_BLOCKS->_CURR_BLOCK = CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK;
        if (tmp->DATA != NULL) free (tmp->DATA);
        free(tmp);
    }
}

int cfclose(cFILE* CFILE){
    deleteBlocks(CFILE);
    free(CFILE->_BLOCKS);
    free(CFILE);
    return 0;
}

int loadCurrBlockData(cFILE* CFILE){
    fseek(CFILE->_FILE, CFILE->_BLOCKS->_CURR_BLOCK->_FPOS, SEEK_SET);
    if (CFILE->_BLOCKS->_CURR_BLOCK != NULL){
        free(CFILE->_BLOCKS->_CURR_BLOCK->DATA);
    }
    CFILE->_BLOCKS->_CURR_BLOCK->DATA = malloc(CFILE->_BLOCKS->_CURR_BLOCK->_CSIZE);
    fread(CFILE->_BLOCKS->_CURR_BLOCK->DATA, 1, CFILE->_BLOCKS->_CURR_BLOCK->_CSIZE, CFILE->_FILE);
    return 0;
}



