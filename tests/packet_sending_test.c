#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include <assert.h>

#include "../vlanpacket.h"
#include "../connect.h"

struct leave_packet pack = { .head = { .packet_type = PACKET_TYPE_LEAVE, .packet_length = 20 }, 
  .local = { .ip = 123456, .port = 1991, .mac_addr = "macadd"}, .ID=1337133713371337};

void* start_tcp_listener(void* socket_arg) {
	int socket_fd = (int)socket_arg;
  int listen_port = 3600;

  int connection_fd;

  struct sockaddr_in socket_info;

  /* Create the socket info */
  if(!get_socket_info(NULL, listen_port, &socket_info)) {
    fprintf(stderr, "Failure getting socket info");
    exit(EXIT_FAILURE);
  }

  /* Bind */
  if(bind(socket_fd, (struct sockaddr*)&socket_info, sizeof(socket_info))) {
    fprintf(stderr, "Binding failed.");
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  /* Listen */
  errno = 0;
  if(listen(socket_fd, 10) == -1) {
    fprintf(stderr, "Listening on device Failed. Error: %s\n", strerror(errno));
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  /* Accept connections, and spawn new threads for each connection */
  while(1) {

    /* Accept */
    if((connection_fd = accept(socket_fd, NULL, NULL)) <= 0) {
      fprintf(stderr, "Accept failed. Error Code: %d", connection_fd);
      close(socket_fd);
      exit(EXIT_FAILURE);
    }

    void* packet_struct;
    uint16_t pack_type = read_packet(connection_fd, &packet_struct);

    struct leave_packet* deser_pack = (struct leave_packet*) packet_struct;

    assert(pack_type == pack.head.packet_type);
    assert(pack.local.ip == deser_pack->local.ip);
    assert(pack.local.ip == deser_pack->local.ip);
    assert(strncmp(pack.local.mac_addr, deser_pack->local.mac_addr, 6) == 0);
    assert(pack.ID == deser_pack->ID);

  }
}


int main() {
  /* Create server socket and accept connections */
  pthread_t server_tcp_thread;
  int server_fd = create_socket();
  pthread_create(&server_tcp_thread, NULL, start_tcp_listener, (void *) server_fd);
  /* End creating server socket thread */

  sleep(2);
  /* Connect to server */
  int connection_fd = connect_to("localhost", 3600);

  char* buffer;
  serialize(pack.head.packet_type, &pack, &buffer);

  socket_write(connection_fd, buffer, pack.head.packet_length);

  pthread_join(server_tcp_thread, NULL);

}


