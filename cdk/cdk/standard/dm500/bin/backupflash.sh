#!/bin/sh
bd=/tmp/var_backup

echo Create a backup to $bd

mkdir -p $bd/etc $bd/tuxbox/config/enigma $bd/tuxbox/config/tuxtxt

cd /var
cp -a keys scce /$bd

cd etc
cp -a CCcam.cfg camx.config rc.config radegast.cfg radegast.users firewall.users passwd init ppanels satellites.xml $bd/etc

cd /var/tuxbox/config
cp -a cardserv.cfg cardspider.cfg newcamd.conf newcs.xml satellites.xml wifiplugin.cfg $bd/tuxbox/config
cp -a tuxtxt/tuxtxt.conf $bd/tuxbox/config/tuxtxt

cd enigma
cp -a config timer.epl bouquets userbouquet* services services.locked $bd/tuxbox/config/enigma

sync

exit 0
