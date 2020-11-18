#include<stdio.h>
#include<stdlib.h>
#include"compressedio.h"

void main (){
    convertFile("input", "ctext");
    cFILE* cfin = NULL;
    cfopen("ctext", &cfin);
    loadCurrBlockData(cfin);
    char* buff = NULL;
    getCurrBlockData(cfin, &buff);
    for(int i =0; i < 71; ++i) printf("%c", buff[i]);
    puts("");
}
