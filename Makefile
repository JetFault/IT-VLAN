CC = gcc
CCFLAGS = -g

all: cs352proxy 

cs352proxy: server.o client.o tap.o socket_read_write.o cs352proxy.c
		$(CC) $(CCFLAGS) -o cs352proxy -lpthread cs352proxy.c tap.o client.o server.o socket_read_write.o

server.o: server.c
	$(CC) $(CCFLAGS) -c server.c

client.o: client.c
	$(CC) $(CCFLAGS) -c client.c

tap.o: tap.c
	$(CC) $(CCFLAGS) -c tap.c

socket_read_write.o: socket_read_write.c
	$(CC) $(CCFLAGS) -c socket_read_write.c


clean:
	rm -f *.o
	rm -f cs352proxy
