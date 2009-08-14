SUBDIRS = lowlevel

INCLUDES = \
	-I$(top_srcdir)/include -I$(top_srcdir)/src

noinst_LIBRARIES = libenigma_dvb.a

libenigma_dvb_a_SOURCES = \
	decoder.cpp dvb.cpp edvb.cpp epgcache.cpp epgstore.cpp eaudio.cpp \
	esection.cpp frontend.cpp si.cpp record.cpp dvbwidgets.cpp dvbservice.cpp \
	eventdata.cpp settings.cpp dvbscan.cpp service.cpp servicecache.cpp servicedvb.cpp \
	servicestructure.cpp cahandler.cpp servicefile.cpp servicemp3.cpp \
	serviceplaylist.cpp dvbci.cpp subtitling.cpp subtitle.cpp \
	servicejpg.cpp serviceexternal.cpp timestampts.cpp
