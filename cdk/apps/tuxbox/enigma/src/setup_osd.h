#ifndef __setuposd_h
#define __setuposd_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/emessage.h>
#include <lib/gui/slider.h>
#include <callablemenu.h>

class eZapOsdSetup: public ePLiWindow, public eCallableMenu
{
private:
	eSlider *sAlpha, *sBrightness, *sGamma, *sSubtitleBlack;

	int alpha, brightness, gamma;
	int eventHandler(const eWidgetEvent&);
	void alphaChanged( int );
	void brightnessChanged( int );
	void gammaChanged( int );
	void SubtitleBlackChanged( int );
	void okPressed();
	void init_eZapOsdSetup();
public:
	eZapOsdSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

class PluginOffsetScreen: public eWidget, public eCallableMenu
{
	enum { posLeftTop, posRightBottom } curPos;
	eLabel *descr;
	eLabel *cursize;
	int eventHandler( const eWidgetEvent & e );
	int left, top, right, bottom;
	void redrawLeftTop( gPainter *target );
	void redrawRightBottom( gPainter *target );
	void redrawWidget(gPainter *target, const eRect &where);
	gColor foreColor, backColor;

public:
	PluginOffsetScreen();
	void init_PluginOffsetScreen();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif
