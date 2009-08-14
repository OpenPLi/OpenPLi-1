AM_CPPFLAGS = \
        -I$(srcdir)/../lib \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/../enigma/include \
        -I$(top_srcdir)/../enigma \
        -I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = rss.la

rss_la_SOURCES = rss.cpp

rss_la_LDFLAGS = -rpath $(PLUGINDIR) -module

rss_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	install .libs/rss.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 rss.cfg $(DESTDIR)$(PLUGINDIR)
	install -m 0644 feeds.xml $(DESTDIR)$(CONFIGDIR)/feeds.xml.default

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/rss.so
	-rm $(DESTDIR)$(PLUGINDIR)/rss.cfg
	-rm $(DESTDIR)$(CONFIGDIR)/feeds.xml.default
	-rmdir $(DESTDIR)$(PLUGINDIR)

