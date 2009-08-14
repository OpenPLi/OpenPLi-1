AM_CPPFLAGS = \
	-I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

bin_PROGRAMS = setupnetwork

setupnetwork_SOURCES = setupnetwork.cpp econfig.cpp enigma_mount.cpp estring.cpp

setupnetwork_CXXFLAGS = $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(CONFIGFILE_CFLAGS)

setupnetwork_LDADD = @SIGC_LIBS@ @XMLTREE_LIBS@ @CONFIGFILE_LIBS@
