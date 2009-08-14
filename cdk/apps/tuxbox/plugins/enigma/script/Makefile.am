AM_CPPFLAGS = \
        -I$(srcdir)/../lib \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/../enigma/include \
        -I$(top_srcdir)/../enigma \
        -I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = script.la

script_la_SOURCES = script.cpp

script_la_LDFLAGS = -rpath $(PLUGINDIR) -module

script_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/script.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 script.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/script.so
	-rm $(DESTDIR)$(PLUGINDIR)/script.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)

