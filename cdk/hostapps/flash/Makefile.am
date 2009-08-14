bin_PROGRAMS = flashcheck flashsign
bin_SCRIPTS = flashmanage.pl

flashcheck_SOURCES = \
	flashcheck.cpp

flashcheck_LDADD = \
	$(top_builddir)/libflashimage/libflashimage.la

flashsign_SOURCES = \
	flashsign.cpp

flashsign_LDADD = \
	$(top_builddir)/libcrypto++/libcrypto++.la \
	$(top_builddir)/libflashimage/libflashimage.la

AM_CXXFLAGS = -fno-rtti

$(top_builddir)/libcrypto++/libcrypto++.la:
	$(MAKE) -C $(top_builddir)/libcrypto++

$(top_builddir)/libflashimage/libflashimage.la:
	$(MAKE) -C $(top_builddir)/libflashimage

