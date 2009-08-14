#ifndef __system_settings_h
#define __system_settings_h

#include <setup_window.h>
#include <callablemenu.h>

class eSystemSettings: public eSetupWindow, public eCallableMenu
{
private:
	void entrySelected(eListBoxEntryMenu* item);
	void init_eSystemSettings();
public:
	eSystemSettings();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

class FactoryReset : public eCallableMenu
{
	public:
		FactoryReset() {};
		
		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif /* __system_settings_h */
