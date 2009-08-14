#!/bin/sh

/usr/bin/showiframe /boot/backdrop.mvi

# if the directory /var/media does not exist create it
if [ ! -d /var/media ] ; then
	mkdir -p /var/media	
fi

# if there is no symlink to /var/media/movie we create the default here
if [ ! -h /var/media/movie ] ; then
	ln -s /media/hdd/movie /var/media/movie	
fi

if [ ! -e /etc/enigma ] ; then
	cp -R /usr/share/enigma/default /etc/enigma
fi

/usr/bin/enigma

ret=$?
echo "Enigma has finished with exit code $ret"
case $ret in
	0)
		/sbin/halt
		;;
	4)
		/sbin/reboot
		;;
	*)
		;;
esac

