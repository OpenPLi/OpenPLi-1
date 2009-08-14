#ifndef DISABLE_FILE

#include <lib/base/eerror.h>
#include <lib/base/buffer.h>
#include <linux/soundcard.h>
#include <lib/codecs/codecogg.h>

size_t eAudioDecoderOGG::my_ogg_read(void *buf, size_t size, size_t count, void *fp)
{
	eIOBuffer *input = (eIOBuffer *)fp;
	if (!input) return -1;

	// eDebug("OGG: Getting data wanted=%d", size * count);
	// long read_size = input->size();
	// eDebug("OGG: Getting data available=%d", read_size);
	return input->read(buf, size * count);
}

int eAudioDecoderOGG::my_ogg_seek(void *fp, ogg_int64_t offset, int whence)
{
	// eDebug("OGG: seek offset=%ld whence=%d", offset, whence);
	return -1;
}

int eAudioDecoderOGG::my_ogg_close(void *fp)
{
	// eDebug("OGG: close");
	return 0;
}

long eAudioDecoderOGG::my_ogg_tell(void *fp)
{
	// eDebug("OGG: tell");
	return -1;
}


int OGG_init;
eAudioDecoderOGG::eAudioDecoderOGG(eIOBuffer &input, eIOBuffer &output): 
		framecnt(0), input(input), output(output)
{
	// eDebug("OGG: Constructor");
        vf = &my_vf;
        ibytes = 0;
        bitstream = &stream_number;

	initial = NULL;
	ibytes = 0;
	OGG_init = 0;
	// eDebug("OGG: setup done");
}

eAudioDecoderOGG::~eAudioDecoderOGG()
{
	// eDebug("OGG: destructor cleanup");
	ov_clear(vf);
	// eDebug("OGG: destructor done");
}

eString eAudioDecoderOGG::getAudioType()
{
	return vi ? eString().sprintf("OGG %d %s", vi->version, vi->channels > 1 ? "Stereo": "Mono") : "OGG";
}

int eAudioDecoderOGG::decodeMore(int last, int maxsamples, Signal1<void, unsigned int> *)
{
	int written = 0;
	// eDebug("OGG: decodeMore last=%d maxsamples=%d", last, maxsamples);
	if (!OGG_init)
	{ 
		ov_callbacks callbacks = {
			(size_t (*)(void *, size_t, size_t, void *))	my_ogg_read,
			(int (*)(void *, ogg_int64_t, int))		my_ogg_seek,
			(int (*)(void *))				my_ogg_close,
			(long (*)(void *))				my_ogg_tell
		};
		// eDebug("OGG: setting callbacks");
		ov_open_callbacks(&input, vf, initial, ibytes, callbacks);
		// eDebug("OGG: getting info");
		vorbis_comment *vc = ov_comment(vf, -1);
		eDebug("OGG: vorbis vendor: %s", vc->vendor);
		for (int i = 0; i < vc->comments; i++)
			eDebug("OGG: vorbis comment %d: %s", i, vc->user_comments[i]);
		OGG_init = 1;
	}
	while (last || (written < maxsamples))
	{
		const int OUTPUT_BUFFER_SIZE = 2048*2;  // 1152*2;
		long read_size;
		char outbuffer[OUTPUT_BUFFER_SIZE];
		
		read_size = input.size();
		if (read_size == 0)
			break;
		// eDebug("OGG: decodeMore bytes available=%ld", read_size);

		read_size = ov_read(vf, outbuffer, OUTPUT_BUFFER_SIZE, bitstream);

		// eDebug("OGG: decodeMore bytes decode=%ld", read_size);
		if (read_size < 0) 
		{
			if (read_size == OV_HOLE)
			{
				eDebug("Ogg: OV_HOLE");
			}
			else if (read_size == OV_EBADLINK)
			{
				eDebug("Ogg: OV_EBADLINK");
			}
			continue;
		}
		// we have sound...

		// eDebug("OGG: getting info");
		vi = ov_info(vf, -1);

		pcmsettings.samplerate = vi->rate;
		pcmsettings.channels = vi->channels;
		pcmsettings.format = AFMT_S16_BE;
		pcmsettings.reconfigure = 0;

		//eDebug("OGG: outputting sr=%d, channels=%d br=%d", vi->rate, vi->channels, vi->bitrate_nominal);
		output.write(outbuffer, read_size);
		written += read_size;
	}
	return written;
}

int eAudioDecoderOGG::getMinimumFramelength()
{
	// eDebug("OGG: getMinimumFramelength");
	return INPUT_BUFFER_SIZE;
}

void eAudioDecoderOGG::resync()
{
	// eDebug("OGG: resync");
}

int eAudioDecoderOGG::getAverageBitrate()
{
	int br = ov_bitrate(vf, -1);
	// eDebug("OGG: getAverageBitrate = %d", br);
	return br;
}

#endif //DISABLE_FILE
