SUBDIRS = driver

INCLUDES = \
	@ZAPIT_CFLAGS@ \
	-I$(top_srcdir)/lib \
	@EVENTSERVER_CFLAGS@

bin_PROGRAMS = controld

controld_LDADD = \
	driver/libcontrold_driver.a \
	$(top_builddir)/lib/lcddclient/liblcddclient.la \
	$(top_builddir)/lib/timerdclient/libtimerdclient.la \
	@CONFIGFILE_LIBS@ \
	@EVENTSERVER_LIBS@ \
	@ZAPIT_LIBS@ \
	-lpthread

controld_SOURCES = controld.cpp eventwatchdog.cpp

install-data-local:
	install -d $(CONFIGDIR)
	install -m 644 $(srcdir)/scart.conf $(CONFIGDIR)

