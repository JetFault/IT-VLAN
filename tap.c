#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <linux/netdevice.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h> 
#include <netinet/udp.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>

#include "tap.h"
#include "vlanpacket.h"

#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif

#define MAX_DEV_LINE 256 


/**************************************************
 * allocate_tunnel:
 * open a tun or tap device and returns the file
 * descriptor to read/write back to the caller
 *****************************************/
int allocate_tunnel(char *dev, int flags, char* local_mac) {
  int fd, error;
  struct ifreq ifr;
  char *device_name = "/dev/net/tun";
  char buffer[MAX_DEV_LINE];

  if( (fd = open(device_name , O_RDWR)) < 0 ) {
    fprintf(stderr,"error opening /dev/net/tun\n%d:%s\n",errno,strerror(errno));
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = flags;

  if (*dev) {
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  }

  if( (error = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
    fprintf(stderr,"ioctl on tap failed\n%d:%s\n",errno,strerror(errno));
    close(fd);
    return error;
  }

  strcpy(dev, ifr.ifr_name);

  // Get device MAC address //
  sprintf(buffer,"/sys/class/net/%s/address",dev);

  FILE* f = fopen(buffer,"r");
  fread(buffer,1,MAX_DEV_LINE,f);
  sscanf(buffer,"%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",local_mac,local_mac+1,local_mac+2,local_mac+3,local_mac+4,local_mac+5);

  fclose(f);

  return fd;
}

int get_tap_info(char* tap_name, struct proxy_addr* tap_info) {
  struct ifaddrs *ifaddr, *ifa;
  int family, s;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(EXIT_FAILURE);
  }

  /* Walk through linked list, maintaining head pointer so we
     can free list later */

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    family = ifa->ifa_addr->sa_family;

    /* Display interface name and family (including symbolic
       form of the latter for the common families) */

#if DEBUG
    printf("%s  address family: %d%s\n",
        ifa->ifa_name, family,
        (family == AF_PACKET) ? " (AF_PACKET)" :
        (family == AF_INET) ?   " (AF_INET)" :
        (family == AF_INET6) ?  " (AF_INET6)" : "");
#endif

    /* For an AF_INET* interface address, display the address */
    if(strcmp(ifa->ifa_name, tap_name) == 0 && family == AF_INET) {
      struct sockaddr_in* sa_in = (struct sockaddr_in*) ifa->ifa_addr;

      tap_info->ip = ntohl(sa_in->sin_addr.s_addr);

      s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
          host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

      if (s != 0) {
        printf("getnameinfo() failed: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
      }
#if DEBUG
      printf("\taddress: <%s>\n", host);
#endif
      break;
    }

  }

  freeifaddrs(ifaddr);
}

/* Find the tap datagram source and destination addresses.
 * param packet: packet buffer to decipher
 * param local: local struct proxy_addr address
 * param remote: remote struct proxy_addr address
 * param id: address of where to store the ID
 * return: -1 on failure, 0 on success, 1 on ARP
 */
int find_tap_dest(char* packet, struct proxy_addr *src, struct proxy_addr *dest, uint16_t* id) {

  struct ether_header *ether_packet_p;  /* Pointer to an Ethernet header */
  struct iphdr *ip_packet_p;            /* Pointer to an IP header  */

  uint32_t ip_source_addr;  /* An IP source address */
  uint32_t ip_dest_addr;    /* An IP destination address */

  char* buffer = packet;

  ether_packet_p = (struct ether_header *) buffer; 

  #if DEBUG
  printf("got Ethernet packet, destination %x:%x:%x:%x:%x:%x source %x:%x:%x:%x:%x:%x\n",
      /* destination address */
      ether_packet_p->ether_dhost[0], ether_packet_p->ether_dhost[1], 
      ether_packet_p->ether_dhost[2], ether_packet_p->ether_dhost[3], 
      ether_packet_p->ether_dhost[4], ether_packet_p->ether_dhost[5],
      /* source address */
      ether_packet_p->ether_shost[0], ether_packet_p->ether_shost[1], 
      ether_packet_p->ether_shost[2], ether_packet_p->ether_shost[3], 
      ether_packet_p->ether_shost[4], ether_packet_p->ether_shost[5]);
  #endif

  //Copy Ethernet source and destination information
  memcpy(src->mac_addr, ether_packet_p->ether_shost, sizeof(uint8_t)*ETH_ALEN);
  memcpy(dest->mac_addr, ether_packet_p->ether_dhost, sizeof(uint8_t)*ETH_ALEN);

  switch ( ntohs(ether_packet_p->ether_type) ) { 

    case ETHERTYPE_ARP:  /* we have an ARP packet. */ 
      #if DEBUG
      printf("got ARP packet \n");
      #endif
      /* add printing out ARP packet 
       * See net/if_arp.h for the header definition */
      return 1;

    case ETHERTYPE_IP:  /* we have an IP packet */

      /* set the pointer of the IP packet header to the memory location 
       * of the buffer base address + the size of the Ethernet header 
       * should check if the length from cread is sane here */ 

      ip_packet_p = (struct iphdr *) &(buffer[sizeof(struct ether_header)] );

      /* get the source and destination IP address */
      /* remember to change the byte order of the IP addresses to the 
       * local host's byte order */ 
      ip_source_addr = ntohl( ip_packet_p->saddr);
      ip_dest_addr = ntohl( ip_packet_p->daddr);

      src->ip  = ip_source_addr;
      dest->ip = ip_dest_addr;
      if(id != NULL) {
        *id = ntohs(ip_packet_p->id);
      }
      
      #if DEBUG
      printf("got IP packet, destination %d.%d.%d.%d source %d.%d.%d.%d\n",
          /* destination IP address */
          /* given a 32 bit integer, mask and shift the bits */
          /* the mask is in hexadecimal */ 
          (ip_dest_addr & 0xFF000000)>>24, 
          (ip_dest_addr & 0x00FF0000)>>16, 
          (ip_dest_addr & 0x0000FF00)>>8,
          (ip_dest_addr & 0x000000FF),
          /* source IP address */
          (ip_source_addr & 0xFF000000)>>24, 
          (ip_source_addr & 0x00FF0000)>>16, 
          (ip_source_addr & 0x0000FF00)>>8,
          (ip_source_addr & 0x000000FF));
      #endif

      break;
    default:
      printf("got unknown packet type \n");
      return -1;
  }
  return 1;
}

ssize_t tap_read_wrapper(int socket_fd, char* buffer, size_t length) {
  ssize_t nread = 0;
  ssize_t bytes_read = 0;
  while(length > 0) {
    nread = read(socket_fd, buffer, length);
    if (nread == -1) {
      return nread;
    }
    bytes_read += nread;
    length -= nread;
  }
  return bytes_read;
}

ssize_t tap_write_wrapper(int socket_fd, char* buffer, size_t length) {
  ssize_t nwrite = 0;
  ssize_t bytes_written = 0;
  while(length > 0) {
    nwrite = write(socket_fd, buffer, length);
    if (nwrite == -1) {
      return nwrite;
    }
    bytes_written += nwrite;
    length -= nwrite;
  }
  return bytes_written;
}

ssize_t tap_read(int socket_fd, char** buffer, size_t length) {
	ssize_t nread;

	nread = tap_read_wrapper(socket_fd, *buffer, length);
	if (nread == -1) {
		fprintf(stderr, "Read failed.\n");
    return -1;
	}

#if DEBUG
	printf("Received %d bytes\n", nread);
#endif
	
	return nread;
}

/* Write to a file descriptor, from a buffer, of a certain size
 * param socket_fd: socket file descriptor
 * param buffer: Buffer to write
 * param length: number of bytes to write
 */
ssize_t tap_write(int socket_fd, char* buffer, size_t length) {
  ssize_t nwrite;

  nwrite = tap_write_wrapper(socket_fd, buffer, length);
	if (nwrite != length) {
		fprintf(stderr, "Failed write.\n");
    return -1;
	}

	return nwrite;
}

