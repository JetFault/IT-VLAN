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
//#include <sys/select.h>
#include <sys/time.h>

#include "tap.h"
#include "vlanpacket.h"
#include "connect.h"
#include "linkstate.h"

#define DATAGRAM_SIZE 2048

#define PROBE_SCHED 1

struct config* conf;
struct membership_list* member_list;
int server = 0;
int tcp_fd = -1;
int tap_fd = -1;

char* TAP_NAME;
char* TAP_MAC;


void* poll_membership_list(void* peers){
	int timeout_passed = 0;
	int timeperiod_passed = 0;

	while(1) {
    /* Every LinkTimeout seconds */
    if(time_passed >= conf->linktimeout) {
      /* Delete expired members */
      delete_expired_members(member_list, conf->linktimeout);
      timeout_passed = 0;
    }
		if(timeperiod_passed == conf->linkperiod){

      /* Send linkstate*/ 
			flood_linkstate(route_list, member_list->list);
			timeperiod_passed = 0;
		}
    /* Send Probes every 1 sec */


		sleep(PROBE_SCHED);
    timeout_passed++;
		timeperiod_passed++;
	}
}

/* Packet retrieval logic
 * param packet: packet typecasted as void*
 * return: -1 on error, 1 on success
 */
int packet_ret_logic(void* packet) {
  uint16_t pack_type = ((struct data_packet*)packet)->head.packet_type;
  if(pack_type == PACKET_TYPE_DATA) {
    struct data_packet* data_pack = (struct data_packet*)packet;

    struct proxy_addr src, dest;
    find_tap_dest(data_pack->datagram, &src, &dest);

    if
    
    
  } else if(pack_type == PACKET_TYPE_LEAVE) {

  } else if(pack_type == PACKET_TYPE_QUIT) {

  } else if(pack_type == PACKET_TYPE_LINKSTATE) {
  
  } else if(pack_type == PACKET_TYPE_PROBEREQ) {

  } else if(pack_type == PACKET_TYPE_PROBERES) {

  } else {
    fprintf(stderr, "Wrong packet type: %u\n", pack_type);
    return -1;
  }
}

void* run_tap_thread(void* arg) {
	int socket_fd = (int)arg;
	char *if_name = "tap0";

  int ret_status;

	//int socket_fd = tcp_fd;

	unsigned short int data_size = 0;

	unsigned short int h_type = 0;
	unsigned short int h_size = 0;
	TAP_MAC = malloc(sizeof(char)*6);

	if ( (tap_fd = allocate_tunnel(TAP_NAME, IFF_TAP | IFF_NO_PI, TAP_MAC)) < 0 ) {
		perror("Opening tap interface failed! \n");
		exit(1);
	}

  /* Keep reading from that socket for more datagrams */
	while(1)
	{
    char* buff_tap_datagram = malloc(sizeof(char) * DATAGRAM_SIZE);

		data_size = socket_read(tap_fd, &buff_tap_datagram, DATAGRAM_SIZE);

    if(data_size == 0) {
      continue;
    }

    struct proxy_addr src, dest;


    ret_status = find_tap_dest(buff_tap_datagram, &src, &dest);
    if(ret_status == -1) {
      fprintf(stderr, "Error decoding tap datagram source/destination\n");
      free(buff_tap_datagram);
      continue;
    } else if(ret_status == 1) {
      //Handle ARP
    }

    /* Create a Data packet */
		struct data_packet* data_pack = malloc(sizeof(struct data_packet));
		data_pack->head.packet_type = 0xABCD;
		data_pack->head.packet_length = data_size;
    data_pack->datagram = buff_tap_datagram;

    /* Send the packet to the destination */
    send_to(&dest, (void *)data_pack);
	 }
}

void* run_accept_thread(void* connection_fd) {
  int remote_fd = (int) connection_fd;

  /* Check if host in membership list */
  /* Yes */
  if(in_member_list(remote_fd)) {

  } 
  /* No */
  else {
    /* Wait for single record link state packet*/
    /* Read from remote */
    void* pack;
    uint16_t pack_type = read_packet(remote_fd, &pack);

    /* If Packet is a single record Linkstate */
    if(pack_type == PACKET_TYPE_LINKSTATE) {
      /* Add client to Membership list */
      add_member(remote_fd, (struct linkstate_packet *)pack);

      /* Send Link State packet with RTT of 1 and current time to now */
      struct linkstate* l_state;
      struct proxy_addr local, remote;

      get_local_info(remote_fd, &local);
      get_remote_info(remote_fd, &remote);
      l_state->avg_RTT = 1;
      l_state->ID = current_time();
      l_state->next = NULL;

      send_linkstate(remote_fd, l_state);
    } 
    /* Not single record linkstate, drop client */
    else {
      close(remote_fd);
    }

	


		
	} else {
		/* If not close connection */
		close(remote_fd);
		return;
	}

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

void connect_to_peers(struct peerlist* plist) {
  struct peerlist* ptr = plist;

  while(ptr != NULL) {
    int connection_fd = connect_to(ptr->hostname, ptr->port);

    /* Send 1 record linkstate packet,
     * with RTT of 1 and ID of current time */
    struct linkstate * self_lstate = malloc(sizeof(struct linkstate));
    linkstate->local = ;
    linkstate->local = ;
    linkstate->local = 1;
    linkstate->local = current_time();
    linkstate->local = NULL;

    /* Accept 1 record linkstate packet of end peer */


    
    ptr = ptr->next;
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
	pthread_t tcp_thread, tap_thread, poll_thread;

  if(argc != 2) {
		exit(EXIT_FAILURE);
	}

  /* Parse config file */
  struct peerlist* peers = parse_file(argv[1], conf);

  /* Run TCP Server Thread */
	pthread_create(&tcp_thread, NULL, start_tcp_listener, (void *)tcp_fd);


  /* Run TAP Thread if device has TAP */
	if(conf->tap != NULL){
		TAP_NAME = conf->tap;
		pthread_create(&tap_thread, NULL, run_tap_thread, (void *)tap_fd);
	}

  /* Run Polling Thread */
	pthread_create(&poll_thread, NULL, poll_membership_list, (void *)peers); 

  /* Connect to peers */
  connect_to_peers(peers);

  /* Wait for TCP and TAP and Polling thread */
  (void) pthread_join(tcp_thread, NULL);
  if(conf->tap !=NULL){
  	(void) pthread_join(tap_thread, NULL);
  }
  pthread_join(poll_thread,NULL);

	return 0;
}
