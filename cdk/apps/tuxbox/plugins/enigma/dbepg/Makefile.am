AM_CPPFLAGS = \
	-I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = dbepg.la

dbepg_la_SOURCES = channelmanager.cpp dbepg.cpp filemap.cpp retrievalwnd.cpp util.cpp

dbepg_la_LDFLAGS = -rpath $(PLUGINDIR) -module

dbepg_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/dbepg.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 dbepg.cfg $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)/dbepg
	install -m 0644 config/* $(DESTDIR)$(CONFIGDIR)/dbepg
	install -m 0755 scripts/* $(DESTDIR)$(bindir)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/dbepg.so
	-rm $(DESTDIR)$(PLUGINDIR)/dbepg.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
