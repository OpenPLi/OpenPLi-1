INCLUDES = \
	@CONFIGFILE_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@LCDDISPLAY_CFLAGS@

bin_PROGRAMS = lcdmenu

lcdmenu_SOURCES = lcdmenu.cpp main.cpp rcinput.cpp

lcdmenu_LDADD = \
	@CONFIGFILE_LIBS@ \
	@FREETYPE_LIBS@ \
	@LCDDISPLAY_LIBS@ \
	-lcrypt

install-data-local:
	install -d $(CONFIGDIR)
	test -e $(CONFIGDIR)/lcdmenu.conf || install -m 644 lcdmenu.conf.example $(CONFIGDIR)/lcdmenu.conf

