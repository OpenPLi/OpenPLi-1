#ifndef __testpicture_h
#define __testpicture_h

#include <lib/gui/ewidget.h>
#include <lib/gdi/grc.h>
#include <lib/base/ebase.h>

class eLabel;

class eTestPicture: public eWidget
{
	gColor red, green, blue, white, black;
	gColor basic[8];
	gColor gray;
	int testmode;
	eLabel *description;
	eTimer desctimer;
	void hideDesc();
	void init_eTestPicture();
protected:
	int eventHandler(const eWidgetEvent &event);
	void redrawWidget(gPainter *target, const eRect &area);
public:
	enum { 
		testColorbar, 
		testRed, 
		testGreen, 
		testBlue,
		testWhite,
		testFUBK, // without circle ;)
		testGray,
		testBlack};
	eTestPicture(int testmode);
	static int display(int mode);
};

#endif
