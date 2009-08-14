bin_PROGRAMS = db_epg
db_epg_SOURCES = crtepgdata.cpp cdbiface.cpp si.cpp si.h epgcache.cpp \
	epgcache.h cmain.cpp crtreader.cpp readerfactory.cpp optionparser.cpp \
	ccmdlinereader.cpp cepgdatreader.cpp event.cpp util.cpp iepgreader.cpp db_epg.cpp \
	xmlreader.cpp cgenremap.cpp cchannelmap.cpp xmlparser.cpp estring.cpp

db_epg_LDADD = -lsqlite3 $(XMLTREE_LIBS)
