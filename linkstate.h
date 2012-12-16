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

struct last_seen_list{
	struct last_seen* head;
	pthread_mutex_t lock;
};

struct last_seen {
	uint16_t ID;
	uint16_t packet_type;
	struct proxy_addr* source;
	struct proxy_addr* dest;
  int time_received;
	struct last_seen* next;
};

//Membership List
struct membership_list {
	struct linkstate* list;
	int size;
	pthread_mutex_t lock;
};

struct probereq_list {
  struct linkstate* link;
  uint64_t ID;
  struct probereq_list* next;
};

struct routes {
  struct route* head;
};

struct route {
  int socket_fd;
  struct linkstate* link;
  struct route* next;
};

/* Parse the config file and put results in conf
 * param input_file: input file
 * param conf: struct config address 
 * return: peerlist of peers to connect to
 */
struct peerlist* parse_file(char* input_file, struct config* conf, struct peerlist* peers); 

/* MEMBERSHIP LIST */

void delete_member(struct membership_list* members, struct linkstate* link);

void delete_expired_members(struct membership_list* members, int link_timeout);

void add_member(struct membership_list* members, struct linkstate* link);

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

struct route* get_route_socket(struct routes* route_list, int socket_fd);
struct route* get_route_link(struct routes* route_list, struct linkstate* link);

/* Close peer route connections and remove from route_list
 *    peer_route param is optional, if NULL clears entire list
 * param route_list: routes struct
 * param peer_route: if not NULL, delete that specific route.
 *                   if NULL, delete all routes
 */
int close_peer_route(struct membership_list* member_list, struct routes* route_list,
      struct route* peer_route);

/*
 * return: 0 for okay, -1 for ERROR
 */
int broadcast(struct routes* route_list, void* packet);

int send_linkstate(int socket_fd, struct linkstate* l_state);

int add_seen(struct last_seen_list* seen_list, struct data_packet* data_pack);

int destroy_vlan(struct membership_list* member_list, struct routes* routes_list, 
    struct probereq_list* probe_list);
/*
int send_probes(struct routes* route_list, struct probereq_list* probe_list);

uint32_t receive_probe(struct membership_list* member_list ,struct probereq_list* probe_list, struct proberes_packet* proberes_pack, unsigned int end_time, struct linkstate* link);
*/
#endif
