#ifndef __helpwindow_h
#define __helpwindow_h

#include <lib/gui/ewindow.h>

class eLabel;
class eProgress;

class eHelpWindow: public eWindow
{
	int entryBeg[255];
	int cur, lastEntry;
	eLabel *label;
	eWidget *scrollbox, *visible;
	eProgress *scrollbar;
	int curPage;
	bool doscroll;
	int eventHandler(const eWidgetEvent &event);
	void updateScrollbar();
	void init_eHelpWindow(ePtrList<eAction> &parseActionHelpList, int helpID);
public:
	eHelpWindow(ePtrList<eAction> &parseActionHelpList, eString &helptext);
	~eHelpWindow();
};

#endif


