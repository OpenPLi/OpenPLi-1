AM_CPPFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/include

noinst_LTLIBRARIES = bouquet.la

bouquet_la_SOURCES = somain.c

bouquet_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

install-exec-local:

#	install -d $(PLUGINDIR)
#	install .libs/bouquet.so $(PLUGINDIR)
#	install -m 0644  bouquet.cfg $(PLUGINDIR)

uninstall-local:
	-rm $(PLUGINDIR)/bouquet.so
	-rm $(PLUGINDIR)/bouquet.cfg
	-rmdir $(PLUGINDIR)
