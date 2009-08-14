SUBDIRS = data

INCLUDES = \
	@ZAPIT_CFLAGS@ \
	-I$(top_srcdir)/src \
	@CONNECTION_CFLAGS@ \
	@EVENTSERVER_CFLAGS@ \
	-I$(top_srcdir)/lib


bin_PROGRAMS = nhttpd

nhttpd_SOURCES = \
	webserver.cpp nhttpd.cpp webdbox.cpp helper.cpp \
	request.cpp webapi.cpp controlapi.cpp bouqueteditapi.cpp debug.cpp

nhttpd_LDADD = \
	$(top_builddir)/lib/controldclient/libcontroldclient.la \
	$(top_builddir)/lib/timerdclient/libtimerdclient.la \
	$(top_builddir)/lib/sectionsdclient/libsectionsdclient.la \
	@CONFIGFILE_LIBS@ \
	@CONNECTION_LIBS@ \
	@ZAPIT_LIBS@ \
	-lpthread
