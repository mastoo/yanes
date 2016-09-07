#include "cbuf.h"
#include <stdlib.h>
#include "extern.h"

void cbuf_init   (struct cbuf *buf, uint32_t cbuf_size, uint32_t cbuf_pkt_size){
	int i;
    buf->last = buf->first = 0;
    buf->cbuf_size = cbuf_size;
    buf->empty = 1;
    buf->full  = 0;
    buf->cpkt_list = (cbuf_pkt_ptr *) nmalloc(sizeof(cbuf_pkt_ptr)*cbuf_size);
	for(i = 0; i < cbuf_size; i++){
		buf->cpkt_list[i] = (cbuf_pkt_ptr) nmalloc(cbuf_pkt_size);
	}
}




void cbuf_pkt_insert (
	struct cbuf *buf, 
	cbuf_pkt_ptr pkt, 
	void (*pkt_cpy)(cbuf_pkt_ptr dst, cbuf_pkt_ptr src)
){

    if(!buf->full){

		pkt_cpy(buf->cpkt_list[buf->last], pkt);

        buf->empty = 0; //se inserisci non è piu vuota
        buf->last ++;    

        if(buf->last == buf->cbuf_size){ 
            buf->last = 0;
        }
        if(buf->last == buf->first){
            buf->full = 1;
        }
    }
}

void cbuf_pkt_extract(
	struct cbuf *buf, 
	cbuf_pkt_ptr pkt, 
	void (*pkt_cpy)(cbuf_pkt_ptr dst, cbuf_pkt_ptr src)
){
    
    if(!buf->empty){

		pkt_cpy(pkt,buf->cpkt_list[buf->first]);

        buf->first += 1;
        buf->full   = 0; //se estrai non è piu piena

        if(buf->first == buf->cbuf_size){ 
            buf->first = 0;
        }

        if(buf->last == buf->first){
            buf->empty = 1;
        }
    }
}
