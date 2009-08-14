#!/bin/sh
#
# This script will mount an USB stick to /media/usb
#
#

USB=/dev/scsi/host0/bus0/target0/lun0

if [ -e $USB/disc ] ; then
	echo === Mount USB ===
	if [ -e $USB/part1 ] ; then
		mount $USB/part1 /media/usb
	else
		mount $USB/disc /media/usb
	fi
fi

#
# The End
#
