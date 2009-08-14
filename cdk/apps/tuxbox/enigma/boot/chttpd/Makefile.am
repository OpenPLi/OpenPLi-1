INCLUDES = \
	-I$(top_srcdir)/include

bin_PROGRAMS = chttpd

chttpd_SOURCES = \
	webserver.cpp chttpd.cpp helper.cpp request.cpp debug.cpp

chttpd_LDADD = \
	@CONFIGFILE_LIBS@ \
	-lpthread
