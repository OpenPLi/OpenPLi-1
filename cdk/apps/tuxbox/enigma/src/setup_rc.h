#ifndef __setuprc_h
#define __setuprc_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/statusbar.h>
#include <callablemenu.h>

class eLabel;
class eButton;
class eSlider;
class eComboBox;
class eListBoxEntryText;
class eCheckbox;
class eNumber;

class eZapRCSetup: public ePLiWindow, public eCallableMenu
{
	eSlider *srrate, *srdelay;
	eLabel *lrrate, *lrdelay, *lrcStyle, *lNextCharTimeout, *lrcChannel;
	eNumber *NextCharTimeout;
	eComboBox* rcStyle;
	eComboBox* rcChannel;
	eString curstyle;

	int rdelay;
	int rrate;
	           
	void okPressed();
	int eventHandler( const eWidgetEvent& );
	void repeatChanged( int );
	void delayChanged( int );
	void styleChanged( eListBoxEntryText* );
	void update();
	void setStyle();
	void nextField(int *);
	void init_eZapRCSetup();
public:
	eZapRCSetup();
	~eZapRCSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif
