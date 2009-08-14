lib_LTLIBRARIES = libflashimage.la

libflashimage_la_SOURCES = \
	flashimage.cpp \
	flashimagecramfs.cpp

libflashimage_la_LIBADD = $(top_builddir)/libcrypto++/libcrypto++.la -lz

AM_CXXFLAGS = -fno-rtti

$(top_builddir)/libcrypto++/libcrypto++.la:
	$(MAKE) -C $(top_builddir)/libcrypto++

INCLUDES = -I$(top_builddir)/../apps -I$(top_builddir)/../apps/libflashimage

all-local:
	@ln -fs $(top_builddir)/../apps/libflashimage/*.hpp .
	@ln -fs $(top_builddir)/../apps/libflashimage/*.ipp .

mostlyclean-local:
	@rm -r *.cpp *.hpp *.ipp

$(libflashimage_la_SOURCES):
	@ln -s $(top_builddir)/../apps/libflashimage/$@
