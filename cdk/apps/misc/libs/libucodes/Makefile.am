INCLUDES = \
        -I$(top_srcdir)/libmd5sum

lib_LTLIBRARIES = libucodes.la

tuxboxincludedir = ${includedir}/tuxbox
tuxboxinclude_HEADERS = libucodes.h

libucodes_la_SOURCES = libucodes.c
libucodes_la_LIBADD = $(top_builddir)/libmd5sum/libmd5sum.la

