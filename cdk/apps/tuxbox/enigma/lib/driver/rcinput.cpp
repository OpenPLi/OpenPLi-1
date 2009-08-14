#include <config.h>
#include <lib/driver/rcinput.h>

#include <sys/ioctl.h>
#include <sys/stat.h>

#include <lib/base/ebase.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/driver/input_fake.h>

void eRCDeviceInputDev::handleCode(int rccode)
{
	struct input_event *ev = (struct input_event *)rccode;


	// eDebug("eRCDeviceInputDev::handleCode: eventtype: %d iskeyboard=%d eventval=%d eventcode=%d descr=%s", ev->type, iskeyboard, ev->value, ev->code, getDescription());

	if (ev->type!=EV_KEY)
		return;

	int km = iskeyboard ? input->getKeyboardMode() : eRCInput::kmNone;
	// eDebug("eRCDeviceInputDev::handleCode: km=%s",
	// 		km == eRCInput::kmNone ? "kmNone" :
	// 		km == eRCInput::kmAll ? "kmAll" :
	// 		km == eRCInput::kmAscii ? "kmAscii" :
	// 		"km Other" );
	
	if (km == eRCInput::kmAll)
		return;
	
	if (km == eRCInput::kmAscii)
	{
//		eDebug("filtering.. %d", ev->code);
		bool filtered = ( ev->code > 0 && ev->code < 61 );
		switch (ev->code)
		{
			case KEY_RESERVED:
			case KEY_ESC:
			case KEY_TAB:
			case KEY_BACKSPACE:
			case KEY_ENTER:
			case KEY_LEFTCTRL:
			case KEY_RIGHTSHIFT:
			case KEY_LEFTALT:
			case KEY_CAPSLOCK:
// added by ims - keys (0-9) for Menu with keyboard and webif's remote controller
			case KEY_0:
			case KEY_1:
			case KEY_2:
			case KEY_3:
			case KEY_4:
			case KEY_5:
			case KEY_6:
			case KEY_7:
			case KEY_8:
			case KEY_9:
// end added by ims
				filtered=false;
			default:
				break;
		}
		if (filtered)
			return;
//		eDebug("passed!");
	}

	switch (ev->value)
	{
	case 0:
		if ( !iskeyboard )
			repeattimer.stop();
		/*emit*/ input->keyPressed(eRCKey(this, ev->code, eRCKey::flagBreak));
		break;
	case 1:
		memcpy(&cur, ev, sizeof(struct input_event) );
		if ( !iskeyboard )
			repeattimer.start(eRCInput::getInstance()->config.rdelay, 1);
		/*emit*/ input->keyPressed(eRCKey(this, ev->code, 0));
		break;
	case 2:
		if ( iskeyboard )
			/*emit*/ input->keyPressed(eRCKey(this, ev->code, eRCKey::flagRepeat));
		break;
	}
}

void eRCDeviceInputDev::repeat()
{
	/* emit */ input->keyPressed(eRCKey(this, cur.code, eRCKey::flagRepeat));
	repeattimer.start(eRCInput::getInstance()->config.rrate, 1);
}

eRCDeviceInputDev::eRCDeviceInputDev(eRCInputEventDriver *driver)
: eRCDevice(driver->getDeviceName(), driver), repeattimer(eApp)
{
	eString tmp=id;
	tmp.upper();
	iskeyboard = !!strstr(tmp.c_str(), "KEYBOARD");
	eDebug("Input device \"%s\" is %s a keyboard.", id.c_str(), iskeyboard ? "" : "not ");
	CONNECT( repeattimer.timeout, eRCDeviceInputDev::repeat);
}

const char *eRCDeviceInputDev::getDescription() const
{
	return id.c_str();
}

const char *eRCDeviceInputDev::getKeyDescription(const eRCKey &key) const
{
	switch (key.code)
	{
	case KEY_0: return "0";
	case KEY_1: return "1";
	case KEY_2: return "2";
	case KEY_3: return "3";
	case KEY_4: return "4";
	case KEY_5: return "5";
	case KEY_6: return "6";
	case KEY_7: return "7";
	case KEY_8: return "8";
	case KEY_9: return "9";
	case KEY_RIGHT: return "rechts";
	case KEY_LEFT: return "links";
	case KEY_UP: return "oben";
	case KEY_DOWN: return "unten";
	case KEY_OK: return "ok";
	case KEY_MUTE: return "mute";
	case KEY_POWER: return "power";
	case KEY_GREEN: return "gruen";
	case KEY_YELLOW: return "gelb";
	case KEY_RED: return "rot";
	case KEY_BLUE: return "blau";
	case KEY_VOLUMEUP: return "Lautstaerke plus";
	case KEY_VOLUMEDOWN: return "Lautstaerke minus";
	case KEY_HELP: return "?";
	case KEY_SETUP: return "d-Box";
#if 0
	case KEY_TOPLEFT: return "oben links";
	case KEY_TOPRIGHT: return "oben rechts";
	case KEY_BOTTOMLEFT: return "unten links";
	case KEY_BOTTOMRIGHT: return "unten rechts";
#endif
	case KEY_HOME: return "home";
	default: return 0;
	}
}

int eRCDeviceInputDev::getKeyCompatibleCode(const eRCKey &key) const
{
	return key.code;
}

class eInputDeviceInit
{
	ePtrList<eRCInputEventDriver> m_drivers;
	ePtrList<eRCDeviceInputDev> m_devices;
public:
	eInputDeviceInit()
	{
		int i = 0;
		while (1)
		{
			struct stat s;
			char filename[128];
			sprintf(filename, "/dev/input/event%d", i);
			if (stat(filename, &s))
				break;
			eRCInputEventDriver *p;
			m_drivers.push_back(p = new eRCInputEventDriver(filename));
			m_devices.push_back(new eRCDeviceInputDev(p));
			++i;
		}
		eDebug("Found %d input devices!", i);
	}

	~eInputDeviceInit()
	{
		while (m_drivers.size())
		{
			delete m_devices.back();
			m_devices.pop_back();
			delete m_drivers.back();
			m_drivers.pop_back();
		}
	}
};

eAutoInitP0<eInputDeviceInit> init_rcinputdev(eAutoInitNumbers::rc+1, "input device driver");
