//#include "table.h"


cFBTABLE* init_fbtable(){
	cFBTABLE* table = malloc(sizeof(cFBTABLE));
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


cFBLOCK** load_fblocks(cFILE* cfile){
	if (cfile == NULL || cfile->header == NULL || cfile->fbtable == NULL){
		return NULL;
	}
	cFBLOCK** blocks = malloc(sizeof(cFBLOCK*)*cfile->fbtable->size);
	if (blocks == NULL){
		return NULL;
	}
	int res = 0;
	size_t i = 0;
	for (i = 0; i < cfile->fbtable->size; ++i){
		size_t fbfpos = cfile->header->fblocks_table_fpos + sizeof(size_t) + i*sizeof(size_t);
		fseek(cfile->file, fbfpos, SEEK_SET);
		cFBLOCK* fblock = init_fblock(0, 0);
		if (fblock == NULL){
			res = -1;
			break;
		}
		size_t res1 = fread((void*)fblock, sizeof(fblock), 1, cfile->file);
		if (res == 0){
			res = -1;
			break;
		}
		blocks[i] = fblock;
	}
	if (res == -1){
		for (size_t j = 0; j < i; j++){
			free(blocks[j]);
		}
		free(blocks);
		return NULL;
	}
	return blocks;
}

cFBTABLE* load_fbtable(cFILE* cfile){
	if (cfile == NULL || cfile->file == NULL || cfile->header == NULL){
		return NULL;
	}

	cFBTABLE* table = init_fbtable();
	if (table == NULL){
		return NULL;
	}

	FILE* file = cfile->file;
	fseek(file, cfile->header->fblocks_table_fpos, SEEK_SET);
	table->fpos = cfile->header->fblocks_table_fpos;
	size_t res = fread((void*)&table->size, sizeof(table->size), 1, file);
	if (res == 0){
		free(table);
		return NULL;
	}
	table->filled_count = table->size;
	table->alloc_size = table->size;
	cfile->fbtable = table;
	cFBLOCK** blocks = load_fblocks(cfile);	
	if (blocks == NULL){
		free(table);
		return NULL;
	}
	table->blocks = blocks;
	return table;		
}

int free_fblocks(cFBTABLE* table){
	if (table->blocks == NULL){
		return 0;
	}
	for (size_t i=0; i < table->filled_count; ++i){
		if(table->blocks[i] != NULL){
			free(table->blocks[i]);
		}
	}
	return 0;
}

int free_fbtable(cFBTABLE* table){
	if (table == NULL){
		return 0;
	}
	if (table->blocks != NULL){
		free_fblocks(table);
		free(table->blocks);
		free(table);
	}
	return 0;
}

int add_fblock(cFILE* cfile, cFBLOCK* fblock){
	if (cfile == NULL || cfile->header == NULL || cfile->fbtable == NULL || cfile->config == NULL){
		return -1;
	}
	if (fblock->size <=  BLOCK_META_SIZE){
		cfile->header->free_size += fblock->size;
		free(fblock);
		return 0;
	}
	cFBTABLE* table = cfile->fbtable;
	cFBLOCK** blocks = NULL;
	if (table->filled_count == table->alloc_size){
		blocks = realloc(table->blocks, sizeof(cFBLOCK*)*(table->filled_count + 1));
		if (blocks == NULL){
			free(fblock);	
			cfile->header->free_size += fblock->size;
			return -1;
		}
		table->alloc_size++;
		table->blocks = blocks;
	}
	table->blocks[table->filled_count] = fblock;
	cfile->header->free_size += fblock->size;
	table->filled_count++;
	return 0;
}


int write_ftable(cFILE* cfile){
	if (cfile == NULL || cfile->file == NULL || cfile->fbtable == NULL){
		return -1;
	}	 
	size_t old_fpos = cfile->fbtable->fpos;
	size_t old_size = cfile->fbtable->size*sizeof(size_t) + sizeof(size_t);
	//cFBLOCK* fblock = init_fblock(old_fpos, old_size);
	//if(fblock != NULL){
	//	add_fblock(cfile, fblock);
	//}
	size_t new_fpos = cfile->header->last_served_fpos;
	fseek(cfile->file, new_fpos, SEEK_SET);
	size_t res = fwrite((void*)&cfile->fbtable->filled_count, sizeof(size_t), 1, cfile->file);
	size_t count = 0;
	for(size_t i =0; i < cfile->fbtable->filled_count; ++i){
		//if (cfile->fbtable->blocks[i]->size > BLOCK_META_SIZE){
			res += fwrite((void*)cfile->fbtable->blocks[i]->fpos, sizeof(size_t) ,1, cfile->file);
		//	count++;
		//}
	}
	if (cfile->fbtable->filled_count + 1 != res){	
		return -1;
	}
	cfile->header->fblocks_table_fpos = new_fpos;
	cfile->fbtable->fpos = new_fpos;
	cfile->fbtable->size = cfile->fbtable->filled_count;
	cfile->header->last_served_fpos = ftell(cfile->file);	
	
	cFBLOCK* fblock = init_fblock(old_fpos, old_size);
	if(fblock != NULL){
		add_fblock(cfile, fblock);
	}
	return 0;
}
