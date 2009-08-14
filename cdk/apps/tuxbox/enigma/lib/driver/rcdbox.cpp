#include <config.h>
#if HAVE_DVB_API_VERSION < 3

#ifndef DISABLE_DBOX_RC
#include <lib/driver/rcdbox.h>

#include <sys/ioctl.h>
#include <dbox/fp.h>
#include <sys/stat.h>

#include <lib/base/ebase.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>

	/* ----------------------- alte fernbedienung ---------------------- */
void eRCDeviceDBoxOld::handleCode(int rccode)
{
	if ((rccode&0xFF00)!=0x5C00)
		return;
	if (rccode==0x5CFE)		// old break code
	{
		timeout.stop();
		repeattimer.stop();
		if (ccode!=-1)
		{
			int old=ccode;
			ccode=-1;
			input->keyPressed(eRCKey(this, old, eRCKey::flagBreak));
		}
	} else // if (rccode!=ccode)
	{
		timeout.start(300, 1);
		int old=ccode;
		ccode=rccode;
		if ((old!=-1) && (old!=rccode))
			/* emit */input->keyPressed(eRCKey(this, old, eRCKey::flagBreak));
		if (old != rccode)
		{
			repeattimer.start(eRCInput::getInstance()->config.rdelay, 1);
			input->keyPressed(eRCKey(this, rccode, 0));
		}
	}
}

void eRCDeviceDBoxOld::timeOut()
{
	int oldcc=ccode;
	ccode=-1;
	repeattimer.stop();
	if (oldcc!=-1)
		input->keyPressed(eRCKey(this, oldcc, eRCKey::flagBreak));
}

void eRCDeviceDBoxOld::repeat()
{
	if (ccode!=-1)
		input->keyPressed(eRCKey(this, ccode, eRCKey::flagRepeat));
	repeattimer.start(eRCInput::getInstance()->config.rrate, 1);
}

eRCDeviceDBoxOld::eRCDeviceDBoxOld(eRCDriver *driver): eRCDevice("DBoxOld", driver), timeout(eApp), repeattimer(eApp)
{
	ccode=-1;
	CONNECT(timeout.timeout, eRCDeviceDBoxOld::timeOut);
	CONNECT(repeattimer.timeout, eRCDeviceDBoxOld::repeat);
}

const char *eRCDeviceDBoxOld::getDescription() const
{
	return "alte d-box Fernbedienung";
}

const char *eRCDeviceDBoxOld::getKeyDescription(const eRCKey &key) const
{
	if ((key.code&0xFF00)!=0x5C00)
		return 0;
	switch (key.code&0xFF)
	{
	case 0x0C: return "power";
	case 0x20: return "home";
	case 0x27: return "d-box";
	case 0x00: return "0";
	case 0x01: return "1";
	case 0x02: return "2";
	case 0x03: return "3";
	case 0x04: return "4";
	case 0x05: return "5";
	case 0x06: return "6";
	case 0x07: return "7";
	case 0x08: return "8";
	case 0x09: return "9";
	case 0x3B: return "Blau";
	case 0x52: return "Gelb";
	case 0x55: return "Gruen";
	case 0x2D: return "Rot";
	case 0x54: return "Doppelpfeil oben";
	case 0x53: return "Doppelpfeil unten";
	case 0x0E: return "oben";
	case 0x0F: return "unten";
	case 0x2F: return "links";
	case 0x2E: return "rechts";
	case 0x30: return "ok";
	case 0x16: return "Lautstaerke plus";
	case 0x17: return "Lautstaerke minus";
	case 0x28: return "Mute";
	case 0x82: return "?";
	}
	return 0;
}

int eRCDeviceDBoxOld::getKeyCompatibleCode(const eRCKey &key) const
{
	if ((key.code&0xFF00)==0x5C00)
	{
		switch (key.code&0xFF)
		{
		case 0x0C: return KEY_POWER;
		case 0x20: return KEY_HOME;
		case 0x27: return KEY_MENU;
		case 0x00: return KEY_0;
		case 0x01: return KEY_1;
		case 0x02: return KEY_2;
		case 0x03: return KEY_3;
		case 0x04: return KEY_4;
		case 0x05: return KEY_5;
		case 0x06: return KEY_6;
		case 0x07: return KEY_7;
		case 0x08: return KEY_8;
		case 0x09: return KEY_9;
		case 0x3B: return KEY_BLUE;
		case 0x52: return KEY_YELLOW;
		case 0x55: return KEY_GREEN;
		case 0x2D: return KEY_RED;
		case 0x54: return KEY_PAGEUP;
		case 0x53: return KEY_PAGEDOWN;
		case 0x0E: return KEY_UP;
 		case 0x0F: return KEY_DOWN;
		case 0x2F: return KEY_LEFT;
 		case 0x2E: return KEY_RIGHT;
		case 0x30: return KEY_OK;
 		case 0x16: return KEY_VOLUMEUP;
 		case 0x17: return KEY_VOLUMEDOWN;
 		case 0x28: return KEY_MUTE;
 		case 0x82: return KEY_HELP;
		default:
			return -1;
		}
	}
	return -1;
}

	/* ----------------------- neue fernbedienung ---------------------- */
void eRCDeviceDBoxNew::handleCode(int rccode)
{
	if ((rccode&0xFF00)!=0x0000)
		return;
	timeout.start(300, 1);
	int old=ccode;
	ccode=rccode;
	if ((old!=-1) && (old!=rccode))
		/*emit*/ input->keyPressed(eRCKey(this, old&0x3F, eRCKey::flagBreak));
	if (old != rccode)
	{
		repeattimer.start(eRCInput::getInstance()->config.rdelay/*+500*/, 1);
		input->keyPressed(eRCKey(this, rccode&0x3F, 0));
	}
}

void eRCDeviceDBoxNew::timeOut()
{
	int oldcc=ccode;
	ccode=-1;
	repeattimer.stop();
	if (oldcc!=-1)
		input->keyPressed(eRCKey(this, oldcc&0x3F, eRCKey::flagBreak));
}

void eRCDeviceDBoxNew::repeat()
{
	if (ccode!=-1)
		input->keyPressed(eRCKey(this, ccode&0x3F, eRCKey::flagRepeat));
	repeattimer.start(eRCInput::getInstance()->config.rrate, 1);
}

eRCDeviceDBoxNew::eRCDeviceDBoxNew(eRCDriver *driver): eRCDevice("DBoxNew", driver), timeout(eApp), repeattimer(eApp)
{
	ccode=-1;
	CONNECT(timeout.timeout, eRCDeviceDBoxNew::timeOut);
	CONNECT(repeattimer.timeout, eRCDeviceDBoxNew::repeat);
}

const char *eRCDeviceDBoxNew::getDescription() const
{
	return "neue d-box Fernbedienung";
}

const char *eRCDeviceDBoxNew::getKeyDescription(const eRCKey &key) const
{
	switch (key.code)
	{
	case 0: return "0";
	case 1: return "1";
	case 2: return "2";
	case 3: return "3";
	case 4: return "4";
	case 5: return "5";
	case 6: return "6";
	case 7: return "7";
	case 8: return "8";
	case 9: return "9";
	case 10: return "rechts";
	case 11: return "links";
	case 12: return "oben";
	case 13: return "unten";
	case 14: return "ok";
	case 15: return "mute";
	case 16: return "power";
	case 17: return "gruen";
	case 18: return "gelb";
	case 19: return "rot";
	case 20: return "blau";
	case 21: return "Lautstaerke plus";
	case 22: return "Lautstaerke minus";
	case 23: return "?";
	case 24: return "d-Box";
	case 27: return "oben links";
	case 28: return "oben rechts";
	case 29: return "unten links";
	case 30: return "unten rechts";
	case 31: return "home";
	default: return 0;
	}
}

int eRCDeviceDBoxNew::getKeyCompatibleCode(const eRCKey &key) const
{
	switch (key.code&0xFF)
	{
		case 0: return KEY_0;
		case 1: return KEY_1;
    case 2: return KEY_2;
		case 3: return KEY_3;
		case 4: return KEY_4;
		case 5: return KEY_5;
		case 6: return KEY_6;
		case 7: return KEY_7;
		case 8: return KEY_8;
		case 9: return KEY_9;
		case 10: return KEY_RIGHT;
		case 11: return KEY_LEFT;
		case 12: return KEY_UP;
		case 13: return KEY_DOWN;
		case 14: return KEY_OK;
		case 15: return KEY_MUTE;
		case 16: return KEY_POWER;
		case 17: return KEY_GREEN;
		case 18: return KEY_YELLOW;
		case 19: return KEY_RED;
		case 21: return KEY_VOLUMEUP;
		case 20: return KEY_BLUE;
		case 22: return KEY_VOLUMEDOWN;
		case 23: return KEY_HELP;
		case 24: return KEY_MENU;
		case 31: return KEY_HOME;
	}
	return -1;
}

	/* ----------------------- dbox buttons ---------------------- */
void eRCDeviceDBoxButton::handleCode(int code)
{
	if ((code&0xFF00)!=0xFF00)
		return;
	
	code=(~code)&0xF;
	
	for (int i=0; i<4; i++)
		if ((last&~code) & (1<<i))
			/*emit*/ input->keyPressed(eRCKey(this, i, eRCKey::flagBreak));
		else if ((~last&code)&(1<<i))
			/*emit*/ input->keyPressed(eRCKey(this, i, 0));
	if (code)
		repeattimer.start(eRCInput::getInstance()->config.rdelay, 1);
	else
		repeattimer.stop();
	last=code;
}

void eRCDeviceDBoxButton::repeat()
{
	for (int i=0; i<4; i++)
		if (last&(1<<i))
			/*emit*/ input->keyPressed(eRCKey(this, i, eRCKey::flagRepeat));
	repeattimer.start(eRCInput::getInstance()->config.rrate, 1);
}

eRCDeviceDBoxButton::eRCDeviceDBoxButton(eRCDriver *driver): eRCDevice("DBoxButton", driver), repeattimer(eApp)
{
	last=0;
	CONNECT(repeattimer.timeout, eRCDeviceDBoxButton::repeat);
}

const char *eRCDeviceDBoxButton::getDescription() const
{
	return "d-box Buttons";
}

const char *eRCDeviceDBoxButton::getKeyDescription(const eRCKey &key) const
{
	switch (key.code)
	{
	case 1: return "power";
	case 2: return "down";
	case 3: return "up";
	default: return 0;
	}
}

int eRCDeviceDBoxButton::getKeyCompatibleCode(const eRCKey &key) const
{
	switch (key.code)
	{
	case 1: return KEY_POWER;
	case 2: return KEY_RIGHT;
	case 3: return KEY_LEFT;
	}
	return -1;
}

eRCDBoxDriver::eRCDBoxDriver(): eRCShortDriver("/dev/dbox/rc0")
{
	if (handle>0)
		ioctl(handle, RC_IOCTL_BCODES, 1);
}


class eDBoxRCHardware
{
  eRCDBoxDriver driver;
  eRCDeviceDBoxOld deviceOld;
  eRCDeviceDBoxNew deviceNew;
  eRCDeviceDBoxButton deviceButton;
public:
  eDBoxRCHardware(): deviceOld(&driver), deviceNew(&driver), deviceButton(&driver)
  {
		struct stat s;
		if (stat("/dev/rawir2", &s))
			driver.enable(1);
		else
			driver.enable(0);
  }
};

eAutoInitP0<eDBoxRCHardware> init_rcdbox(eAutoInitNumbers::rc+1, "d-Box RC Hardware");

#endif // DISABLE_DBOX_RC

#endif // HAVE_DVB_API_VERSION < 3
