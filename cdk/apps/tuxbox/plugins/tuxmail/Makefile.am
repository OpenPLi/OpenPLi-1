SUBDIRS = daemon

AM_CPPFLAGS = \
	@FREETYPE_CFLAGS@ \
	-I$(top_srcdir)/include

noinst_LTLIBRARIES = tuxmail.la

tuxmail_la_SOURCES = tuxmail.c

tuxmail_la_LIBADD = \
	@FREETYPE_LIBS@

tuxmail_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	install -d $(DESTDIR)$(CONFIGDIR)/tuxmail
	$(LIBTOOL) --mode=install install tuxmail.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/tuxmail.cfg $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/tuxmail.conf $(DESTDIR)$(CONFIGDIR)/tuxmail
