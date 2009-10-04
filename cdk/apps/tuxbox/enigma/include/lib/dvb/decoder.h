#ifndef __DECODER_H
#define __DECODER_H

		// all kind of decoding-related stuff
#define DECODE_AUDIO_MPEG	0
#define DECODE_AUDIO_AC3  1
#define DECODE_AUDIO_DTS  2
#define DECODE_AUDIO_AC3_VOB  3

#define TYPE_ES 0
#define TYPE_PES 1
#define TYPE_MPEG1 2

struct decoderParameters
{
	int vpid, apid, tpid, pcrpid, pmtpid;
	int audio_type;

	__u8 restart_camd;

	__u8 descriptors[2048];
	int descriptor_length;
};

class Decoder
{
	static struct fd
	{
		static int video, audio, demux_video, demux_audio, demux_pcr, demux_vtxt, mpeg;
	} fd;
public:
	static decoderParameters current;
	static int locked;
	static int getAudioDevice()	{ return fd.audio; }
	static int getVideoDevice()	{ return fd.video; }
	static int getMpegDevice()	{ return fd.mpeg; }
	static void setMpegDevice(int fdm)	{ fd.mpeg = fdm; }
	static decoderParameters parms;
	static int Initialize();
	static void Close();
	static void Flush(int keepaudiotype);
	static void Flush(); // for Binary compatibility
	static void Pause( int flags=1 );
	static void Resume( bool enableAudio=true );
	static void addCADescriptor(__u8 *descriptor);
	static int Set();
	static void flushBuffer();
	static void startTrickmode();
	static void stopTrickmode();
	static void SetStreamType(int type);
	static void setVideoFormat( int format );
	static int  displayIFrame(const char *frame, int len);
	static int  displayIFrameFromFile(const char *filename);
// non api functions
	static void flushClipBuffer();
	static void clearScreen();
			/* video pts is only 32 bit - the lsb is missing! (buggy driver..) */
	static void getVideoPTS( unsigned int &dest );
	static int  getSTC(unsigned long long &stc);
	static void setAutoFlushScreen(int);
	static void setFastZap(int);
};

#endif
