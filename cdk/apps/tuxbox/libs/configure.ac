AC_INIT(tuxbox-apps-tuxbox-libs,0.0.1,,libs)
AM_INIT_AUTOMAKE(libs,0.0.1)

TUXBOX_APPS
TUXBOX_APPS_DIRECTORY
TUXBOX_BOXTYPE

AC_PROG_CC
AC_PROG_CXX
AC_DISABLE_STATIC
AM_PROG_LIBTOOL
AC_HEADER_STDC

TUXBOX_APPS_DRIVER
TUXBOX_APPS_DVB
TUXBOX_APPS_LIB_CONFIG(FREETYPE,freetype-config)
TUXBOX_APPS_LIB_PKGCONFIG(PNG,libpng)

AC_SUBST(LIBTUXBOX_LIBS)
if test "$BOXTYPE" = "dbox2" ; then
	LIBTUXBOX_LIBS=-ltuxbox
else
	LIBTUXBOX_LIBS=
fi

AC_OUTPUT([
Makefile
libavs/Makefile
libavs/tuxbox-avs.pc
liblcddisplay/Makefile
liblcddisplay/tuxbox-lcddisplay.pc
liblircdclient/Makefile
liblircdclient/tuxbox-lircdclient.pc
libtuxtxt/Makefile
libtuxtxt/tuxbox-tuxtxt.pc
])

