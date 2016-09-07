#ifndef ICMP_H
#define ICMP_H

#include "netdev.h"
#include "ipv4.h"



struct icmp {
	unsigned char icmp_type;
	unsigned char icmp_code;
	unsigned short icmp_cksum;
	union {
		struct {
			unsigned short id;	/* identifier */
			unsigned short seq;	/* sequence number */
		} echo;
		unsigned int gw;		/* icmp redirect gateway */
		unsigned int pad;
	} icmp_un;
	unsigned char icmp_data[];
} __attribute__((packed));
    
void icmp_incoming(struct netdev *netdev,struct eth_hdr *hdr );

#endif
