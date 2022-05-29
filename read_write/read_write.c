//#include "read_write.h"

size_t cfread(void* ptr, size_t size, size_t nmemb, cFILE* cfile){
	if (cfile == NULL || cfile->btable == NULL){
		return 0;
	}
	if ((cfile->mode&2) == 0){
		return 0;
	}
	size_t block_size = cfile->config->block_size;
	size_t bytes_read = 0;
	size_t ptr_filled = 0;
	while ( cfile->cur_block < cfile->btable->filled_count && bytes_read != size*nmemb){
		int res = 0;
		cBLOCK* block = get_block(cfile, cfile->cur_block, &res);
		if (res != 0){
			break;
		}
		size_t block_offset = cfile->pos % block_size;
		if (block == NULL){
			//'hole' block
			size_t bytes_to_set = 0;
			if (size*nmemb - bytes_read > block_size - block_offset) {
					bytes_to_set = block_size - block_offset;
			} else {
				bytes_to_set = size*nmemb - bytes_read;
			}
			memset((ptr + ptr_filled), 0, bytes_to_set);
			bytes_read += bytes_to_set;
			cfile->pos += bytes_to_set;
			ptr_filled += bytes_to_set;
			//if ( cfile->pos >= (cfile->cur_block + 1)*block_size){
			//	cfile->cur_block++;
			//}
		}

		if (block != NULL){
			//блок есть в таблице, значит либо подгружаем данные, либо просим добавиться в кэш
			if (cfile->config->cache_size == 0){
				res = load_block_data(cfile, block);
				//check
			} else {
				res = cache_quire(cfile, block);
			}
			if (block_offset < block->size){
				//can read from block data
				size_t bytes_to_copy = 0;
				if (size*nmemb - bytes_read > block->size - block_offset){
					bytes_to_copy = block->size - block_offset;
				} else {
					bytes_to_copy = size*nmemb - bytes_read;
				}
				memcpy((ptr + ptr_filled),  (block->data + block_offset), bytes_to_copy);
				bytes_read += bytes_to_copy;
				cfile->pos += bytes_to_copy;
				ptr_filled += bytes_to_copy;
				block_offset += bytes_to_copy;
			}
			//carefull!
			if ( cfile->cur_block ==  cfile->btable->filled_count-1){
				break;
			}
			if (block_offset >= block->size && block_offset < block_size){
				size_t bytes_to_set = 0;
				if (size*nmemb - bytes_read > block_size - block_offset){
					bytes_to_set = block_size - block_offset;
				} else {
					bytes_to_set = size*nmemb - bytes_read;
				}
				memset((ptr + ptr_filled), 0, bytes_to_set);
				bytes_read += bytes_to_set;
				cfile->pos += bytes_to_set;
				ptr_filled += bytes_to_set;
			}
			//if (cfile->pos >= (cfile->cur_block + 1*block_size)){
			//	cfile->cur_block++;
			//}
			//с блока почитали, теперь если нет кэша дропнем его данные
			if (cfile->config->cache_size == 0){
				res = drop_block_data(cfile, block); // check 
			}
		}
		if (block_offset == block_size){
		//if ((size_t)cfile->pos >= (size_t)((size_t)((size_t)cfile->cur_block + 1)*block_size)){
			cfile->cur_block++;
		}	
	}

	return bytes_read / size;
}

size_t cfwrite(void* ptr, size_t size, size_t nmemb, cFILE* cfile){
	if (cfile == NULL || cfile->btable == NULL){
		return 0;
	}
	if ((cfile->mode&1) == 0){
		return 0;
	}
	size_t block_size = cfile->config->block_size;
	size_t bytes_write = 0;
	size_t ptr_filled = 0;

	while ( /*cfile->cur_block < cfile->btable->filled_count &&*/ bytes_write != size*nmemb){
		int res = 0;
		cBLOCK* block = get_block(cfile,cfile->cur_block, &res);
		if (res == -1){
			//err loading block metadata
			break;			
		}
		size_t block_offset = cfile->pos % block_size;
		//res == -2 in case of block is new to table
		if (block == NULL){
			//'hole' block
			cBLOCK* new_block = init_block(0, 0, cfile->cur_block*block_size);
			if (new_block == NULL){
				break;
			}
			size_t to_alloc = block_offset;
			if (size*nmemb - bytes_write < block_size - block_offset){
				to_alloc += size*nmemb - bytes_write;
			} else {
				to_alloc += block_size - block_offset;
			}
			void* data = calloc(1, to_alloc);
			if (data == NULL){
				break;
			}
			size_t to_copy = to_alloc - block_offset;
			memcpy((data + block_offset), (ptr + ptr_filled), to_copy);
			new_block->size = to_alloc;
			new_block->data = data;
			new_block->data_changed = 1;
			cfile->pos += to_copy;
			bytes_write += to_copy;
			ptr_filled += to_copy;			
		//	add block if it is not in file...
			if ( res == -2){
				res = add_block(cfile, new_block);
			 	if (res != 0){
			  		free(data);
					free(new_block);
					break;
				}
			} else {
				cfile->btable->blocks[cfile->cur_block] = new_block;
			}
						
			//depends on cache flash or not
			//заспавнили новый блок, если нужно, добавили в таблицу и либо закинули в кэш, либо записали в файл
			block = new_block;	
			if (cfile->config->cache_size == 0){
				res = cblock_flash(cfile);//flasher не дропает данные блока :)
				res = drop_block_data(cfile, block);
			} else {
				res = cache_quire(cfile, new_block);
			}
		} else {
		//if (block != NULL){
			//depends on cache load data or not
			// блок уже уже был в таблице, значит надо подгрузить его данные и затем скинуть, либо закинуть его в кэш, пусть он там разбирается, когда записывать в файл
			if (cfile->config->cache_size == 0){
				res = load_block_data(cfile, block);
				if (res != 0){
					break;
				}
			} else {
				res = cache_quire(cfile, block);
				//check pls
			}

			size_t to_copy = 0;
			if (size*nmemb - bytes_write < block_size - block_offset){
				to_copy = size*nmemb - bytes_write;
			} else {
				to_copy = block_size - block_offset;
			}
			if (block->size - 1 < block_offset + to_copy){
				size_t to_realloc = to_copy +  block_offset;
				void* data = realloc(block->data, to_realloc);
				if(data == NULL){
					break;
				}
				block->size = to_realloc;
				block->data = data;
			}
			memcpy((block->data + block_offset), (ptr + ptr_filled), to_copy);
			block->data_changed = 1;
			
			cfile->pos += to_copy;
			bytes_write += to_copy;
			ptr_filled += to_copy;
			// с блоком поработали, теперь, если нет кэша, записать блок в файл
			if (cfile->config->cache_size == 0){
				res = cblock_flash(cfile); //check pls
				res = drop_block_data(cfile, block);
			}
		}
		if (cfile->pos >= ((cfile->cur_block + 1)*block_size)){
			cfile->cur_block++;
		}	


	}
	return bytes_write / size;
}	
