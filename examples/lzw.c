
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libq/lzw.h>

int main(int argc,char *argv[]) {
	long n;
	char *fn = "../../seshat/test1.r20";
	long sz,csz,esz;
	char *d = NULL;
	char *c = NULL;
	char *e = NULL;
	int r = 0;
	FILE *fp;

	if(argc>=2) fn = argv[1];

	fp = fopen(fn,"r");
	if(fp==NULL) {
		perror("fopen");
		return 1;
	} else {
		fseek(fp,0,SEEK_END);
		n = ftell(fp);
		if(n>0) {
			d = (char *)malloc(n+1);
			fseek(fp,0,SEEK_SET);
			fread(d,n,1,fp);
			d[n] = '\0';
		}
		fclose(fp);
	}

	if(d!=NULL) {
		sz = strlen(d);
		c = (char *)q_lzw_compress((const void *)d,sz,1,&csz);
		if(c!=NULL) {
			e = (char *)q_lzw_extract((const void *)c,csz,1,&esz);
			printf(""
				"Compare: %d\n"
				"Size of file: %ld\n"
				"Compressed size: %ld\n"
				"Extracted size: %ld\n"
				"Compression ratio: %.2f\n",
				strcmp(d,e),sz,csz,esz,100.0*((double)csz/(double)sz));
		} else r = 2;
	} else r = 1;

	if(c!=NULL) free(c);
	if(e!=NULL) free(e);
	if(d!=NULL) free(d);

	return r;
}

