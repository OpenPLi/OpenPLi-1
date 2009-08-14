SUBDIRS = gui daemonc driver system

INCLUDES = \
	-I$(top_srcdir)/daemons \
	-I$(top_srcdir)/lib \
        @CONNECTION_CFLAGS@ \
	@EVENTSERVER_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@NET_CFLAGS@ \
	@ZAPIT_CFLAGS@

bin_PROGRAMS = neutrino

neutrino_SOURCES = neutrino.cpp

neutrino_LDADD = \
	daemonc/libneutrino_daemonc.a \
	driver/libneutrino_driver.a \
	gui/bedit/libneutrino_gui_bedit.a \
	gui/widget/libneutrino_gui_widget.a \
	gui/libtimerlist.a \
	gui/libneutrino_gui.a \
	gui/widget/libneutrino_gui_widget2.a \
	gui/widget/libneutrino_gui_widget_basic.a \
	system/libneutrino_system.a \
	gui/libneutrino_gui2.a \
	$(top_builddir)/lib/controldclient/libcontroldclient.la \
	$(top_builddir)/lib/lcddclient/liblcddclient.la \
	$(top_builddir)/lib/sectionsdclient/libsectionsdclient.la \
	$(top_builddir)/lib/timerdclient/libtimerdclient.la \
	@CONFIGFILE_LIBS@ \
        @CONNECTION_LIBS@ \
	@CRAMFS_LIBS@ \
	@CURL_LIBS@ \
	@FREETYPE_LIBS@ \
	@MD5SUM_LIBS@ \
	@NET_LIBS@ \
	@UCODES_LIBS@ \
	@ZAPIT_LIBS@ \
	-lpthread -lz

