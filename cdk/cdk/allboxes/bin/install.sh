#!/bin/sh
# A package installer and remover for the dreambox

DEBUG=0
INSTALL_VERSION=1.37

#
# set these var to enable loging to file and saving downloaded files
#
#savedir=/media/hdd/dl
#logfile=/media/hdd/install.log

CURR_VERSION=`echo $INSTALL_VERSION | tr -d '.'`

if [ -x /var/bin/install.sh ]; then
	OTHER_VERSION=`grep ^INSTALL_VERSION= /var/bin/install.sh | cut -d= -f2 | tr -d '.'`
	[ "$OTHER_VERSION" -gt "$CURR_VERSION" ] && exec /var/bin/install.sh "$@"
fi

if [ -x /bin/install.sh ]; then
	OTHER_VERSION=`grep ^INSTALL_VERSION= /bin/install.sh | cut -d= -f2 | tr -d '.'`
	[ "$OTHER_VERSION" -gt "$CURR_VERSION" ] && exec /bin/install.sh "$@"
fi

debug()
{
	if [ $DEBUG -eq 1 ] ; then
		d=`date +%Y%m%d-%H%M%S`
		if [ -f "$logfile" ] ; then
			echo "$d: $*" >> $logfile
		else
			echo "$d: $*"
		fi
	fi
}

disk_free()
{
	fs=$1
	DF=`df -k 2>/dev/null | grep "$fs$" | tail -1 | sed 's:.* \([0-9][0-9]*\)  *[^ ]*%  */.*$:\1:'`
	expr $DF - $MIN_FREE # Leave $MIN_FREE k free on filesystem
}

disk_usage()
{
	du -s $1 | sed 's/	.*//'
}

extract_tar()
{
	[ ! -f "$TAR" ] && {
		echo "No package $PACKAGE found with version $VERSION"
		return 1
	}
	cd $TEMP/root || {
		echo "Cannot CD to dir: $TEMP/root"
		exit
	}
	[ -d /var/tuxbox/installer ] || mkdir -p /var/tuxbox/installer
	rm -f /tmp/.foutje
	if [ "$SETTINGS" = "1" ]; then
		( gzip -dc $TAR || touch /tmp/.foutje ) | tar xf - || /tmp/.foutje
	else
		LISTFILE=/var/tuxbox/installer/$PACKAGE
		rm -f $LISTFILE
		tar zxvf $TAR | while read file; do
			echo "[ -f \"/$file\" ] && rm -f \"/$file\"" >> $LISTFILE
		done
		chmod u+x "$LISTFILE"
	fi
	[ -e /tmp/.foutje ] && {
		echo "Error extracting package"
		return 1
	}
	return 0
}

do_install()
{
	if [ ! -f "$TAR" ]; then
		echo "$TAR not found"
		touch /tmp/installerror
		return 1
	fi
	extract_tar || {
		echo
		echo "************************************************"
		echo "Error extracting package"
		echo "************************************************"
		touch /tmp/installerror
		return 1
	}
	#find $TEMP/root
	if [ "$SETTINGS" = "1" ]; then
		rm -f /var/tuxbox/config/enigma/userbouquet*
	else
		DF="`disk_free $FS`"
		DU="`disk_usage $TEMP/root`"
		echo
		echo "Free space in $FS: $DF kB"
		echo "package needs: $DU kB"
		if [ "$DF" -le "$DU" ]; then
			echo
			echo "************************************************"
			echo "Not enough free space in $FS"
			echo "************************************************"
			touch /tmp/installerror
			return 1
		fi
	fi
	( cd $TEMP/root && tar cf - * ) | ( cd / ; tar xpf - ) || {
		echo
		echo "************************************************"
		echo "Error installing package"
		echo "************************************************"
		touch /tmp/installerror
		return 1
	}
	 
	if [ -e /tmp/postinstall.sh ]; then
		mv /tmp/postinstall.sh /tmp/postinstall.sh.$$
		sh /tmp/postinstall.sh.$$
		rm -f /tmp/postinstall.sh.$$
	fi
	 
	echo
	echo "Package $PACKAGE installed successfully"
}

do_ipkinstall()
{
	ipkg install $IPK || {
		echo
		echo "************************************************"
		echo "Error installing package"
		echo "************************************************"
		touch /tmp/installerror
		return 1
	}
	echo "ipkg remove $URL" > "/var/tuxbox/installer/$PACKAGE"
	chmod u+x "/var/tuxbox/installer/$PACKAGE"

	echo
	echo "Package $PACKAGE installed successfully"
}

do_remove()
{
	[ ! -f "/var/tuxbox/installer/$PACKAGE" ] && return

	. "/var/tuxbox/installer/$PACKAGE"
	rm -f "/var/tuxbox/installer/$PACKAGE"
	echo
	echo "Package $PACKAGE removed successfully"
}

####################### MAIN #########################


ACTION=$1
PACKAGE=`echo $2 | tr '/' ' '`
VERSION=$3
URL=$4
[ "`echo $PACKAGE |grep ^set_`" != "" ] && SETTINGS=1
MIN_FREE=40
eval `grep ^version /.version`
V=`echo $version | sed -e 's/^00*20\(......\).*$/\1/'`

echo "Installer version $INSTALL_VERSION"
if [ $DEBUG -eq 1 ] ; then
	debug "main: V=$V"
	debug "main: ACTION=$ACTION"
	debug "main: PACKAGE=$PACKAGE"
	debug "main: VERSION=$VERSION"
	debug "main: URL=$URL"
else
	echo
	echo "Requesting action '$ACTION'"
	echo "for package $PACKAGE ($VERSION)"
fi

rm -f /tmp/installerror

case $PACKAGE in
	set_*|var_*)
		TEMP=/var/tmp/TempDirForInstaller
		FS=/var
		mkdir -p $TEMP/root
		trap "rm -rf $TEMP ; exit" 0 1 2 3 15
		# Test for the case that /var is not a separate mount
		if [ `df -k 2>/dev/null | grep "$FS$" | wc -l` -eq 0 ] ; then
			FS=/
		fi
		;;
	hdd_*)
		TEMP=/media/hdd/tmp/TempDirForInstaller
		FS=/media/hdd
		mkdir -p $TEMP/root
		trap "rm -rf $TEMP ; exit" 0 1 2 3 15
		;;
	ipk_*)
		;;
	*)
		echo
		echo "************************************************"
		echo "Invalid package name $PACKAGE"
		echo "************************************************"
		touch /tmp/installerror
		exit
		;;
esac

TAR=$TEMP/tmp.tgz
IPK=$TEMP/tmp.ipk

PACKAGE=`echo $PACKAGE | sed 's/^[^_]*_//'`

case $ACTION in
	install)
		if [ "$SETTINGS" != "1" ]; then
			if [ -e "/var/tuxbox/installer/$PACKAGE" ]; then
				echo
				echo "************************************************"
				echo "Package $PACKAGE already installed"
				echo "You have to remove this packet first"
				echo "before you can reinstall it"
				echo "************************************************"
				touch /tmp/installerror
				exit
			fi
		fi
		do_install || do_remove
		;;
	ipkupgrade)
		URL=$PACKAGE
		do_ipkinstall || do_remove
		;;
	ipkinstall)
		if [ "$SETTINGS" != "1" ]; then
			if [ -e "/var/tuxbox/installer/$PACKAGE" ]; then
				echo
				echo "************************************************"
				echo "Package $PACKAGE already installed"
				echo "You have to remove this packet first"
				echo "before you can reinstall it"
				echo "************************************************"
				touch /tmp/installerror
				exit
			fi
		fi
		URL=$PACKAGE
		do_ipkinstall || do_remove
		;;
	ipkfeedinstall)
		IPK=$URL
		ipkg update
		do_ipkinstall || do_remove
		;;
	remove)
		do_remove
		;;
	upgrade)
		do_remove && do_install
		;;
	*)
		echo
		echo "************************************************"
		echo "Invalid action: $ACTION"
		echo "************************************************"
		touch /tmp/installerror
		exit
		;;
esac
