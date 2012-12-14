#ifndef _LINKSTATE_H_
#define _LINKSTATE_H_

#include <pthread.h>
#include "vlanpacket.h"

struct config {
	int listenport;
	int linkperiod;
	int linktimeout;
	char* tap;
	int quitafter;
};

struct peerlist {
	char* hostname;
	int port;
	struct peerlist* next;
};

struct last_seen {
	uint64_t ID;
	struct proxy_addr source;
	struct proxy_addr dest;
	struct last_seen* next;
};

//Membership List
struct membership_list {
	struct linkstate* list;
	int size;
	pthread_mutex_t lock;
}

struct probereq_list {
  struct proxy_addr remote;
  uint64_t ID;
  double start_time;
  struct probereq_list* next;
};

struct routes {
  int socket_fd;
  struct proxy_addr remote;
};

/* Parse the config file and put results in conf
 * param input_file: input file
 * param conf: struct config address 
 * return: peerlist of peers to connect to
 */
struct peerlist* parse_file(char* input_file, struct config* conf); 

/* MEMBERSHIP LIST */

void delete_member(struct membership_list* members, struct linkstate* link);

void delete_expired_members(struct membership_list* members, int link_timeout);

void add_member(struct membership_list* members, struct linkstate* link);

void add_members(struct membership_list* members, struct linkstate* link_list);

struct linkstate* in_member_list(struct membership_list* members,
    struct proxy_addr* local, struct proxy_addr* remote);

/* Checks membership list for the next peer to send to
 * If the destination is directly connected, send to neighbor
 * If not, check your directed graph for all the edges
 *  that lead to final destination. And return the one that
 *  is closest to destination based on RTT
 *  DIJKSTRA
 * pararm dest: final destination struct proxy_addr
 * return: connection file descriptor of who to send to
 */
int get_peer_route(struct proxy_addr* dest);

int broadcast(struct routes* route_list, void* packet);

int send_linkstate(int socket_fd, struct linkstate* l_state);

/* Helper function to see if you have seen the packet in last 5 sec
 *return: 0 if it has seen it, -1 if it has not
 */
int is_seen(struct last_seen* list, void* packet, struct proxy_addr* source, struct proxy_addr* dest);

void send_probes(struct routes* route_list, struct probereq_list* probe_list);

uint32_t receive_probe(struct membership_list* member_list, struct proberes_packet* proberes_pack);

#endif
