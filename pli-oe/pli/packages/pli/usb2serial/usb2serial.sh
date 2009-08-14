#!/bin/sh
#
# start/stop usb2serial

case "$1" in
	start)
		echo "Starting usb2serial"
		modprobe usbserial
		modprobe ftdi_sio
		modprobe belkin_sa
		modprobe pl2303	
		;;
	stop)
		echo "Stopping usb2serial"
		rmmod pl2303
		rmmod belkin_sa
		rmmod ftdi_sio
		rmmod usbserial
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
