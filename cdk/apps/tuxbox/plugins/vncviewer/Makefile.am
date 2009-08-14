SUBDIRS = rfb

if BOXTYPE_DBOX2
AM_CPPFLAGS = -I./rfb @FREETYPE_CFLAGS@ @CONFIGFILE_CFLAGS@ -I$(top_srcdir)/include
vnc_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version @FREETYPE_LIBS@ @CONFIGFILE_LIBS@
else
AM_CPPFLAGS = @FREETYPE_CFLAGS@ -I./rfb  -I$(top_srcdir)/include
vnc_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version @FREETYPE_LIBS@
endif
noinst_LTLIBRARIES = vnc.la

vnc_la_SOURCES = args.c fbdraw.c fscale.c fbgl.c keyboard.c \
                 list.c overlay.c rfbproto.c sockets.c fbvnc.cpp 


vnc_la_LIBADD = rfb/librfb.a

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	$(LIBTOOL) --mode=install install vnc.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/vnc.cfg $(DESTDIR)$(PLUGINDIR)
	if test ! -e  $(DESTDIR)$(CONFIGDIR)/vnc.conf; then cp $(srcdir)/vnc.conf $(DESTDIR)$(CONFIGDIR);fi
