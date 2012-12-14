#ifndef _VLANPACKET_H_
#define _VLANPACKET_H_

#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>

#define PACKET_TYPE_DATA 0xABCD
#define PACKET_TYPE_LEAVE 0xAB01
#define PACKET_TYPE_QUIT 0xAB12
#define PACKET_TYPE_LINKSTATE 0xABAC
#define PACKET_TYPE_PROBEREQ 0xAB34
#define PACKET_TYPE_PROBERES 0xAB35

/* Extra Credit Packet Types 
 
 Proxy Public Key 0xAB21
 Signed Data 0xABC1
 Proxy Secret key 0xAB22
 Encrypted Data 0xABC2
 Encrypted Link State 0XABAB
 Signed link-state 0XABAD
 Bandwidth Probe Request 0xAB45
 Bandwidth Response 0xAB46

 */

struct proxy_addr {
  uint32_t ip;
  uint16_t port;
  uint8_t  mac_addr[6]; //48 bits, 6 bytes.
};

// Edge of a network
struct linkstate {
  struct proxy_addr local;
  struct proxy_addr remote;
  uint32_t avg_RTT;
  uint64_t ID;
  struct linkstate *next;
};

//Membership List
struct membership_list {
	struct linkstate* list;
	int size;
	pthread_mutex_t lock;
}

struct header {
  uint16_t packet_type;
  uint16_t packet_length;
};

struct data_packet {
  struct header head;
  char* datagram;
};

//Length should be 20
struct leave_packet {
  struct header head;
  struct proxy_addr local;
  uint64_t ID;
};

//Length should be 20
struct quit_packet {
  struct header head;
  struct proxy_addr local;
  uint64_t ID;
};

struct linkstate_packet {
  struct header head;
  uint16_t num_neighbors;
  struct proxy_addr source;
  struct linkstate* linkstate_head;
};

//Length should be 8
struct probereq_packet {
  struct header head;
  uint64_t ID;
};

//Length should be 8
struct proberes_packet {
  struct header head;
  uint64_t ID; // Echo of the corresponding probe
};

/* Get the current time in milliseconds
 * return: double of current time in milliseconds
 */
double current_time();

/* Read from the socket fd to get a packet
 * param socket_fd: Socket fd to read from
 * param packet_struct: Address of where to store packet_struct
 * return: the packet type, 0 if error
 */
uint16_t read_packet(int socket_fd, void** packet_struct);

/* Send the packet to the destination
 * param dest: struct proxy_addr of the destination
 * param packet: a packet typecasted as void* to send
 * return: -1 on failure, 0 on success
 */
int send_to(struct proxy_addr* dest, void* packet);

/* Deserialize a buffer sent over the network to a packet struct
 * param buffer: the buffer to deserialize
 * param packet_struct: the memory location where to allocate packet
 * return: the packet type, PACKET_TYPE_*
 */
uint16_t deserialize(struct header* head, char *buffer, void** packet_struct);

/* Serlialize a packet struct into a char* buffer to send over network
 * param packet_type: the packet_type, defined as PACKET_TYPE_*
 * param packet: the struct for the packet_type typecasted as void*
 * param buffer: location to malloc new char* buffer
 * return: size of buffer
 */
size_t serialize(uint16_t packet_type, void* packet, char** buffer);


/* Read from a file descriptor, to a buffer, of a certain size
 * param socket_fd: socket file descriptor
 * param buffer: Buffer address to read into
 * param length: number of bytes to read
 */
ssize_t socket_read(int socket_fd, char** buffer, size_t length);

/* Write to a file descriptor, from a buffer, of a certain size
 * param socket_fd: socket file descriptor
 * param buffer: Buffer to write
 * param length: number of bytes to write
 */
ssize_t socket_write(int socket_fd, char* buffer, size_t length);

/* Get local socket information and create proxy_addr
 * param socket_fd: socket file descriptor
 * param proxy_addr: address of where to put proxy_addr
 * return: -1 on failure, 0 on success
 */
int get_local_info(int socket_fd, struct proxy_addr* info);

/* Get remote socket information and create proxy_addr
 * param socket_fd: socket file descriptor
 * param proxy_addr: address of where to put proxy_addr
 * return: -1 on failure, 0 on success
 */
int get_remote_info(int socket_fd, struct proxy_addr* info);


#endif
