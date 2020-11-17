#include<stdio.h>
#include<stdlib.h>
#include "compressedio.h"

void main (){

//    convertFile("/tmp/dada");
    cFILE* CFILE = NULL;
   cfopen("output_file", CFILE);
   //for (int i=0; i< 25; ++i) printf("%d\n", moveNextBlock(CFILE));
      loadCurrBlockData ( CFILE);
      cfclose(CFILE, CBURN);
}
