AM_CPPFLAGS = -I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = lancontrol.la

lancontrol_la_SOURCES = lancontrol.cpp 
                       
lancontrol_la_LDFLAGS = -rpath $(PLUGINDIR) -module

lancontrol_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) 

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/lancontrol.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 lancontrol.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/lancontrol.so
	-rm $(DESTDIR)$(PLUGINDIR)/lancontrol.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
