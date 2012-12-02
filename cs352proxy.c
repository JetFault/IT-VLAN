#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>

#include "tap.h"
#include "socket_read_write.h"
#include "server.h"
#include "client.h"
#include "linkstate.h"

#define HEADER_SIZE_SIZE 2
#define HEADER_TYPE_SIZE 2
#define DATAGRAM_SIZE 2048 - HEADER_SIZE_SIZE - HEADER_TYPE_SIZE

struct config* conf;
int server = 0;
int tcp_fd = -1;
int tap_fd = -1;

char* TAP_NAME;

void* run_tap_thread(void* arg) {
	int socket_fd = (int)arg;
	char *if_name = "tap0";

	//int socket_fd = tcp_fd;

	unsigned short int data_size = 0;

	char* buff_tap_datagram = malloc(sizeof(char) * DATAGRAM_SIZE);
	unsigned short int h_type = 0;
	unsigned short int h_size = 0;
	char* local_mac = malloc(sizeof(char)*6);

	if ( (tap_fd = allocate_tunnel(TAP_NAME, IFF_TAP | IFF_NO_PIi,local_mac)) < 0 ) {
		perror("Opening tap interface failed! \n");
		exit(1);
	}

	for(;;)
	{
		data_size = socket_read(tap_fd, buff_tap_datagram, DATAGRAM_SIZE);

    if(data_size == 0) {
      continue;
    }

		h_type = htons(0xABCD);
		h_size = htons(data_size);

		socket_write(socket_fd, (char *)&h_type, HEADER_TYPE_SIZE);
		socket_write(socket_fd, (char *)&h_size, HEADER_SIZE_SIZE);
		socket_write(socket_fd, buff_tap_datagram, data_size);
	}
}

void* run_tcp_thread(void* socket_arg) {
	unsigned short int h_type = 0;
	unsigned short int h_size = 0;
	char* buff_datagram = malloc(sizeof(char) * DATAGRAM_SIZE);
	char* buff_tap_datagram = malloc(sizeof(char) * DATAGRAM_SIZE);

	int socket_fd = (int)socket_arg;

	for(;;)
	{
		//Get Type
		socket_read(socket_fd, (char*)&h_type, HEADER_TYPE_SIZE);

		//Correct packet type
		if(ntohs(h_type) == 0xABCD) {

			/* READ FROM NETWORK */

			//Get length
			socket_read(socket_fd, (char *)&h_size, HEADER_SIZE_SIZE);
			unsigned short int packet_length = ntohs(h_size);

			//Get Datagram
			socket_read(socket_fd, buff_datagram, packet_length);

			/* END READ FROM NETWORK */



			/* WRITE TO TAP */

			socket_write(tap_fd, buff_datagram, packet_length);

			/* END WRITE TO TAP */



			/* READ FROM THE TAP */

			unsigned short int data_size = 0;
			data_size = socket_read(tap_fd, buff_tap_datagram, DATAGRAM_SIZE);

			/* Write to Network */
			h_type = htons(0xABCD);
			h_size = htons(data_size);
			socket_write(socket_fd, (char *)&h_type, HEADER_TYPE_SIZE);
			socket_write(socket_fd, (char *)&h_size, HEADER_SIZE_SIZE);
			socket_write(socket_fd, buff_tap_datagram, data_size);

			/* END READ FROM THE TAP */
		}
	}
}

/**
 * Usage:
 * Server: cs352proxy <port> <local interface>
 * Client: cs352proxy <remote host> <remote port> <local interface>
 *  The arguments are:
 *  local port: 			a string that is a number from 1024-65535. The proxy will accept connections on this.
 *  local interface: 	A string that defines the local tap device, e.g. tun2  
 *  remote host: 			A string that defines which peer proxy to connect to.
 *  									It can be a DNS hostname or dotted decimal notation. 
 *  remote port: 			This is the remote TCP port the proxy should connect to.  
 *
 * Create threads for the tap device and for the socket.
 */
int main(int argc, char** argv) {
	pthread_t tcp_thread, tap_thread;

	if(argc == 2)
		struct peerlist* list = parse_file(argv[1], conf);
	else
		exit(EXIT_FAILURE);

	
 /* if(argc == 3 || argc == 4) {*/
    /* Server Mode */
    /*if(argc == 3) {
      char * tap_arg = argv[2];
      TAP_NAME = malloc(strlen(tap_arg) + 1);
      TAP_NAME = strcpy(TAP_NAME, tap_arg);
      server = 1;
      tcp_fd = server_connect(atoi(argv[1]));
    } */
    /* Client Mode */
    /*
    else if(argc == 4) {
      char * tap_arg = argv[3];
      TAP_NAME = malloc(strlen(tap_arg) + 1);
      TAP_NAME = strcpy(TAP_NAME, tap_arg);
      server = 0;
      tcp_fd = client_connect(argv[1], atoi(argv[2]));
    } 
    int socket_fd = initialize_tcp();
  }*/
	/* Wrong Arguments */
/*	else {
		perror("Usage: \
				\tFor the 1st proxy (e.g. on machine X) \
				\t\tcs352proxy <port> <local interface> \
				\tFor the 2nd proxy (e.g. on machine Y) \
				\t\tcs352proxy <remote host> <remote port> <local interface>");
		return -1;
	}*/

	pthread_create(&tcp_thread, NULL, run_tcp_thread, (void *)tcp_fd);

	if(config->tap != NULL){
		TAP_NAME = config->tap;
		pthread_create(&tap_thread, NULL, run_tap_thread, (void *)tap_fd);
	}

  (void) pthread_join(tcp_thread, NULL);
  if(config->tap !=NULL){
  	(void) pthread_join(tap_thread, NULL);
  } 

	return 0;
}
