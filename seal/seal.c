//#include "seal.h"

FILE* create_tmp_file(char** tmp_name, char* filename){
	*tmp_name = malloc(strlen(filename) + 1 + 6);
	if (*tmp_name == NULL){
		*tmp_name = NULL;
		return NULL;
	}
	strcpy(*tmp_name, filename);
	strcpy(*tmp_name + strlen(filename), "XXXXXX");
	int tmpfd = mkstemp((char*)*tmp_name);
	if (tmpfd == -1){
		return NULL;
	}

	FILE* tmpfile = fdopen(tmpfd, "r+b");
	if (tmpfile == NULL){
		free(*tmp_name);
		*tmp_name = NULL;
	}
	return tmpfile;
}


int cseal_blocks(cFILE* cfile){
	if (cfile == NULL || cfile->file == NULL || cfile->btable == NULL){
		return -1;
	}
	int succ = 0;
	cCOMPRESSOR* compressor = cfile->compressor;
	int compressor_valid = validate_compressor(compressor);
	if (compressor_valid != 0){
		return -1;
	}
	//creating and opening tmp_file
	char* tmp_name;
	FILE* tmpfile = create_tmp_file(&tmp_name, cfile->filename);
	if (tmpfile == NULL){
		return -1;
	}

	// tmp file is created and opened
	//i know the block table size and cdata_size too, free block table size is zero, free space size is zero
	//so i can warite correct header
	
	size_t blocks_start = sizeof(cHEADER);
	size_t* blocks_fposes = calloc(sizeof(size_t), cfile->btable->filled_count);
	if (blocks_fposes == NULL){
		fclose(tmpfile);
		return -1;
	}
	fseek(tmpfile, blocks_start, SEEK_SET);

	for(size_t i = 0; i < cfile->btable->filled_count; ++i){
		int res = 0;
		cBLOCK* block = get_block(cfile, i, &res);
		if (res != 0){
			free(blocks_fposes);
			fclose(tmpfile);
			succ = -1;
			break;
		}

		if (block == NULL){
			blocks_fposes[i] = 0;
			continue;
		}
		//at this moment all blocks data should be written to file, because sealing on cfclose
		void* buff = malloc(block->csize);
		if (buff == NULL){
			free(blocks_fposes);
			fclose(tmpfile);	
			succ = -1;
			break;
		}
		fseek(cfile->file, block->fpos + BLOCK_META_SIZE, SEEK_SET);
		size_t read = fread(buff, block->csize, 1, cfile->file);
		if (read == 0){
			free(blocks_fposes);
			fclose(tmpfile);			
			free(buff);
			succ = -1;
			break;
		}
		blocks_fposes[i] = ftell(tmpfile); 
		size_t write = fwrite((void*)&block->size, sizeof(size_t), 1, tmpfile);
		write += fwrite((void*)&block->csize, sizeof(size_t), 1, tmpfile);
		write += fwrite((void*)&block->mapped_fpos, sizeof(size_t), 1, tmpfile);
		write += fwrite(buff, block->csize, 1, tmpfile);
		if (write != 4){
			free(blocks_fposes);
			fclose(tmpfile);			
			free(buff);
			succ = -1;
			break;
		}
		free(buff);	
	}
	
	if (succ == -1){
		remove(tmp_name);
		free(tmp_name);
		return -1;
	}

	cHEADER* new_header = malloc(sizeof(cHEADER));
	memcpy(new_header, cfile->header, sizeof(cHEADER));
	new_header->free_size = 0;
	//writing tables 
	new_header->fblocks_table_fpos = ftell(tmpfile);

	size_t zero = 0;
	size_t write = fwrite((void*)&zero, sizeof(zero), 1, tmpfile);
	new_header->blocks_table_fpos = ftell(tmpfile);
	write += fwrite((void*)&cfile->btable->filled_count, sizeof(size_t), 1, tmpfile);
	for (size_t i = 0; i < cfile->btable->filled_count; ++i){
		write += fwrite((void*)&blocks_fposes[i], sizeof(size_t), 1, tmpfile);
	}
	new_header->last_served_fpos = ftell(tmpfile);
	fseek(tmpfile, 0, SEEK_SET);
	int res = write_header(tmpfile, new_header);

	free(new_header);
	free(blocks_fposes);
	if (res != 0 || write != 2 + cfile->btable->filled_count){
		free(new_header);
		free(blocks_fposes);
		fclose(tmpfile);
		remove(tmp_name);
		free(tmp_name);
	} else {
		fclose(cfile->file);
		cfile->file = tmpfile;
		remove(cfile->filename);
		rename(tmp_name, cfile->filename);
		free(tmp_name);
		//free(cfile->filename);
		return 0;

	}
	return -1;
}
