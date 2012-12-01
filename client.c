#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "client.h"

/**
 * Connect client to a host and port.
 * param host: either hostname or IP
 * param port: port number
 * return: file descriptor for the socket
 */
int client_connect(char* host, int port, int socket_fd) {
  int connection_fd, ret_status;

  struct sockaddr_in stSockAddr;
  memset(&stSockAddr, 0, sizeof(stSockAddr));
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(port);

  ret_status = inet_pton(AF_INET, host, &stSockAddr.sin_addr);
  if(ret_status != 1){
    fprintf(stderr, "Error: Could not bind to host. Return Status: %d", ret_status);
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in to;    /* remote internet address */ 
  struct hostent *hp;       /* remote host info from gethostbyname() */
  memset(&to, 0, sizeof(to));

  to.sin_family = AF_INET; 
  o.sin_port = htons(port);  

  /* If internet "a.d.c.d" address is specified, use inet_addr()

   * to convert it into real address.  If host name is specified,

   * use gethostbyname() to resolve its address */ 

  to.sin_addr.s_addr = inet_addr(hostname); /* If "a.b.c.d" addr */ 

  if (to.sin_addr.s_addr == -1) {     hp = gethostbyname(hostname); 

    if (hp == NULL) { 

      fprintf(stderr, "Host name %s not found\n", hostname); 

      exit(1); 

    } 

    bcopy(hp->h_addr, &to.sin_addr, hp->h_length); 

  } 

  /* CODE TO OPEN THE SOCKET GOES HERE */ 

  /* give the connect call the sockaddr_in struct that has the address

   * in it on the connect call */ 

  if (connect(s, &to, sizeof(to)) < 0) { 

    perror("connect"); 

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
