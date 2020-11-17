#include "compressedio.h"
cBLOCK* _getBlock(FILE* fin){
    cBLOCK* block = malloc(sizeof(cBLOCK));
    int res = fread(&block->_SIZE, sizeof(uLongf), 1, fin);
    if(res == 0) return NULL;
    fread(&block->_CSIZE, sizeof(uLongf), 1, fin);
    block->_FPOS=ftell(fin);
    block->DATA = NULL;
    block->_NEXT_BLOCK = NULL;
    fseek(fin, block->_CSIZE, SEEK_CUR);
    return block;
}

cBLOCKS* _getBlocks(FILE* fin){
    cBLOCKS* blocks = malloc(sizeof(cBLOCKS));
    blocks->_HEAD = _getBlock(fin);
    blocks->_CURR_BLOCK = blocks->_HEAD;
    long fpos_prev = 0;
    long fpos = ftell(fin);
    while(fpos!=fpos_prev){
        blocks->_CURR_BLOCK->_NEXT_BLOCK = _getBlock(fin);
        blocks->_CURR_BLOCK = blocks->_CURR_BLOCK->_NEXT_BLOCK;
        fpos_prev = fpos;
        fpos = ftell(fin);
    }
    blocks->_CURR_BLOCK = blocks->_HEAD;

    return blocks;
}

int cfopen (const char* CFILENAME, cFILE** CFILE ){
    if (*CFILE != NULL) {
        return CFILE_ALREADY_OPENED;
    }

    FILE* fin = fopen (CFILENAME, FILE_MODE);
    if (fin == NULL) {
        return FOPEN_ERROR;
    }
    fseek(fin, 0, SEEK_END);
    long pos = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    
    (*CFILE) = malloc(sizeof(cFILE));
    (*CFILE)->_FILE = fin;
    (*CFILE)->_MODE = FILE_MODE;
    if(pos>0){
        (*CFILE)->_BLOCKS = _getBlocks(fin);
        fseek(fin, 0, SEEK_SET);
    } 
    else{
        (*CFILE)->_BLOCKS = malloc(sizeof(cBLOCKS));
        (*CFILE)->_BLOCKS->_HEAD = NULL;
        (*CFILE)->_BLOCKS->_CURR_BLOCK = NULL;
    }
    return FOPEN_OK;
}

void deleteBlocks(cFILE* CFILE){
        CFILE->_BLOCKS->_CURR_BLOCK = CFILE->_BLOCKS->_HEAD;
        while (CFILE->_BLOCKS->_CURR_BLOCK != NULL){
            cBLOCK* tmp = CFILE->_BLOCKS->_CURR_BLOCK;
            CFILE->_BLOCKS->_CURR_BLOCK = CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK;
            if (tmp->DATA != NULL) free (tmp->DATA);
            free(tmp);
        }
}

int cfclose(cFILE* CFILE, int IF_CBURN){
    if (CFILE == NULL || CFILE->_BLOCKS == NULL) {
        return CFILE_ERROR;
    }
    if(IF_CBURN != CBURN || IF_CBURN != NOT_CBURN) {
        return CBRUN_ERROR;
    }
    if (IF_CBURN == CBURN) {
        writeBlocks(CFILE);
    }
    deleteBlocks(CFILE);
    free(CFILE->_BLOCKS);
    
    fclose(CFILE->_FILE);
    free(CFILE);
    CFILE = NULL;
    return FCLOSE_OK;
}

int loadCurrBlockData(cFILE* CFILE){
    if (CFILE == NULL || CFILE->_BLOCKS ==  NULL)
        return CFILE_ERROR;

    if ( CFILE->_BLOCKS->_CURR_BLOCK == NULL){
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
    if(res == 0)
        return BLOCK_LOAD_OK;
    else 
       return BLOCK_LOAD_ERROR; // Z_OK, Z_MEM_ERROR, Z_BUFF_ERROR, Z_DATA_ERROR
}


cBLOCK* _createBlock (void* DATA, size_t DATA_SIZE){
    cBLOCK* block = malloc(sizeof(cBLOCK));
    block->DATA = DATA;
    block->_SIZE = DATA_SIZE;
    block->_CSIZE = 0;//means that block has been added by user, but haven't been written to the file yet
    DATA = NULL;    
    block->_NEXT_BLOCK = NULL;
}

int insertBlock (void* DATA, size_t DATA_SIZE, cFILE* CFILE){
    if(CFILE != NULL && CFILE->_BLOCKS != NULL){

        if (CFILE->_BLOCKS->_HEAD== NULL){
            CFILE->_BLOCKS->_HEAD = _createBlock(DATA, DATA_SIZE);
            CFILE->_BLOCKS->_CURR_BLOCK = CFILE->_BLOCKS->_HEAD;
            return INSERT_OK;    
        } 
        cBLOCK* tmp_blk = CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK;
        CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK = _createBlock(DATA, DATA_SIZE);
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
                if (tmp->DATA != NULL)
                    free(tmp->DATA);
                free(tmp);
                return REMOVE_OK;                
            }

            while (tmp->_NEXT_BLOCK != CFILE->_BLOCKS->_CURR_BLOCK) {
                tmp = tmp->_NEXT_BLOCK;
            }
            tmp->_NEXT_BLOCK = tmp->_NEXT_BLOCK->_NEXT_BLOCK;
            if (CFILE->_BLOCKS->_CURR_BLOCK->DATA != NULL)
                free(CFILE->_BLOCKS->_CURR_BLOCK->DATA);
            free(CFILE->_BLOCKS->_CURR_BLOCK);
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

    if (CFILE == NULL && CFILE->_BLOCKS == NULL)
        return CFILE_ERROR;

    rewind(CFILE->_FILE);
    cBLOCK* tmp = CFILE->_BLOCKS->_HEAD;

    while(tmp != NULL){
            if (tmp->DATA == NULL)
                return NULL_BLOCK_ERROR;
        uLongf BUFF_SIZE = tmp->_SIZE + tmp->_SIZE/100 + 1 + 12;
        void* BUFF = malloc(BUFF_SIZE);
        int res = compress(BUFF, &BUFF_SIZE, tmp->DATA, tmp->_SIZE);
        if (res == Z_OK) {
            tmp->_CSIZE = BUFF_SIZE;
            fwrite(&tmp->_SIZE, sizeof(uLongf), 1, CFILE->_FILE);
            fwrite(&tmp->_CSIZE, sizeof(uLongf), 1, CFILE->_FILE);
            tmp->_FPOS = ftell(CFILE->_FILE);
            fwrite(BUFF, 1, tmp->_CSIZE, CFILE->_FILE);
            free(BUFF);
            tmp = tmp -> _NEXT_BLOCK;
        }
        else {
            return WRITE_BLOCKS_ERROR;
        }
    }
    return WRITE_BLOCKS_OK;

}

int moveNextBlock(cFILE* CFILE){
    if (CFILE != NULL && CFILE->_BLOCKS!=NULL){
        if(CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK != NULL){
            CFILE->_BLOCKS->_CURR_BLOCK = CFILE->_BLOCKS->_CURR_BLOCK->_NEXT_BLOCK;
        }
        else {
            return CANNOT_MOVE;
        }
    }
    else {
        return CFILE_ERROR;
    }
    return 0;
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
    return 0;
}


int  dropCurrBlockData(cFILE* CFILE){
    if (CFILE ==  NULL && CFILE->_BLOCKS == NULL)
        return CFILE_ERROR;

    if(CFILE->_BLOCKS->_CURR_BLOCK == NULL) 
        return NULL_BLOCK_ERROR;

    if (CFILE->_BLOCKS->_CURR_BLOCK->DATA != NULL){ 
        free(CFILE->_BLOCKS->_CURR_BLOCK->DATA);
        CFILE->_BLOCKS->_CURR_BLOCK->DATA = NULL;
        CFILE->_BLOCKS->_CURR_BLOCK->_SIZE = 0;
    }
    else return -1;
    return 0;
}


int  getCurrBlockData(cFILE* CFILE, void** DATA){
    if (CFILE ==  NULL && CFILE->_BLOCKS == NULL)
        return CFILE_ERROR;

    if(CFILE->_BLOCKS->_CURR_BLOCK == NULL)
        return NULL_BLOCK_ERROR;

    if (CFILE->_BLOCKS->_CURR_BLOCK->DATA == NULL)
       return BLOCK_DATA_NULL_ERROR; 

    *DATA = CFILE->_BLOCKS->_CURR_BLOCK->DATA;
    return 0;
}


int  crewind(cFILE* CFILE){
    if (CFILE == NULL)
        return CFILE_ERROR;
    fseek(CFILE->_FILE, 0, SEEK_SET);
        return 0;
}

int convertFile(const char*  filename){
    
    FILE* fin = fopen(filename, "rb");
    FILE* fout = fopen("output_file" ,"wb");
    uLongf BUFF_SIZE = DEFAULT_BLOCK_SIZE;
    void* BUFF = malloc(BUFF_SIZE);
    void* COMPRESSED_BUFF = malloc(BUFF_SIZE + 180);
    uLongf COMPRESSED_BUFF_SIZE = DEFAULT_BLOCK_SIZE + 180;

    while(!feof(fin)){
        uLongf BUFF_SIZE = fread(BUFF, 1, DEFAULT_BLOCK_SIZE, fin);
        int res = compress(COMPRESSED_BUFF, &COMPRESSED_BUFF_SIZE, BUFF, BUFF_SIZE);
        if (!res) {
            fwrite(&BUFF_SIZE, sizeof(uLongf), 1, fout);
            fwrite(&COMPRESSED_BUFF_SIZE, sizeof(uLongf), 1, fout);
            fwrite(COMPRESSED_BUFF, 1, COMPRESSED_BUFF_SIZE, fout);
        }
        else return -1;
    }
    fclose(fin);
    fclose(fout);
    free(BUFF);
    free(COMPRESSED_BUFF);
    return 0;
}
