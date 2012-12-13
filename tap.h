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
int find_tap_dest(char* datagram, struct proxy_addr *src, struct proxy_addr *dest);

#endif
