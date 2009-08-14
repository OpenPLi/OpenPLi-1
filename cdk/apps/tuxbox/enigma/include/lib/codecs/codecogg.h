#ifndef DISABLE_FILE

#ifndef __lib_codecs_codecogg_h
#define __lib_codecs_codecogg_h

#include <tremor/ivorbisfile.h>
#include <lib/codecs/codec.h>

class eAudioDecoderOGG: public eAudioDecoder
{
	enum { INPUT_BUFFER_SIZE=8192 };
	
        OggVorbis_File my_vf;
        OggVorbis_File *vf;
	vorbis_info *vi;
        char *initial;
        long ibytes;
	int stream_number;
        int *bitstream;

	int framecnt;

	eIOBuffer &input, &output;
	static int my_ogg_close(void *);
	static int my_ogg_seek(void *, ogg_int64_t, int);
	static size_t my_ogg_read(void *buf, size_t size, size_t count, void *fp);
	static long my_ogg_tell(void *);

public:
	eAudioDecoderOGG(eIOBuffer &input, eIOBuffer &output);
	~eAudioDecoderOGG();
	
	void resync();
	int getMinimumFramelength();
	int decodeMore(int last, int maxsamples, Signal1<void, unsigned int>*cb=0);
	int getAverageBitrate();
	eString getAudioType();
};

#endif

#endif //DISABLE_FILE
