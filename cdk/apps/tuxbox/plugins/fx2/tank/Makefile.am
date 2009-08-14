AM_CPPFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/include

noinst_LTLIBRARIES = tank.la

tank_la_SOURCES = board.c somain.c

tank_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	$(LIBTOOL) --mode=install install tank.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/tank.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/tank.so
	-rm $(DESTDIR)$(PLUGINDIR)/tank.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
