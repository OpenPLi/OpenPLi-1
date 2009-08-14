AM_CPPFLAGS = \
        -I$(srcdir)/../lib \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/../enigma/include \
        -I$(top_srcdir)/../enigma \
        -I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = dbswitch.la

dbswitch_la_SOURCES = lcddisplay.cpp rcinput.cpp dbswitch.cpp

dbswitch_la_LDFLAGS = -rpath $(PLUGINDIR) -module

dbswitch_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/dbswitch.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 dbswitch.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/dbswitch.so
	-rm $(DESTDIR)$(PLUGINDIR)/dbswitch.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)



