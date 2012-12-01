#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#include "vlanpacket.h"

/* Deserialize a buffer sent over the network to a packet struct
 * param buffer: the buffer to deserialize
 * param packet_struct: the memory location where to allocate packet
 * return: the packet type, PACKET_TYPE_*
 */
uint16_t deserialize(char *buffer, void* packet_struct) {
  
};

/* Serlialize a packet struct into a char* buffer to send over network
 * param packet_type: the packet_type, defined as PACKET_TYPE_*
 * param packet: the struct for the packet_type typecasted as void*
 * param buffer: location to malloc new char* buffer
 * return: size of buffer
 */
size_t serialize(uint16_t packet_type, void* packet, char* buffer) {


  switch(packet_type) {
    case PACKET_TYPE_DATA:
      break;
    case PACKET_TYPE_LEAVE:
      break;
    case PACKET_TYPE_QUIT:
      break;
    case PACKET_TYPE_LINKSTATE:
      break;
    default:
      fprintf(stderr, "Wrong packet type: %u", packet_type);
      return 0;
  }

};
