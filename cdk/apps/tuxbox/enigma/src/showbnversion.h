#ifndef __showbnversion_h_
#define __showbnversion_h_

#include <lib/gui/ewindow.h>

class eLabel;
class BNDirectory;
class eDVBEvent;

class ShowBNVersion: public eWindow
{
	eLabel *text, *res1, *res2;
	BNDirectory *bnd[2];
	void init_ShowBNVersion();
protected:
	void willShow();
	void willHide();
	int eventHandler(const eWidgetEvent &event);
private:
	void eventOccured(const eDVBEvent &event);
public:
	ShowBNVersion();
	~ShowBNVersion();
};

#endif
