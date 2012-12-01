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
int server_connect(int port, int socket_fd) {
	int connection_fd, ret_status;

	struct sockaddr_in stSockAddr;
	memset(&stSockAddr, 0, sizeof(stSockAddr));
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(port);

  stSockAddr.sin_addr.s_addr = INADDR_ANY;

  /* Bind */
  if(bind(socket_fd, (struct sockaddr*)&stSockAddr, sizeof(stSockAddr))) {
    fprintf(stderr, "Binding failed.");
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  errno = 0;
  
  /* Listen */
  if(listen(socket_fd, 10) == -1) {
    fprintf(stderr, "Listening on device Failed. Error: %s\n", strerror(errno));
    close(socket_fd);
    exit(EXIT_FAILURE);
  }
  
  /* Accept */
  if((connection_fd = accept(socket_fd, NULL, NULL)) <= 0) {
    fprintf(stderr, "Accept failed. Error Code: %d", connection_fd);
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

	return connection_fd;
}
