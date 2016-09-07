#include "Byte_order.h"

#include "netdev.h"
#include "ipv4.h"
#include "tcp.h"
#include "nutils.h"
#include "cstream.h"
#include "extern.h"

//#include "../uart_phy.h"

struct netdev *dev;
struct eth_hdr *shdr;

#define CLOSED       0 
#define LISTEN       1
#define SYN_SENT     2
#define SYN_RECEIVED 3
#define ESTABLISHED  4
#define FIN_WAIT_1   5
#define FIN_WAIT_2   6
#define CLOSE_WAIT   7
#define CLOSING 	 8
#define LAST_ACK	 9
#define TIME_WAIT	 10

#define MAX_NUM_CONNECTION 3

struct tcp_connection{
	int state;
	int sk_num;

	uint16_t dst_prt;
	uint16_t src_prt;

	
	uint32_t seq_number;
  	uint32_t last_acked_number;
    
    
    uint16_t saddr_low;
    uint16_t saddr_high;	
    uint16_t daddr_low;
	uint16_t daddr_high;
	
	uint32_t ack_number;
	uint16_t window_size;

	struct cstream incoming_data;
	struct cstream outcoming_data;  

	uint32_t next_sent;

};

static struct tcp_connection tcp_connection[MAX_NUM_CONNECTION];

uint16_t tcp_checksum(
    struct tcphdr *tcphdr, size_t len, 
    uint16_t saddr_low,
    uint16_t saddr_high,	
    uint16_t daddr_low,
	uint16_t daddr_high );

void tcp_init(struct netdev  *dev_){
	int i = 0;
	dev  = dev_;
	shdr  = (struct eth_hdr *)nmalloc(sizeof(struct eth_hdr)+1500);

	for(i = 0; i < MAX_NUM_CONNECTION; i++){
		tcp_connection[i].state = CLOSED;
		tcp_connection[i].sk_num = i;
		cstream_init(&tcp_connection[i].incoming_data, 1024*1024 );
		cstream_init(&tcp_connection[i].outcoming_data, 1024*1024 );

	}
	
    //printf("tcp_init\n");
}

int accept_new_tcp_conn(uint16_t prt){
	int i = 0;
	for(i = 0; i < MAX_NUM_CONNECTION; i++){
		if(tcp_connection[i].state == CLOSED){
			tcp_connection[i].dst_prt = htons(prt);
			tcp_connection[i].state = LISTEN;
			return tcp_connection[i].sk_num;
		}
	}
	return -1;
}


int send_syn(
	struct eth_hdr *hdr, 
	uint32_t seq_number,
    uint16_t saddr_low,
    uint16_t saddr_high,	
    uint16_t daddr_low,
	uint16_t daddr_high,
	uint16_t sprt, 
	uint16_t dprt){

    struct iphdr  *iphdr  = (struct iphdr * ) hdr->payload;
	struct tcphdr *tcphdr = (struct tcphdr *) iphdr->data;	
	//uint32_t taddr;
	//uint16_t tdst;

	//send  ack
	//scambia gli indirizzi e le porte
	//taddr         = iphdr->daddr;
	//iphdr->daddr  = iphdr->saddr;
	//iphdr->saddr  = taddr;
	//Per il momento inviamo solo un pachetto di ack senza dati e opzioni
	iphdr->tot_len = htons(sizeof(struct iphdr) + sizeof(struct tcphdr));
	//printf("nuovi dati per la connessione %d %d\n",ntohs(tcphdr->seq), ack_number);
	iphdr->protocol = S_IP_P_TCP;
	//tdst = tcphdr->dst;
	//tcphdr->dst = tcphdr->src;
	//tcphdr->src = tdst;
	
	iphdr->daddr_low  = daddr_low; 
	iphdr->daddr_high = daddr_high; 

	iphdr->saddr_low = saddr_low; 
	iphdr->saddr_low = saddr_high; 

	tcphdr->doff = 5; //no options

	//tcphdr->seq  = htonl(seq_number);
	//tcphdr->ackn = 0;
	seq_number = 	htonl(seq_number);
	tcphdr->seq_low   = seq_number & 0xFFFF;
	tcphdr->seq_high  = (seq_number >>16) & 0xFFFF;
	

	tcphdr->ackn_low  =  0;
	tcphdr->ackn_high =  0;
	
	
	tcphdr->syn = 1;
	tcphdr->ack = 0;
	tcphdr->psh = 0;
	tcphdr->src = (sprt);
	tcphdr->dst = (dprt);
	tcphdr->checksum = 0;
	tcphdr->checksum = tcp_checksum(tcphdr, sizeof(struct tcphdr),
		iphdr->saddr_low,
		iphdr->saddr_high,
		iphdr->daddr_low,
		iphdr->daddr_high			 
	);

	//printf("send syn ********************** \n");
	return ip_send(dev,hdr);

}


void  connect_to(int * sk, uint16_t saddr_low, uint16_t saddr_high, uint16_t sprt, uint16_t dprt){
	int i = 0;
	if(*sk < 0){
		for(i = 0; i < MAX_NUM_CONNECTION; i++){
			if(tcp_connection[i].state == CLOSED){
				break;
			}
		}
	}else {
		i = *sk;
	}

	if(tcp_connection[i].state == CLOSED){
		tcp_connection[i].src_prt = htons(sprt);
		tcp_connection[i].dst_prt = htons(dprt);
		tcp_connection[i].daddr_low = saddr_low;
		tcp_connection[i].daddr_high = saddr_high;
		
		tcp_connection[i].saddr_low = dev->addr_low;
		tcp_connection[i].saddr_high = dev->addr_high;
		
		if (send_syn(shdr, 0, 
		             tcp_connection[i].saddr_low, tcp_connection[i].saddr_high, 
		             tcp_connection[i].daddr_low, tcp_connection[i].daddr_high,
		             tcp_connection[i].src_prt,
		             tcp_connection[i].dst_prt )
		    ){
			tcp_connection[i].state = SYN_SENT;
			*sk = tcp_connection[i].sk_num;
		}

		//dovrebbe partire un timer 
	}else if(tcp_connection[i].state == SYN_SENT){
		//se il timer è scaduto rimanda il pacchetto
		// per il momento non fa nulla
		send_syn(shdr, 0, 
		    tcp_connection[i].saddr_low, tcp_connection[i].saddr_high,
		    tcp_connection[i].daddr_low, tcp_connection[i].saddr_low,
		    tcp_connection[i].src_prt,
		    tcp_connection[i].dst_prt 
		);
	}

	*sk = -1;
}

int is_connected(int sk){
	return (sk >= 0  && (tcp_connection[sk].state == ESTABLISHED));
}


int is_tcp_closed(int sk){
	return (tcp_connection[sk].state == CLOSED);
}

//! \brief Calculate the TCP checksum.
//! \param tcphdr The TCP packet.
//! \param len The size of the TCP packet in byte.
//! \param src_addr The IP source address (in network format).
//! \param dest_addr The IP destination address (in network format).
//! \return The result of the checksum.
//! \brief Calculate the TCP checksum.
//! \param tcphdr The TCP packet.
//! \param len The size of the TCP packet in byte.
//! \param src_addr The IP source address (in network format).
//! \param dest_addr The IP destination address (in network format).
//! \return The result of the checksum.
uint16_t tcp_checksum(
    struct tcphdr *tcphdr, size_t len, 
    uint16_t saddr_low,
    uint16_t saddr_high,	
    uint16_t daddr_low,
	uint16_t daddr_high ){
	
	const uint16_t *buf = (uint16_t *)tcphdr;
	//uint16_t *ip_src=(uint16_t *)&src_addr, *ip_dst=(uint16_t *)&dest_addr;
	
	uint32_t sum;
	size_t length=len;

	// Calculate the sum		
	sum = 0;
	while (len > 1)
	{
		sum += *buf++;
		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		len -= 2;
	}

	if ( len & 1 )
		// Add the padding if the packet lenght is odd		//
		sum += *((uint8_t *)buf);

	// Add the pseudo-header					//
//	sum += *(ip_src++);
//	sum += *ip_src;
//	sum += *(ip_dst++);
//	sum += *ip_dst;

	sum += saddr_low;
	sum += saddr_high;
	sum += daddr_low;
	sum += daddr_high;
	
	sum += htons(IPPROTO_TCP);
	sum += htons(length);

	// Add the carries						//
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	// Return the one's complement of sum				//
	return ( (uint16_t)(~sum));
}



int sizeof_tcpdata(struct tcphdr *tcphdr, int tcplen){
	return tcplen - tcphdr->doff*4;
}

int sizeof_ipheader(struct iphdr  *iphdr){
	return iphdr->ihl*4;
}

int sizeof_tcpheader(struct tcphdr *tcphdr){
	return tcphdr->doff*4;
}

int tcp_recv(int sk,void *buf, uint32_t len){
	if(!is_tcp_closed(sk)){
		return  cstream_extract(&tcp_connection[sk].incoming_data,(uint8_t *)buf,len );
	}
	
	return -1;
}

int tcp_send(int sk,void *buf, uint32_t len){
	//printf("tcp send\n");
	if(!is_tcp_closed(sk)){
		return  cstream_insert(&tcp_connection[sk].outcoming_data,(uint8_t *)buf,len );
	}
	return -1;
}

uint32_t make_tcp_data(
	struct eth_hdr *hdr, 
	struct tcp_connection *tcp_conn,
	uint32_t len,	
	uint32_t ack_number, 
	uint32_t seq_number){

	struct iphdr  *iphdr  = (struct iphdr * ) hdr->payload;
	struct tcphdr *tcphdr = (struct tcphdr *) iphdr->data;	
	int tcp_len;
	int ip_tot_len;
	//creal l'heder del pacchetto tcp	
	tcphdr->dst = tcp_conn->src_prt;

	tcphdr->src = tcp_conn->dst_prt;


	seq_number = 	htonl(seq_number);
	tcphdr->seq_low   = seq_number & 0xFFFF;
	tcphdr->seq_high  = (seq_number >>16) & 0xFFFF;
	
	ack_number = htonl(ack_number );
	tcphdr->ackn_low  =  ack_number& 0xFFFF;
	tcphdr->ackn_high = (ack_number >> 16)& 0xFFFF;


	tcphdr->doff = 5; //no options
	tcphdr->ack = 1;
	tcphdr->window  = tcp_conn->window_size;

	tcp_len = tcphdr->doff*4 + len;
	//crea l'header IP
	iphdr->ihl	= 5; //5 word
	ip_tot_len = iphdr->ihl*4 + tcp_len;
	iphdr->tot_len = htons(ip_tot_len);

	iphdr->protocol = S_IP_P_TCP;
	iphdr->saddr_low = tcp_conn->daddr_low;
	iphdr->saddr_high = tcp_conn->daddr_high;
		
	iphdr->daddr_low  = tcp_conn->saddr_low;
	iphdr->daddr_high = tcp_conn->saddr_high;
	
	tcphdr->checksum = 0;
	
	tcphdr->checksum = tcp_checksum(
		tcphdr,tcp_len,
		iphdr->saddr_low, iphdr->saddr_high,
		iphdr->daddr_low, iphdr->daddr_high
	);
	//	UartSendWord(tcphdr->checksum);

	return tcphdr->doff*4;

}

void handle_tcp_outcoming_data(){
	static int sk = 0;
	int j;
    struct iphdr  *iphdr  = (struct iphdr * ) shdr->payload;
	struct tcphdr *tcphdr = (struct tcphdr *) iphdr->data;	
	//printf("handle %d %d \n",sk,tcp_connection[sk].state);
	//scorri le connessioni
	for(j = 0; j < MAX_NUM_CONNECTION; j++){
		if(tcp_connection[sk].state == ESTABLISHED){

			//printf("handle %d %d \n",sk,tcp_connection[sk].state);
            uint32_t stream_offset = tcp_connection[sk].seq_number - tcp_connection[sk].last_acked_number;
			if (stream_offset > ntohs(tcp_connection[sk].window_size)){
			    //ag_printf("not acknoledged\n");
			    continue;
		    }
			    
			uint32_t nbytes;
			nbytes = cstream_getsize(&tcp_connection[sk].outcoming_data);
			//printf("nbytes %d \n",nbytes);
			if (nbytes > 1024)
    			nbytes = 1024;
    			
			if(nbytes>0){
				//ag_printf("nbytes %d \n", nbytes);

				//printf("** offset: %d * \n",stream_offset);
				cstream_extract(
					&tcp_connection[sk].outcoming_data , 
					(uint8_t *)tcphdr+20 ,
					nbytes
				);

				uint32_t offset = make_tcp_data(
					shdr, &tcp_connection[sk],nbytes,
					tcp_connection[sk].ack_number, 
					tcp_connection[sk].seq_number
				);

				tcp_connection[sk].seq_number += nbytes;



				sk = (sk+1) % MAX_NUM_CONNECTION;
				ip_send(dev, shdr);

				break;

			}
			

		}
		sk = (sk+1) % MAX_NUM_CONNECTION; 

	}
}

//gen random seq number
uint32_t gen_random_seq_number(){
	return 0;
}

int send_syn_and_ack(
	struct eth_hdr *hdr, 
	uint32_t ack_number, 
	uint32_t seq_number ){

	uint16_t taddr_low, taddr_high;
	
	uint16_t tdst;
	uint16_t cksum = 0;
    struct iphdr  *iphdr  = (struct iphdr * ) hdr->payload;
	struct tcphdr *tcphdr = (struct tcphdr *) iphdr->data;	

	//send syn + ack
	taddr_low           = iphdr->daddr_low;
	iphdr->daddr_low    = iphdr->saddr_low;
	iphdr->saddr_low    = taddr_low;

	taddr_high          = iphdr->daddr_high;
	iphdr->daddr_high   = iphdr->saddr_high;
	iphdr->saddr_high   = taddr_high;

	tdst = tcphdr->dst ;
	tcphdr->dst = tcphdr->src;
	tcphdr->src = tdst;


	//tcphdr->seq  = htonl(seq_number);
	//tcphdr->ackn = htonl(ack_number );
	
	seq_number = 	htonl(seq_number);
	tcphdr->seq_low   = seq_number & 0xFFFF;
	tcphdr->seq_high  = (seq_number >>16) & 0xFFFF;
	
	ack_number = htonl(ack_number );
	tcphdr->ackn_low  =  ack_number& 0xFFFF;
	tcphdr->ackn_high = (ack_number >> 16)& 0xFFFF;
	
	tcphdr->syn = 1;
	tcphdr->ack = 1;

	tcphdr->checksum = 0;
	tcphdr->checksum = tcp_checksum(tcphdr,ntohs(iphdr->tot_len)-sizeof(struct iphdr),
		iphdr->saddr_low, iphdr->saddr_high,
		iphdr->daddr_low, iphdr->daddr_high
	);
	//UartSendWord(tcphdr->checksum);
    printf("send syn_and_ack\n");
	return ip_send(dev,hdr);
}

void send_ack_and_fin(
	struct eth_hdr *hdr, 
	uint32_t seq_number, 
	uint32_t ack_number ){

    struct iphdr  *iphdr  = (struct iphdr * ) hdr->payload;
	struct tcphdr *tcphdr = (struct tcphdr *) iphdr->data;	
	uint16_t taddr_low;
	uint16_t taddr_high;
	
	uint16_t tdst;

	//send  ack
	//scambia gli indirizzi e le porte
	taddr_low         = iphdr->daddr_low;
	iphdr->daddr_low  = iphdr->saddr_low;
	iphdr->saddr_low  = taddr_low;

	taddr_high         = iphdr->daddr_high;
	iphdr->daddr_high  = iphdr->saddr_high;
	iphdr->saddr_high  = taddr_high;

	//Per il momento inviamo solo un pachetto di ack senza dati e opzioni
	iphdr->tot_len = htons(sizeof_ipheader(iphdr) + sizeof(struct tcphdr));
	//printf("nuovi dati per la connessione %d %d\n",ntohs(tcphdr->seq), ack_number);

	tdst = tcphdr->dst;
	tcphdr->dst = tcphdr->src;
	tcphdr->src = tdst;
	tcphdr->doff = 5; //no options

	//tcphdr->seq  = htonl(seq_number);
	//tcphdr->ackn = htonl(ack_number);
	
	seq_number = 	htonl(seq_number);
	tcphdr->seq_low   = seq_number & 0xFFFF;
	tcphdr->seq_high  = (seq_number >>16) & 0xFFFF;
	
	ack_number = htonl(ack_number );
	tcphdr->ackn_low  =  ack_number& 0xFFFF;
	tcphdr->ackn_high = (ack_number >> 16)& 0xFFFF;
	
	tcphdr->syn = 0;
	tcphdr->ack = 1;
	tcphdr->psh = 0;
	tcphdr->checksum = 0;
	tcphdr->checksum = tcp_checksum(tcphdr, sizeof(struct tcphdr),
		iphdr->saddr_low,		iphdr->saddr_high,
		iphdr->daddr_low, 		iphdr->daddr_high
    );
	//UartSendWord(tcphdr->checksum);

	ip_send(dev,hdr);
}


void tcp_incoming(struct eth_hdr *hdr){
	printf("tcp incoming\n");
	int i = 0;
    struct iphdr  *iphdr  = (struct iphdr * ) hdr->payload;
	struct tcphdr *tcphdr = (struct tcphdr *) iphdr->data;	
	
	if(tcphdr->syn & !tcphdr->ack){ // Nuova connessione 

		for(i = 0; i < MAX_NUM_CONNECTION; i++){
			if(tcp_connection[i].state == LISTEN){ 
				// se la connessione è in ascolto
				//printf("connection listen found\n");
				//controlla che la  porta sia quella aperta
				if(tcp_connection[i].dst_prt == tcphdr->dst){
					printf("port ok \n");

					//instaura la connessione 
					tcp_connection[i].src_prt    = tcphdr->src;
					uint32_t seq = (tcphdr->seq_high << 16) | tcphdr->seq_low;
					
					tcp_connection[i].ack_number = ntohl(seq);
					tcp_connection[i].seq_number = gen_random_seq_number();
					tcp_connection[i].last_acked_number = tcp_connection[i].seq_number;
					
					tcp_connection[i].saddr_low = iphdr->saddr_low;
					tcp_connection[i].saddr_high = iphdr->saddr_high;
					
					tcp_connection[i].daddr_low = iphdr->daddr_low;
					tcp_connection[i].daddr_high = iphdr->daddr_high;
					
					tcp_connection[i].window_size = tcphdr->window;
					tcp_connection[i].state = SYN_RECEIVED;

					//il prossimo numero di sequenza è ack +1
					tcp_connection[i].ack_number += 1;

					//send syn + ack
					
					if(!send_syn_and_ack(
							hdr,
							tcp_connection[i].ack_number, 
							tcp_connection[i].seq_number)
					   ){
						//la connessione non è stata stabilità perche nella tabella arp 
						//non c'era nessuno
						//riporta la connessione nello stato LISTEN
						tcp_connection[i].state = LISTEN;
					}

					break;
				}
			}
		}
	
	}else if(tcphdr->ack){

		for(i = 0; i < MAX_NUM_CONNECTION; i++){
			if(tcp_connection[i].state == SYN_RECEIVED){
				if(tcp_connection[i].dst_prt == tcphdr->dst){

					tcp_connection[i].state = ESTABLISHED;
					//verifica che il pachetto è stato riconosciuto
					uint32_t ackn = (tcphdr->ackn_high << 16) | tcphdr->ackn_low;

					if(ntohl(ackn)-1 == tcp_connection[i].seq_number){
						tcp_connection[i].seq_number++; 
						//se ho ricevuto l'ack per il syn allora 
						//posso passare aventi nella sequenza
						//printf("connessione established \n");
					}else{						
						send_syn_and_ack(
							hdr,
							tcp_connection[i].ack_number, 
							tcp_connection[i].seq_number
						);
					}
					
					break;
				}					
			}else if(tcp_connection[i].state == ESTABLISHED){
				//è arrivato un ack, controlla che sia riferito ad una connesione attiva
				//una connessione è identificata dalla coppia di porte di destinazione 
				//e dalla coppia di porte di sorgente e dalla coppia di indirizzi ip 
				if(tcp_connection[i].dst_prt == tcphdr->dst &&
				   tcp_connection[i].src_prt == tcphdr->src &&
				   tcp_connection[i].saddr_low == iphdr->saddr_low   &&
   				   tcp_connection[i].saddr_high == iphdr->saddr_high &&
				   tcp_connection[i].daddr_low == iphdr->daddr_low   &&
   				   tcp_connection[i].daddr_high == iphdr->daddr_high
				  ){

					//uint16_t taddr_low;
					//uint16_t taddr_high;
					
					//uint16_t tdst;
					//Calcola la lunghezza del pacchetto tcp
					uint32_t tcp_len  = ntohs(iphdr->tot_len) - sizeof_ipheader(iphdr);
					//calcola la lunghezza del campo dati del tcp
					uint32_t data_len = sizeof_tcpdata(tcphdr,tcp_len );
					//il prossimo sequence number che lui vuole è
					uint32_t ackn = (tcphdr->ackn_high << 16) | tcphdr->ackn_low;
					tcp_connection[i].last_acked_number = ntohl(ackn);


					//se ci sono dati nel pacchetto

					if(data_len > 0){
						//controlla che i nuovi dati arrivati siano nel corretto ordine
						uint32_t seq = (tcphdr->seq_high << 16) | tcphdr->seq_low;

						if(ntohl(seq) == tcp_connection[i].ack_number){
							//printf("dati arrivati nell'ordine corretto\n");
							//Inseriscili nel buffer
							//incrementa il valore di ack 
							tcp_connection[i].ack_number += data_len;
							if(data_len > 0){
								int insert_bytes = cstream_insert(
									&tcp_connection[i].incoming_data,
									((uint8_t *)(tcphdr)) + sizeof_tcpheader(tcphdr),
									data_len
								);
								//printf("inseriti nuovi dati\n");
    		                    //printf("datalen %d %d \n", data_len, insert_bytes);
							}


						}

						//send  ack
						//scambia gli indirizzi e le porte
						//taddr         = iphdr->daddr;
						//iphdr->daddr  = iphdr->saddr;
						//iphdr->saddr  = taddr;
						//Per il momento inviamo solo un pachetto di ack senza dati
						//iphdr->tot_len = htons(sizeof_ipheader(iphdr) + sizeof(struct tcphdr));
						//printf("nuovi dati per la connessione %d %d\n",ntohs(tcphdr->seq), tcp_connection[i].ack_number);
		
						//tdst = tcphdr->dst;
						//tcphdr->dst = tcphdr->src;
						//tcphdr->src = tdst;
						//tcphdr->doff = 5; //no options
			
						if(tcphdr->fin == 1){
							tcp_connection[i].ack_number +=1;
							tcp_connection[i].state = CLOSE_WAIT;
							//printf ("*******************  ****************** chiusura connssione \n");
						}
						send_ack_and_fin(hdr,tcp_connection[i].seq_number,tcp_connection[i].ack_number);
						//tcphdr->seq  = htonl(tcp_connection[i].seq_number);
						//tcphdr->ackn = htonl(tcp_connection[i].ack_number);
						//tcphdr->syn = 0;
						//tcphdr->ack = 1;
						//tcphdr->psh = 0;
						//tcphdr->checksum = 0;
						//tcphdr->checksum = tcp_checksum(tcphdr, sizeof(struct tcphdr),
						//	iphdr->saddr,
						//	iphdr->daddr			 
						//);

						//ip_send(dev,hdr);
					}else{
						//printf("pacchetto di dati senza ack\n");
						if(tcphdr->fin == 1){
							tcp_connection[i].ack_number +=1;
							tcp_connection[i].state = CLOSE_WAIT;
							//printf ("*******************  ****************** chiusura connssione \n");
						}
						send_ack_and_fin(hdr,tcp_connection[i].seq_number,tcp_connection[i].ack_number);

					}
					
					break;				
				}
			}else if(tcp_connection[i].state == CLOSE_WAIT){

					
					//printf ("CONNESSION CHIUSA CHIUSA CHIUSA CHIUSA CHIUSA CHIUSA CHIUSA CHIUSA CHIUSA \n");
					tcp_connection[i].state = CLOSED;
					cstream_reset(&tcp_connection[i].incoming_data);
					cstream_reset(&tcp_connection[i].outcoming_data);
					break;
	
			}
		}
		
		//in tutti gli altri casi 
		//send a RST
		if(i == MAX_NUM_CONNECTION){
			uint16_t taddr_low;
			uint16_t taddr_high;
			
			
			uint16_t tdst;
			uint32_t tcp_len  = ntohs(iphdr->tot_len) - sizeof_ipheader(iphdr);
			uint32_t data_len = sizeof_tcpdata(tcphdr,tcp_len );

			
			//send  ack
			//scambia gli indirizzi e le porte
			taddr_low          = iphdr->daddr_low;
			iphdr->daddr_low   = iphdr->saddr_low;
			iphdr->saddr_low   = taddr_low;
			
			taddr_high         = iphdr->daddr_high;
			iphdr->daddr_high  = iphdr->saddr_high;
			iphdr->saddr_high  = taddr_high;
			
			//Per il momento inviamo solo un pachetto di ack senza dati
			iphdr->tot_len = htons(sizeof_ipheader(iphdr) + sizeof(struct tcphdr));
					
			tdst = tcphdr->dst;
			tcphdr->dst = tcphdr->src;
			tcphdr->src = tdst;
			tcphdr->doff = 5; //no options
			
			tcphdr->syn = 0;
			tcphdr->ack = 1;
			tcphdr->psh = 0;
			tcphdr->rst = 1;
			tcphdr->checksum = 0;
			tcphdr->checksum = tcp_checksum(tcphdr, sizeof(struct tcphdr),
				iphdr->saddr_low, iphdr->saddr_high,
				iphdr->daddr_low, iphdr->daddr_high
			);

			ip_send(dev,hdr);	

		}	
	}
}
