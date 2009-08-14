
AM_CPPFLAGS =  \
	@FREETYPE_CFLAGS@ \
	-I$(top_srcdir)/include

noinst_LTLIBRARIES = mosaic.la


mosaic_la_SOURCES = mosaic.cpp decode.c header.c idct.c cpu_state.c cpu_accel.c slice.c motion_comp.c alloc.c rgb.c

mosaic_la_LIBADD = \
	 @FREETYPE_LIBS@
	 

mosaic_la_LDFLAGS = -rpath $(PLUGINDIR) -module -avoid-version 

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	$(LIBTOOL) --mode=install install mosaic.la $(DESTDIR)$(PLUGINDIR) 
	install -m 0644 $(srcdir)/mosaic.cfg $(DESTDIR)$(PLUGINDIR)
