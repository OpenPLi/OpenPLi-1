#ifndef __setupvideo_h
#define __setupvideo_h

#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/statusbar.h>
#include <callablemenu.h>

class eNumber;
class eButton;
class eCheckbox;

class eZapVideoSetup: public eWindow, public eCallableMenu
{
	eButton *ok, *tuxtxtpos, *testpicture;
	eStatusBar *status;
	eCheckbox *c_disableWSS, *ac3default, *VCRSwitching;
	eListBox<eListBoxEntryText> *colorformat, *pin8, *tvsystem;

	unsigned int v_colorformat, v_pin8, v_disableWSS, v_tvsystem, v_VCRSwitching, Wizard;
	int off_top, off_bottom, off_left, off_right;
	eStatusBar *statusbar;
private:
	void ac3defaultChanged( int i );
	void VPin8Changed( eListBoxEntryText *);
	void DisableWSSChanged(int);
	void TVSystemChanged( eListBoxEntryText * );
	void CFormatChanged( eListBoxEntryText * );
	void ColTVChanged();
	void VCRChanged(int);	
	void okPressed();
	void TuxtxtPosition();
	void showTestpicture();
	int eventHandler( const eWidgetEvent &e );
	void init_eZapVideoSetup();
public:
	eZapVideoSetup(int wizardmode = 0);
	~eZapVideoSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif
