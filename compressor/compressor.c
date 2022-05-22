
int validate_compressor(cCOMPRESSOR* compressor){
	if (compressor == NULL || compressor->compress == NULL || compressor->uncompress == NULL || compressor->compressBound == NULL){
		return -1;
	}
	return 0;
}

int set_compressor(cCOMPRESSOR* compressor, cFILE* cfile){
	if(cfile == NULL){
		return -1;
	}
	int res = validate_compressor(compressor);
	if (res == 0){
		cfile->compressor = compressor;
	}
	return res;
}


