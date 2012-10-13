#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * Connect client to a host and port.
 * param host: either hostname or IP
 * param port: port number
 * return: file descriptor for the socket
 */
int client_connect(char* host, char* port) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, ret_status;

	/* Obtain address(es) matching host/port */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;							/* Null out flags */
	hints.ai_protocol = 0;          /* Any protocol */

	ret_status = getaddrinfo(host, port, &hints, &result);
	if (ret_status != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret_status));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
		 Try each address until we successfully connect(2).
		 If socket(2) (or connect(2)) fails, we (close the socket
		 and) try the next address. */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			//Success
			break;
		}
		//Not the correct fd, close it
		close(sfd);
	}

	//No address succeeded
	if (rp == NULL) {
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	//No longer needed
	freeaddrinfo(result);

	return sfd;
}

int write_socket(int socket_fd, char* datagram) {
	unsigned int len;
	char buf[BUF_SIZE];

	/* Read datagrams, and read responses from server */

	//+1 for terminating null byte
	len = strlen(datagram) + 1;

	if (len + 1 > BUF_SIZE) {
		fprintf(stderr,
				"Datagram too big sending anyway.");
		continue;
	}

	if (write(socket_fd, datagram, len) != len) {
		fprintf(stderr, "Partial or Failed write.");
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	return len;

}

int read_socket(int socket_fd, char* datagram_store, unsigned int length) {
	int nread;

	nread = read(socket_fd, datagram_store, length);
	if (nread == -1) {
		perror("Read failed.");
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	printf("Received %ld bytes: %s\n", (long) nread, buf);
	
	return nread;
}

