AM_CPPFLAGS = -Ienigma

noinst_LTLIBRARIES = ipkgpl.la

ipkgpl_la_SOURCES = ipkg.cpp ipkgdownload.cpp ipkgsetup.cpp ipkgmanager.cpp

ipkgpl_la_LDFLAGS = -rpath $(PLUGINDIR) -module

ipkgpl_la_CXXFLAGS = -fno-rtti -fno-exceptions $(SIGC_CFLAGS) $(XMLTREE_CFLAGS)
