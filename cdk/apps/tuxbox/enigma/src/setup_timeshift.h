/*************************************************************************
* THIS CODE IS INACTIVE AND WILL NOT BE COMPILED
* IT'S ONLY FOR CVS MERGING PURPOSES STILL HERE
*************************************************************************/

#ifndef DISABLE_HDD
#ifndef DISABLE_FILE

#ifndef __lib_apps_enigma_setup_timeshift_h
#define __lib_apps_enigma_setup_timeshift_h

#include <lib/gui/ewindow.h>
#include <lib/gui/statusbar.h>

class eButton;
class eCheckbox;
class eNumber;
class eTextInputField;

class eZapTimeshiftSetup: public eWindow
{
	
	eNumber *delay;
	eNumber *minutes;
	eCheckbox* active;
	eCheckbox* pause;
	eTextInputField* path;
	eButton *seldir;
	eButton *store;
	eStatusBar* sbar;
	void init_eZapTimeshiftSetup();
private:
	void storePressed();
	void selectDir();

public:
	eZapTimeshiftSetup();
	~eZapTimeshiftSetup();
};

#endif

#endif
#endif
