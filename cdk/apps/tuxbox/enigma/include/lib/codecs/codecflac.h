#ifndef DISABLE_FILE

#ifndef __lib_codecs_codecflac_h
#define __lib_codecs_codecflac_h

#include <FLAC/stream_decoder.h>
#include <lib/codecs/codec.h>

struct flac_data {
	eIOBuffer *in;
	eIOBuffer *out;
	int channels;
	int bitrate;
	int samplerate;
	int written;
	int bps; // bits per sample
	size_t total_read;
	size_t total_write;
	size_t last_read_size;
	size_t last_write_size;
};
		
class eAudioDecoderFLAC: public eAudioDecoder
{
	enum { INPUT_BUFFER_SIZE=64*1024 };
	
	FLAC__StreamDecoder * fl;
	int framecnt;
	struct flac_data my_data;
	eIOBuffer &input, &output;
	static FLAC__StreamDecoderReadStatus eAudioDecoderFLAC::my_flac_read(const FLAC__StreamDecoder *fl, FLAC__byte buffer[], unsigned *bytes, struct flac_data *client_data);
	static FLAC__StreamDecoderWriteStatus eAudioDecoderFLAC::my_flac_write(const FLAC__StreamDecoder *fl, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], struct flac_data *client_data);
	static void eAudioDecoderFLAC::my_flac_meta(const FLAC__StreamDecoder *fl, const FLAC__StreamMetadata *metadata, struct flac_data *client_data);
	static void eAudioDecoderFLAC::my_flac_error (const FLAC__StreamDecoder *fl, FLAC__StreamDecoderErrorStatus status, struct flac_data *client_data);

public:
	eAudioDecoderFLAC(eIOBuffer &input, eIOBuffer &output);
	~eAudioDecoderFLAC();
	
	void resync();
	int getMinimumFramelength();
	int decodeMore(int last, int maxsamples, Signal1<void, unsigned int>*cb=0);
	int getAverageBitrate();
	eString getAudioType();
};

#endif

#endif //DISABLE_FILE
