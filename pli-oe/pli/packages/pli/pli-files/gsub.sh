#!/bin/sh
#
# start/stop gsub

if ! [ -x /usr/bin/gSUB ]; then
	exit 0
fi

case "$1" in
	start)
		start-stop-daemon -S -b -x /usr/bin/gSUB
		;;
	stop)
		start-stop-daemon -K -x /usr/bin/gSUB
		;;
	*)
		;;
esac

exit 0

