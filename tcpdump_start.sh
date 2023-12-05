#!/bin/sh
rm -f nohup.out
sudo nohup tcpdump -ni ens160 -s 65535 -w captured_packets.pcap &

# Write tcpdump's PID to a file
echo $! > /var/run/tcpdump.pid
                                