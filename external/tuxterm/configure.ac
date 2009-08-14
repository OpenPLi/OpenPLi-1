#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(main.c)
AM_INIT_AUTOMAKE(main,0.1)

TUXBOX_APPS
TUXBOX_APPS_DIRECTORY

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

TUXBOX_APPS_LIB_CONFIG(FREETYPE,freetype-config)

CPPFLAGS="$CPPFLAGS $FREETYPE_CFLAGS"

AC_ARG_WITH(bpp,
        [  --with-bpp=DEPTH [[32,8]]],
                [bpp="$withval"],[bpp="32"])

if test "$bpp" = "8"; then
	AC_DEFINE(FB8BIT,1,[use 8bit framebuffer])
fi

AC_OUTPUT([
Makefile
fonts/Makefile
])
