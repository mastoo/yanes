#ifndef UDP_H
#define UDP_H
#include "netdev.h"
#include <stdint.h>

struct udphdr {
	uint16_t src_prt;
	uint16_t dst_prt;
	uint16_t len;
	uint16_t checksum;
	uint8_t  data[];
} __attribute__((packed));


struct udp_datagram{
	uint16_t src_addr_low;
	uint16_t src_addr_high;
	
	uint16_t dst_addr_low;
	uint16_t dst_addr_high;
		
	uint16_t src_prt;
	uint16_t dst_prt;
	uint16_t len;
	uint8_t  data[65535];
};

void udp_init    (struct netdev  *dev_       );
void udp_incoming(struct eth_hdr *hdr        );
void udp_send    (struct udp_datagram * dgram);
int  udp_available();
void get_next_udp_pkt(struct udp_datagram *dgram);

#endif