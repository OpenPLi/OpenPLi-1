SUBDIRS = daemon

AM_CPPFLAGS = \
	@FREETYPE_CFLAGS@ \
	-I$(top_srcdir)/include

noinst_LTLIBRARIES = tuxcal.la

tuxcal_la_SOURCES = tuxcal.c

tuxcal_la_LIBADD = \
	@FREETYPE_LIBS@

tuxcal_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	install -d $(DESTDIR)$(CONFIGDIR)/tuxcal
	$(LIBTOOL) --mode=install install tuxcal.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/tuxcal.cfg $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/tuxcal.conf $(DESTDIR)$(CONFIGDIR)/tuxcal
