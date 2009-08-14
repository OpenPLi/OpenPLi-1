#ifndef __enigma_h
#define __enigma_h

#include <lib/base/ebase.h>
#include <sselect.h>
#include <hotplug.h>

class eServiceSelector;
class eServicePath;
class eZapMain;
class eService;
class eWidget;
class eInit;
class eRCKey;
class eHTTPD;
class eHTTPConnection;
class eHTTPDynPathResolver;
class eHTTPFilePathResolver;
class eHTTPXMLRPCResolver;
class eHTTPLogResolver;

extern eWidget *currentFocus;

class eZap: public eApplication, public Object
{
	eHotplug hotplug;
	static eZap *instance;

	eWidget *desktop_fb;
//#ifndef DISABLE_LCD
	eWidget *desktop_lcd;
//#endif

	eHTTPD *httpd;
	eHTTPConnection *serialhttpd;
	eHTTPDynPathResolver *dyn_resolver;
	eHTTPFilePathResolver *fileresolver;
	eHTTPXMLRPCResolver *xmlrpcresolver;
	eHTTPLogResolver *logresolver;

	void keyEvent(const eRCKey &key);
	void status();

	eInit *init;
	eServiceSelector *serviceSelector;
	std::list<void*> plugins;

	eZapMain *main;
	void init_eZap(int argc, char **argv);
public:
	enum
	{
//#ifndef DISABLE_LCD
		desktopLCD,
//#endif
		desktopFB
	};

	eWidget *getDesktop(int nr)
	{
		switch (nr)
		{
#ifndef DISABLE_LCD
		case desktopLCD:
			return desktop_lcd;
#endif
		case desktopFB:
			return desktop_fb;
		default:
			return 0;
		}
	}
	static eZap *getInstance();
	eServiceSelector *getServiceSelector()
	{
		return serviceSelector;
	}
	void reconfigureHTTPServer();
	int tts_fd;

	eZap(int argc, char **argv);
	~eZap();
};

#endif /* __enigma_h */
