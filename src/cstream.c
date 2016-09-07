#include "cstream.h"
#include "extern.h"

void cstream_init (struct cstream *buf, uint32_t cstream_size){

    buf->last = buf->first = 0;
    buf->cstream_size = cstream_size;
    buf->empty = 1;
    buf->full  = 0;
    buf->data = (uint8_t *) nmalloc(sizeof(uint8_t)*cstream_size);
	buf->size = 0;
}

void cstream_reset (struct cstream *buf){
    buf->last = buf->first = 0;
    buf->empty = 1;
    buf->full  = 0;
	buf->size =0 ;
}


int cstream_insert(
	struct cstream *buf, 
	uint8_t *new_data,
	uint32_t new_data_size 
){
	int i =0;
	for(i =0 ; i <  new_data_size ; i++){
		if(!buf->full){
			buf->data[buf->last] = new_data[i];
			buf->size++;

		    buf->empty = 0; //se inserisci non è piu vuota
		    buf->last ++;    

		    if(buf->last == buf->cstream_size){ 
		        buf->last = 0;
		    }
		    if(buf->last == buf->first){
		        buf->full = 1;
		    }
		}else{
			break;
		}
	}

	return i;	
}

uint32_t cstream_getsize(	struct cstream *buf){
	return (buf->size);
}

int cstream_extract(
	struct cstream *buf, 
	uint8_t *new_data,
	uint32_t new_data_size
){
	int i =0;
  	for(i =0 ; i <  new_data_size ; i++){ 
		if(!buf->empty){

			new_data[i] = buf->data[buf->first] ;
			buf->size--;

		    buf->first += 1;
		    buf->full   = 0; //se estrai non è piu piena

		    if(buf->first == buf->cstream_size){ 
		        buf->first = 0;
		    }

		    if(buf->last == buf->first){
		        buf->empty = 1;
		    }
		}else{
			break;
		}			
	}
	return i;
}






