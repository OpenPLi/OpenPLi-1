#!/bin/sh

VERSION=1.05

#
# Check for new version
#
if [ -x /var/bin/cardinfo-pli.sh ]; then
    CURR_VERSION=`echo $VERSION | tr -d '.'`
    VAR_VERSION=`grep ^VERSION= /var/bin/cardinfo-pli.sh | cut -d= -f2 | tr -d '.'`
    [ "$VAR_VERSION" -gt "$CURR_VERSION" ] && exec /var/bin/cardinfo-pli.sh
fi 

tf=/tmp/readme.tmp

rm -f $tf
dash="\n----------------------------------------"

plist=`ps | grep -v grep`

if echo "$plist" | grep -q rdgd ; then
	if [ -e /tmp/card00.info ] ; then
		echo "Radegast SmartCard information for Slot1$dash" >> $tf
		cat /tmp/card00.info >> $tf
	fi
	if [ -e /tmp/card01.info ] ; then
		echo "Radegast SmartCard information for Slot2$dash" >> $tf
		cat /tmp/card01.info >> $tf
	fi
fi

if echo "$plist" | grep -q cardserver ; then
	if [ -e /var/tuxbox/config/cardserv.cfg ] ; then #newcamd 5.xx
		CARDSERV="/var/tuxbox/config/cardserv.cfg"
	elif [ -e /var/tuxbox/config/newcamd/cardserv.cfg ] ; then #newcamd 6.xx
		CARDSERV="/var/tuxbox/config/newcamd/cardserv.cfg"
	fi
	port1=`grep DEBUG_PORT $CARDSERV | grep -v "^#" | tr -d "\r" | sed "s/^.*=\(.*\)/\1/;s/^ //"`
	port2=`grep ENTITLEMENT_PORT $CARDSERV | grep -v "^#" | tr -d "\r" | sed "s/^.*=\(.*\)/\1/;s/^ //"`
	echo "Cardserver SmartCard information$dash" >> $tf
	nc localhost $port2 | sed -e "s/  */ /g" -e "s/---*/---------------------------/" -e "s/.\{38\}[ :]/&\n /g" >> $tf
	nc localhost $port1 | sed -e "s/  */ /g" -e "s/, card/\ncard/" -e "s/online /online\n /" >> $tf
fi

if echo "$plist" | grep -q serverng ; then
	if [ -e /tmp/sc_info0 ] ; then
		echo "CamX SmartCard information for Slot1$dash" >> $tf
		cat /tmp/sc_info0 >> $tf
	fi
	if [ -e /tmp/sc_info1 ] ; then
		echo "CamX SmartCard information for Slot2$dash" >> $tf
		cat /tmp/sc_info1 >> $tf
	fi
fi

if echo "$plist" | grep -q gbox ; then
	if [ -e /tmp/sc01.info ] ; then
		echo "Gbox SmartCard information for Slot1$dash" >> $tf
		cat /tmp/sc01.info >> $tf
	fi
	if [ -e /tmp/sc02.info ] ; then
		echo "Gbox SmartCard information for Slot2$dash" >> $tf
		cat /tmp/sc02.info >> $tf
	fi
fi

if echo "$plist" | grep -q newcs ; then
	port3=`grep "<tcp_port>" /var/tuxbox/config/newcs.xml | sed "s/^.*<.*>\(.*\).*<.*>.*$/\1/"`
	[ -z "$port3" ] && port3=16000
	echo "NewCS SmartCard information$dash" >> $tf
	( echo "NewCSpwd"
	sleep 1
	echo "sub -1/n" 
	) | nc localhost $port3 >> $tf 
fi

if echo "$plist" | grep -q CCcam ; then
	port4=`grep -i "^INFO LISTEN PORT" /var/etc/CCcam.cfg | sed "s/.*:\(.*\)/\1/"` 
	[ -z "$port4" ] && port4=16000
	echo "CCcam SmartCard information$dash" >> $tf
	echo "entitlements" | nc localhost $port4 >> $tf
fi

[ ! -e $tf ] && echo "No cards inserted\nOr check your softcam configuration" > $tf

cat $tf

#
# The End
#
