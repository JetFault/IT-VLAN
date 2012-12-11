CC = gcc -std=c99
CCFLAGS = -gdwarf-2 -g3

all: serialization

serialization: vlanpacket.o
		$(CC) $(CCFLAGS) -o serialization serialization_test.c  vlanpacket.o 

sending: vlanpacket.o connect.o 
		$(CC) $(CCFLAGS) -o sending packet_sending_test.c vlanpacket.o connect.o -lpthread

connect.o: ../connect.c
	$(CC) $(CCFLAGS) -c ../connect.c

tap.o: tap.c
	$(CC) $(CCFLAGS) -c ../tap.c

vlanpacket.o: ../vlanpacket.c
	$(CC) $(CCFLAGS) -c ../vlanpacket.c


clean:
	rm -f *.o
	rm -f serialization
	rm -f sending
