#!/bin/sh
#
# This script will mount CF and HDD to the correct mount points
#
#

# if /proc/ide/hdb exist mount this on /media/hdd 
if [ -e /proc/ide/hdb ] ; then
	HD=1
else
	HD=0
fi

HDD=/dev/ide/host0/bus0/target${HD}/lun0
CF=/dev/ide/host0/bus0/target0/lun0

if [ $HD -eq 1 -a -e $CF/part1 ] ; then
	echo === Mount Compact Flash ===
	# be sure this device is not mounted so unmount first!
	umount $CF/part1 >/dev/null 2>&1
	mount $CF/part1 /media/cf -osync
fi

if [ -e $HDD/part1 ] ; then 
	echo === Mount HDD ===
	umount $HDD/part1 >/dev/null 2>&1
	mount $HDD/part1 /media/hdd
fi
