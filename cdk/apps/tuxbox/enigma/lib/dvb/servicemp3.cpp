#ifndef DISABLE_FILE

#define HAVE_OGG
#define HAVE_FLAC
//#define HAVE_LAVC

#include <lib/dvb/servicemp3.h>
#include <config.h>
#include <lib/dvb/servicefile.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/base/i18n.h>
#include <lib/codecs/codecmp3.h>
#include <lib/codecs/codecmpg.h>
#ifdef HAVE_OGG
# include <lib/codecs/codecogg.h>
#endif
#ifdef HAVE_FLAC
# include <lib/codecs/codecflac.h>
#endif
#ifdef HAVE_LAVC
# include <lib/codecs/codeclavc.h>
#endif
#include <lib/dvb/decoder.h>

#include <unistd.h>
#include <fcntl.h>
#include <id3tag.h>
#include <linux/soundcard.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

#if HAVE_DVB_API_VERSION < 3
# include <ost/audio.h>
# define AUDIO_DEV "/dev/dvb/card0/audio0"
#else
# include <linux/dvb/audio.h>
# define AUDIO_DEV "/dev/dvb/adapter0/audio0"
#endif

/*
	note: mp3 decoding is done in ONE seperate thread with multiplexed input/
	decoding and output. The only problem arises when the ::read-call,
	encapsulated in eIOBuffer::fromfile, blocks. althought we only call
	it when data is "ready" to read (according to ::poll), this doesn't help
	since linux/posix/unix/whatever doesn't support proper read-ahead with
	::poll-notification. bad luck for us.
	
	the only way to address this problem (except using ::aio_*) is to
	use another thread. i don't like threads so if you really have a slow
	network/harddisk, it's your problem. sorry.
*/
int curr_br;
unsigned int curr_sr;

eHTTPStream::eHTTPStream(eHTTPConnection *c, eIOBuffer &buffer): eHTTPDataSource(c), buffer(buffer)
{
#if 0
	if (c->remote_header.count("Content-Length"))
		total=atoi(c->remote_header["Content-Length"].c_str());
	else
		total=-1;
	received=0;
#endif
	eDebug("HTTP stream sink created!");
	metadatainterval=metadataleft=bytes=0;
	if (c->remote_header.count("icy-metaint"))
		metadatainterval=atoi(c->remote_header["icy-metaint"].c_str());
	eDebug("metadata interval: %d", metadatainterval);
}

eHTTPStream::~eHTTPStream()
{
	eDebug("HTTP stream sink deleted!");
}

void eHTTPStream::processMetaData()
{
	metadata[metadatapointer]=0;
	metaDataUpdated((const char*)metadata);
//	eDebug("processing metadata! %s", metadata);

	metadatapointer=0;
}

void eHTTPStream::haveData(void *vdata, int len)
{
	__u8 *data=(__u8*)vdata;
	
	while (len)
	{
		int valid=len;
		if (!metadataleft)
		{
				// not in metadata mode.. process mp3 data (stream to input buffer)

				// are we just at the beginning of metadata? (pointer)
			if (metadatainterval && (metadatainterval == bytes))
			{
						// enable metadata mode
				metadataleft=*data++*16;
				metadatapointer=0;
				len--;
				bytes=0;
				continue;
			} else if (metadatainterval && (metadatainterval < bytes))
				eFatal("metadatainterval < bytes");

				// otherwise there's really data.
			if (metadatainterval)
			{
					// is metadata in our buffer?
				if ((valid + bytes) > metadatainterval)
					valid=metadatainterval-bytes;
			}
			buffer.write(data, valid);
			data+=valid;
			len-=valid;
			bytes+=valid;
		} else
		{
				// metadata ... process it.
			int meta=len;
			if (meta > metadataleft)
				meta=metadataleft;

			memcpy(metadata+metadatapointer, data, meta);

			metadatapointer+=meta;
			data+=meta;
			len-=meta;
			metadataleft-=meta;
			
			if (!metadataleft)
				processMetaData();
		}
	}
	dataAvailable();
}

void eMP3Decoder::checkVideoFinished()
{
	unsigned int newPTS=0xFFFFFFFF;
	Decoder::getVideoPTS(newPTS);
	if ( prevVideoPTS != 0xFFFFFFFF )
	{
		if ( newPTS == prevVideoPTS )
			handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::done));
		else
			checkVideoFinishedTimer.start(2000, true);
	}
	else
		checkVideoFinishedTimer.start(2000, true);
	prevVideoPTS = newPTS;
}

eMP3Decoder::eMP3Decoder(int type, const char *filename, eServiceHandlerMP3 *handler)
: handler(handler), input(8*1024), output(256*1024),
	output2(256*1024), skipping(false), type(type), outputbr(0)
	,inputsn(0), audio_tracks(0), checkVideoFinishedTimer(this)
	,prevVideoPTS(0xFFFFFFFF), messages(this, 1)
{
	init_eMP3Decoder(filename);
}
void eMP3Decoder::init_eMP3Decoder(const char* filename)
{
	state=stateInit;

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&lock, &attr);
	pthread_mutexattr_destroy(&attr);

	http=0;

	seekbusy=0;

	curr_br = 0;
	curr_sr = 0;
//	filename="http://205.188.209.193:80/stream/1003";

//	filename="http://sik1.oulu.fi:8002/";
//	filename="http://64.236.34.141:80/stream/1022";
//	filename="http://ios.h07.org:8006/";
//	filename="http://10.0.0.112/join.mp3";
	
	if (strstr(filename, "://")) // assume streaming
	{
		if (!strncmp(filename, "file://", 7))
			filename+=7;
		else
		{
			http=eHTTPConnection::doRequest(filename, this, &error);
			if (!http)
			{
				streamingDone(error);
			} else
			{
				CONNECT(http->transferDone, eMP3Decoder::streamingDone);
				CONNECT(http->createDataSource, eMP3Decoder::createStreamSink);
				http->local_header["User-Agent"]="enigma-mp3/1.0.0";
				http->local_header["Icy-MetaData"]="1"; // enable ICY metadata
				http->start();
				http_status=_("Connecting...");
				filelength=-1;
				handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::status));
			}
			filename=0;
		}
	}
	
	if (filename) // not streaming
	{
		sourcefd=::open(filename, O_RDONLY|O_LARGEFILE);
		if (sourcefd<0)
		{
			error=errno;
			eDebug("MP3dec: error opening %s", filename);
			state=stateError;
		} else
		{
			filelength=lseek64(sourcefd, 0, SEEK_END);
			lseek(sourcefd, 0, SEEK_SET);
		}
	} else
		sourcefd=-1;
	
	length=-1;

	if (type != codecMPG)
	{
		divisor=1;
		pcmsettings.reconfigure=1;
		dspfd[1]=-1;

		dspfd[0]=::open("/dev/sound/dsp", O_WRONLY|O_NONBLOCK);
		if (dspfd[0]<0)
		{
			eDebug("MP3dec: output failed! (%m)");
			error=errno;
			state=stateError;
		}
	
		if (dspfd[0] >= 0)
		{
			outputsn[0]=new eSocketNotifier(this, dspfd[0], eSocketNotifier::Write, 0);
			CONNECT(outputsn[0]->activated, eMP3Decoder::outputReady);
		} else
			outputsn[0]=0;
		outputsn[1]=0;
	} else
	{
		divisor=1024;
		Decoder::parms.vpid=0x1FFF;
		Decoder::parms.apid=0x1FFF;
		Decoder::parms.pcrpid=-1;
		Decoder::parms.audio_type=DECODE_AUDIO_MPEG;
		Decoder::Set();

		dspfd[0]=::open("/dev/video", O_WRONLY|O_NONBLOCK);
		dspfd[1]=::open("/dev/sound/dsp1", O_WRONLY|O_NONBLOCK);

		if ((dspfd[0]<0) || (dspfd[1]<0))
		{
			if (dspfd[0]>=0)
				::close(dspfd[0]);
			if (dspfd[1]>=0)
				::close(dspfd[1]);
			eDebug("output failed! (%m)");
			error=errno;
			state=stateError;
			outputsn[0]=0;
			outputsn[1]=0;
		} else
		{
			Decoder::setMpegDevice(dspfd[0]);
			outputsn[0]=new eSocketNotifier(this, dspfd[0], eSocketNotifier::Write, 0);
			CONNECT(outputsn[0]->activated, eMP3Decoder::outputReady);
			outputsn[1]=new eSocketNotifier(this, dspfd[1], eSocketNotifier::Write, 0);
			CONNECT(outputsn[1]->activated, eMP3Decoder::outputReady2);
		}
	}

	switch (type)
	{
	case codecMP3:
		audiodecoder=new eAudioDecoderMP3(input, output);
		break;
#ifdef HAVE_OGG
	case codecOGG:
		audiodecoder=new eAudioDecoderOGG(input, output);
		break;
#endif
#ifdef HAVE_FLAC
	case codecFLAC:
		audiodecoder=new eAudioDecoderFLAC(input, output);
		break;
#endif
#ifdef HAVE_LAVC
	case codecLAVC:
		audiodecoder=new eAudioDecoderLAVC(input, output);
		break;
#endif
	case codecMPG:
	{
		eString fname=filename;
		if (fname.right(4).upper()==".PVA")
			audiodecoder=new ePVADemux(input, output, output2, dspfd[0], sourcefd);
		else
			audiodecoder=new eMPEGDemux(input, output, output2, dspfd[0], sourcefd);
		CONNECT(checkVideoFinishedTimer.timeout, eMP3Decoder::checkVideoFinished );
		break;
	}
	default:
		audiodecoder = NULL;
		return; /* don't start */
	}

	if (sourcefd >= 0)
	{
		if ( type == codecMPG )
		{
			unsigned char buffer[256*1024];
			eString fname=filename;
			off64_t oldpos = lseek64(sourcefd,
				fname.right(4).upper()==".BIN" ? 1024*1024 : 0, SEEK_CUR);
			((eDemux*)audiodecoder)->extractSequenceHeader(buffer, read(sourcefd, buffer, 256*1024));
			lseek64(sourcefd, oldpos, SEEK_SET);
		}
		inputsn=new eSocketNotifier(this, sourcefd, eSocketNotifier::Read, 0);
		CONNECT(inputsn->activated, eMP3Decoder::decodeMore);
	}

	CONNECT(messages.recv_msg, eMP3Decoder::gotMessage);

	maxOutputBufferSize=256*1024;
	minOutputBufferSize=8*1024;

	run();
}

		// we got (http) metadata.
void eMP3Decoder::metaDataUpdated(eString meta)
{
	{ // protect the singleLock from going through the handler->message.send() call
		eDebug("MP3: metaupdate: %s", meta.c_str());			
		eString streamTitle, streamUrl;
		singleLock s(lock);  // must protect access on metadata array
		if (meta.left(6) == "Stream")
			while (!meta.empty())
			{
				unsigned int eq=meta.find('=');
				if (eq == eString::npos)
						break;
				eString left=meta.left(eq);
				meta=meta.mid(eq+1); // skip until =
				eq=meta.find(';');
				if (eq == eString::npos)
					break;
				eString right=meta.left(eq);
				meta=meta.mid(eq+1);
				if (left=="StreamTitle")
					streamTitle=right;
				else if (left == "StreamUrl")
					streamUrl=right;
				else
					eDebug("MP3dec: unknown tag: %s = %s", left.c_str(), right.c_str());			
			}
		else
			streamTitle=meta;

		metadata[0]=streamTitle;
		metadata[1]=streamUrl;
	}

	handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::infoUpdated));
}

int eMP3Decoder::getOutputDelay(int i)
{
	int delay = 0;
	(void)i;
	if (::ioctl(dspfd[type==codecMPG], SNDCTL_DSP_GETODELAY, &delay) < 0)
		eDebug("MP3dec: SNDCTL_DSP_GETODELAY failed (%m)");
	return delay;
}

int eMP3Decoder::getOutputRate(int i)
{
	(void)i;
	return pcmsettings.samplerate*pcmsettings.channels*2; // use 16 bit samples!
}

void eMP3Decoder::streamingDone(int err)
{
	if (err || !http || http->code != 200)
	{
		{
			singleLock s(lock);  // must protect access on http_status
			if (err)
			{
				switch (err)
				{
				case -2:
					http_status="Can't resolve hostname!";
					break;
				case -3:
					http_status="Can't connect!";
					break;
				default:
					http_status.sprintf("unknown error %d", err);
				}
			}
			else if (http && (http->code!=200))
				http_status.sprintf("error: %d (%s)", http->code, http->code_descr.c_str());
			else	
				http_status="unknown";
		}
		handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::status));
	}
	else
	{
		state=stateFileEnd;
		outputsn[0]->start();
		if (outputsn[1])
			outputsn[1]->start();
		eDebug("MP3dec: streaming done!");
	}
	http=0;
}

eHTTPDataSource *eMP3Decoder::createStreamSink(eHTTPConnection *conn)
{
	stream=new eHTTPStream(conn, input);
	CONNECT(stream->dataAvailable, eMP3Decoder::decodeMoreHTTP);
	CONNECT(stream->metaDataUpdated, eMP3Decoder::metaDataUpdated);
	{
		singleLock s(lock);  // must protect access on http_status
		if (conn->remote_header["Content-Type"].upper().find("OGG") != eString::npos)
			type = eMP3Decoder::codecOGG;
		http_status=_("buffering...");
	}
	handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::status));
	return stream;
}

void eMP3Decoder::thread()
{
	exec();
}

void eMP3Decoder::outputReady(int what)
{
	(void)what;
	{
		singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
		if (type != codecMPG)
		{
			int sendmsg = 0;
			if ( ( pcmsettings.reconfigure 
				|| (pcmsettings.samplerate != audiodecoder->pcmsettings.samplerate) 
				|| (pcmsettings.channels != audiodecoder->pcmsettings.channels)))
			{
				pcmsettings=audiodecoder->pcmsettings;

				outputbr=pcmsettings.samplerate*pcmsettings.channels*16;
				if (::ioctl(dspfd[0], SNDCTL_DSP_SPEED, &pcmsettings.samplerate) < 0)
					eDebug("MP3dec: SNDCTL_DSP_SPEED failed (%m)");
				if (::ioctl(dspfd[0], SNDCTL_DSP_CHANNELS, &pcmsettings.channels) < 0)
					eDebug("MP3dec: SNDCTL_DSP_CHANNELS failed (%m)");
				if (::ioctl(dspfd[0], SNDCTL_DSP_SETFMT, &pcmsettings.format) < 0)
					eDebug("MP3dec: SNDCTL_DSP_SETFMT failed (%m)");
				//eDebug("MP3dec: reconfigured audio interface...");
				singleLock s(lock); // protect access on metadata array
				metadata[2] = audiodecoder->getAudioType();
				metadata[3] = eString().sprintf("%d kHz", pcmsettings.samplerate/1000);
				sendmsg = 1;
			}
			if (curr_br != audiodecoder->getAverageBitrate())
			{
				curr_br = audiodecoder->getAverageBitrate();
				singleLock s(lock); // protect access on metadata array
				metadata[4] = eString().sprintf("%d kbps", curr_br/1000);
				sendmsg = 1;
			}
			if (sendmsg)
				handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::infoUpdated));
		}
		seekbusy -= output.tofile(dspfd[0], 65536);
		if (seekbusy < 0)
			seekbusy=0;
	}
	checkFlow(0);
}

void eMP3Decoder::outputReady2(int what)
{
	(void)what;
	{
		singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
		output2.tofile(dspfd[1], 65536);
	}
	checkFlow(0);
}

void eMP3Decoder::checkFlow(int last)
{
	//eDebug("I: %d O: %d S: %d", input.size(), output.size(), state);

	if (state == statePause)
		return;

	int i=input.size(), o[2];
	o[0]=output.size();
	if (outputsn[1])
		o[1]=output2.size();
	else
		o[1]=0;

	//eDebug("input: %d  video %d   audio %d", input.size(), output.size(), output2.size());
	
	// states:
	// buffering  -> output is stopped (since queue is empty)
	// playing    -> input queue (almost) empty, output queue filled
	// bufferFull -> input queue full, reading disabled, output enabled

	if (state == stateFileEnd)
	{
		int bla=0;
		if ( !o[0] )
		{
			bla |= 1;
			outputsn[0]->stop();
		}
		if (outputsn[1] && !o[1])
		{
			bla|=2;
			outputsn[1]->stop();
		}
		if ( bla == (outputsn[1] ? 3 : 1) )
		{
			eDebug("MP3dec: ok, everything played..");
			if ( type != codecMPG )
				handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::done));
			else
				checkVideoFinished();
		}
		return;
	} 
	else if (state != stateBuffering && !o[0] && (!outputsn[1] || !o[1]) )
	{
		//eDebug("MP3dec: stateBuffering");
		state=stateBuffering;
		if ( !o[0] )
			outputsn[0]->stop();
	}

	if (outputsn[1])
	{
		if (!o[1])
			outputsn[1]->stop();
		else if (o[1] > 16384)
			outputsn[1]->start();
	}

	if ( o[0] > maxOutputBufferSize || o[1] > maxOutputBufferSize )
	{
		if (state != stateBufferFull)
		{
			// eDebug("MP3dec: stateBufferFull");
			if (inputsn)
				inputsn->stop();
			else if (http)
				http->disableRead();
			state=stateBufferFull;
		}
	}

	if ( o[0] < maxOutputBufferSize)// && o[1] < maxOutputBufferSize )
	{
		int samples=0;
		if (last || (i >= audiodecoder->getMinimumFramelength()))
		{
			Signal1<void, unsigned int> callback;
			CONNECT(callback, eMP3Decoder::newAudioStreamIdFound);
			singleLock s(lock); // protect access on all eIOBuffer
			samples=audiodecoder->decodeMore(last, 16384, &callback);
		}
		if (samples < 0)
		{
			state=stateFileEnd;
			eDebug("MP3dec: invalid data found");
		}

#if 0
		if (curr_sr != audiodecoder->pcmsettings.samplerate || curr_br != audiodecoder->getAverageBitrate())
		{
			// eDebug("[servicemp3]: oldsr=%d, newsr=%d oldbr=%d newbr=%d", curr_sr, audiodecoder->pcmsettings.samplerate, curr_br, audiodecoder->getAverageBitrate());
			{
				singleLock s(lock); // protect access on metadata array
				metadata[2] = audiodecoder->getAudioType();
				curr_sr = audiodecoder->pcmsettings.samplerate;
				metadata[3] = eString().sprintf("%d kHz", curr_sr/1000);
				curr_br = audiodecoder->getAverageBitrate();
				metadata[4] = eString().sprintf("%d kbps", curr_br/1000);
			}
			handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::infoUpdated));
		}
#endif
			
		if ((o[0] + samples) < maxOutputBufferSize)
		{
			if (state == stateBufferFull)
			{
//				eDebug("stateBufferFull -> statePlaying");
				state=statePlaying;
			}
			if (inputsn)
				inputsn->start();
			else if (http)
				http->enableRead();
		}
	}

	if ( state == stateBuffering
		&& o[0] > (inputsn?minOutputBufferSize:minOutputBufferSize*16) )
	{
//		eDebug("statePlaying...");
		if ( !inputsn && http )
		{
			singleLock s(lock);  // must protect access on http_status
			http_status=_("playing...");
			handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::status));
		}
		state=statePlaying;
		outputsn[0]->start();
		if (outputsn[1])
			outputsn[1]->start();
	}
}

void eMP3Decoder::recalcPosition()
{
	singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
	if (audiodecoder->getAverageBitrate() > 0)
	{
		if (filelength != -1)
			length=filelength/(audiodecoder->getAverageBitrate()>>3);
		else
			length=-1;
		if (sourcefd > 0)
		{
			position=::lseek64(sourcefd, 0, SEEK_CUR);
			position+=input.size();
			position/=(audiodecoder->getAverageBitrate()>>3);
			if (type != codecMPG)
				position += output.size() / pcmsettings.samplerate / pcmsettings.channels / 2;
			else
				position += (output.size() + output2.size()) / audiodecoder->getAverageBitrate();
		} else
			position=-1;
	} else
		length=position=-1;
}

void eMP3Decoder::dspSync()
{
	eDebug("dspSync");
	if (::ioctl(dspfd[type==codecMPG], SNDCTL_DSP_RESET) < 0)
		eDebug("MP3dec: SNDCTL_DSP_RESET failed (%m)");

	if (type == codecMPG)
		Decoder::flushClipBuffer();
	Decoder::flushBuffer();
}

void eMP3Decoder::newAudioStreamIdFound(unsigned int id)
{
	eDebug("MP3dec::newAudioStreamIdFound(%02x)", id);
	handler->messages.send(eServiceHandlerMP3::eMP3DecoderMessage(eServiceHandlerMP3::eMP3DecoderMessage::newAudioStreamFound, (int)id));
	++audio_tracks;
}

void eMP3Decoder::decodeMoreHTTP()
{
	checkFlow(0);
}

void eMP3Decoder::decodeMore(int what)
{
	int flushbuffer=0;
	(void)what;

	if (state == stateFileEnd)
	{
		inputsn->stop();
		return;
	}

	while ( input.size() < audiodecoder->getMinimumFramelength() )
	{
		singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
		if (input.fromfile(sourcefd, audiodecoder->getMinimumFramelength()) < audiodecoder->getMinimumFramelength())
		{
			eDebug("MP3dec: couldn't read %d bytes", audiodecoder->getMinimumFramelength());
			flushbuffer=1;
			break;
		}
	}

	checkFlow(flushbuffer);

	if (flushbuffer)
	{
		eDebug("MP3dec: end of file...");
		state=stateFileEnd;
		inputsn->stop();
		outputsn[0]->start();
		if (outputsn[1])
			outputsn[1]->start();
		if ( skipping )
		{
			skipping=false;
			if ( type == codecMPG )
				Decoder::stopTrickmode();
		}
	}
}

eMP3Decoder::~eMP3Decoder()
{
	messages.send(eMP3DecoderMessage(eMP3DecoderMessage::exit));
	kill(); // wait for thread exit.

	int fd = Decoder::getAudioDevice();
	bool wasOpen = fd != -1;
	if (!wasOpen)
		fd = open(AUDIO_DEV, O_RDWR);
	if ( type == codecMPG && ioctl(fd, AUDIO_SET_MUTE, 0) < 0)
		eDebug("MP3dec: AUDIO_SET_MUTE error (%m)");
	if (!wasOpen)
		close(fd);

	delete inputsn;
	delete outputsn[0];
	delete outputsn[1];
	if (dspfd[0] >= 0)
		close(dspfd[0]);
	if (dspfd[1] >= 0)
		close(dspfd[1]);
	if (sourcefd >= 0)
		close(sourcefd);
	if (http)
		delete http;
	delete audiodecoder;
	Decoder::setMpegDevice(-1);
	Decoder::SetStreamType(TYPE_PES);
	Decoder::parms.vpid=-1;
	Decoder::parms.apid=-1;
	Decoder::parms.pcrpid=-1;
	Decoder::parms.audio_type=DECODE_AUDIO_MPEG;
	Decoder::Set();
	pthread_mutex_destroy(&lock);
}

void eMP3Decoder::gotMessage(const eMP3DecoderMessage &message)
{
	switch (message.type)
	{
	case eMP3DecoderMessage::start:
		if (state == stateError)
			break;
		if (state == stateInit)
		{
			state=stateBuffering;
			outputsn[0]->stop();
			if ( outputsn[1] )
				outputsn[1]->stop();
			if (inputsn)
				inputsn->start();
			else
				eDebug("MP3dec: handle streaming init");
		}
		break;
	case eMP3DecoderMessage::exit:
		eDebug("MP3dec: got quit message..");
		if ( state == statePause )
			Decoder::Resume();
		dspSync();
		quit();
		break;
	case eMP3DecoderMessage::setSpeed:
		if (state == stateError)
			break;
		if (!inputsn)
			break;
		// speed=message.parm;
		if (message.parm == 0)
		{
			if ((state==stateBuffering) ||
				(state==stateBufferFull) ||
				(statePlaying))
			{
				inputsn->stop();
				outputsn[0]->stop();
				if (outputsn[1])
					outputsn[1]->stop();
				state=statePause;
				if ( type == codecMPG )
				{
					singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
					::lseek64(sourcefd, getPosition(1)*divisor, SEEK_SET);
					input.clear();
					output.clear();
					output2.clear();
					dspSync();
					audiodecoder->resync();
					Decoder::Pause();      
				}
				else
					dspSync();
			}
		} else if (state == statePause)
		{
			inputsn->start();
			outputsn[0]->start();
			if (outputsn[1])
				outputsn[1]->start();
			//speed=message.parm;
			state=stateBuffering;
			if ( type == codecMPG )
			{
				Decoder::Resume();
				checkFlow(0);
			}
		}
		else
		{
			singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
			output.clear();
			if (outputsn[1])
				output2.clear();
			dspSync();
		}
		break;
	case eMP3DecoderMessage::setAudioStream:
		if ( type == codecMPG && audiodecoder )
		{
			singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
			int position = getPosition(1);
			Decoder::Pause(0);
			((eMPEGDemux*)audiodecoder)->setAudioStream(message.parm);
			::lseek64(sourcefd, ((off64_t)position)*divisor, SEEK_SET);
			input.clear();
			output.clear();
			output2.clear();
			dspSync();
			audiodecoder->resync();
			Decoder::Resume(false);
			checkFlow(0);
		}
		break;
	case eMP3DecoderMessage::startSkip:
		if ( !skipping )
		{
			skipping=true;
			if (ioctl(Decoder::getAudioDevice(), AUDIO_SET_MUTE, 1) < 0)
				eDebug("MP3dec: AUDIO_SET_MUTE error (%m)");
		}
		break;
	case eMP3DecoderMessage::stopSkip:
		if ( skipping )
		{
			skipping=false;
			if (ioctl(Decoder::getAudioDevice(), AUDIO_SET_MUTE, 0) < 0)
				eDebug("MP3dec: AUDIO_SET_MUTE error (%m)");
		}
		break;
	case eMP3DecoderMessage::seek:
	case eMP3DecoderMessage::seekreal:
	case eMP3DecoderMessage::skip:
	{
		if (state == stateError)
			break;
		if (!inputsn)
			break;
		eDebug("MP3dec: seek/seekreal/skip, %d", message.parm);
		if (seekbusy && type == codecMPG)
		{
			checkFlow(0);
			outputsn[0]->start();
			break;
		}
		seekbusy=128*1024; // next seek only after 128k (video) data
		off64_t offset=0;
		if (message.type != eMP3DecoderMessage::seekreal)
		{
			off64_t br=audiodecoder->getAverageBitrate();
			if ( br <= 0 )
				break;
			br*=(message.parm > 0 || type != codecMPG ? message.parm : message.parm - 16000)/8000;
			offset=input.size()+br;
			if (message.type == eMP3DecoderMessage::skip)
			{
				singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
				offset+=::lseek64(sourcefd, 0, SEEK_CUR);
			}
			if (offset<0)
				offset=0;
		}
		else
			offset=((off64_t)message.parm)*divisor;

		singleLock s(lock); // must protect access on all eIOBuffer and position
		input.clear();
		if ( ::lseek64(sourcefd, offset, SEEK_SET) < 0 )
			eDebug("MP3dec: seek error (%m)");
		output.clear();
		if ( type == codecMPG )
			output2.clear();
		dspSync();
		audiodecoder->resync();
		checkFlow(0);

		break;
	}
	}
}

eString eMP3Decoder::getInfo(int id)
{
	singleLock s(lock);  // must protect access on http_status and metadata
	if (id == 0)
		return http_status;
	else if (id < 6)
		return metadata[id - 1];
	else
		return "";
}

int eMP3Decoder::getPosition(int real)
{
	if (sourcefd < 0)
		return -1;

	singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
	if ( type == codecMPG && real)
	{
		off64_t position=::lseek64(sourcefd, 0, SEEK_CUR);
		if ( !position )
			return 0;
		if ( position > (1024*1024*2) )
			position-=1024*1024*2;// latency
		else
			return 0;
		if ( position > (unsigned int)input.size() )
			position-=input.size();
		else
			return 0;
		if ( position > (unsigned int)output.size() )
			position-=output.size();
		else
			return 0;
		if ( position > (unsigned int)(output2.size()*audio_tracks) )
			position-=output2.size()*audio_tracks;
		else
			return 0;
		if ( position > (unsigned int)getOutputDelay(0) )
			position-=getOutputDelay(0);
		else
			return 0;
		return position / divisor;
	}
	recalcPosition();
	if (real)
	{
		// our file pos
		off64_t realpos = ::lseek64(sourcefd, 0, SEEK_CUR);
		// minus bytes still in input buffer
		realpos -= input.size();
		// minus not yet played data in a.) output buffers and b.) dsp buffer
		long long int nyp = output.size();
//		eDebug("%d bytes not played in outputbuffer", nyp);
		int obr = getOutputRate(0);
		if (obr > 0)
		{
			int tmp = getOutputDelay(0);
//			eDebug("%d bytes not played in audio dsp", tmp);
			nyp += tmp;
			// convert to input bitrate
//			eDebug("average bitrate %d, outputbuffer rate %d", audiodecoder->getAverageBitrate()/8, obr);
			nyp *= (long long int)audiodecoder->getAverageBitrate()/8;
			nyp /= (long long int)obr;
//			eDebug("odelay: %d bytes", (int)nyp);
		} else
			nyp = 0;

		return (realpos-nyp)/divisor;
	}
	int ret = audiodecoder->getSecondsCurrent();
	if (ret >=0)
		return ret;

	return position;
}

int eMP3Decoder::getLength(int real)
{
	if (sourcefd < 0)
		return -1;
	singleLock s(lock); // must protect access on all eIOBuffer, position, outputbr
	if (real)
		return filelength / divisor;
	int ret = audiodecoder->getSecondsDuration();
	if (ret >= 0)
		return ret;
	return length+output.size()/(outputbr/8);
}

void eServiceHandlerMP3::gotMessage(const eMP3DecoderMessage &message)
{
	if (message.type == eMP3DecoderMessage::done)
	{
		state=stateStopped;
		serviceEvent(eServiceEvent(eServiceEvent::evtEnd));
	}
	else if (message.type == eMP3DecoderMessage::status)
		serviceEvent(eServiceEvent(eServiceEvent::evtStatus));
	else if (message.type == eMP3DecoderMessage::infoUpdated)
		serviceEvent(eServiceEvent(eServiceEvent::evtInfoUpdated));
	else if (message.type == eMP3DecoderMessage::newAudioStreamFound)
		serviceEvent(eServiceEvent(eServiceEvent::evtAddNewAudioStreamId,message.parm));
}

eService *eServiceHandlerMP3::createService(const eServiceReference &service)
{
	if ( service.descr )
		return new eServiceMP3(service.path.c_str(), service.descr.c_str() );
	return new eServiceMP3(service.path.c_str());
}

int eServiceHandlerMP3::play(const eServiceReference &service, int workaround )
{
	if ( service.path )
	{
		struct stat64 s;
		if (::stat64(service.path.c_str(), &s))
		{
			if ( service.path.find("://") == eString::npos )
			{
				eDebug("ServiceHandlerMP3: file %s not exist.. don't play", service.path.c_str() );
				return -1;
			}
		}
	}
	else
		return -1;

	runningService = service;
	eService *s = addRef(service);
	if ( s && s->id3 )
		s->id3->getID3Tags();

	if (!workaround)
	{
		decoder=new eMP3Decoder(service.data[0], service.path.c_str(), this);
		decoder->messages.send(eMP3Decoder::eMP3DecoderMessage(eMP3Decoder::eMP3DecoderMessage::start));
	}
	
	if (!decoder->getError())
		state=statePlaying;
	else
		state=stateError;

	flags=flagIsSeekable|flagSupportPosition;

//	if ( service.data[0] != eMP3Decoder::codecMPG )
		flags|=flagIsTrack;

	serviceEvent(eServiceEvent(eServiceEvent::evtStart));
	serviceEvent(eServiceEvent(eServiceEvent::evtFlagsChanged) );

	return 0;
}

int eServiceHandlerMP3::getErrorInfo()
{
	return decoder ? decoder->getError() : -1;
}

int eServiceHandlerMP3::serviceCommand(const eServiceCommand &cmd)
{
	if (!decoder)
	{
		eDebug("ServiceHandlerMP3: no decoder");
		return 0;
	}
	switch (cmd.type)
	{
	case eServiceCommand::cmdSetSpeed:
		if ((state == statePlaying) || (state == statePause) || (state == stateStopped) || (state == stateSkipping))
		{
			if (cmd.parm < 0)
				return -1;
			decoder->messages.send(eMP3Decoder::eMP3DecoderMessage(eMP3Decoder::eMP3DecoderMessage::setSpeed, cmd.parm));
			if (cmd.parm == 0)
				state=statePause;
			else if (cmd.parm == 1)
				state=statePlaying;
			else
				state=stateSkipping;
		} else
			return -2;
		break;
	case eServiceCommand::cmdSeekBegin:
		decoder->messages.send(eMP3Decoder::eMP3DecoderMessage(eMP3Decoder::eMP3DecoderMessage::startSkip));
		break;
	case eServiceCommand::cmdSeekEnd:
		decoder->messages.send(eMP3Decoder::eMP3DecoderMessage(eMP3Decoder::eMP3DecoderMessage::stopSkip));
		break;
	case eServiceCommand::cmdSkip:
		decoder->messages.send(eMP3Decoder::eMP3DecoderMessage(eMP3Decoder::eMP3DecoderMessage::skip, cmd.parm));
		break;
	case eServiceCommand::cmdSeekAbsolute:
		decoder->messages.send(eMP3Decoder::eMP3DecoderMessage(eMP3Decoder::eMP3DecoderMessage::seekreal, cmd.parm));
		if ( cmd.parm == 0 && state == statePause )
			state = stateStopped;
		break;
	case eServiceCommand::cmdSeekReal:
		decoder->messages.send(eMP3Decoder::eMP3DecoderMessage(eMP3Decoder::eMP3DecoderMessage::seekreal, cmd.parm));
		break;
	default:
		return -1;
	}
	return 0;
}

eServiceHandlerMP3::eServiceHandlerMP3(): eServiceHandler(0x1000), messages(eApp, 0)
{
        // eDebug("[eServiceHandlerMP3] registering serviceInterface %d", id);
	if (eServiceInterface::getInstance()->registerHandler(id, this)<0)
		eFatal("couldn't register serviceHandler %d", id);
	CONNECT(eServiceFileHandler::getInstance()->fileHandlers, eServiceHandlerMP3::addFile);
	CONNECT(messages.recv_msg, eServiceHandlerMP3::gotMessage);
	decoder=0;
}

eServiceHandlerMP3::~eServiceHandlerMP3()
{
	eServiceInterface::getInstance()->unregisterHandler(id);
}

void eServiceHandlerMP3::addFile(void *node, const eString &filename)
{
	int codec;
	// eDebug("ServiceHandlerMP3: new stream %s", filename.c_str());
	if (filename.left(7) == "http://")
		eServiceFileHandler::getInstance()->addReference(node, eServiceReference(id, 0, filename));
	else
	{
//		struct stat64 s;
//		if (::stat64(filename.c_str(), &s) || filename.right(7)=="epg.dat")
		if (filename.right(7)=="epg.dat")
			return;
		if (filename.right(4).upper()==".MP3")
	//		codec = eMP3Decoder::codecLAVC;
			codec = eMP3Decoder::codecMP3;
#ifdef HAVE_OGG
		else if (filename.right(4).upper()==".OGG")
			codec = eMP3Decoder::codecOGG;
#endif
#ifdef HAVE_FLAC
		else if (filename.right(5).upper()==".FLAC")
			codec = eMP3Decoder::codecFLAC;
#endif
		else if ((filename.right(5).upper()==".MPEG")
			|| (filename.right(4).upper()==".MPG")
			|| (filename.right(4).upper()==".VOB")
			|| (filename.right(4).upper()==".DAT")
			|| (filename.right(4).upper()==".BIN")
			|| (filename.right(4).upper()==".VDR")
			|| (filename.right(4).upper()==".PVA"))
			codec = eMP3Decoder::codecMPG;
		else 
			return;

		eServiceReference ref(id, 0, filename);
		ref.data[0] = codec;
		eServiceFileHandler::getInstance()->addReference(node, ref);
	}
}

eService *eServiceHandlerMP3::addRef(const eServiceReference &service)
{
	return eServiceFileHandler::getInstance()->addRef(service);
}

void eServiceHandlerMP3::removeRef(const eServiceReference &service)
{
	return eServiceFileHandler::getInstance()->removeRef(service);
}

int eServiceHandlerMP3::getState()
{
	return state;
}

int eServiceHandlerMP3::stop( int workaround )
{
	if ( decoder && !workaround )
	{
		delete decoder;
		removeRef(runningService);
		runningService=eServiceReference();
		decoder=0;
	}
	serviceEvent(eServiceEvent(eServiceEvent::evtStop));
	return 0;
}

int eServiceHandlerMP3::getPosition(int what)
{
	if (!decoder)
		return -1;
	switch (what)
	{
	case posQueryLength:
		return decoder->getLength(0);
	case posQueryCurrent:
		return decoder->getPosition(0);
	case posQueryRealLength:
		return decoder->getLength(1);
	case posQueryRealCurrent:
		return decoder->getPosition(1);
	default:
		return -1;
	}
}

eString eServiceHandlerMP3::getInfo(int id)
{
	if (!decoder)
		return "";
	return decoder->getInfo(id);
}

std::map<eString,eString> &eServiceID3::getID3Tags()
{
	eDebug("[getID3Tags] looking for id3 tags for %s", filename.c_str());
	if ( state == NOTPARSED )
	{
#if 0
		// Get some basic MP3 info first -- this should bne updated by the decoder every sample to get
		// VBR correct. In fact this should be easy as the libmad already collects all the into in its
		// header. Just did not have time to figure how the id3tag structure can be brought down to
		// the codec...
		int myfd = ::open(filename.c_str(), O_RDONLY);
		if (myfd)
		{
			int info;
			if (read(myfd, &info, 4) == 4)
			{
				getMP3info(info, tags);
			}
			::close(myfd);
		}
#endif

		id3_file *file;
		file=::id3_file_open(filename.c_str(), ID3_FILE_MODE_READONLY);
		if (!file)
		{
			eDebug("[getID3Tags] could not process %s for id3 tags", filename.c_str());
			return tags;
		}

		id3_tag *tag=id3_file_tag(file);
		if ( !tag )
		{
			eDebug("[getID3Tags] %s does not seem to have id3 tags", filename.c_str());
			state=NOTEXIST;
			id3_file_close(file);
			return tags;
		}

		struct id3_frame const *frame;
		id3_ucs4_t const *ucs4;
		id3_utf8_t *utf8;

		eDebug("[getID3Tags] %d frames detected", tag->nframes);
		for (unsigned int i=0; i<tag->nframes; ++i)
		{
			frame=tag->frames[i];
			if ( !frame->nfields )
				continue;
			for ( unsigned int fr=0; fr < frame->nfields; fr++ )
			{
				union id3_field const *field;
				field    = &frame->fields[fr];
				if ( field->type != ID3_FIELD_TYPE_STRINGLIST )
					continue;

				unsigned int nstrings = id3_field_getnstrings(field);

				for (unsigned int j = 0; j < nstrings; ++j)
				{
					ucs4 = id3_field_getstrings(field, j);
					ASSERT(ucs4);

					if (strcmp(frame->id, ID3_FRAME_GENRE) == 0)
						ucs4 = id3_genre_name(ucs4);

					utf8 = id3_ucs4_utf8duplicate(ucs4);
					if (utf8 == 0)
						break;

					tags.insert(std::pair<eString,eString>(frame->id, eString((char*)utf8)));
					eDebug("id3: %s = %s", frame->id, (char*)utf8);
					free(utf8);
				}
			}
		}
		id3_file_close(file);
		state=PARSED;
	}
	return tags;
}

eServiceID3::eServiceID3( const eServiceID3 &ref )
	:tags(ref.tags), filename(ref.filename), state(ref.state)
{
}

eServiceMP3::eServiceMP3(const char *filename, const char *descr)
: eService(""), id3tags(filename)
{
	init_eServiceMP3(filename, descr);
}
void eServiceMP3::init_eServiceMP3(const char *filename, const char *descr)
{
//	eDebug("*************** servicemp3.cpp FILENAME: %s", filename);
	if (descr)
	{
		if (!isUTF8(descr))
			service_name=convertLatin1UTF8(descr);
		else
			service_name=descr;
	}

	if (!strncmp(filename, "http://", 7))
	{
		if (!descr)
		{
			if (!isUTF8(filename))
				service_name=convertLatin1UTF8(filename);
			else
				service_name=filename;
		}
		return;
	}

	if ( !descr )
	{
		eString f=filename;
		eString l=f.mid(f.rfind('/')+1);
		if (!isUTF8(l))
			l=convertLatin1UTF8(l);
		service_name=l;
	}

	if (service_name.right(4).upper()==".MP3"
#ifdef HAVE_OGG
	    || service_name.right(4).upper()==".OGG"
#endif
#ifdef HAVE_FLAC
	    || service_name.right(4).upper()==".FLAC"
#endif
		)
		id3 = &id3tags;
}

eServiceMP3::eServiceMP3(const eServiceMP3 &c)
:eService(c), id3tags( c.id3tags )
{
	id3=&id3tags;
}

eAutoInitP0<eServiceHandlerMP3> i_eServiceHandlerMP3(eAutoInitNumbers::service+2, "eServiceHandlerMP3");

#endif //DISABLE_FILE
