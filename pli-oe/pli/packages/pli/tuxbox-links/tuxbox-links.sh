#!/bin/sh

ln -s . /etc/tuxbox/config
ln -s /etc/enigma /etc/tuxbox/
ln -s /usr/keys /var/
ln -s /etc/tuxbox/scce /var/
ln -s /usr/scam /var/
ln -s /usr/bin /var/
mkdir -p /usr/www
ln -s /usr/www /var/
mkdir -p /usr/wifi
ln -s /usr/wifi /var/
[ -e /var/lib ] && rm -rf /var/lib
ln -s /usr/lib /var/
ln -s /etc/cron /var/spool
ln -s /usr/lib /var/iptables
ln -s /usr/lib/libcrypto.so /usr/lib/libcrypto.so.0
#don't known whether anybody still needs this:
ln -s tuxbox/plugins/libfx2.so /usr/lib/libfx2.so
ln -s fb/0 /dev/fb0