#ifndef ETHERNET_H_
#define ETHERNET_H_

#include <stdint.h>

#define S_ETH_P_ARP	0x0806
#define S_ETH_P_RARP	0x8035

struct eth_hdr 
{
    unsigned char dmac[6];
    unsigned char smac[6];
    uint16_t ethertype;
    unsigned char payload[];
} __attribute__((packed));

struct eth_hdr* init_eth_hdr(char* buf);

#endif