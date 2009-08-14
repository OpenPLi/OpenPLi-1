#ifndef __streamwd_h
#define __streamwd_h

#include <libsig_comp.h>
#include <lib/base/ebase.h>
#include <lib/dvb/si.h>
#include <lib/dvb/dvb.h>

class eStreamWatchdog: public Object
{
	eSocketNotifier* sn;
	int handle;
	int isanamorph;
	static eStreamWatchdog *instance;
	void init_eStreamWatchdog();
private:
	void check(int);
public:
	int getVCRActivity();
	void reloadSettings(int aspect=-1);
	eStreamWatchdog();
	~eStreamWatchdog();
	static eStreamWatchdog *getInstance();
	int isAnamorph();
	Signal1<void, int> AspectRatioChanged;
	Signal1<void, int> VCRActivityChanged;
};

#endif
