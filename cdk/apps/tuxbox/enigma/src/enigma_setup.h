#ifndef __enigma_setup_h
#define __enigma_setup_h

#include <lib/gui/ProgressSetupWindow.h>
#include <callablemenu.h>

class eZapSetup: public ProgressSetupWindow
{
	private:
		void updateProgressBar();
		int getFsFullPerc(const char* filesystem);
		void entrySelected(eListBoxEntryMenu* item);
		void init_eZapSetup();
		
	public:
		eZapSetup();
};

class eZapSetupWrapper : public eCallableMenu
{
	public:
		eZapSetupWrapper() {};

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

class eSoftwareManagement : public eCallableMenu
{
	public:
		eSoftwareManagement() {};
		
		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif /* __enigma_setup_h */
