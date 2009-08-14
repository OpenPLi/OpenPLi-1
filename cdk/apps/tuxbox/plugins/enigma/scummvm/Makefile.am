AM_CPPFLAGS = -I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include 

noinst_LTLIBRARIES = scummvm.la

scummvm_la_SOURCES = scummvm.cpp

scummvm_la_LDFLAGS = -rpath $(PLUGINDIR) -module

scummvm_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) 

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/scummvm.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644  scummvm.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/scummvm.so
	-rm $(DESTDIR)$(PLUGINDIR)/scummvm.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
