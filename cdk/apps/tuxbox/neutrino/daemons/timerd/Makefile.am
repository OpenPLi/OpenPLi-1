INCLUDES = \
	-I$(top_srcdir)/lib \
	@CONNECTION_CFLAGS@ \
	@EVENTSERVER_CFLAGS@ \
	@ZAPIT_CFLAGS@

bin_PROGRAMS = timerd

timerd_SOURCES = timerd.cpp timermanager.cpp

timerd_LDADD = \
	$(top_builddir)/lib/sectionsdclient/libsectionsdclient.la \
	@CONFIGFILE_LIBS@ \
	@CONNECTION_LIBS@ \
	@EVENTSERVER_LIBS@ \
	@ZAPIT_LIBS@ \
	-lpthread

