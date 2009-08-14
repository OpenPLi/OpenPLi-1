AM_CPPFLAGS = \
	-I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = ngrabstart.la

ngrabstart_la_SOURCES = ngrabstart.cpp

ngrabstart_la_LDFLAGS = -rpath $(PLUGINDIR) -module -pthread

ngrabstart_la_CXXFLAGS = -fno-exceptions -fno-rtti $(SIGC_CFLAGS) $(XMLTREE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/ngrabstart.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 ngrabstart.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/ngrabstart.so
	-rm $(DESTDIR)$(PLUGINDIR)/ngrabstart.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
