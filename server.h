#ifndef _SERVER_H_
#define _SERVER_H_

/**
 * Connect server to a port.
 * param port: port number
 * param: file descriptor for the source socket
 * return: file descriptor for the destination accepted socket
 */
int server_connect(int port, int socket_fd);

#endif
