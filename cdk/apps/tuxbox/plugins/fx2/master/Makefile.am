AM_CPPFLAGS = -I$(srcdir)/../lib -I$(top_srcdir)/include

noinst_LTLIBRARIES = master.la

master_la_SOURCES = board.c somain.c

master_la_LIBADD = ../lib/libfx2.la

master_la_LDFLAGS = -rpath $(PLUGINDIR) -R$(TARGET_PLUGINDIR) -module -avoid-version

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	$(LIBTOOL) --mode=install install master.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/master.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/master.so
	-rm $(DESTDIR)$(PLUGINDIR)/master.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
