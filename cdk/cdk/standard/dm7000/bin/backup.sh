#!/bin/sh

cmdline=$1
box=`cat /proc/bus/dreambox | grep ^type | sed -e 's/type=//'`

if [ "$2" = "HDD" ] ; then
	device="/media/hdd"
elif [ "$2" = "USB" ] ; then
	device="/media/usb"
elif [ "$2" = "db" ] ; then
	device="dummy"
else
	device=$2
fi

backup="$device/backup/$box"
addons="$backup/ADD_ONS"
v=/var

CanRestore()
{	mount | grep $device && exit 0
	exit 1
}

restore()
{ # $1 source $2 text $3 standard/addons dir
	if [ -n "$3" ] ; then
		source=$addons$1
	else
		source=$backup$1
	fi

	if [ ! -e $source ] ; then
		echo "File/Dir $source doesn't exist"
		return 1
	fi

	[ -d $source ] && [ -n "$3" ] && return 0

	target=`dirname $1`
	[ ! -d $target ] && echo "Making dir $target" && mkdir -p $target
	#echo "Restoring $source" 
	
	if [ -n "$2" ] ; then
		info=$2
	else
		info=$1
	fi
	if ! cp -af $source $target ; then
		echo "$info...	Not Restored"
		return 1
	fi
	echo "$info....		Restored"
	return 0
}


backup()
{	if [ -d $backup ] ; then
		 echo "Removing previous backup..."
		rm -rf $backup
	fi
	echo "Creating new backup...."

	echo "Regular files..."
	mkdir -p $backup/$v
	cd $v
	for i in bin etc keys lib scce share spool tuxbox ; do
		[ -e $i ] && echo "Copying $v/$i" && cp -a $i $backup/$v || echo "$v/$i...	Not Backed up"
	done

	echo "Add ons..."
	[ ! "`ls $v/tuxbox/installer/`" ] && echo "No Addons to Backup" && return 0 
	# Backup all contents.. The addons files were backupped during previous $v/tuxbox
	awk '{print $3}' $v/tuxbox/installer/* | tr -d '"' | while read file dummy ; do
		targetdir=`dirname $addons/$file`
		if [ -e $file -a ! -d $file ] ; then
			[ ! -d $targetdir ] && echo "Making dir <add ons>"`dirname $file` && mkdir -p $targetdir
			echo "Copying $file" && cp -p $file $targetdir || echo "$file...	Not Backed up"
		fi
	done
	echo "Done !"
}

settings()
{	rm -rf $v/tuxbox/config/enigma
	restore $v/tuxbox/config/enigma
	restore $v/etc/satellites.xml "Settings"
}

samba()
{	restore $v/etc/smb.conf "Samba Config"
}

cams()
{	restore $v/etc/plimgr "Softcam Config"
}

etc()
{	restore $v/etc "etc"
}

bin()
{	restore $v/bin "bin"
}

keys()
{	restore $v/keys "Keys"
	restore $v/scce "Scce"
}

lib()
{	restore $v/lib "Lib"
}

share()
{	restore $v/share "Share"
}

spool()
{	restore $v/spool "Spool"
}

config()
{	restore $v/tuxbox/config "Complete Config"
}

enigma()
{	restore $v/tuxbox/config/enigma "Enigma"
}

addons()
{	# first copy back all addons files
	[ ! "`ls $backup/$v/tuxbox/installer/`" ] && echo "No Addons to Restore" && return 0 
	restore $v/tuxbox/installer "Addons Files"
	# then all contents
	awk '{print $3}' $backup/$v/tuxbox/installer/* | tr -d '"' | while read file dummy ; do
		restore $file "" true
	done
	echo "Addons Contents....	 Restored"
}

reload_notify()
{	echo "---------"
	echo "Now choose Reload Settings"
	echo "in backup/restore menu"
}

reloadenigma()
{	killall -9 enigma
	#touch /tmp/.ReloadBouquets
}


[ -z "${device}" ] && echo "No Device Given" && exit 1

case "${cmdline}" in
"restoreq")	CanRestore;;
"backup")	backup;;
"settings")	settings
		reload_notify;;
"addons")	addons;;
"config")	config;;
"enigma")	enigma;;
"cams")		cams;;
"samba")	samba;;
"bin")		bin;;
"etc")		etc;;
"lib")		lib;;
"spool")	spool;;
"share")	share;;
"keys")		keys;;
"reloadenigma")	reloadenigma;;
"all")	config
	settings
	bin
	etc
	lib
	spool
	share
	keys
	addons
	cams
	samba
	reload_notify
	;;
esac
