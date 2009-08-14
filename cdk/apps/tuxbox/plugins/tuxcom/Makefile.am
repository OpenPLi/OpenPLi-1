
AM_CPPFLAGS =  \
	@FREETYPE_CFLAGS@ \
	-I$(top_srcdir)/include

noinst_LTLIBRARIES = tuxcom.la

#tuxcom_la_CFLAGS = -DDREAMBOX 

tuxcom_la_SOURCES = tuxcom.c

tuxcom_la_LIBADD = \
	 @FREETYPE_LIBS@
	 

tuxcom_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version 

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	install -d $(DESTDIR)$(CONFIGDIR)/tuxcom
	$(LIBTOOL) --mode=install install tuxcom.la $(DESTDIR)$(PLUGINDIR) 
	install -m 0644 $(srcdir)/tuxcom.cfg $(DESTDIR)$(PLUGINDIR)
	install -m 0755 $(srcdir)/mp3 $(DESTDIR)$(CONFIGDIR)/tuxcom
