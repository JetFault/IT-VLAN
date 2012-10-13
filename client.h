#ifndef _CLIENT_H_
#define _CLIENT_H_

/**
 * Connect client to a host and port.
 * param host: either hostname or IP
 * param port: port number
 * return: file descriptor for the socket
 */
int client_connect(char* host, char* port);

#endif
