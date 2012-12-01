#ifndef _CLIENT_H_
#define _CLIENT_H_

/**
 * Connect client to a host and port.
 * param host: either hostname or IP
 * param port: port number
 * param socket_fd: source socket file descriptor
 * return: file descriptor for the connected destination socket
 */
int client_connect(char* host, int port, int socket_fd);

#endif
