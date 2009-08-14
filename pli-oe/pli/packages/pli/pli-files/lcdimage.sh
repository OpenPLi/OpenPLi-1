#!/bin/sh

usage()
{
	echo "Usage: $0 {start|stop|status|restart|reload}"
}

if [ $# -lt 1 ] ; then usage ; break ; fi
action=$1

case "$action" in

start)
	echo -e "Show PLi\xAE lcd logo"
	/usr/bin/lcdimage.ppc /usr/share/misc/pli_logo
	;;

stop)
	;;

status)
	;;

restart|reload)
	$0 stop
	$0 start
	;;

*)
	usage
	;;

esac

exit 0
