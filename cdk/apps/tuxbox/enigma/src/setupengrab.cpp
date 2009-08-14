#ifndef DISABLE_NETWORK

#include <setupengrab.h>
#include <plugin.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <lib/gui/ePLiWindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>
#include <lib/gui/textinput.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/epgcache.h>
#include <lib/socket/socket.h>
#include <lib/base/estring.h>
#include <lib/gui/enumber.h>
#include <lib/gui/statusbar.h>
#include <lib/system/info.h>

#define MENUNAME N_("Ngrab streaming")

class ENgrabSetupFactory : public eCallableMenuFactory
{
public:
	ENgrabSetupFactory() : eCallableMenuFactory("ENgrabSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new ENgrabSetup;
	}

	bool isAvailable()
	{
		return eSystemInfo::getInstance()->hasNetwork();
	}
};

ENgrabSetupFactory ENgrabSetup_factory;

ENgrabSetup::ENgrabSetup():
	ePLiWindow(_(MENUNAME), 370)
{
	init_ENgrabSetup();
}
void ENgrabSetup::init_ENgrabSetup()
{
	struct in_addr sinet_address;
	int nsrvport;
	int de[4];

	if ( eConfig::getInstance()->getKey("/elitedvb/network/nserver", sinet_address.s_addr) )
		sinet_address.s_addr = 0xC0A80028; // 192.168.0.40
	if ( eConfig::getInstance()->getKey("/elitedvb/network/nservport", nsrvport ) )
		nsrvport = 4000;

	eLabel *l=new eLabel(this);
	l->setText(_("Server IP:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(140, widgetHeight()));

	eNumber::unpack(sinet_address.s_addr, de);
	inet_address=new eNumber(this, 4, 0, 255, 3, de, 0, l);
	inet_address->move(ePoint(160, yPos()));
	inet_address->resize(eSize(200, widgetHeight()));
	inet_address->setFlags(eNumber::flagDrawPoints);
	inet_address->setHelpText(_("Enter IP Address of the Ngrab server (0..9, left, right)"));
	inet_address->loadDeco();

	nextYPos(35);
	l=new eLabel(this);
	l->setText(_("Server port:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(140, widgetHeight()));

	srvport=new eNumber(this, 1, 0, 9999, 4, &nsrvport, 0, l);
	srvport->move(ePoint(160, yPos()));
	srvport->resize(eSize(200, widgetHeight()));
	srvport->setFlags(eNumber::flagDrawPoints);
	srvport->setHelpText(_("Enter Ngrab server port (standard is 4000)"));
	srvport->loadDeco();

	nextYPos(35);
	l=new eLabel(this);
	l->setText(_("Server MAC:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(140, widgetHeight()));

	serverMAC=new eTextInputField(this);
	serverMAC->move(ePoint(160, yPos()));
	serverMAC->resize(eSize(200, widgetHeight()));
	serverMAC->setHelpText(_("Enter MAC address of server (for Wake-on-LAN)"));
	serverMAC->setUseableChars("01234567890abcdefABCDEF:");
	serverMAC->setMaxChars(17);
	serverMAC->loadDeco();

	char* sMAC=0;
	if ( eConfig::getInstance()->getKey("/elitedvb/network/hwaddress", sMAC ) )
		serverMAC->setText("00:00:00:00:00:00");
	else
	{
		serverMAC->setText(sMAC);
		free(sMAC);
	}

	nextYPos(35);
	bServerMAC=new eButton(this);
	bServerMAC->move(ePoint(10, yPos()));
	bServerMAC->resize(eSize(350, 40));
	bServerMAC->setShortcut("blue");
	bServerMAC->setShortcutPixmap("blue");
	bServerMAC->setText(_("Detect MAC address"));
	bServerMAC->setHelpText(_("Try to autodetect server MAC address"));
	bServerMAC->loadDeco();
	CONNECT( bServerMAC->selected, ENgrabSetup::detectMAC );

	buildWindow();
	CONNECT(bOK->selected, ENgrabSetup::okPressed);

	/* help text for Ngrab streaming setup */
	setHelpText(_("\tNgrab Streaming Setup\n\n>>> [MENU] >>> [6] Setup >>> [6] Expert Setup\n>>> [2] Ngrab Streaming Setup\n. . . . . . . . . .\n\n" \
		"When using NGrab for recording purposes, you will need to do some network settings here\n. . . . . . . . . .\n\n" \
		"Usage:\n\n[UP]/[DOWN]\tSelect Inputfield or Button\n\nSrv IP:\n[NUMBERS]\tIP-Address of your Computer\n\n" \
		"Srv Port:\n[NUMBERS]\tTCP/IP Port to be used by NGrab\n\t(default 4000)\n\n[GREEN]\tSave Settings and Close Window\n\n" \
		"[RED]/[EXIT]\tClose window without saving changes"));
}

ENgrabSetup::~ENgrabSetup()
{
}

void ENgrabSetup::okPressed()
{
	int einet_address[4];
	int nsrvport;

	struct in_addr sinet_address;

	for (int i=0; i<4; i++)
		einet_address[i] = inet_address->getNumber(i);

	eNumber::pack(sinet_address.s_addr, einet_address);

	nsrvport = srvport->getNumber();

	eDebug("write ip = %04x, port = %d", sinet_address.s_addr, nsrvport );
	eConfig::getInstance()->setKey("/elitedvb/network/nserver", sinet_address.s_addr );
	eConfig::getInstance()->setKey("/elitedvb/network/nservport", nsrvport);
	eConfig::getInstance()->setKey("/elitedvb/network/hwaddress", serverMAC->getText().c_str() );
	eConfig::getInstance()->flush();

	close(0);
}

void ENgrabSetup::detectMAC()
{
	eString serverip;

	serverip.sprintf("%d.%d.%d.%d",
		inet_address->getNumber(0),
		inet_address->getNumber(1),
		inet_address->getNumber(2),
		inet_address->getNumber(3) );

	if ( system(eString().sprintf("ping -c 2 %s",serverip.c_str()).c_str()) == 0 )
	{
		FILE *f = fopen("/proc/net/arp", "r");
		if ( f )
		{
			char line[1024];
			fgets(line, 1024, f);
			int HWAddrPos = strstr(line, "HW address") - line;
			if ( HWAddrPos  <  0)
			{
				fclose(f);
				return;
			}
			while (1)
			{
				if (!fgets(line, 1024, f))
					break;
				if ( strstr(line, serverip.c_str() ) )
				{
					serverMAC->setText( eString(line+HWAddrPos,17) );
					break;
				}       
			}
			fclose(f);
		}
	}
	else
	{
		hide();
		eMessageBox::ShowBox(
			_("Please check your NGrab Server or the IP"),
			_("HW Address(MAC) detection failed"),
			eMessageBox::btOK|eMessageBox::iconInfo );
		show();
	}
}

void ENgrabSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

#endif // DISABLE_NETWORK
