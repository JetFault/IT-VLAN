#ifndef _LINKSTATE_H_
#define _LINKSTATE_H_

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

/* Parse the config file and put results in conf
 * param input_file: input file
 * param conf: struct config address 
 * return: peerlist of peers to connect to
 */
struct peerlist* parse_file(char* input_file, struct config* conf); 

void delete_expired_members(struct linkstate* list, int link_timeout);

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

int send_linkstate(int socket_fd, struct linkstate* l_state);

#endif
