#include "ipv4.h"
#include "netdev.h"
#include "arp.h"
#include "icmp.h"
#include "nutils.h"
#include "udp.h"
#include "tcp.h"


int if_id  = 0;

void ipv4_incoming(struct netdev *netdev, struct eth_hdr *hdr)
{
    struct iphdr *iphdr = (struct iphdr *) hdr->payload;

    uint16_t tot_len = ntohs(iphdr->tot_len);
    //uint16_t id 	 = ntohs(iphdr->id);
    //uint16_t flags   = ntohs(iphdr->flags);
    //uint16_t check   = ntohs(iphdr->check);
    //uint32_t saddr   = ntohl(iphdr->saddr);
   // uint32_t daddr   = ntohl(iphdr->daddr);



	/* pass to upper-level */
	switch (iphdr->protocol) {
	case S_IP_P_ICMP:
		icmp_incoming(netdev,hdr);
		break;
	 case S_IP_P_TCP:
		 tcp_incoming(hdr);
		 break;
	 case S_IP_P_UDP:
		 udp_incoming(hdr);
		 break;
	default:
		
		break;
	}
}


int ip_send(struct netdev *dev, struct eth_hdr *hdr){

	unsigned char dmac[6];
	struct  iphdr * iphdr = (struct  iphdr *) hdr->payload;
	int len =  ntohs(iphdr->tot_len);

	//crea l'header del pacchetto udp	
	iphdr->version = 4; //ipv4
	iphdr->ihl	   = 5; //5 word

	iphdr->dsfield.dscp = 0;
	iphdr->dsfield.ecn  = 0;
	iphdr->id 	   = ntohs(if_id++);
	iphdr->flags   = 0;
	iphdr->frag_offset = 0;
	iphdr->ttl 	    = 80;
	iphdr->check   = 0;
	iphdr->check   = htons(checksum((uint8_t *) iphdr,sizeof(struct iphdr)));
	
    uint16_t saddr_low  = (iphdr->saddr_low);
    uint16_t daddr_low  = (iphdr->daddr_low);	
    uint16_t saddr_high = (iphdr->saddr_high);
    uint16_t daddr_high = (iphdr->daddr_high);
	
	if(get_dmac(daddr_low,daddr_high, dmac)){


		// printf("ip send pkt dst %x src %x \n",daddr, saddr);
		// printf("mac %x:%x:%x:%x:%x:%x\n", dmac[0],
                                          // dmac[1],
                                          // dmac[2],
                                          // dmac[3],
                                          // dmac[4],
                                          // dmac[5]);
		netdev_transmit(dev, hdr, S_ETH_P_IP, len, dmac);
		return 1;
	}else{
		//printf("pkt dst %x src %x \n",daddr, saddr);
		arp_send_request(dev, hdr, daddr_low, daddr_high);
		//printf("unknown host send arp\n");
		return 0;
	}
}
