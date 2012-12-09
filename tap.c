#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>

#include "tap.h"

/**************************************************
 * allocate_tunnel:
 * open a tun or tap device and returns the file
 * descriptor to read/write back to the caller
 *****************************************/

#define MAX_DEV_LINE 256 

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
