AM_CPPFLAGS = \
        -I$(srcdir)/../lib \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/../enigma/include \
        -I$(top_srcdir)/../enigma \
        -I$(top_srcdir)/../enigma/src



noinst_LTLIBRARIES = webstream2db.la

webstream2db_la_SOURCES = webstream2db.cpp

webstream2db_la_LDFLAGS = -rpath $(PLUGINDIR) -module

webstream2db_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/webstream2db.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644  webstream2db.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/webstream2db.so
	-rm $(DESTDIR)$(PLUGINDIR)/webstream2db.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
