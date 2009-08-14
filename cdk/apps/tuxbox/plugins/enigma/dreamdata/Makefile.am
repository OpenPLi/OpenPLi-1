AM_CPPFLAGS = \
        -I$(srcdir)/../lib \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/../enigma/include \
        -I$(top_srcdir)/../enigma \
        -I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = dreamdata.la

dreamdata_la_SOURCES = dreamdata.cpp

dreamdata_la_LDFLAGS = -rpath $(PLUGINDIR) -module

dreamdata_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	install .libs/dreamdata.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 dreamdata.cfg $(DESTDIR)$(PLUGINDIR)
	install -m 0644 dreamdata.xml $(DESTDIR)$(CONFIGDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/dreamdata.so
	-rm $(DESTDIR)$(PLUGINDIR)/dreamdata.cfg
	-rm $(DESTDIR)$(CONFIGDIR)/dreamdata.xml
	-rmdir $(DESTDIR)$(PLUGINDIR)
	-rmdir $(DESTDIR)$(CONFIGDIR)

