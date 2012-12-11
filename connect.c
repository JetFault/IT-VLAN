#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "connect.h"

/**
 * Create a socket
 * return: file descriptor for the socket, -1 if error
 */
int create_socket() {
  int socket_fd;


  if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "Failed creating socket.\n");
    return -1;
  }

  /* Allow socket reuseability */
  int optval = 1; 
  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)) < 0) {
    fprintf(stderr, "Failed to set socket options.\n");
    return -1;
  }

  return socket_fd;
}


/* Create a sockaddr_in structure
 * param host: host either DNS, IPv4, or null for server sockets
 * param port: port
 * param sock_info: address to struct sockaddr_in 
 * return: 1 on succes, 0 on failure
 */
int get_socket_info(char* host, int port, struct sockaddr_in* sock_info) {

  struct hostent *hp;       /* remote host info from gethostbyname() */

  memset(sock_info, 0, sizeof(struct sockaddr_in));
  sock_info->sin_family = AF_INET;   //IPv4
  sock_info->sin_port = htons(port); //Port

  /* Server socket */
  if(host == NULL) {
    sock_info->sin_addr.s_addr = INADDR_ANY;
  } else {
  /* Client Socket */

    /* If internet "a.d.c.d" address is specified, use inet_addr()
     * to convert it into real address.  If host name is specified,
     * use gethostbyname() to resolve its address */ 
    sock_info->sin_addr.s_addr = inet_addr(host); /* If "a.b.c.d" addr */ 

    if (sock_info->sin_addr.s_addr == -1) {
      hp = gethostbyname(host); 
      if (hp == NULL) { 
        fprintf(stderr, "Host name %s not found\n", host); 
        return -1;
      } 
      memcpy(&(sock_info->sin_addr), hp->h_addr, hp->h_length); 
    } 
  }

  return 1;
}


/* Connect to a host and port. Client connections
 * param host: host either DNS, IPv4, or null for server sockets
 * param port: port * return: socket file descriptor for the connect, -1 on error
 */
int connect_to(char* host, int port) {
  struct sockaddr_in socket_info;
  int socket_fd;

  socket_fd = create_socket();
  if(socket_fd == -1) {
    fprintf(stderr, "Failure creating socket\n");
    return -1;
  }

  /* Create the socket info */
  if(!get_socket_info(host, port, &socket_info)) {
    fprintf(stderr, "Failure getting socket info\n");
    return -1;
  }

  if(connect(socket_fd,(struct sockaddr *) &socket_info, sizeof(socket_info)) < 0)  {
    fprintf(stderr, "ERROR connecting\n");
    return -1;
  }

  return socket_fd;
}


#if 0
//comment out for now

  /* CODE TO OPEN THE SOCKET */
  /* give the connect call the sockaddr_in struct that has the address
   * in it on the connect call */ 
  if (connect(connection_fd, (struct sockaddr*)&to, sizeof(to)) < 0) { 
    perror("connect"); 
    close(socket_fd);
    exit(-1); 
  }

  connection_fd = connect(socket_fd, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr));
  if(connection_fd == -1) {
    perror("connect failed");
    close(socket_fd);
    exit(EXIT_FAILURE);
  }

  return connection_fd;
}

#endif
