INCLUDES = -I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = dreamflash.la

dreamflash_la_SOURCES = dreamflash.cpp

dreamflash_la_LDFLAGS = -rpath $(PLUGINDIR) -module

dreamflash_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/dreamflash.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644  dreamflash.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/dreamflash.so
	-rm $(DESTDIR)$(PLUGINDIR)/dreamflash.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
