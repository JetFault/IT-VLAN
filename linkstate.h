#ifndef _LINKSTATE_H_
#define _LINKSTATE_H_

struct packet {
	int listenport;
	int linkperiod;
	int linktimeout;
	char* peer;
	int port;
	int quitafter;
};


#endif
