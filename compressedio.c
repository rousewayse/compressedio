#include "compressedio.h"

cBLOCK* init_block(size_t size,  void* data, size_t mapped_fpos);
cFBLOCK* init_fblock(size_t fpos, size_t size);
int load_block_data(cFILE* cfile, cBLOCK* block);
int drop_block_data(cFILE* cfile, cBLOCK* block);
int write_block(cFILE* cfile, cBLOCK* block, void* cdata);
cBLOCK* load_block_meta(cFILE* cfile,  size_t fpos);
cBLOCK* get_block(cFILE* cfile, size_t num, int* res);


int sync_and_drop_node(cFILE* cfile, cBLOCK* block);
int sync_node(cFILE* cfile, cBLOCK* block);
QNode*  init_qnode(cBLOCK* block);
Queue* init_queue(size_t size);
cCACHE* init_ccache(size_t size);
int queue_full (Queue* queue);
int queue_empty(Queue* queue);
int cfsync(cFILE* cfile);
int free_cache(cFILE* cfile);
int deQueue_upper(cFILE* cfile);
int enQueue_upper(cFILE* cfile, cBLOCK* block);
int deQueue_lower(cFILE* cfile);
int enQueue_lower(cFILE* cfile, cBLOCK* block);
int exclude_node(QNode* node, Queue* lower, cFILE* cfile);
int cache_quire(cFILE* cfile, cBLOCK* block);


int validate_compressor(cCOMPRESSOR* compressor);
int set_compressor(cCOMPRESSOR* compressor, cFILE* cfile);



cCONFIG* get_default_cconfig();
int verify_config(cCONFIG* config);


int coptimized_flash(cFILE* cfile, char mode);
int cdummy_flash(cFILE* cfile);
int cblock_flash(cFILE* cfile);


cHEADER* init_header();
cHEADER* load_header(cFILE* cfile);
int write_header(FILE* file, cHEADER* header);



void crewind(cFILE* cfile);
long int cftell(cFILE* cfile);
int cfseek_SEEK_END(cFILE* cfile, long int offset);
int cfseek_SEEK_CUR(cFILE* cfile, long int offset);
int cfseek_SEEK_SET(cFILE* cfile, long int offset);
int cfseek(cFILE* cfile, long int offset, int origin);



size_t cfread(void* ptr, size_t size, size_t nmemb, cFILE* cfile);
size_t cfwrite(void* ptr, size_t size, size_t nmemb, cFILE* cfile);


int cseal_blocks(cFILE* cfile);


cBTABLE* init_btable();
cBTABLE* load_btable(cFILE* cfile);
int free_blocks(cBTABLE* table);
int free_btable(cBTABLE* table);
size_t btable_size(cBTABLE* table);
int write_btable(cFILE* cfile);
int add_block(cFILE* cfile, cBLOCK* block);


cFBTABLE* init_fbtable();
cFBLOCK** load_fblocks(cFILE* cfile);
cFBTABLE* load_fbtable(cFILE* cfile);
int free_fblocks(cFBTABLE* table);
int add_fblock(cFILE* cfile, cFBLOCK* fblock);
int write_ftable(cFILE* cfile);


#include "blocks/blocks.c"
#include "cache/cache.c"
#include "compressor/compressor.c"
#include "config/config.c"
#include "flash/flash.c"
#include "header/header.c"
#include "read_write/read_write.c"
#include "seal/seal.c"
#include "table/table.c"
#include "table/ftable.c"
#include "pos/fpos.c"
double overusage_persent(cFILE* cfile){
	if (cfile == NULL || cfile->header == NULL){
		return -1.0;
	}
	if (cfile->header->cdata_size == 0){
		return -1.0;
	}
	return (double)cfile->header->free_size/cfile->header->cdata_size*100;
}

//not needed to rebind blocks...

//if returned -1 do not worry, nothing critical....
int sparse(cFILE* cfile, size_t start, size_t stop){
	int res = fseek(cfile->file, start, SEEK_SET);
	if (res != 0){
		return -1;
	}
	void* buff = calloc(1, stop - start);
	if (buff == NULL){
		return -1;
	}
	res = 0;
	res = fwrite(buff, 1, stop - start, cfile->file);
	free(buff);
	if (res < stop - start){
		return -1;
	}
	return 0;
}

cHEADER* init_empty_header(){
	
}


int init_empty_file(cFILE* cfile){
	cHEADER* header = init_header();
	if(header == NULL){
		return -1;
	}

	header->block_size = cfile->config->block_size;
	header->blocks_table_fpos = sizeof(cHEADER) + sizeof(size_t);
	header->fblocks_table_fpos = sizeof(cHEADER);
	header->last_served_fpos = sizeof(cHEADER) + 2*sizeof(size_t);
	fseek(cfile->file, 0 , SEEK_SET);
	size_t res = fwrite((void*)header, sizeof(cHEADER), 1, cfile->file);
	if (res == 0){
		free(header);
		return -1;
	}
	fseek(cfile->file, header->fblocks_table_fpos, SEEK_SET);
	size_t zero = 0;
	res = fwrite((void*)&zero, sizeof(zero), 1, cfile->file);
	res += fwrite((void*)&zero, sizeof(zero), 1, cfile->file);
	if (res != 2){
		free(header);
		return -1;
	}
	cfile->header = header;
	return 0;
}

//AppendCreatePosReadWrite - xxx
int parse_mode(char* mode){
	if (mode == NULL){
		return -1;//invalid mode
	}
	int res = 0;
	
	switch(mode[0]){
		case 'r':
			res += 2;
			break;
		case 'w':
			res += 1 + 8;
			break;
		case 'a':
			res += 1 + 4 + 8 + 16;
			break;
		default:
			res = -1;
			break;
	}
	if (res == -1){
		return -1;
	}

	if (strlen(mode) > 1 && mode[1] == '+'){
		res = res|(1+2);
	}
	return res;
}


cFILE* cfopen(const char* filename, cCONFIG* user_config, char* mode){
	int mode_parsed = parse_mode(mode);
	if (mode_parsed == -1){
		return NULL;
	}
	
	FILE* tmp_file = fopen(filename, "r");
	int file_exists = 0;
	if(tmp_file == NULL){
		file_exists = 0;
	} else {
		file_exists = 1;
		fclose(tmp_file);
	}
	
	if ((mode_parsed&8) != 0){
		file_exists = 0;
	}

	const char* file_mode = "r+b";
	FILE* file;
	if (file_exists == 1){
		file = fopen(filename, file_mode);
	} else {
		file = fopen(filename, "w+b");
		fclose(file);
		file = fopen(filename, file_mode);
	}

	if (file == NULL){
		return NULL;
	}
	
	cFILE* cfile = malloc(sizeof(cFILE));
	if (cfile == NULL){
		fclose(file);
		return NULL;
	}
	
	//user_config == NULL -> using default config;
	cCONFIG* config = get_default_cconfig();
	if (config == NULL){
		fclose(file);
		return NULL;
	}

	if (user_config == NULL){
		cfile->config = config;	
	} else {
		if (verify_config(user_config) != 0){
			fclose(file);
			free(config);
			free(cfile);
			return NULL;
		}
		memcpy(config, user_config, sizeof(cCONFIG));
		cfile->config = config;
	}

	cfile->file = file;
	cfile->mode = mode_parsed;
	cfile->filename = malloc(strlen(filename) +1);
	strcpy(cfile->filename, filename);

	int res = 0;
	struct stat stat_record;
	stat(filename, &stat_record);
	if (stat_record.st_size == 0){
		// prepare empty file
		res = init_empty_file(cfile);
		if (res != 0){
			free(cfile);
			free(config);
			fclose(file);
			return NULL;
		}
	} else {
		cHEADER* header = load_header(cfile);
		if (header == NULL){
			free(cfile);
			free(config);
			fclose(file);
			return NULL;
		}
		cfile->header = header;
	}

	cBTABLE* btable = load_btable(cfile);
	if (btable == NULL){
		free(cfile->header);
		free(cfile);
		free(config);
		fclose(file);
		return NULL;
	}

	cfile->btable = btable;

	cFBTABLE* fbtable = load_fbtable(cfile);
	if (fbtable == NULL){
		free(btable->blocks);
		free(btable);
		free(cfile->header);
		free(cfile);
		free(config);
		fclose(file);
		return NULL;	
	}

	cfile->fbtable = fbtable;

	//cache init
	if (cfile->config->cache_size == 0){
		cfile->cache = NULL;
	} else {
		cfile->cache = init_ccache(cfile->config->cache_size);
	}
		
	cfile->pos = 0;
	cfile->cur_block = 0;
	if((mode_parsed&16) == 16){
		cfseek(cfile, 0, SEEK_SET);
	}
	return cfile;
}


int cfclose(cFILE* cfile){
	if (cfile == NULL){
		return 0;
	}

	if (cfile->cache != NULL){
		free_cache(cfile);
	}

	if ( overusage_persent(cfile) > cfile->config->sealing_treshold){
		cseal_blocks(cfile);
	} else {
		write_ftable(cfile);
		write_btable(cfile);	
		write_header(cfile->file, cfile->header );
	}

	free_btable(cfile->btable);
	free_fbtable(cfile->fbtable);
	
	free(cfile->config);
	free(cfile->filename);
	free(cfile->header);
	fclose(cfile->file);
	free(cfile);
	return 0;
}


int convertFile(const char* filename, const char* output_filename, size_t block_size, cCOMPRESSOR* compressor){
	int compressor_valid = validate_compressor(compressor);
	if (compressor_valid != 0){
		return -1;
	}
	FILE* fin = fopen(filename, "rb");
	cCONFIG* config  = get_default_cconfig();
	config->block_size = block_size;
	cFILE* fout = cfopen(output_filename, config, "w+");
	set_compressor(compressor, fout);
	void* buff = malloc(block_size);
	
	struct stat stat_record;
	stat(filename, &stat_record);
	size_t filesize = stat_record.st_size;
	int succ = 0;
	while(filesize > 0){

		size_t to_read; 
		if (filesize / block_size > 0){
			to_read = block_size;
		} else {
			to_read  = filesize % block_size;
		}

		size_t res = fread(buff, to_read, 1, fin);
		if (res == 0){
			succ = -1;
			break;
		}

		res = cfwrite(buff,to_read,  1, fout);
		if (res == 0){
			succ = -1;
			break;
		}
		filesize -= to_read;
	}
	printf("ftell = %li", ftell(fin));
	fclose(fin);
	cfclose(fout);
	free(buff);
	return succ;
}
