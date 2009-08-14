INCLUDES = \
	@CURL_CFLAGS@ \
	@FREETYPE_CFLAGS@ \
	@PLUGINS_CFLAGS@ \
	@TUXBOX_CFLAGS@ \
	@CONFIGFILE_CFLAGS@ \
	@CONNECTION_CFLAGS@ \
	@LCDDISPLAY_CFLAGS@ \
	@CRAMFS_CFLAGS@

bin_PROGRAMS = lcars

lcars_SOURCES = \
	main.cpp timer.cpp pig.cpp plugins.cpp \
	hardware.cpp scan.cpp network.cpp teletext.cpp \
	osd.cpp sdt.cpp zap.cpp nit.cpp pat.cpp pmt.cpp \
	eit.cpp settings.cpp channels.cpp rc.cpp help.cpp fbClass.cpp \
	tot.cpp checker.cpp tuner.cpp update.cpp control.cpp variables.cpp \
	ir.cpp cam.cpp lcd.cpp tdt.cpp container.cpp serial.cpp showview.cpp xmlrpc.cpp

lcars_CFLAGS = -Werror
	-DHAVE_DREAMBOX_HARDWARE

lcars_LDADD = \
	@CURL_LIBS@ \
	@FREETYPE_LIBS@ \
	@TUXBOX_LIBS@ \
	@CONNECTION_LIBS@ \
	@CONFIGFILE_LIBS@ \
	@CRAMFS_LIBS@ \
	@UCODES_LIBS@ \
	@LCDDISPLAY_LIBS@ \
	@CRAMFS_LIBS@ \
	 -ldl -lpthread -lcrypt -lucodes

install-data-local:
	install -d $(CONFIGDIR)/lcars

