#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/ether.h> 
#include <netinet/ip.h>
#include <netinet/tcp.h> 
#include <netinet/udp.h>

#include "tap.h"
#include "vlanpacket.h"

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
};

/* Find the tap datagram source and destination addresses.
 * param packet: packet buffer to decipher
 * param local: local struct proxy_addr address
 * param remote: remote struct proxy_addr address
 * return: -1 on failure, 0 on success, 1 on ARP
 */
int find_tap_dest(char* packet, struct proxy_addr *src, struct proxy_addr *dest) {

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
