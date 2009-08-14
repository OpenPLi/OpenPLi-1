lib_LTLIBRARIES = libtuxtxt.la

pkgincludedir = ${includedir}/tuxbox/tuxtxt
pkginclude_HEADERS = tuxtxt_def.h tuxtxt_common.h

libtuxtxt_la_SOURCES = libtuxtxt.c

AM_CPPFLAGS = \
	@FREETYPE_CFLAGS@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tuxbox-tuxtxt.pc

install-data-local:
	$(mkinstalldirs) $(FONTDIR)
	cp ${top_srcdir}/../plugins/tuxtxt/font/tuxtxt.otb $(FONTDIR)
