#ifndef _TAP_H_
#define _TAP_H_

#include "vlanpacket.h"

/**************************************************
 * allocate_tunnel:
 * open a tun or tap device and returns the file
 * descriptor to read/write back to the caller
 *****************************************/
int allocate_tunnel(char *dev, int flags, char* local_mac);

/* Find the source and destination addresses of a datagram
 *    using the Layer 2 and 3 information.
 * param datagram: datagram buffer to decipher
 * param local: local struct proxy_addr address
 * param remote: remote struct proxy_addr address
 * return: -1 on failure, 0 on success, 1 on ARP
 */
int find_tap_dest(char* datagram, struct proxy_addr *src, struct proxy_addr *dest, uint16_t* id);

int get_tap_info(char* tap_name, struct proxy_addr* tap_info);

int send_to_tap(int tap_fd, char* datagram);

/* Read from a tap file descriptor, to a buffer, of a certain size
 * param socket_fd: tap file descriptor
 * param buffer: Buffer address to read into
 * param length: number of bytes to read
 */
ssize_t tap_read(int socket_fd, char** buffer, size_t length);

/* Write to a tap file descriptor, from a buffer, of a certain size
 * param socket_fd: tap file descriptor
 * param buffer: Buffer to write
 * param length: number of bytes to write
 */
ssize_t tap_write(int socket_fd, char* buffer, size_t length);

#endif
