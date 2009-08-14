if BOXTYPE_DBOX2
# mpegtools are only used on DVB_API > 1 => dbox2
INCLUDES = @MPEGTOOLS_CFLAGS@
sbin_PROGRAMS = streampes streamts streamsec udpstreamts streamfile fserver zapstream
streamts_LDADD = @MPEGTOOLS_LIBS@
streamfile_LDADD = @MPEGTOOLS_LIBS@ -lpthread
streamfile_SOURCES = streamfile.c ringbuffer.c
fserver_SOURCES = fserver.c
else
sbin_PROGRAMS = streampes streamts streamsec udpstreamts udpstreampes zapstream
udpstreampes_LDADD = -lpthread
udpstreampes_SOURCES = udpstreampes.cpp
endif
streampes_SOURCES = streampes.c
streamts_SOURCES = streamts.c
zapstream_SOURCES = zapstream.c
streamsec_SOURCES = streamsec.c
udpstreamts_SOURCES = udpstreamts.c
