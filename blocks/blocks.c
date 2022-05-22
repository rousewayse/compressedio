//#include "blocks.h"

cBLOCK* init_block(size_t size,  void* data, size_t mapped_fpos){
	cBLOCK* block = malloc(sizeof(cBLOCK));
	if (block == NULL){
		return NULL;
	}
	block->size = size;
	block->fpos = 0; // new block has 0 fpos as it's not in file
	block->mapped_fpos = mapped_fpos;
	block->data = data;
	if (data != NULL){
		block->data_changed = 1;
	} else {
		block->data_changed = 0;
	}
	block->csize = 0;
	block->cache_node = NULL;
	return block;
}

cFBLOCK* init_fblock(size_t fpos, size_t size){
	cFBLOCK* fblock = malloc(sizeof(cFBLOCK));
	if (fblock == NULL){
		return NULL;
	}
	fblock->fpos = fpos;
	fblock->size = size;
	return fblock;
}

int load_block_data(cFILE* cfile, cBLOCK* block){
	if (cfile == NULL || cfile->file == NULL || block == NULL){
		return -1;
	}
	if (block->data != NULL){
		//not needed to load block data twice
		return 0;
	}
	int compressor_valid = validate_compressor(cfile->compressor);
	if (compressor_valid != 0){
		return -1;
	}
	cCOMPRESSOR* compressor = cfile->compressor;
	void* cdata = malloc(block->csize);
	if (cdata == NULL){
		return -1;
	}
	fseek(cfile->file, block->fpos + BLOCK_META_SIZE, SEEK_SET);
	size_t res = fread(cdata, block->csize, 1, cfile->file);
	if (res == 0){
		free(cdata);
		return -1;
	}
	void* data = malloc(block->size);
	if (data == NULL){
		free(cdata);
		return -1;
	}

	int res1 = compressor->uncompress(data, &block->size, cdata, block->csize);
	if (res1 != 0){
		free(cdata);
		free(data);
		return -1;
	}
	free(cdata);
	block->data = data;
	return 0;
}

int drop_block_data(cFILE* cfile, cBLOCK* block){
	if (block == NULL || block->data_changed == 1){
		//if data changed => write it to file first
		return -1;
	}
	if (block->data == NULL){
		return 0;
	}
	free(block->data);
	block->data = NULL;
	block->data_changed = 0;
	return 0;
}
//fpos is set to appropriate pos
int write_block(cFILE* cfile, cBLOCK* block, void* cdata){
	if (cfile == NULL || cfile->file == NULL || block == NULL || block->data == NULL){
		return -1;
	}
	int compressor_valid = validate_compressor(cfile->compressor);
	if (compressor_valid != 0){
		return -1;
	}
	cCOMPRESSOR* compressor = cfile->compressor;
	if (block->data_changed == 0){
		//not needed to write unchanged data
		return 0;
	}
	size_t old_block_csize = block->csize;
	if (cdata == NULL){
		size_t bound = compressor->compressBound(block->size);
		void* cbuff = malloc(bound);
		if (cbuff == NULL){
			return -1;
		}

		int res = compressor->compress(cbuff, &bound, block->data, block->size);
		if (res != 0){
			free(cbuff);
			return -1;
		}
		block->csize = bound;
		cdata = cbuff;
	}

	int fseekre = 	fseek(cfile->file, block->fpos, SEEK_SET);
	size_t res = fwrite((void*)&block->size, sizeof(size_t), 1, cfile->file);
	res += fwrite((void*)&block->csize, sizeof(size_t), 1, cfile->file);
	res += fwrite((void*)&block->mapped_fpos, sizeof(size_t), 1, cfile->file);
	res += fwrite(cdata, block->csize, 1, cfile->file);
	free(cdata);
	if(res != 1 + 3){
		return -1;
	}
	if (old_block_csize > block->csize){
		cfile->header->cdata_size -= old_block_csize - block->csize;
	} else {
		//old_block_csize <= block->csize
		cfile->header->cdata_size += block->csize - old_block_csize;
	}
	return 0;
}

cBLOCK* load_block_meta(cFILE* cfile,  size_t fpos){
	if (cfile == NULL || cfile->file == NULL || cfile->btable == NULL || cfile->btable->blocks == NULL){
		return NULL;
	}
	cBLOCK* block = init_block(0, NULL, 0);
	if (block == NULL){
		return NULL;
	}

	fseek(cfile->file, fpos, SEEK_SET);
	size_t res = fread((void*)&block->size, sizeof(size_t), 1, cfile->file);
	res += fread((void*)&block->csize, sizeof(size_t), 1, cfile->file);
	res += fread((void*)&block->mapped_fpos, sizeof(size_t), 1, cfile->file);
	if (res != 3){
		free(block);
		return NULL;
	}
	block->fpos = fpos;
	return block;	

}


cBLOCK* get_block(cFILE* cfile, size_t num, int* res){
	if (cfile == NULL || cfile->file == NULL || cfile->btable == NULL || cfile->btable->blocks == NULL){
		*res = -1;
		return NULL;
	}
	cBTABLE* table = cfile->btable;
	if ( table->filled_count <= num){
		//no such block in table
		*res = -2;
		return NULL;
	}
	cBLOCK* block = table->blocks[num];
	if (block == NULL){
		//not loaded or hole block

		if (num < table->size){
			//can load block fpos from table in file
			size_t offset = table->fpos + sizeof(size_t) + (sizeof(size_t)*num);
			fseek(cfile->file, offset , SEEK_SET);
			size_t fpos = 0;
			fread((void*)&fpos, sizeof(fpos), 1, cfile->file);
			if (fpos == 0){
				//hole block
				*res = 0;
				return NULL;
			}
			//load block will initialize block :)
			block = load_block_meta(cfile, fpos);
			//int res1 = load_block_meta(cfile,  fpos);
			if (block == NULL){
				*res = -1;
				return NULL;
			}
			*res = 0;
			table->blocks[num] = block;
			return block;
		} else {
			//seems to be hole block that is not in file yet
			*res = 0;
			return NULL;
		}
				
	} else {
		// block is loaded 
		*res = 0;
		return block;
	}
	//smf went wrong?
	*res = -1;
	return NULL;
}
