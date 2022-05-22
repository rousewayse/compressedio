//#include "config.h"
//16kb
//if packing_mode set to CSEALING_MODE, sealing_treshold is ignored

cCONFIG* get_default_cconfig(){
	cCONFIG* config = malloc(sizeof(cCONFIG));
	if (config == NULL){
		return NULL;
	}
	if (config != NULL) {
		// DEFAULT UNCOMPRESSED BLOCK SIZE
		config->block_size = DEFAULT_BLOCK_SIZE;
		// DEFAULT PACKING MODE: gapping or keep blocks sealed
		config->packing_mode = DEFAULT_PACKING_MODE;
		// if it's needed to fill empty blocks with CFILLING_BYTE
		config->to_sparse = DEFAULT_TO_SPARSE;
		// default strategy of writing new blocks to the file
		config->flash_method = DEFAULT_FLASH_METHOD;
		// TRESHOLD for sealing blocks, in persents o file space overusage
		config->sealing_treshold = DEFAULT_REPACKING_TRESHOLD;
		//CACHE
		config->cache_size = 0; // no cache	
		return config;
	}
	
	return NULL;
}

int verify_config(cCONFIG* config){
	if (config == NULL){
		//default config is valid
		return 0;
	}
	
	if (config->block_size == 0){
		return -1;
	}
	char packing_mode = config->packing_mode;
	if (packing_mode != CSEALING_MODE && packing_mode != CGAPING_MODE){
		return -1;
	}
	
	if (config->to_sparse != 0 && config->to_sparse != 1){
		return -1;	
	}
	
	char flash_method = config->flash_method;
	if(flash_method != CDUMMY_FLASH && flash_method != COPTIMIZED_FLASH && flash_method != CFIRST_SUITABLE_FLASH){
		return -1;
	}

	if (config->sealing_treshold <=0){
		return -1;
	}
	
	return 0;		
}

