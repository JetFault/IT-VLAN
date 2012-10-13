#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "socket_read_write.h"

#define HEADER_SIZE_SIZE 2
#define HEADER_TYPE_SIZE 2
#define DATAGRAM_SIZE 2048 - HEADER_SIZE_SIZE - HEADER_TYPE_SIZE

int socket_write(int socket_fd, char* datagram, unsigned short int length) {
	unsigned int len;

	/* Read datagrams, and read responses from server */

	//+1 for terminating null byte
	len = strlen(datagram) + 1;

	if (len + 1 > DATAGRAM_SIZE) {
		fprintf(stderr,
				"Datagram too big sending anyway.");
	}

	if (write(socket_fd, datagram, len) != len) {
		fprintf(stderr, "Partial or Failed write.");
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	return len;

}

int socket_read(int socket_fd, char* datagram_store, unsigned short int length) {
	int nread;

	nread = read(socket_fd, datagram_store, length);
	if (nread == -1) {
		perror("Read failed.");
		close(socket_fd);
		exit(EXIT_FAILURE);
	}

	//Null Terminate
	datagram_store[nread] = '\0';

	printf("Received %ld bytes: %s\n", (long) nread, datagram_store);
	
	return nread;
}
