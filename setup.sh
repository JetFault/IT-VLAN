#! /bin/bash

PATH=$PATH:/usr/sbin:/sbin

if [ "$1" == "" ]; then
	echo "RUN like this: $0 <IPEND> for 10.2.2.<IPEND>"
	exit
fi

#Create tap devices on each machine by executing this command:
sudo openvpn --mktun --dev tap2 --lladdr "DE:AD:AC:BD:09:0$1"
 
#The the kernel the interface is active/up:
sudo ip link set tap2 up
 
#Add an IP to the interface and tell the kernel to route packets matching the first 24 bits of the address to it:
sudo ip addr add 10.2.2.$1/24 dev tap2
 
#Verify the interface is up and packets will be routed to it by looking at the routing table:
echo -e "\nRouting to:\n\t`sudo route -n | grep tap2`"
