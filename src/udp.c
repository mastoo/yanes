#include "nutils.h"
#include "udp.h"
#include "ipv4.h"
#include "cbuf.h"

#include "Byte_order.h"
#include "extern.h"

#define MAX_NUM_UDP_PKT 16

static struct udp_datagram  *curr_dgram;
static struct cbuf			 *udp_dgram_cbuf;
static struct netdev 		 *dev;
static struct eth_hdr 		 *shdr;

void udp_init(struct netdev  *dev_){
	udp_dgram_cbuf = (struct cbuf *) nmalloc(sizeof(struct cbuf));
	curr_dgram     = (struct udp_datagram *) nmalloc(sizeof(struct udp_datagram));

	cbuf_init (udp_dgram_cbuf, MAX_NUM_UDP_PKT, sizeof(struct udp_datagram));
	dev  = dev_;
	shdr  = (struct eth_hdr *)nmalloc(sizeof(struct eth_hdr)+1500);
	//SerialSendString("udp init \n");
}

void udp_datagram_copy(cbuf_pkt_ptr dst_, cbuf_pkt_ptr src_){

	int i =0;
	struct udp_datagram  *dst = (struct udp_datagram  *)dst_;
	struct udp_datagram  *src = (struct udp_datagram  *)src_;
	dst->src_addr_low = src->src_addr_low;
	dst->src_addr_high = src->src_addr_high;
	
	dst->dst_addr_low = src->dst_addr_low;
	dst->dst_addr_high = src->dst_addr_high;
	
	dst->src_prt  = src->src_prt;
	dst->dst_prt  = src->dst_prt;
	dst->len      = src->len;

	for(i =0; i  < dst->len; i++){
		dst->data[i] = src->data[i];
	}
}

int udp_available(){
	return !udp_dgram_cbuf->empty;
}

void get_next_udp_pkt(struct udp_datagram *dgram){
	cbuf_pkt_extract(
		udp_dgram_cbuf, 
		(cbuf_pkt_ptr)dgram, 
		udp_datagram_copy
	);	
}

void udp_incoming( struct eth_hdr *hdr){

	int i = 0;
    struct iphdr  *iphdr  = (struct iphdr * ) hdr->payload;
	struct udphdr *udphdr = (struct udphdr *) iphdr->data;


	curr_dgram->src_addr_low = iphdr->saddr_low;
	curr_dgram->src_addr_high = iphdr->saddr_high;
	
	curr_dgram->dst_addr_low = iphdr->daddr_low;
	curr_dgram->dst_addr_high = iphdr->daddr_high;
	
	curr_dgram->src_prt  = ntohs(udphdr->src_prt);
	curr_dgram->dst_prt  = ntohs(udphdr->dst_prt);
	curr_dgram->len 	 = ntohs(udphdr->len);
	
	for(i = 0; i < curr_dgram->len; i++){
		curr_dgram->data[i] = udphdr->data[i];
	}	
	
	cbuf_pkt_insert(
		udp_dgram_cbuf, 
		(cbuf_pkt_ptr)curr_dgram, 
		udp_datagram_copy
	);
	
	//SerialSendString("udp incoming \n");
}


void udp_send (struct udp_datagram * dgram){
	int i =0;
    struct iphdr  *iphdr  = (struct iphdr  *) shdr->payload;
	struct udphdr *udphdr = (struct udphdr *) iphdr->data;
	udphdr->src_prt   = htons (dgram->src_prt);
	udphdr->dst_prt   = htons (dgram->dst_prt);

	udphdr->len		  = htons(sizeof(struct udphdr) + dgram->len);

	udphdr->checksum  = 0;

	iphdr->tot_len	  = htons(sizeof(struct iphdr)+sizeof(struct udphdr) + dgram->len);
	//printf("\ndgram len %d %d %d %d \n",dgram->len, sizeof(struct iphdr),sizeof(struct udphdr),sizeof(iphdr)+sizeof(struct udphdr) + dgram->len);
	iphdr->id		    = 0;
	iphdr->saddr_low	  = dgram->src_addr_low;
	iphdr->saddr_high	  = dgram->src_addr_high;
	
	iphdr->daddr_low	  = dgram->dst_addr_low;
	iphdr->daddr_high	  = dgram->dst_addr_high;
	iphdr->protocol   	  = S_IP_P_UDP;
////	for(i = 0; i <dgram->len; i++ ){
	//	udphdr->data[i] 	  =  dgram->data[i];
	//}
	//printf("src port dst port %x %x  \n",udphdr->src_prt,udphdr->dst_prt);
	ip_send(dev,shdr);
}
