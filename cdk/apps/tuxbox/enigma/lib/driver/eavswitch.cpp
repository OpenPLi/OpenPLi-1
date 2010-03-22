#include <lib/driver/eavswitch.h>

#include <config.h>
#if HAVE_DVB_API_VERSION < 3
#define VIDEO_DEV "/dev/dvb/card0/video0"
#define AUDIO_DEV "/dev/dvb/card0/audio0"
#include <ost/audio.h>
#include <ost/video.h>
#else
#define VIDEO_DEV "/dev/dvb/adapter0/video0"
#define AUDIO_DEV "/dev/dvb/adapter0/audio0"
#include <linux/dvb/audio.h>
#include <linux/dvb/video.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include <dbox/avs_core.h>
#include <sys/ioctl.h>
#include <lib/base/eerror.h>
#include <lib/base/estring.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <lib/dvb/decoder.h>
#include <lib/driver/streamwd.h>

/* sucks */

#define SAAIOGREG               1 /* read registers                             */
#define SAAIOSINP               2 /* input control                              */
#define SAAIOSOUT               3 /* output control                     */
#define SAAIOSENC               4 /* set encoder (pal/ntsc)             */
#define SAAIOSMODE              5 /* set mode (rgb/fbas/svideo/component) */
#define SAAIOSWSS              10 /* set wide screen signaling data */

#define SAA_MODE_RGB    0
#define SAA_MODE_FBAS   1
#define SAA_MODE_SVIDEO 2
#define SAA_MODE_COMPONENT 3

#define SAA_NTSC                0
#define SAA_PAL                 1
#define SAA_PAL_M               2

#define SAA_INP_MP1             1
#define SAA_INP_MP2             2
#define SAA_INP_CSYNC   4
#define SAA_INP_DEMOFF  6
#define SAA_INP_SYMP    8
#define SAA_INP_CBENB   128

#define SAA_WSS_43F     0
#define SAA_WSS_169F    7
#define SAA_WSS_OFF     8

eAVSwitch *eAVSwitch::instance=0;

eAVSwitch::eAVSwitch()
{
	init_eAVSwitch();
}
void eAVSwitch::init_eAVSwitch()
{
	input=0;
	active=0;
	audioChannel=0;
	if (!instance)
		instance=this;

	avsfd=open("/dev/dbox/avs0", O_RDWR);

	saafd=open("/dev/dbox/saa0", O_RDWR);

	system = vsPAL;

	useOst =
		eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000;
}

void eAVSwitch::init()
{
	loadScartConfig();

	reloadSettings();  // only Colorsettings...

	if (eConfig::getInstance()->getKey("/elitedvb/audio/mute", mute))
		mute=0;

	if (eConfig::getInstance()->getKey("/elitedvb/audio/VCRvolume", VCRVolume))
		VCRVolume=10;

	if (eConfig::getInstance()->getKey("/elitedvb/audio/volume", volume))
		volume=10;

	setInput(0);
	setActive(1);
	changeVolume(0,0);
}

eAVSwitch *eAVSwitch::getInstance()
{
	return instance;
}

eAVSwitch::~eAVSwitch()
{
	eConfig::getInstance()->setKey("/elitedvb/audio/volume", volume);
	eConfig::getInstance()->setKey("/elitedvb/audio/VCRvolume", VCRVolume);
	eConfig::getInstance()->setKey("/elitedvb/audio/mute", mute);

	if (instance==this)
		instance=0;

	if (avsfd>=0)
		close(avsfd);
	if (saafd>=0)
		close(saafd);
}

void eAVSwitch::reloadSettings()
{
	unsigned int colorformat;
	eConfig::getInstance()->getKey("/elitedvb/video/colorformat", colorformat);
	setColorFormat((eAVColorFormat)colorformat);
	setVSystem(system);
}

void eAVSwitch::selectAudioChannel( int c )
{
#if HAVE_DVB_API_VERSION < 3
	audioChannelSelect_t chan=AUDIO_STEREO;
#else
	audio_channel_select_t chan=AUDIO_STEREO;
#endif
	switch(c)
	{
		case 0: // Mono Left
			chan = AUDIO_MONO_LEFT;
			break;
		case 1: // Stereo
			break;
		case 2: // Mono Right
			chan = AUDIO_MONO_RIGHT;
			break;
	}
	audioChannel=c;
	
	int fd = Decoder::getAudioDevice();

	if ( fd == -1 )
		fd = open( AUDIO_DEV, O_RDWR );

	int ret=ioctl(fd, AUDIO_CHANNEL_SELECT, chan);
	if(ret)
		eDebug("AUDIO_CHANNEL_SELECT failed (%m)");

	if (Decoder::getAudioDevice() == -1)
		close(fd);
}

int eAVSwitch::setVolume(int vol)
{
	vol=63-vol/(65536/64);

#if 0	
	char volumetext[100];
	sprintf(volumetext, "Old: %d", vol);
	eDebug(volumetext);
#endif

	// Make low volumes a bit "faster" and
	// high volumes a bit "slower".
	// The whole setVolume interface is stupid is it's
	// divided here by (65536/64) and multiplied while called.
	// But I don't want to change that now.
	
	if(vol<=45)
	{
		vol = (vol * 35) / 45;
	}
	else
	{
		vol = (vol * 28) / 18 - 35;
	}

#if 0	
	sprintf(volumetext, "New: %d", vol);
	eDebug(volumetext);
#endif

	if (vol<0)
		vol=0;
	if (vol>63)
		vol=63;

	if ( useOst )
	{
#if HAVE_DVB_API_VERSION < 3   
		audioMixer_t mix;
#else
		audio_mixer_t mix;
#endif
		mix.volume_left=(vol*vol)/64;

		mix.volume_right=(vol*vol)/64;

		int fd = Decoder::getAudioDevice();

		if ( fd == -1 )
			fd = open( AUDIO_DEV, O_RDWR );

		int ret=ioctl(fd, AUDIO_SET_MIXER, &mix);
		if(ret)
			eDebug("AUDIO_SET_MIXER failed(%m)");

		if (Decoder::getAudioDevice() == -1)
			close(fd);

		return ret;
	}
	return ioctl(avsfd, AVSIOSVOL, &vol);
}

void eAVSwitch::changeVolume(int abs, int vol)
{
	switch (abs)
	{
		case 0:
			volume+=vol;
			break;
		case 1:
			volume=vol;
			break;
	}

	if (volume<0)
		volume=0;

	if (volume>63)
		volume=63;

//	if (volume) 
//	{
		if ( (!mute && volume == 63)
			|| (mute && volume != 63 ) )
			toggleMute();

		sendVolumeChanged();
//	}

	setVolume( (63-volume) * 65536/64 );
}

void eAVSwitch::changeVCRVolume(int abs, int vol)
{
	switch (abs)
	{
		case 0:
			VCRVolume+=vol;
		break;
		case 1:
			VCRVolume=vol;
		break;
	}

	if (VCRVolume<0)
		VCRVolume=0;

	if (VCRVolume>63)
		VCRVolume=63;

	setVolume( (63-VCRVolume) * 65536/64 );

	/*emit*/ volumeChanged(mute, VCRVolume);
}

void eAVSwitch::muteOstAudio(bool b)
{
	int fd = Decoder::getAudioDevice();

	if (fd == -1)
		fd = open(AUDIO_DEV, O_RDWR);

	if (ioctl(fd, AUDIO_SET_MUTE, b?1:0) < 0)
	{
		perror("OST SET MUTE:");
		return;
	}
	if (Decoder::getAudioDevice() == -1)
		close(fd);
}

void eAVSwitch::sendVolumeChanged()
{
	/*emit*/ volumeChanged(mute, volume);
}

void eAVSwitch::toggleMute()
{
	if ( input == 1 ) //Scart
	{
		switch ( eSystemInfo::getInstance()->getHwType() )
		{
			case eSystemInfo::dbox2Nokia:
			case eSystemInfo::dbox2Sagem:
			case eSystemInfo::dbox2Philips:
				break;
			default:
				return;
		}
	}
	if (mute)
	{
		if ( useOst )
			muteOstAudio(0);
		else
			muteAvsAudio(0);

		mute = 0;
	}
	else
	{
		if ( useOst )
			muteOstAudio(1);
		else
			muteAvsAudio(1);

		mute = 1;
	}
	sendVolumeChanged();
}

void eAVSwitch::muteAvsAudio(bool m)
{
	if ( input == 1 ) //Scart 
	{
		switch ( eSystemInfo::getInstance()->getHwType() )
		{
			case eSystemInfo::dbox2Nokia:
			case eSystemInfo::dbox2Sagem:
			case eSystemInfo::dbox2Philips:
				break;
			default:
				return;
		}
	}
	int a;

	if(m)
		a=AVS_MUTE;
	else
		a=AVS_UNMUTE;

	if (ioctl(avsfd, AVSIOSMUTE, &a) < 0)
	{
		perror("AVSIOSMUTE:");
		return;
	}
}
int eAVSwitch::setTVPin8CheckVCR(int vol)
{
	int isVCRActive = eStreamWatchdog::getInstance()->getVCRActivity();
	int isVCRManual = 0;
	eConfig::getInstance()->getKey("/elitedvb/video/VCRmanual", isVCRManual);
	if (active && input && (isVCRActive || isVCRManual))
	{
		return 1;
	}
	return setTVPin8(vol);
}
int eAVSwitch::setTVPin8(int vol)
{
/* results from philips:	fnc=0 -> 0V
				fnc=1 -> 0V
				fnc=2 -> 6V
				fnc=3 -> 12V
*/
	int fnc;
	if ( vol == -1 ) // switch to last aspect
		vol = aspect == r169?6:12;
	switch (vol)
	{
	case 0:
		fnc=(Type==PHILIPS?1:0);
		break;
	case 6:
		fnc=(Type==PHILIPS?2:1);
		break;
	default:
	case 12:
		fnc=(Type==PHILIPS?3:2);
		break;
	}
	return ioctl(avsfd, AVSIOSFNC, &fnc);
}

int eAVSwitch::setColorFormat(eAVColorFormat c)
{
	colorformat=c;
	int arg=0;
	switch (c)
	{
	case cfNull:
		return -1;
	default:
	case cfCVBS:
		arg=SAA_MODE_FBAS;
		break;
	case cfRGB:
		arg=SAA_MODE_RGB;
		break;
	case cfYC:
		arg=SAA_MODE_SVIDEO;
		break;
	case cfYPbPr:
		arg=SAA_MODE_COMPONENT;
		break;
	}
	int fblk = ((c == cfRGB) || (c == cfYPbPr))?1:0;
	ioctl(saafd, SAAIOSMODE, &arg);
	ioctl(avsfd, AVSIOSFBLK, &fblk);
	return 0;
}

int eAVSwitch::setInput(int v)
{
	eDebug("[eAVSwitch] setInput %d, avsfd=%d", v, avsfd);
	switch (v)
	{
	case 0:	//	Switch to DVB
		ioctl(avsfd, AVSIOSVSW1, dvb);
		ioctl(avsfd, AVSIOSASW1, dvb+1);
		ioctl(avsfd, AVSIOSVSW2, dvb+2);
		ioctl(avsfd, AVSIOSASW2, dvb+3);
		ioctl(avsfd, AVSIOSVSW3, dvb+4);
		ioctl(avsfd, AVSIOSASW3, dvb+5);
		changeVolume(1, volume);  // set Volume to TV Volume
		if (mute)
		{
			if ( useOst )
				muteOstAudio(1);
			else
				muteAvsAudio(1);
			sendVolumeChanged();
		}
		reloadSettings();						// reload ColorSettings
		input=v;
		break;
	case 1:   // Switch to VCR
		input=v;
		v = (Type == SAGEM)? 0 : 2;
		ioctl(avsfd, AVSIOSFBLK, &v);
		ioctl(avsfd, AVSIOSVSW1, scart);
		ioctl(avsfd, AVSIOSASW1, scart+1);
		ioctl(avsfd, AVSIOSVSW2, scart+2);
		ioctl(avsfd, AVSIOSASW2, scart+3);
		ioctl(avsfd, AVSIOSVSW3, scart+4);
		ioctl(avsfd, AVSIOSASW3, scart+5);
		if (mute)
		{
			if ( useOst )
				muteOstAudio(0);
			else
				muteAvsAudio(0);
		}
		changeVCRVolume(1, VCRVolume);
	}
	return 0;
}

int eAVSwitch::setAspectRatio(eAVAspectRatio as)
{
	int saa;
	aspect=as;

	unsigned int disableWSS = 0;
	eConfig::getInstance()->getKey("/elitedvb/video/disableWSS", disableWSS );

	saa = (aspect==r169) ? SAA_WSS_169F :
		( disableWSS ? SAA_WSS_OFF : SAA_WSS_43F );
	if ( ioctl(saafd,SAAIOSWSS,&saa) < 0 )
		eDebug("SAAIOSWSS failed (%m)");

	return setTVPin8CheckVCR((active && (!input))?((aspect==r169)?6:12):0);
}

int eAVSwitch::toggleAspect()
{
	eAVAspectRatio newval;
	switch (aspect)
	{
	case r169:
		newval = r43;
		break;
	default:
	case r43:
		newval = r169;
		break;
	}
	return setAspectRatio(newval);
}

void eAVSwitch::setVSystem(eVSystem _system)
{
	unsigned int tvsystem = 0;
	eConfig::getInstance()->getKey("/elitedvb/video/tvsystem", tvsystem );
	
	int saa;
	// we *want* to set _system, but let's see if we support this
	if ((_system == vsPAL) && (tvsystem & 1))
		saa = SAA_PAL;
	else if ((_system == vsNTSC) && (tvsystem & 2))
		saa = SAA_NTSC;
	else if ((_system == vsNTSC) && (tvsystem & 4))
		saa = SAA_PAL_M;
				// we give up.
	else if (tvsystem & 1)
		saa = SAA_PAL;
	else if (tvsystem & 2)
		saa = SAA_NTSC;
				// we are broken.
	else
		saa = SAA_PAL;
		
	if ( ::ioctl(saafd, SAAIOSENC, &saa) < 0 )
		eDebug("SAAIOSENC failed (%m)");

	system = _system;
}

int eAVSwitch::setActive(int a)
{
	active=a;
	return setTVPin8CheckVCR((active && (!input))?((aspect==r169)?6:12):0);
}

bool eAVSwitch::loadScartConfig()
{
	FILE* fd = fopen(CONFIGDIR"/scart.conf", "r");
	if(fd)
	{
		eDebug("[eAVSwitch] loading scart-config (scart.conf)");

		char buf[1000];
		fgets(buf,sizeof(buf),fd);

		eString readline_scart = "_scart: %d %d %d %d %d %d\n";
		eString readline_dvb = "_dvb: %d %d %d %d %d %d\n";

		switch (Type)
		{
			case NOKIA:
				readline_scart.insert(0, "nokia");
				readline_dvb.insert(0, "nokia");
			break;
			case PHILIPS:
				readline_scart.insert(0, "philips");
				readline_dvb.insert(0, "philips");
			break;
			case SAGEM:
				readline_scart.insert(0, "sagem");
				readline_dvb.insert(0, "sagem");
			break;
		}

		int i=0;
		while ( fgets(buf,sizeof(buf),fd) != NULL && (i < 3) )
		{
			if ( !(1&i) && sscanf( buf, readline_scart.c_str(), &scart[0], &scart[1], &scart[2], &scart[3], &scart[4], &scart[5] ) == 6)
				i |= 1;
			else if ( !(2&i) && sscanf( buf, readline_dvb.c_str(), &dvb[0], &dvb[1], &dvb[2], &dvb[3], &dvb[4], &dvb[5]) == 6 )
				i |= 2;
		}

		if ( i != 3 )
			eDebug( "[eAVSwitch] failed to parse scart-config (scart.conf), using default-values" );

		eDebug("[eAVSwitch] readed scart conf : %i %i %i %i %i %i", scart[0], scart[1], scart[2], scart[3], scart[4], scart[5] );
		eDebug("[eAVSwitch] readed dvb conf : %i %i %i %i %i %i", dvb[0], dvb[1], dvb[2], dvb[3], dvb[4], dvb[5] );
	
		fclose(fd);
	}
	else
		eDebug("[eAVSwitch] failed to load scart-config (scart.conf), using default-values");

	return 0;
}

void eAVSwitch::setVideoFormat( int format )
{
	int fd = Decoder::getVideoDevice();

	if ( fd == -1 )
		fd = open ( VIDEO_DEV, O_RDWR );

	if (ioctl(fd, VIDEO_SET_DISPLAY_FORMAT, format))
		perror("VIDEO SET DISPLAY FORMAT:");

	if (Decoder::getVideoDevice() == -1)
		close(fd);
}

