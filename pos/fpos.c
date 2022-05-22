//#include "fpos.h"

//rewinds file, sets cfile position to 0
void crewind(cFILE* cfile){
	if (cfile != NULL && cfile->file != NULL && cfile->btable != NULL && cfile->btable->blocks != NULL){
		cfile->pos = 0;
		cfile->cur_block = 0;
	}
}


long int cftell(cFILE* cfile){
	if (cfile == NULL || cfile->file == NULL || cfile->btable == NULL ){
		return -1;
	}
	return cfile->pos;
}

int cfseek_SEEK_END(cFILE* cfile, long int offset){
	cCONFIG* config = cfile->config;
	cBTABLE* table  = cfile->btable;
	if(table->filled_count == 0){
		return -1;
	}
	size_t last_block_num = table->filled_count-1;
	int res = 0;
	cBLOCK* block = get_block(cfile, last_block_num, &res);
	size_t last_fpos = 0;
	//last block should not be "hole" 
	if (res == 0 && block != NULL){
		 last_fpos = config->block_size*last_block_num + block->size;
	} else {
		//error, no such block or hole block
		return -1;
	}
	if (offset >= 0){
		cfile->pos = last_fpos + offset;
		cfile->cur_block = cfile->pos/config->block_size;
	} else {
		if (offset*(-1) > last_fpos){
			return -1;
		}
		cfile->pos = last_fpos + offset;
		cfile->cur_block = cfile->pos/config->block_size;
	}
	return 0;
}

int cfseek_SEEK_CUR(cFILE* cfile, long int offset){
	cCONFIG* config = cfile->config;
	if ( offset >= 0){
		cfile->pos += offset;
	} else {
		if (cfile->pos >= (-1)*offset){
			cfile->pos += offset;
		} else {
			return -1;
		}
	}
	size_t block_num = cfile->pos / config->block_size;
	cfile->cur_block = block_num;
	return 0;
}

int cfseek_SEEK_SET(cFILE* cfile, long int offset){
	if (offset < 0){
		return -1;
	}
	cCONFIG* config = cfile->config;

	cfile->pos = offset;
	size_t block_num = cfile->pos / config->block_size;
	cfile->cur_block = block_num;
	return 0;
}


int cfseek(cFILE* cfile, long int offset, int origin){
	if (cfile == NULL || cfile->file == NULL || cfile->btable == NULL) {
		return -1;
	}

	int res = -1;
	switch(origin){
		case CSEEK_SET:
			res = cfseek_SEEK_SET(cfile, offset);	
			break;
		case CSEEK_CUR:
			res = cfseek_SEEK_CUR(cfile, offset);
			break;
		case CSEEK_END:
			res = cfseek_SEEK_END(cfile, offset);
			break;
		deafult:
			res = -1;
			break;
	}
	return res;
}
