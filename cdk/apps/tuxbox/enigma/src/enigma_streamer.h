#ifndef __enigma_streamer__
#define __enigma_streamer__

#include <lib/base/estring.h>
#include <lib/dvb/dvb.h>

class eStreamer
{
private:
	static eStreamer *instance;
	eServiceReference sref;
public:
	eStreamer();
	~eStreamer();

	bool getServiceReference(eServiceReference&);
	void setServiceReference(eServiceReference);
	static eStreamer *getInstance() {return instance;}
};
#endif

