#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
