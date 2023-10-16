#!/bin/bash
#
# $1: srcip, $2: srcport, $3: dstip, $4: dstport
#
# by Fabian Bieker <fabian.bieker@web.de>
# modified by DataSoft Corporation

. scripts/misc/base.sh

SRCIP=$1
SRCPORT=$2
DSTIP=$3
DSTPORT=$4

SERVICE="lpd"
HOST="serv"

my_start

while read name; do

	# remove control-characters
	name=`echo $name | sed s/[[:cntrl:]]//g`

	echo "$name" >> $LOG
	echo "$HOST: lpd: Illegal service request"
done

my_stop
