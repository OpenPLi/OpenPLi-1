#!/bin/sh
#
# ppanelrecovery.sh DM500PLUS version
# Tries to download known XML files from the server if the file is missing.

VERSION=0.8
RELEASE=jade
BOX=dm500plus

# Check for new version
if [ -x /var/bin/ppanelrecovery.sh ]; then
   CURR_VERSION=`echo $VERSION | tr -d '.'`
   VAR_VERSION=`grep ^VERSION= /var/bin/ppanelrecovery.sh | cut -d= -f2 | tr -d '.'`
   [ "$VAR_VERSION" -gt "$CURR_VERSION" ] && exec /var/bin/ppanelrecovery.sh
fi

XMLFILE=$1

echo "PPanelrecovery version $VERSION"
echo "The file $XMLFILE is"
echo "missing on your system."
echo "Downloading the latest version now..."

case $XMLFILE in
   /var/etc/software.xml)
   rm /var/etc/software.ver
   ppanelupdate.sh http://servername/$BOX/software.tar.gz /
   rm -f /tmp/ppanelrefresh
   ;;

   *)
   echo "Unknown XML file, could not be recovered"
   ;;
esac

