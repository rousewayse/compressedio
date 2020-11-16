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
        return MODE_ERROR;
    }

    if (CFILE != NULL) {
        return CFILE_NOT_NULL;
    }

    FILE* fin = fopen (CFILENAME, FILE_MODE);
    if (fin == NULL) {
        printf("Error when open file;\n");
        return FOPEN_ERROR;
    }

    CFILE = malloc(sizeof(cFILE));
    CFILE->_FILE = fin;
    CFILE->_MODE = FILE_MODE;
    CFILE->_BLOCKS = getBlocks(fin);

    return FOPEN_OK;
}

void deleteBlocks(cFILE* CFILE){
    while (CFILE->_BLOCKS->_CURR_BLOCK != NULL){
        cBLOCK* tmp = CFILE->_BLOCKS->_CURR_BLOCK;
        CFILE->_BLOCKS->_CURR_BLOCK = CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK;
        if (tmp->DATA != NULL) free (tmp->DATA);
        free(tmp);
    }
}

int cfclose(cFILE* CFILE, int IF_CBURN){
    if (CFILE == NULL) {
        return CFILE_IS_NULL;
    }
    if(IF_CBURN != CBURN || IF_CBURN != NOT_CBURN) {
        return BRUN_ERROR;
    }
    if (IF_BURN == CBURN) {
        writeBlocks(CFILE);
    }
//NEEDED TO SAVE BEFORE CLOSING IF CBURN!!!!
    deleteBlocks(CFILE);
    free(CFILE->_BLOCKS);
    free(CFILE);
    CFILE = NULL;
    return FCLOSE_OK;
}

int loadCurrBlockData(cFILE* CFILE){
       if (CFILE->_BLOCKS->_CURR_BLOCK ==  NULL){
        //free(CFILE->_BLOCKS->_CURR_BLOCK->DATA);
        return NULL_BLOCK_ERROR;
    }
    
    if (CFILE->_BLOCKS->_CURR_BLOCK->DATA != NULL){
        return BLOCK_NOT_EMPTY_ERROR;
    }

    if (CFILE->_BLOCKS->_CURR_BLOCK->_CSIZE == 0 ){
        return NEW_BLOCK_LOAD_ERROR;
    }

    fseek(CFILE->_FILE, CFILE->_BLOCKS->_CURR_BLOCK->_FPOS, SEEK_SET);

    CFILE->_BLOCKS->_CURR_BLOCK->DATA = malloc(CFILE->_BLOCKS->_CURR_BLOCK->_SIZE);
    void* BUFF = malloc (CFILE->_BLOCKS->_CURR_BLOCK->_CSIZE);
    
    fread(BUFF, 1, CFILE->_BLOCKS->_CURR_BLOCK->_CSIZE, CFILE->_FILE);
    
    int res = uncompress(CFILE->_BLOCKS->_CURR_BLOCK->DATA, &CFILE->_BLOCKS->_CURR_BLOCK->_SIZE, BUFF, CFILE->_BLOCKS->_CURR_BLOCK->_CSIZE);
    free(BUFF);
    return res; // Z_OK, Z_MEM_ERROR, Z_BUFF_ERROR, Z_DATA_ERROR
}


cBLOCK* _createBlock (void* DATA, size_t DATA_SIZE){
    cBLOCK* block = malloc(sizeof(cBLOCK);
    block->DATA = DATA;
    block->_SIZE=DATA_SIZE;
    block->_CSIZE=0;//means that block has been added by user, but haven't been written to the file yet
    DATA = NULL;    
    block->_NEXT_BLOCK = NULL;
}

int insertBlock (void* DATA, size_t DATA_SIZE, cFILE* CFILE){
    if(CFILE != NULL && CFILE->_BLOCKS != NULL){
        if (CFILE->_BLOCKS->_HEAD  == NULL){
            CFILE->_BLOCKS->_HEAD = _createBlock(DATA, DATA_SIZE);
            CFILE->_BLOCKS->_CURR_BLOCK = CFILE->_BLOCKS->_HEAD;    
        }
        cBLOCK* tmp_blk = CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK;
        CFILE->_BLOKS->_CURR_BLOCK->_NEXT_BLOCK = _createBlock(DATA, DATA_SIZE);
        CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK->_NEXT_BLOCK = tmp_blk;
        return INSERT_OK;
    }
    else{
        return CFILE_ERROR;
    }
}



int removeCurrBlock(cFILE* CFILE){
    if (CFILE != NULL && CFILE->_BLOCKS != NULL){
        if (CFILE->_BLOCKS->_CURR_BLOCK != NULL){

            cBLOCK* tmp = CFILE->_BLOCKS->_HEAD;
            if (tmp == CFILE->_BLOCKS->_CURR_BLOCK){
                CFILE->_BLOCKS->_HEAD = tmp->_NEXT_BLOCK;
                CFILE->_BLOCKS->_CURR_BLOCK = CFILE->_BLOCKS->_HEAD;
                free(tmp->DATA);
                free(tmp);
                return REMOVE_OK;                
            }

            while (tmp->_NEXT_BLOCK != CFILE->_BLOCKS->_CURR_BLOCK) {
                tmp = tmp->_NEXT_BLOCK;
            }
            tmp->_NEXT_BLOCK = tmp->_NEXT_BLOCK->_NEXT_BLOCK;
            free(CFILE->_BLOCKS->_CURR_BLOCK->DATA);
            free(CFILE->_BLOCKS->_CURR_CLOCK);
            CFILE->_BLOCKS->_CURR_BLOCK = tmp->_NEXT_BLOCK;
            return REMOVE_OK;
        }
        else {
            return NULL_BLOCK_ERROR;
        }
    }
    else {
        return CFILE_ERROR;
    }
}


int writeBlocks (cFILE* CFILE){
    rewind(CFILE->_FILE);
    cBLOCK* tmp = CFILE->_BLOCKS->_HEAD;

    while(tmp != NULL){
        size_t BUFF_SIZE = tmp->_SIZE + tmp->_SIZE/100 + 1 + 12;
        void* BUFF = malloc(BUFF_SIZE);
        int res = compress(BUFF, &BUFF_SIZE, tmp->DATA, tmp->_SIZE);
        if (res == Z_OK) {
            tmp->_CSIZE = BUFF_SIZE;
            fwrite(&tmp->_SIZE, sizeof(size_t), 1, CFILE->_FILE);
            fwrite(&tmp->_CSIZE, sizeof(size_t), 1, CFILE->_FILE);
            tmp->_FPOS = ftell(CFILE->_FILE);
            fwrite(&BUFF, 1, tmp->_CSIZE, CFILE->_FILE);
            free(BUFF);
            tmp = tmp -> _NEXT_BLOCK;
        }
        else {
            return WRITE_CLOCKS_ERROR;
        }
    }
    return WRITE_BLOCKS_OK;

}

int moveNextBlock(cFILE* CFILE){
    if (CFILE != NULL && CFILE->_BLOCKS!=NULL){
        if(CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK != NULL){
            CFILE->_BLOCKS->CURR_BLOCK = CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK;
        }
        else {
            return CANNOT_MOVE;
        }
    }
    else {
        return CFILE_ERROR;
    }
}

int movePrevBlock(cFILE* CFILE){
    if (CFILE != NULL && CFILE->_BLOCKS!=NULL){
        if(CFILE->_BLOCKS->_CURR_BLOCK == CFILE->_BLOCKS->_HEAD)
            return CANNOT_MOVE;
         cBLOCK* tmp = CFILE->_BLOCKS->_HEAD;
         while(tmp->_NEXT_BLOCK != CFILE->_BLOCKS->_CURR_BLOCK){
             tmp->_NEXT_BLOCK = tmp->_NEXT_BLOCK->_NEXT_BLOCK;
         }
         CFILE->_BLOCKS->_CURR_BLOCK = tmp;
    }
    else {
        return CFILE_ERROR;
    }
}
