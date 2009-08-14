/***************************************************************************
                          emuconfig.cpp  -  description
                             -------------------
    begin                : Oct 2005
    copyright            : (C) 2005 by the PLi(R) team
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Credits to The_Hydra, who's smartcam implementation was used as an example
 * for the config gui in this eEmuConfig class
 *
 * Mirakels: add close button; made lb names more meaningfull; changed layout
 *           so channel widget is on top and has focus.
 * PLi: integrated emuconfig in PLi setup plugin
 *      added several items so you can enable/disable services
 *      and store the settings in plimgr
 * PLi: added popup when saving and saving emu and restarting server
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <sys/timeb.h>

#include <plugin.h>
#include <parentallock.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/listbox.h>
#include <lib/gui/ExecuteOutput.h>
#include <lib/dvb/dvbservice.h>
#include <lib/system/info.h>

#include "emuconfig.h"

#include <streaminfo.h>

#define MENUNAME N_("Softcam setup")

class eEmuConfigFactory : public eCallableMenuFactory
{
public:
	eEmuConfigFactory() : eCallableMenuFactory("eEmuConfig", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eEmuConfig;
	}
};

eEmuConfigFactory eEmuConfig_factory;

extern struct caids_t caids[];
extern unsigned int caids_cnt;


const char* EMUD_SERVER_SOCKET = "/tmp/.emud.socket";

eEmuConfig::eEmuConfig() : chk_twoservices(0)
{
	emudSocket = -1;

	setText(_(MENUNAME));

	lb_service = new eLabel(this);
	lbx_stick_serv = new eComboBox(this, 3, lb_service);
	lb_provider = new eLabel(this);
	lbx_stick_prov = new eComboBox(this, 3, lb_provider);
	lb_defemu = new eLabel(this);
	lbx_Emulator = new eComboBox(this, 3, lb_defemu);
	lb_cardserver = new eLabel(this);
	lbx_cardserver = new eComboBox(this, 3, lb_cardserver);

	lb_caids = new eLabel(this);
	lb_caids2 = new eLabel(this);

	int socketReconnect = 1;
	eConfig::getInstance()->getKey("/elitedvb/extra/cahandlerReconnect", socketReconnect);
	chk_reconnect = new eCheckbox(this, socketReconnect, 1);	

	/* This is probably not a 100% check, but for now, the boxes that can timeshift can handle two services */
	if (eSystemInfo::getInstance()->canTimeshift())
	{
		int handleTwo = 1;
		eConfig::getInstance()->getKey("/ezap/ci/handleTwoServices", handleTwo);
		chk_twoservices = new eCheckbox(this, handleTwo, 1);
	}

	sbStatus = new eStatusBar(this);

	gFont font;

	int fd = eSkin::getActive ()->queryValue ("fontsize", 16);

	int iYPoint = 10;

	lb_service->move(ePoint(10, iYPoint));
	lb_service->resize(eSize(300, fd + 4));
	lb_service->loadDeco();

	lbx_stick_serv->move(ePoint(310, iYPoint));
	lbx_stick_serv->resize(eSize(180, fd + 4));
	lbx_stick_serv->setHelpText(_("Select softcam for this channel"));
	lbx_stick_serv->loadDeco();

	iYPoint += 35;

	lb_provider->move(ePoint(10, iYPoint));
	lb_provider->resize(eSize(300, fd + 4));
	lb_provider->loadDeco();

	lbx_stick_prov->move(ePoint(310, iYPoint));
	lbx_stick_prov->resize(eSize(180, fd + 4));
	lbx_stick_prov->setHelpText(_("Select softcam for this provider"));
	lbx_stick_prov->loadDeco();

	iYPoint += 35;

	lb_defemu->setText(_("Default SoftCam"));
	lb_defemu->move(ePoint(10, iYPoint));
	lb_defemu->resize(eSize(300, fd + 4));
	lb_defemu->loadDeco();

	lbx_Emulator->move(ePoint(310, iYPoint));
	lbx_Emulator->resize(eSize(180, fd + 4));
	lbx_Emulator->setHelpText(_("Select default softcam"));
	lbx_Emulator->loadDeco();

	iYPoint += 35;

	lb_cardserver->setText(_("Cardserver"));
	lb_cardserver->resize(eSize(300, fd + 4));
	lb_cardserver->move(ePoint(10, iYPoint));
	lb_cardserver->loadDeco();

	lbx_cardserver->move(ePoint(310, iYPoint));
	lbx_cardserver->resize(eSize(180, fd + 4));
	lbx_cardserver->setHelpText(_("Select cardserver"));
	lbx_cardserver->loadDeco();

	iYPoint += 35;

	chk_reconnect->setText(_("Auto reconnect cahandler"));
	chk_reconnect->move(ePoint(10, iYPoint));
	chk_reconnect->resize(eSize(235, fd + 4));
	chk_reconnect->setHelpText(_("Try to reconnect when an external cahandler connection was lost"));
	chk_reconnect->loadDeco();

	if(chk_twoservices)
	{
		chk_twoservices->setText(_("Can handle two services"));
		chk_twoservices->move(ePoint(255, iYPoint));
		chk_twoservices->resize(eSize(235, fd + 4));
		chk_twoservices->setHelpText(_("Cam or common interface can handle two services at the same time"));
		chk_twoservices->loadDeco();
	}

	int iRows = 0;
	char cardservername[128];

	lbx_cardserver->clear();
	new eListBoxEntryText(*lbx_cardserver, _("-- None --"), (void *) -1);
	lbx_cardserver->forEachEntry(selectListEntry((void *) -1, lbx_cardserver));
	int i = 0;
	while (1)
	{
		if (transferEmudCommand(CMD_GET_CARDSERVER_NAME, &i, sizeof(i), cardservername, sizeof(cardservername)) < 0) break;
		/* ensure name is terminated */
		cardservername[sizeof(cardservername) - 1] = 0;
		if (strlen(cardservername) <= 0) break;
		new eListBoxEntryText(*lbx_cardserver, cardservername, (void *) i);
		i++;
	}

	// get active cardserver
	if (transferEmudCommand(CMD_GET_CARDSERVER_SETTING, NULL, 0, cardservername, sizeof(cardservername)) >= 0)
	{
		/* ensure name is terminated */
		cardservername[sizeof(cardservername) - 1] = 0;
		lbx_cardserver->forEachEntry(selectListEntry(cardservername, lbx_cardserver));
	}


	cresize (eSize (500, 375 + (iRows * 35)));
	valign();

	iYPoint += 35;

	font = lb_caids->getFont();
	lb_caids->setFont(gFont(font.family, 16));
	lb_caids->move(ePoint(10, iYPoint));
	lb_caids->resize(eSize(200, 200));
	lb_caids->loadDeco();

	lb_caids2->setFont(gFont(font.family, 16));
	lb_caids2->move(ePoint(220, iYPoint));
	lb_caids2->resize(eSize(200, 200));
	lb_caids2->loadDeco();

	bt_save = new eButton(this);
	bt_save->move(ePoint(10, clientrect.height()-120));
	bt_save->resize(eSize(153, 30));
	bt_save->setShortcut("blue");
	bt_save->setShortcutPixmap("blue");
	bt_save->setText(_("Save"));
	bt_save->setHelpText(_("Save settings"));
	bt_save->loadDeco();
	CONNECT_1_0(bt_save->selected, eEmuConfig::saveSettings, false);

	bt_list = new eButton(this);
	bt_list->move(ePoint(173, clientrect.height()-120));
	bt_list->resize(eSize(154, 30));
	bt_list->setText(_("List stickies"));
	bt_list->setHelpText(_("List current stickies"));
	bt_list->loadDeco();
	CONNECT(bt_list->selected, eEmuConfig::enumSettings);

	bt_cardinfo = new eButton(this);
	bt_cardinfo->move(ePoint(337, clientrect.height()-120));
	bt_cardinfo->resize(eSize(153, 30));
	bt_cardinfo->setShortcut("yellow");
	bt_cardinfo->setShortcutPixmap("yellow");
	bt_cardinfo->setText(_("Cardinfo"));
	bt_cardinfo->setHelpText(_("Get cardinfo from current active cardserver"));
	bt_cardinfo->loadDeco();
	CONNECT(bt_cardinfo->selected, eEmuConfig::getCardInfo);

	bt_saveandrestart = new eButton(this);
	bt_saveandrestart->move(ePoint(10, clientrect.height()-80));
	bt_saveandrestart->resize(eSize(235, 30));
	bt_saveandrestart->setShortcut("green");
	bt_saveandrestart->setShortcutPixmap("green");
	bt_saveandrestart->setText(_("Save and restart cam"));
	bt_saveandrestart->setHelpText(_("Save changes, restart the softcam and return to enigma"));
	bt_saveandrestart->loadDeco();
	CONNECT_1_0(bt_saveandrestart->selected, eEmuConfig::saveSettings, true);

	bt_restartcardserver = new eButton(this);
	bt_restartcardserver->move(ePoint(255, clientrect.height()-80));
	bt_restartcardserver->resize(eSize(235, 30));
	bt_restartcardserver->setShortcut("red");
	bt_restartcardserver->setShortcutPixmap("red");
	bt_restartcardserver->setText(_("Restart cardserver"));
	bt_restartcardserver->setHelpText(_("Press OK to restart the cardserver"));
	bt_restartcardserver->loadDeco();
	CONNECT(bt_restartcardserver->selected, eEmuConfig::restartCardserver);


	sbStatus->move(ePoint(0, getClientSize().height() - 40));
	sbStatus->resize(eSize(getClientSize().width(), 40));
	sbStatus->loadDeco();

	populateList();
}



eEmuConfig::~eEmuConfig()
{
	if (emudSocket >= 0) ::close(emudSocket);
}



int eEmuConfig::writeToSocket(int socket, const void *data, int size)
{
	/* Wrapper to write data to a socket */

	int written = 0;

	while (written < size)
	{
		int result;
		if ((result = ::write(socket, &((char*)data)[written], size - written)) < 0)
		{
			if (errno == EINTR) continue;
			return -1;
		}
		else if (result == 0)
		{
			return -1;
		}
		written += result;
	}
	return written;
}



int eEmuConfig::readFromSocket(int socket, void *data, int size)
{
	/* Wrapper to read data from a socket */

	int read = 0;

	while (read < size)
	{
		int result;
		if ((result = ::read(socket, &((char*)data)[read], size - read)) < 0)
		{
			if (errno == EINTR) continue;
			return -1;
		}
		else if (result == 0)
		{
			return -1;
		}
		read += result;
	}
	return read;
}


int eEmuConfig::commandExchange(int socket, int cmd, const void *txdata, int txsize, void *rxdata, int rxsize)
{
	if (socket < 0) return -2;
	packet datapacket;

	datapacket.cmd = cmd;
	datapacket.datasize = txsize;

	if (writeToSocket(socket, &datapacket, sizeof(datapacket)) < 0) return -2;
	if (txdata && datapacket.datasize)
	{
		if (writeToSocket(socket, txdata, datapacket.datasize) < 0) return -2;
	}
	if (readFromSocket(socket, &datapacket, sizeof(datapacket)) < 0) return -2;
	if (datapacket.datasize > 0 && datapacket.datasize <= rxsize)
	{
		if (readFromSocket(socket, rxdata, datapacket.datasize) < 0) return -2;
	}

	return datapacket.cmd;
}



int eEmuConfig::transferEmudCommand(int cmd, const void *txdata, int txsize, void *rxdata, int rxsize)
{
	int result = -1;
	int i = 0;
	for (i = 0; i < 2; i++)
	{
		if (emudSocket < 0)
		{
			struct sockaddr_un servaddr;
			memset(&servaddr, 0, sizeof(servaddr));
			servaddr.sun_family = AF_UNIX;
			strcpy(servaddr.sun_path, EMUD_SERVER_SOCKET);
			if ((emudSocket = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) goto error;
			if (connect(emudSocket, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) goto error;
		}
		if ((result = commandExchange(emudSocket, cmd, txdata, txsize, rxdata, rxsize)) < -1)
		{
			/* < -1 is a socket error, disconnect and try again */
			if (emudSocket >= 0)
			{
				::close(emudSocket);
				emudSocket = -1;
			}
			continue;
		}
		break;
	}
	return result;
error:
	if (emudSocket >= 0)
	{
		::close(emudSocket);
		emudSocket = -1;
	}
	return -1;
}



void eEmuConfig::saveSettings(bool restart)
{
	eMessageBox mb(_("Saving settings\nand starting softcam"), _("Softcam config info"), eMessageBox::iconInfo);
	mb.show();
	putchannelsettings settings;
	bzero(&settings, sizeof(settings));

	eListBoxEntryText *selected = 0;
	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		if (eDVB::getInstance() && eDVB::getInstance()->settings && eDVB::getInstance()->settings->getTransponders())
		{
			eServiceDVB *servicedvb = eDVB::getInstance()->settings->getTransponders()->searchService(sapi->service);
			if (servicedvb)
			{
				strncpy(settings.channelname, servicedvb->service_name.c_str(), sizeof(settings.channelname) - 1);
				settings.channelname[sizeof(settings.channelname) - 1] = 0;
				strncpy(settings.providername, servicedvb->service_provider.c_str(), sizeof(settings.providername) - 1);
				settings.providername[sizeof(settings.providername) - 1] = 0;
			}
		}
	}

	if ((selected = lbx_Emulator->getCurrent()))
	{
		settings.defaultemu = (int)selected->getKey();
	}
	else
	{
		settings.defaultemu = -1;
	}

	if ((selected = lbx_stick_serv->getCurrent()))
	{
		settings.channelemu = (int)selected->getKey();
	}
	else
	{
		settings.channelemu = -1;
	}

	if ((selected = lbx_stick_prov->getCurrent()))
	{
		settings.provideremu = (int)selected->getKey();
	}
	else
	{
		settings.provideremu = -1;
	}
	
	eConfig::getInstance()->setKey("/elitedvb/extra/cahandlerReconnect", 
		chk_reconnect->isChecked() ? 1 : 0); 

	if(chk_twoservices)
	{
		eConfig::getInstance()->setKey("/ezap/ci/handleTwoServices", 
			chk_twoservices->isChecked() ? 1 : 0);
	}

	selected = lbx_cardserver->getCurrent();
	/*
	 * always save the cardserver first (if we just removed the cardserver,
	 * it has to be stopped before we restart the softcam, so the softcam can
	 * use the cardreaders)
	 */
	saveCardserver();
	if (selected && (int)selected->getKey() != -1)
	{
		/* we have a cardserver, wait 5s for it to start... */
		sleep(5);
	}

	transferEmudCommand(restart ? CMD_PUT_CHANNEL_SETTINGS_AND_RESTART : CMD_PUT_CHANNEL_SETTINGS, &settings, sizeof(settings));

	sleep(1);
	mb.hide();

	if (restart) close(0);
}

void eEmuConfig::saveCardserver()
{
	char cardservername[NAMELENGTH + 1];
	eListBoxEntryText *selected = lbx_cardserver->getCurrent();
	if (!selected || (int)selected->getKey() == -1)
	{
		cardservername[0] = 0;
	}
	else
	{
		strncpy(cardservername, lbx_cardserver->getText().c_str(), sizeof(cardservername) - 1);
		cardservername[sizeof(cardservername) - 1] = 0;
	}
	transferEmudCommand(CMD_SET_CARDSERVER_SETTING, &cardservername, sizeof(cardservername));
}

void eEmuConfig::restartCardserver()
{
	eMessageBox mb(_("Restarting cardserver"), _("Cardserver info"), eMessageBox::iconInfo);
	mb.show();
	saveCardserver();
	transferEmudCommand(CMD_RESTART_CARDSERVER);
	sleep(1);
	mb.hide();
}



void eEmuConfig::enumSettings()
{
	this->hide();

	eEmuStickyList dlg(this);
	dlg.show();
	dlg.exec();
	dlg.hide();

	this->show();
}



void eEmuConfig::populateList()
{
	int i;
	getchannelsettings settings;

	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();

	if (sapi)
	{
		if (eDVB::getInstance() && eDVB::getInstance()->settings && eDVB::getInstance()->settings->getTransponders())
		{
			eServiceDVB *servicedvb = eDVB::getInstance()->settings->getTransponders()->searchService(sapi->service);
			if (servicedvb)
			{
				lb_service->setText((eString)_("Channel") + (eString)": " + servicedvb->service_name);
				lb_provider->setText((eString)_("Provider") + (eString)": " + servicedvb->service_provider);
			}
		}
	}

	if (!lbx_Emulator || !lbx_stick_serv || !lbx_stick_prov) return;

	lbx_Emulator->clear();
	lbx_stick_serv->clear();
	lbx_stick_prov->clear();

	new eListBoxEntryText(*lbx_Emulator, _("-- None --"), (void *) -1);
	new eListBoxEntryText(*lbx_stick_serv, _("-- Default --"), (void *) -1);
	new eListBoxEntryText(*lbx_stick_prov, _("-- Default --"), (void *) -1);

	i = 0;
	while (1)
	{
		emuinfo info;

		if (transferEmudCommand(CMD_GET_EMU_INFO, &i, sizeof(i), &info, sizeof(info)) < 0) break;
		if (info.name[0] == 0) break;

		/* ensure names are terminated */
		info.name[sizeof(info.name) - 1] = 0;
		info.version[sizeof(info.version) - 1] = 0;

		new eListBoxEntryText(*lbx_Emulator, (eString)info.name + (info.version[0] ? ((eString)"-" + (eString)info.version) : (eString)info.version), (void *)i);
		new eListBoxEntryText(*lbx_stick_serv, (eString)info.name + (info.version[0] ? ((eString)"-" + (eString)info.version) : (eString)info.version), (void *)i);
		new eListBoxEntryText(*lbx_stick_prov, (eString)info.name + (info.version[0] ? ((eString)"-" + (eString)info.version) : (eString)info.version), (void *)i);
		i++;
	}

	if (transferEmudCommand(CMD_GET_CHANNEL_SETTINGS, NULL, 0, &settings, sizeof(settings)) >= 0)
	{
		lbx_stick_serv->forEachEntry(selectListEntry((void *) settings.channelemu, lbx_stick_serv));
		lbx_stick_prov->forEachEntry(selectListEntry((void *) settings.provideremu, lbx_stick_prov));
		lbx_Emulator->forEachEntry(selectListEntry((void *) settings.defaultemu, lbx_Emulator));
	}

	if (lb_caids && sapi)
	{
		eString caids;
		eString caids2;
		std::set<int>& calist = sapi->usedCASystems;
		int iCount(0);

		for (std::set<int>::iterator i(calist.begin()); i != calist.end(); ++i)
		{
			iCount++;
			eString caname = eStreaminfo::getInstance()->getCAName(*i, 1);
			if (iCount < 5)
				caids = caids + eString().sprintf(" [%4.4x]",*i) + " " + caname + "\n";
			else
				caids2 = caids2 + eString().sprintf(" [%4.4x]",*i) + " " + caname + "\n";
		}

		lb_caids->setText(caids.c_str());
		lb_caids2->setText(caids2.c_str());
	}

	setFocus(lbx_stick_serv);
}


void eEmuConfig::getCardInfo()
{
	hide();

	eString strCaption = _("getting cardinfo");
	eString strCommand = "cardinfo-pli.sh";
	ExecuteOutput oExecuteOutput(strCaption, strCommand);
	oExecuteOutput.show();
	oExecuteOutput.exec();
	oExecuteOutput.hide();

	show();
}

void eEmuConfig::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	if(pinCheck::getInstance()->checkPin(pinCheck::setup))
	{
		setLCD(lcdTitle, lcdElement);
		show();
		exec();
		hide();
	}
}

eEmuStickyList::eEmuStickyList(eEmuConfig *eEmuConf)
{

	cmove (ePoint (100, 110));
	cresize (eSize (520, 400));

	setText(_("Current Stickies"));
	listb = new eListBox < eListBoxEntryText > (this);
	listb->setText (_("Sticky Settings 1"));
	listb->move (ePoint (10, 10));
	listb->resize (eSize (500, 380));
	listb->loadDeco ();
	listb->show ();

	int entry = 0;
	int enumOffset = 0;
	while (1) {
		enumsettings settings;

		if ((enumOffset = eEmuConf->transferEmudCommand(CMD_ENUM_SETTINGS, &enumOffset, sizeof(enumOffset), &settings, sizeof(settings))) < 0) break;

		/* ensure names are terminated */
		settings.settingname[sizeof(settings.settingname) - 1] = 0;
		settings.emuname[sizeof(settings.emuname) - 1] = 0;

		if (settings.channel) {
			eString number;
			number.setNum(settings.channel, 16);
#if 0
			eString servname = "Unknown";
			eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
			if (sapi) {
				eServiceReferenceDVB serviceRef = eDVB::getInstance()->settings->getTransponders()->searchServiceByNumber(settings.channel);
                                servname = serviceRef.service_name;
			}
#endif
			new eListBoxEntryText (listb, (eString)_("Channel") + (eString)" \"" + (eString)(const char*)settings.settingname + "\": " + (eString)(const char*)settings.emuname, (void *) entry);
		}
		else if (settings.provider) {
			eString number;
			number.setNum(settings.provider, 16);
			new eListBoxEntryText (listb, (eString)_("Provider") + (eString)" \"" + (eString)(const char*)settings.settingname + (eString)"\": " + (eString)(const char*)settings.emuname, (void *) entry);
		}
	}
}



eEmuStickyList::~eEmuStickyList()
{
}


