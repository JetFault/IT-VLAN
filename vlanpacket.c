#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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

  if(packet_type == PACKET_TYPE_DATA) {
    struct data_packet* data_pack = malloc(sizeof(struct data_packet));

    data_pack->packet_type = pack_length;
  } 
  else if(packet_type == PACKET_TYPE_LEAVE) {

  }
  else if(packet_type == PACKET_TYPE_QUIT) {

  }
  else if(packet_type == PACKET_TYPE_LINKSTATE) {

  }
  else {
    fprintf(stderr, "Wrong packet type: %u", packet_type);
    return 0;
  }
  
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

