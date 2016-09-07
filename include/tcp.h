#ifndef TCP_H
#define TCP_H

#include "netdev.h"

struct tcphdr {
	uint16_t src;	/* source port */
	uint16_t dst;	/* dest port */
	uint16_t seq_low;	/* sequence number */
	uint16_t seq_high;	/* sequence number */
	uint16_t ackn_low;	/* acknowledgment number */
	uint16_t ackn_high;	/* acknowledgment number */
	
#if __BYTE_ORDER == __LITTLE_ENDIAN
	uint16_t	reserved:4;
	uint16_t	doff:4;	/* data offset(head length)in 32 bits long */
	/* control bits */
	uint16_t	fin:1;
	uint16_t	syn:1;
	uint16_t	rst:1;
	uint16_t	psh:1;	/* push */
	uint16_t	ack:1;	/* acknowlegment */
	uint16_t	urg:1;	/* urgent */
	uint16_t	ece:1;	/* See RFC 3168 */
	uint16_t	cwr:1;	/* See RFC 3168 */

#elif __BYTE_ORDER == __BIG_ENDIAN
	uint16_t	doff:4;	/* data offset(head length)in 32 bits long */
	uint16_t	reserved:4;	/* control bits */
	uint16_t	cwr:1;	/* See RFC 3168 */
	uint16_t	ece:1;	/* See RFC 3168 */
	uint16_t	urg:1;	/* urgent */
	uint16_t	ack:1;	/* acknowlegment */
	uint16_t	psh:1;	/* push */
	uint16_t	rst:1;
	uint16_t	syn:1;
	uint16_t	fin:1;
#endif
	uint16_t window;
	uint16_t checksum;
	uint16_t urgptr;		/* urgent pointer */
	uint8_t  data[];
} __attribute__((packed));


void tcp_init(struct netdev  *dev_ );
void tcp_incoming(struct eth_hdr *hdr);

int accept_new_tcp_conn(uint16_t prt);
//create a client socket
void  connect_to(int * sk, uint16_t saddr_low, uint16_t saddr_high, uint16_t sprt, uint16_t dprt);
int is_connected(int sk);


int is_tcp_closed(int sk);
int tcp_recv(int sk,void *buf, uint32_t len);
int tcp_send(int sk,void *buf, uint32_t len);
void handle_tcp_outcoming_data();


#endif
