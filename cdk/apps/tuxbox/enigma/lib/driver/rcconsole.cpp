#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/driver/rcconsole.h>
#include <stdio.h>
#include <fcntl.h>

eRCConsoleDriver::eRCConsoleDriver(const char *filename): eRCDriver(eRCInput::getInstance())
{
	init_eRCConsoleDriver(filename);
}
void eRCConsoleDriver::init_eRCConsoleDriver(const char *filename)
{
	handle=open(filename, O_RDONLY|O_NONBLOCK);
	if (handle<0)
	{
		eDebug("failed to open %s", filename);
		sn=0;
	} else
	{
		sn=new eSocketNotifier(eApp, handle, eSocketNotifier::Read);
		CONNECT(sn->activated, eRCConsoleDriver::keyPressed);
		eRCInput::getInstance()->setFile(handle);
	}
	
		/* set console mode */
	struct termios t,ot;
	tcgetattr(handle, &t);
	t.c_lflag &= ~(ECHO | ICANON | ECHOK | ECHOE | ECHONL);
	ot = t;
	tcsetattr(handle, TCSANOW,&t);
}

eRCConsoleDriver::~eRCConsoleDriver()
{
	tcsetattr(handle,TCSANOW, &ot);
 	if (handle>=0)
		close(handle);
	if (sn)
		delete sn;
}

void eRCConsoleDriver::keyPressed(int)
{
	unsigned char data[16];
	unsigned char *d = data;
	int num = read(handle, data, 16);
	int code;
	
	int km = input->getKeyboardMode();

	if (km == eRCInput::kmNone)
		return;
	
	while (num--)
	{
		if (km == eRCInput::kmAll)
			code = *d++;
		else
		{
			if (*d == 27) // escape code
			{
				while (num)
				{
					num--;
					if (*++d != '[')
						break;
				}
				code = -1;
			} else
				code = *d;
			++d;
			
			if (code < 32)			/* control characters */
				code = -1;
			if (code == 0x7F)		/* delete */
				code = -1;
		}

		if (code != -1)
			for (std::list<eRCDevice*>::iterator i(listeners.begin()); i!=listeners.end(); ++i)
				(*i)->handleCode(code);
	}
}

void eRCConsole::handleCode(int code)
{
//	eDebug("console code is %d", code);
	input->keyPressed(eRCKey(this, code, 0));
}

eRCConsole::eRCConsole(eRCDriver *driver)
			: eRCDevice("Console", driver)
{
}

const char *eRCConsole::getDescription() const
{
	return "Console";
}

const char *eRCConsole::getKeyDescription(const eRCKey &key) const
{
	return 0;
}

int eRCConsole::getKeyCompatibleCode(const eRCKey &key) const
{
	return key.code | KEY_ASCII;
}

class eRCConsoleInit
{
	eRCConsoleDriver driver;
	eRCConsole device;
public:
	eRCConsoleInit(): driver("/dev/vc/0"), device(&driver)
	{
	}
};

eAutoInitP0<eRCConsoleInit> init_rcconsole(eAutoInitNumbers::rc+1, "Console RC Driver");
