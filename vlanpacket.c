#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>

#include "vlanpacket.h"

double current_time() {
  struct timeval  tv;
  gettimeofday(&tv, NULL);

  // convert tv_sec & tv_usec to millisecond
  return (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ;
}


/* Helper function to serialize the proxy address
 * param proxy_address: proxy_addr to read from
 * param data_start: Starting point in char* array of where to store
 * return: number of bytes serialized
 */
size_t serialize_proxy_addr(struct proxy_addr proxy_address, char* data_start) {
    uint32_t* ip = (uint32_t*) data_start;
    *ip = proxy_address.ip;
    uint16_t* port = (uint16_t*) (data_start + sizeof(uint32_t));
    *port = proxy_address.port;
    memcpy((data_start + sizeof(uint32_t) + sizeof(uint16_t)), proxy_address.mac_addr, 6);

    return sizeof(struct proxy_addr);
}


/* Helper function to deserialize the proxy address
 * param data_start: Starting point in char* array
 * param proxy_address: Where to store proxy address
 * return: number of bytes deserialized
 */
int deserialize_proxy_addr(char* data_start, struct proxy_addr * proxy_address) {
    uint32_t local_ip = *( (uint32_t*)data_start );
    uint16_t local_port = *( (uint16_t*)(data_start + sizeof(uint32_t)) );
    char* mac_addr = (char*) (data_start + sizeof(uint32_t) + sizeof(uint16_t));

    proxy_address->ip = local_ip;
    proxy_address->port = local_port;
    memcpy(proxy_address->mac_addr, mac_addr, 6*sizeof(uint8_t));

    return sizeof(struct proxy_addr);
}


/* Read from the socket fd to get a packet
 * param socket_fd: Socket fd to read from
 * param packet_struct: Address of where to store packet_struct
 * return: the packet type, 0 if error
 */
uint16_t read_packet(int socket_fd, void** packet_struct) {
  char* header = malloc(sizeof(struct header));
  if(socket_read(socket_fd, &header, sizeof(struct header)) != sizeof(struct header)) {
    fprintf(stderr, "Incorrect header structure\n");
    return 0;
  }
  
  /* Bitwise magic to get packet fields */
  uint16_t pack_type = *((uint16_t*)header);
  uint16_t pack_length = *( ((uint16_t*)header) + 1 );

  struct header head = { .packet_type = pack_type, .packet_length = pack_length };

  /* Read from socket to get the datagram/fields */
  char* datagram = malloc(pack_length);
  if(socket_read(socket_fd, &datagram, pack_length) != pack_length) {
    fprintf(stderr, "Incorrect packet datagram structure\n");
    return 0;
  }

  return deserialize(&head, datagram, packet_struct);
}

/* Send the packet to the destination
 * param dest: struct proxy_addr of the destination
 * param packet: a packet typecasted as void* to send
 * param socket_fd: optional, if this is not 0, send directly
 *    to that socket, ignoring the dest parameter
 * return: -1 on failure, 0 on success
 */
int send_to(struct proxy_addr* dest, void* packet, int socket_fd) {

};


/* Deserialize a buffer sent over the network to a packet struct
 * param buffer: the buffer to deserialize
 * param packet_struct: the memory location where to allocate packet
 * return: the packet type, PACKET_TYPE_*. 0 if error/wrong packet type
 */
uint16_t deserialize(struct header* head, char *buffer, void** packet_struct) {

  size_t num_bytes;

  char * curr_ptr = buffer;

  uint16_t packet_type = head->packet_type;
  uint16_t pack_length = head->packet_length;

  if(packet_type == PACKET_TYPE_DATA) {
    struct data_packet* data_pack = malloc(sizeof(struct data_packet));

    data_pack->datagram = malloc(pack_length);

    memcpy(data_pack->datagram, curr_ptr, pack_length);

    *packet_struct = data_pack;
  } 
  else if(packet_type == PACKET_TYPE_LEAVE) {
    struct leave_packet* leave_pack = malloc(sizeof(struct leave_packet));

    if(pack_length != 20) {
      fprintf(stderr, "Wrong packet length of %u", pack_length);
      return 0;
    }

    num_bytes = deserialize_proxy_addr(curr_ptr, &(leave_pack->local));
    curr_ptr = curr_ptr + num_bytes;

    leave_pack->ID = *( (uint64_t*)curr_ptr);

    *packet_struct = leave_pack;
  }
  else if(packet_type == PACKET_TYPE_QUIT) {
    struct quit_packet* quit_pack = malloc(sizeof(struct quit_packet));

    if(pack_length != 20) {
      fprintf(stderr, "Wrong packet length of %u", pack_length);
      return 0;
    }

    num_bytes = deserialize_proxy_addr(curr_ptr, &(quit_pack->local));
    curr_ptr = curr_ptr + num_bytes;

    quit_pack->ID = *( (uint64_t*)curr_ptr);

    *packet_struct = quit_pack;
  }
  else if(packet_type == PACKET_TYPE_LINKSTATE) {
    struct linkstate_packet* linkstate_pack = malloc(sizeof(struct linkstate_packet));

    /* Number of Neighbors */
    uint16_t number_neighbors = (uint16_t) *curr_ptr;
    linkstate_pack->num_neighbors = number_neighbors;
    curr_ptr = curr_ptr + sizeof(uint16_t);

    /* Source address */
    num_bytes = deserialize_proxy_addr(curr_ptr, &(linkstate_pack->source));
    curr_ptr = curr_ptr + num_bytes;

    linkstate_pack->linkstate_head = NULL;

		int i;
    for(i = 0; i < number_neighbors; i++) {
      struct linkstate* ls_node = malloc(sizeof(struct linkstate));

      /* Linkstate Local address */
      num_bytes = deserialize_proxy_addr(curr_ptr, &(ls_node->local));
      curr_ptr = curr_ptr + num_bytes;

      /* Linkstate Remote address */
      num_bytes = deserialize_proxy_addr(curr_ptr, &(ls_node->remote));
      curr_ptr = curr_ptr + num_bytes;

      /* Get RTT, ID */
      uint32_t rtt = *( (uint32_t*)curr_ptr );
      curr_ptr = curr_ptr + sizeof(uint32_t);
      uint64_t ID = *( (uint64_t*) curr_ptr );
      curr_ptr = curr_ptr + sizeof(uint64_t);

      /* Attach Linkstate to front of list */
      ls_node->next = linkstate_pack->linkstate_head;
      linkstate_pack->linkstate_head = ls_node; 
    }

    *packet_struct = linkstate_pack;
  }
  else if(packet_type == PACKET_TYPE_PROBEREQ) {
    struct probereq_packet* pack = malloc(sizeof(struct probereq_packet));

    if(pack_length != 8) {
      fprintf(stderr, "Wrong packet length of %u", pack_length);
      return 0;
    }

    pack->ID = *( (uint64_t*)curr_ptr);

    *packet_struct = pack;
  }
  else if(packet_type == PACKET_TYPE_PROBERES) {
    struct proberes_packet* pack = malloc(sizeof(struct proberes_packet));

    if(pack_length != 8) {
      fprintf(stderr, "Wrong packet length of %u", pack_length);
      return 0;
    }

    pack->ID = *( (uint64_t*)curr_ptr);

    *packet_struct = pack;
  }
  else {
    fprintf(stderr, "Wrong packet type: %u\n", packet_type);
    return 0;
  }

  /* Treat as data packet just to add header */
  ((struct data_packet*)*packet_struct)->head = *head;

  return packet_type;
  
}

/* Serlialize a packet struct into a char* buffer to send over network
 * param packet_type: the packet_type, defined as PACKET_TYPE_*
 * param packet: the struct for the packet_type typecasted as void*
 * param buffer: location to malloc new char* buffer
 * return: size of buffer
 */
size_t serialize(uint16_t packet_type, void* packet, char** buffer) {

  size_t num_bytes;
  
  size_t header_size = sizeof(uint16_t) * 2; //data & length
  size_t buffer_size = header_size + ((struct data_packet *)packet)->head.packet_length;
  *buffer = malloc(buffer_size);

  /* Bitwise magic to set packet fields */
  char* curr_ptr = *buffer;
  uint16_t* pack_type = (uint16_t*) curr_ptr;
  *pack_type = packet_type;
  uint16_t* pack_len = (uint16_t*) (curr_ptr + sizeof(uint16_t));
  *pack_len = ((struct data_packet *)packet)->head.packet_length;
  curr_ptr = curr_ptr + (sizeof(struct header));


  if(packet_type == PACKET_TYPE_DATA) {
    struct data_packet* pack = (struct data_packet *) packet;

    memcpy(curr_ptr, pack->datagram, pack->head.packet_length);
  } 
  else if(packet_type == PACKET_TYPE_LEAVE) {
    struct leave_packet* pack = (struct leave_packet *) packet;

    num_bytes = serialize_proxy_addr(pack->local, curr_ptr);
    curr_ptr = curr_ptr + num_bytes;

    uint64_t* ID = (uint64_t*) curr_ptr;
    *ID = pack->ID;

    curr_ptr = curr_ptr + sizeof(uint64_t);
  }
  else if(packet_type == PACKET_TYPE_QUIT) {
    struct quit_packet* pack = (struct quit_packet *) packet;

    serialize_proxy_addr(pack->local, curr_ptr);
    curr_ptr = curr_ptr + num_bytes;

    uint64_t* ID = (uint64_t*) curr_ptr;
    *ID = pack->ID;

    curr_ptr = curr_ptr + sizeof(uint64_t);
  }
  else if(packet_type == PACKET_TYPE_LINKSTATE) {
    struct linkstate_packet* pack = (struct linkstate_packet *) packet;

    uint16_t* num_neighbors = (uint16_t*) curr_ptr;
    *num_neighbors = pack->num_neighbors;

    curr_ptr = (char*)(curr_ptr + sizeof(uint16_t));

    /* Source address */
    num_bytes = serialize_proxy_addr(pack->source, curr_ptr);
    curr_ptr = curr_ptr + num_bytes;
  
    /* Linkstate */
    struct linkstate * ls_node = pack->linkstate_head;
    for(int i = 0; i < pack->num_neighbors; i++) {

      /* Linkstate Local address */
      num_bytes = serialize_proxy_addr(ls_node->local, curr_ptr);
      curr_ptr = curr_ptr + num_bytes;


      /* Linkstate Remote address */
      num_bytes = serialize_proxy_addr(ls_node->remote, curr_ptr);
      curr_ptr = curr_ptr + num_bytes;

      /* RTT, ID */
      uint32_t* avg_RTT = (uint32_t*) curr_ptr;
      *avg_RTT = ls_node->avg_RTT;
      curr_ptr = curr_ptr + sizeof(uint32_t);
      uint64_t* ID = (uint64_t*) curr_ptr;

      ls_node = ls_node->next;
    }

  }
  else if(packet_type == PACKET_TYPE_PROBEREQ) {
    struct probereq_packet* pack = (struct probereq_packet *) packet;

    uint64_t* ID = (uint64_t*) curr_ptr;
    *ID = pack->ID;

    curr_ptr = curr_ptr + sizeof(uint64_t);
  }
  else if(packet_type == PACKET_TYPE_PROBERES) {
    struct proberes_packet* pack = (struct proberes_packet *) packet;

    uint64_t* ID = (uint64_t*) curr_ptr;
    *ID = pack->ID;

    curr_ptr = curr_ptr + sizeof(uint64_t);
  }
  else {
    fprintf(stderr, "Wrong packet type: %u\n", packet_type);
    free(buffer);
    return 0;
  }

  return buffer_size;
}

ssize_t read_wrapper(int socket_fd, char* buffer, size_t length) {
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

ssize_t socket_read(int socket_fd, char** buffer, size_t length) {
	ssize_t nread;

	nread = read_wrapper(socket_fd, *buffer, length);
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
		fprintf(stderr, "Partial or Failed write.\n");
    return -1;
	}

	return nwrite;
}


/* Get local socket information and create proxy_addr
 * param socket_fd: socket file descriptor
 * param proxy_addr: address of where to put proxy_addr
 * return: -1 on failure, 0 on success
 */
int get_local_info(int socket_fd, struct proxy_addr* info) {
  struct sockaddr_in sock_info;
  size_t sock_info_size = sizeof(sock_info);

  errno=0;
  if(getsockname(socket_fd, (struct sockaddr*)&sock_info, &sock_info_size) == -1) {
    fprintf(stderr, "Error getting local socket info. Error: %s", strerror(errno));
    return -1;
  }

  info->ip = ntohl(sock_info.sin_addr.s_addr);
  info->port = ntohs(sock_info.sin_port);

  return 0;
}

/* Get remote socket information and create proxy_addr
 * param socket_fd: socket file descriptor
 * param proxy_addr: address of where to put proxy_addr
 * return: -1 on failure, 0 on success
 */
int get_remote_info(int socket_fd, struct proxy_addr* info) {
  struct sockaddr_in sock_info;
  size_t sock_info_size = sizeof(sock_info);

  errno=0;
  if(getsockname(socket_fd, (struct sockaddr*)&sock_info, &sock_info_size) == -1) {
    fprintf(stderr, "Error getting local socket info. Error: %s", strerror(errno));
    return -1;
  }

  info->ip = ntohl(sock_info.sin_addr.s_addr);
  info->port = ntohs(sock_info.sin_port);

  return 0;
}
