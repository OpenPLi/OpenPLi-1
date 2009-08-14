#ifndef __lib_dvb_servicestructure_h
#define __lib_dvb_servicestructure_h

#include <lib/dvb/service.h>
#include <lib/dvb/servicecache.h>

class eServiceStructureHandler: public eServiceHandler
{
	eServiceCache<eServiceStructureHandler> cache;
	void init_eServiceStructureHandler();
public:
	void loadNode(eServiceCache<eServiceStructureHandler>::eNode &node, const eServiceReference &ref);
	eService *createService(const eServiceReference &node);
	
	eServiceStructureHandler();
	~eServiceStructureHandler();

		// service list functions
	void enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback);
	void leaveDirectory(const eServiceReference &dir);
	eService* addRef(const eServiceReference&);
	void removeRef(const eServiceReference&);
	
	enum { modeRoot, modeTV, modeRadio, modeFile, modeBouquets, modeTvRadio, modeData };
	static eServiceReference getRoot(int mode)
	{
    return eServiceReference(eServiceReference::idStructure, eServiceReference::flagDirectory, mode);
	}
};

#endif
