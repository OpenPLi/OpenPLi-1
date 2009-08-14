lib_LTLIBRARIES = libflashimage.la

libflashimage_la_SOURCES = \
	flashimage.cpp \
	flashimagecramfs.cpp \
	noexcept.cpp

libflashimage_la_LIBADD = $(top_builddir)/libcrypto++/libcrypto++.la -lz

AM_CXXFLAGS = -fno-rtti

$(top_builddir)/libcrypto++/libcrypto++.la:
	$(MAKE) -C $(top_builddir)/libcrypto++

