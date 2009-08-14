#ifndef DISABLE_NETWORK

#ifndef __setupengrab_h
#define __setupengrab_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/statusbar.h>
#include <callablemenu.h>

class eNumber;
class eButton;
class eCheckbox;
class eTextInputField;

class ENgrabSetup: public ePLiWindow, public eCallableMenu
{
	eButton *type;
	eNumber *inet_address, *srvport;
	eButton *bServerMAC;
	eTextInputField *serverMAC;
private:
	void fieldSelected(int *number);
	void okPressed();
	void detectMAC();
	void init_ENgrabSetup();
public:
	ENgrabSetup();
	~ENgrabSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};
#endif

#endif // DISABLE_NETWORK
