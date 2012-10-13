#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

	//Null Terminate
	datagram_store[nread] = '\0';

	printf("Received %ld bytes: %s\n", (long) nread, buf);
	
	return nread;
}
