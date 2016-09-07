#include "netdev.h"
#include "Byte_order.h"
#include "extern.h"



void netdev_init(struct netdev *dev, uint32_t addr, char *hwaddr){
	dev-> addr_low  = addr & 0xFFFF;
	dev-> addr_high = addr >> 16;
	dev-> hwaddr[0] = hwaddr[0];
	dev-> hwaddr[1] = hwaddr[1];
	dev-> hwaddr[2] = hwaddr[2];
	dev-> hwaddr[3] = hwaddr[3];
	dev-> hwaddr[4] = hwaddr[4];
	dev-> hwaddr[5] = hwaddr[5];
	

}

void netdev_transmit(struct netdev *dev, struct eth_hdr *hdr, 
                     uint16_t ethertype, int len, unsigned char *dst){

    hdr->ethertype = htons(ethertype);					 
					 
	hdr->smac[0] = dev-> hwaddr[0];
	hdr->smac[1] = dev-> hwaddr[1];
	hdr->smac[2] = dev-> hwaddr[2];
	hdr->smac[3] = dev-> hwaddr[3];
	hdr->smac[4] = dev-> hwaddr[4];
	hdr->smac[5] = dev-> hwaddr[5];					 


	hdr->dmac[0] = dst[0];
	hdr->dmac[1] = dst[1];
	hdr->dmac[2] = dst[2];
	hdr->dmac[3] = dst[3];
	hdr->dmac[4] = dst[4];
	hdr->dmac[5] = dst[5];
	
	len += sizeof(struct eth_hdr);
 
 
	physendframe((unsigned char*) hdr,len);

}
					 
					 
 
