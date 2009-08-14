#include <enigma_standby.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <enigma_lcd.h>
#include <lib/gdi/lcd.h>
#include <lib/dvb/record.h>
#include <lib/dvb/service.h>
#include <lib/dvb/frontend.h>
#include <lib/dvb/dvbservice.h>
#include <lib/driver/eavswitch.h>
#include <lib/driver/streamwd.h>
#include <lib/system/init.h>
#include <lib/system/info.h>
#include <lib/system/init_num.h>

#include <linux/hdreg.h>
#include <errno.h>

struct enigmaStandbyActions
{
	eActionMap map;
	eAction wakeUp;
	enigmaStandbyActions():
		map("enigmaStandby", _("enigma standby")),
		wakeUp(map, "wakeUp", _("wake up enigma"), eAction::prioDialog)	{
	}
};

eAutoInitP0<enigmaStandbyActions> i_enigmaStandbyActions(eAutoInitNumbers::actions, "enigma standby actions");

eZapStandby* eZapStandby::instance=0;

Signal0<void> eZapStandby::enterStandby, 
							eZapStandby::leaveStandby,
							eZapStandby::RCWakeUp;

void eZapStandby::wakeUp(int norezap)
{
	if (norezap)
		rezap=0;
	close(0);
}

void eZapStandby::renewSleep()
{
	eDebug("renewSleep");
	eServiceHandler *handler=eServiceInterface::getInstance()->getService();
	rezap = 0;
	if (handler)
	{
		ref = eServiceInterface::getInstance()->service;
		if (handler->getFlags() & eServiceHandler::flagIsSeekable)
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 0));
		else
		{
			rezap=1;
			eServiceInterface::getInstance()->stop();
		}
	}
	else
		rezap=1;
	if ( !eDVB::getInstance()->recorder )
	{
		::sync();
		struct stat s;
		if (!::stat("/var/etc/enigma_enter_idle.sh", &s))
			system("/var/etc/enigma_enter_idle.sh");
		
		// set hdd to sleep
		setHddToStandby("/dev/ide/host0/bus0/target0/lun0/disc");
		setHddToStandby("/dev/ide/host0/bus0/target1/lun0/disc");
	}
}

extern bool canPlayService( const eServiceReference & ref ); // implemented in timer.cpp

int eZapStandby::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::evtAction:
		if (event.action == &i_enigmaStandbyActions->wakeUp)
		{
			close(0);
			/*emit*/ RCWakeUp();
		}
		else
			break;
		return 0;
	case eWidgetEvent::execBegin:
	{
		eConfig::getInstance()->flush();
		/*emit*/ enterStandby();
		struct stat s;
		if (!::stat("/var/etc/enigma_enter_standby.sh", &s))
			system("/var/etc/enigma_enter_standby.sh");
#ifndef DISABLE_LCD
		eZapLCD *pLCD=eZapLCD::getInstance();
		pLCD->lcdMain->hide();
		pLCD->lcdStandby->show();
		eDBoxLCD::getInstance()->switchLCD(0);
#endif
		eAVSwitch::getInstance()->setInput(1);
		unsigned int autoVCRswitching = 0;
		eConfig::getInstance()->getKey("/elitedvb/video/vcr_switching", autoVCRswitching );
		if(autoVCRswitching)
			eAVSwitch::getInstance()->setTVPin8CheckVCR(0);
		else
			eAVSwitch::getInstance()->setTVPin8(0);
		renewSleep();
		if( !eSystemInfo::getInstance()->hasLCD() ) //  in standby
		{
			time_t c=time(0)+eDVB::getInstance()->time_difference;
			tm *t=localtime(&c);

			int num=9999;
			int stdby=1;
			if (t && eDVB::getInstance()->time_difference)
			{
				int clktype = 0;
				eConfig::getInstance()->getKey("/ezap/osd/12hourClock", clktype);
				int hour = t->tm_hour;
				if (clktype) {
					hour %= 12;
					if (hour == 0) hour = 12;
				}
				num = hour*100+t->tm_min;
			}

			eDebug("write standby time to led-display");
			int fd=::open("/dev/dbox/fp0",O_RDWR);
			::ioctl(fd,11,&stdby);
			::ioctl(fd,4,&num);
			::close(fd);
		}

		break;
	}
	case eWidgetEvent::execDone:
	{
		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
#ifndef DISABLE_LCD
		eZapLCD *pLCD=eZapLCD::getInstance();
		pLCD->lcdStandby->hide();
		pLCD->lcdMain->show();
		eDBoxLCD::getInstance()->switchLCD(1);
#endif
		eAVSwitch::getInstance()->setInput(0);
		eAVSwitch::getInstance()->setTVPin8(-1); // reset prev voltage
		if (handler->getFlags() & eServiceHandler::flagIsSeekable)
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSetSpeed, 2));
		else if ( rezap && canPlayService(ref) )
			eServiceInterface::getInstance()->play(ref);
		else if ( rezap && eDVB::getInstance()->recorder )
			eServiceInterface::getInstance()->play(eDVB::getInstance()->recorder->recRef);
		eStreamWatchdog::getInstance()->reloadSettings();
		if( !eSystemInfo::getInstance()->hasLCD() ) //  out of standby
		{
			int stdby=0;
			int fd=::open("/dev/dbox/fp0",O_RDWR);
			::ioctl(fd,11,&stdby);
			::close(fd);
		}
		/*emit*/ leaveStandby();
		struct stat s;
		if (!::stat("/var/etc/enigma_leave_standby.sh", &s))
			system("/var/etc/enigma_leave_standby.sh");
		if ( !eDVB::getInstance()->recorder )
		{
			if (!::stat("/var/etc/enigma_leave_idle.sh", &s))
				system("/var/etc/enigma_leave_idle.sh");
		}
		break;
	}
	default:
		break;
	}
	return eWidget::eventHandler(event);
}

eZapStandby::eZapStandby(): eWidget(0, 1)
{
	init_eZapStandby();
}
void eZapStandby::init_eZapStandby()
{
	addActionMap(&i_enigmaStandbyActions->map);
	if (!instance)
		instance=this;
	else
		eFatal("more than ob eZapStandby instance is created!");
}

void eZapStandby::setHddToStandby(const char* devicename)
{
#ifndef WIN_CHECKPOWERMODE1
#define WIN_CHECKPOWERMODE1 0xE5
#endif
#ifndef WIN_CHECKPOWERMODE2
#define WIN_CHECKPOWERMODE2 0x98
#endif
	int fd;
	struct stat stat_buf;
	if (!stat(devicename, &stat_buf))
	{
		fd = ::open(devicename, O_RDONLY|O_NONBLOCK, 0777);
		if (fd >= 0)
		{
			bool hdd_not_sleeping;
			unsigned char args[4] = {WIN_CHECKPOWERMODE1,0,0,0};
			if (::ioctl(fd, HDIO_DRIVE_CMD, &args)
				&& (args[0] = WIN_CHECKPOWERMODE2) /* try again with 0x98 */
				&& ::ioctl(fd, HDIO_DRIVE_CMD, &args))
			{
				if (errno != EIO || args[0] != 0 || args[1] != 0)
					hdd_not_sleeping = true;
				else
					hdd_not_sleeping = false;
			}
			else
			{
				hdd_not_sleeping = (args[2] == 255) ? true : false;
			}
			
			if (hdd_not_sleeping)
			{
#ifndef WIN_STANDBYNOW1
#define WIN_STANDBYNOW1 0xE0
#endif
#ifndef WIN_STANDBYNOW2
#define WIN_STANDBYNOW2 0x94
#endif
				unsigned char args1[4] = {WIN_STANDBYNOW1,0,0,0};
				unsigned char args2[4] = {WIN_STANDBYNOW2,0,0,0};
				::ioctl(fd, HDIO_DRIVE_CMD, &args1);
				::ioctl(fd, HDIO_DRIVE_CMD, &args2);
			}
			::close(fd);
		}
	}
}
