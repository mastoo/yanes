#ifndef __EXTERN_H__
#define __EXTERN_H__
					 

extern uint32_t physendframe    (unsigned char *hdr,   uint32_t  len);
extern    void* nmalloc         (uint32_t size);
extern    void  nmemcpy         (void *dst, void *src, uint32_t size);
extern    void  nmemset         (void *ptr, int val,   uint32_t size); 
#endif
