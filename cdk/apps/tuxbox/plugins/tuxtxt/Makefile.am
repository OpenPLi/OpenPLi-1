SUBDIRS = font 

if BOXTYPE_DBOX2
AM_CPPFLAGS = \
	@TUXBOX_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@TUXTXT_CFLAGS@ \
	-I$(top_srcdir)/include 

tuxtxt_la_LIBADD = \
	@TUXBOX_LIBS@  \
	@FREETYPE_LIBS@ \
	@TUXTXT_LIBS@ 
else
AM_CPPFLAGS = \
	@FREETYPE_CFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(includedir)/tuxbox/tuxtxt 

tuxtxt_la_LIBADD = \
	@FREETYPE_LIBS@ \
	@TUXTXT_LIBS@
endif

noinst_LTLIBRARIES = tuxtxt.la

tuxtxt_la_SOURCES = tuxtxt.c

tuxtxt_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	install -d $(DESTDIR)$(CONFIGDIR)/tuxtxt
	$(LIBTOOL) --mode=install install tuxtxt.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/tuxtxt.cfg $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/tuxtxt.conf $(DESTDIR)$(CONFIGDIR)/tuxtxt
