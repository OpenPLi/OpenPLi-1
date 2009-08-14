AC_INIT(tuxbox-neutrino,0.0.1)
AM_INIT_AUTOMAKE(tuxbox-neutrino,0.0.1)

TUXBOX_APPS
TUXBOX_APPS_DIRECTORY

AC_PROG_CC
AC_PROG_CXX
AC_DISABLE_STATIC
AM_PROG_LIBTOOL

TUXBOX_APPS_DVB
TUXBOX_APPS_DRIVER
TUXBOX_APPS_LIB_PKGCONFIG(CONFIGFILE,tuxbox-configfile)
TUXBOX_APPS_LIB_PKGCONFIG(CONNECTION,tuxbox-connection)
TUXBOX_APPS_LIB_PKGCONFIG(CRAMFS,tuxbox-cramfs)
TUXBOX_APPS_LIB_CONFIG(CURL,curl-config)
TUXBOX_APPS_LIB_PKGCONFIG(EVENTSERVER,tuxbox-eventserver)
TUXBOX_APPS_LIB_CONFIG(FREETYPE,freetype-config)
TUXBOX_APPS_LIB_PKGCONFIG(LCDDISPLAY,tuxbox-lcddisplay)
TUXBOX_APPS_LIB_PKGCONFIG(MD5SUM,tuxbox-md5sum)
TUXBOX_APPS_LIB_PKGCONFIG(NET,tuxbox-net)
TUXBOX_APPS_LIB_PKGCONFIG(UCODES,tuxbox-ucodes)
TUXBOX_APPS_LIB_PKGCONFIG(ZAPIT,tuxbox-zapit)

AC_OUTPUT([
Makefile
lib/Makefile
lib/controldclient/Makefile
lib/lcddclient/Makefile
lib/sectionsdclient/Makefile
lib/timerdclient/Makefile
daemons/Makefile
daemons/controld/Makefile
daemons/controld/driver/Makefile
daemons/nhttpd/Makefile
daemons/nhttpd/data/Makefile
daemons/nhttpd/data/images/Makefile
daemons/lcdd/Makefile
daemons/lcdd/clock/Makefile
daemons/lcdd/icons/Makefile
daemons/sectionsd/Makefile
daemons/timerd/Makefile
src/Makefile
src/daemonc/Makefile
src/driver/Makefile
src/gui/Makefile
src/gui/bedit/Makefile
src/gui/widget/Makefile
src/system/Makefile
data/Makefile
data/fonts/Makefile
data/icons/Makefile
data/locale/Makefile
])

