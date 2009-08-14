#ifndef __lib_system_dmfp_h
#define __lib_system_dmfp_h

class eDreamboxFP
{
public:
	static int isUpgradeAvailable();
	static int doUpgrade(int magic);
	static int getFPVersion();
};

#endif
