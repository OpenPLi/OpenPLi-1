lib_LTLIBRARIES = libavs.la

libavs_la_SOURCES = libavs.c
libavs_la_LDFLAGS = -version-info 0:0:0

bin_PROGRAMS = testavs
testavs_SOURCES = testavs.c
testavs_LDADD = libavs.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = tuxbox-avs.pc

