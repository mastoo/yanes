#include "tuntap_if.h"
#include "syshead.h"


uint32_t physendframe(unsigned char *hdr,uint32_t  len){
    return tun_write(hdr,  len);
}

void* nmalloc         (uint32_t size){
    return malloc(size);
}

void  nmemcpy         (void *dst, void *src, uint32_t size){
    memcpy(dst,src,size);
}

void  nmemset         (void *ptr, int val,   uint32_t size){
    memset(ptr,val,size);
}
