CC = gcc
CCFLAGS = -g

all: cs352proxy 

cs352proxy: connect.o tap.o vlanpacket.o cs352proxy.c
		$(CC) $(CCFLAGS) -o cs352proxy -lpthread cs352proxy.c tap.o connect.o vlanpacket.o

connect.o: connect.c
	$(CC) $(CCFLAGS) -c connect.c

tap.o: tap.c
	$(CC) $(CCFLAGS) -c tap.c

vlanpacket.o: vlanpacket.c
	$(CC) $(CCFLAGS) -c vlanpacket.c


clean:
	rm -f *.o
	rm -f cs352proxy
