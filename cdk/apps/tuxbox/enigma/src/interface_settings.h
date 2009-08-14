#ifndef __interface_settings_h
#define __interface_settings_h

#include <setup_window.h>
#include <callablemenu.h>

class eInterfaceSettings: public eSetupWindow, public eCallableMenu
{
	public:
		eInterfaceSettings();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);

	private:
		void entrySelected(eListBoxEntryMenu* item);
};

#endif /* __interface_settings_h */
