#!/bin/sh
# Copy /var to another location

MOVEPATH=$2

eval `grep ^version /.version`
VERSION=`echo ${version} | cut -c 1-12`
DATE=`date +%y%m%d%H%M%S`
TARGETPATH=${MOVEPATH}/pli/${VERSION}

movevar()
{	
	echo "Going to move to " ${MOVEPATH}
	touch ${MOVEPATH}/.test

	# check if ${MOVEPATH} exists
	if [ ! -e ${MOVEPATH}/.test ]; then
		echo ${MOVEPATH} " could not be written"
		exit
	fi

	rm ${MOVEPATH}/.test

	# check if ${MOVEPATH}/pli/${VERSION} exists and is already linked
	if [ -e ${TARGETPATH} ]; then
		echo "/var is already moved to " ${MOVEPATH}
		exit
	fi

	# /var not yet moved to ${MOVEPATH}
	echo "Creating /var on " ${MOVEPATH}

	if [ -e ${TARGETPATH} ]; then
		echo "${TARGETPATH} exists"
	else
		mkdir -p ${TARGETPATH}
	fi

	echo "Copying /var"
	cp -a /var ${TARGETPATH}

	# make sure our copy was good
	if [ ! -e ${TARGETPATH}/var ]; then
		echo "Copy failed!!!"
		exit
	fi

	echo "Success!!!"
	echo "Your Dreambox will reboot shortly."
	sync
	reboot
}

remove_moved_var()
{
	# check if ${MOVEPATH}/pli/${VERSION} exists and is already linked
	if [ -e ${TARGETPATH} ]; then
		mv ${TARGETPATH} ${MOVEPATH}/pli/removeatboot
		echo "Success!!!"
		echo "Your Dreambox will reboot shortly."
		sync
		reboot
	fi
}

unmove_var()
{
	for i in "/media/hdd" "/media/usb" "/media/cf" "/media/var"; do
	# check if /xxx/pli/${VERSION} exists 
		if [ -e $i/pli/${VERSION} ]; then
			mkdir -p $i/pli/varbackup
			mv $i/pli/${VERSION} $i/pli/varbackup/${DATE}
		fi
	done

	sync

	exit
}


case $1 in 
	move)
		movevar
		;;
	remove)
		remove_moved_var
		;;
	unmove)
		unmove_var
		;;
	*)	echo "No command given"
		exit 2
		;;
esac
