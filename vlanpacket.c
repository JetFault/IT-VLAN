#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "vlanpacket.h"

/* Deserialize a buffer sent over the network to a packet struct
 * param buffer: the buffer to deserialize
 * param packet_struct: the memory location where to allocate packet
 * return: the packet type, PACKET_TYPE_*
 */
uint16_t deserialize(char *buffer, void* packet_struct) {

  /* Bitwise magic to get packet fields */
  uint16_t packet_type = *((uint16_t*)buffer);
  uint16_t pack_length = *( ((uint16_t*)buffer) +1);

  if(packet_type == UINT16_C(PACKET_TYPE_DATA)) {
    struct data_packet* data_pack = malloc(sizeof(struct data_packet));

    data_pack->packet_type = packet_type;
    data_pack->packet_length = pack_length;

    data_pack->datagram = (char*)( ((uint16_t*)buffer) +2);

    packet_struct = data_pack;
  } 
  else if(packet_type == PACKET_TYPE_LEAVE) {
    struct leave_packet* leave_pack = malloc(sizeof(struct leave_packet));

    leave_pack->packet_type = packet_type;
    leave_pack->packet_length = pack_length;

    if(pack_length != 20) {
      fprintf(stderr, "Wrong packet length of %u", pack_length);
    }

    char* fields = (char*)( ((uint16_t*)buffer) +2);
    
    uint32_t local_ip = *( ((uint32_t*)fields));
    uint16_t local_port = (uint16_t)* ( ((uint32_t*)fields) +1);
    char* mac_addr = (char*) (fields + 6);

    leave_pack->local.ip = local_ip;
    leave_pack->local.port = local_port;
    memcpy(leave_pack->local.mac_addr, mac_addr, 6);

    leave_pack->ID = (uint64_t)* (fields + sizeof(struct proxy_addr));

    free(buffer);

    packet_struct = leave_pack;
  }
  else if(packet_type == PACKET_TYPE_QUIT) {
    struct quit_packet* quit_pack = malloc(sizeof(struct quit_packet));

    quit_pack->packet_type = packet_type;
    quit_pack->packet_length = pack_length;

    if(pack_length != 20) {
      fprintf(stderr, "Wrong packet length of %u", pack_length);
    }

    char* fields = (char*)(((uint16_t*)buffer) +2);
    
    uint32_t local_ip = *( ((uint32_t*)fields));
    uint16_t local_port = (uint16_t)* ( ((uint32_t*)fields) +1);
    char* mac_addr = (char*) (fields + 6);

    quit_pack->local.ip = local_ip;
    quit_pack->local.port = local_port;
    memcpy(quit_pack->local.mac_addr, mac_addr, 6);

    quit_pack->ID = (uint64_t)* (fields + sizeof(struct proxy_addr));

    free(buffer);

    packet_struct = quit_pack;

  }
  else if(packet_type == PACKET_TYPE_LINKSTATE) {
    struct linkstate_packet* linkstate_pack = malloc(sizeof(struct linkstate_packet));

    linkstate_pack->packet_type = packet_type;
    linkstate_pack->packet_length = pack_length;

    uint16_t number_neighbors = (uint16_t)* ( ((uint16_t*)buffer) +2);

    linkstate_pack->num_neighbors = number_neighbors;

    char* fields = (char*)(((uint16_t*)buffer) +3);
  
    uint32_t source_ip = *( ((uint32_t*)fields));
    uint16_t source_port = (uint16_t)* ( ((uint32_t*)fields) +1);
    char* mac_addr = (char*) (fields + 6);

    /* Source addres */
    linkstate_pack->source.ip = source_ip;
    linkstate_pack->source.port = source_port;
    memcpy(linkstate_pack->source.mac_addr, mac_addr, 6);
		
		int i;
    for(i = 0; i < number_neighbors; i++) {
      struct linkstate* ls_node = malloc(sizeof(struct linkstate));

      /* Get local address */

      /* Get remote address */

      /* Get RTT, ID */

    }

  }
  else {
    fprintf(stderr, "Wrong packet type: %u", packet_type);
    return -1;
  }

  return packet_type;
  
}

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

}

ssize_t read_wrapper(int socket_fd, char* buffer, size_t length) {
  ssize_t nread = 0;
  while(length > 0) {
    nread = read(socket_fd, buffer, length);
    if (nread == -1) {
      fprintf(stderr, "Read failed.");
      return nread;
    }
    length -= nread;
  }
}

ssize_t socket_read(int socket_fd, char* buffer, size_t length) {
	ssize_t nread;

	nread = read_wrapper(socket_fd, buffer, length);
	if (nread == -1) {
		fprintf(stderr, "Read failed.");
    return -1;
	}

#if DEBUG
	printf("Received %d bytes\n", nread);
#endif
	
	return nread;
}

ssize_t socket_write(int socket_fd, char* buffer, size_t length) {
  ssize_t nwrite;

/*
	if (len + 1 > DATAGRAM_SIZE) {
		fprintf(stderr,
				"Datagram too big sending anyway.");
	}
*/

  nwrite = write(socket_fd, buffer, length);
	if (nwrite != length) {
		fprintf(stderr, "Partial or Failed write.");
    return -1;
	}

	return nwrite;
}

