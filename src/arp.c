#include "arp.h"
#include "netdev.h"
#include <string.h>
/*
 * https://tools.ietf.org/html/rfc826
 */

static const  unsigned char BRD_HWADDR [6] = {0xff,0xff,0xff,0xff,0xff,0xff};

static struct arp_cache_entry arp_cache[S_ARP_CACHE_LEN];

static int insert_arp_translation_table(struct arp_hdr *hdr, struct arp_ipv4 *data)
{
    struct arp_cache_entry *entry;
	int i;
    for ( i = 0; i<S_ARP_CACHE_LEN; i++) {
        entry = &arp_cache[i];

        if (entry->state == S_ARP_FREE) {
            entry->state = S_ARP_RESOLVED;

            entry->hwtype = hdr->hwtype;
            entry->sip_low 	  = data->sip_low;
            entry->sip_high   = data->sip_high;
			
			// printf("********************* src ip  %x \n",entry->sip );
			// printf("insert mac %x:%x:%x:%x:%x:%x\n", data->smac[0],
                                                	// data->smac[1],
                                                	// data->smac[2],
                                                	// data->smac[3],
                                                	// data->smac[4],
                                                	// data->smac[5]);
//            MemCopy(entry->smac, data->smac, sizeof(entry->smac));
            nmemcpy (entry->smac, data->smac, sizeof(entry->smac));
            return 0;
        }
    }

    return -1;
}

static int update_arp_translation_table(struct arp_hdr *hdr, struct arp_ipv4 *data)
{
    struct arp_cache_entry *entry;
	int i;
    for ( i = 0; i<S_ARP_CACHE_LEN; i++) {
        entry = &arp_cache[i];

        if (entry->state == S_ARP_FREE) continue;

        if (entry->hwtype == hdr->hwtype && entry->sip_low == data->sip_low && entry->sip_high == data->sip_high) {
            //MemCopy(entry->smac, data->smac, 6);
            nmemcpy(entry->smac, data->smac, 6);            
            return 1;
        }
    }
    
    return 0;
}

void arp_init()
{
    nmemset(arp_cache, 0, S_ARP_CACHE_LEN * sizeof(struct arp_cache_entry));
}

int  get_dmac(uint16_t daddr_low,uint16_t daddr_high, unsigned char dmac[6]){
	int i;

    struct arp_cache_entry *entry;	
    for ( i = 0; i<S_ARP_CACHE_LEN; i++) {
		entry = &arp_cache[i];
		if (entry->state == S_ARP_FREE) continue;	
		//printf( "######################à dest addr %x %x******\n",daddr,entry->sip);
		if (entry->sip_low == daddr_low && entry->sip_high == daddr_high) {
			
			//MemCopy(dmac,entry->smac, 6);
			nmemcpy(dmac,entry->smac, 6);			
            
            return 1;
        }
	}

	return 0;
}

void arp_send_request(struct netdev *netdev, struct eth_hdr *hdr, uint16_t daddr_low,uint16_t daddr_high )
{
    struct arp_hdr *arphdr;
    struct arp_ipv4 *arpdata;
    int len;

	len = sizeof(struct arp_hdr) + sizeof(struct arp_ipv4);
    arphdr = (struct arp_hdr *) hdr->payload;
    arpdata = (struct arp_ipv4 *) arphdr->data;


	arphdr->hwtype  = htons(S_ARP_ETHERNET);
	arphdr->protype = htons(S_ARP_IPV4);
	arphdr->hwsize  = S_ETH_ALEN;
	arphdr->prosize = S_IP_ALEN;
	arphdr->opcode =  htons(S_ARP_REQUEST);

	/* address */
    //memcpy(arpdata->smac, netdev->hwaddr, 6);
	arpdata->smac[0] = netdev-> hwaddr[0];
	arpdata->smac[1] = netdev-> hwaddr[1];
	arpdata->smac[2] = netdev-> hwaddr[2];
	arpdata->smac[3] = netdev-> hwaddr[3];
	arpdata->smac[4] = netdev-> hwaddr[4];
	arpdata->smac[5] = netdev-> hwaddr[5];	
	
	
	arpdata->sip_low  = netdev->addr_low;
	arpdata->sip_high = netdev->addr_high;
		
	//memcpy(arpdata->dmac, BRD_HWADDR, 6);
	arpdata->dmac[0] = BRD_HWADDR[0];
	arpdata->dmac[1] = BRD_HWADDR[1];
	arpdata->dmac[2] = BRD_HWADDR[2];
	arpdata->dmac[3] = BRD_HWADDR[3];
	arpdata->dmac[4] = BRD_HWADDR[4];
	arpdata->dmac[5] = BRD_HWADDR[5];		
	
	arpdata->dip_low = daddr_low;
	arpdata->dip_high = daddr_high;

	netdev_transmit(netdev, hdr, S_ETH_P_ARP, len, BRD_HWADDR);
}



void arp_incoming(struct netdev *netdev, struct eth_hdr *hdr)
{
    struct arp_hdr *arphdr;
    struct arp_ipv4 *arpdata;
    uint16_t 		hwtype;
    uint16_t 		protype;
    uint16_t     	opcode;

    int merge = 0;

    arphdr = (struct arp_hdr *) hdr->payload;

    hwtype  = ntohs(arphdr->hwtype);
    protype = ntohs(arphdr->protype);
    opcode  = ntohs(arphdr->opcode);



    if (hwtype != S_ARP_ETHERNET) {
        //printf("Unsupported HW type\n");
        return;
    }

    if (protype != S_ARP_IPV4) {
        //printf("Unsupported protocol\n");
        return;
    }

    arpdata = (struct arp_ipv4 *) arphdr->data;

    merge = update_arp_translation_table(arphdr, arpdata);

    if (!(netdev->addr_low == arpdata->dip_low && 
		netdev->addr_high == arpdata->dip_high)) {
        //printf("ARP was not for us\n");
        return;
    }

    if (!merge && insert_arp_translation_table(arphdr, arpdata) != 0) {
       //perror("ERR: No free space in ARP translation table\n"); 
    }

    switch (opcode) {
    case S_ARP_REQUEST:
        arp_reply(netdev, hdr, arphdr);
        break;
    default:
        //printf("Opcode not supported\n");
        break;
    }
}

void arp_reply(struct netdev *netdev, struct eth_hdr *hdr, struct arp_hdr *arphdr) 
{
    struct arp_ipv4 *arpdata;
    int len;

    arpdata = (struct arp_ipv4 *) arphdr->data;

    //memcpy(arpdata->dmac, arpdata->smac, 6);

	arpdata->dmac[0] = arpdata->smac[0];
	arpdata->dmac[1] = arpdata->smac[1];
	arpdata->dmac[2] = arpdata->smac[2];
	arpdata->dmac[3] = arpdata->smac[3];
	arpdata->dmac[4] = arpdata->smac[4];
	arpdata->dmac[5] = arpdata->smac[5];	

    arpdata->dip_low = arpdata->sip_low;
	arpdata->dip_high = arpdata->sip_high;
    //memcpy(arpdata->smac, netdev->hwaddr, 6);
	arpdata->smac[0] = netdev-> hwaddr[0];
	arpdata->smac[1] = netdev-> hwaddr[1];
	arpdata->smac[2] = netdev-> hwaddr[2];
	arpdata->smac[3] = netdev-> hwaddr[3];
	arpdata->smac[4] = netdev-> hwaddr[4];
	arpdata->smac[5] = netdev-> hwaddr[5];
    arpdata->sip_low = netdev->addr_low;
	arpdata->sip_high = netdev->addr_high;

    arphdr->opcode = S_ARP_REPLY;

    arphdr->opcode = htons(arphdr->opcode);
    //arphdr->hwtype = htons(arphdr->hwtype);
    //arphdr->protype = htons(arphdr->protype);

    len = sizeof(struct arp_hdr) + sizeof(struct arp_ipv4);
    netdev_transmit(netdev, hdr, S_ETH_P_ARP, len, arpdata->dmac);
}

