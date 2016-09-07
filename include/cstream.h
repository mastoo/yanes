#ifndef __CSTREAM_H__
#define __CSTREAM_H__

#include <stdint.h>


//circular stream  definition
struct cstream{
    uint32_t first;
    uint32_t last;
    uint8_t *data;
    uint32_t cstream_size; //numero massimo di dati nella coda
    int empty;
    int full;
	uint32_t size;
};


void 	 cstream_init  (struct cstream *buf, uint32_t cstream_size);
void     cstream_reset (struct cstream *buf);
int  	 cstream_isempty (struct cstream *buf);
int  	 cstream_isfull  (struct cstream *buf);
uint32_t cstream_getsize(struct cstream *buf);


int cstream_insert(
	struct cstream *buf, 
	uint8_t *new_data,
	uint32_t new_data_size 
);

int cstream_extract(
	struct cstream *buf, 
	uint8_t *new_data,
	uint32_t new_data_size
);

 
#endif

