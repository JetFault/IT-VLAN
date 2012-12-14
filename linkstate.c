#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "linkstate.h"

#define LINE_SIZE 256

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

/* Delete expired members in membership list
 * param list: membership list
 * param link_timeout: link_timeout
 */
void delete_expired_members(struct linkstate* list, int link_timeout) {

	struct timeval time;
	struct linkstate* tmp;
  struct linkstate* ptr = list;

	gettimeofday(&time,NULL);

	while(ptr != NULL)	{
		if((time.tv_sec - ptr->ID) > link_timeout){

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
