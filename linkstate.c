#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linkstate.h"

#define LINE_SIZE 100

void parse_file(char* input_file, packet* link){

	FILE* config_file;
	char line[LINE_SIZE]; //max size of line
	char* tok;

	config_file = fopen(input_file, "r");
	
	if(config_file == NULL)  {
		perror(input_file);
		exit(EXIT_FAILURE);
		
	}

	while(fgets(line, LINE_SIZE, config_file) != NULL){
		tok = strtok(line," \n");

		if(strcmp(tok[0],"/") == 0){
			continue;
		}else if(strcmp(tok, "listenport") == 0) {
			link->listenport = strtok(NULL," \n");
		}else if(strcmp(tok, "linkperiod") == 0) {
			link->linkperiod = strtok(NULL," \n");
		}else if(strcmp(tok, "linkTimeout") == 0) {
			link->linkTimeout = strtok(NULL," \n");
		}else if(strcmp(tok, "peer") == 0){
			link->peer = (char*)malloc(sizeof(char)*strlen(strtok(NULL,"\n"))));
			link->port = strtok(NULL, " \n"); //port number - if not will be null either way
		}else if(strcmp(tok, "quitafter") == 0){
			link->quitafter = strtok(NULL," \n");
		}
	}

	fclose(config_file);

	return;
}
