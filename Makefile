CC = gcc
CCFLAGS = -Wall -g

all: cs352proxy 

cs352proxy: proxy.o
		$(CC) $(CCFLAGS) -o proxy proxy.o -lpthread

proxy.o:
	$(CC) $(CCFLAGS) -c Sockets.c


clean:
	rm -f *.o
	rm -f cs352proxy
