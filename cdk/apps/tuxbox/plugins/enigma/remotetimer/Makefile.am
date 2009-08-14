AM_CPPFLAGS = \
        -I$(srcdir)/../lib \
        -I$(top_srcdir)/include \
        -I$(top_srcdir)/../enigma/include \
        -I$(top_srcdir)/../enigma \
        -I$(top_srcdir)/../enigma/src \
	-I$(top_srcdir)/include/sigc++-1.2 \
	-I$(top_srcdir)/include/freetype2 \
	-I$(srcdir)/.../lib/sigc++-1.2/include \
	-I$(top_srcdir)/driver/include 

noinst_LTLIBRARIES = RemoteTimer.la

RemoteTimer_la_SOURCES = RemoteTimer.cpp

RemoteTimer_la_LDFLAGS = -rpath $(PLUGINDIR) -module

RemoteTimer_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)
RemoteTimer_la_CXXFLAGS += -DENABLE_PRIVATE_EPG -DENABLE_MHW_EPG -DENABLE_DISH_EPG

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	install .libs/RemoteTimer.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644  RemoteTimer.cfg $(DESTDIR)$(PLUGINDIR)
	install -m 0644  RemoteTimer.conf $(DESTDIR)$(CONFIGDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/RemoteTimer.so
	-rm $(DESTDIR)$(PLUGINDIR)/RemoteTimer.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
	-rm $(DESTDIR)$(CONFIGDIR)/RemoteTimer.conf

