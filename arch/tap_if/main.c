#include "syshead.h"
#include "tuntap_if.h"
#include "print_utils.h"
#include "ethernet.h"
#include "udp.h"
#include "tcp.h"
#include "netdev.h"
#include "arp.h"
#include "ipv4.h"


/**********************/
/* NETWORK PARAMETERS */
/**********************/
#define BUFLEN 2000



void handle_frame(struct netdev *netdev, struct eth_hdr *hdr)
{
    switch (hdr->ethertype) {
        case ETH_P_ARP:
            arp_incoming(netdev, hdr);
            break;
        case ETH_P_IP:
            ipv4_incoming(netdev, hdr);
            break;
       default:
            printf("Unrecognized ethertype %x\n", hdr->ethertype);
            break;
    }
}

int main(){

    // --- allocate the buffuer of tap interface
    char * buf = (char *) malloc(BUFLEN*sizeof(char));
	if (!buf){
		printf("malloc error\n");
		exit(0);
	}
    
    // --- Allocate the device interface name. Example: tap0
    char *dev_name = calloc(128, 1);
    // --- Init the tap interface and connect it to the 10.0.0.0/24 network
    tun_init(dev_name,"10.0.0.0/24");



    // --- create the virtual interface of the sftp server with ip address 10.0.0.4
    // --- and mac address 00:0c:29:6d:50:25
    struct netdev netdev;
    netdev_init(&netdev, inet_addr("10.0.0.4"), "00:0c:29:6d:50:25");    

    // --- init low level protocol modules
    udp_init(&netdev);
	tcp_init(&netdev);
    arp_init();

	int sk,sk2;
	int nbytes;
    char *str_data = malloc(200);
	sk = accept_new_tcp_conn(80);

    while (1){

        if ((nbytes = tun_read(buf, BUFLEN)) < 0) {
            print_error("ERR: Read from tun_fd: %s\n", strerror(errno));
        }

        //print_hexdump(buf, BUFLEN);

        struct eth_hdr *hdr = init_eth_hdr(buf);
        handle_frame(&netdev, hdr);




		if(is_tcp_closed(sk)) {
		    sk = accept_new_tcp_conn(80);
		}else{
						
			int nbytes =  tcp_recv(sk, str_data , 100);
			if(nbytes){
			
				str_data[nbytes]='\0';
				printf("data from tcp: %d \"%s\" \n",nbytes, str_data);
				tcp_send(sk,str_data,nbytes);
				handle_tcp_outcoming_data();
			}
		}
    }
}
