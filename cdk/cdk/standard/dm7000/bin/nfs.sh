#!/bin/sh

case "$1" in
	'start')
		[ ! -d /var/lib/nfs ] && mkdir /var/lib/nfs
		[ ! -d /var/lib/nfs/sm ] && mkdir /var/lib/nfs/sm
		[ ! -d /var/lib/nfs/sm.bak ] && mkdir /var/lib/nfs/sm.bak
		[ ! -f /var/lib/nfs/etab ] && touch /var/lib/nfs/etab
		[ ! -f /var/lib/nfs/xtab ] && touch /var/lib/nfs/xtab
		[ ! -f /var/lib/nfs/rmtab ] && touch /var/lib/nfs/rmtab
		[ ! -f /var/lib/nfs/state ] && touch /var/lib/nfs/state

		export INSMOD="/sbin/insmod"
		export MODDIR="/lib/modules/"$(uname -r)

		$INSMOD $MODDIR/kernel/fs/exportfs/exportfs.ko
		$INSMOD $MODDIR/kernel/fs/nfsd/nfsd.ko

		/sbin/portmap
		/sbin/rpc.mountd
		/sbin/rpc.nfsd 2
		/sbin/exportfs -ra
	;;
	
	'stop')
		/sbin/exportfs -ua
		killall rpc.nfsd
		killall rpc.mountd
		killall portmap
		
		# Cannot rmmod the kernel modules here, it "hangs"
		
		rm -rf /var/lib/nfs
	;;
	
	*)
		$0 start
	;;
esac
	
