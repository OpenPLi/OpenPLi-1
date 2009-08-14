#ifndef __time_settings_h
#define __time_settings_h

#include <setup_window.h>
#include <callablemenu.h>

class eTimeSettings: public eSetupWindow, public eCallableMenu
{
private:
	void time_zone();
	void time_correction();
	void selClockTypeChanged(eListBoxEntryMenu* e);
	void selTimesourceChanged(eListBoxEntryMenu* e);
	
	int i12hourClock;
	int systemTime;
	eListBoxEntryMulti *clockType;
	eListBoxEntryMulti *timeSource;

	void init_eTimeSettings();
public:
	eTimeSettings();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif /* __time_settings_h */

