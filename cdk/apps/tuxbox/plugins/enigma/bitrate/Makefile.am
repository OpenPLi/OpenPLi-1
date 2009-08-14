AM_CPPFLAGS = \
        -I$(srcdir)/../lib \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/../enigma/include \
        -I$(top_srcdir)/../enigma \
        -I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = bitrate.la

bitrate_la_SOURCES = brdlg.cpp main.cpp utils.cpp

bitrate_la_LDFLAGS = -rpath $(PLUGINDIR) -module

bitrate_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)/bitrate
	install .libs/bitrate.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 bitrate.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/bitrate.so
	-rm $(DESTDIR)$(PLUGINDIR)/bitrate.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
	-rmdir $(DESTDIR)$(CONFIGDIR)/bitrate
