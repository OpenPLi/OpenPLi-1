#ifndef DISABLE_FILE

#include <lib/codecs/codecmp3.h>
#include <lib/base/eerror.h>
#include <lib/base/buffer.h>
#include <linux/soundcard.h>

static inline unsigned short MadFixedToUshort(mad_fixed_t Fixed)
{
	// CLIPPING !!!
	Fixed >>= MAD_F_FRACBITS-14;
	return Fixed;
}


eAudioDecoderMP3::eAudioDecoderMP3(eIOBuffer &input, eIOBuffer &output): 
		avgbr(-1), framecnt(0), input(input), output(output)
{
	init_eAudioDecoderMP3();
}
void eAudioDecoderMP3::init_eAudioDecoderMP3()
{
	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);
	mad_timer_reset(&timer);
}

eAudioDecoderMP3::~eAudioDecoderMP3()
{
	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
}

eString eAudioDecoderMP3::getAudioType()
{
	return eString().sprintf("MP3 L%s %s",
			frame.header.layer == 1 ? "I" : frame.header.layer == 2 ? "II" : "III",
			frame.header.mode == 0 ? "mono" : frame.header.layer == 1 ? "Dual Stereo" :
			frame.header.mode == 2 ? "Jnt Stereo" : "Stereo");
}

int eAudioDecoderMP3::decodeMore(int last, int maxsamples, Signal1<void, unsigned int> *)
{
	int written = 0;
	// eDebug("MP3:decodeMore: last=%d, maxsamples=%d", last, maxsamples);
	while (last || (written < maxsamples))
	{
		const int OUTPUT_BUFFER_SIZE = 1152*2;
		int status = 0;
		
		// eDebug("MP3:decodeMore: next frame");
		if ((!stream.buffer) || (stream.error == MAD_ERROR_BUFLEN))
		{
			size_t read_size, remaining, read_bytes;
			unsigned char *read_start;
			
			read_size = input.size();
			if (read_size == 0)
			{
				// eDebug("MP3:decodeMore: no data available yet");
				break;
			}
			if (stream.next_frame)
			{

				remaining = stream.bufend - stream.next_frame;
				if (input_buffer != stream.next_frame)
					memmove(input_buffer, stream.next_frame, remaining);
				read_start = input_buffer + remaining;
				read_size = INPUT_BUFFER_SIZE - remaining;
			}
			else  
			{
				read_size = INPUT_BUFFER_SIZE;
				read_start = input_buffer;
				remaining = 0;
			}
				
			read_bytes = input.read(read_start, read_size);
                        //eDebug("MP3:decodeMore:  got=%d %02x %02x %02x %02x", read_bytes, input_buffer[0], input_buffer[1], input_buffer[2], input_buffer[3]);

			// eDebug("MP3:decodeMore:  trying to get data(%s), remain=%ld, asking=%ld got=%ld", (stream.error == MAD_ERROR_BUFLEN) ? "MADBUFLEN" : "streambuffer", remaining, read_size, read_bytes);
			
			mad_stream_buffer(&stream, input_buffer, read_bytes + remaining);
			stream.error = (mad_error)0;
		}
		
		if (mad_frame_decode(&frame, &stream))
		{
			if (MAD_RECOVERABLE(stream.error))
			{
				eWarning("mp3: recoverable frame level error (%s)", mad_stream_errorstr(&stream));
				continue;
			}
			else
			{
				if (stream.error == MAD_ERROR_BUFLEN)
				{
//					eDebug("MAD_ERROR_BUFLEN");
					continue; // lets see if addig more data to the buffer helps
				}
				else
				{
					eWarning("mp3: unrecoverable frame level error (%s)", mad_stream_errorstr(&stream));
					status = 2;
					break;
				}
			}
		}
		
		mad_timer_add(&timer, frame.header.duration);

		avgbr = (avgbr == -1)  ? frame.header.bitrate : (avgbr*255+frame.header.bitrate)>>8;

		if (++framecnt < speed)
		{
			eDebug("SPEED");
			continue;
		}
		framecnt=0;

		mad_synth_frame(&synth, &frame);

		pcmsettings.samplerate = synth.pcm.samplerate;
		pcmsettings.channels = synth.pcm.channels;
		pcmsettings.format = AFMT_S16_BE;
		pcmsettings.reconfigure = 0;

		unsigned short outbuffer[OUTPUT_BUFFER_SIZE];
		int ptr = 0;
		int len = synth.pcm.length;
		int stereo = MAD_NCHANNELS(&frame.header)==2;
		
		/* endianess kaputt !!! */
		while (len)
		{
			int tw = len;
			if (stereo)
			{
				if (tw > OUTPUT_BUFFER_SIZE/2)
					tw=OUTPUT_BUFFER_SIZE/2;
				for (int i = 0; i < tw; i++)
				{
					outbuffer[i*2] = MadFixedToUshort(synth.pcm.samples[0][ptr]);
					outbuffer[i*2+1] = MadFixedToUshort(synth.pcm.samples[1][ptr]);
					ptr++;
				}
			}
			else
			{
				if (tw > OUTPUT_BUFFER_SIZE)
					tw = OUTPUT_BUFFER_SIZE;
				for (int i = 0; i < tw; i++)
					outbuffer[i] = MadFixedToUshort(synth.pcm.samples[0][ptr++]);
			}
			output.write(outbuffer, tw*(stereo?4:2));
			written += tw;
			len -= tw;
		}
		if (status)
			return -1;
	}
	return written;
}

int eAudioDecoderMP3::getMinimumFramelength()
{
	return INPUT_BUFFER_SIZE;
}

void eAudioDecoderMP3::resync()
{
	if (stream.buffer)
		mad_stream_sync(&stream);
}

int eAudioDecoderMP3::getAverageBitrate()
{
	return avgbr;
}

#endif //DISABLE_FILE
