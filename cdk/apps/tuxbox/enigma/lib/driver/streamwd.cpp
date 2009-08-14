#include <lib/driver/streamwd.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if HAVE_DVB_API_VERSION < 3
#include "ost/video.h"
#else
#include "linux/dvb/video.h"
#endif

#include <dbox/event.h>

#include <lib/base/eerror.h>
#include <lib/driver/eavswitch.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/decoder.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>

#define EVENT_DEVICE "/dev/dbox/event0"

#ifndef EVENT_FRATE_CHANGE
#define EVENT_FRATE_CHANGE  64  /* framerate has changed */
#endif

eStreamWatchdog *eStreamWatchdog::instance;

eStreamWatchdog::eStreamWatchdog()
{
	init_eStreamWatchdog();
}
void eStreamWatchdog::init_eStreamWatchdog()
{
	sn=0;
	handle=open( EVENT_DEVICE, O_RDONLY | O_NONBLOCK );

	if (handle<0)
	{
		eDebug("failed to open %s", EVENT_DEVICE);
		sn=0;
	}
	else
	{
		if ( ioctl(handle, EVENT_SET_FILTER, EVENT_ARATIO_CHANGE | EVENT_VCR_CHANGED | EVENT_FRATE_CHANGE ) < 0 )
		{
			perror("ioctl");
			close(handle);
		}
		else
		{
			sn=new eSocketNotifier(eApp, handle, eSocketNotifier::Read);
			CONNECT(sn->activated, eStreamWatchdog::check);
		}
	}

	if (!instance)
		instance=this;
	isanamorph=0;
}

eStreamWatchdog *eStreamWatchdog::getInstance()
{
	return instance;
}

void eStreamWatchdog::check(int)
{
	struct event_t event;
	int eventSize = sizeof (event);
	int status;
	while ( (status = read(handle, &event, eventSize)) == eventSize )
	{
		if (event.event & (EVENT_ARATIO_CHANGE|EVENT_FRATE_CHANGE|EVENT_VCR_CHANGED))
			reloadSettings();
	}
}

#define FP_IOCTL_GET_VCR 7

int eStreamWatchdog::getVCRActivity()
{
	int val;
	int fp = open("/dev/dbox/fp0",O_RDWR);

	ioctl(fp, FP_IOCTL_GET_VCR, &val);

	close(fp);

	return val;
}

void eStreamWatchdog::reloadSettings(int override_aspect)
{
	static int prevVcrSlbVlt=-1;
	int VcrSlbVlt = getVCRActivity();
	if (eAVSwitch::getInstance()->getInput() && VcrSlbVlt && VcrSlbVlt != prevVcrSlbVlt)  // VCR selected
	{
		prevVcrSlbVlt=VcrSlbVlt;
		// Loop through VCR Slowblanking values to TV Slowblanking
		if ( eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000 )
			eAVSwitch::getInstance()->setTVPin8(VcrSlbVlt==2?12:6);
		return;
	}

	FILE *bitstream=fopen("/proc/bus/bitstream", "rt");
	int frate=0;
	if (bitstream)
	{
		char buffer[100];
		int aspect=0;
		while (fgets(buffer, 100, bitstream))
		{
			if (!strncmp(buffer, "A_RATIO: ", 9))
				aspect=atoi(buffer+9);
			if (!strncmp(buffer, "F_RATE: ", 8))
				frate=atoi(buffer+8);
		}
		fclose(bitstream);
		if (override_aspect != -1)
			aspect = override_aspect;
		switch (aspect)
		{
			case 1:
			case 2:
			default:
				isanamorph=0;
				break;
			case 3:
			case 4:
				isanamorph=1;
		}
	}
	/*emit*/ AspectRatioChanged(isanamorph);

	int videoDisplayFormat=VIDEO_LETTER_BOX;
	int doanamorph=0;
	unsigned int pin8; // Letterbox
	eConfig::getInstance()->getKey("/elitedvb/video/pin8", pin8);
	switch (pin8)
	{
		case 0:
			doanamorph=0;
			videoDisplayFormat=isanamorph?VIDEO_LETTER_BOX:VIDEO_PAN_SCAN;
			break;
		case 1:
			doanamorph=0;
			videoDisplayFormat=VIDEO_PAN_SCAN;
			break;
		case 2:
			doanamorph=isanamorph;
			videoDisplayFormat=isanamorph?VIDEO_CENTER_CUT_OUT:VIDEO_PAN_SCAN;
			break;
		case 3:
			doanamorph=1;
			videoDisplayFormat=VIDEO_CENTER_CUT_OUT;
			break;
	}
	eAVSwitch::getInstance()->setVideoFormat( videoDisplayFormat );
	eAVSwitch::getInstance()->setAspectRatio(doanamorph?r169:r43);

	switch (frate)
	{
		case 1:
		case 2:
		case 3:
		case 6:
			eAVSwitch::getInstance()->setVSystem(vsPAL);
			break;
		case 4:
		case 5:
		case 7:
		case 8:
			eAVSwitch::getInstance()->setVSystem(vsNTSC);
			break;
	}
	unsigned int auto_vcr_switching=1;
	eConfig::getInstance()->getKey("/elitedvb/video/vcr_switching", auto_vcr_switching );
	if ( auto_vcr_switching && VcrSlbVlt != prevVcrSlbVlt )
	{
		prevVcrSlbVlt=VcrSlbVlt;
		/*emit*/VCRActivityChanged( VcrSlbVlt );
	}
}

int eStreamWatchdog::isAnamorph()
{
	return isanamorph;
}

eStreamWatchdog::~eStreamWatchdog()
{
	if (instance==this)
		instance=0;

	if (handle>=0)
		close(handle);

	if (sn)
		delete sn;
}

eAutoInitP0<eStreamWatchdog> eStreamWatchdog_init(eAutoInitNumbers::dvb-1, "stream watchdog");
