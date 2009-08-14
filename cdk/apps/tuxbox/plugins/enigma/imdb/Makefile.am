AM_CPPFLAGS = -I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include 

noinst_LTLIBRARIES = imdb.la

imdb_la_SOURCES = imdb.cpp

imdb_la_LDFLAGS = -rpath $(PLUGINDIR) -module

imdb_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/imdb.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 imdb.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/imdb.so
	-rm $(DESTDIR)$(PLUGINDIR)/imdb.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
