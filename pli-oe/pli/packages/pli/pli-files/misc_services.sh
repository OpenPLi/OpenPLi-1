#!/bin/sh

usage()
{
	echo "Usage: $0 {start|stop|status|restart|reload}"
}

if [ $# -lt 1 ] ; then usage ; break ; fi
action=$1

[ -e /etc/rc.config ] && . /etc/rc.config

case "$action" in

start)
	# Start ecmhelper if available
	if [ -x /usr/bin/pli_ecmhelper ]; then
		/usr/bin/pli_ecmhelper
	fi

	# If Inadyn is installed and enabled start it
	if [ "$INADYN" = enabled ] ; then
		echo "Going to start Inadyn"
		/usr/bin/inadyn --input_file /etc/inadyn.config
	fi
	
	# If firewall needs to be started do it now
	if [ "$FIREWALL" = enabled -a -x "/usr/bin/firewall.sh" ] ; then
		/usr/bin/firewall.sh start &
	fi

	# If Apache server is installed and enabled start it
	if [ "$APACHE" = enabled -a -x /hdd/opt/httpd/bin/apachectl ] ; then
		echo "Going to start Apache"
		/hdd/opt/httpd/bin/apachectl start
	fi 
	
	# If gSUB is installed and is enabled, start it
	if [ -x /usr/bin/gSUB -a "$GSUB" = enabled ] ; then 
		/etc/init.d/gsub start
	fi
	;;

stop)
	killall pli_ecmhelper
	
	# If Apache server is installed attempt to stop it
	if [ -x /hdd/opt/httpd/bin/apachectl ] ; then
		echo "Going to stop Apache"
		/hdd/opt/httpd/bin/apachectl stop
	fi 
	
	# If firewall needs to be stopped do it now
	if [ "$FIREWALL" = enabled -a -x "/usr/bin/firewall.sh" ] ; then
		/usr/bin/firewall.sh stop &
	fi

	# If Gsub is enabled stop it
	if [ "$GSUB" = enabled ] ; then 
		/etc/init.d/gsub stop
	fi

	# If Inadyn is enabled stop it
	if [ "$INADYN" = enabled ] ; then
		killall inadyn
	fi
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
