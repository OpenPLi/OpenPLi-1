INCLUDES = -I$(srcdir)/../lib \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../enigma/include \
	-I$(top_srcdir)/../enigma \
	-I$(top_srcdir)/../enigma/src \
	-I$(top_srcdir)/../plugins/enigma/plimgr


noinst_LTLIBRARIES = setup.la

setup_la_SOURCES = setup_main.cpp \
			wifi.cpp setup_firewall.cpp

setup_la_LDFLAGS = -rpath $(PLUGINDIR) -module

setup_la_CXXFLAGS =

if BOXTYPE_DM7020
setup_la_CXXFLAGS += -DBOXTYPE_DM7020 -DMOUNT_UNMOUNT -DSETUP_USB -DNFSSERVER -DAPACHE -DSWAP -DSETUP_EXTRA -DEPG_USB_CF
endif

if BOXTYPE_DM7000
setup_la_CXXFLAGS += -DBOXTYPE_DM7000 -DMOUNT_UNMOUNT -DSETUP_USB -DNFSSERVER -DAPACHE -DSWAP -DMOVE_VAR -DSETUP_DEBUG -DSETUP_EXTRA -DEPG_USB_CF
endif

if BOXTYPE_DM56x0
setup_la_CXXFLAGS += -DBOXTYPE_DM56X0 -DSETUP_CS -DSETUP_DEBUG -DMOVE_VAR -DSETUP_EXTRA
endif

if BOXTYPE_DM600
setup_la_CXXFLAGS += -DBOXTYPE_DM600 -DMOUNT_UNMOUNT -DNFSSERVER -DAPACHE -DSWAP -DSETUP_EXTRA
endif

if BOXTYPE_DM500
setup_la_CXXFLAGS += -DBOXTYPE_DM500 -DSETUP_CS -DSETUP_DEBUG -DMOVE_VAR -DSETUP_EXTRA
endif

setup_la_CXXFLAGS += -DSETUP_EMU -DFIREWALL -DSETUP_SERVICES -DCron \
	-fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS) $(FREETYPE_CFLAGS)

install-exec-local:
	install -d $(DESTDIR)$(PLUGINDIR)
	install .libs/setup.so $(DESTDIR)$(PLUGINDIR)
	install -m 0644  setup.cfg $(DESTDIR)$(PLUGINDIR)

uninstall-local:
	-rm $(DESTDIR)$(PLUGINDIR)/setup.so
	-rm $(DESTDIR)$(PLUGINDIR)/setup.cfg
	-rmdir $(DESTDIR)$(PLUGINDIR)

SUBDIRS = po

##
## on every build, record the working copy revision string
##
svn_revision.cpp: force
	echo -n 'const char* svn_revision(void) { const char* SVN_REVISION = "' > svn_revision.cpp.new
	svnversion -n . >> svn_revision.cpp.new
	echo '"; return SVN_REVISION; }' >> svn_revision.cpp.new
	if ! cmp svn_revision.cpp svn_revision.cpp.new; then cp svn_revision.cpp.new svn_revision.cpp; fi
force: ;

setup_la_SOURCES += svn_revision.cpp
