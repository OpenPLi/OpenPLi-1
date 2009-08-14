AM_CPPFLAGS = -I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = ipdyn.la

ipdyn_la_SOURCES = ipdyn.cpp 
                       
ipdyn_la_LDFLAGS = -rpath $(PLUGINDIR) -module

ipdyn_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) 

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/ipdyn.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 ipdyn.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/ipdyn.so
	-rm $(DESTDIR)$(PLUGINDIR)/ipdyn.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
