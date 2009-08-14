AM_CPPFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/include

noinst_LTLIBRARIES = pacman.la

pacman_la_SOURCES = maze.c somain.c

pacman_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	$(LIBTOOL) --mode=install install pacman.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/pacman.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/pacman.so
	-rm $(DESTDIR)$(PLUGINDIR)/pacman.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
