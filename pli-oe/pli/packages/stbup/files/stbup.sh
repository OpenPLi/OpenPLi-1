#!/bin/sh
#
# start/stop stbup

if ! [ -x /usr/bin/stbup ]; then
	exit 0
fi

case "$1" in
	start)
	echo -n "Starting stbup daemon... "
	/usr/bin/stbup &
	echo "done."
	;;
	stop)
	echo -n "Stopping stbup daemon... "
	killall stbup
	echo "done."
	;;
	restart)
	echo "* Restarting stbup daemon... "
	$0 stop
	$0 start
	echo "done."
	;;
	*)
	echo "Usage: /etc/init.d/stbup {start|stop|restart}"
	exit 1
	;;
esac

exit 0

