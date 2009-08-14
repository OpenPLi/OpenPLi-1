#include <config.h>

#ifndef __rcinput_h
#define __rcinput_h

#include <linux/input.h>
#include <lib/driver/rc.h>

class eRCDeviceInputDev: public eRCDevice
{
	struct input_event cur;
	int iskeyboard;
	eTimer repeattimer;
public:
	void repeat();
	void handleCode(int code);
	eRCDeviceInputDev(eRCInputEventDriver *driver);
	const char *getDescription() const;

	const char *getKeyDescription(const eRCKey &key) const;
	int getKeyCompatibleCode(const eRCKey &key) const;
};

#endif // __rcinput_h

