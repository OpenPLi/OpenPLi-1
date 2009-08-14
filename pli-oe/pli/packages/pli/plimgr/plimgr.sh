#!/bin/sh
#
# start/stop plimgr

if ! [ -x /usr/bin/plimgr ]; then
	exit 0
fi

case "$1" in
	start)
		echo -n "Starting PLi manager: "
		/usr/bin/plimgr
		start-stop-daemon -S -x /usr/bin/pli_ecmhelper
		echo "done."
		;;
	stop)
		echo -n "Stopping PLi manager: "
		/usr/bin/plimgr -q
		start-stop-daemon -K -x /usr/bin/pli_ecmhelper
		echo "done."
		;;
	restart|reload)
		$0 stop
		$0 start
		;;
	*)
		echo "Usage: $0 {start|stop|restart}"
		exit 1
		;;
esac

exit 0

