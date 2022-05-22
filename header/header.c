//#include "header.h"

cHEADER* init_header(){
	cHEADER* header = malloc(sizeof(cHEADER));
	if (header == NULL){
		return NULL;
	}
	header->block_size = 0;
	header->blocks_table_fpos = 0;
	header->fblocks_table_fpos = 0;
	header->last_served_fpos = 0;
	header->cdata_size = 0;
	header->free_size = 0;
	return header;
}

cHEADER* load_header(cFILE* cfile){
	if (cfile == NULL || cfile->file == NULL){
		return NULL;
	}

	FILE* file = cfile->file;
//	rewind(file);
	fseek(cfile->file, 0 , SEEK_SET);
	cHEADER* header = init_header();
	if (header == NULL){
		return NULL;
	}

	size_t res = fread((void*)header, sizeof(cHEADER), 1, file);
	if(res == 0) {
		free(header);
		return NULL;
	}

	return header;
}

int write_header(FILE* file, cHEADER* header){
	if (header == NULL){
		return -1;
	}
	fseek(file, 0, SEEK_SET);
	size_t res = fwrite((void*)header, sizeof(cHEADER), 1, file);
	if (res == 0){
		return -1;
	}
	return 0;
}


