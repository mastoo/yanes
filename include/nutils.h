#ifndef NUTILS_H
#define NUTILS_H

#include <stdint.h>

uint16_t checksum(uint8_t *msg, int len);
extern void     MemCopy(char *dst,char *src,int len);

#endif