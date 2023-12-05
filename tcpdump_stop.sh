#!/bin/sh
if [ -f /var/run/tcpdump.pid ]
then
        kill `cat /var/run/tcpdump.pid`
        echo tcpdump `cat /var/run/tcpdump.pid` killed.
        rm -f /var/run/tcpdump.pid
else
        echo tcpdump not running.
fi