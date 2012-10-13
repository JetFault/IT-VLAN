#ifndef _SERVER_H_
#define _SERVER_H_

/**
 * Connect server to a port.
 * param port: port number
 * return: file descriptor for the socket
 */
int server_connect(char* port);

#endif
