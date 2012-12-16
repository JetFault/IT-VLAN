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
#include <sys/time.h>

#include "tap.h"
#include "vlanpacket.h"
#include "connect.h"
#include "linkstate.h"

#define DATAGRAM_SIZE 2048

#define PROBE_SCHED 1

struct config* conf;
struct routes* route_list;
struct membership_list* member_list;
struct probereq_list* probe_list;

uint8_t arp_broadcast_mac[6];

struct proxy_addr* local_addr;

struct last_seen_list* seen_list;
int server = 0;
int tcp_fd = -1;
int tap_fd = -1;

int is_alive = 1;

char* TAP_NAME;
char* TAP_MAC;
char* ETH0_MAC;


void* poll_membership_list(void* peers){
  int timeout_passed = 0;
  int timeperiod_passed = 0;

  while(1) {
    /* Every LinkTimeout seconds */
    if(timeout_passed >= conf->linktimeout) {
      /* Delete expired members */
      delete_expired_members(member_list, conf->linktimeout);
      timeout_passed = 0;
    }
    if(timeperiod_passed == conf->linkperiod){
      /* Send linkstate*/ 
      //			flood_linkstate(route_list, member_list->list);
      timeperiod_passed = 0;
    }
    /* Send Probes every 1 sec */
    //send_probes(route_list, probe_list);

    sleep(PROBE_SCHED);
    timeout_passed++;
    timeperiod_passed++;
  }
}

/* Packet retrieval logic
 * param packet: packet typecasted as void*
 * return: -1 on error, 1 on success
 */
int packet_ret_logic(void* packet, int socket_fd) {
  uint16_t pack_type = ((struct data_packet*)packet)->head.packet_type;

  if(pack_type == PACKET_TYPE_DATA) {
    struct data_packet* data_pack = (struct data_packet*)packet;

    /* Already seen this packet */
    if(add_seen(seen_list, packet) == 0) {

    } else {
      struct proxy_addr src, dest, local;
      find_tap_dest(data_pack->datagram, &src, &dest, NULL);
      get_local_info(socket_fd, &local, local_addr);

      //If I am the destination
      if(compare_proxy_addr(&dest, &local) == 0 || 
          arp_broadcast_mac == dest.mac_addr) {
        /* Send to TAP */
        tap_write(tap_fd, data_pack->datagram, data_pack->head.packet_length);
      }
      //I'm not the destination
      else {
        broadcast(route_list, packet);

      }
    }

  } else if(pack_type == PACKET_TYPE_LEAVE) {
    broadcast(route_list, packet);
    struct route* peer_route = get_route_socket(route_list, socket_fd);
    close_peer_route(member_list, route_list, peer_route);

  } else if(pack_type == PACKET_TYPE_QUIT) {
    broadcast(route_list, packet);
    is_alive = 0;

  } else if(pack_type == PACKET_TYPE_LINKSTATE) {
    struct linkstate_packet* lstate_pack = (struct linkstate_packet*)packet;
    add_member(member_list, lstate_pack->linkstate_head);

  } else if(pack_type == PACKET_TYPE_PROBEREQ) {
    /* Send probe res */
    send_to((struct proberes_packet*)packet, socket_fd);

  } else if(pack_type == PACKET_TYPE_PROBERES) {
    /* update RTT based on probe res time */
    //receive_probe(member_list, (struct proberes_packet*)packet);

  } else {
    fprintf(stderr, "Wrong packet type: %u\n", pack_type);
    return -1;
  }

  return 1;
}

void* run_tap_thread(void* arg) {
  int socket_fd = (int)arg;

  int ret_status;

  unsigned short int data_size = 0;

  unsigned short int h_type = 0;
  unsigned short int h_size = 0;
  TAP_MAC = malloc(sizeof(uint8_t)*6);

  if ( (tap_fd = allocate_tunnel(TAP_NAME, IFF_TAP | IFF_NO_PI, TAP_MAC)) < 0 ) {
    perror("Opening tap interface failed! \n");
    exit(1);
  }


  get_tap_info(TAP_NAME, local_addr);

  /* Keep reading from that socket for more datagrams */
  while(is_alive)
  {
    char* buff_tap_datagram = malloc(sizeof(char) * DATAGRAM_SIZE);

    data_size = socket_read(tap_fd, &buff_tap_datagram, DATAGRAM_SIZE);

    if(data_size == 0) {
      continue;
    }

    struct proxy_addr src, dest;


    ret_status = find_tap_dest(buff_tap_datagram, &src, &dest, NULL);
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
    broadcast(route_list, (void*)data_pack);
  }
}

void* run_accept_thread(void* connection_fd) {
  int remote_fd = (int) connection_fd;

  void* pack;
  struct proxy_addr local, remote;

  get_local_info(remote_fd, &local, local_addr);
  get_remote_info(remote_fd, &remote);

  /* Check if host in membership list */
  /* Yes */
  if(in_member_list(member_list, &local, &remote)) {

  } 
  /* No */
  else {
    /* Wait for single record link state packet*/
    /* Read from remote */
    uint16_t pack_type = read_packet(remote_fd, &pack);

    /* If Packet is a single record Linkstate */
    if(pack_type == PACKET_TYPE_LINKSTATE &&
        ((struct linkstate_packet *)pack)->num_neighbors == 1) {

      struct linkstate_packet* link_pack = (struct linkstate_packet*)pack;

      /* Manipulate dest to include your mac address TODO  getlocalinfo */
      struct proxy_addr dest;
      get_local_info(remote_fd, &dest, local_addr);
      link_pack->linkstate_head->remote= dest;

      /* Reverse local & dest and add to membership list  TODO */
      struct linkstate* link = malloc(sizeof(struct linkstate));
      link->local = dest;
      link->remote = link_pack->linkstate_head->local;
      link->ID = current_time();
      link->avg_RTT = 1;
      link->next = NULL;

      /* Add client to Membership list */
      add_member(member_list, link);

      /* Send the unreversed but manipulated lstate pack back, but change source to you TODO */
      link_pack->source = link_pack->linkstate_head->local;
      send_to(link_pack, remote_fd);

    } 
    /* Not single record linkstate, drop client */
    else {
      close(remote_fd);
      return NULL;
    }

  } 

  /* Keep this connection alive
   * If read or write return error, then connection has been closed */
  while(is_alive) {

    /* Read from remote */
    /* Deserialize packet */
    void* pack;
    uint16_t pack_type = read_packet(remote_fd, &pack);
    if(packet_ret_logic(pack, remote_fd) < 0) {
#if DEBUG
      printf("Packet error. Connection might have closed.\n");
#endif
      break;
    }
  }

}

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
  while(is_alive) {
    pthread_t child;

    /* Accept */
    if((connection_fd = accept(socket_fd, NULL, NULL)) <= 0) {
      fprintf(stderr, "Accept failed. Error Code: %d", connection_fd);
      close(socket_fd);
      exit(EXIT_FAILURE);
    }

    pthread_create(&child, NULL, run_accept_thread, (void*) connection_fd);
  }

}

void connect_to_peers(struct peerlist* plist) {
  struct peerlist* ptr = plist;
  struct peerlist* tmp;

  while(ptr != NULL) {
    int connection_fd = connect_to(ptr->hostname, ptr->port);

    /* Send 1 record linkstate packet,
     * with RTT of 1 and ID of current time */
    struct linkstate self_lstate;
    self_lstate.ID = current_time();
    self_lstate.avg_RTT = 1;
    self_lstate.next = NULL;

    struct proxy_addr local, remote;

    if(get_local_info(connection_fd, &local, local_addr) == -1){
      fprintf(stderr, "Could not get local info. Error Code: %d", connection_fd);
      exit(EXIT_FAILURE);
    }
    if(get_remote_info(connection_fd, &remote) == -1){
      fprintf(stderr, "Could not get remote info. Error Code: %d", connection_fd);
      exit(EXIT_FAILURE);
    }
    self_lstate.local = local;
    self_lstate.remote = remote;


    /* Send 1 record linkstate packet of end peer */
    send_to((void*)&self_lstate,connection_fd);

    tmp = ptr;
    ptr = ptr->next;
    free(tmp);
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

  arp_broadcast_mac[0] = 255;
  arp_broadcast_mac[1] = 255;
  arp_broadcast_mac[2] = 255;
  arp_broadcast_mac[3] = 255;
  arp_broadcast_mac[4] = 255;
  arp_broadcast_mac[5] = 255;

  /* Parse config file */
  struct peerlist* peers;
  parse_file(argv[1], conf, peers);

  /* Run TCP Server Thread */
  pthread_create(&tcp_thread, NULL, start_tcp_listener, (void *)tcp_fd);

  local_addr = malloc(sizeof(struct proxy_addr));

  /* Run TAP Thread if device has TAP */
  if(conf->tap != NULL){
    TAP_NAME = conf->tap;
    pthread_create(&tap_thread, NULL, run_tap_thread, (void *)tap_fd);
  } else {
    // Get device MAC address //
    char buffer[256];
    sprintf(buffer,"/sys/class/net/eth0/address");

    char* local_mac = local_addr->mac_addr;
    get_tap_info("eth0", local_addr);

    FILE* f = fopen(buffer,"r");
    fread(buffer,1,256,f);
    sscanf(buffer,"%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",local_mac,local_mac+1,local_mac+2,local_mac+3,local_mac+4,local_mac+5);
  }

  /* Run Polling Thread */
  pthread_create(&poll_thread, NULL, poll_membership_list, (void *)peers); 

  /* Connect to peers */
  connect_to_peers(peers);

  /* Wait for TCP and TAP and Polling thread */
  pthread_join(tcp_thread, NULL);
  if(conf->tap !=NULL){
    pthread_join(tap_thread, NULL);
  }
  pthread_join(poll_thread,NULL);

  return 0;
}
