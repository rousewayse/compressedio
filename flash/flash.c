//#include "flash.h"


int coptimized_flash(cFILE* cfile, char mode){
	int res = 0;
	size_t block_num = cfile->cur_block;
	cBLOCK* block = get_block(cfile, block_num, &res);
	cCOMPRESSOR* compressor = cfile->compressor;
	int compressor_valid = validate_compressor(compressor);
	if (compressor_valid != 0){
		return -1;
	}
	if (block->data_changed == 0){
		return 0;
	}
	size_t block_old_fpos = block->fpos;
	size_t block_old_csize = block->csize;
	
	size_t bound = compressor->compressBound(block->size);
	void* cbuff = malloc(bound);
	if (cbuff == NULL){
		return -1;
	}
	res = compressor->compress(cbuff, &bound, block->data, block->size);
	if (res != 0){
		free(cbuff);
		return -1;
	}
	block->csize = bound;
	cFBLOCK* opti_block = NULL;
	size_t opti_free_block = 0;
	//need to find free block with min suitable size
	if (cfile->fbtable->filled_count >  0)
	for (size_t  i = cfile->fbtable->filled_count - 1  ; i > 0 ; --i){
		if (cfile->fbtable->blocks[i]->size >= BLOCK_META_SIZE + block->csize){
			if (mode == 'O'){
				if (opti_block != NULL && cfile->fbtable->blocks[i]->size < opti_block->size){
					opti_block = cfile->fbtable->blocks[i];
				}
			}
			if (opti_block == NULL){
				opti_block = cfile->fbtable->blocks[i];
			}
			if (mode == 'F'){
				break;
			}
		}
	}
	size_t new_block_fpos = 0;
	if (opti_block != NULL){
		new_block_fpos = opti_block->fpos;
		block->fpos = new_block_fpos;
		res = write_block(cfile, block, cbuff);
		if (res != 0){
			block->fpos = block_old_fpos;
			return -1;
		}
		opti_block->fpos = block->fpos + BLOCK_META_SIZE + block->csize;
		opti_block->size -= BLOCK_META_SIZE + block->csize;
		if (block_old_fpos != 0 && block_old_fpos + BLOCK_META_SIZE + block_old_csize == cfile->header->last_served_fpos){
			cfile->header->last_served_fpos = block_old_fpos;
		}
	} else {
		new_block_fpos = cfile->header->last_served_fpos;
		res = write_block(cfile, block, cbuff); if (res != 0){
			block->fpos = block_old_fpos;
			return -1;
		}
		cfile->header->last_served_fpos = block->fpos + BLOCK_META_SIZE + block->csize;
	}
	if (block_old_fpos != 0){
		size_t start = block_old_fpos;
		size_t stop = start + BLOCK_META_SIZE + block_old_csize;
		if (cfile->config->to_sparse){
			sparse(cfile, start, stop);
		}
		cFBLOCK* fblock = init_fblock(start, BLOCK_META_SIZE + block_old_csize);
		//if failed to alloc new fblock, need to change header 
		if (fblock == NULL){
			cfile->header->free_size += BLOCK_META_SIZE + block_old_csize;
		} else {
			add_fblock(cfile, fblock);
		}
	}
	return 0;
}
int cdummy_flash(cFILE* cfile){
	int res = 0;
	size_t block_num = cfile->cur_block;
	cBLOCK* block = get_block(cfile, block_num, &res);
	cCOMPRESSOR* compressor = cfile->compressor;
	int compressor_valid = validate_compressor(compressor);
	if (compressor_valid != 0){
		return -1;
	}
	/*
	 * not overwriting block to not to damage it in case of failure
	 */
	if (block->data_changed == 0){
		//not needed to write unchanged data
		return 0;
	}
	size_t block_old_fpos = block->fpos;
	size_t block_old_csize = block->csize;
	size_t new_block_fpos = cfile->header->last_served_fpos;
	
	block->fpos = new_block_fpos;
	res = write_block(cfile, block, NULL);
	if (res != 0){
	//err 
		block->fpos = block_old_fpos;
		return -1;
	}

	cfile->header->last_served_fpos = block->fpos + BLOCK_META_SIZE + block->csize;	
	
	if (block_old_fpos != 0){
		size_t start = block_old_fpos;
		size_t stop = start + BLOCK_META_SIZE + block_old_csize;
		if (cfile->config->to_sparse){
			sparse(cfile, start, stop);
		}
		cFBLOCK* fblock = init_fblock(start, BLOCK_META_SIZE + block_old_csize);
		//if failed to alloc new fblock, need to change header 
		if (fblock == NULL){
			cfile->header->free_size += BLOCK_META_SIZE + block_old_csize;
		} else {
			add_fblock(cfile, fblock);
		}
	}
	return 0;
}

int cblock_flash(cFILE* cfile){
	if (cfile == NULL || cfile->header == NULL || cfile->file == NULL || cfile->btable == NULL || cfile->fbtable == NULL){
	return -1;
	}
	cCONFIG* config = cfile->config;
	size_t block_num = cfile->cur_block;
	int res = 0;
	cBLOCK* block = get_block(cfile, block_num, &res);
	if (res != 0){
		return -1;
	}
	//not needed to flash 'hole' block
	if (block == NULL){
		return 0;
	}
	//not neede to flash uncanged blocks
//	char da = block->data_changed;
	if (block->data_changed == 0){
		return 0;
	}

	res = -1;
	switch(config->flash_method){
		case CDUMMY_FLASH: 
				res = cdummy_flash(cfile);
				break;
		case COPTIMIZED_FLASH:
				res = coptimized_flash(cfile, 'O');
				break;
		case CFIRST_SUITABLE_FLASH:
				res = coptimized_flash(cfile, 'F');
				break;
		default: 
				res = -1;
				break;
	}
	if (res == 0){
		//ok
		block->data_changed = 0;
	}
	return res;
}
