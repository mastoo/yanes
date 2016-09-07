#ifndef NUTILS_H
#define NUTILS_H

#include <stdint.h>

uint16_t carry_around_add(uint16_t a, uint16_t b){
	uint32_t c = a+b;
	return (c & 0xffff) + (c >>16);
}

uint16_t checksum(uint8_t *msg, int len){

	uint16_t s = 0;
	int i;
	for(i = 0; i < len ; i+=2){

		uint16_t w = (msg[i]<<8) + (msg[i+1]);
		s = carry_around_add(s, w);
		 
	}
	return ~s & 0xffff;
}


#endif