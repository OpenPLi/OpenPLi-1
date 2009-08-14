INCLUDES = \
	-I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = dreamnetcast.la

dreamnetcast_la_SOURCES = dreamnetcast.cpp filemgr.cpp

dreamnetcast_la_LDFLAGS = -rpath $(PLUGINDIR) -module

dreamnetcast_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/dreamnetcast.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 dreamnetcast.cfg $(DESTDIR)$(PLUGINDIR)
	install -m 0644 stations.xml $(DESTDIR)$(CONFIGDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/dreamnetcast.so
	-rm $(DESTDIR)$(PLUGINDIR)/dreamnetcast.cfg
	-rm $(DESTDIR)$(CONFIGDIR)/stations.xml
	-rmdir $(PLUGINDIR)
