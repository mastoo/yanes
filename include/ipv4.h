#ifndef IPV4_H
#define IPV4_H

#include "netdev.h"
#include "udp.h"
#define S_IP_P_ICMP 0x01
#define S_IP_P_TCP  0x06
#define S_IP_P_UDP  0x11
#define S_ETH_P_IP	0x0800		/* Internet Protocol packet	*/

struct iphdr {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t ihl : 4;
    uint8_t version : 4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    uint8_t version : 4;
    uint8_t ihl : 4;
#else
# error "Please fix <bits/endian.h>"
#endif
    struct dsfield {
        uint8_t dscp : 6;
        uint8_t ecn : 2;
    } dsfield;
    uint16_t tot_len;
    uint16_t id;
    uint16_t flags : 3;
    uint16_t frag_offset : 13;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint16_t saddr_low;
    uint16_t saddr_high;	
    uint16_t daddr_low;
	uint16_t daddr_high;
	unsigned char data[];	/* data field */
} __attribute__((packed));
    
void ipv4_incoming(struct netdev *netdev, struct eth_hdr *hdr);
int ip_send(struct netdev *dev, struct eth_hdr *hdr);
int sizeof_ipheader(struct iphdr  *iphdr);
#endif