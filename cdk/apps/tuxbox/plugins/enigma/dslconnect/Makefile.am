AM_CPPFLAGS = \
	-I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = dslconnect.la

dslconnect_la_SOURCES = dslconnect.cpp

dslconnect_la_LDFLAGS = -rpath $(PLUGINDIR) -module

dslconnect_la_CXXFLAGS = $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) -fno-exceptions -fno-rtti

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/dslconnect.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 dslconnect.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/dslconnect.so
	-rm $(DESTDIR)$(PLUGINDIR)/dslconnect.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
