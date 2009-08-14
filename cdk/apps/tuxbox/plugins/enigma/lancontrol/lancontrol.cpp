/* 
LAN-Control Plugin for Enigma 1
Copyright (C) 2007 Dre (dre@drecomx.net) [http://dreambox.funfiles.cc]

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef DISABLE_NETWORK

#include <lancontrol.h>

#define MAX_ENTRIES 8
#define VERNUM "0.40"

extern "C" int plugin_exec( PluginParam *par );

//start plugin
int plugin_exec( PluginParam *par )
{
	eWake dlg;
	dlg.show();
	dlg.exec();
	dlg.hide();
	
	return 0;
}

//main GUI
eWake::eWake(): eWindow(1)
{
	cmove(ePoint(100, 100));
	cresize(eSize(520, 400));
	curEntry=0;
	
	headline.sprintf(_("LAN-Control %s - Server Profile %d/%d"), VERNUM, curEntry + 1, MAX_ENTRIES);
	setText(headline);

	struct in_addr sinet_address;
	int de[4];
	int fd=eSkin::getActive()->queryValue("fontsize", 20);


	
	//Check config for stored IP-address. Else: use 192.168.0.40
	cmd.sprintf("/elitedvb/network/server%d/", curEntry+1);
	if ( eConfig::getInstance()->getKey((cmd+"smbsrv").c_str(), sinet_address.s_addr) )
	sinet_address.s_addr = 0xC0A80028; // 192.168.0.40
	

	//Description for entryfields for IP-address
	eLabel *IP=new eLabel(this);
	IP->setText(_("IP:"));
	IP->move(ePoint(20, 20));
	IP->resize(eSize(140, fd+4));

	//Entryfields for IP-address
	eNumber::unpack(sinet_address.s_addr, de);
	inet_address=new eNumber(this, 4, 0, 255, 3, de, 0, IP);
	inet_address->move(ePoint(160, 20));
	inet_address->resize(eSize(200, fd+10));
	inet_address->setFlags(eNumber::flagDrawPoints);
	inet_address->setHelpText(_("Key in server's IP-address (0..9, left, right)"));
	inet_address->loadDeco();

	//Description for entryfield MAC-address
	MAC=new eLabel(this);
	MAC->setText(_("MAC-Address:"));
	MAC->move(ePoint(20,60));
	MAC->resize(eSize(140, fd+4));

	//Entryfield for MAC-address
	serverMAC=new eTextInputField(this);
	serverMAC->move(ePoint(160,60));
	serverMAC->resize(eSize(200, fd+10));
	serverMAC->setHelpText(_("enter MAC address of server (for wake on lan)"));
	serverMAC->setUseableChars("01234567890abcdefABCDEF:");
	serverMAC->setMaxChars(17);
	serverMAC->loadDeco();

	//Key in stored MAC-address. Else: use 00:00:00:00:00:00
	char* sMAC=0;
	if ( eConfig::getInstance()->getKey((cmd+"smbmac").c_str(), sMAC ) )
		serverMAC->setText("00:00:00:00:00:00");
	else
	{
		serverMAC->setText(sMAC);
		free(sMAC);
	}
	
	//Search MAC-address
	bt_MAC=new eButton(this);
	bt_MAC->move(ePoint(370,60));
	bt_MAC->resize(eSize(120,30));
	bt_MAC->setText(_("Search MAC"));
	bt_MAC->setHelpText(_("try to autodetect server MAC address"));
	bt_MAC->loadDeco();
	CONNECT( bt_MAC->selected, eWake::detectMAC );

	//Network-Settings
	bt_NET = new eButton(this);
	bt_NET->move(ePoint(10,220));
	bt_NET->resize(eSize(160,30));
	bt_NET->setText(_("Network"));
	bt_NET->setHelpText(_("Network-Settings"));
	bt_NET->loadDeco();
	CONNECT(bt_NET->selected, eWake::communication_setup2);

	//Lan-Control-Settings
	bt_CONFIG = new eButton(this);
	bt_CONFIG->move(ePoint(180, 220));
	bt_CONFIG->resize(eSize(160, 30));
	bt_CONFIG->setText(_("Settings"));
	bt_CONFIG->setHelpText(_("LAN-Control settings"));
	bt_CONFIG->loadDeco();
	CONNECT(bt_CONFIG->selected, eWake::showSettingsMenu);

	//NGrab-Settings
	bt_NGRAB = new eButton(this);
	bt_NGRAB->move(ePoint(350,220));
	bt_NGRAB->resize(eSize(160,30));
	bt_NGRAB->setText(_("NGrab"));
	bt_NGRAB->setHelpText(_("NGrab-Einstellungen"));
	bt_NGRAB->loadDeco();
	CONNECT(bt_NGRAB->selected, eWake::ngrab_setup);

	//Query status
	bt_STATUS = new eButton(this);
	bt_STATUS->move(ePoint(10, 260));
	bt_STATUS->resize(eSize(160, 30));
	bt_STATUS->setShortcut("blue");
	bt_STATUS->setShortcutPixmap("blue");
	bt_STATUS->setText(_("Server-Status"));
	bt_STATUS->setHelpText(_("Query server's status"));
	bt_STATUS->loadDeco();
	CONNECT(bt_STATUS->selected, eWake::status);

	//Previous entry
	bt_PREV = new eButton(this);
	bt_PREV->move(ePoint(180, 260 ));
	bt_PREV->resize(eSize(30, 30));
	bt_PREV->setText("<");
	bt_PREV->setHelpText(_("go to previous share"));
	bt_PREV->loadDeco();
	CONNECT(bt_PREV->selected, eWake::prevPressed);

	//Next entry
	bt_NEXT = new eButton(this);
	bt_NEXT->move(ePoint(310, 260 ));
	bt_NEXT->resize(eSize(30, 30));
	bt_NEXT->setText(">");
	bt_NEXT->loadDeco();
	bt_NEXT->setHelpText(_("go to next share"));
	CONNECT(bt_NEXT->selected, eWake::nextPressed);

	//Show Mounts
	bt_MOUNTS = new eButton(this);
	bt_MOUNTS->move(ePoint(350, 260 ));
	bt_MOUNTS->resize(eSize(160, 30 ));
	bt_MOUNTS->setText(_("mounts"));
	bt_MOUNTS->setHelpText(_("show mounts"));
	bt_MOUNTS->loadDeco();
	CONNECT(bt_MOUNTS->selected, eWake::communication_setup);

	//save settings
	bt_OK=new eButton(this);
	bt_OK->move(ePoint(10,300));
	bt_OK->resize(eSize(160, 30));
	bt_OK->setShortcut("green");
	bt_OK->setShortcutPixmap("green");
	bt_OK->setText(_("save"));
	bt_OK->setHelpText(_("save changes and return"));
	bt_OK->loadDeco();
	CONNECT(bt_OK->selected, eWake::saveSettings);
	
	//stop server
	bt_STOP=new eButton(this);
	bt_STOP->move(ePoint(180,300)); 
	bt_STOP->resize(eSize(160, 30));
	bt_STOP->setShortcut("red");
	bt_STOP->setShortcutPixmap("red");
	bt_STOP->setText(_("stop server"));
	bt_STOP->setHelpText(_("stop selected server"));
	bt_STOP->loadDeco();
	CONNECT(bt_STOP->selected, eWake::stopServer);

	//start server
	bt_START=new eButton(this);
	bt_START->move(ePoint(350, 300));
	bt_START->resize(eSize(160, 30));
	bt_START->setShortcut("yellow");
	bt_START->setShortcutPixmap("yellow");
	bt_START->setText(_("start server"));
	bt_START->setHelpText(_("start selected server"));
	bt_START->loadDeco();
	CONNECT(bt_START->selected, eWake::startServer);

	//Statusbar
	statusbar=new eStatusBar(this);
	statusbar->move( ePoint(0, clientrect.height()-50 ) );
	statusbar->resize( eSize( clientrect.width(), 50) );
	statusbar->loadDeco();
	
	setHelpID(155);

}

//Save settings
void eWake::saveSettings()
{	
	curEntry = ((curEntry + 1) % MAX_ENTRIES);
	int einet_address[4];

	struct in_addr sinet_address;

	for (int i=0; i<4; i++)
		einet_address[i] = inet_address->getNumber(i);

	eNumber::pack(sinet_address.s_addr, einet_address);

	eDebug("write ip = %04x", sinet_address.s_addr);

	cmd.sprintf("/elitedvb/network/server%d/", curEntry);
	eConfig::getInstance()->setKey((cmd+"smbsrv").c_str(), sinet_address.s_addr );
	eConfig::getInstance()->setKey((cmd+"smbmac").c_str(), serverMAC->getText().c_str() );
	eConfig::getInstance()->flush();

		eMessageBox mb_stored(
			_("Settings saved"),
			_(eString().sprintf(_("Settings saved")).c_str()),
			eMessageBox::btOK|eMessageBox::iconInfo );
		hide();
		mb_stored.show();
		mb_stored.exec();
		mb_stored.hide();
		show();
}

//search MAC-address
void eWake::detectMAC()
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
		eMessageBox mb(
			_("The MAC-address could not be queried. Check, if your server is running and if the IP-address is correct!"),
			_("MAC-address not found"),
			eMessageBox::btOK|eMessageBox::iconInfo );
		hide();
		mb.show();
		mb.exec();
		mb.hide();
		show();
	}
}

//load config

void eWake::getConf()
{
	int de[4],i;

	//Check config for stored IP-address. Else: use 192.168.0.40
	struct in_addr sinet_address;


	//load IP-address
	cmd.sprintf("/elitedvb/network/server%d/", curEntry+1);
	if ( eConfig::getInstance()->getKey((cmd+"smbsrv").c_str(), sinet_address.s_addr) )
		sinet_address.s_addr = 0xC0A80028; // 192.168.0.40

	eNumber::unpack(sinet_address.s_addr, de);
	
	for(i=0;i<4;i++)
		inet_address->setNumber(i, de[i]);

	//load MAC-address
	char* sMAC=0;
	if ( eConfig::getInstance()->getKey((cmd+"smbmac").c_str(), sMAC ) )
		serverMAC->setText("00:00:00:00:00:00");
	else
	{
		serverMAC->setText(sMAC);
		free(sMAC);
	}
}

//start server
void eWake::startServer()
{
	eString cmd;
	if (eSystemInfo::getInstance()->isOpenEmbedded())
	{
		cmd = "/usr/bin/wakelan -m";
	}
	else
	{
		cmd = "/bin/etherwake -b";
	}
	system(eString().sprintf("%s %s", cmd.c_str(), serverMAC->getText().c_str()).c_str());
	
	eMessageBox mb(
	_("The command for booting the server has been sent."),
	_("Server-Start"),
	eMessageBox::btOK|eMessageBox::iconInfo );
	hide();
	mb.show();
	mb.exec();
	mb.hide();
	show();
}

//stop server
void eWake::stopServer()
{
	// get BoxType to determine directory
	eWakeHelper BoxType;

	path = BoxType.getBoxType();

	system(eString().sprintf("%s/etc/shutdown%d.sh", path.c_str(), curEntry+1).c_str() );
	
	eMessageBox shutdown( _("The command for shutting down the server has been sent."), _("Server-Stop"), eMessageBox::btOK|eMessageBox::iconInfo );
	hide();
	shutdown.show();
	shutdown.exec();
	shutdown.hide();
	show();

}

//Query server status
void eWake::status()
{
	eString view;

	eString temp;

	eString serverip;
	serverip.sprintf("%d.%d.%d.%d",
		inet_address->getNumber(0),
		inet_address->getNumber(1),
		inet_address->getNumber(2),
		inet_address->getNumber(3) );
	
	if ( system(eString().sprintf("ping -c 2 %s",serverip.c_str()).c_str()) == 0 )
	{
	eMessageBox mb(_("The server is running"),_("Server-Status"), eMessageBox::btOK|eMessageBox::iconInfo );
	hide();
	mb.show();
	mb.exec();
	mb.hide();
	show();
	}
	else
	{
	eMessageBox mb(_("The server is not running. If the server is running, there could be a network problem"),_("Server-Status"), eMessageBox::btOK|eMessageBox::iconInfo );
	hide();
	mb.show();
	mb.exec();
	mb.hide();
	show();
	}
	
	setFocus(bt_STATUS);
}

//go to previous entry
void eWake::prevPressed()
{
	curEntry = ((curEntry + MAX_ENTRIES - 1) % MAX_ENTRIES);
	getConf();
	headline.sprintf(_("LAN-Control %s - Server Profile %d/%d"), VERNUM, curEntry + 1, MAX_ENTRIES);
	setText(headline);
	setFocus(bt_PREV);
}

//go to next entry
void eWake::nextPressed()
{
	curEntry = ((curEntry + 1) % MAX_ENTRIES);
	getConf();
	headline.sprintf(_("LAN-Control %s - Server Profile %d/%d"), VERNUM, curEntry + 1, MAX_ENTRIES);
	setText(headline);
	setFocus(bt_NEXT);
}

//call mounts
void eWake::communication_setup()
{
	hide();
	int result = 1;
	while(result == 1)
	{
		int nrOfMounts = eNetworkMountMgr::getInstance()->mountPointCount();
		if(nrOfMounts > 8)
		{
			nrOfMounts = 8;
		}
		
		eMountSetup* setup = new eMountSetup(nrOfMounts);
#ifndef DISABLE_LCD
		setup->setLCD(LCDTitle, LCDElement);
#endif
		setup->show();
		result = setup->exec();
		setup->hide();
		delete setup;
	}
	
	show();
}

//call network settings
void eWake::communication_setup2()
{
	hide();
	eZapNetworkSetup setup;
#ifndef DISABLE_LCD
 	setup.setLCD(LCDTitle, LCDElement);
 #endif
 	setup.show();
 	setup.exec();
 	setup.hide();
 	show();
}

//call ngrab setup
void eWake::ngrab_setup()
{
	hide();
	ENgrabSetup setup;
#ifndef DISABLE_LCD
	setup.setLCD(LCDTitle, LCDElement);
#endif
	setup.show();
	setup.exec();
	setup.hide();
	show();
}

//call configuration window
void eWake::showSettingsMenu()
{
	hide();
	eWakeConf setup(curEntry);
#ifndef DISABLE_LCD
	setup.setLCD(LCDTitle, LCDElement);
#endif
	setup.show();
	setup.exec();
	setup.hide();
	show();
}

eWake::~eWake()
{
}

eWakeConf::eWakeConf(int curEntry): eWindow(1), loadTimer(0)
{
	cmove(ePoint(200, 100));
	cresize(eSize(320, 280));
	setText(_("LAN-Control Settings"));

	//Timer to prevent entries for cbb_RSA after having loaded GUI (otherwise enigma crashes)
	loadTimer = new eTimer(eApp);
	CONNECT(loadTimer->timeout, eWakeConf::getStoredKeys);
	loadTimer->start(1000);

	//Label for Combobox cbb_SERVER_OS
	lb_SERVER_OS = new eLabel(this);
	lb_SERVER_OS->move(ePoint(20, 20));
	lb_SERVER_OS->resize(eSize(140, 30));
	lb_SERVER_OS->setText(_("Server OS:"));

	//Combobox to select server os
	cbb_SERVER_OS = new eComboBox(this, 2, lb_SERVER_OS);
	cbb_SERVER_OS->move(ePoint(160, 20));
	cbb_SERVER_OS->resize(eSize(140, 30));
	cbb_SERVER_OS->setHelpText(_("Push OK to select server OS"));
	cbb_SERVER_OS->loadDeco();

	new eListBoxEntryText( *cbb_SERVER_OS, "Linux", (void*)0, 0, "Linux");
	new eListBoxEntryText( *cbb_SERVER_OS, "Windows", (void*)1, 0, "Windows");

	CONNECT(cbb_SERVER_OS->selchanged, eWakeConf::changeOS);

	//Label for Combobox cbb_PROFILE_NO
	lb_PROFILE_NO = new eLabel(this);
	lb_PROFILE_NO->move(ePoint(20, 60));
	lb_PROFILE_NO->resize(eSize(140, 30));
	lb_PROFILE_NO->setText(_("Profile Number:"));

	//Combobox to select profile number
	cbb_PROFILE_NO = new eComboBox(this, 4, lb_PROFILE_NO);
	cbb_PROFILE_NO->move(ePoint(160, 60));
	cbb_PROFILE_NO->resize(eSize(140, 30));
	cbb_PROFILE_NO->setHelpText(_("Push OK to select the profile the script shall be created for"));
	cbb_PROFILE_NO->loadDeco();

	new eListBoxEntryText( *cbb_PROFILE_NO, "1", (void*)0, 0, "Profile 1 will be used");
	new eListBoxEntryText( *cbb_PROFILE_NO, "2", (void*)1, 0, "Profile 2 will be used");
	new eListBoxEntryText( *cbb_PROFILE_NO, "3", (void*)2, 0, "Profile 3 will be used");
	new eListBoxEntryText( *cbb_PROFILE_NO, "4", (void*)3, 0, "Profile 4 will be used");
	new eListBoxEntryText( *cbb_PROFILE_NO, "5", (void*)4, 0, "Profile 5 will be used");
	new eListBoxEntryText( *cbb_PROFILE_NO, "6", (void*)5, 0, "Profile 6 will be used");
	new eListBoxEntryText( *cbb_PROFILE_NO, "7", (void*)6, 0, "Profile 7 will be used");
	new eListBoxEntryText( *cbb_PROFILE_NO, "8", (void*)7, 0, "Profile 8 will be used");

	cbb_PROFILE_NO->setCurrent(curEntry, false);

	CONNECT(cbb_PROFILE_NO->selchanged, eWakeConf::changeProfile);

	//Label for Inputfield USER
	lb_USER = new eLabel(this);
	lb_USER->move(ePoint(20, 100));
	lb_USER->resize(eSize(140, 30));
	lb_USER->setText(_("Username"));


	//Inputfield to type in username
	tf_USER = new eTextInputField(this);
	tf_USER->move(ePoint(160, 100));
	tf_USER->resize(eSize(140, 30));
	tf_USER->setHelpText(_("Push OK to type in username"));
	tf_USER->loadDeco();

	lb_dummy = new eLabel(this);
	lb_dummy->move(ePoint(20, 300));
	lb_dummy->resize(eSize(380, 30));

	//Label for Combobox cbb_RSA
	lb_RSA = new eLabel(this);
	lb_RSA->move(ePoint(20, 140));
	lb_RSA->resize(eSize(140, 30));
	lb_RSA->setText(_("Key"));

	//Combobox to select key
	cbb_RSA = new eComboBox(this, 4, lb_RSA);
	cbb_RSA->move(ePoint(160, 140));
	cbb_RSA->resize(eSize(140, 30));
	cbb_RSA->setHelpText(_("Push OK to select the key"));
	cbb_RSA->loadDeco();

	// get BoxType to determine directory
	eWakeHelper BoxType;
	path = BoxType.getBoxType();

	//Button to write the script
	bt_SCRIPT = new eButton(this);
	bt_SCRIPT->move(ePoint(20, 180));
	bt_SCRIPT->resize(eSize(140, 30));
	bt_SCRIPT->setText(_("Write script"));
	bt_SCRIPT->setHelpText(_("Push button to generate shutdown script"));
	bt_SCRIPT->setShortcut("blue");
	bt_SCRIPT->setShortcutPixmap("blue");
	bt_SCRIPT->loadDeco();
	bt_SCRIPT->hide();

	CONNECT(bt_SCRIPT->selected, eWakeConf::writeScript);

	//get the IP-address for the initial profile
	getServerIP();

	//Statusbar
	statusbar=new eStatusBar(this);
	statusbar->move( ePoint(0, clientrect.height()-50 ) );
	statusbar->resize( eSize( clientrect.width(), 50) );
	statusbar->loadDeco();
	
	setHelpID(158);
}

//change helptext and shutdown command when changing combobox cbb_SERVER_OS
void eWakeConf::changeOS(eListBoxEntryText *os)
{
	cbb_SERVER_OS->setHelpText(os->getHelpText());
	
	eString tmpos = os->getText();
	
	if(tmpos == "Linux")
	{
		shutdown = "shutdown -h now";
	}
	else
	{
		shutdown = "shutdown -s";
	}
}

//change helptext and ip when changing combobox cbb_PROFILE_NO
void eWakeConf::changeProfile(eListBoxEntryText *profile)
{
	cbb_PROFILE_NO->setHelpText(profile->getHelpText());

	getServerIP();
}

// retrieve IP for currently selected profile
void eWakeConf::getServerIP()
{
	int de[4];

	__u32 sip;

	profileno = atoi(cbb_PROFILE_NO->getCurrent()->getText().c_str() );

	//load IP address for current selection
	cmd.sprintf("/elitedvb/network/server%d/", profileno);
	if ( eConfig::getInstance()->getKey((cmd+"smbsrv").c_str(), sip) )
	{
		eMessageBox noip(_("No IP-address stored for this profile"), _("LAN-Control"), eMessageBox::iconWarning|eMessageBox::btOK );
		noip.show();
		noip.exec();
		noip.hide();
		bt_SCRIPT->hide();
	}
	else
	{
		eNumber::unpack(sip, de);

		ipaddress = eString().sprintf("%d.%d.%d.%d", de[0], de[1], de[2], de[3]);
		if(keystate==1)
		{
			bt_SCRIPT->show();
		}
	}
}

//write the values into sh-file
void eWakeConf::writeScript()
{
	user = tf_USER->getText();
	key = cbb_RSA->getText();

	FILE *g;

	system(eString().sprintf("rm -f /var/etc/shutdown%d.sh", profileno ).c_str());
	system(eString().sprintf("touch /var/etc/shutdown%d.sh", profileno ).c_str());
	system(eString().sprintf("chmod 755 /var/etc/shutdown%d.sh", profileno ).c_str());
	g = fopen(eString().sprintf("/var/etc/shutdown%d.sh", profileno ).c_str(), "w");

	eString script = eString().sprintf("#!/bin/sh\n\ndbclient -i %s/etc/dropbear/%s -l %s %s %s", path.c_str(), key.c_str(), user.c_str(), ipaddress.c_str(), shutdown.c_str());

	if (g)
	{
	fputs(script.c_str(), g);
	fclose(g);
	eMessageBox done(_("The script has been generated."), ("LAN-Control"), eMessageBox::btOK);
		done.show();
		done.exec();
		done.hide();
	}
	else
	{
	eMessageBox nofile(_("The file could not be created"), ("LAN-Control"), eMessageBox::btOK);
		nofile.show();
		nofile.exec();
		nofile.hide();
	}
}

//read content of /var/etc/dropbear or /usr/etc/dropbear
void eWakeConf::getStoredKeys()
{
	//we stop the timer because we only need it once
	loadTimer->stop();

	int i=0;
	int j=0;

	DIR *keydirectory;
	struct dirent *content;

	eString mypath = eString().sprintf("%s/etc/dropbear", path.c_str() );

	keydirectory = opendir(mypath.c_str());
	
	if(keydirectory)
	{
		do
		{
        		content = readdir(keydirectory);
        		if (content)
        		{
				keys[j]=content->d_name;
					if ( (!strcmp(keys[j].c_str(), "." ) ) || (!strcmp(keys[j].c_str(), ".." ) )|| (content->d_type==4) )
						j--;
					j++;

        		}
		}
		while (content);
		closedir(keydirectory);
	}
	
	if(j==0)
	{
		eMessageBox box(eString().sprintf(_("No key found in %s"), mypath.c_str()), _("LAN-Control"), eMessageBox::iconWarning|eMessageBox::btOK );
		box.show();
		box.exec();
		box.hide();
		bt_SCRIPT->hide();
		keystate = 0;
		return;
	}
	else
	{
		for (i=0; i < j; i++)
		{
  			new eListBoxEntryText(*cbb_RSA, eString().sprintf("%s", keys[i].c_str()), (void*)(i-1),0, "Use this key");
		}
		keystate = 1;
		bt_SCRIPT->show();
	}
}

eWakeConf::~eWakeConf()
{
	//we remove the timer
	delete loadTimer;
}

//get the boxtype to use the correct path for rsa key lookup
const eString& eWakeHelper::getBoxType()
{
	path="";

	switch( eSystemInfo::getInstance()->getHwType() )
	{
		case eSystemInfo::DM500:
		case eSystemInfo::DM5600:
		case eSystemInfo::DM5620:
		case eSystemInfo::DM7000:
		case eSystemInfo::TR_DVB272S:
			path = "/var";
			break;

		case eSystemInfo::DM500PLUS:
		case eSystemInfo::DM600PVR:
		case eSystemInfo::DM7020:
			path = "/usr";
			break;
		default:
			path = "/var";
			break;
	}
	return path;
}


#endif // DISABLE_NETWORK
