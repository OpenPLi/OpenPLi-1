#ifndef __lib_dvb_servicefile_h
#define __lib_dvb_servicefile_h

#include <lib/dvb/service.h>
#include <lib/dvb/servicecache.h>
#include <list>

class eServiceFileHandler: public eServiceHandler
{
	eServiceCache<eServiceFileHandler> cache;
	static eServiceFileHandler *instance; 
	eServiceReference result;

public:

	Signal2<void,void*,const eString &> fileHandlers, directoryHandlers;
	void addReference(void *node, const eServiceReference &ref);
	
	static eServiceFileHandler *getInstance() { return instance; }
	void loadNode(eServiceCache<eServiceFileHandler>::eNode &node, const eServiceReference &ref);
	eService *createService(const eServiceReference &node);
	
	eServiceFileHandler();
	~eServiceFileHandler();

	// service list functions
	void enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback);
	void leaveDirectory(const eServiceReference &dir);

	int deleteService(const eServiceReference &dir, const eServiceReference &ref);

	eService *addRef(const eServiceReference &service);
	void removeRef(const eServiceReference &service);

	int lookupService(eServiceReference &, const char *filename);
	
private:

	int getFsFullPerc(const char* filesystem);
};

#endif
