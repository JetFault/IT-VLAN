#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "linkstate.h"
#include "tap.h"

#define LINE_SIZE 256
#define SEEN_LIMIT 5
/* Reading from the Config file */
struct peerlist* parse_file(char* input_file,struct config* conf){

	FILE* config_file;
	char line_buffer[LINE_SIZE];
	char* line = &line_buffer[0];
	char* tok;
	struct peerlist* list = malloc(sizeof(struct peerlist));
	conf = malloc(sizeof(struct config));
	conf->tap = NULL;
	struct peerlist* ptr = list;
	config_file = fopen(input_file, "r");
	
	if(config_file == NULL)  {
		perror(input_file);
		exit(EXIT_FAILURE);
		
	}

	while(fgets(line, LINE_SIZE, config_file) != NULL) {

		tok = strtok(line," \n");

		if(!tok) {
			continue;
		}

		if(tok[0] == '/') {
			continue;
		} else if(strcmp(tok, "listenPort") == 0) {
			conf->listenport = atoi(strtok(NULL," \n"));
		} else if(strcmp(tok, "linkPeriod") == 0) {
			conf->linkperiod = atoi(strtok(NULL," \n"));
		} else if(strcmp(tok, "linkTimeout") == 0) {
			conf->linktimeout = atoi(strtok(NULL," \n"));
		} else if(strcmp(tok, "peer") == 0) {
			char* temp = strtok(NULL," \n");
			ptr->hostname  = (char*)malloc(sizeof(char)*(strlen(temp) + 1));
			strcpy(ptr->hostname, temp);
			ptr->port=atoi(strtok(NULL, " \n"));
			ptr = ptr->next;
		} else if(strcmp(tok, "quitAfter") == 0) {
			conf->quitafter = atoi(strtok(NULL," \n"));
		} else if(strcmp(tok, "tapDevice") == 0) {
			char* temp = strtok(NULL, " \n");
			conf->tap = (char*)malloc(sizeof(char)*(strlen(temp)+1));
			strcpy(conf->tap, temp);
		}else {
			fprintf(stderr, "Command in config not recognized: %s", tok);
			exit(EXIT_FAILURE);
		}
	}
	fclose(config_file);
	
	return list;
}

void add_member(struct membership_list* members, struct linkstate* link){
	
	pthread_mutex_lock(&(members->lock));

	struct linkstate* temp = in_member_list(members, &(link->local), &(link->remote));

	if(temp){
		if(temp->ID > link->ID){
			delete_member(members, link);
		}else{
			return;
		}
	}else {
		struct linkstate* temp = members->list;
		struct linkstate* ptr = link;
		ptr->next = temp;
		members->list = ptr;
		members->size++;
	}
	pthread_mutex_unlock(&(members->lock));
	
	return;
}


struct linkstate* in_member_list(struct membership_list* members, 
		struct proxy_addr* local, struct proxy_addr* remote){

	pthread_mutex_lock(&(members->lock));
	struct linkstate* ptr = members->list;
	struct linkstate* found = NULL;

	while(ptr != NULL){
		if((compare_proxy_addr(remote, &(ptr->remote)) == 0) && (compare_proxy_addr(local, &(ptr->local)) == 0)){		
			found = ptr;
			break;
		}
		else{
			ptr = ptr->next;
		}
		
	}

	pthread_mutex_unlock(&(members->lock));
	
	return found;
}

/* Delete a member in the membership list
 * param members: membership list
 * param link: linkstate to delete
 */
void delete_member(struct membership_list* members, struct linkstate* link){
	pthread_mutex_lock(&(members->lock));
	struct linkstate* curr = members->list;
	struct linkstate* prev = NULL;
	
	while(curr != NULL)	{

		if(curr == link) { //Found linkstate
			if(prev == NULL) { // Delete Head
				members->list = curr->next;
			} else{
				prev->next = curr->next;
				free(curr);
			}
			members->size--;
			break;
		}
		else{
			prev = curr;
			curr = curr->next;
		}
	}
	pthread_mutex_unlock(&(members->lock));
}

/* Delete expired members in membership list
 * param list: membership list
 * param link_timeout: link_timeout
 */
void delete_expired_members(struct membership_list* members, int link_timeout) {

	struct linkstate* tmp;
  struct linkstate* ptr = members->list;
	
	double curr_time = current_time();
	int micro_timeout = link_timeout*1000;

	while(ptr != NULL)	{
		if((curr_time - ptr->ID) > micro_timeout){
			tmp = ptr;
			ptr = ptr->next;

			delete_member(members, tmp);
		}else {
			ptr = ptr->next;
		}
	}
}

/* */
int compare_last_seen(struct last_seen* seen1, struct last_seen* seen2) {
  
}

/* Helper function to see if you have seen the packet
 *return: 0 if it has seen it, -1 if it has not
 */
int is_seen(struct last_seen_list* list, struct last_seen* seen_item){

		struct last_seen* ptr = list->head;
    struct last_seen* tmp = list->head;
		struct last_seen* prev;
	
		int saw = -1;

		while(ptr != NULL){
      if(compare_last_seen(seen_item, ptr) ==0) {
        saw = 0;
        break;
      }
			if(current_time() - ptr->time_received > SEEN_LIMIT){
				if(prev == NULL){
					tmp = ptr;
					list->head = ptr->next;
					free(tmp);
					ptr = list->head;
				}else{
					prev->next = ptr->next;
					free(ptr);
					ptr = prev->next;
				}
			}
			else {
				prev = ptr;
				ptr = ptr->next;
			}
		}
	
		return saw;

}

/* Add to seen list if not in list. If in list return 0. Else return 1 */
int add_seen(struct last_seen_list* seen_list, struct data_packet* data_pack){
  struct proxy_addr* src = malloc(sizeof(struct proxy_addr));
  struct proxy_addr* dest = malloc(sizeof(struct proxy_addr));
  uint16_t id;

  find_tap_dest(data_pack->datagram, src, dest, &id);

  struct last_seen* seen_item = malloc(sizeof(struct last_seen));
  seen_item->ID = id;
  seen_item->packet_type = data_pack->head.packet_type;
  seen_item->source = src;
  seen_item->dest = dest;
  seen_item->time_received = current_time();


  pthread_mutex_lock(&(seen_list->lock));

  //If seen already return 0
  if(is_seen(seen_list, seen_item) == 0) {
    return 0;
  }

  seen_item->next = seen_list->head;
  seen_list->head = seen_item;
  pthread_mutex_unlock(&(seen_list->lock));

  return 1;

}

void send_probes(struct routes* route_list, struct probereq_list* probe_list){
	
	struct probereq_packet* probereq_pack = malloc(sizeof(struct probereq_packet));
	probereq_pack->ID = current_time();
	
	struct routes* ptr = route_list;

	while(ptr != NULL){
		
		if(send_to(ptr->remote, (void*)probereq_pack,ptr->socket_fd) == -1){
		}

		struct probereq_list* new_probe = malloc(sizeof(struct probereq_list));
		new_probe->link = ptr->link;
		new_probe->ID = probereq_pack->ID;

		tmp = probe_list;
		new_probe->next = tmp;
		probe_list = new_probe;
		
		ptr = ptr->next;
	}

}

uint32_t receive_probe(struct membership_list* member_list, struct routes* route_list, struct probereq_list* probe_list, struct proberes_packet* proberes_pack, unsigned int end_time, struct linkstate* link){
	
	struct probereq_list* ptr = probe_list;

	while(ptr != NULL){
		if(ptr->link == link && proberes_pack->ID == ){
			
		}
		uint32_t diff = end_time -
	
	}
	
}
/* Send a packet to everybody in the route list
 * param route_list: the ptr to the head of the list of routes
 * param packet: packet to send to everybody in route_list
 * return: -1 if there is an error, 0 otherwise
 */
int broadcast(struct routes* route_list, void* packet){
	struct routes* ptr = route_list;

	while(ptr != NULL){
		
		if(send_to(NULL, packet, ptr->socket_fd) == -1){
			return -1;
		}
		ptr = ptr->next;
	}

	return 0;

}
/* Send a linkstate packet to a socket
 * param socket_fd: socket to send to
 * param membership list: list of linkstates to create
 *      a linkstate packet from and send
 * return: Number of neighbors sent
 */
int send_linkstate(int socket_fd, struct linkstate* membership_list) {
  struct linkstate_packet lstate_pack;

  get_local_info(socket_fd, &lstate_pack.source);

  /* Send linkstate packet to socket */
  send_to(NULL, &lstate_pack, socket_fd);
  //TODO: Figure out how to lock here and send
}

int create_linkstate_packet(struct membership_list* member_list, 
    struct proxy_addr* source, struct linkstate_packet* lstate_pack) {

  int list_size = member_list->size;

  if(list_size == 0) {
    #if DEBUG
    printf("Size of 0, not sending\n");
    #endif
    return 0;
  }

  lstate_pack->head.packet_type = PACKET_TYPE_LINKSTATE;
  lstate_pack->head.packet_length = sizeof(uint16_t) +
      sizeof(struct proxy_addr) + (sizeof(struct linkstate)*list_size);
  lstate_pack->num_neighbors = list_size;
  lstate_pack->linkstate_head = member_list->list;

  return list_size;
}

int flood_linkstate(struct routes* route_list, struct linkstate* member_list) {

};
