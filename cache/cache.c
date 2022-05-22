
//needs updates
int sync_and_drop_node(cFILE* cfile, cBLOCK* block){
	if (block == NULL){
		return 0;
	}
	size_t old_cur_block = cfile->cur_block;
	size_t block_num = block->mapped_fpos / cfile->config->block_size;
	cfile->cur_block = block_num;
	int res = cblock_flash(cfile);
	drop_block_data(cfile, block);
	cfile->cur_block = old_cur_block;	
	//cBLOCK* old_cur = cfile->_BLOCKS->_CURR_BLOCK;
	//cfile->_BLOCKS->_CURR_BLOCK =block;
	//int res = cblock_flash(cfile);
	//
	//OLD
	//cdrop_cur_block_data(cfile);
	//cfile->_BLOCKS->_CURR_BLOCK = old_cur;
	return 0;
}

int sync_node(cFILE* cfile, cBLOCK* block){
	if (block == NULL){
		return 0;
	}
	size_t old_cur_block = cfile->cur_block;
	size_t block_num = block->mapped_fpos / cfile->config->block_size;
	cfile->cur_block = block_num;
	int res = cblock_flash(cfile);
	cfile->cur_block = old_cur_block;
// OLD
//	cBLOCK* old_cur = cfile->_BLOCKS->_CURR_BLOCK;
//	cfile->_BLOCKS->_CURR_BLOCK =block;
//	int res = cblock_flash(cfile);
//	cdrop_cur_block_data(cfile);
//	cfile->_BLOCKS->_CURR_BLOCK = old_cur;
	return 0;
}
//int sync_block(cFILE* cfile, cBLOCK* block);

QNode*  init_qnode(cBLOCK* block){
	QNode* node = malloc(sizeof(QNode));
	if (node == NULL){
		return NULL;
	}

	node->block = block;
	node->next = NULL;
	node->prev = NULL;
	node->in_lower = 0;
	return node;
}

Queue* init_queue(size_t size){
	Queue* queue = malloc(sizeof(Queue));
	if (queue == NULL){
		return NULL;
	}
	queue->size = size;
	queue->filled_count = 0;
	queue->front = NULL;
	queue->rear = NULL;
	return queue;
}

cCACHE* init_ccache(size_t size){
	cCACHE* cache = malloc(sizeof(cCACHE));
	if (cache == NULL){
		return NULL;
	}

	size_t upper_size = size / 2;
	size_t lower_size = size - upper_size;
	
	Queue* lower = init_queue(lower_size);
	if (lower == NULL){
		free(cache);
		return NULL;
	}

	Queue* upper = init_queue(upper_size);
	
	if (upper == NULL){
		free(lower);
		free(cache);
		return NULL;
	}

	cache->lower = lower;
	cache->upper = upper;
	return cache;
}

int queue_full (Queue* queue){
	if (queue->size <=  queue->filled_count ){
		return 1;
	}
	return 0;
}
int queue_empty(Queue* queue){
	if (queue->filled_count == 0){
		return 1;
	}
	return 0;
}



/*
int sync_block(cFILE* cfile, cBLOCK* block){
	cBLOCK* old_curr = cfile->_BLOCKS->_CURR_BLOCK;
	cfile->_BLOCKS->_CURR_BLOCK = block;
	
	int res = cblock_flash(cfile);
	block->_DATA_CHANGED = 0;
//	cdrop_cur_block_data(cfile);
	
	cfile->_BLOCKS->_CURR_BLOCK = old_curr;
	return res;
}
*/
int cfsync(cFILE* cfile){
	//cache == NULL so there is no cache and nothing ot sync
	if (cfile->cache == NULL){
		return 0;
	}	
	cCACHE* cache = cfile->cache;
	Queue* lower = cache->lower;
	Queue* upper = cache->upper;

	QNode* temp = lower->front;
	while(temp != NULL){
		sync_node(cfile, temp->block);
		temp = temp->next;
	}
//	lower->filled_count = 0;

	temp = upper->front;
	while (temp != NULL){
		sync_node(cfile, temp->block);
		temp = temp->next;
	}
//	upper->filled_count = 0;
	return 0;
}



int free_cache(cFILE* cfile){
	cCACHE* cache = cfile->cache;
	if (cache == NULL){
		return 0;
	}

	cfsync(cfile);
	if (cache->lower != NULL){
		QNode* temp = cache->lower->front;
		QNode* ftemp;
		//free Qnodes
		while(temp != NULL){
			ftemp = temp;
			temp = temp->next;
			free(ftemp);
		}
		//free lower queue
		free(cache->lower);
	}

	if (cache->upper != NULL){
		QNode* temp = cache->upper->front;
		QNode* ftemp;
		while (temp != NULL){
			ftemp = temp;
			temp = temp->next;
			free(ftemp);	
		}
		free(cache->upper);
	}

	free(cache);
	return 0;
}
// NEW CODE //



int deQueue_upper(cFILE* cfile){
	Queue* upper = cfile->cache->upper;
	if (queue_empty(upper)){
		return 0;
	}

	QNode* node;
	if (upper->front == upper->rear){
		node = upper->front;
		upper->front = NULL;
		upper->rear = NULL;
		upper->filled_count--;
	} else {
		node = upper->rear;
		upper->rear = upper->rear->prev;
		upper->rear->next = NULL;
		upper->filled_count--;
	}

	node->block->cache_node = NULL;
	enQueue_lower(cfile, node->block);
	free(node);
	return 0;
}

int enQueue_upper(cFILE* cfile, cBLOCK* block){
	Queue* upper = cfile->cache->upper;
	QNode* node = init_qnode(block);
	node->in_lower = 0;
	block->cache_node = node;
	
	if (queue_empty(upper)){
		upper->front = node;
		upper->rear = node;
		upper->filled_count++;
		return 0;
	} 
   
	if (queue_full(upper)){
		deQueue_upper(cfile);
	}

	node->next = upper->front;
	upper->front->prev = node;
	upper->front = node;
	upper->filled_count++;
	return 0;
}


int deQueue_lower(cFILE* cfile){
	
	Queue* lower = cfile->cache->lower;
	if (queue_empty(lower)){
		return 0;
	}
	//one node in cache
	QNode* node;
	if (lower->front == lower->rear) {
		node = lower->front;
		lower->front = NULL;
		lower->rear = NULL;
		lower->filled_count--;
	} else {
		node = lower->rear;
		lower->rear = lower->rear->prev;
		lower->rear->next = NULL;
		lower->filled_count--;
	}
	
	node->block->cache_node = NULL;
	
	sync_and_drop_node(cfile, node->block);
	free(node);
	return 0;
}

int enQueue_lower(cFILE* cfile, cBLOCK* block){
	Queue* lower = cfile->cache->lower;
	
	QNode* node = init_qnode(block);
//	cBLOCK* old_cur_block = cfile->_BLOCKS->_CURR_BLOCK;	
//	cfile->_BLOCKS->_CURR_BLOCK = block;
	int res = load_block_data(cfile, block);
//	int res = cload_cur_block_data(cfile);
	if (res != 0){
		free(node);
		return -1;
	}
//	cfile->_BLOCKS->_CURR_BLOCK = old_cur_block;
	//my fix moved queue_full check before empty check
	//lower is not empty
	if (queue_full(lower)){
		// lower is full => dequeue it and add new node
		deQueue_lower(cfile);
	}
	//check if lower is empty
	int if_empty = queue_empty(lower);
	if (if_empty){
		lower->front = node;
		lower->rear = node;	
		node->in_lower = 1;
		block->cache_node = node;
		lower->filled_count++;
		return 0;	
	}
	


	//add new node
	node->next = lower->front;
	lower->front->prev = node;
	lower->front = node;
	node->in_lower = 1;
	lower->filled_count++;
	block->cache_node = node;
	return 0;
}
int exclude_node(QNode* node, Queue* lower, cFILE* cfile){
	if (lower->front == node){
		if (lower->front == lower->rear){
			lower->front = NULL;
			lower->rear = NULL;
			lower->filled_count--;
		} else {
			lower->front = lower->front->next;
			lower->front->prev = NULL;
			lower->filled_count--;
		}
	} else {
		if (lower->rear == node){
			lower->rear = lower->rear->prev;
			lower->rear->next = NULL;
			lower->filled_count--;
		} else {
			node->prev->next = node->next;
			//fix 1
			node->next->prev = node->prev;
			lower->filled_count--;
		}
	}
	return 0;
}
/*
int make_node_first(QNode* node, Queue* upper){
	if (node == upper->front){
		return 0;
	}

}
*/
//checks if block is cached
int cache_quire(cFILE* cfile, cBLOCK* block){
	if (block == NULL ){
		return -1;//????
	}

	if (block->cache_node != NULL){
		if (block->cache_node->in_lower != 0){
			//move it to upper queue
			exclude_node(block->cache_node, cfile->cache->lower, cfile);
			free(block->cache_node);
			block->cache_node = NULL;
			enQueue_upper(cfile, block);
			
		} 
	   	else
	   {
			//move it to begining of upperi	
			if (cfile->cache->upper->front != NULL && cfile->cache->upper->front->block != block){
			
			exclude_node(block->cache_node, cfile->cache->upper, cfile);
			free(block->cache_node);
			block->cache_node = NULL;
			enQueue_upper(cfile, block);
			}
			
		
		}
		
		return 0;
	} else {
		return enQueue_lower(cfile, block);
		
	}
}
