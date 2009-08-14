SUBDIRS = zapci zapost zapsi

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib \
	@CONFIGFILE_CFLAGS@ \
	@LCDDCLIENT_CFLAGS@ \
	@XMLTREE_CFLAGS@

bin_PROGRAMS = pzapit zapit

pzapit_SOURCES = \
	pzapit.cpp

pzapit_LDADD = \
	$(top_builddir)/lib/libzapitclient.la

zapit_SOURCES = \
	zapit.cpp basicserver.cpp bouquets.cpp channel.cpp dvbstring.cpp \
	getservices.cpp scan.cpp xmlinterface.cpp $(DBOX2)

zapit_LDADD = \
	@CONFIGFILE_LIBS@ \
	@XMLTREE_LIBS@ \
	$(top_builddir)/lib/libzapitclient.la \
	zapci/libzapci.a \
	zapost/libzapost.a \
	zapsi/libzapsi.a \
	-lpthread

if DBOX2
DBOX2 = lcddclient.cpp
lcddclient.cpp:
	ln -sf @LCDDCLIENT_SOURCE@
endif

AM_CXXFLAGS = -fno-rtti -fno-exceptions

install-data-local:
	$(INSTALL) -d $(CONFIGDIR)/zapit

