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
#define BUF_SIZE 2048

int server = 0;
int client = 0;

int PORT, conn_fd, tapFD;
char* HOST;
char* TAP_DEV;

/**************************************************
 * allocate_tunnel:
 * open a tun or tap device and returns the file
 * descriptor to read/write back to the caller
 *****************************************/
int allocate_tunnel(char *dev, int flags) {

	int fd, error;
	struct ifreq ifr;
	char *device_name = "/dev/net/tun";

	if( (fd = open(device_name , O_RDWR)) < 0 ) {
		perror("error opening /dev/net/tun");
		return fd;
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = flags;

	if (*dev) {
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	if( (error = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
		perror("ioctl on tap failed");
		close(fd);
		return error;
	}

	strcpy(dev, ifr.ifr_name);
	return fd;
}


tap_thread(void* arg) 
{
	int sock_fd, curr_fd;
	sock_fd = (int)arg;
	char *if_name = “tap0”;
	if ( (tap_fd = allocate_tunnel(if_name, IFF_TAP | IFF_NO_PI)) < 0 ) 
	{
		perror("Opening tap interface failed! \n");
		exit(1);
	}


	if (server = 1)
	{
		curr_fd = conn_fd;
	}
	else if(server = 0)
	{
		curr_fd = sock_fd;
	}
	for(;;)
	{
		write_socket(curr_fd, (char*) arg);
		write_socket(curr_fd, (char*) arg);
		write_socket(curr_fd, (char*) arg);
		
		read_socket(curr_fd, (char*) arg);
	}
}

	bindOrConnect() 
	{
		if(server = 1)
		{
			//BIND PORT
			//SET PORT LISTEN
			//SET PORT ACCEPT CONNECTION
			conn_fd = conn_fd;
		}
		else
		{
			conn_fd = connect;
		}
	}

	int tcp_thread(void* arg) 
	{
		int sock_fd, curr_fd;
		socketFD = (int)arg;
		int BOC = bindOrConnect;
		
		pthread_t tap;
		int tapret;
		tapret = pthread_create( &tap, NULL, tap_thread, arg);
		
		if(server = 1)
		{
			curr_fd = connectFD;
		}
		else
		{
			curr_fd = sock_fd;
		}
		
		for(;;)
		{
			socket_read(curr_fd)
			
			if(PACKTYPE IS CORRECT)
			{
				//READ AGAIN TO GET LENGTH
				socket_read(curr_fd);
				
				socket_write(curr_fd, (char*) arg);
								
				socket_read(curr_fd);
				
				socket_write(curr_fd, type);
				socket_write(curr_fd, packlength);
				socket_write(curr_fd, packdata)
				
			}
			else
			{
				read_from_tap();
				read_from_tap();
			}
			
		}
	}


		int main(int argc, char* argv[]) {
			if(argc == 3)
			{
				server = 1;
			}
			//check if argc == 3
			// if it does then we are server, set PORT and TAP_DEV

			else if (argc == 4)
			{
				
			}
			//check if argc == 4
			// if it does then we are client, set HOST, PORT, TAP_DEV
			// make new socket
			// start tcp thread with socket as argument
		}

