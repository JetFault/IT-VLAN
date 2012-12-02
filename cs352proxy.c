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
#include "vlanpacket.h"
#include "connect.h"
#include "linkstate.h"

struct config* conf;
struct linkstate* list;
int server = 0;
int tcp_fd = -1;
int tap_fd = -1;

char* TAP_NAME;

void delete_members() {

	struct timeval time;
  struct linkstate* ptr = list;
	struct linkstate* tmp;

	gettimeofday(&time,NULL);

	while(ptr != NULL)	{
		if((time.tv_sec - ptr->ID) > conf->linktimeout){

      if(ptr == list) { // Delete Head
        struct linkstate* t = ptr;
				list = list->next;
				ptr = list;
        free(t);

      } else {
				tmp->next = ptr->next;
				free(ptr);
				ptr = tmp->next;
      }
		}
		else{
			tmp = ptr;
			ptr = ptr->next;
		}

	}
}

void poll_membership_list(){
	
	for(;;){
	/*send packets */
		delete_members();
		sleep(conf->linkperiod);
	}
}


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

void* run_accept_thread(void* connection_fd) {
  int remote_fd = (int) connection_fd;

  /* Wait for single record link state packet*/

  /* Read from remote */

  /* Deserialize packet and check if Linkstate & 1 record only */
    /* Add client to Membership list */
    
    /* Send Link State packet with RTT of 1 and current time to now */

  /* If not close connection */
  close(remote_fd);

  /* Keep this connection alive */
  while(1) {

    /* Read from remote */
    /* Deserialize packet */
      /* Part 2: Data packets get sent to local tap */

  }

};

void* start_tcp_listener(void* socket_arg) {
	int socket_fd = (int)socket_arg;
  int listen_port = conf->listenport;

  int connection_fd;

  struct sockaddr_in socket_info;

  /* Create the socket info */
  if(get_socket_info(NULL, listen_port, &socket_info)) {
    fprintf(stderr, "Failure getting socket info");
    exit(EXIT_FAILURE);
  }

  /* Bind */
  if(bind(socket_fd, (struct sockaddr*)&socket_info, sizeof(socket_info))) {
    fprintf(stderr, "Binding failed.");
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  /* Listen */
  errno = 0;
  if(listen(socket_fd, 10) == -1) {
    fprintf(stderr, "Listening on device Failed. Error: %s\n", strerror(errno));
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  /* Accept connections, and spawn new threads for each connection */
  while(1) {
    pthread_t child;

    /* Accept */
    if((connection_fd = accept(socket_fd, NULL, NULL)) <= 0) {
      fprintf(stderr, "Accept failed. Error Code: %d", connection_fd);
      close(socket_fd);
      exit(EXIT_FAILURE);
    }

    pthread_create(&child, NULL, run_accept_thread, (void*) connection_fd);
  };

};

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
	pthread_t tcp_thread, tap_thread, poll_thread;

	if(argc == 2){
		struct peerlist* peers = parse_file(argv[1], conf);
	}
	else{
		exit(EXIT_FAILURE);
	}

	
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

	if(conf->tap != NULL){
		TAP_NAME = conf->tap;
		pthread_create(&tap_thread, NULL, run_tap_thread, (void *)tap_fd);
	}
	pthread_create(&poll_thread, NULL, poll_membership_list); 

  (void) pthread_join(tcp_thread, NULL);
  if(conf->tap !=NULL){
  	(void) pthread_join(tap_thread, NULL);
  }
  pthread_join(poll_thread,NULL);

	return 0;
}
