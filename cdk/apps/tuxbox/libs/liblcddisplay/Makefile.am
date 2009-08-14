lib_LTLIBRARIES = liblcddisplay.la

pkgincludedir = ${includedir}/tuxbox/lcddisplay
pkginclude_HEADERS = fontrenderer.h lcddisplay.h

liblcddisplay_la_SOURCES = lcddisplay.cpp fontrenderer.cpp

AM_CXXFLAGS = \
	@FREETYPE_CFLAGS@ \
	@PNG_CFLAGS@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tuxbox-lcddisplay.pc

