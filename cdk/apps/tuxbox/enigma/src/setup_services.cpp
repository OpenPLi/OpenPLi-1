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

#include "setup_services.h"

#include <plugin.h>

#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <lib/base/i18n.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>

#include <lib/gui/echeckbox.h>
#include <lib/gui/emessage.h>

#include <lib/dvb/edvb.h>

#define MENUNAME N_("Services to run")

class ServicesSetupFactory : public eCallableMenuFactory
{
public:
	ServicesSetupFactory() : eCallableMenuFactory("ServicesSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new ServicesSetup;
	}
};

ServicesSetupFactory ServicesSetup_factory;

ServicesSetup::ServicesSetup() : 
	ePLiWindow(_(MENUNAME), 390),
	pConfigureFirewall(0),
	eStartCron(0), eStartNfsServer(0), eStartApache(0), eStartGsub(0), 
	eStartInadyn(0), eStartDropbear(0), eEnableFirewall(0), eEnableSamba(0),
	fStartSambaChanged(false), fStartNfsServerChanged(false), fStartApacheChanged(false),
	fEnableFirewallChanged(false), fStartInadynChanged(false), fStartDropbearChanged(false),
	fStartCronChanged(false), fStartGsubChanged(false)
{
	// Lock remote control temporary
	eRCInput::getInstance()->lock();
	oRc_Config.ReadConfig();

#ifndef DISABLE_NETWORK
	if (!eSystemInfo::getInstance()->isOpenEmbedded() &&
		(access("/bin/smbd", X_OK) == 0 ||
		access("/var/bin/smbd", X_OK) == 0 ||
		access("/usr/sbin/smbd", X_OK) == 0))
	{
		int iEnableSamba = 0;
		eConfig::getInstance()->getKey("/elitedvb/network/samba", iEnableSamba);
		
		eEnableSamba = new eCheckbox (this, iEnableSamba, 1);
		eEnableSamba->setText(_("Start samba (enable file sharing)"));
		eEnableSamba->move(ePoint(10, yPos()));
		eEnableSamba->resize(eSize(370, widgetHeight()));
		eEnableSamba->setHelpText(_("start/do not start samba to make the Dreambox files available on a PC"));
		CONNECT(eEnableSamba->checked, ServicesSetup::EnableSambaChanged);
		nextYPos();
	}
#endif

	if ((access("/var/bin/firewall.sh", X_OK) == 0 ||
		access("/usr/bin/firewall.sh", X_OK) == 0))
	{
		eEnableFirewall = new eCheckbox (this, oRc_Config.getStartFirewall(), 1);
		eEnableFirewall->setText (_("Start firewall"));
		eEnableFirewall->move (ePoint (10, yPos()));
		eEnableFirewall->resize (eSize (370, widgetHeight()));
		eEnableFirewall->setHelpText (_("start/do not start firewall"));
		CONNECT (eEnableFirewall->checked, ServicesSetup::EnableFirewallChanged);
		nextYPos();

#ifdef SETUP_FIREWALL
      pConfigureFirewall = new eButton (this);
      pConfigureFirewall->setShortcut ("blue");
      pConfigureFirewall->setShortcutPixmap ("blue");
      pConfigureFirewall->setText (_("Configure FW"));
      pConfigureFirewall->move (ePoint (210, yPos()));
      pConfigureFirewall->resize (eSize (170, widgetHeight()));
      pConfigureFirewall->setHelpText (_("Configure the firewall"));
      pConfigureFirewall->loadDeco ();
      CONNECT (pConfigureFirewall->selected, ServicesSetup::configureFirewall);
		nextYPos();
#endif // SETUP_FIREWALL
	}

	// Test if cron is installed
	if (!eSystemInfo::getInstance()->isOpenEmbedded() && ((access("/sbin/crond", X_OK) == 0) || (access("/usr/sbin/crond", X_OK) == 0)))
	{
		eStartCron = new eCheckbox (this, oRc_Config.getStartCron(), 1);
		eStartCron->setText (_("Start crond"));
		eStartCron->move (ePoint (10, yPos()));
		eStartCron->resize (eSize (370, widgetHeight()));
		eStartCron->setHelpText (_("start/do not start crond the time daemon"));
		CONNECT (eStartCron->checked, ServicesSetup::StartCronChanged);
		nextYPos();
	}

	// Test if Inadyn is installed
	if (access("/var/bin/inadyn", X_OK) == 0)
	{
		eStartInadyn = new eCheckbox (this, oRc_Config.getStartInadyn(), 1);
		eStartInadyn->setText (_("Start Inadyn"));
		eStartInadyn->move (ePoint (10, yPos()));
		eStartInadyn->resize (eSize (370, widgetHeight()));
		eStartInadyn->setHelpText (_("start/do not start Inadyn"));
		CONNECT (eStartInadyn->checked, ServicesSetup::StartInadynChanged);
		nextYPos();
	}

	// Test if Dropbear is installed
	if (access("/var/bin/dropbear.sh", X_OK) == 0)
	{
		eStartDropbear = new eCheckbox (this, oRc_Config.getStartDropbear(), 1);
		eStartDropbear->setText (_("Start Dropbear"));
		eStartDropbear->move (ePoint (10, yPos()));
		eStartDropbear->resize (eSize (370, widgetHeight()));
		eStartDropbear->setHelpText (_("start/do not start Dropbear"));
		CONNECT (eStartDropbear->checked, ServicesSetup::StartDropbearChanged);
		nextYPos();
	}

	// Test if NFS is installed
	// NFS is installed by default on a 7000 only
	if (access("/bin/nfs.sh", X_OK) == 0)
	{
		eStartNfsServer = new eCheckbox (this, oRc_Config.getStartNfs(), 1);
		eStartNfsServer->setText (_("Start NFS server"));
		eStartNfsServer->move (ePoint (10, yPos()));
		eStartNfsServer->resize (eSize (370, widgetHeight()));
		eStartNfsServer->setHelpText (_("start/do not start NFS server"));
		CONNECT (eStartNfsServer->checked, ServicesSetup::StartNfsServerChanged);
		nextYPos();
	}

	// Test if Apache is installed
	if (access("/media/hdd/opt/httpd/bin/apachectl", X_OK) == 0)
	{
		eStartApache = new eCheckbox (this, oRc_Config.getStartApache(), 1);
		eStartApache->setText (_("Start Apache server"));
		eStartApache->move (ePoint (10, yPos()));
		eStartApache->resize (eSize (370, widgetHeight()));
		eStartApache->setHelpText (_("start/do not start Apache webserver"));
		CONNECT (eStartApache->checked, ServicesSetup::StartApacheChanged);
		nextYPos();
	}

	// Test if gSUB is installed
	if (access("/var/bin/gSUB", X_OK) == 0)
	{
		eStartGsub = new eCheckbox (this, oRc_Config.getStartGsub(), 1);
		eStartGsub->setText (_("Start gSUB"));
		eStartGsub->move (ePoint (10, yPos()));
		eStartGsub->resize (eSize (370, widgetHeight()));
		eStartGsub->setHelpText (_("start/do not start gSUB"));
		CONNECT (eStartGsub->checked, ServicesSetup::StartGsubChanged);
		nextYPos();
	}

	nextYPos(-30);
	buildWindow();
	CONNECT (bOK->selected, ServicesSetup::okPressed);

	// Unlock remote control
	eRCInput::getInstance()->unlock();
}

#ifndef DISABLE_NETWORK
void ServicesSetup::EnableSambaChanged(int i)
{
	fStartSambaChanged = true;
}
#endif

void ServicesSetup::StartNfsServerChanged(int i)
{
	fStartNfsServerChanged = true;
	oRc_Config.setStartNfs(i);
}

void ServicesSetup::StartApacheChanged(int i)
{
	fStartApacheChanged = true;
	oRc_Config.setStartApache(i);
}

void ServicesSetup::StartCronChanged (int i)
{
	fStartCronChanged = true;
	oRc_Config.setStartCron(i);
}

void ServicesSetup::StartGsubChanged (int i)
{
	fStartGsubChanged = true;
	oRc_Config.setStartGsub(i);
}

void ServicesSetup::StartInadynChanged (int i)
{
	fStartInadynChanged = true;
	oRc_Config.setStartInadyn(i);
}

void ServicesSetup::StartDropbearChanged (int i)
{
	fStartDropbearChanged = true;
	oRc_Config.setStartDropbear(i);
}

void ServicesSetup::EnableFirewallChanged (int i)
{
	fEnableFirewallChanged = true;
	oRc_Config.setStartFirewall(i);
}

void ServicesSetup::okPressed ()
{
	oRc_Config.WriteConfig();

#ifndef DISABLE_NETWORK
	if(fStartSambaChanged && eEnableSamba)
	{
		int enable = eEnableSamba->isChecked() ? 1 : 0;
		eConfig::getInstance()->setKey("/elitedvb/network/samba", enable);
		
		if(enable)
		{
			eDVB::getInstance()->restartSamba();
		}
		else
		{
			system("killall nmbd smbd");
		}
	}
#endif

	if(fStartInadynChanged)
	{
		if(oRc_Config.getStartInadyn() == 0)
		{
			system("killall inadyn");
		}
		else
		{
			system("/var/bin/inadyn --input_file /var/etc/inadyn.config");
		}
	}

	if(fStartDropbearChanged)
	{
		if(oRc_Config.getStartDropbear() == 0)
		{
			system("killall dropbear");
		}
		else
		{
			system("/var/bin/dropbear.sh");
		}
	}

	if(fStartNfsServerChanged)
	{
		if(oRc_Config.getStartNfs() == 0)
		{
			system("/bin/nfs.sh stop");
		}
		else
		{
			system("/bin/nfs.sh start");
		}
	}

	if(fStartApacheChanged)
	{
		if(oRc_Config.getStartApache() == 0)
		{
			system("/media/hdd/opt/httpd/bin/apachectl stop");
		}
		else
		{
			system("/media/hdd/opt/httpd/bin/apachectl start");
		}
	}

	if(fStartCronChanged)
	{
		bool useInitD = (access("/etc/init.d/busybox-cron", X_OK) == 0);
		if(oRc_Config.getStartCron() == 0) 
		{
			if(useInitD)
			{
				system("/etc/init.d/busybox-cron stop");
			}
			else
			{
				system("killall crond");
			}
		} 
		else 
		{
			if(useInitD)
			{
				system("/etc/init.d/busybox-cron start");
			} 
			else 
			{
				system("crond &");
			}
		}
	}

	if(fStartGsubChanged)
	{
		if(oRc_Config.getStartGsub() == 0)
		{
			system("killall gSUB");
		}
		else
		{
			system("/var/bin/gSUB &");
		}
	}

	if(fEnableFirewallChanged) 
	{
		if(oRc_Config.getStartFirewall() == 0) 
		{
			system("/var/bin/firewall.sh stop");
		} 
		else 
		{
			system("/var/bin/firewall.sh start");
		}
	}
	
  	close (0);
}

#ifdef SETUP_FIREWALL
void ServicesSetup::configureFirewall()
{
	FirewallSetup oFirewallSetup;
	hide();
	oFirewallSetup.show();
	oFirewallSetup.exec();
	oFirewallSetup.hide();
	show();
}
#endif // SETUP_FIREWALL

void ServicesSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
