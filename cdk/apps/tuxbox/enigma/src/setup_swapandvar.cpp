/*
 *  PLi extension to Enigma: swap and /var settings
 *
 *  Copyright (C) 2007 dAF2000 <David@daf2000.nl>
 *  Copyright (C) 2007 PLi team <http://www.pli-images.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <setup_swapandvar.h>

#include <list>
#include <sstream>
#include <fstream>
#include <iostream>

#include <lib/base/i18n.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>

#include <lib/gui/emessage.h>
#include <lib/gui/ExecuteOutput.h>
#include <enigma_mount.h>

#ifdef TARGET_CDK
	#ifdef DISABLE_SWAP
		#define MENUNAME N_("/var settings")
	#else
		#define MENUNAME N_("Swap file and /var")
	#endif
#else
	#define MENUNAME N_("Swap file")
#endif

class SwapAndVarSetupFactory : public eCallableMenuFactory
{
public:
	SwapAndVarSetupFactory() : eCallableMenuFactory("SwapAndVarSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new SwapAndVarSetup;
	}

	bool isAvailable()
	{
		int hwType = eSystemInfo::getInstance()->getHwType();

		return (hwType == eSystemInfo::DM5620)||
			(hwType == eSystemInfo::DM500) ||
			(hwType == eSystemInfo::DM7000) || 
			(hwType == eSystemInfo::DM7020) || 
			(hwType == eSystemInfo::DM600PVR);
	}
};

SwapAndVarSetupFactory SwapAndVarSetup_factory;

const char* noSwapLoc = "";
const char* usbSwapLoc = "/media/usb/swapfile";
const char* hddSwapLoc = "/media/hdd/swapfile";
const char* cfSwapLoc = "/media/cf/swapfile";
	
SwapAndVarSetup::SwapAndVarSetup(): 
	ePLiWindow(_(MENUNAME), 530),
   swap_on(0), swap_size(0), comVarLocation(0)
{
	oRc_Config.ReadConfig();
	int hwType = eSystemInfo::getInstance()->getHwType();

	if ((hwType == eSystemInfo::DM7000) || (hwType == eSystemInfo::DM7020) || (hwType == eSystemInfo::DM600PVR))
	{
		int UsbAvailable(0);
		int HDDAvailable(0);
		int CfAvailable(0);

		eConfig::getInstance()->getKey("/pli/UsbAvailable", UsbAvailable);
		eConfig::getInstance()->getKey("/pli/HDDAvailable", HDDAvailable);
		eConfig::getInstance()->getKey("/pli/CfAvailable", CfAvailable);

		eLabel* swap_on_label = new eLabel (this);
		swap_on_label->move (ePoint (10, yPos()));
		swap_on_label->resize (eSize (120, widgetHeight()));
		swap_on_label->setText (_("Swap file"));

		swap_on = new eComboBox (this, 4, swap_on_label);
		swap_on->move (ePoint (130, yPos()));
		swap_on->resize (eSize (180, widgetHeight()));
		swap_on->setHelpText (_("Make use of swap file: No, USB, HDD or CF card."));
		swap_on->loadDeco ();

		eListBoxEntryText *noswap = 0;
		eListBoxEntryText *usbswap = 0;
		eListBoxEntryText *hddswap = 0;
		eListBoxEntryText *cfswap = 0;
	
		noswap = new eListBoxEntryText (*swap_on, eString ().sprintf (_("none")), (void *) noSwapLoc);

		if(UsbAvailable) usbswap = new eListBoxEntryText (*swap_on, _("USB stick"), (void *) usbSwapLoc);
		if(HDDAvailable) hddswap = new eListBoxEntryText (*swap_on, _("Internal harddisk"), (void *) hddSwapLoc);
		if(CfAvailable) cfswap = new eListBoxEntryText (*swap_on, _("Compact Flash card"), (void *) cfSwapLoc);

		if (strcmp (oRc_Config.getSwapOn(), usbSwapLoc) == 0) 
		{
			swap_on->setCurrent (usbswap, false);
		}	
		else if (strcmp (oRc_Config.getSwapOn(), hddSwapLoc) == 0) 
		{
			swap_on->setCurrent (hddswap, false);
		} 
		else if (strcmp (oRc_Config.getSwapOn(), cfSwapLoc) == 0) 
		{	
			swap_on->setCurrent (cfswap, false);
		}
		else
		{
			swap_on->setCurrent (noswap, false);
		}

		eLabel* swap_size_label = new eLabel (this);
		swap_size_label->move (ePoint (330, yPos()));
		swap_size_label->resize (eSize (200, widgetHeight()));
		swap_size_label->setText (_("Swap size"));

		swap_size = new eComboBox (this, 4, swap_size_label);
		swap_size->move (ePoint (450, yPos()));
		swap_size->resize (eSize (70, widgetHeight()));
		swap_size->setHelpText (_("Give size of swap file in MB"));
		swap_size->loadDeco ();
		
		new eListBoxEntryText (*swap_size, "8", (void *) 8);
		new eListBoxEntryText (*swap_size, "16", (void *) 16);
		new eListBoxEntryText (*swap_size, "32", (void *) 32);
		new eListBoxEntryText (*swap_size, "64", (void *) 64);

		if (strcmp (oRc_Config.getSwapSize(), "8") == 0) 
		{
			swap_size->setCurrent (0, false);
		}
		else if (strcmp (oRc_Config.getSwapSize(), "32") == 0) 
		{
			swap_size->setCurrent (2, false);
		}
		else if (strcmp (oRc_Config.getSwapSize(), "64") == 0) 
		{
			swap_size->setCurrent (3, false);
		}	
		else 
		{
			swap_size->setCurrent (1, false);
		}

		nextYPos(35);
	} // DM7000, DM7020, DM600PVR

	// TODO: moveVar should become an Enigma config
	// Move var for DM7000, DM5620 and DM500 only
	if((hwType == eSystemInfo::DM7000) ||
		(hwType == eSystemInfo::DM5620)||
		(hwType == eSystemInfo::DM500))
	{
		// Only show var setup if not in multiboot
		if(access("/go", R_OK) != 0)
		{
			// Check if VAR is on USB, then do not show USB item
			int UsbAvailable(0);
			int HDDAvailable(0);
			int CfAvailable(0);
			int MountedOnUSB(0);
			int MountedOnHDD(0);
			int MountedOnCf(0);
			int MountedOnNetwork(0);

			eConfig* config = eConfig::getInstance();
			config->getKey("/pli/UsbAvailable", UsbAvailable);
			config->getKey("/pli/HDDAvailable", HDDAvailable);
			config->getKey("/pli/CfAvailable", CfAvailable);
			config->getKey("/pli/MountedOnUSB", MountedOnUSB);
			config->getKey("/pli/MountedOnHDD", MountedOnHDD);
			config->getKey("/pli/MountedOnCf", MountedOnCf);
			config->getKey("/pli/MountedOnNetwork", MountedOnNetwork);

			eLabel* lblVarLocation = new eLabel(this);
			comVarLocation = new eComboBox(this, 4, lblVarLocation);
	
			lblVarLocation->move(ePoint(10, yPos()));
			lblVarLocation->resize(eSize(170, widgetHeight()));
			lblVarLocation->setText(_("/var location"));
			lblVarLocation->loadDeco();

			comVarLocation->move(ePoint(130, yPos()));
			comVarLocation->resize(eSize(390, widgetHeight()));
			comVarLocation->setHelpText(_("Select the device to store /var on"));
			comVarLocation->clear();
	
			if ((MountedOnUSB == 0) && (MountedOnHDD == 0) && (MountedOnCf == 0) && (MountedOnNetwork == 0))
			{
				new eListBoxEntryText (*comVarLocation, _("Internal flash"), (void *)NO_ACTION);
			
				if (UsbAvailable == 1)
				{
					new eListBoxEntryText (*comVarLocation, _("Move to USB stick"), (void *)TO_USB);
				}

				if (HDDAvailable == 1)
				{
					new eListBoxEntryText (*comVarLocation, _("Move to harddisk"), (void *)TO_HDD);
				}

				if (CfAvailable == 1)
				{
					new eListBoxEntryText (*comVarLocation, _("Move to CF card"), (void *)TO_CF);
				}
				
				eNetworkMountMgr* networkMountMgr = eNetworkMountMgr::getInstance();
				int mountIndex = networkMountMgr->searchMountPoint("/media/var");
				if(mountIndex != -1)
				{
					eMountPoint mnt = networkMountMgr->getMountPointData(mountIndex);
					if(mnt.isMounted())
					{
						new eListBoxEntryText (*comVarLocation, (eString)_("Move to") + (eString)" " + mnt.getLongDescription(), (void *)TO_NETWORK);
					}
				}
				else
				{
					nextYPos(40);
					eLabel* lblHint = new eLabel(this);
					lblHint->move(ePoint(10, yPos()));
					lblHint->resize(eSize(510, widgetHeight()));
					lblHint->setText(_("Mount on /media/var to enable move /var to network"));
				}
			}
			else
			{
				// mount is already on usb, hdd of cf, offer item to remove
				if (MountedOnUSB == 1)
				{
					new eListBoxEntryText (*comVarLocation, _("Keep on USB stick"), (void *)NO_ACTION);
					new eListBoxEntryText (*comVarLocation, _("Move to internal flash"), (void *)FROM_USB);
				}

				if (MountedOnHDD == 1)
				{
					new eListBoxEntryText (*comVarLocation, _("Keep on harddisk"), (void *)NO_ACTION);
					new eListBoxEntryText (*comVarLocation, _("Move to internal flash"), (void *)FROM_HDD);
				}

				if (MountedOnCf == 1)
				{
					new eListBoxEntryText (*comVarLocation, _("Keep on CF card"), (void *)NO_ACTION);
					new eListBoxEntryText (*comVarLocation, _("Move to internal flash"), (void *)FROM_CF);
				}

				if (MountedOnNetwork == 1)
				{
					new eListBoxEntryText (*comVarLocation, _("Keep on network"), (void *)NO_ACTION);
					new eListBoxEntryText (*comVarLocation, _("Move to internal flash"), (void *)FROM_NETWORK);
				}
			}

			comVarLocation->setCurrent((void *)NO_ACTION);
			comVarLocation->loadDeco();
		} // access("/go", R_OK) != 0
		else
		{
			nextYPos(5);
			eLabel* lblNoVar = new eLabel(this);
			lblNoVar->move(ePoint(10, yPos()));
			lblNoVar->resize(eSize(510, widgetHeight()));
			lblNoVar->setText(_("You are running in multiboot, moving /var is disabled"));
		}
	}
	
	buildWindow();
	CONNECT (bOK->selected, SwapAndVarSetup::okPressed);
}

void SwapAndVarSetup::okPressed()
{
	int swapSize = 16;
	int oldSwapSize = atoi(oRc_Config.getSwapSize());
	eString swapFile = "";
	eString oldSwapFile = oRc_Config.getSwapOn();
	 
	if(swap_size) 
	{
		swapSize = (int)swap_size->getCurrent()->getKey();
	} 
	
	if(swap_on) 
	{
		swapFile = (const char*)swap_on->getCurrent()->getKey();
	}
	
	if((swapSize != oldSwapSize) || (swapFile != oldSwapFile))
	{
		eMessageBox mb(_("Creating swap file"), _("Please wait"), eMessageBox::iconInfo);
		mb.show();
		
		oRc_Config.setSwapSize(eString().sprintf("%d", swapSize).c_str());
		oRc_Config.setSwapOn(swapFile.c_str());
		oRc_Config.WriteConfig();

		addSwapToFstab(swapFile);
		
		if(oldSwapFile)
		{
			eDebug("SwapAndVarSetup::okPressed switching off swap on %s", oldSwapFile.c_str());
			eString command = "swapoff " + oldSwapFile;
			system(command.c_str());
		}
		
		if(swapFile)
		{
			createSwapFile(swapFile, swapSize);
		}
		
		mb.hide();
	}

	if(comVarLocation)
	{
		switch((int)comVarLocation->getCurrent()->getKey())
		{
			case TO_HDD:
				move_to_hdd();
				break;
				
			case TO_USB:
				move_to_usb();
				break;
				
			case TO_CF:
				move_to_cf();
				break;
				
			case TO_NETWORK:
				move_to_network();
				break;
				
			case FROM_HDD:
				remove_from_hdd();
				break;
				
			case FROM_USB:
				remove_from_usb();
				break;
				
			case FROM_CF:
				remove_from_cf();
				break;
				
			case FROM_NETWORK:
				remove_from_network();
				break;
				
			default:
				break;
		}
	}
	
	close(0);
}

void SwapAndVarSetup::move_to_hdd()
{
	eDebug("move_to_hdd");
	moveVar(_("harddisk"), "move", "/media/hdd");
}

void SwapAndVarSetup::remove_from_hdd()
{
	eDebug("remove_from_hdd");
	moveVar(_("harddisk"), "remove", "/media/hdd");
}

void SwapAndVarSetup::move_to_usb()
{
	eDebug("move_to_usb");
	moveVar(_("USB stick"), "move", "/media/usb");
}

void SwapAndVarSetup::remove_from_usb()
{
	eDebug("remove_from_usb");
	moveVar(_("USB stick"), "remove", "/media/usb");
}

void SwapAndVarSetup::move_to_cf()
{
	eDebug("move_to_cf");
	moveVar(_("CF card"), "move", "/media/cf");
}

void SwapAndVarSetup::remove_from_cf()
{
	eDebug("remove_from_cf");
	moveVar(_("CF card"), "remove", "/media/cf");
}

void SwapAndVarSetup::move_to_network()
{
	eDebug("move_to_network");
	moveVar(_("network"), "move", "/media/var");
}

void SwapAndVarSetup::remove_from_network()
{
	eDebug("remove_from_network");
	moveVar(_("network"), "remove", "/media/var");
}

void SwapAndVarSetup::moveVar(
	const eString& medium,
	const eString& action,
	const eString& mountpoint)
{
	hide();
	eMessageBox Check(eString().sprintf(_("/var will be %s %s.\n"
		"After that, your dreambox will reboot automatically.\n"
		"Are you sure you want to do this?\n"),
		(action == "move") ? _("moved to") : _("removed from"), 
		medium.c_str()),
		_("Are you sure?"),
		eMessageBox::iconQuestion|eMessageBox::btYes|eMessageBox::btNo,
		eMessageBox::btNo);

	const eSize s = Check.getSize();
	const ePoint p = Check.getPosition();
	Check.move(ePoint((parent->width() - s.width()) / 2, p.y()));

	Check.show();
	int iResult = Check.exec();
	Check.hide();

	if (iResult == eMessageBox::btYes) 
	{
		ExecuteOutput run(eString().sprintf(_("%s %s, please wait!"),
			(action == "move") ? _("Moving /var to") : _("Removing /var from"),
			medium.c_str()), 
			"/bin/movevar.sh " + action + " " + mountpoint);
		run.show();
		run.exec();
		run.hide();
	}
	
	show();
}

void SwapAndVarSetup::addSwapToFstab(const eString& swapFile)
{
	eDebug("SwapAndVarSetup::addSwapToFstab adding %s", swapFile.c_str());
	std::list<eString> mountEntries;
	std::ifstream fstabIn;
	eString fstabEntry;
	
	// First read each line in /etc/fstab
	fstabIn.open("/etc/fstab", std::ifstream::in);
	if(fstabIn.is_open())
	{
		while(getline(fstabIn, fstabEntry, '\n'))
		{
			fstabEntry = fstabEntry + "\n";
			mountEntries.push_back(fstabEntry);
		}
	
		fstabIn.close();
	}
	
	// Search for the first swapfile entry
	if(mountEntries.begin() != mountEntries.end())
	{
		std::stringstream fstabStr;
		eString fsname;
		eString fsdir;
		eString fstype;
		eString fsopts;
		
		fstabStr.str(*mountEntries.begin());
		fstabStr >> fsname >> fsdir >> fstype >> fsopts;
		fstabStr.clear();
		
		if((fsdir == "none") && (fstype == "swap") && (fsopts == "sw"))
		{
			// Swapfile entry found, remove it
			mountEntries.erase(mountEntries.begin());
		}
	}
	
	// Add the new swapfile entry (at the front)
	if(swapFile)
	{
		fstabEntry = swapFile + "\tnone\tswap\tsw\t0\t0\n";
		mountEntries.push_front(fstabEntry);
	}
	
	// And rewrite a new fstab again
	std::ofstream fstabOut;

	fstabOut.open("/etc/fstab", std::ifstream::out);
	if(fstabOut.is_open())
	{
		for(std::list<eString>::iterator i = mountEntries.begin(); 
			i != mountEntries.end(); ++i)
		{
			fstabOut << *i;
		}
	
		fstabOut.close();
	}
}

void SwapAndVarSetup::createSwapFile(const eString& swapFile, int swapSize)
{
	eString command;

	eDebug("SwapAndVarSetup::createSwapFile creating %s, size=%d", 
		swapFile.c_str(), swapSize);

	// Create the swapfile
	command = "dd if=/dev/zero of=" + swapFile + 
		" bs=1k count=" + eString().sprintf("%d", swapSize * 1024);
	system(command.c_str());
	
	command = "mkswap " + swapFile;
	system(command.c_str());
	
	// Enable the swapfile
	command = "swapon " + swapFile;
	system(command.c_str());
}

void SwapAndVarSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}
