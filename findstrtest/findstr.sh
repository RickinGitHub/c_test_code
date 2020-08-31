#!/bin/bash
echo "find specific string"

#cd /data/cfg

if [ -z "`grep "device_name" wpa_supplicant.conf &`" ] ;then
	echo device_name=dangbei >> wpa_supplicant.conf
	echo devoce_type=1-0050F204-1 >> wpa_supplicant.conf
	echo p2p_go_intent=0 >> wpa_supplicant.conf
else
	echo 'device_name existed'
fi

#cd /

# Startup p2p daemon
#wpa_cli -D &

# Startup server for VoIP
#cd /oem
if [ -f "server_duplex" ];then
	chmod 777 server_duplex
	ifconfig
	./server_duplex
else
	echo "server_duplex inexsitent!!!"
fi
#cd -

