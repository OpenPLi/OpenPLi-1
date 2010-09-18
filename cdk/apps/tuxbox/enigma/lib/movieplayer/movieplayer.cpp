/*
 * $Id: movieplayer.cpp,v 1.43 2009/02/03 18:53:43 dbluelle Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
 
#ifdef ENABLE_EXPERT_WEBIF

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <lib/base/buffer.h>
#include <lib/system/econfig.h>
#include <lib/dvb/decoder.h>
#include <lib/movieplayer/mpconfig.h>
#include <lib/movieplayer/movieplayer.h>
#include <src/enigma_main.h>
#include <src/enigma_dyn_utils.h>

#if HAVE_DVB_API_VERSION < 3
#define PVRDEV "/dev/pvr"
#define BLOCKSIZE 65424*4
#define INITIALBUFFER BLOCKSIZE*10
#else
#define PVRDEV "/dev/dvb/adapter0/dvr0"
#define BLOCKSIZE 65424
#define INITIALBUFFER BLOCKSIZE*48
#endif

eIOBuffer tsBuffer(BLOCKSIZE*4);

static pthread_mutex_t mutex = PTHREAD_ADAPTIVE_MUTEX_INITIALIZER_NP;

extern void find_avpids(eIOBuffer *tsBuffer, int *vpid, int *apid, int *ac3);
extern int tcpOpen(eString, int, int);
extern int tcpRequest(int, char *, int);
extern bool playService(const eServiceReference &ref);

pthread_t pvr = 0;
void *pvrThread(void *);
pthread_t receiver = 0;
void *receiverThread(void *);

static int fd = -1;
static int pvrfd = -1;

static int cancelBuffering = 0;

#define CMD "requests/status.xml?command="

eMoviePlayer *eMoviePlayer::instance;

void createThreads()
{
	// create receiver thread
	if (!receiver)
	{
		pthread_create(&receiver, 0, receiverThread, (void *)&fd);
//		eDebug("[MOVIEPLAYER] createThreads: receiver thread created.");
	}
	// create pvr thread
	if (!pvr)
	{
		pvrfd = open(PVRDEV, O_RDWR);
		pthread_create(&pvr, 0, pvrThread, (void *)&pvrfd);
//		eDebug("[MOVIEPLAYER] createThreads: pvr thread created.");
	}
}

void killPVRThread()
{
	if (pvr)
	{	
		pthread_cancel(pvr);
//		eDebug("[MOVIEPLAYER] killThreads: waiting for pvr thread to join.");
		pthread_join(pvr, 0);
		pvr = 0;
//		eDebug("[MOVIEPLAYER] killThreads: pvr thread cancelled.");
	}
}

void killReceiverThread()
{
	if (receiver)
	{
		pthread_cancel(receiver);
//		eDebug("[MOVIEPLAYER] killThreads: waiting for receiver thread to join.");
		pthread_join(receiver, 0);
		receiver = 0;
//		eDebug("[MOVIEPLAYER] killThreads: receiver thread cancelled.");
	}
}

void killThreads()
{
	killPVRThread();
	killReceiverThread();
}

void eMoviePlayer::supportPlugin()
{
	status.PLG = false;
	status.DVB = false;
	status.NSF = false;
	percBuffer = 100;
	initialBuffer = INITIALBUFFER;
	status.BUFFERFILLED = false;
	status.A_SYNC = true;
	status.SUBT = true;
	status.DVDSUBT = true;
	status.RES = false;
	status.STAT = STOPPED;
}

eMoviePlayer::eMoviePlayer(): messages(this, 1)
{
    init_eMoviePlayer();
}
void eMoviePlayer::init_eMoviePlayer()
{
	if (!instance)
		instance = this;
	supportPlugin();
	CONNECT(messages.recv_msg, eMoviePlayer::gotMessage);
	eDebug("[MOVIEPLAYER] Version 2.60 starting...");
	status.ACTIVE = false;
	run();
}

eMoviePlayer::~eMoviePlayer()
{
	killThreads();
	messages.send(Message::quit);
	if (thread_running())
		kill();
	if (instance == this)
		instance = 0;
	status.ACTIVE = false;
}

void eMoviePlayer::thread()
{
	nice(1);
	exec();
}

void eMoviePlayer::startDVB()
{
	playService(suspendedServiceReference);
}

void eMoviePlayer::leaveStreamingClient()
{
	sendRequest2VLC((eString)CMD+"pl_empty");

	pthread_mutex_lock(&mutex);
	tsBuffer.clear();
	pthread_mutex_unlock(&mutex);
	Decoder::Flush();
	status.ACTIVE = false;
	status.STAT = STOPPED;
	status.BUFFERFILLED = false;
	// restore suspended dvb service
	if(!status.DVB)
		startDVB();
	eDebug("[MOVIEPLAYER] leaving...");
}


void eMoviePlayer::control(const char *command, const char *filename)
{
	if (thread_running())
		eDebug("[MOVIEPLAYER] main thread is running.");
	else
		eDebug("[MOVIEPLAYER] main thread is NOT running.");

	eString cmd = eString(command);

	if (cmd.find("start") != eString::npos)
	{
		mpconfig.load();
		server = mpconfig.getServerConfig();
	}

	cancelBuffering = 0;

	eDebug("[MOVIEPLAYER] control: serverIP = %s", server.serverIP.c_str());
	eDebug("[MOVIEPLAYER] control: webifPort = %d", atoi(server.webifPort.c_str()));
	eDebug("[MOVIEPLAYER] control: streamingPort = %d", atoi(server.streamingPort.c_str()));

	eDebug("[MOVIEPLAYER] control: command = %s", command);
	if (cmd == "start")
		messages.send(Message(Message::start, filename ? strdup(filename) : 0));
	else
	if (cmd == "start2")
		messages.send(Message(Message::start2, filename ? strdup(filename) : 0));
	else
	if (cmd == "stop" || cmd == "terminate")
	{
		killThreads();
		pthread_mutex_lock(&mutex);
		cancelBuffering = 1;
		pthread_mutex_unlock(&mutex);

		sendRequest2VLC((eString)CMD+"pl_empty");

		status.ACTIVE = false;
		status.STAT = STOPPED;
		status.BUFFERFILLED = false;
	}
	else
	if (cmd == "play")
		messages.send(Message(Message::play, 0));
	else 
	if (cmd == "pause")
		messages.send(Message(Message::pause, 0));
	else
	if (cmd == "rewind")
		messages.send(Message(Message::rewind, 0));
	else
	if (cmd == "forward")
		messages.send(Message(Message::forward, 0));
	else
	if (cmd == "async")
		messages.send(Message(Message::async, 0));
	else
	if (cmd == "subtitles")
		messages.send(Message(Message::subtitles, 0));
	else
	if (cmd == "dvdsubtitles")
		messages.send(Message(Message::dvdsubtitles, 0));
	else
	if (cmd == "nsf")
		messages.send(Message(Message::nsf, 0));
	else
	if (cmd == "dvbon")
		messages.send(Message(Message::dvbon, 0));
	else
	if (cmd == "dvboff")
		messages.send(Message(Message::dvboff, 0));
	else
	if (cmd == "runplg")
		messages.send(Message(Message::runplg, 0));
	else
	if (cmd == "endplg")
		messages.send(Message(Message::endplg, 0));
	else
	if (cmd == "origres")
		messages.send(Message(Message::origres, 0));
	else
	if (cmd == "bufsize")
		messages.send(Message(Message::bufsize, filename ? strdup(filename) : 0));
	else

	if (cmd == "jump")
		messages.send(Message(Message::jump, filename ? strdup(filename) : 0));
}

int bufferStream(int fd, int bufferSize)
{
	
//	char tempBuffer[BLOCKSIZE];
	char *tempBuffer=NULL;
	if((tempBuffer = (char *)malloc(sizeof(char) * BLOCKSIZE))!=NULL)
	{ 
	    int len = 0;
	    int rc = 1;
	    int error = 0;
	    fd_set rfds;
	    struct timeval tv;
	    int tsBufferSize = 0;

	    eDebug("[MOVIEPLAYER] buffering stream...");

	    // fill buffer and temp file
	    while ((tsBufferSize < bufferSize) && (rc > 0))
	    {
	    	tv.tv_sec = 30; // max. 30sec
	    	tv.tv_usec = 0;
	    	FD_ZERO(&rfds);
	    	FD_SET(fd, &rfds);
	    	rc = select(fd + 1, &rfds, NULL, NULL, &tv);
	    	if (rc)
	    	{
	    		len = recv(fd, tempBuffer, BLOCKSIZE, 0);
	    		if (len < 0)
	    		{
	    			int error = errno;
	    			if (error == EINTR)
	    			{
	    				continue;
	    			}
	    			else if (error == EAGAIN)
	    			{
	    				usleep(100000);
	    				continue;
	    			}
	    			/* all other errors are fatal */
	    			eDebug("[MOVIEPLAYER] bufferStream fatal recv error %d", error);
	    			rc = 0;
	    		}
	    		else if (len == 0)
	    		{
	    			eDebug("[MOVIEPLAYER] bufferStream socket closed");
	    			rc = 0;
	    		}
	    		else
	    		{
	    			error = 0;
	    			tsBuffer.write(tempBuffer, len);
	    			tsBufferSize = tsBuffer.size();
// 	    			eDebug("[MOVIEPLAYER] writing %d bytes to buffer, total: %d", len, tsBufferSize);
	    		}
	    	}
	    	pthread_mutex_lock(&mutex);
	    	if (cancelBuffering > 0)
	    	{
	    		rc = 0;
	    		tsBuffer.clear();
	    		tsBufferSize = 0;
	    	}
	    	pthread_mutex_unlock(&mutex);
	    }
	    free(tempBuffer);
	    return tsBufferSize;
	}
	else
	{
	    eDebug("[MOVIEPLAYER] !!! cannot be allocate buffer in bufferStream !!!");
	    free(tempBuffer);
	    return 0;
	}
}

int eMoviePlayer::requestStream()
{
	char ioBuffer[512];

	eDebug("[MOVIEPLAYER] requesting VLC stream... %s:%s", server.serverIP.c_str(), server.streamingPort.c_str());

	fd = tcpOpen(server.serverIP, atoi(server.streamingPort.c_str()), 10);
	if (fd < 0)
	{
		eDebug("[MOVIEPLAYER] tcpOpen failed.");
		return - 1;
	}
	strcpy(ioBuffer, "GET /dboxstream HTTP/1.0\r\n\r\n");
	if (tcpRequest(fd, ioBuffer, sizeof(ioBuffer) - 1) < 0)
	{
		eDebug("[MOVIEPLAYER] get stream request failed.");
		close(fd);
		return -2;
	}
	if (strstr(ioBuffer, "HTTP/1.0 200 OK") == 0)
	{
		eDebug("[MOVIEPLAYER] 200 OK not received...");
		close(fd);
		return -3;
	}
	else
	{
		eDebug("[MOVIEPLAYER] 200 OK...");
	}
	return 0;
}

void eMoviePlayer::stopDVB()
{
	// save current dvb service for later
	eDVBServiceController *sapi;
	if (sapi = eDVB::getInstance()->getServiceAPI())
		suspendedServiceReference = sapi->service;
	// stop dvb service
	eServiceInterface::getInstance()->stop();
}

int eMoviePlayer::playStream(eString mrl)
{
	int apid = -1, vpid = -1, ac3 = -1;
	
	// receive video stream from VLC on PC
	eDebug("[MOVIEPLAYER] start playing stream...");

	status.BUFFERFILLED = false;
	pthread_mutex_lock(&mutex);
	tsBuffer.clear();
	pthread_mutex_unlock(&mutex);

	if (requestStream() < 0)
	{
		eDebug("[MOVIEPLAYER] requesting stream failed...");
		close(fd);
		return -1;
	}
	int ibuff;
	if(status.PLG)
		ibuff = initialBuffer;
	else
		ibuff = INITIALBUFFER;
	
	if (bufferStream(fd, ibuff) == 0)
	{
		eDebug("[MOVIEPLAYER] buffer is empty.");
		close(fd);
		if (cancelBuffering > 0)
			return 0;
		else
			return -4;
	}

	status.BUFFERFILLED = true;

	find_avpids(&tsBuffer, &vpid, &apid, &ac3);
	if (vpid == -1 || apid == -1)
	{
		if(status.NSF ) // plugin can playback file without A or V too
		{
			eDebug("[MOVIEPLAYER] both AV pids not found: vpid %d apid %d, but play.",vpid,apid);
		}
		else
		{
			eDebug("[MOVIEPLAYER] no AV pids found.");
			close(fd);
			return -5;
		}
	}
	status.NSF=false;

	status.AVPIDS_FOUND = true;

	if(!status.DVB)
		stopDVB();

	Decoder::setMpegDevice(-1);
	// set pids
	Decoder::parms.vpid = vpid;
	Decoder::parms.apid = apid;
	Decoder::parms.audio_type = DECODE_AUDIO_MPEG;

	int ac3default = 0;
	eConfig::getInstance()->getKey("/elitedvb/audio/ac3default", ac3default);
	if (ac3 && ac3default)
	{
		if (mrl.right(3) == "vob" || mrl.left(3) == "dvd")
			Decoder::parms.audio_type = DECODE_AUDIO_AC3_VOB;
		else
			Decoder::parms.audio_type = DECODE_AUDIO_AC3;
	}

	eZapMain::getInstance()->hideInfobar();
	usleep(100000);
	Decoder::SetStreamType(TYPE_PES);
	Decoder::Set();

#ifndef DISABLE_LCD
	unsigned int pos = mrl.find_last_of('/');
	eZapLCD::getInstance()->lcdMain->ServiceName->setText(mrl.right(mrl.size() - pos - 1));
#endif

	createThreads();

	status.ACTIVE = true;

	return 0;
}

void eMoviePlayer::setErrorStatus()
{
	status.ACTIVE = false;
	status.STAT = STREAMERROR;
	status.BUFFERFILLED = false;
}

void eMoviePlayer::gotMessage(const Message &msg )
{
	eString mrl;
	eString command = CMD;

	eDebug("[MOVIEPLAYER] message %d coming in...", msg.type);
	switch (msg.type)
	{
		case Message::start:
		case Message::start2:
		{
			status.STAT = PLAY;
			killThreads();
			if (msg.filename)
			{
				mrl = eString(msg.filename);
				eDebug("[MOVIEPLAYER] mrl = %s", mrl.c_str());
				free((char*)msg.filename);
				
				eDebug("[MOVIEPLAYER] Server IP: %s", server.serverIP.c_str());
				eDebug("[MOVIEPLAYER] Server Port: %d", atoi(server.streamingPort.c_str()));
				
				if (msg.type == Message::start2)
				{
					
					// empty vlc's playlist
					if (int res = sendRequest2VLC(command + "pl_empty") < 0)
					{
						eDebug("[MOVIEPLAYER] couldn't communicate with vlc, streaming server ip address may be wrong in settings. Errno: %d",res);
						setErrorStatus();
						break;
					}
					usleep(200000);
					// vlc: set sout...
					if (sendRequest2VLC("?sout=" + httpEscape(sout(mrl))) < 0) 
					{
						setErrorStatus();
						break;
					}
					usleep(200000);
					// vlc: add mrl to playlist	and play
					if (sendRequest2VLC(command + "in_play&input=" + httpEscape(mrl).strReplace("%5c","%5c%5c")) < 0)
					{
						setErrorStatus();
						break;
					}
				}
				usleep(200000);
				// receive and play ts stream
				// initialBuffer = INITIALBUFFER;  // for future for plugin 
				if (playStream(mrl) < 0)
				{
					setErrorStatus();
					sendRequest2VLC(command + "pl_empty"); // dont streaming, stop VLC
					break;
				}
			}
			break;
		}
		case Message::pause:
			status.STAT = PAUSE;
			break;
		case Message::play:
			status.STAT = PLAY;
			break;
		case Message::forward:
			break;
		case Message::rewind:
			break;
		case Message::async:
		        status.A_SYNC=false;
			break;
		case Message::subtitles:
		        status.SUBT=false;
			break;
		case Message::dvdsubtitles:
		        status.DVDSUBT=false;
			break;
		case Message::nsf:
		        status.NSF=true;
			break;
		case Message::dvbon:
		        status.DVB=false;
			break;
		case Message::dvboff:
		        status.DVB=true;
			break;
		case Message::runplg:
		        status.PLG=true;
			break;
		case Message::endplg:
		        status.PLG=false;
			break;
		case Message::origres:
		        status.RES=true;
			break;
		case Message::bufsize:
		{
			percBuffer = atoi(msg.filename);
//			eDebug("### percBuffer %d", percBuffer);
			initialBuffer = (int)( INITIALBUFFER/100. * percBuffer );
			break;
		}
		case Message::jump:
		{
			int jump = atoi(eString(msg.filename).c_str());
//			eDebug("[MOVIEPLAYER] jump: %d seconds", jump);
			break;
		}
		case Message::quit:
		{
			quit(0);
			break;
		}
		default:
			eDebug("unhandled thread message");
	}
	eDebug("[MOVIEPLAYER] message %d handled.", msg.type);
}

int eMoviePlayer::sendRequest2VLC(eString command)  // sending tcp commands 	 
{  	 	 
    char ioBuffer[512];  	 	 
	int rc = -1;  	 	 
	int fd = tcpOpen(server.serverIP, atoi(server.webifPort.c_str()), 1);  	 	 
	if (fd > 0)  	 	 
	{  	 	 
	    eString url = "GET /" + command + " HTTP/1.1\r\n\r\n";  	 	 
	    strcpy(ioBuffer, url.c_str());  	 	 
    	//eDebug("[MOVIEPLAYER] sendRequest2VLC : %d, %s", fd, ioBuffer);  	 	 
	    rc = tcpRequest(fd, ioBuffer, sizeof(ioBuffer) - 1);	 	 
	    if (rc == 0)  	 	 
	    {  	 	 
	        if (strstr(ioBuffer, "HTTP/1.1 200 OK") == 0)  	 	 
	        {  	
    			//eDebug("[MOVIEPLAYER] sendRequest2VLC return: %s", ioBuffer);
				eDebug("[MOVIEPLAYER] 200 OK NOT received...");  
//				if (strstr(ioBuffer, "403 Forbidden") != 0)
// 					eDebug("[MOVIEPLAYER] is not enabled IP in .host !");  	 
	            rc = -2;  	 	 
	        }  	 	 
	        else   	 	 
	        {  	 	 
	            eDebug("[MOVIEPLAYER] 200 OK...");  	 	 
	        }  	 	 
	    }  	 	 
	    else   	 	 
	        rc = -3;  	 	 
	    close(fd);  	 	 
	}  	 	 
	return rc;  	 	 
}


eString eMoviePlayer::sout(eString mrl)
{
	eString soutURL = "#";
	
	unsigned int pos = mrl.find_last_of('.');
	eString extension = mrl.right(mrl.length() - pos - 1);
	
	eString name = "File";
	if (mrl.find("dvdsimple:") != eString::npos)
	{
		name = "DVD";
		extension = "NONE";
	}
	else
	if (mrl.find("vcd:") != eString::npos)
	{
		name = "VCD";
		extension = "NONE";
	}
	
	struct serverConfig server = mpconfig.getServerConfig();
	struct videoTypeParms video = mpconfig.getVideoParms(name, extension);
	
	eDebug("[MOVIEPLAYER] determine ?sout for mrl: %s", mrl.c_str());
	eDebug("[MOVIEPLAYER] transcoding audio: %d, video: %d, subtitles: %d, dvbs: %d" , video.transcodeAudio, video.transcodeVideo, status.PLG ? status.SUBT : video.soutadd,status.DVDSUBT);

	// add sout (URL encoded)
	// example (with transcode to mpeg1):
	//  ?sout=#transcode{vcodec=mpgv,vb=2000,acodec=mpga,ab=192,channels=2}:duplicate{dst=std{access=http,mux=ts,url=:8080/dboxstream}}
	// example (without transcode to mpeg1): 
	// ?sout=#duplicate{dst=std{access=http,mux=ts,url=:8080/dboxstream}}

	pos = video.videoRatio.find("x");
	eString res_horiz = video.videoRatio.left(pos);
	eString res_vert = video.videoRatio.right(video.videoRatio.length() - pos - 1);
	eDebug("[MOVIEPLAYER] res_horiz = %s, res_vert = %s", res_horiz.c_str(), res_vert.c_str());
	
	if (video.transcodeVideo || video.transcodeAudio)
	{
		soutURL += "transcode{";
		if (video.transcodeVideo)
		{
			soutURL += "vcodec=" + video.videoCodec;
			soutURL += ",vb=" + video.videoRate;
			
			if(status.PLG && status.RES)
			{
			    int w = 0; 
			    eConfig::getInstance()->getKey("/enigma/plugins/movieplayer/w", w);
			    int h = 0;
			    eConfig::getInstance()->getKey("/enigma/plugins/movieplayer/h", h);
	            
	            if(w)
	                soutURL += ",width=" + eString().sprintf("%d",w);
	            else
			        soutURL += ",width=";
			    
			    if(h)
			        soutURL += ",height=" + eString().sprintf("%d",h); 
			    else
			        soutURL += ",height="; 
			    eDebug("used original resolution h:%d x w:%d",h,w);
			}
			else
			{    
			    soutURL += ",width=" + res_horiz;
			    soutURL += ",height=" + res_vert;
			}
			
			soutURL += ",fps=" + video.fps;
			
			if(status.PLG)
			{
				if(status.SUBT)
					soutURL += ",soverlay";
			}
			else
			{
				if (video.soutadd)
					soutURL += ",soverlay";
			}
		}
						
		if (video.transcodeAudio)
		{
			if (video.transcodeVideo)
				soutURL += ",";
			soutURL += "acodec=mpga,ab=" + video.audioRate + ",channels=2";
		}
		
		if(status.PLG)
		{
		    if(status.DVDSUBT)
				soutURL += ",scodec=dvbs";
		}
				
		if(status.PLG)
		{
			if(status.A_SYNC)
				soutURL += ",audio-sync";
		}
		else
			soutURL += ",audio-sync";
			
		soutURL += "}:";
	}
	
	//soutURL += "duplicate{dst=std{access=http,mux=ts,dst=:" + server.streamingPort + "/dboxstream}}";
	soutURL += "standard{access=http,mux=ts,dst=:" + server.streamingPort + "/dboxstream}";

	status.A_SYNC=true;
	status.SUBT = true;
	status.DVDSUBT = true;
	status.RES = false;
   	eDebug("[MOVIEPLAYER] sout = %s", soutURL.c_str());
	return soutURL;
}

void pvrThreadCleanup(void *pvrfd)
{
	close(*(int *)pvrfd);
	killReceiverThread();
	eMoviePlayer::getInstance()->leaveStreamingClient();
	eDebug("[MOVIEPLAYER] pvrThreadCleanup done.");
}

void *pvrThread(void *pvrfd)
{
//	char tempBuffer[BLOCKSIZE];
	char *tempBuffer=NULL;
	if((tempBuffer = (char *)malloc(sizeof(char) * BLOCKSIZE))!=NULL)
	{ 
	    int rd = 0;
    	int tsBufferSize = 0;
	    int errors = 0;
    	eDebug("[MOVIEPLAYER] pvrThread starting: pvrfd = %d", *(int *)pvrfd);
    	pthread_cleanup_push(pvrThreadCleanup, (void *)pvrfd);
	    nice(-1);
	    while (true)
	    {
	    	pthread_testcancel();
	    	pthread_mutex_lock(&mutex);
	    	rd = tsBuffer.read(tempBuffer, BLOCKSIZE);
	    	tsBufferSize = tsBuffer.size();
	    	pthread_mutex_unlock(&mutex);
	    	if (rd > 0)
	    	{
	    		errors = 0;
	    		while (1)
	    		{
	    			int result;
	    			result = write(*(int *)pvrfd, tempBuffer, rd);
	    			if (result < 0)
	    			{
	    				int error = errno;
	    				if (error == EINTR)
	    				{
	    					continue;
	    				}
	    				else if (error == EAGAIN)
	    				{
	    					usleep(100000);
	    					continue;
	    				}
	    				else
	    				{
	    					/* all other errors are fatal */
	    					eDebug("[MOVIEPLAYER] fatal pvr write error occurred %d", error);
	    				}
	    			}
	    			break;
	    		}
	    		// eDebug("[MOVIEPLAYER] %d >>> writing %d bytes to pvr...", tsBufferSize, rd); - docasne
	    	}
	    	else
	    	{
	    		if (tsBufferSize == 0)
	    		{
	    			if (++errors > 100)
	    			{
	    				eDebug("[MOVIEPLAYER] pvrThread: exit after %d attempts reading from empty buffer", errors);
	    				break;
	    			}
	    			/* wait a bit for new data to arrive */
	    			usleep(100000);
	    		}
	    		else
	    		{
	    			/* fatal error, should never happen, stop at once */
	    			eDebug("[MOVIEPLAYER] pvrThread: fatal error: failed to read from nonempty buffer");
	    			break;
	    		}
	    	}
	    }
	    pthread_exit(NULL);
	    pthread_cleanup_pop(1);
	}
	else
	{
	    eDebug("[MOVIEPLAYER] !!! cannot be allocate buffer in pvrThread !!!");
	}
	free(tempBuffer);
}

void receiverThreadCleanup(void *fd)
{
	close(*(int *)fd);
	eDebug("[MOVIEPLAYER] receiverThreadCleanup done.");
}

void *receiverThread(void *fd)
{
//	char tempBuffer[BLOCKSIZE];
	char *tempBuffer=NULL;
	if((tempBuffer = (char *)malloc(sizeof(char) * BLOCKSIZE))!=NULL)
	{ 
	    int len = 0;
	    int tsBufferSize = 0;
	
	    eDebug("[MOVIEPLAYER] receiverThread starting: fd = %d", *(int *)fd);
	    pthread_cleanup_push(receiverThreadCleanup, (void *)fd);
	    nice(-1);
	    // fill buffer
	    while (true)
	    {
	    	pthread_testcancel();
	    	pthread_mutex_lock(&mutex);
	    	tsBufferSize = tsBuffer.size();
	    	pthread_mutex_unlock(&mutex);
	    	if (tsBufferSize < INITIALBUFFER)
	    	{
	    		while (1)
	    		{
	    			len = recv(*(int *)fd, tempBuffer, BLOCKSIZE, 0);
	    			if (len < 0)
	    			{
	    				int error = errno;
	    				if (error == EINTR)
	    				{
	    					continue;
	    				}
	    				else if (error == EAGAIN)
	    				{
	    					usleep(100000);
	    					continue;
	    				}
	    				/* all other errors are fatal */
	    				eDebug("[MOVIEPLAYER] fatal recv error %d", error);
	    			}
	    			else if (len == 0)
	    			{
	    				eDebug("[MOVIEPLAYER] socket closed");
	    			}
	    			break;
	    		}
//	    		eDebug("[MOVIEPLAYER] %d <<< writing %d bytes to buffer...", tsBufferSize, len);
	    		if (len > 0)
	    		{
	    			pthread_mutex_lock(&mutex);
	    			tsBuffer.write(tempBuffer, len);
	    			pthread_mutex_unlock(&mutex);
	    		}
	    		else
	    		{
	    			/* socket closed, or fatal error */
	    			break;
	    		}
	    	}
	    	else
	    	{
	    		/* buffer full, wait a bit */
	    		usleep(200000);
	    	}
	    }
	    pthread_exit(NULL);
	    pthread_cleanup_pop(1);
	}
	else
	{
	    eDebug("[MOVIEPLAYER] !!! cannot be allocate buffer in receiverThread !!!");
	}
	free(tempBuffer);
}
#endif
