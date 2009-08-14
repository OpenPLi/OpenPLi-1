AM_CPPFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/include

noinst_LTLIBRARIES = snake.la

snake_la_SOURCES = snake.c somain.c

snake_la_LIBADD = ../lib/libfx2.la

snake_la_LDFLAGS = -rpath $(PLUGINDIR) -R$(TARGET_PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	$(LIBTOOL) --mode=install install snake.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/snake.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/snake.so
	-rm $(DESTDIR)$(PLUGINDIR)/snake.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
