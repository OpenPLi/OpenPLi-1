AM_CPPFLAGS = \
	-I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = ngrabstop.la

ngrabstop_la_SOURCES = ngrabstop.cpp

ngrabstop_la_LDFLAGS = -rpath $(PLUGINDIR) -module -pthread

ngrabstop_la_CXXFLAGS = $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) -fno-exceptions -fno-rtti

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/ngrabstop.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 ngrabstop.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/ngrabstop.so
	-rm $(DESTDIR)$(PLUGINDIR)/ngrabstop.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
