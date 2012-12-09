#ifndef _VLANPACKET_H_
#define _VLANPACKET_H_

#include <stdint.h>
#include <sys/types.h>

#define PACKET_TYPE_DATA 0xABCD
#define PACKET_TYPE_LEAVE 0xAB01
#define PACKET_TYPE_QUIT 0xAB12
#define PACKET_TYPE_LINKSTATE 0xABAC


struct proxy_addr {
  uint32_t ip;
  uint16_t port;
  char     mac_addr[6]; //48 bits, 6 bytes
};

struct linkstate {
  struct proxy_addr local;
  struct proxy_addr remote;
  uint32_t avg_RTT;
  uint64_t ID;
  struct linkstate *next;
};

struct data_packet {
  uint16_t packet_type;
  uint16_t packet_length;
  char* datagram;
};

struct leave_packet {
  uint16_t packet_type;
  uint16_t packet_length; //Should be 20
  struct proxy_addr local;
  uint64_t ID;
};

struct quit_packet {
  uint16_t packet_type;
  uint16_t packet_length; //Should be 20
  struct proxy_addr local;
  uint64_t ID;
};

struct linkstate_packet {
  uint16_t packet_type;
  uint16_t packet_length;
  uint16_t num_neighbors;
  struct proxy_addr source;
  struct linkstate* linkstate_head;
};

/* Deserialize a buffer sent over the network to a packet struct
 * param buffer: the buffer to deserialize
 * param packet_struct: the memory location where to allocate packet
 * return: the packet type, PACKET_TYPE_*
 */
uint16_t deserialize(char *buffer, void* packet_struct);

/* Serlialize a packet struct into a char* buffer to send over network
 * param packet_type: the packet_type, defined as PACKET_TYPE_*
 * param packet: the struct for the packet_type typecasted as void*
 * param buffer: location to malloc new char* buffer
 * return: size of buffer
 */
size_t serialize(uint16_t packet_type, void* packet, char* buffer);

#endif
