#ifndef _LINKSTATE_H_
#define _LINKSTATE_H_

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

struct peerlist* parse_file(char* input_file, struct config* conf); 

#endif
