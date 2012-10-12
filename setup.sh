#! /bin/bash

PATH=$PATH:/usr/sbin:/sbin


if [ "$1" == "server" ]; then
	IP_END=1
elif [ "$1" == "client" ]; then
	IP_END=2
else
	echo "RUN like this: $0 client for 10.2.2.2"
	echo -e "\t $0 server for 10.2.2.1"
	exit
fi

#Create tap devices on each machine by executing this command:
sudo openvpn --mktun --dev tun2
 
#The the kernel the interface is active/up:
sudo ip link set tun2 up
 
#Add an IP to the interface and tell the kernel to route packets matching the first 24 bits of the address to it:
sudo ip addr add 10.2.2.${IP_END}/24 dev tun2
 
#Verify the interface is up and packets will be routed to it by looking at the routing table:
echo -e "\nRouting to:\n\t`sudo route -n | grep tun2`"
