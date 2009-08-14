#ifndef DISABLE_FILE

#include <lib/base/eerror.h>
#include <lib/base/buffer.h>
#include <linux/soundcard.h>
#include <lib/codecs/codecflac.h>

/**
 * decoder  The decoder instance calling the callback.
 * buffer   A pointer to a location for the callee to store data to be decoded.
 * bytes    A pointer to the size of the buffer.  On entry to the callback,
 *          it contains the maximum number of bytes that may be stored in buffer.
 *          The callee must set it to the actual number of bytes stored (0 in
 *          case of error or end-of-stream) before returning.
 * client_data  The callee's client data set through FLAC__stream_decoder_set_client_data().
 */
FLAC__StreamDecoderReadStatus eAudioDecoderFLAC::my_flac_read(const FLAC__StreamDecoder *fl, FLAC__byte buffer[], unsigned *bytes, struct flac_data *client_data)
{
	long read_size = client_data->in->size();
	//eDebug("FLAC: read: wanted=%d available=%d", *bytes, read_size);

	if (read_size == 0)
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;

        read_size = client_data->in->read(buffer, *bytes);
	*bytes = read_size;
	/* 
	 * We need to do some calculations on the input bitrate so enigma can calculate
	 * the time left. FLAC does not have any info on on this.
	 * See the calculation in getAverageBitrate()
	 * To make it a bit stable, do not take into account the last read chunk as nothing
	 * of it is processed yet. For this reason we also cannot take all written data into 
	 * account yet. 
	 * Furthermore we assume that when FLAC want new data it more or less has processed the prevous
	 * read data and has decompressed (almost) all of it. So at that point we can make a 
	 * reasonable guess of the input bitrate.
	 */
	client_data->total_read += client_data->last_read_size;
	client_data->total_write += client_data->last_write_size;
	client_data->last_read_size = read_size;
	client_data->last_write_size = 0;

	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

/** 
 * decoder  The decoder instance calling the callback.
 * frame    The description of the decoded frame.  See FLAC__Frame.
 * buffer   An array of pointers to decoded channels of data.
 *          Each pointer will point to an array of signed samples of
 *          length frame->header.blocksize.
 * client_data  The callee's client data set through FLAC__stream_decoder_set_client_data().
 */
FLAC__StreamDecoderWriteStatus eAudioDecoderFLAC::my_flac_write(const FLAC__StreamDecoder *fl, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], struct flac_data *client_data)
{
	int error = 0;
	int blocksize = frame->header.blocksize;
	int bps;

	client_data->channels = FLAC__stream_decoder_get_channels(fl);
	client_data->samplerate = FLAC__stream_decoder_get_sample_rate(fl);
	bps = client_data->bps = FLAC__stream_decoder_get_bits_per_sample(fl);
	client_data->bitrate = 0;
	//eDebug("FLAC: write: chs=%d b/samp=%d sr=%d B/chan=%d %s=%d", client_data->channels, bps, client_data->samplerate, blocksize, frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER ? "framenum" : "samplenum", frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER ? frame->header.number.frame_number : frame->header.number.sample_number);
	// process per channel
	//	for stereo streams; channel 0 is left and 1 is right.

	int len = blocksize;
	int ptr = 0;
	int stereo = (client_data->channels==2);
	int shiftsize = bps - 16; // our dsp is set to 16 bit samples. so take the upper 16 bits of the data.
	if (shiftsize < 0)
		shiftsize = 0;

	const int OUTPUT_BUFFER_SIZE = 2047;  // 1152*2;
	unsigned short outbuffer[OUTPUT_BUFFER_SIZE];
	while (len)
	{
		int tw = len;
		if (stereo)
		{
			if (tw > OUTPUT_BUFFER_SIZE/2)
				tw = OUTPUT_BUFFER_SIZE/2;
			for (int i = 0; i < tw; i++)
			{
				outbuffer[i*2] = (unsigned short) ((buffer[0][ptr] >> 0) & 0xffff);
				outbuffer[i*2+1] = (unsigned short) ((buffer[1][ptr++] >> 0) & 0xffff);
			}
			client_data->out->write(outbuffer, tw*4);
		}
		else
		{
			if (tw > OUTPUT_BUFFER_SIZE)
				tw = OUTPUT_BUFFER_SIZE;
			for (int i = 0; i < tw; i++)
			{
				outbuffer[i] = (unsigned short) ((buffer[0][ptr++] >> 0) & 0xffff);
			}
			client_data->out->write(outbuffer, tw*2);
		}
		client_data->written += tw;
		// eDebug("FLAC: write: %s %d bytes total written=%d", stereo ? "stereo" : "mono", stereo ? tw*4 : tw * 2, client_data->written);
		len -= tw;
	}
	client_data->last_write_size += blocksize * client_data->channels;

	if (error)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	return  FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void eAudioDecoderFLAC::my_flac_error (const FLAC__StreamDecoder *fl, FLAC__StreamDecoderErrorStatus status, struct flac_data *client_data)
{
	/* FLAC__StreamDecoderErrorStatus:
		FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC,
		< An error in the stream caused the decoder to lose synchronization.

		FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER,
		< The decoder encountered a corrupted frame header.

		FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH
		< The frame's data did not match the CRC in the footer.
	*/
	eDebug("FLAC error: %s", FLAC__StreamDecoderErrorStatusString[status]);
	return;
}

void eAudioDecoderFLAC::my_flac_meta(const FLAC__StreamDecoder *fl, const FLAC__StreamMetadata *metadata, struct flac_data *client_data)
{
	// eDebug("FLAC: meta: last=%d %s len=%d", metadata->is_last, FLAC__MetadataTypeString[metadata->type], metadata->length);
	if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT)
	{
	// HOW CAN WE GET THIS DATA back to our caller class...
 		FLAC__StreamMetadata_VorbisComment *vc = (FLAC__StreamMetadata_VorbisComment *) &(metadata->data);
		eDebug("FLAC: meta: vorbis_vendor=%s", vc->vendor_string.entry);
		for (int i = 0; i < vc->num_comments; i++)
			eDebug("FLAC: meta: %d %s", i, vc->comments[i].entry);
	}
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
	{
 		FLAC__StreamMetadata_StreamInfo *si = (FLAC__StreamMetadata_StreamInfo *) &(metadata->data);
		eDebug("FLAC: meta: stream min_blocksize=%d max_blocksize=%d min_framesize=%d max_framesize=%d", si->min_blocksize, si->max_blocksize, si->min_framesize, si->max_framesize);
		eDebug("FLAC: meta: stream sample_rate=%d channels=%d bits_per_channel=%d total_samples=%ld", si->sample_rate, si->channels, si->bits_per_sample, si->total_samples);
	}
	return ;
}

eAudioDecoderFLAC::eAudioDecoderFLAC(eIOBuffer &input, eIOBuffer &output): 
		framecnt(0), input(input), output(output)
{
	my_data.in = &input;
	my_data.out = &output;
	my_data.total_read = my_data.total_write = 0;
	my_data.last_read_size = my_data.last_write_size = 0;

	fl = FLAC__stream_decoder_new();
#if !defined(FLAC_API_VERSION_CURRENT) || FLAC_API_VERSION_CURRENT <= 7
	// set callbacks
	FLAC__stream_decoder_set_read_callback(fl,
		(FLAC__StreamDecoderReadStatus (*)(const FLAC__StreamDecoder*, FLAC__byte*, unsigned int*, void*))my_flac_read);
	FLAC__stream_decoder_set_write_callback(fl,
		(FLAC__StreamDecoderWriteStatus (*)(const FLAC__StreamDecoder*, const FLAC__Frame*, const FLAC__int32* const*, void*))my_flac_write);
	FLAC__stream_decoder_set_metadata_callback(fl,
		(void (*)(const FLAC__StreamDecoder*, const FLAC__StreamMetadata*, void*))my_flac_meta);
	FLAC__stream_decoder_set_error_callback(fl,
		(void (*)(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus, void*))my_flac_error);
	// set my_data to a hold input and output buffers so the can be found in the read an dwrite callbacks.
	FLAC__stream_decoder_set_client_data(fl, &my_data);
	FLAC__stream_decoder_set_metadata_respond(fl, FLAC__METADATA_TYPE_STREAMINFO);
	FLAC__stream_decoder_set_metadata_respond(fl, FLAC__METADATA_TYPE_VORBIS_COMMENT);
	/* FLAC__StreamDecoderState */ FLAC__stream_decoder_init(fl);
#else
	FLAC__stream_decoder_set_metadata_respond(fl, FLAC__METADATA_TYPE_STREAMINFO);
	FLAC__stream_decoder_set_metadata_respond(fl, FLAC__METADATA_TYPE_VORBIS_COMMENT);

	FLAC__stream_decoder_init_stream(
		fl,
		(FLAC__StreamDecoderReadStatus (*)(const FLAC__StreamDecoder*, FLAC__byte*, unsigned int*, void*))my_flac_read,
		NULL, /* seek */
		NULL, /* tell */
		NULL, /* length */
		NULL, /* EOF */
		(FLAC__StreamDecoderWriteStatus (*)(const FLAC__StreamDecoder*, const FLAC__Frame*, const FLAC__int32* const*, void*))my_flac_write,
		(void (*)(const FLAC__StreamDecoder*, const FLAC__StreamMetadata*, void*))my_flac_meta,
		(void (*)(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus, void*))my_flac_error,
		&my_data
	);
#endif
}

eAudioDecoderFLAC::~eAudioDecoderFLAC()
{
	FLAC__stream_decoder_finish(fl);
	FLAC__stream_decoder_delete(fl);
}

eString eAudioDecoderFLAC::getAudioType()
{
	return eString().sprintf("FLAC %s", my_data.channels > 1 ? "Stereo": "Mono");
}

int eAudioDecoderFLAC::decodeMore(int last, int maxsamples, Signal1<void, unsigned int> *)
{
	my_data.written = 0;
	// eDebug("decodeMore: maxsamples=%d", maxsamples);
	while (last || (my_data.written < maxsamples))
	{
		long read_size;
		
		read_size = input.size();
		// eDebug("FLAC: decodeMore bytes available=%ld", read_size);
		if (read_size == 0)
			break;

		FLAC__stream_decoder_process_single(fl);
		// the write callback will update the written count.
		FLAC__StreamDecoderState fl_state;
		fl_state = FLAC__stream_decoder_get_state(fl);
		// eDebug("FLAC: decodeMore2 written=%d (continue till %d) %s", my_data.written, maxsamples, FLAC__StreamDecoderStateString[fl_state]); // should be the same as above?
		if (! ( fl_state == FLAC__STREAM_DECODER_READ_METADATA ||
			fl_state == FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC ||
			fl_state == FLAC__STREAM_DECODER_READ_FRAME) )
		{
			// probably a read error or so, because input buffer was empty...
			// eDebug("FLAC: decodeMore: do we like this flac state?");
			FLAC__stream_decoder_flush(fl);
			break;
		}
		pcmsettings.samplerate = my_data.samplerate;
		pcmsettings.channels = my_data.channels;
		pcmsettings.format = AFMT_S16_BE;
		pcmsettings.reconfigure = 0;
	}
	return my_data.written;
}

int eAudioDecoderFLAC::getMinimumFramelength()
{
	// eDebug("FLAC: getMinimumFramelength");
	return INPUT_BUFFER_SIZE;
}

void eAudioDecoderFLAC::resync()
{
	// eDebug("FLAC: resync");
	// FLAC__stream_decoder_flush(fl);
}

int eAudioDecoderFLAC::getAverageBitrate()
{
	// eDebug("FLAC: getAverageBitrate = %d", br);
	// there seems no header val specifying this so guess with a rough caclculation
	// this does not take into account reads for metadata, and if we are inbetween reads and writes...

	long tot_sample_secs = (my_data.total_write)/ my_data.samplerate / (my_data.bps/8);
	
	return tot_sample_secs ? my_data.total_read * 8 / tot_sample_secs : 0;
}

#endif //DISABLE_FILE
