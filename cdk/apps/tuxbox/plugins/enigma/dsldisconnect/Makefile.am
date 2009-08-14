AM_CPPFLAGS = \
	-I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = dsldisconnect.la

dsldisconnect_la_SOURCES = dsldisconnect.cpp

dsldisconnect_la_LDFLAGS = -rpath $(PLUGINDIR) -module

dsldisconnect_la_CXXFLAGS = $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) -fno-exceptions -fno-rtti

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/dsldisconnect.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 dsldisconnect.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/dsldisconnect.so
	-rm $(DESTDIR)$(PLUGINDIR)/dsldisconnect.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
