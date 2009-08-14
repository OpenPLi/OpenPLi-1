AM_CPPFLAGS = -DAS_PLUGIN -I$(top_srcdir)/include
dvbsub_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version

# @PTHREAD_LIBS@

noinst_LTLIBRARIES = dvbsub.la

dvbsub_la_SOURCES = dvbsub.cpp reader_thread.cpp dvbsub_thread.cpp dvbsubtitle.cpp \
		    tools.cpp osd.cpp PacketQueue.cpp helpers.cpp Debug.cpp

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	$(LIBTOOL) --mode=install install dvbsub.la $(DESTDIR)$(PLUGINDIR)
	install -m 0644 $(srcdir)/dvbsub.cfg $(DESTDIR)$(PLUGINDIR)
