#!/bin/sh
	echo Restoring backup
	cp -a /tmp/var_backup/keys /var
	cp -a /tmp/var_backup/scce /var
	mkdir /var/etc
	cp -a /tmp/var_backup/etc/rc.config /var/etc
	cp -a /tmp/var_backup/etc/init /var/etc/init.save
	cp -a /tmp/var_backup/etc/CCcam.cfg /var/etc
	cp -a /tmp/var_backup/keys/CCcam.ignore /var/keys
	cp -a /tmp/var_backup/etc/camx.config /var/etc
	cp -a /tmp/var_backup/etc/radegast.cfg /var/etc
	cp -a /tmp/var_backup/etc/radegast.users /var/etc
	cp -a /tmp/var_backup/etc/firewall.users /var/etc
	cp -a /tmp/var_backup/etc/passwd /var/etc
	cp -a /tmp/var_backup/etc/ppanels/* /var/etc/ppanels
	mkdir -p /tmp/var_backup/tuxbox/config/enigma/
	cp -a /tmp/var_backup/tuxbox/config/cardserv.cfg /var/tuxbox/config
	cp -a /tmp/var_backup/tuxbox/config/cardspider.cfg /var/tuxbox/config
	cp -a /tmp/var_backup/tuxbox/config/newcamd.conf /var/tuxbox/config
	cp -a /tmp/var_backup/tuxbox/config/newcs.xml /var/tuxbox/config
	cp -a /tmp/var_backup/tuxbox/config/wifiplugin.cfg /var/tuxbox/config
	cp -a /tmp/var_backup/tuxbox/config/satellites.xml /var/tuxbox/config
	cp -a /tmp/var_backup/tuxbox/config/enigma/config /var/tuxbox/config/enigma
	cp -a /tmp/var_backup/tuxbox/config/enigma/timer.epl /var/tuxbox/config/enigma
	cp -a /tmp/var_backup/tuxbox/config/enigma/bouquets /var/tuxbox/config/enigma
	cp -a /tmp/var_backup/tuxbox/config/enigma/userbouquet* /var/tuxbox/config/enigma
	cp -a /tmp/var_backup/tuxbox/config/enigma/services /var/tuxbox/config/enigma
	cp -a /tmp/var_backup/tuxbox/config/enigma/services.locked /var/tuxbox/config/enigma	
	mkdir -p /var/tuxbox/config/tuxtxt
	cp -a /tmp/var_backup/tuxbox/config/tuxtxt/tuxtxt.conf /var/tuxbox/config/tuxtxt
	rm -rf /tmp/var_backup
	sync

exit 0
