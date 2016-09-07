#include "icmp.h"
#include "Byte_order.h"
#include "nutils.h"

void icmp_incoming(struct netdev *netdev, struct eth_hdr *hdr ){



	struct iphdr *iphdr	   = (struct iphdr *) hdr->payload;
    struct icmp * icmp_hdr = (struct icmp *)  iphdr->data;

	uint16_t tot_len = ntohs(iphdr->tot_len);

	/*if (iphdr->tot_len < ICMP_HRD_SZ) {
		printf ("icmp header is too small\n");
		return;
	}*/

	switch(icmp_hdr->icmp_type){
		case 0x08:{ // echo request
			icmp_hdr->icmp_type = 0;
			//ricalcola il check sum del pachetto icmp
			icmp_hdr->icmp_cksum = 0;
			icmp_hdr->icmp_cksum  = htons(checksum((uint8_t *)icmp_hdr, tot_len - sizeof(struct iphdr)));

			//printf("a icmp request src %x dest %x \n",iphdr->saddr,iphdr->daddr );
			uint16_t taddr_low = iphdr->saddr_low;
			uint16_t taddr_high = iphdr->saddr_high;
			
			iphdr->saddr_low   = iphdr->daddr_low;
			iphdr->saddr_high   = iphdr->daddr_high;
			
			iphdr->daddr_low   = taddr_low;
			iphdr->daddr_high  = taddr_high;
			//printf("b icmp request src %x dest %x \n",iphdr->saddr,iphdr->daddr );
			iphdr->check   = 0;

			//iphdr->tot_len = htons(iphdr->tot_len);
			//iphdr->id 	   = htons(iphdr->id);
			//iphdr->flags   = htons(iphdr->flags);
			//iphdr->check   = htons(iphdr->check);
			//iphdr->saddr   = htonl(iphdr->saddr);
			//iphdr->daddr   = htonl(iphdr->daddr);
			//printf("c icmp request src %x dest %x \n",iphdr->saddr,iphdr->daddr );
			//iphdr->check   = htons(checksum((uint8_t *) iphdr,sizeof(struct iphdr)));


			ip_send(netdev, hdr);
		}break;
	}

}