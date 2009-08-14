AM_CPPFLAGS = -I$(DRIVER)/include

noinst_LTLIBRARIES = libfx2.la

libfx2_la_SOURCES = draw.c pig.c rcinput.c math.c

libfx2_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	$(LIBTOOL) --mode=install install libfx2.la $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/libfx2.so
