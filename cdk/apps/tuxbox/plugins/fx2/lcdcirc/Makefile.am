AM_CPPFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/include -I$(DRIVER)/include

noinst_LTLIBRARIES = lcdcirc.la

lcdcirc_la_SOURCES = lcdcirc.c

lcdcirc_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	$(LIBTOOL) --mode=install install lcdcirc.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/lcdcirc.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/lcdcirc.so
	-rm $(DESTDIR)$(PLUGINDIR)/lcdcirc.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
