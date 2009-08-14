INCLUDES = \
	-I$(top_srcdir)/lib \
	-I$(top_srcdir)/src \
	@EVENTSERVER_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@NET_CFLAGS@ \
	@ZAPIT_CFLAGS@

noinst_LIBRARIES = libneutrino_system.a

libneutrino_system_a_SOURCES = \
	localize.cpp setting_helpers.cpp debug.cpp \
	actionlog.cpp ping.c flashtool.cpp httptool.cpp \
	settings.cpp lastchannel.cpp 
