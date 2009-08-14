#ifndef __software_update_h
#define __software_update_h

#include <setup_window.h>
#include <callablemenu.h>

class eSoftwareUpdate: public eSetupWindow, public eCallableMenu
{
private:
#ifndef DISABLE_NETWORK
	void internet_update();
	void manual_update();
#ifdef ENABLE_FLASHTOOL
	void flash_tool();
#endif
	void stop_mode();
#endif
//	void satellite_update();
	void init_eSoftwareUpdate();
public:
	eSoftwareUpdate();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif /* __software_update_h */
