bin_PROGRAMS = epgidx

epgidx_CPPFLAGS = -fno-exceptions -fno-check-new -fno-rtti

epgidx_SOURCES = epgidx.cpp

INCLUDES = -I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src

noinst_LTLIBRARIES = extepg.la

extepg_la_SOURCES = 	mv.cpp util.cpp somain.cpp program.cpp channel.cpp timebar.cpp enigmacache.cpp extinfo.cpp detailcache.cpp \
			text.cpp inputmanager.cpp channelmanager.cpp files.cpp  downloadcache.cpp downloader.cpp inputdefreader.cpp \
			daybar.cpp timepicker.cpp enigmautil.cpp xmltv.cpp filemap.cpp imdb.cpp imdblistentry.cpp \
			inputmenu.cpp presentationmenu.cpp featuremenu.cpp miscmenu.cpp favmanager.cpp favedit.cpp favmanentry.cpp \
			programlist.cpp proglistentry.cpp menu.cpp chanpicker.cpp  

extepg_la_LDFLAGS = -rpath $(PLUGINDIR) -module
extepg_la_CPPFLAGS = -DENABLE_PRIVATE_EPG -DENABLE_MHW_EPG -DENABLE_DISH_EPG
extepg_la_CXXFLAGS =

if BOXTYPE_DM7020
extepg_la_CXXFLAGS += -DDM7020
endif

if BOXTYPE_DM7000
extepg_la_CXXFLAGS += -DDM7000
endif

if BOXTYPE_DM56x0
extepg_la_CXXFLAGS += -DDM5620
endif

if BOXTYPE_DM500
extepg_la_CXXFLAGS += -DDM500
endif

extepg_la_CXXFLAGS += -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install -d $(DESTDIR)$(CONFIGDIR)
	install -d $(DESTDIR)$(CONFIGDIR)/mv
	install .libs/extepg.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644 extepg.cfg $(DESTDIR)$(PLUGINDIR)
	install -m 0644 config/* $(DESTDIR)$(CONFIGDIR)/mv

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/extepg.so
	-rm $(DESTDIR)$(PLUGINDIR)/extepg.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)
