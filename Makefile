CC = gcc -std=c99
CCFLAGS = -g -gdwarf-2 -g3

all: cs352proxy 

cs352proxy: connect.o tap.o vlanpacket.o linkstate.o
		$(CC) $(CCFLAGS) -o cs352proxy -lpthread cs352proxy.c tap.o connect.o vlanpacket.o linkstate.o

connect.o: connect.c
	$(CC) $(CCFLAGS) -c connect.c

linkstate.o: linkstate.c
	$(CC) $(CCFLAGS) -c linkstate.c

tap.o: tap.c
	$(CC) $(CCFLAGS) -c tap.c

vlanpacket.o: vlanpacket.c
	$(CC) $(CCFLAGS) -c vlanpacket.c


clean:
	rm -f *.o
	rm -f cs352proxy
