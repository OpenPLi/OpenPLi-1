#!/bin/sh
#
# This script will reload kernel mods, some can be installed on /var which was not mounted in an earlier stage
#
#

/etc/init.d/init1
# check if /media/hdd is already mounted, if not try to mount it now, it is possible that reiserfs is now loaded for instance
if ! mount | grep -q -c /media/hdd ; then 
	/bin/mount_hdd_cf.sh
fi

exit 0
