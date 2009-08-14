AM_CPPFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/include

noinst_LTLIBRARIES = vierg.la

vierg_la_SOURCES = board.c somain.c

vierg_la_LIBADD = ../lib/libfx2.la

vierg_la_LDFLAGS = -rpath $(PLUGINDIR) -R$(TARGET_PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	$(LIBTOOL) --mode=install install vierg.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/vierg.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/vierg.so
	-rm $(DESTDIR)$(PLUGINDIR)/vierg.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
