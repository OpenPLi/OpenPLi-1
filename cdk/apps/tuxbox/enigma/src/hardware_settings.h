#ifndef __hardware_settings_h
#define __hardware_settings_h

#include <setup_window.h>
#include <callablemenu.h>

class eHardwareSettings: public eSetupWindow, public eCallableMenu
{
	public:
	eHardwareSettings();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
	
	private:
	void entrySelected(eListBoxEntryMenu* item);
};

#endif /* __hardware_settings_h */
