#include <config.h>
#if HAVE_DVB_API_VERSION < 3
#include <ost/dmx.h>
#include <ost/video.h>
#include <ost/audio.h>
#define VIDEO_DEV "/dev/dvb/card0/video0"
#define AUDIO_DEV "/dev/dvb/card0/audio0"
#define DEMUX_DEV "/dev/dvb/card0/demux0"
typedef struct ca_pid {
        unsigned int pid;
        int index;              /* -1 == disable*/
} rdn_ca_pid_t;
#define RDN_CA_SET_PID      _IOW('o', 135, rdn_ca_pid_t)
#else
#include <linux/dvb/dmx.h>
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#define VIDEO_DEV "/dev/dvb/adapter0/video0"
#define AUDIO_DEV "/dev/dvb/adapter0/audio0"
#define DEMUX_DEV "/dev/dvb/adapter0/demux0"
#define audioStatus audio_status
#define videoStatus video_status
#define pesType pes_type
#define playState play_state
#define audioStreamSource_t audio_stream_source_t
#define videoStreamSource_t video_stream_source_t
#define streamSource stream_source
#define dmxPesFilterParams dmx_pes_filter_params
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <lib/dvb/servicedvb.h>
#include <lib/dvb/record.h>

// #define OLD_VBI

#undef strcpy
#undef strcmp
#undef strlen
#undef strncmp

#ifdef OLD_VBI  // oldvbi header
#include <dbox/avia_gt_vbi.h>
#endif

typedef unsigned char __u8;

#include <lib/system/econfig.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvbservice.h>
#include <lib/base/eerror.h>

#ifndef TUXTXT_CFG_STANDALONE
extern "C" int  tuxtxt_init();
extern "C" int  tuxtxt_start(int tpid);
extern "C" int  tuxtxt_stop();
extern "C" void tuxtxt_close();
#endif

decoderParameters Decoder::current;
decoderParameters Decoder::parms;
int Decoder::fd::video;
int Decoder::fd::audio;
int Decoder::fd::demux_video;
int Decoder::fd::demux_audio;
int Decoder::fd::demux_pcr;
int Decoder::fd::demux_vtxt;
int Decoder::fd::mpeg;
int Decoder::locked=0;

int rdn_addPid(int fd_ca, int index, int pid) {
	rdn_ca_pid_t ca_pid;
	ca_pid.index = index;
	ca_pid.pid = pid;
	printf ( "Adding pid to descrambler: %x\n", pid);
	return ioctl(fd_ca, RDN_CA_SET_PID, &ca_pid);
}


static void SetECM(int vpid, int apid, int pmtpid, int descriptor_length, __u8 *descriptors)
{
#if 0
// to use this old code you should read my comment in addCADescriptor..
	if ( eDVB::getInstance()->recorder && eServiceInterface::getInstance()->service.path )
		return;

	eDebug("-------------------Set ECM-----------------");
	static int lastpid=-1;

	if (lastpid != -1)
	{
		kill(lastpid, SIGKILL);
		waitpid(lastpid, 0, 0);
		lastpid=-1;
	}

	if (!descriptor_length)
		return;

	char buffer[3][5];
	sprintf(buffer[0], "%x", vpid);
	sprintf(buffer[1], "%x", apid);
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (!sapi)
		return;
	sprintf(buffer[2], "%x", sapi->service.getServiceID().get());

	char descriptor[2048];

	for (int i=0; i<descriptor_length; i++)
		sprintf(descriptor+i*2, "%02x", descriptors[i]);

	switch (lastpid=fork())
	{
	case -1:
		eDebug("fork failed!");
		return;
	case 0:
	{
#if 0
		close(0);
		close(1);
		close(2);
#endif
		for (unsigned int i=3; i < 90; ++i )
			close(i);

		for (int i=0; i<=2; i++)
		{
			char qqq[24];
			sprintf (qqq, "/dev/dvb/card0/ca%d", i);
                	int rdn_fd = open ( qqq, O_RDWR );
			if (rdn_fd>0)
			{
				rdn_addPid(rdn_fd, 0, vpid);
				rdn_addPid(rdn_fd, 0, apid);
				close(rdn_fd);
			}
		}

		if (execlp("camd", "camd", buffer[0], buffer[1], buffer[2], descriptor, 0)<0)
			eDebug("camd");

		_exit(0);
		break;
	}
	}
#endif
}

int Decoder::Initialize()
{
	parms.vpid = parms.apid = parms.tpid = parms.pcrpid = parms.pmtpid = -1;
	parms.audio_type=0;
	parms.descriptor_length=0;
	parms.restart_camd=0;
	current=parms;
	fd.video = fd.audio = fd.demux_video = fd.demux_audio = fd.demux_pcr = fd.demux_vtxt = fd.mpeg = -1;
#ifndef TUXTXT_CFG_STANDALONE
	int enable_background_caching=0;
	eConfig::getInstance()->getKey("/ezap/extra/teletext_caching", enable_background_caching);
	if ( enable_background_caching )
		tuxtxt_init();
#endif
	return 0;
}

void Decoder::Close()
{
	Flush();
#ifndef TUXTXT_CFG_STANDALONE
	int enable_background_caching=0;
	eConfig::getInstance()->getKey("/ezap/extra/teletext_caching", enable_background_caching);
	if ( enable_background_caching )
		tuxtxt_close();
#endif
	eDebug("fd video = %d, audio = %d, demux_video = %d, demux_audio = %d, demux_pcr = %d, demux_vtxt = %d, mpeg = %d",
					fd.video, fd.audio, fd.demux_video, fd.demux_audio, fd.demux_pcr, fd.demux_vtxt, fd.mpeg);
}

void Decoder::Flush(int keepaudiotype)
{
//	eDebug("Decoder::Flush()");
	parms.vpid = parms.apid = parms.tpid = parms.pcrpid = parms.pmtpid = -1;
	if (!keepaudiotype)
		parms.audio_type=0;
	parms.descriptor_length=parms.restart_camd=0;
	Set();
}
void Decoder::Flush() // for Binary compatibility
{ 
	Flush(0); 
} 
void Decoder::Pause( int flags )
// flags & 1 = disableAudio
// flags & 2 = onlyFreeze
{
	eDebug("Decoder::Pause()");
	if (fd.video != -1)
	{
		if ( ::ioctl(fd.video, VIDEO_FREEZE) < 0 )
			eDebug("VIDEO_FREEZE failed (%m)");
		if ( flags & 2 )
			return;
		if ( fd.audio != -1 )
		{
#if HAVE_DVB_API_VERSION >= 3
			if (::ioctl(fd.audio, AUDIO_STOP)<0)
				eDebug("AUDIO_STOP failed(%m)");
			else
				eDebug("audio_pause (success)");
#else
			if ( ::ioctl(fd.audio, AUDIO_SET_AV_SYNC, 0) < 0 )
				eDebug("AUDIO_SET_AV_SYNC failed (%m)");
			if ( flags & 1 )
			{
				if ( ::ioctl(fd.audio, AUDIO_SET_MUTE, 1 )<0)
					eDebug("AUDIO_SET_MUTE failed (%m)");
			}
#endif
		}
	}
}

void Decoder::Resume(bool enableAudio)
{
	eDebug("Decoder::Resume()");
	if (fd.video != -1)
	{
//		clearScreen();
#if HAVE_DVB_API_VERSION >= 3
		if (::ioctl(fd.video, VIDEO_STOP)<0)
			eDebug("VIDEO_STOP failed(%m)");
		if (::ioctl(fd.video, VIDEO_PLAY)<0)
			eDebug("VIDEO_PLAY failed(%m)");
#else
		if (::ioctl(fd.video, VIDEO_CONTINUE)<0)
			eDebug("VIDEO_CONTINUE failed(%m)");
		if ( ::ioctl(fd.audio, AUDIO_SET_AV_SYNC, 1 ) < 0 )
			eDebug("AUDIO_SET_AV_SYNC failed (%m)");
#endif
		if ( enableAudio )  // Video Clip Mode
		{
#if HAVE_DVB_API_VERSION >= 3
			if (::ioctl(fd.audio, AUDIO_PLAY)<0)
				eDebug("AUDIO_PLAY failed (%m)");
			else
				eDebug("audio continue (success)");
#else
			if (::ioctl(fd.audio, AUDIO_SET_MUTE, 0 )<0)
				eDebug("AUDIO_SET_MUTE failed (%m)");
			else
				eDebug("audio_pause (success)");
#endif
		}
	}
}

void Decoder::flushBuffer()
{
	if (fd.video != -1 && ::ioctl(fd.video, VIDEO_CLEAR_BUFFER)<0 )
		eDebug("VIDEO_CLEAR_BUFFER failed (%m)");
	if (fd.audio != -1 && ::ioctl(fd.audio, AUDIO_CLEAR_BUFFER)<0 )
		eDebug("AUDIO_CLEAR_BUFFER failed (%m)");
}

void Decoder::SetStreamType(int type)
{
	uint val=0;
	switch ( type )
	{
		case TYPE_ES:
			val=0;
			break;
		case TYPE_PES:
			val=1;
			break;
		case TYPE_MPEG1:
			val=2;
			break;
	}
	if (fd.audio != -1 && ::ioctl(fd.audio, AUDIO_SET_STREAMTYPE, val)<0 )
		eDebug("AUDIO_SET_STREAMTYPE failed (%m)");
}

int Decoder::Set()
{
	if (locked)
		return -1;

	int changed=0;

	if (parms.vpid != current.vpid)
		changed |= 1;
	if (parms.apid != current.apid)
		changed |= 2;
	if (parms.tpid != current.tpid)
		changed |= 4;
	if (parms.pcrpid != current.pcrpid)
		changed |= 8;
	if (parms.pmtpid != current.pmtpid)
		changed |= 0x40;
	if (parms.descriptor_length != current.descriptor_length)
		changed |= 0x80;
	if (parms.audio_type != current.audio_type)
		changed |= 0x100;

	eDebug(" ------------> changed! %x", changed);
	if (!changed)
		return 0;

#ifndef TUXTXT_CFG_STANDALONE
	int enable_background_caching=0;
	eConfig::getInstance()->getKey("/ezap/extra/teletext_caching", enable_background_caching);
#endif

	dmxPesFilterParams pes_filter;

	if (changed & 0xC7 || parms.restart_camd)
	{
		// SetECM(parms.vpid, parms.apid, parms.pmtpid, parms.descriptor_length, parms.descriptors);
		parms.restart_camd=0;
	}

	if ( changed & 11 )
	{
		if (fd.demux_audio == -1)
		{
			fd.demux_audio=open(DEMUX_DEV, O_RDWR);
			if (fd.demux_audio<0)
				eDebug("fd.demux_audio couldn't be opened");
/*			else
				eDebug("fd.demux_audio opened");*/
		}

		if ( fd.audio == -1 )  // open audio dev... if not open..
		{
			fd.audio=open(AUDIO_DEV, O_RDWR);
			if (fd.audio<0)
			eDebug("fd.audio couldn't be opened");
/*			else
				eDebug("fd.audio opened");*/
		}

		// get audio status
		audioStatus astatus;

		eDebugNoNewLine("AUDIO_GET_STATUS - ");
		if (::ioctl(fd.audio, AUDIO_GET_STATUS, &astatus)<0)
			eDebug("failed (%m)");
		else
			eDebug("%s", astatus.playState == AUDIO_STOPPED ? "stopped" : astatus.playState == AUDIO_PAUSED ? "paused" : "playing" );

		// DEMUX STOP AUDIO
		if ( astatus.playState != AUDIO_STOPPED /* current.apid != -1 */ )
		{
			eDebugNoNewLine("DEMUX_STOP - audio - ");
			if (::ioctl(fd.demux_audio, DMX_STOP)<0)
				eDebug("failed (%m)");
			else
				eDebug("ok");
		}

		if ( (changed & 0x0F) != 2 )  // only apid changed
		{
			if ( fd.video == -1 ) // open video dev... if not open
			{
				fd.video=open(VIDEO_DEV, O_RDWR);
				if (fd.video<0)
					eDebug("fd.video couldn't be opened");
/*			else
					eDebug("fd.video opened");*/
			}

			// get video status
			videoStatus vstatus;
			eDebugNoNewLine("VIDEO_GET_STATUS - ");
			if (::ioctl(fd.video, VIDEO_GET_STATUS, &vstatus)<0)
				eDebug("failed (%m)");
			else
				eDebug("%s", vstatus.playState == VIDEO_STOPPED ? "stopped" : vstatus.playState == VIDEO_PLAYING ? "playing" : "freezed" );

			// DEMUX STOP VIDEO
			if (fd.demux_video == -1)
			{
				fd.demux_video=open(DEMUX_DEV, O_RDWR);
				if (fd.demux_video<0)
					eDebug("fd.demux_video couldn't be opened");
/*			else
					eDebug("fd.demux_video opened");*/
			}
			if ( vstatus.playState != VIDEO_STOPPED /*current.vpid != -1*/ )
			{
				eDebugNoNewLine("DEMUX_STOP - video - ");
				if (::ioctl(fd.demux_video, DMX_STOP)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}
			// DEMUX STOP PCR
			if (fd.demux_pcr == -1)
			{
				fd.demux_pcr=open(DEMUX_DEV, O_RDWR);
				if (fd.demux_pcr<0)
					eDebug("fd.demux_pcr couldn't be opened");
/*			else
					eDebug("fd.demux_pcr opened");*/
			}
			if ( current.pcrpid != -1 )
			{
				eDebugNoNewLine("DEMUX_STOP - pcr - ");
				if (::ioctl(fd.demux_pcr, DMX_STOP)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}

			if ( astatus.playState != AUDIO_STOPPED /* current.apid != -1*/ )
			{
				eDebugNoNewLine("AUDIO_STOP - ");
				if (::ioctl(fd.audio, AUDIO_STOP)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}

			if ( parms.apid != -1 )
			{
				audioStreamSource_t n =
					( parms.vpid != -1 && parms.pcrpid == -1) ?
						AUDIO_SOURCE_MEMORY : AUDIO_SOURCE_DEMUX;
				if ( astatus.streamSource != n )
				{
					eDebugNoNewLine("AUDIO_SELECT_SOURCE - ");
					if (::ioctl(fd.audio, AUDIO_SELECT_SOURCE, n)<0)
						eDebug("failed (%m)");
					else
						eDebug("ok");
				}
			}

			if ( vstatus.playState != VIDEO_STOPPED ) //*/ current.vpid != -1 )
			{
				eDebugNoNewLine("VIDEO_STOP - ");
				if (::ioctl(fd.video, VIDEO_STOP)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}

			if ( parms.vpid != -1 )
			{
				videoStreamSource_t n =
					( parms.apid != -1 && parms.pcrpid == -1 ) ?
							VIDEO_SOURCE_MEMORY : VIDEO_SOURCE_DEMUX;
				if ( vstatus.streamSource != n )
				{
					eDebugNoNewLine("VIDEO_SELECT_SOURCE - ");
					if (::ioctl(fd.video, VIDEO_SELECT_SOURCE, n)<0)
						eDebug("failed (%m)");
					else
						eDebug("ok");
				}
			}

////////////////////////// DEMUX_VIDEO SET FILTER /////////////////////////
			if ( parms.vpid != -1 )
			{
				pes_filter.pid		 = parms.vpid;
				pes_filter.input	 = DMX_IN_FRONTEND;
				pes_filter.output	 = DMX_OUT_DECODER;
				pes_filter.pesType = DMX_PES_VIDEO;
				pes_filter.flags	 = 0;
				eDebugNoNewLine("DMX_SET_PES_FILTER(0x%02x) - video - ", parms.vpid);
				if (::ioctl(fd.demux_video, DMX_SET_PES_FILTER, &pes_filter)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}
			else  // we have no vpid
			{
				close(fd.demux_video);
				fd.demux_video=-1;
//				eDebug("fd.demux_video closed");
			}

////////////////////////// DEMUX_PCR SET FILTER /////////////////////////
			if ( parms.pcrpid != -1 )
			{
				pes_filter.pid=parms.pcrpid;
				pes_filter.input=DMX_IN_FRONTEND;
				pes_filter.output=DMX_OUT_DECODER;
				pes_filter.pesType=DMX_PES_PCR;
				pes_filter.flags=0;

				eDebugNoNewLine("DMX_SET_PES_FILTER(0x%02x) - pcr - ",parms.pcrpid);
				if (::ioctl(fd.demux_pcr, DMX_SET_PES_FILTER, &pes_filter)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}
			else  // we have no pcrpid
			{
				close(fd.demux_pcr);
				fd.demux_pcr=-1;
//				eDebug("fd.demux_pcr closed");
			}
		}
	}

	if (changed & 0x100 && parms.audio_type != current.audio_type )
	{
		int bypass=0;
		switch (parms.audio_type)
		{
		case DECODE_AUDIO_MPEG:
			bypass=1;
			break;
		case DECODE_AUDIO_AC3:
			bypass=0;
			break;
		case DECODE_AUDIO_AC3_VOB:
			bypass=3;
			break;
		case DECODE_AUDIO_DTS:
			bypass=2;
			break;
		}
		eDebugNoNewLine("AUDIO_SET_BYPASS_MODE to %d - ", bypass);
		int wasclosed = 0;
		if ( fd.audio == -1 )  // open audio dev... if not open..
		{
			fd.audio=open(AUDIO_DEV, O_RDWR);
			if (fd.audio<0)
				eDebug("fd.audio couldn't be opened");
/*			else
				eDebug("fd.audio opened");*/
			wasclosed = 1;
		}
		if (::ioctl( fd.audio , AUDIO_SET_BYPASS_MODE, bypass ) < 0)
			eDebug("failed (%m)");
		else
			eDebug("ok");
		if ( wasclosed && fd.audio != -1 )  // audio dev was closed before
		{
			close(fd.audio);
			fd.audio = -1;
//			eDebug("fd.audio closed");
		}
		
	}

	if (changed & 11)
	{
////////////////////////// DEMUX AUDIO SET FILTER /////////////////////////
		if ( parms.apid != -1 )
		{
			pes_filter.pid		 = parms.apid;
			pes_filter.input	 = DMX_IN_FRONTEND;
			pes_filter.output	 = DMX_OUT_DECODER;
			pes_filter.pesType = DMX_PES_AUDIO;
			pes_filter.flags	 = 0;
			eDebugNoNewLine("DMX_SET_PES_FILTER(0x%02x) - audio - ", parms.apid);
			if (::ioctl(fd.demux_audio, DMX_SET_PES_FILTER, &pes_filter)<0)
				eDebug("failed (%m)");
			else
				eDebug("ok");
		}
		else  // we have no apid
		{
			close(fd.demux_audio);
			fd.demux_audio=-1;
//			eDebug("fd.demux_audio closed");
		}
		if ( (changed & 0x0F) != 2 )  //  only apid changed
		{
			if ( parms.pcrpid != -1 )
			{
				eDebugNoNewLine("DMX_START (pcr) - ");
				if (::ioctl(fd.demux_pcr, DMX_START)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}
			else if ( parms.apid != -1 && parms.vpid != -1 )
			{
				eDebugNoNewLine("enabling av sync mode - ");
				if (::ioctl(fd.audio, AUDIO_SET_AV_SYNC, 1)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}
			if ( parms.vpid != -1 )
			{
				eDebugNoNewLine("DMX_START (video) - ");
				if (::ioctl(fd.demux_video, DMX_START)<0)
					eDebug("failed (%m)");
				else
						eDebug("ok");
			}
			if ( parms.vpid != -1 )
			{
				eDebugNoNewLine("VIDEO_PLAY - ");
				if (::ioctl(fd.video, VIDEO_PLAY)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}
			else  // no vpid
			{
				eDebug("fd.video closed");
				close(fd.video);
				fd.video = -1;
			}
			if ( parms.apid != -1 )
			{
				eDebugNoNewLine("AUDIO_PLAY - ");
				if (::ioctl(fd.audio, AUDIO_PLAY)<0)
					eDebug("failed (%m)");
				else
					eDebug("ok");
			}
			else  // no apid
			{
				close(fd.audio);
				fd.audio = -1;
//				eDebug("fd.audio closed");
			}
		}

		if ( parms.apid != -1 )
		{
			eDebugNoNewLine("DMX_START (audio) - ");
			if (::ioctl(fd.demux_audio, DMX_START)<0)
				eDebug("failed (%m)");
			else
				eDebug("ok");
		}
	}

	if (changed & 4)
	{
#ifdef OLD_VBI // for old drivers in alexW Image...
		// vtxt reinsertion (ost api)
		if ( fd.demux_vtxt == -1 )
		{
			fd.demux_vtxt=open("/dev/dbox/vbi0", O_RDWR);
			if (fd.demux_vtxt<0)
				eDebug("fd.demux_vtxt couldn't be opened");
/*			else
				eDebug("fd.demux_vtxt opened");*/
		}
		if ( current.tpid != -1 ) // we stop old vbi vtxt insertion
		{
			eDebugNoNewLine("VBI_DEV_STOP - vtxt - ");
			if (::ioctl(fd.demux_vtxt, AVIA_VBI_STOP_VTXT )<0)
				eDebug("failed (%m)");
			else
				eDebug("ok");
		}
		if ( parms.tpid != -1 )  // we start old vbi vtxt insertion
		{
			eDebugNoNewLine("VBI_DEV_START - vtxt - ");
			if (::ioctl(fd.demux_vtxt, AVIA_VBI_START_VTXT, parms.tpid)<0)
				eDebug("failed (%m)");
			else
				eDebug("ok");
		}
		else  // we have no tpid ... close device
		{
			close(fd.demux_vtxt);
			fd.demux_vtxt = -1;
//			eDebug("fd.demux_vtxt closed");
		}
#else
		// vtxt reinsertion (ost api)
		if ( fd.demux_vtxt == -1 )
		{
			fd.demux_vtxt=open(DEMUX_DEV, O_RDWR);
			if (fd.demux_vtxt<0)
				eDebug("fd.demux_vtxt couldn't be opened");
/*			else
				eDebug("fd.demux_vtxt opened");*/
		}
		if ( current.tpid != -1 ) // we stop dmx vtxt
		{
#ifndef TUXTXT_CFG_STANDALONE
			if ( enable_background_caching )
				tuxtxt_stop();
#endif
			eDebugNoNewLine("DEMUX_STOP - vtxt - ");
			if (::ioctl(fd.demux_vtxt, DMX_STOP)<0)
				eDebug("failed (%m)");
			else
				eDebug("ok");
		}
		if ( parms.tpid != -1 )
		{
#ifndef TUXTXT_CFG_STANDALONE
			if ( enable_background_caching )
				tuxtxt_start(parms.tpid);
#endif
			pes_filter.pid=parms.tpid;
			pes_filter.input=DMX_IN_FRONTEND;
			pes_filter.output=DMX_OUT_DECODER;
			pes_filter.pesType=DMX_PES_TELETEXT;
			pes_filter.flags=0;
			eDebugNoNewLine("DMX_SET_PES_FILTER(0x%02x) - vtxt - ", parms.tpid);
			if (::ioctl(fd.demux_vtxt, DMX_SET_PES_FILTER, &pes_filter)<0)
				eDebug("failed (%m)");
			else
				eDebug("ok");
			eDebugNoNewLine("DEMUX_START - vtxt - ");
			if (::ioctl(fd.demux_vtxt, DMX_START)<0)
				eDebug("failed (%m)");
			else
				eDebug("ok");
		}
		else  // we have no tpid
		{
			close(fd.demux_vtxt);
			fd.demux_vtxt = -1;
//			eDebug("fd.demux_vtxt closed");
		}
#endif
	}

	current=parms;

	return 0;
}

void Decoder::startTrickmode()
{
	eDebug("startTrickmode");
	if (fd.video != -1)
	{
		if (::ioctl(fd.video, VIDEO_FAST_FORWARD, 0)<0)
			eDebug("VIDEO_FAST_FORWARD failed (%m)");
		if (fd.audio != -1 && ::ioctl(fd.audio, AUDIO_SET_AV_SYNC, 0)<0)
			eDebug("AUDIO_SET_AV_SYNC failed (%m)");
		if (fd.audio != -1 && ::ioctl(fd.audio, AUDIO_SET_MUTE,1)<0)
			eDebug("AUDIO_SET_MUTE failed (%m)");
	}
}

void Decoder::stopTrickmode()
{
	eDebug("stopTrickmode");
	if (fd.video != -1)
	{
		if (::ioctl(fd.video, VIDEO_CONTINUE)<0)
			eDebug("VIDEO_CONTINUE failed (%m)");
		if (fd.audio != -1 && ::ioctl(fd.audio, AUDIO_SET_AV_SYNC, 1)<0)
			eDebug("AUDIO_SET_AV_SYNC failed (%m)");
		if (fd.audio != -1 && ::ioctl(fd.audio, AUDIO_SET_MUTE,0)<0)
			eDebug("AUDIO_SET_MUTE failed (%m)");
	}
}

void Decoder::addCADescriptor(__u8 *descriptor)
{
#if 0
	// this code is broken.. do not use this..
	// on PMT updates parms.descriptor_length are not set to 0 ..
	// so after many pmt updates parms.descriptor_length is longer than 2048..
	// and then .. *booom*
	memcpy(parms.descriptors+parms.descriptor_length, descriptor, descriptor[1]+2);
	parms.descriptor_length+=descriptor[1]+2;
#endif
}

int Decoder::displayIFrame(const char *frame, int len)
{
	(void)frame;
	(void)len;
	int fdv=::open("/dev/video", O_WRONLY);
	eDebug("Decoder::displayIFrame: opening /dev/video: %d", fdv);
	if (fdv < 0)
		return -1;

	if ( fd.video != -1 )
	{
		parms.vpid=-1;
		Set();
	}

	int wasOpen = fd.video != -1;
	if (!wasOpen)
		fd.video = open(VIDEO_DEV, O_RDWR);
	if (::ioctl(fd.video, VIDEO_SELECT_SOURCE, VIDEO_SOURCE_MEMORY )<0)
		eDebug("VIDEO_SELECT_SOURCE failed (%m)");
	if ( ::ioctl(fd.video, VIDEO_CLEAR_BUFFER)<0 )
		eDebug("VIDEO_CLEAR_BUFFER failed (%m)");
	if ( ::ioctl(fd.video, VIDEO_PLAY) < 0 )
		eDebug("VIDEO_PLAY failed (%m)");

	for ( int i=0; i < 2; i++ )
		write(fdv, frame, len);

	unsigned char buf[128];
	memset(&buf, 0, 128);
	write(fdv, &buf, 128);

	setAutoFlushScreen(0);
	if ( ::ioctl(fd.video, VIDEO_SET_BLANK, 0) < 0 )
		eDebug("VIDEO_SET_BLANK failed (%m)");
	if ( !wasOpen )
	{
		::close(fd.video);
		fd.video=-1;
	}
	setAutoFlushScreen(1);

	close(fdv);
	return 0;
}

int Decoder::displayIFrameFromFile(const char *filename)
{
	int file=::open(filename, O_RDONLY);
	if (file < 0)
		return -1;
	int size=::lseek(file, 0, SEEK_END);
	::lseek(file, 0, SEEK_SET);
	if (size < 0)
	{
		::close(file);
		return -1;
	}
	char *buffer = new char[size];
	::read(file, buffer, size);
	::close(file);
	int res=displayIFrame(buffer, size);
	delete[] buffer;
	return res;
}

// Non DVB API Functions and ioctls

#define VIDEO_FLUSH_CLIP_BUFFER 0
#define VIDEO_GET_PTS           _IOR('o', 1, unsigned int*)
#define VIDEO_SET_AUTOFLUSH     _IOW('o', 2, int)
#define VIDEO_CLEAR_SCREEN      3
#define VIDEO_SET_FASTZAP	_IOW('o', 4, int)

#ifndef DMX_GET_STC
struct dmx_stc
{
	unsigned int num;	/* input : which STC? O..N */
	unsigned int base;	/* output: divisor for stc to get 90 kHz clock */
	unsigned long long stc; /* output: src in 'base'*90 kHz units */
};
#define DMX_GET_STC		_IOR('o', 50, struct dmx_stc)
#endif

void Decoder::flushClipBuffer()
{
	int wasOpen = fd.mpeg != -1;
	if ( !wasOpen )
		fd.mpeg = ::open("/dev/video", O_RDONLY);
	{
		if (::ioctl(fd.mpeg, VIDEO_FLUSH_CLIP_BUFFER) < 0)
			eDebug("VIDEO_FLUSH_BUFFER failed (%m)");
	}
	if (!wasOpen)
	{
		::close(fd.mpeg);
		fd.mpeg = -1;
	}
}

void Decoder::clearScreen()
{
	int wasOpen = fd.mpeg != -1;
	if ( !wasOpen )
		fd.mpeg=::open("/dev/video", O_RDONLY);
	if ( fd.mpeg > -1 )
	{
		if ( ::ioctl(fd.mpeg, VIDEO_CLEAR_SCREEN) < 0 )
			eDebug("VIDEO_CLEAR_SCREEN failed (%m)");
		if (!wasOpen)
		{
			::close(fd.mpeg);
			fd.mpeg = -1;
		}
	}
}

void Decoder::getVideoPTS( unsigned int &dest )
{
	if ( fd.mpeg == -1 )
	{
		eDebug("MPEG Decoder fd not valid.. dont get PTS");
		return;
	}
	if ( ::ioctl(fd.mpeg, VIDEO_GET_PTS, &dest) < 0 )
		eDebug("VIDEO_GET_PTS failed (%m)");
}


int Decoder::getSTC(unsigned long long &dst)
{
	struct dmx_stc stc;
	stc.num = 0;

	int f = fd.demux_video;
	if (f < 0)
		f = fd.demux_audio;
	if (f < 0)
		return -1;
	if (ioctl(f, DMX_GET_STC, &stc) < 0)
		return -errno;
	dst = stc.stc;
	return 0;
}

void Decoder::setFastZap(int val)
{
	int wasOpen = fd.mpeg != -1;
	if ( !wasOpen )
		fd.mpeg = ::open("/dev/video", O_RDONLY);
	if ( fd.mpeg > -1 )
	{
		if ( ::ioctl(fd.mpeg, VIDEO_SET_FASTZAP, val) < 0 )
			eDebug("VIDEO_SET_FASTZAP failed (%m)");
		if (!wasOpen)
		{
			::close(fd.mpeg);
			fd.mpeg = -1;
		}
	}
}

void Decoder::setAutoFlushScreen( int on )
{
	int wasOpen = fd.mpeg != -1;
	if ( !wasOpen )
		fd.mpeg = ::open("/dev/video", O_RDONLY);
	if ( fd.mpeg > -1 )
	{
		if ( ::ioctl(fd.mpeg, VIDEO_SET_AUTOFLUSH, on) < 0 )
			eDebug("VIDEO_SET_AUTOFLUSH failed (%m)");
		if (!wasOpen)
		{
			::close(fd.mpeg);
			fd.mpeg = -1;
		}
	}
}

