INCLUDES = \
	-I$(top_srcdir)/lib \
	@CONNECTION_CFLAGS@ \
	@EVENTSERVER_CFLAGS@

bin_PROGRAMS = bat epg epgMini nit sectionsd sdt

bat_SOURCES = bat.cpp dmxapi.cpp SIsections.cpp SIevents.cpp SIutils.cpp
epg_SOURCES = epg.cpp dmxapi.cpp SIsections.cpp SIevents.cpp SIutils.cpp
epgMini_SOURCES = epgMini.cpp dmxapi.cpp SIsections.cpp SIevents.cpp SIutils.cpp
nit_SOURCES = nit.cpp dmxapi.cpp SIsections.cpp SIevents.cpp SIutils.cpp
sectionsd_SOURCES = sectionsd.cpp dmxapi.cpp debug.cpp dmx.cpp SIsections.cpp SIevents.cpp SIutils.cpp
sdt_SOURCES = sdt.cpp dmxapi.cpp SIsections.cpp SIevents.cpp SIutils.cpp

sectionsd_LDADD = \
	@CONNECTION_LIBS@ \
	@EVENTSERVER_LIBS@ \
	-lpthread

