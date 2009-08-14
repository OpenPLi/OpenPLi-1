#ifndef __lib_system_info_h
#define __lib_system_info_h

#include <set>
#include <config.h>

#include <lib/dvb/serviceplaylist.h>

#if HAVE_DVB_API_VERSION < 3
#include <ost/dmx.h>
#ifndef DMX_SET_NEGFILTER_MASK
	#define DMX_SET_NEGFILTER_MASK   _IOW('o',48,uint8_t *)
#endif
#endif

class eString;

class eSystemInfo
{
	static eSystemInfo *instance;
	int hashdd, hasci, hasrfmod, haslcd, hasnetwork, haskeyboard, 
		canmeasurelnbcurrent, hwtype, fetype, hasnegfilter, 
		canupdateTransponder, canshutdown, canrecordts, defaulttimertype, 
		alphaincrement, hasstandbywakeuptimer, cantimeshift, hasscartswitch,
		hascf, hasusb, isOE;
	std::set<int> caids;
	const char *helpstr, *midstr, *modelstr, *manufactstr, *cpustr;
	eString getInfo(const char *info, bool dreambox=false);
	void init_eSystemInfo();
public:
	static eSystemInfo *getInstance() { return instance; }
	eSystemInfo();
	enum { dbox2Nokia, dbox2Sagem, dbox2Philips, DM7000, DM7020, DM5600, DM5620, DM500, DM600PVR, TR_DVB272S, DM500PLUS, Unknown };
	enum { feSatellite, feCable, feTerrestrial, feUnknown };

	const char *getHelpStr() { return helpstr; }
	const char *getmidStr() { return midstr; }
	const char *getModel() { return modelstr; }
	const char *getManufacturer() { return manufactstr; }
	const char *getCPUInfo() { return cpustr; }
	int hasNegFilter() { return hasnegfilter; }
	int hasHDD() { return hashdd; }
	int hasCI() { return hasci; }
	int hasRFMod() { return hasrfmod; }
	int hasLCD() { return haslcd; }
	int hasNetwork() { return hasnetwork; }
	int hasKeyboard() { return haskeyboard; }	
	int canMeasureLNBCurrent() { return canmeasurelnbcurrent; }
	int canShutdown() { return canshutdown; }
	int canRecordTS() { return canrecordts; }
	int canUpdateTransponder() { return canupdateTransponder; }
	int getHwType() { return hwtype; }
	int getAlphaIncrement() { return alphaincrement; }
	int getDefaultTimerType() { return defaulttimertype; }
	int getFEType() { return fetype; }
	int hasStandbyWakeupTimer() { return hasstandbywakeuptimer; }
	int canTimeshift() { return cantimeshift; }
	int hasScartSwitch() { return hasscartswitch; }
	int hasCF() { return hascf; }
	int hasUSB() { return hasusb; }
	int isOpenEmbedded() { return isOE; }
	const std::set<int> &getCAIDs() { return caids; }
};

#endif
