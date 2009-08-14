#ifndef DISABLE_NETWORK

#ifndef __setupnetwork_h
#define __setupnetwork_h

#include <lib/gui/ePLiWindow.h>
#include "enigma_mount.h"
#include <callablemenu.h>

class eNumber;
class eButton;
class eCheckbox;
class eComboBox;
class eTextInputField;
class eListBoxEntryText;

class eZapNetworkSetup: public ePLiWindow, public eCallableMenu
{
	eNumber *ip, *netmask, *dns, *gateway;
	eCheckbox *dosetup, *dhcp;
	eLabel *lNameserver, *lGateway;
	//eComboBox *combo_type;
	eStatusBar *statusbar;

#ifdef ENABLE_PPPOE
	eButton *tdsl;
	eCheckbox *rejectTelnet, *rejectWWW, *rejectSamba, *rejectFTP;
	eTextInputField *login, *password;
	eLabel *lLogin, *lPassword;
	eString secrets;
#endif
	
private:
	void fieldSelected(int *number);
	void okPressed();
	void abortPressed();
	void dhcpStateChanged(int);
#ifdef ENABLE_PPPOE
	void typeChanged(eListBoxEntryText*);
	void passwordSelected();
	void loginSelected();
	void tdslPressed();
#endif
	void init_eZapNetworkSetup();
public:
	static void getNameserver(__u32 &ip); 
	static void getDefaultGateway(__u32 &ip);
	static void getIP(char *dev, __u32 &ip, __u32 &mask);

	eZapNetworkSetup();
	~eZapNetworkSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif // __setupnetwork_h
#endif // DISABLE_NETWORK
