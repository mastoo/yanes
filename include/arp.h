#ifndef ARP_H
#define ARP_H
#include "Byte_order.h"
#include "ethernet.h"
#include "netdev.h"
#include "extern.h"

#define S_ARP_ETHERNET    0x0001
#define S_ARP_IPV4        0x0800
#define S_ARP_REQUEST     0x0001
#define S_ARP_REPLY       0x0002


#define S_IP_ALEN 4
#define S_ETH_ALEN	6	/* ether address len */


#define S_ARP_CACHE_LEN   32
#define S_ARP_FREE        0
#define S_ARP_WAITING     1
#define S_ARP_RESOLVED    2

struct arp_hdr
{
    uint16_t hwtype;
    uint16_t protype;
    unsigned char hwsize;
    unsigned char prosize;
    uint16_t opcode;
    unsigned char data[];
} __attribute__((packed));

struct arp_ipv4
{
    unsigned char smac[6];
    uint16_t sip_low;
    uint16_t sip_high;	
    unsigned char dmac[6];
    uint16_t dip_low;
	uint16_t dip_high;
} __attribute__((packed));;

struct arp_cache_entry
{
    uint16_t hwtype;
    uint16_t sip_low;
    uint16_t sip_high;	
    unsigned char smac[6];
    unsigned int state;
};

void arp_init();
void arp_incoming(struct netdev *netdev, struct eth_hdr *hdr);
void arp_reply(struct netdev *netdev, struct eth_hdr *hdr, struct arp_hdr *arphdr);
int  get_dmac(uint16_t daddr_low, uint16_t daddr_high, unsigned char *dmac);
void arp_send_request(struct netdev *netdev, struct eth_hdr *hdr, uint16_t daddr_low, uint16_t daddr_high);

#endif
