AM_CPPFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/include

noinst_LTLIBRARIES = lemmings.la

lemmings_la_SOURCES = somain.c sprite.c lemm.c pic.c sound.c double.c

lemmings_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version -lz

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	$(LIBTOOL) --mode=install install lemmings.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/lemmings.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/lemmings.so
	-rm $(DESTDIR)$(PLUGINDIR)/lemmings.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
