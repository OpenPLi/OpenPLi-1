
AM_CPPFLAGS =  \
	@FREETYPE_CFLAGS@ \
	-I$(top_srcdir)/include
	

noinst_LTLIBRARIES = pip.la


pip_la_SOURCES = pip.cpp decode.c header.c idct.c cpu_state.c cpu_accel.c slice.c motion_comp.c alloc.c

pip_la_LIBADD = \
	 @FREETYPE_LIBS@
	 

pip_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version -O1 -Wl,--as-needed -O3 

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	$(LIBTOOL) --mode=install install pip.la $(DESTDIR)$(PLUGINDIR) 
	install -m 0644 $(srcdir)/pip.cfg $(DESTDIR)$(PLUGINDIR)
