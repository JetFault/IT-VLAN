#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#define HEADER_SIZE 16
#define BUF_SIZE 2048 - HEADER_SIZE 

int main(char* host, char* port, char* data) {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, ret_status, j;
	size_t len;
	ssize_t nread;
	char buf[BUF_SIZE];

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

	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* No longer needed */

	/* Read datagrams, and read responses from server */

	//+1 for terminating null byte
	len = strlen(datagram) + 1;

	if (len + 1 > BUF_SIZE) {
		fprintf(stderr,
				"Ignoring long message in argument\n");
		continue;
	}

	if (write(sfd, datagram, len) != len) {
		fprintf(stderr, "partial/failed write\n");
		exit(EXIT_FAILURE);
	}

	nread = read(sfd, buf, BUF_SIZE);
	if (nread == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	printf("Received %ld bytes: %s\n", (long) nread, buf);

	exit(EXIT_SUCCESS);
}

