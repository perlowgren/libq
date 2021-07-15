
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libq/json.h>
#include <libq/base64.h>
#include <libq/lzw.h>
#include <libq/rle.h>

char *compress_rle_lzw_b64(const void *ptr,long len,long *clen) {
	long rle_sz,lzw_sz,b64_sz;
	char *rle,*lzw,*b64;
	rle = q_rle_compress(ptr,len,&rle_sz);
	lzw = q_lzw_compress(rle,rle_sz,1,&lzw_sz);
	b64 = q_base64_encode(lzw,lzw_sz,&b64_sz);
	free(rle);
	free(lzw);
	if(clen!=NULL) *clen = b64_sz;
	return b64;
}

void *extract_b64_lzw_rle(const char *b64,long len,long *elen) {
	long sz,rle_sz,lzw_sz;
	char *lzw,*rle,*ptr;
	lzw = q_base64_decode(b64,len,&lzw_sz);
	rle = q_lzw_extract(lzw,lzw_sz,1,&rle_sz);
	ptr = q_rle_extract(rle,rle_sz,&sz);
	free(rle);
	free(lzw);
	if(elen!=NULL) *elen = sz;
	return (void *)ptr;
}

int main(int argc,char *argv[]) {
	char *fn = "../../seshat/test1.r20";
	QJson json;

	if(argc>=2) fn = argv[1];

	json = q_json_read(fn);
	if(json!=NULL) {
		QType val = q_json_get(json,NULL);
		if(val.t==JSON_OBJECT && val.a!=NULL) {
			QArray arr = val.a;
			val = q_array_get(arr,"layers");
			if(val.t==ARR_ARRAY && val.a!=NULL) {
				QArray arr2 = val.a;
				q_array_reset(arr2);
				for(; q_array_each(arr2,&val); ) {
					if(val.t==ARR_ARRAY && val.a!=NULL) {
						QArray arr3 = val.a;
						val = q_array_get(arr3,"pixels");
						if(val.t==ARR_STRING && val.s!=NULL) {
							long sz;
							void *p1 = extract_b64_lzw_rle((const char *)val.s,0,&sz);
							char *p2 = compress_rle_lzw_b64((const void *)p1,sz,&sz);
							free(p1);
							free(p2);
						}
					}
				}
			}
		}
		q_json_free(json);
	}

	return 0;
}

