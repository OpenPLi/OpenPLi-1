/*
 * Ronald's setup plugin for dreambox
 * Copyright (c) 2004 Ronaldd <Ronaldd@sat4all.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include "setup_debug.h"

#include <plugin.h>

#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <lib/base/i18n.h>

#include <enigma.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/combobox.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>

#define MENUNAME N_("Debugging")

class DebugSetupFactory : public eCallableMenuFactory
{
public:
	DebugSetupFactory() : eCallableMenuFactory("DebugSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new DebugSetup;
	}
};

DebugSetupFactory DebugSetup_factory;

DebugSetup::DebugSetup ():
	ePLiWindow(_(MENUNAME), 380),
	pLogEmuDaemon(0), pLogEnigma(0), pUseSyslog(0), pSyslog(0),
	pCoreFiles(0), pRemoteSyslogHost(0), pLogPath(0),
	pRemoteSyslogHostLabel(0), pLogPathLabel(0)
{
	int hwType = eSystemInfo::getInstance()->getHwType();
	oRc_Config.ReadConfig();

	if (!eSystemInfo::getInstance()->isOpenEmbedded())
	{
		pLogEmuDaemon = new eCheckbox (this, oRc_Config.getEnableEmuDaemonLog(), 1);
		pLogEmuDaemon->setText (_("Log EmuDaemon to file"));
		pLogEmuDaemon->move (ePoint (10, yPos()));
		pLogEmuDaemon->resize (eSize (360, widgetHeight()));
		pLogEmuDaemon->setHelpText (_("Log EmuDaemon to file"));
		nextYPos();

		pLogEnigma = new eCheckbox (this, oRc_Config.getEnableEnigmaLog(), 1);
		pLogEnigma->setText (_("Log Enigma to file"));
		pLogEnigma->move (ePoint (10, yPos()));
		pLogEnigma->resize (eSize (360, widgetHeight()));
		pLogEnigma->setHelpText (_("Log Enigma to file"));
		nextYPos();

		if (hwType == eSystemInfo::DM7000)
		{
			int corefilesEnable = 1;
			if (access("/var/etc/.no_corefiles", R_OK) == 0)
			{
				corefilesEnable = 0;
			}
			pCoreFiles = new eCheckbox (this, corefilesEnable, 1);
			pCoreFiles->setText (_("Enable Enigma core files"));
			pCoreFiles->move(ePoint(10, yPos()));
			pCoreFiles->resize(eSize(360, widgetHeight()));
			pCoreFiles->setHelpText(_("Create core files after an Enigma crash"));
			nextYPos();
		}
	}

	int syslog = 0;
	eConfig::getInstance()->getKey("/elitedvb/extra/syslog", syslog);
	pUseSyslog = new eCheckbox(this, syslog, 1);
	pUseSyslog->setText (_("Log Enigma to syslog"));
	pUseSyslog->move(ePoint (10, yPos()));
	pUseSyslog->resize(eSize (360, widgetHeight()));
	pUseSyslog->setHelpText (_("Send Enigma log to syslog"));
	nextYPos();

	if (!eSystemInfo::getInstance()->isOpenEmbedded())
	{
		pSyslog = new eCheckbox (this, oRc_Config.getEnableSysLog(), 1);
		pSyslog->setText (_("Start syslog daemon at boot"));
		pSyslog->move (ePoint (10, yPos()));
		pSyslog->resize (eSize (360, widgetHeight()));
		pSyslog->setHelpText (_("Start syslog daemon at boot"));
		nextYPos();
	}

	int disableSerialOutput = 1;
	eConfig::getInstance()->getKey("/ezap/extra/disableSerialOutput", disableSerialOutput);

	pSerial = new eCheckbox(this, !disableSerialOutput, 1);
	pSerial->setText(_("Enable serial debug"));
	pSerial->move(ePoint (10, yPos()));
	pSerial->resize(eSize (360, widgetHeight()));
	pSerial->setHelpText(_("Open serial http interface on /dev/tts/0 for debugging"));
	nextYPos(35);

	if (!eSystemInfo::getInstance()->isOpenEmbedded())
	{
		pRemoteSyslogHostLabel = new eLabel (this);
		pRemoteSyslogHostLabel->move (ePoint (10, yPos()));
		pRemoteSyslogHostLabel->resize (eSize (120, widgetHeight()));
		pRemoteSyslogHostLabel->setText (_("Remote host:"));

		pRemoteSyslogHost = new eTextInputField(this);
		pRemoteSyslogHost->setMaxChars(40);
		pRemoteSyslogHost->move(ePoint(130, yPos()));
		pRemoteSyslogHost->resize(eSize(240, widgetHeight()));
		pRemoteSyslogHost->loadDeco();
		pRemoteSyslogHost->setHelpText(_("Host of remote syslog daemon (local leave empty)"));
			pRemoteSyslogHost->setText(eString().sprintf("%s", oRc_Config.getRemoteSyslogHost().c_str()));
		nextYPos(35);

		pLogPathLabel = new eLabel (this);
		pLogPathLabel->move (ePoint (10, yPos()));
		pLogPathLabel->resize (eSize (120, widgetHeight()));
		pLogPathLabel->setText (_("Logfile path:"));

		pLogPath = new eComboBox (this, 3, pLogPathLabel);
		pLogPath->move (ePoint (130, yPos()));
		pLogPath->resize (eSize (165, widgetHeight()));
		pLogPath->setHelpText (_("Choose the logpath to use"));
		pLogPath->loadDeco ();

		int currentSelection = 0;
		new eListBoxEntryText (*pLogPath, eString ().sprintf ("/tmp/log"), (void *) 0);
		if (hwType == eSystemInfo::DM7000)
		{
			new eListBoxEntryText (*pLogPath, eString ().sprintf ("/media/hdd/log"), (void *) 1);
			new eListBoxEntryText (*pLogPath, eString ().sprintf ("/media/usb/log"), (void *) 2);
			if (strcmp (oRc_Config.getLogPath(), "/media/usb/log") == 0)
			{
				currentSelection = 2;
			}
			else if (strcmp (oRc_Config.getLogPath(), "/media/hdd/log") == 0)
			{
				currentSelection = 1;
			}
		}

		pLogPath->setCurrent(currentSelection, false);
	}

	buildWindow();
	CONNECT (bOK->selected, DebugSetup::okPressed);
}

DebugSetup::~DebugSetup()
{
}

void DebugSetup::okPressed ()
{
	if (pLogEmuDaemon)
	{
		oRc_Config.setEnableEmuDaemonLog(pLogEmuDaemon->isChecked() ? 1 : 0);
	}

	if (pLogEnigma)
	{
		oRc_Config.setEnableEnigmaLog(pLogEnigma->isChecked() ? 1 : 0);
	}

	if (pCoreFiles)
	{
		eString cmd = pCoreFiles->isChecked() ? "rm -f" : "touch";
		cmd += " /var/etc/.no_corefiles";
		system(cmd.c_str());
	}

	if (pUseSyslog)
	{
		eConfig::getInstance()->setKey("/elitedvb/extra/syslog", pUseSyslog->isChecked() ? 1 : 0);
	}

	if (pSyslog)
	{
		oRc_Config.setEnableSysLog(pSyslog->isChecked() ? 1 : 0);
		if (pRemoteSyslogHost)
		{
			oRc_Config.setRemoteSyslogHost(pRemoteSyslogHost->getText().c_str());
		}
	}

	if (pLogPath)
	{
		oRc_Config.setLogPath((int) pLogPath->getCurrent ()->getKey ());
	}
	
	if (pSerial)
	{
		eConfig::getInstance()->setKey("/ezap/extra/disableSerialOutput",
			pSerial->isChecked() ? 0 : 1);
		eZap::getInstance()->reconfigureHTTPServer();
	}
		
	oRc_Config.WriteConfig();

	close (0);
}

void DebugSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
