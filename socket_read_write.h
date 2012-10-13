#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
 * Write to a socket.
 * param socket_fd: file descripter
 * param datagram: null escaped string to write
 * return: number of bytes written
 */
int write_socket(int socket_fd, char* datagram);

/**
 * Read `length` bytes from a socket and store in `datagram_store`.
 * param socket_fd: file descripter
 * param datagram_store: char array big enough to hold `length` + 1 bits. 
 * 		Will be null terminated.
 * param length: number of bytes to read from socket
 * return: number of bytes read
 */
int read_socket(int socket_fd, char* datagram_store, unsigned int length);

