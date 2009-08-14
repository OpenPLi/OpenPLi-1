SUBDIRS = icons

AM_CPPFLAGS = \
        -I$(srcdir)/../lib \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/../enigma/include \
        -I$(top_srcdir)/../enigma \
        -I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = weather.la

weather_la_SOURCES = weather.cpp

weather_la_LDFLAGS = -rpath $(PLUGINDIR) -module

weather_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	install .libs/weather.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 weather.cfg $(DESTDIR)$(PLUGINDIR)
#	install -m 0644 weather.xml.de $(DESTDIR)$(CONFIGDIR)/weather.xml
#	install -m 0644 weather.xml.nl $(DESTDIR)$(CONFIGDIR)/weather.xml
	install -m 0644 weather.xml $(DESTDIR)$(CONFIGDIR)/weather.xml

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/weather.so
	-rm $(DESTDIR)$(PLUGINDIR)/weather.cfg
	-rm $(DESTDIR)$(CONFIGDIR)/weather.xml
	-rmdir $(DESTDIR)$(PLUGINDIR)
	-rmdir $(DESTDIR)$(CONFIGDIR)

