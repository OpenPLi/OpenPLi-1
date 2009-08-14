#ifndef DISABLE_FILE

#ifndef __lib_dvb_servicejpg_h
#define __lib_dvb_servicejpg_h

#include <lib/dvb/service.h>
#include <lib/base/buffer.h>

#include <lib/base/ebase.h>
#include <lib/base/thread.h>
#include <lib/base/message.h>

class eServiceHandlerJPG: public eServiceHandler
{
	void addFile(void *node, const eString &filename);
	int state;
	eServiceReference runningService;
public:
	int getID() const;
	int play(const eServiceReference &service, int workaround = 0);
	int stop(int workaround = 0);

	eServiceHandlerJPG();
	~eServiceHandlerJPG();

	eService *addRef(const eServiceReference &service);
	void removeRef(const eServiceReference &service);
};
#endif
#endif //DISABLE_FILE
