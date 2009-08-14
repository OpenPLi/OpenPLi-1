INCLUDES = \
	-I$(top_srcdir)/daemons \
	-I$(top_srcdir)/src \
	@EVENTSERVER_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@NET_CFLAGS@ \
	@ZAPIT_CFLAGS@ \
	-I$(top_srcdir)/lib

noinst_LIBRARIES = libneutrino_driver.a

libneutrino_driver_a_SOURCES = fontrenderer.cpp framebuffer.cpp rcinput.cpp vcrcontrol.cpp

