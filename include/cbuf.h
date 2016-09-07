#ifndef __CBUF_H__
#define __CBUF_H__

#include <stdint.h>


//tipo del dato contenuto all'interno della coda circolare
typedef uint8_t *cbuf_pkt_ptr;


// CMD PKT DEFINITION
struct cbuf{
    uint32_t first;
    uint32_t last;
    cbuf_pkt_ptr *cpkt_list;
    uint32_t  cbuf_size; //numero massimo di pacchetti nella coda
    uint32_t  cbuf_pkt_size; //dimensione di un pacchetto nella coda
	
    int empty;
    int full;
};

void cbuf_init       (struct cbuf *buf, uint32_t cbuf_size, uint32_t cbuf_pkt_size);
int  cbuf_isempty    (struct cbuf *buf);
int  cbuf_isfull     (struct cbuf *buf);

void cbuf_pkt_insert(
	struct cbuf *buf, 
	cbuf_pkt_ptr pkt, 
	void (*pkt_cpy)(cbuf_pkt_ptr dst, cbuf_pkt_ptr src)
);

void cbuf_pkt_extract(
	struct cbuf *buf, 
	cbuf_pkt_ptr pkt, 
	void (*pkt_cpy)(cbuf_pkt_ptr dst, cbuf_pkt_ptr src)
);

 
#endif
