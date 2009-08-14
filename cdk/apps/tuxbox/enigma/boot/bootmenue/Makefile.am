SUBDIRS = skins

AM_CPPFLAGS = \
	@FREETYPE_CFLAGS@ \
	-I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

bin_PROGRAMS = bootmenue

bootmenue_SOURCES = my_fb.cpp my_rc.cpp my_lcd.cpp png.cpp my_timer.cpp bootmenue.cpp

bootmenue_LDADD = \
	@SIGC_LIBS@ \
	@PNG_LIBS@ \
	@FREETYPE_LIBS@
