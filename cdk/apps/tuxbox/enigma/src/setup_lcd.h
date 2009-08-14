#ifndef DISABLE_LCD

#ifndef __setuplcd_h
#define __setuplcd_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/statusbar.h>
#include <callablemenu.h>

class eLabel;
class eButton;
class eSlider;
class eCheckbox;

class eZapLCDSetup: public ePLiWindow, public eCallableMenu
{
	eSlider *p_brightness, *p_contrast, *p_standby;
	eLabel *bbrightness, *bcontrast, *bstandby;
	eCheckbox* inverted, *shortnames;
	
	int lcdbrightness;
	int lcdcontrast;
	int lcdstandby;

	void okPressed();
	int eventHandler( const eWidgetEvent&);
	void brightnessChanged( int );
	void contrastChanged( int );
	void standbyChanged( int );
	void invertedChanged( int );
	void update(int brightness, int contrast);
	void init_eZapLCDSetup();
public:
	eZapLCDSetup();
	~eZapLCDSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif

#endif //DISABLE_LCD
