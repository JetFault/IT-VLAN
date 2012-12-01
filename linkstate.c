#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkstate.h"

#define LINE_SIZE 256
struct packet* config;

/* Reading from the Config file */
parse_file(char* input_file){

	FILE* config_file;
	char line_buffer[LINE_SIZE];
	char* line = &line_buffer[0];
	char* tok;
	config = malloc(sizeof(struct packet));

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
			config->listenport = atoi(strtok(NULL," \n"));
		} else if(strcmp(tok, "linkPeriod") == 0) {
			config->linkperiod = atoi(strtok(NULL," \n"));
		} else if(strcmp(tok, "linkTimeout") == 0) {
			config->linktimeout = atoi(strtok(NULL," \n"));
		} else if(strcmp(tok, "peer") == 0) {
			char* temp = strtok(NULL," \n");
			config->peer = (char*)malloc(sizeof(char)*(strlen(temp) + 1));
			strcpy(config->peer, temp);
			config->port = atoi(strtok(NULL, " \n"));
		} else if(strcmp(tok, "quitAfter") == 0) {
			config->quitafter = atoi(strtok(NULL," \n"));
		} else {
			fprintf(stderr, "Command in config not recognized: %s", tok);
			exit(EXIT_FAILURE);
		}
	}
	fclose(config_file);
	
	return;
}
