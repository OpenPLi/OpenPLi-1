AM_CPPFLAGS = -I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include 

noinst_LTLIBRARIES = enigma_demo.la

enigma_demo_la_SOURCES = demo.cpp

enigma_demo_la_LDFLAGS = -rpath $(PLUGINDIR) -module

enigma_demo_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) 

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/enigma_demo.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644  enigma_demo.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/enigma_demo.so
	-rm $(DESTDIR)$(PLUGINDIR)/enigma_demo.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
