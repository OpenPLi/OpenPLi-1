#ifndef __download_h
#define __download_h

#include <lib/system/httpd.h>
#include <lib/gui/ewindow.h>
	
class eDownloadWindow: public eWindow
{
	eHTTPConnection *c;
	eString url;
	int eventFilter(const eWidgetEvent &event);
private:
	void httpDone();
public:
	eDownloadWindow(const char *url);
	~eDownloadWindow();
};

#endif
