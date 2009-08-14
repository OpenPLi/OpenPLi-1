#ifndef DISABLE_FILE

#ifndef __lib_dvb_servicemp3_h
#define __lib_dvb_servicemp3_h

#include <lib/dvb/service.h>
#include <lib/base/buffer.h>

#include <lib/base/ebase.h>
#include <lib/base/thread.h>
#include <lib/base/message.h>
#include <lib/system/elock.h>
#include <lib/codecs/codec.h>

#include <lib/system/httpd.h>

class eServiceHandlerMP3;

class eHTTPStream: public eHTTPDataSource
{
	eIOBuffer &buffer;
	int bytes;
	int metadatainterval, metadataleft, metadatapointer;
	__u8 metadata[16*256+1]; // maximum size
	void processMetaData();
public:
	eHTTPStream(eHTTPConnection *c, eIOBuffer &buffer);
	~eHTTPStream();
	void haveData(void *data, int len);
	Signal0<void> dataAvailable;
	Signal1<void,eString> metaDataUpdated;
};

class eMP3Decoder: public eThread, public eMainloop, public Object
{
	eServiceHandlerMP3 *handler;
	eAudioDecoder *audiodecoder;
	eIOBuffer input;
	eIOBuffer output, output2;
	enum
	{
		stateInit, stateError, stateBuffering, stateBufferFull, statePlaying, statePause, stateFileEnd
	};

	bool skipping;
	int state;
	int dspfd[2];
	int type;

	int sourcefd;
	eHTTPStream *stream;
	eHTTPConnection *http;
	eString http_status;
	eString metadata[6];
	
	void metaDataUpdated(eString metadata);
	int getOutputDelay(int i); // in bytes
	int getOutputRate(int i); // in bytes/s
	
	int error;
	int outputbr;
	eSocketNotifier *inputsn, *outputsn[2];
	void streamingDone(int err);
	void decodeMoreHTTP();
	void decodeMore(int what);
	void outputReady(int what);
	void outputReady2(int what);
	void checkFlow(int last);
	void newAudioStreamIDFound( unsigned int id );

	void recalcPosition();
	eHTTPDataSource *createStreamSink(eHTTPConnection *conn);
	
	int maxOutputBufferSize, minOutputBufferSize;
	
	eAudioDecoder::pcmSettings pcmsettings;

	off64_t filelength, length, position;	// in bytes
	int divisor;

	int seekbusy;
	pthread_mutex_t lock;
	int audio_tracks;

	void dspSync();
	void newAudioStreamIdFound( unsigned int );

	void checkVideoFinished();
	eTimer checkVideoFinishedTimer;
	unsigned int prevVideoPTS;
	void init_eMP3Decoder(const char* filename);
public:
	int getType() { return type; }
	struct eMP3DecoderMessage
	{
		enum
		{
			start, exit,
			skip,
			setSpeed, // 0..
			seek,	// 0..65536
			seekreal,
			startSkip,
			stopSkip,
			setAudioStream
		};
		int type;
		int parm;
		eMP3DecoderMessage() { }
		eMP3DecoderMessage(int type): type(type) { }
		eMP3DecoderMessage(int type, int parm): type(type), parm(parm) { }
	};
	eFixedMessagePump<eMP3DecoderMessage> messages;
	
	void gotMessage(const eMP3DecoderMessage &message);
	eString getInfo(int id);

	enum { codecMP3, codecMPG, codecOGG, codecFLAC, codecLAVC };
	
	eMP3Decoder(int type, const char *filename, eServiceHandlerMP3 *handler);
	~eMP3Decoder();
	
	int getPosition(int);
	int getLength(int);
	
	int getError() const { return (state == stateError) ? error : 0; }
	
	void thread();
};

class eServiceHandlerMP3: public eServiceHandler
{
	eService *createService(const eServiceReference &service);
	void addFile(void *node, const eString &filename);
	friend class eMP3Decoder;

	struct eMP3DecoderMessage
	{
		enum
		{
			done,
			infoUpdated,
			status,
			newAudioStreamFound
		};
		int type;
		int parm;
		eMP3DecoderMessage() { }
		eMP3DecoderMessage(int type): type(type) { }
		eMP3DecoderMessage(int type, int status): type(type), parm(status) { }
	};
	eFixedMessagePump<eMP3DecoderMessage> messages;

	void gotMessage(const eMP3DecoderMessage &message);

	int state;
	eMP3Decoder *decoder;
	eServiceReference runningService;
public:
	int getID() const;

	eServiceHandlerMP3();
	~eServiceHandlerMP3();

	int play(const eServiceReference &service, int workaround=0 );
	int serviceCommand(const eServiceCommand &cmd);
	
	eString getInfo(int id);

	int getState();
	int getErrorInfo();

	int stop(int workaround=0);

	int getPosition(int what);
	int getErrorInfo() const;

	eService *addRef(const eServiceReference &service);
	void removeRef(const eServiceReference &service);

	void setAudioStream(unsigned int stream_id)
	{
		if ( decoder )
			decoder->messages.send(eMP3Decoder::eMP3DecoderMessage(eMP3Decoder::eMP3DecoderMessage::setAudioStream, (int)stream_id));
	}
};

class eServiceID3
{
		// tags are according to ID3v2
	std::map<eString, eString> tags;

	eString filename;

	enum
	{
		NOTPARSED,
		PARSED,
		NOTEXIST
	}state;
public:
	eServiceID3( const char *filename )
		:filename(filename), state( NOTPARSED )
	{
	}
	eServiceID3( const eServiceID3 &ref );
	
	std::map<eString, eString> &getID3Tags();
};

class eServiceMP3: public eService
{
	eServiceID3 id3tags;
	void init_eServiceMP3(const char *filename, const char *descr);
public:
	eServiceMP3(const char *filename, const char *descr=0);
	eServiceMP3(const eServiceMP3 &c);
};

#endif

#endif //DISABLE_FILE
