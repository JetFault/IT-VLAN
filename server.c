#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include "server.h"

/**
 * Connect server to a port.
 * param port: port number
 * return: file descriptor for the socket
 */
int server_connect(char* port) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, accept_sfd, ret_status;

	/* Set up getaddrinfo for connecting to host */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	ret_status = getaddrinfo(NULL, port, &hints, &result);
	if (ret_status != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret_status));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
		 Try each address until we successfully bind(2).
		 If socket(2) (or bind(2)) fails, we (close the socket
		 and) try the next address. */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);

		if (sfd == -1)
			continue;

		if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
			break;                  /* Success */
		}

		close(sfd);
	}

	//No address succeeded
	if (rp == NULL) {
		fprintf(stderr, "Could not bind, no address succeeded.\n");
		exit(EXIT_FAILURE);
	}

	if(listen(sfd, 10) == -1) {
		fprintf(stderr, "Could not listen.\n");
		fprintf(stderr, "Error: %s\n", strerror(errno));
		close(sfd);
		exit(EXIT_FAILURE);
	}

	accept_sfd = accept(sfd, NULL, NULL);
	if(accept_sfd < 1) {
		fprintf(stderr, "Couldn't accept connection");
		close(sfd);
		exit(EXIT_FAILURE);
	}

	//No longer needed
	freeaddrinfo(result);

	return accept_sfd;
}
