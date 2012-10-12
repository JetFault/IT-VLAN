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
	if (flag == -1)	{

	}
	return 0;
}


/*
 * Create and bind a socket code
 */
int open_socket(int sock_port)
{
	int listenfd = 0;
	struct sockaddr_in serv_addr; 

	char sendBuff[1025];
	time_t ticks; 

	//For clients
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd < 0) {
		printf("Error Opening Socket");
		graceful_close(-2);
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(sendBuff, '0', sizeof(sendBuff)); 

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(sock_port); 

	
	int bind_ret = bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 
	if(bind_ret < 0) {
		printf("Error Binding");
		graceful_close(-3);
	}

	return listenfd;
}


/*
 *TCP socket listen thread code - (inteprets input, and either listens on our socket or creates an outgoing connection our socket, depending on the inputs.
 */
int tcp_listener(int listenfd, int argc, char* argv)
{
	int connfd = 0;

	if(argc > 4 || argc < 3)
	{
		printf("Error: usage:");
		return 0;
	}

	/* Server Mode */
	if(argc == 4)
	{
	}
	/* Peer Mode */
	else if (argc == 3)
	{
		//Code to connect to the proxy 'server', because this will act as a 'client' machine... basically, create an outgoing connection from our socket.
		
		/* TODO: Everything below is stolen from the man page of getaddrinfo */

		struct addrinfo hints;
		struct addrinfo *result, *rp;
		int sfd, s, j;
		size_t len;
		ssize_t nread;
		char buf[BUF_SIZE];

		if (argc < 3) {
			fprintf(stderr, "Usage: %s host port msg...\n", argv[0]);
			exit(EXIT_FAILURE);
		}

		/* Obtain address(es) matching host/port */

		memset(&hints, 0, sizeof(struct addrinfo));
		hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
		hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
		hints.ai_flags = 0;
		hints.ai_protocol = 0;          /* Any protocol */

		s = getaddrinfo(argv[1], argv[2], &hints, &result);
		if (s != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
			exit(EXIT_FAILURE);
		}

		/* getaddrinfo() returns a list of address structures.
			 Try each address until we successfully connect(2).
			 If socket(2) (or connect(2)) fails, we (close the socket
			 and) try the next address. */

		for (rp = result; rp != NULL; rp = rp->ai_next) {
			sfd = socket(rp->ai_family, rp->ai_socktype,
					rp->ai_protocol);
			if (sfd == -1)
				continue;

			if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
				break;                  /* Success */

			close(sfd);
		}

		if (rp == NULL) {               /* No address succeeded */
			fprintf(stderr, "Could not connect\n");
			exit(EXIT_FAILURE);
		}

		freeaddrinfo(result);           /* No longer needed */

		/* Send remaining command-line arguments as separate
			 datagrams, and read responses from server */

		for (j = 3; j < argc; j++) {
			len = strlen(argv[j]) + 1;
			/* +1 for terminating null byte */

			if (len + 1 > BUF_SIZE) {
				fprintf(stderr,
						"Ignoring long message in argument %d\n", j);
				continue;
			}
			if (write(sfd, argv[j], len) != len) {
				fprintf(stderr, "partial/failed write\n");
				exit(EXIT_FAILURE);
			}

			nread = read(sfd, buf, BUF_SIZE);
			if (nread == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}

			printf("Received %ld bytes: %s\n", (long) nread, buf);
		}

	}
}



/**
 * if server use connectfd
 * if client use socketfd


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
 * Usage:
 * Client: cs352proxy <port> <local interface>
 * Server: cs352proxy <remote host> <remote port> <local interface>
 *  The arguments are:
 *  local port: a string that is a number from 1024-65535. This is the TCP port the proxy will accept connections on.  
 *  local interface: A string that defines the local tap device, e.g. tun2  
 *  remote host: A string that defines which peer proxy to connect to. It can be a DNS hostname or dotted decimal notation. 
 *  remote port: This is the remote TCP port the proxy should connect to.  
 *
 *
 * Create threads for the tap device and for the socket.
 */
int main(int argc, char** argv)
{
	
	int 

	/* Server Mode */
	if(argc == 3) {

	} 
	/* Client Mode */
	else if(argc == 4) {

	} 
	/* Wrong Arguments */
	else {
		perror("Usage:
				/tFor the 1st proxy (e.g. on machine X)
					/t/tcs352proxy <port> <local interface>
				/tFor the 2nd proxy (e.g. on machine Y)
					/t/tcs352proxy <remote host> <remote port> <local interface>");
		return -1;
	}

	//int sock_descr = open_socket(argv[0]);
	//thread identifier
	//pthread_t vtap_pth, sock_pth;	
	//int i = 0;

	/* Create tap thread */
	//pthread_create(&vtap_pth,NULL,vtap_listener,"processing...");

	/* Create tcp listener thread */
	//pthread_create(&sock_th,NULL,tcp_listener(sock_descr, argc, argv),"processing...");

	//pthread_join(&vtap_pth, 0);
	//pthread_join(&sock_pth, 0);

	return 0;
}
