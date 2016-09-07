#ifndef __NETDEV_H__
#define __NETDEV_H__

#include "nstack_config.h"
#include "ethernet.h"
#include <stdint.h>

struct netdev {
    uint16_t addr_low;
	uint16_t addr_high;
    unsigned char hwaddr[6];
};


void netdev_init(struct netdev *dev, uint32_t addr, char *hwaddr);

void netdev_transmit(struct netdev *dev, struct eth_hdr *hdr, 
                     uint16_t ethertype, int len, unsigned char *dst);
					 
					 
 
#endif
