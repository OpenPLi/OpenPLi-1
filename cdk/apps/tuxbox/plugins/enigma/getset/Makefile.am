AM_CPPFLAGS = -I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = enigma_getset.la

enigma_getset_la_SOURCES = getset.cpp

enigma_getset_la_LDFLAGS = -rpath $(PLUGINDIR) -module

enigma_getset_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) 

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/enigma_getset.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644  enigma_getset.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/enigma_getset.so
	-rm $(DESTDIR)$(PLUGINDIR)/enigma_getset.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
