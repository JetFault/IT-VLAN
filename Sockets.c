#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>

/* Ethernet protocol ID's */	
#define ETHERTYPE_PUP 0x0200 /* Xerox PUP */
#define ETHERTYPE_SPRITE 0x0500 /* Sprite */
#define ETHERTYPE_IP 0x0800 /* IP */
#define ETHERTYPE_ARP 0x0806 /* Address resolution */
#define ETHERTYPE_REVARP 0x8035 /* Reverse ARP */
#define ETHERTYPE_AT 0x809B /* AppleTalk protocol */
#define ETHERTYPE_AARP 0x80F3 /* AppleTalk ARP */
#define ETHERTYPE_VLAN 0x8100 /* IEEE 802.1Q VLAN tagging */
#define ETHERTYPE_IPX 0x8137 /* IPX */
#define ETHERTYPE_IPV6 0x86dd /* IP protocol version 6 */
#define ETHERTYPE_LOOPBACK 0x9000 /* used to test interfaces */

/*
 * Accessory method to gracefully close:
 */
int graceful_close(int flag)
{
	if (flag == -1)
	{
		printf("Usage:/n/tFor the 1st proxy (e.g. on machine X)/n/t/tcs352proxy <port> <local interface>/n/tFor the 2nd proxy (e.g. on machine Y)/n/t/tcs352proxy <remote host> <remote port> <local interface>");
		return -1;
	}
	return 0;
}

/*
 * Create and bind a socket code
 */
int open_socket( int sock_port)
{
	int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(sock_port); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

   
     return listenfd;
}
/*
 * END Create and bind a socket code
 */
 
 
/*
 * Encapsulate(create) and de-encapsulate(interpret) packets code
 */
int encapsulate_packet(char* source_ip)
{

	/*
	 * TODO: Packet should be:
	 * Type (16 bits): Length (16 bits): Data
 	 *
	 * The fields for part 1 are:
	 * -Type: This 16-bit integer in the form 0xABCD  (hexadecimal). It indicates a VLAN packet. Parts 2 and 3 will expand the type field to include more types.  
	 * -Length: The length of the remainder of the data payload, as an unsigned 16 bit integer.  
	 * -Data: The packet as received from tap device.   
	 */
	struct packet_header
	{
		unsigned int source;
		unsigned int dest;
		unsigned char payload[1500];
	};
	
	struct packet_header *header_p = malloc(sizeof(struct packet_header));
	unsigned int src,dst;
	
	
	//get the source IP address
	header_p->source = htonl(/*hex representation of IP address*/);
	header_p->dest = htonl(/*hex representation of IP address*/);
	
	//debugging utilities: just to be able to print what the ehader source and estination values are
	src = header_p->source;
	dst = header_p->dest;
	
	//this will be our packet message (payload) We will fill this with a header and a message
	my_byte = byte_p[1607];
	
	//This will be where we create our packet header, and put it in the payload
	
}

int deencapsulate_packet(char* buffer)
{
	/*Since we know how the packets are encapsulated (see above) we can just parse out our header and our message body*/
}

/*
 * Create our packet headers
 */
int create_ethernet_header()
{
	struct ether_addr
	{
		u_int8_t ether_addr_octet[ETH_ALEN];
	} __attribute__ ((__packed__));
	/* 10Mb/s ethernet header */
	
	struct ether_header
	{
		u_int8_t ether_dhost[ETH_ALEN]; /* destination eth addr */
		u_int8_t ether_shost[ETH_ALEN]; /* source ether addr */
		u_int16_t ether_type; /* packet type ID field */
	} __attribute__ ((__packed__));

	
}
/*
 * END Encapsulate(create) and de-encapsulate(interpret) packets code 
 */

/*
 *TCP socket listen thread code - (inteprets input, and either listens on our socket or creates an outgoing connection our socket, depending on the inputs.
 */
int tcp_listener(int listenfd, int argc, char* argsv)
{
	
	if(argc > 4 || argc < 3)
	{
		printf("Error: usage:");
		return 0;
	}
	
	/* Server Mode */
	if(argc == 4)
	{
		// put socket in listen mode... the arguments are listen(socket_file_descriptor, num_connections_to_queue)... I just chose 10 arbitrarily
		listen(listenfd, 10); 
		//infinite listening loop
		for(;;)
		{
			connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 

			ticks = time(NULL);
			snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
			write(connfd, sendBuff, strlen(sendBuff)); 

			int i, recv_length, sockfd;
			u_char buffer[9000];

			if ((listenfd = socket(PF_INET, SOCK_RAW, IPPROTO_TCP)) == -1)
				// your error message here

				for(i=0; i < 3; i++)
				{
					recv_length = recv(sockfd, buffer, 8000, 0);
					printf("Got a %d byte packet\n", recv_length);
					deencapsulate_packet(buffer);
				}

			//closes each connection as it get it
			close(connfd);
			//sleep for a second to prevent DDOS
			sleep(1);
		}		
	}
	/* Peer Mode */
	else if (argc == 3)
	{
		//Code to connect to the proxy 'server', because this will act as a 'client' machine... basically, create an outgoing connection from our socket.
	}
}
/*
 * END TCP socket listen thread code
 */

/*
 *
 *Local Tap Device code
 * 
 */
int vtap_listener()
{
	//allocate_tunnel();
	char *if_name = "tap0";
 
	if ( (tap_fd = allocate_tunnel(if_name, IFF_TAP | IFF_NO_PI)) < 0 )
	{
		perror("Opening tap interface failed! \n");
		exit(1);
	}
/* now you can read/write on tap_fd */
}

/*
 * I got this code directly from assignment, do not really understand it.
 */
int allocate_tunnel(char *dev, int flags) {
  int fd, error;
  struct ifreq ifr;
  char *device_name = "/dev/net/tun";
  if( (fd = open(device_name , O_RDWR)) < 0 ) {
    perror("error opening /dev/net/tun");
    return fd;
  }
  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = flags;
  if (*dev) {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }
  if( (error = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
    perror("ioctl on tap failed");
    close(fd);
    return error;
  }
  strcpy(dev, ifr.ifr_name);
  return fd;
}
/*
 * end Local Tap Device Code
 */
	
/*
 * 
 * Main method of course... creates threads for the tap device and for the socket.
 * 
 */
int main(int argc, char* argsv)
{
	if(argc != 3 || argc != 4)
	{
		int close = graceful_close(-1);
		return close;
	}
	
	int sock_descr = open_socket(argsv[0]);
	pthread_t vtap_pth, sock_pth;	// this is our thread identifier
	int i = 0;

	/* Create worker thread */
	pthread_create(&vtap_pth,NULL,vtap_listener,"processing...");

	pthread_create(&sock_th,NULL,tcp_listener(sock_descr, argc, argsv),"processing...");

	pthread_kill(&vtap_pth, 0);
	pthread_kill(&sock_pth, 0);
	
	return 0;
}
