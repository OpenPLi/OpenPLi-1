#!/bin/sh
#
# ppanelupdate.sh
# Downloads ppanel(s) from the server and installs them if the versions differ

VERSION=0.12
RELEASE=openpli

#
# Check for new version
#
CURR_VERSION=`echo $VERSION | tr -d '.'`

if [ -x /var/bin/ppanelupdate.sh ]; then
   OTHER_VERSION=`grep ^VERSION= /var/bin/ppanelupdate.sh | cut -d= -f2 | tr -d '.'`
   [ "$OTHER_VERSION" -gt "$CURR_VERSION" ] && exec /var/bin/ppanelupdate.sh "$@"
fi

if [ -x /bin/ppanelupdate.sh ]; then
   OTHER_VERSION=`grep ^VERSION= /bin/ppanelupdate.sh | cut -d= -f2 | tr -d '.'`
   [ "$OTHER_VERSION" -gt "$CURR_VERSION" ] && exec /bin/ppanelupdate.sh "$@"
fi

TEMPPANEL=/tmp/ppanel.tar.gz.tmp
TEMPVER=/tmp/ppanel.ver
TEMPPANELDIR=/tmp/ppaneltemp
TEMPFILELIST=/tmp/ppanelfiles
TEMPREFRESH=/tmp/ppanelrefresh
TEMPWGETOUT=/tmp/ppanelwgetout

URL=`echo $1 | sed s/#RELEASE#/$RELEASE/`
DESTDIR=$2
VERURL=`echo $URL | sed s/.tar.gz/.ver/`

echo "PPanelupdate version $VERSION"

rm -f $TEMPREFRESH

# Retrieve current .ver file
CURRENTVERFILE=`echo $VERURL | sed "s/.*\///"`
if [ -f /var/etc/$CURRENTVERFILE ] ; then
   CURRENTVERSION=`cat /var/etc/$CURRENTVERFILE | grep VERSION | cut -f2 -d=`
else
	 # No version file, take version 0 to force download
	 CURRENTVERSION=0
fi

# Download .ver file first
if wget -O $TEMPVER "$VERURL" > $TEMPWGETOUT ; then
    cat $TEMPWGETOUT
    SERVERVERSION=`cat $TEMPVER | grep VERSION | cut -f2 -d=`
    SERVERMD5=`cat $TEMPVER | grep -v VERSION | cut -f1 -d' '`
    echo "Version on Dreambox: $CURRENTVERSION"
    echo "Version on server: $SERVERVERSION"
    VERFOUND=1
else
    # No version file, this looks like a non-PLi server
    cat $TEMPWGETOUT
    echo "Downloading from a non-PLi server"
    VERFOUND=0
fi

if [ $VERFOUND -eq 1 ] ; then
   # Compare versions
   if [ 0$SERVERVERSION -gt 0$CURRENTVERSION ] ; then
      # Download tarball
      if wget -O $TEMPPANEL "$URL" ; then
         DOWNLOADEDMD5=`md5sum $TEMPPANEL | cut -f1 -d' '`
         if [ x$DOWNLOADEDMD5 = x$SERVERMD5 ] ; then
            echo "Check 1: tarball checksum valid"
         else
            # MD5 do not match
            # In most cases the servers are not in sync now
            echo
            echo "Menu was already up-to-date"
            echo "But it looks like a new menu will"
            echo "be available in a short time"
            exit 0
         fi
      else
         echo
         echo "**************************************"
         echo "Server is down or you have no internet"
         echo "connection. Please try later again."
         echo "**************************************"
         exit 1
      fi
   else
	    echo
      echo "Menu was already up-to-date"
      exit 0
   fi
else
   # No version found, download tarball anyway
   if ! wget -O $TEMPPANEL "$URL" ; then
      echo
      echo "**************************************"
      echo "Server is down or you have no internet"
      echo "connection. Please try later again."
      echo "**************************************"
      exit 1
   fi
   # We could add here the code to check the diffs between files
fi

# Install tarball
mkdir -p $TEMPPANELDIR
tar xzvf $TEMPPANEL -C $TEMPPANELDIR > $TEMPFILELIST
for XMLFILES in `grep "\.xml"  $TEMPFILELIST` ; do
   if ! grep -q "</directory>" $TEMPPANELDIR/$XMLFILES  ; then
      echo "Check 2: $XMLFILES seems corrupt. Quitting!"
      exit 1
   else
      echo "Check 2: xml files in tarball valid"
   fi
done

# Trick with tar in order to copy over symlink when used in OE images
tar -C $DESTDIR -zxf $TEMPPANEL
if [ $VERFOUND -eq 1 ] ; then
	touch $TEMPREFRESH
	cp $TEMPVER /var/etc/$CURRENTVERFILE
fi
echo
echo "Menu updated!"

rm -Rf $TEMPPANEL $TEMPPANELDIR $TEMPFILELIST $TEMPVER $TEMPWGETOUT

#
# The End
#
