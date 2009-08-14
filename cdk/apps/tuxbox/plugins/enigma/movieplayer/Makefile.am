AM_CPPFLAGS = \
	-I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = movieplayer.la

movieplayer_la_SOURCES = movieplayer.cpp

movieplayer_la_LDFLAGS = -rpath $(PLUGINDIR) @CURL_LIBS@ -module

movieplayer_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/movieplayer.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 movieplayer.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/movieplayer.so
	-rm $(DESTDIR)$(PLUGINDIR)/movieplayer.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)

