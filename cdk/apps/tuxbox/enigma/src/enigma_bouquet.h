#ifndef __setup_bouquet_h
#define __setup_bouquet_h

#include <setup_window.h>
#include <callablemenu.h>

class eZapBouquetSetup: public eSetupWindow, public eCallableMenu
{
	void editSelected();
	void createNewEmptyBouquet();
	void editModeSelected();
	void lockUnlockServices();
	void init_eZapBouquetSetup();
public:
	eZapBouquetSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif //__setup_bouquet_h
