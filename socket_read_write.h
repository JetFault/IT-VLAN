#ifndef _SOCKET_READ_WRITE_H_
#define _SOCKET_READ_WRITE_H_

/**
 * Write to a socket.
 * param socket_fd: file descripter
 * param datagram: null escaped string to write
 * return: number of bytes written
 */
int socket_write(int socket_fd, char* datagram, unsigned short int length);

/**
 * Read `length` bytes from a socket and store in `datagram_store`.
 * param socket_fd: file descripter
 * param datagram_store: char array big enough to hold `length` + 1 bits. 
 * 		Will be null terminated.
 * param length: number of bytes to read from socket
 * return: number of bytes read
 */
int socket_read(int socket_fd, char* datagram_store, unsigned short int length);

#endif
