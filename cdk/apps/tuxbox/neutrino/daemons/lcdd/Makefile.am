SUBDIRS = clock icons

INCLUDES = \
	-I$(top_srcdir)/lib \
	@CONFIGFILE_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@LCDDISPLAY_CFLAGS@ \
	-I@DRIVER@/lcd

bin_PROGRAMS = lcdd

lcdd_SOURCES = lcdd.cpp lcdpainter.cpp bigclock.cpp newclock.cpp

lcdd_LDADD = \
	@CONFIGFILE_LIBS@ \
	@FREETYPE_LIBS@ \
	@LCDDISPLAY_LIBS@ \
	-lpthread
