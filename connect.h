#ifndef _CONNECT_H_
#define _CONNECT_H_

/**
 * Create a socket
 * return: file descriptor for the socket, -1 if error
 */
int create_socket();

/* Create a sockaddr_in structure
 * param host: host either DNS, IPv4, or null for server sockets
 * param port: port
 * param sock_info: address to struct sockaddr_in 
 * return: 1 on succes, 0 on failure
 */
int get_socket_info(char* host, int port, struct sockaddr_in* sock_info);

#endif
