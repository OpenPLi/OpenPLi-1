AC_INIT(Makefile.am)
AM_INIT_AUTOMAKE(tuxbox-hostapps, 0.0.1)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_DISABLE_STATIC

AM_PROG_LIBTOOL

AC_HEADER_STDC

AC_CHECK_LIB(crypto,PEM_write_bio,[HAVE_LIBCRYPTO="yes"],[AC_MSG_WARN([*argh*])])
AC_CHECK_HEADER(openssl/bio.h,[HAVE_LIBCRYPTO="yes"],[AC_MSG_WARN([*argh*])])

AC_OUTPUT([
Makefile
bmp2raw/Makefile
flash/Makefile
grab/Makefile
libcrypto++/Makefile
libflashimage/Makefile
stream/Makefile
ucodex/Makefile
])

