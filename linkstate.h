#ifndef _LINKSTATE_H_
#define _LINKSTATE_H_

struct packet {
	int listenport;
	int linkperiod;
	int linkTimeout;
	char* peer;
	int quitafter;
};


#endif
