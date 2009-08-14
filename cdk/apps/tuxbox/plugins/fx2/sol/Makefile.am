AM_CPPFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/include

noinst_LTLIBRARIES = sol.la

sol_la_SOURCES = solboard.c somain.c

sol_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	$(LIBTOOL) --mode=install install sol.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/sol.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/sol.so
	-rm $(DESTDIR)$(PLUGINDIR)/sol.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
