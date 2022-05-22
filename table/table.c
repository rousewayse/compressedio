//#include "table.h"

cBTABLE* init_btable(){
	cBTABLE* table = malloc(sizeof(cBTABLE));
	if (table == NULL){
		return NULL;
	}
	table->filled_count = 0;
	table->size = 0;
	table->alloc_size = 0;
	table->fpos = 0;
	table->blocks = NULL;
	return table;
}

//do not loads blocks info
cBTABLE* load_btable(cFILE* cfile){
	if (cfile == NULL || cfile->file == NULL || cfile->header == NULL){
		return NULL;
	}

	cBTABLE* table = init_btable();
	if (table == NULL){
		return NULL;
	}

	FILE* file = cfile->file;
	fseek(file, cfile->header->blocks_table_fpos, SEEK_SET);
	table->fpos = cfile->header->blocks_table_fpos;
	size_t res = fread((void*)&table->size, sizeof(table->size), 1, file);
	if (res == 0){
		free(table);
		return NULL;
	}
	table->filled_count = table->size;
	table->alloc_size = table->size;

	cBLOCK** blocks = calloc( sizeof(cBLOCK*), table->size);
	if (blocks == NULL){
		free(table);
		return NULL;
	}
	table->blocks = blocks;	
	return table;		
}

int free_blocks(cBTABLE* table){
	if (table->blocks == NULL){
		return 0;
	}
	// at this moment all blocks data should be not loaded
	for (size_t i=0; i < table->filled_count; ++i){
		if(table->blocks[i] != NULL){
			free(table->blocks[i]);
		}
	}
	return 0;
}

int free_btable(cBTABLE* table){
	if (table == NULL){
		return 0;
	}

	if (table->blocks != NULL){
		//free each block;
		free_blocks(table);
		free(table->blocks);
	}
	free(table);
	return 0;
}

size_t btable_size(cBTABLE* table){
	//num of blocks | blocks_fposes
	return sizeof(size_t) + sizeof(size_t)*table->size;
}

int write_btable(cFILE* cfile){
	if (cfile == NULL || cfile->file == NULL || cfile->header == NULL || cfile->btable == NULL){
		return -1;
	}
	size_t old_fpos = cfile->btable->fpos;
	size_t old_size = cfile->btable->size;
	size_t new_fpos = cfile->header->last_served_fpos;
	//need to load every block...
	cBTABLE* table = cfile->btable;
	fseek(cfile->file, new_fpos, SEEK_SET);
	size_t res1 = fwrite((void*)&table->filled_count, sizeof(size_t), 1, cfile->file);
	if (res1 == 0){
		return -1;
	}
	for(size_t i=0; i < table->filled_count; ++i){
		size_t block_pos = new_fpos +sizeof(size_t) /* for table size */ + i*sizeof(size_t);

		//printf("\n\tfpos = 0x%x\n", block_pos);
		if (table->blocks[i] == NULL){
			int res = 0;
			cBLOCK* block = get_block(cfile, i, &res);
			if (res != 0){
				return -1;
			}
			fseek(cfile->file, block_pos, SEEK_SET);
			if(block == NULL){
				//hole block
				size_t zero = 0;
				size_t res2 = fwrite((void*)&zero, sizeof(size_t), 1, cfile->file);
				if(res2 == 0){
					return -1;
				}
			}
			if(block != NULL){
				size_t res2 = fwrite((void*)&block->fpos, sizeof(size_t), 1, cfile->file);
				//so, it was not loaded at the moment file is closing, so it's data is ok
				table->blocks[i] = NULL;
				free(block);
				if (res2 == 0){
					return -1;
				}
			}
		} else {
			fseek(cfile->file, block_pos, SEEK_SET);
			//printf("print block pos 0x%x, table 0x%x", table->blocks[i]->fpos, block_pos);
			size_t res2 = fwrite((void*)&table->blocks[i]->fpos, sizeof(size_t), 1, cfile->file);
			if (res2 == 0){
				return -1;
			}
		}
	}
	table->fpos = new_fpos;
	cfile->header->blocks_table_fpos = new_fpos;
	cfile->header->last_served_fpos = table->fpos + sizeof(size_t) + sizeof(size_t)*table->filled_count;
	size_t old_table_size = old_size*sizeof(size_t) + sizeof(size_t);
	//printf("\n\t\t %u \n", old_table_size);
	cFBLOCK* fblock = init_fblock(old_fpos, old_table_size);
	add_fblock(cfile, fblock);

	return 0;
}
/*
int write_btable(cFILE* cfile){
	if (cfile == NULL || cfile->file == NULL || cfile->header == NULL || cfile->btable == NULL){
		return -1;
	}
	cBTABLE* table = cfile->btable;
	size_t old_fpos = table->fpos;
	size_t old_size = btable_size(table);

	//copying table
	void* buff = malloc(sizeof(size_t)*table->size);
	if (buff == NULL){
		return -1;
	}
	fseek(cfile->file, table->fpos, SEEK_SET);
	size_t res = fread(buff, sizeof(size_t), table->size, cfile->file);
	if (res != table->size){
		free(buff);
		return -1;
	}
	size_t new_fpos = cfile->header->last_served_fpos;
	fseek(cfile->file, new_fpos, SEEK_SET);
	//writing new table size
	res = fwrite((void*)&table->alloc_size, sizeof(size_t), 1, cfile->file);
	if (res == 0){
		free(buff);
		return -1;	
	}

	res = fwrite(buff, sizeof(size_t), table->size, cfile->file);
	if (res != table->size){
		free(buff);
		return -1;
	}
	//writitng new blocks fposes
	res = 0;
	for (size_t i = table->size; i < table->filled_count; ++i){
		if (table->blocks[i] != NULL){
			res += fwrite((void*)&table->blocks[i]->fpos, sizeof(size_t), 1, cfile->file);
		} else {
			size_t zero = 0;
			res += fwrite((void*)&zero, sizeof(zero), 1, cfile->file);
		}
	}
	if (res != table->filled_count - table->size){
		free(buff);
		return -1;
	}

	cfile->header->last_served_fpos = ftell(cfile->file);
	cfile->header->blocks_table_fpos = new_fpos;
	table->fpos = new_fpos;
	table->size = table->filled_count;
	free(buff);

	//table moved, need to add free space
	//fblock fpos is old_fpos, fblock_size is old_size 
	cFBLOCK* fblock = init_fblock(old_fpos, old_size);
	add_fblock(cfile, fblock);

	return 0;
}
*/
//check
int add_block(cFILE* cfile, cBLOCK* block){
	if (cfile == NULL || cfile->header == NULL || cfile->btable == NULL){
		return -1;
	} 
	if (block == NULL){
		return -1;
	}
	size_t block_num = block->mapped_fpos/cfile->config->block_size;
	cBTABLE* table = cfile->btable;
	cBLOCK** blocks = NULL;
	if (table->alloc_size <= block_num){
		blocks = realloc(table->blocks, sizeof(cBLOCK*)*(block_num + 1));
		if (blocks == NULL){
			return -1;
		}
		memset((void*)(blocks + table->alloc_size), 0, sizeof(cBLOCK*)*(block_num + 1 - table->alloc_size));
		table->alloc_size = block_num + 1;
		table->blocks = blocks;
	} 
	table->blocks[block_num] = block;
	cfile->header->cdata_size += block->csize + BLOCK_META_SIZE;
	table->filled_count = block_num + 1;

	return 0;
}

