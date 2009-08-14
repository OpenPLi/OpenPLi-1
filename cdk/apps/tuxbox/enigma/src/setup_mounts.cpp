/*
 *  PLi extension to Enigma: setup mounts (extracted from setupnetwork)
 *
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

#ifndef DISABLE_NETWORK

#include <setup_mounts.h>

#include <netinet/in.h>
#include <linux/route.h>

#ifndef DISABLE_NFS
#include <sys/mount.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <lib/gui/multipage.h>
#include <lib/gui/emessage.h>
#include <lib/base/console.h>
#endif

#ifdef USE_IFUPDOWN
// in Makefile.am INCLUDES @NET_CFLAGS@
// in configure.ac TUXBOX_APPS_LIB_PKGCONFIG(NET,tuxbox-net)
#include <sys/types.h>
#include <sys/wait.h>
#include <network_interfaces.h> /* getInetAttributes, setInetAttributes */
#endif

#include <enigma.h>
#include <setupnetwork.h>
#include <lib/base/i18n.h>
#include <lib/dvb/edvb.h>
#include <lib/driver/rc.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>

#ifndef DISABLE_NFS

#define MENUNAME N_("Network mounts")

class eMountSetupFactory : public eCallableMenuFactory
{
public:
	eMountSetupFactory() : eCallableMenuFactory("eMountSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eMountSetupWrapper;
	}
	
	bool isAvailable()
	{
		return eSystemInfo::getInstance()->hasNetwork();
	}
};

eMountSetupFactory eMountSetup_factory;

void eMountSetupWrapper::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
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
		setup->setLCD(lcdTitle, lcdElement);
#endif
		setup->show();
		result = setup->exec();
		setup->hide();
		delete setup;
	}
}

eMountSetup::eMountSetup(int nrOfMounts)
	:eListBoxWindow<eListBoxEntryMenu>(_(MENUNAME), nrOfMounts + 1, 410, true)
{
	valign();

	networkMountMgr = eNetworkMountMgr::getInstance();
	int nrNetworkMounts = networkMountMgr->mountPointCount();

	for(int i = 0; i < nrNetworkMounts; ++i)
	{
		eMountPoint mnt = networkMountMgr->getMountPointData(i);
		eString helptext = mnt.localDir + " (";
		bool isMounted = networkMountMgr->isMounted(i);

		if(isMounted)
		{
			helptext += _("mounted");
		}
		else
		{
			helptext += _("not mounted");
		}
		
		helptext = helptext + ")\n" + _("Show, edit or remove this mount");

		(new eListBoxEntryCheck(&list, mnt.getLongDescription(), 
			helptext, 0, (void*)i))->setChecked(isMounted);
	}

	new eListBoxEntryCheck(&list, _("Add a new mount"), _("Add a new mount"), 0, (void*)-1);
	CONNECT(list.selected, eMountSetup::selectedMount);
}

void eMountSetup::selectedMount(eListBoxEntryMenu* item)
{
	if(item)
	{
		eMountDetails mountScreen((int)item->getKey());
		mountScreen.show();
		int result = mountScreen.exec();
		mountScreen.hide();
		
		if(result == 1)
		{
			close(1); // Force eMountSetup to be restarted
		}
	}
}

eMountDetails::eMountDetails(int mountIndex)
	:ePLiWindow("", 530), currentMountId(mountIndex)
{
	const int labelSize = 160;
	const int entryPos = labelSize + 20;
	const int entrySize = 530 - entryPos - 10;
	
	networkMountMgr = eNetworkMountMgr::getInstance();
	eString windowTitle;
	
	currentMount = networkMountMgr->getMountPointData(mountIndex);

	if(mountIndex == -1)
	{
		windowTitle = _("New network mount");
		currentMount.fstype = eMountPoint::cifsMount;
		currentMount.localDir = "/media/server1";
		currentMount.options = "nolock,intr,soft,rsize=8192,wsize=8192";
	}
	else
	{
		windowTitle = currentMount.getLongDescription() + " - " + (eString)_("network mount");
	}
	
	setText(windowTitle);

	eLabel* lbDesc = new eLabel(this);
	lbDesc->setText(_("Description"));
	lbDesc->move(ePoint(10, yPos()));
	lbDesc->resize(eSize(labelSize, widgetHeight()));
	
	tiDesc = new eTextInputField(this, lbDesc);
	tiDesc->setText(currentMount.description);
	tiDesc->setHelpText(_("Enter an optional description"));
	tiDesc->loadDeco();
	tiDesc->move(ePoint(entryPos, yPos()));
	tiDesc->resize(eSize(entrySize, widgetHeight()));
	
	nextYPos(35);

	eLabel* lbFS = new eLabel(this);
	lbFS->setText(_("Filesystem"));
	lbFS->move(ePoint(10, yPos()));
	lbFS->resize(eSize(labelSize, widgetHeight()));
	
	cbFS = new eComboBox(this, 3);
	cbFS->loadDeco();
	cbFS->move(ePoint(entryPos, yPos()));
	cbFS->resize(eSize(100, widgetHeight()));
	cbFS->setHelpText(_("Select filesystem type"));

	bool haveCifs = false;
	bool haveSmbfs = false;
	FILE* f = fopen("/proc/filesystems", "rt");

	if(f)
	{
		while(true)
		{
			char buffer[128];
			if(!fgets(buffer, 128, f))
			{
				break;
			}
			
			if(strstr(buffer, "cifs"))
			{
				haveCifs = true;
			}
			
			if(strstr(buffer, "smbfs"))
			{
				haveSmbfs = true;
			}
		}
		
		fclose(f);
	}

	new eListBoxEntryText(*cbFS, "NFS", (void*)0, 0, _("Especially for Linux and Dreamboxes"));
	if(haveCifs)
	{
		new eListBoxEntryText(*cbFS, "CIFS", (void*)1, 0, _("Especially for Windows, Linux, Dreamboxes and NAS devices"));
	}
	if(haveSmbfs)
	{
		new eListBoxEntryText(*cbFS, "SMBFS", (void*)2, 0, _("Especially for some old NAS devices"));
	}
	
	cbFS->setCurrent((void*)currentMount.fstype);
	((eMountDetails*)cbFS)->setProperty("showEntryHelp", "");
	
	chAutoMount = new eCheckbox(this, 0, 1);
	chAutoMount->setText(_("Auto mount"));
	chAutoMount->move(ePoint(entryPos + 110, yPos()));
	chAutoMount->resize(eSize(530 - (entryPos + 110) - 10, widgetHeight()));
	chAutoMount->setHelpText(_("Mount this share when Enigma starts"));
	chAutoMount->setCheck(currentMount.automount);

	nextYPos(35);

	cbIP = new eComboBox(this, 2);
	cbIP->loadDeco();
	cbIP->move(ePoint(10, yPos()));
	cbIP->resize(eSize(labelSize, widgetHeight()));
	cbIP->setHelpText(_("Select IP address or hostname"));
	
	new eListBoxEntryText(*cbIP, _("IP address"), (void*)0);
	new eListBoxEntryText(*cbIP, _("Hostname"), (void*)1);

	if(currentMount.remoteIP == currentMount.remoteHost)
	{
		// If hostname is IP, it must be an IP number
		cbIP->setCurrent((void*)0);
	}
	else
	{
		cbIP->setCurrent((void*)1);
	}
	
	tiHost = new eTextInputField(this, cbIP);
	tiHost->setText(currentMount.remoteHost);
	tiHost->setHelpText(_("Enter hostname"));
	tiHost->loadDeco();
	tiHost->move(ePoint(entryPos, yPos()));
	tiHost->resize(eSize(entrySize, widgetHeight()));
	
	int ipAddress[4] = {0};
	sscanf(currentMount.remoteIP.c_str(), "%d.%d.%d.%d",
		&ipAddress[0], &ipAddress[1], &ipAddress[2], &ipAddress[3]);
		
	// Get default IP if empty
	if(ipAddress[0] == 0)
	{
		__u32 ip;
		__u32 netmask;
		
		#if 0 // Does not work for DHCP
			eConfig::getInstance()->getKey("/elitedvb/network/ip", ip);
			eConfig::getInstance()->getKey("/elitedvb/network/netmask", netmask);
		#else
			eZapNetworkSetup::getIP("eth0", ip, netmask);
		#endif
		
		ip &= netmask;
		eNumber::unpack(ip, ipAddress);
	}
	
	nuIP = new eNumber(this, 4, 0, 255, 3, ipAddress, 0, cbIP);
	nuIP->setHelpText(_("Enter IP address"));
	nuIP->setFlags(eNumber::flagDrawPoints);
	nuIP->loadDeco();
	nuIP->move(ePoint(entryPos, yPos()));
	nuIP->resize(eSize(entrySize, widgetHeight()));
	
	nextYPos(35);

	eLabel* lbMountPoint = new eLabel(this);
	lbMountPoint->setText(_("Mountpoint"));
	lbMountPoint->move(ePoint(10, yPos()));
	lbMountPoint->resize(eSize(labelSize, widgetHeight()));

	cbMountPoint = new eMountSelectionComboBox(
		this, 5, lbMountPoint, 
		eMountSelectionComboBox::ShowMountpoints |
		eMountSelectionComboBox::ShowCustomLocation);

	cbMountPoint->loadDeco();
	cbMountPoint->move(ePoint(entryPos, yPos()));
	cbMountPoint->resize(eSize(entrySize, widgetHeight()));
	cbMountPoint->setHelpText(_("Select local mountpoint"));
	cbMountPoint->setCurrentLocation(currentMount.localDir);
	CONNECT(cbMountPoint->selchanged, eMountDetails::mountpointChanged);

	nextYPos(35);

	eLabel* lbServerDir = new eLabel(this);
	lbServerDir->setText(_("Server directory"));
	lbServerDir->move(ePoint(10, yPos()));
	lbServerDir->resize(eSize(labelSize, widgetHeight()));
	
	tiServerDir = new eTextInputField(this, lbServerDir);
	tiServerDir->setText(currentMount.mountDir);
	tiServerDir->setHelpText(_("Enter the server directory"));
	tiServerDir->loadDeco();
	tiServerDir->move(ePoint(entryPos, yPos()));
	tiServerDir->resize(eSize(entrySize, widgetHeight()));
	
	nextYPos(35);

	eLabel* lbUser = new eLabel(this);
	lbUser->setText(_("Username"));
	lbUser->move(ePoint(10, yPos()));
	lbUser->resize(eSize(labelSize, widgetHeight()));
	
	tiUser = new eTextInputField(this, lbUser);
	tiUser->setText(currentMount.userName);
	tiUser->setHelpText(_("Enter the username of the server"));
	tiUser->loadDeco();
	tiUser->move(ePoint(entryPos, yPos()));
	tiUser->resize(eSize(entrySize, widgetHeight()));
	
	nextYPos(35);

	eLabel* lbPass = new eLabel(this);
	lbPass->setText(_("Password"));
	lbPass->move(ePoint(10, yPos()));
	lbPass->resize(eSize(labelSize, widgetHeight()));
	
	tiPass = new eTextInputField(this, lbPass);
	tiPass->setText(currentMount.password);
	tiPass->setHelpText(_("Enter the password of the server"));
	tiPass->loadDeco();
	tiPass->move(ePoint(entryPos, yPos()));
	tiPass->resize(eSize(entrySize, widgetHeight()));

	nextYPos(50);
	
	// Reuse of bCancel and bOK in ePLiWindow, not very nice, but works
	bCancel = new eButton(this);
	bCancel->setText(_("Delete"));
	bCancel->setShortcut("red");
	bCancel->setShortcutPixmap("red");
	bCancel->move(ePoint(10, yPos()));
	bCancel->resize(eSize(120, 40));
	bCancel->setHelpText(_("Delete this mount entry"));
	bCancel->loadDeco();
	CONNECT(bCancel->selected, eMountDetails::deletePressed);
	
	bOK = new eButton(this);
	bOK->setText(_("Save"));
	bOK->setShortcut("green");
	bOK->setShortcutPixmap("green");
	bOK->move(ePoint(140, yPos()));
	bOK->resize(eSize(120, 40));
	bOK->setHelpText(_("Save this mount entry"));
	bOK->loadDeco();
	CONNECT(bOK->selected, eMountDetails::savePressed);
	
	eButton* btMount = new eButton(this);
	btMount->setShortcut("yellow");
	btMount->setShortcutPixmap("yellow");
	btMount->move(ePoint(270, yPos()));
	btMount->resize(eSize(120, 40));
	btMount->loadDeco();
	
	if(currentMount.isMounted())
	{
		btMount->setText(_("Unmount"));
		btMount->setHelpText(_("Unmount this network mount"));
	}
	else
	{
		btMount->setText(_("Mount"));
		if(currentMountId == -1)
		{
			btMount->setHelpText(_("Save and mount this network mount"));
		}
		else
		{
			btMount->setHelpText(_("Mount this network mount"));
		}
	}
	
	CONNECT(btMount->selected, eMountDetails::mountPressed);
	
	eButton* btOptions = new eButton(this);
	btOptions->setText(_("Options"));
	btOptions->setShortcut("blue");
	btOptions->setShortcutPixmap("blue");
	btOptions->move(ePoint(400, yPos()));
	btOptions->resize(eSize(120, 40));
	btOptions->setHelpText(_("Advanced options for this mount"));
	btOptions->loadDeco();
	CONNECT(btOptions->selected, eMountDetails::optionsPressed);
	
	buildWindow();

	CONNECT(cbIP->selchanged, eMountDetails::hostIPChanged);
	hostIPChanged(cbIP->getCurrent());
}

void eMountDetails::hostIPChanged(eListBoxEntryText *sel)
{
	if(sel)
	{
		if((int)sel->getKey() == 0) // IP address
		{
			tiHost->hide();
			nuIP->show();
		}
		else // Hostname
		{
			nuIP->hide();
			tiHost->show();
		}
	}
}

void eMountDetails::deletePressed()
{
	eMessageBox mb(
		_("Are you sure to delete this mount?"),
		_("Remove mount"),
		eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion,
		eMessageBox::btNo);
		
	mb.show();
	int result = mb.exec();
	mb.hide();
	
	if(result == eMessageBox::btYes)
	{
		networkMountMgr->unmountMountPoint(currentMountId);
		networkMountMgr->removeMountPoint(currentMountId);
		close(1); // Force eMountSetup to be restarted
	}
}

eString eMountDetails::saveMount()
{
	eString result;
	
	currentMount.description = tiDesc->getText();
	currentMount.fstype = (enum eMountPoint::mountType)(int)cbFS->getCurrent()->getKey();
	currentMount.automount = (int)chAutoMount->isChecked();
	
	if(cbIP->getCurrent()->getKey() == 0) // IP address
	{
		currentMount.remoteHost.sprintf("%d.%d.%d.%d",
			nuIP->getNumber(0), nuIP->getNumber(1), 
			nuIP->getNumber(2), nuIP->getNumber(3));
	}
	else // Hostname
	{
		if(tiHost->getText().length() != 0)
		{
			currentMount.remoteHost = tiHost->getText();
		}
		else
		{
			result = _("Hostname is empty");
		}
	}
	
	if(result.length() == 0)
	{
		currentMount.setIP();
		currentMount.localDir = cbMountPoint->getCurrent()->getText();
		
		if(tiServerDir->getText().length() != 0)
		{
			currentMount.mountDir = tiServerDir->getText();
		}
		else
		{
			result = _("Server directory is empty");
		}
	}
	
	if(result.length() == 0)
	{
		currentMount.userName = tiUser->getText();
		currentMount.password = tiPass->getText();
		// currentMount.options and currentMount.linuxExtensions
		// is set by eMountOptions::okPressed()
	
		if(currentMountId == -1)
		{
			currentMountId = networkMountMgr->addMountPoint(currentMount);
			
			if(currentMountId == -1)
			{
				result = _("Local directory is already in use");
			}
		}
		else
		{
			networkMountMgr->changeMountPoint(currentMountId, currentMount);
		}
	}
	
	return result;
}

void eMountDetails::savePressed()
{
	eString result;
	
	result = saveMount();
	
	eMessageBox::ShowBox(
		(result.length() == 0) ?
		(eString)_("Mount entry saved") +"\n" + (eString)_("This mountpoint is unmounted now")
		: (eString)_("Saving mount entry failed") +"\n" + result,
		_("Saving mount entry"),
		(result ? eMessageBox::iconError : eMessageBox::iconInfo) | eMessageBox::btOK);

	if(result.length() == 0)
	{
		close(1); // Force eMountSetup to be restarted
	}
}

void eMountDetails::mountPressed()
{
	eString result;
	
	if(currentMount.isMounted())
	{
		// Unmount now
		result = networkMountMgr->unmountMountPoint(currentMountId);
		
		eMessageBox::ShowBox(
			(result.length() != 0) ? (eString)_("Unmounting failed") + "\n" + result
			: _("Unmounting succeeded"),
			_("Mount status"),
			(result ? eMessageBox::iconError : eMessageBox::iconInfo) | eMessageBox::btOK);
	}
	else
	{
		// Mount and save now
		result = saveMount();
		
		if(result.length() == 0)
		{
			result = networkMountMgr->mountMountPoint(currentMountId);
		}
		
		eMessageBox::ShowBox(
			(result.length() != 0) ? (eString)_("Mounting failed") + "\n" + result 
			: _("Mounting succeeded"),
			_("Mount status"),
			(result ? eMessageBox::iconError : eMessageBox::iconInfo) | eMessageBox::btOK);
	}
	
	if(result.length() == 0)
	{
		close(1); // Force eMountSetup to be restarted
	}
}

void eMountDetails::optionsPressed()
{
	eMountOptions mountOptions(&currentMount);
	
	mountOptions.show();
	mountOptions.exec();
	mountOptions.hide();
}

void eMountDetails::mountpointChanged(eListBoxEntryText *sel)
{
	cbMountPoint->locationChanged(sel);
}

static const char* mountOptionStrings[] =
	{ "sync", "atime", "execm", "noexec", "ro", "rw", "users", "nolock", "intr", "soft", "tcp" };

static const int mountOptionSize = sizeof(mountOptionStrings) / sizeof(char*);

eMountOptions::eMountOptions(eMountPoint* currentMount)
	:ePLiWindow(_("Mount options"), 490), currentMount(currentMount)
{
	const int chSize = 150;
	const int columns = 3;
	int currentColumn = 0;
	
	for(int i = 0; i < mountOptionSize; ++i)
	{
		chOption[i] = new eCheckbox(this);
		chOption[i]->setText(mountOptionStrings[i]);
		chOption[i]->setHelpText((eString)_("Activate mount option") + " " + (eString)mountOptionStrings[i]);
		chOption[i]->move(ePoint((chSize + 10) * currentColumn + 10, yPos()));
		chOption[i]->resize(eSize(chSize, widgetHeight()));
		
		++currentColumn;
		
		if(currentColumn == columns)
		{
			currentColumn = 0;
			nextYPos();
		}
	}
	
	chLinuxExt = new eCheckbox(this);
	chLinuxExt->setText(_("LinuxExt"));
	chLinuxExt->setHelpText(_("Enable Linux extensions (some NAS devices need this disabled)"));
	chLinuxExt->move(ePoint((chSize + 10) * currentColumn + 10, yPos()));
	chLinuxExt->setCheck(currentMount->linuxExtensions);
	chLinuxExt->resize(eSize(chSize, widgetHeight()));
	
	unsigned int pos = 0;
	eString tmpOptions = currentMount->options;
	eString option;
	eString extraOptions;
	int rsize = 8192;
	int wsize = 8192;
	
	while(tmpOptions.length() > 0)
	{
		int i;
		
		if((pos = tmpOptions.find(",")) != eString::npos)
		{
			option = tmpOptions.substr(0, pos);
			tmpOptions = tmpOptions.substr(pos + 1);
		}
		else
		{
			option = tmpOptions;
			tmpOptions = "";
		}

		for(i = 0; i < mountOptionSize; ++i)
		{
			if(option == (eString)mountOptionStrings[i])
			{
				chOption[i]->setCheck(1);
				break;
			}
		}
		
		if(i == mountOptionSize)
		{
			// Not one of the standard options
			if(option.left(5) == "rsize")
			{
				rsize = atoi(option.substr(6).c_str());
			}
			else if(option.left(5) == "wsize")
			{
				wsize = atoi(option.substr(6).c_str());
			}
			else
			{
				// Option could not be found, so is an extra option
				extraOptions += (extraOptions) ? ("," + option) : option;
			}
		}
	}
	
	nextYPos(35);
	
	eLabel* lbRsize = new eLabel(this);
	lbRsize->setText("rsize");
	lbRsize->move(ePoint(10, yPos()));
	lbRsize->resize(eSize(80, widgetHeight()));
	
	cbRsize = new eComboBox(this, 5);
	cbRsize->loadDeco();
	cbRsize->move(ePoint(100, yPos()));
	cbRsize->resize(eSize(100, widgetHeight()));
	cbRsize->setHelpText(_("Select read buffer size"));
	
	eLabel* lbWsize = new eLabel(this);
	lbWsize->setText("wsize");
	lbWsize->move(ePoint(290, yPos()));
	lbWsize->resize(eSize(80, widgetHeight()));
	
	cbWsize = new eComboBox(this, 5);
	cbWsize->loadDeco();
	cbWsize->move(ePoint(380, yPos()));
	cbWsize->resize(eSize(100, widgetHeight()));
	cbWsize->setHelpText(_("Select write buffer size"));
	
	for(unsigned int i=1024; i<=65536; i*=2)
	{
		new eListBoxEntryText(*cbRsize, eString().sprintf("%d", i), (void*)i);
		new eListBoxEntryText(*cbWsize, eString().sprintf("%d", i), (void*)i);
	}
	
	cbRsize->setCurrent((void*)rsize);
	cbWsize->setCurrent((void*)wsize);

	nextYPos(35);
	
	eLabel* lbOptions = new eLabel(this);
	lbOptions->setText(_("Extra mount options"));
	lbOptions->move(ePoint(10, yPos()));
	lbOptions->resize(eSize(200, widgetHeight()));
	
	tiOptions = new eTextInputField(this, lbOptions);
	tiOptions->setText(extraOptions);
	tiOptions->setHelpText(_("Extra options for the mount"));
	tiOptions->loadDeco();
	tiOptions->move(ePoint(210, yPos()));
	tiOptions->resize(eSize(270, widgetHeight()));
	
	buildWindow();
	CONNECT(bOK->selected, eMountOptions::okPressed);
}

void eMountOptions::okPressed()
{
	eString options;
	
	for(int i = 0; i < mountOptionSize; ++i)
	{
		if(chOption[i]->isChecked())
		{
			options = options + (eString)mountOptionStrings[i] + ",";
		}
	}
	
	eString extraOptions = tiOptions->getText();
	
	if(extraOptions.length() > 0)
	{
		options = options + extraOptions + ",";
	}
	
	int rsize = (int)cbRsize->getCurrent()->getKey();
	int wsize = (int)cbWsize->getCurrent()->getKey();

	options = options + eString().sprintf("rsize=%d,wsize=%d", rsize, wsize);

	currentMount->options = options;
	currentMount->linuxExtensions = chLinuxExt->isChecked() ? 1 : 0;
	
	close(0);
}

/*** All below this line is the old mount manager ***/

#if 0
static void errorMessage(const eString message, int type=0)
{
	int flags;
	if(type==1)
		flags = eMessageBox::iconInfo|eMessageBox::btOK;
	else	
		flags = eMessageBox::iconWarning|eMessageBox::btOK;
	eMessageBox mb(message, _("Info"), flags);
	mb.show();
	mb.exec();
	mb.hide();
}

struct NFSSetupActions
{
        eActionMap map;
        eAction prevShare, nextShare;
        NFSSetupActions():
                map("NFSSetup", _("NFS setup actions")),
                prevShare(map, "prevShare", _("Goto previous share"), eAction::prioDialog),
                nextShare(map, "nextShare", _("Goto Next share"), eAction::prioDialog)
        {
        }
};

eAutoInitP0<NFSSetupActions> i_NFSSetupActions(eAutoInitNumbers::actions, "NFS setup actions");

eNFSSetup::eNFSSetup()
	:eWindow(0), timeout(eApp), mountContainer(0)
{
//	init_eNFSSetup();
//}
//
//void eNFSSetup::init_eNFSSetup()
//{
	// eDebug("NFSSetup] checking fs types");
	bool have_cifs = false;
	bool have_smbfs = false;
	FILE *f=fopen("/proc/filesystems", "rt");
	if (f)
	{
		while (1)
		{
			char buffer[128];
			if (!fgets(buffer, 128, f))
				break;
			if ( strstr(buffer, "cifs") )
				have_cifs=true;
			if ( strstr(buffer, "smbfs") )
				have_smbfs=true;
		}
		fclose(f);
	}

	// eDebug("NFSSetup] Building widgets");
//	CONNECT(timeout.timeout, eNFSSetup::mountTimeout);

	addActionMap(&i_NFSSetupActions->map);
	cresize(eSize(455, 400));
	valign();

	int fd=eSkin::getActive()->queryValue("fontsize", 20);

	lip = new eLabel(this);
	lip->resize(eSize(120,fd+4));
	lip->setText("Host/IP:");

	shost = new eTextInputField(this, lip);
	shost->resize(eSize(200, fd+10));
	shost->setUseableChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.");
	shost->loadDeco();
	shost->setHelpText(_("IP address or Fully qualified hostname"));

	combo_fstype=new eComboBox(this, 2);
	combo_fstype->resize(eSize(100, fd+10));
	combo_fstype->loadDeco();
	combo_fstype->setHelpText(_("press ok to change mount type"));
	new eListBoxEntryText( *combo_fstype, "NFS", (void*)0, 0, "Network File System");
	if (have_cifs)
		new eListBoxEntryText( *combo_fstype, "CIFS", (void*)1, 0, "Common Internet File System");
	if (have_smbfs)
		new eListBoxEntryText( *combo_fstype, "SMBFS", (void*)2, 0, _("Samba File System(to mount share from another Dreambox)"));
	combo_fstype->setCurrent((void*)0, true);
	CONNECT(combo_fstype->selchanged, eNFSSetup::fstypeChanged);

	lsdir = new eLabel(this);
	lsdir->resize(eSize(120, fd+4));
	lsdir->setText(_("Dir:"));

	sdir = new eTextInputField(this,lsdir);
	sdir->resize(eSize(320, fd+10));
	sdir->setUseableChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.,:|!?/");
	sdir->loadDeco();

	lldir = new eLabel(this);
	lldir->resize(eSize(120, fd+4));
	lldir->setText(_("LocalDir:"));

	ldir = new eTextInputField(this,lldir);
	ldir->resize(eSize(320, fd+10));
	ldir->setUseableChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.,:|!?/");
	ldir->loadDeco();
	ldir->setHelpText(_("enter name of the local mount point with trailing slash"));

	loptions = new eLabel(this);
	loptions->resize(eSize(120, fd+4));
	loptions->setText(_("Options:"));

	combo_options=new eComboBox(this, 3, loptions);
	combo_options->resize(eSize(320, fd+10));
	combo_options->loadDeco();
	combo_options->setHelpText(_("press ok to change mount options"));
	for (int i = 0; i < size_options_table; i++)
	{
		new eListBoxEntryText( *combo_options, options_table[i].szOption, (void*)i, 0);
	}

	combo_options->setCurrent(0,true);

	lextras = new eLabel(this);
	lextras->resize(eSize(120, fd+4));
	lextras->setText(_("Extra:"));

	extraoptions=new eTextInputField(this, lextras);
	extraoptions->resize(eSize(320, fd+10));
	extraoptions->setMaxChars(100);
	extraoptions->loadDeco();
	extraoptions->setHelpText(_("press ok to edit extra options"));

	luser = new eLabel(this);
	luser->resize(eSize(120, fd+4));
	luser->setText(_("User:"));

	user=new eTextInputField(this, luser);
	user->resize(eSize(320, fd+10));
	user->setMaxChars(100);
	user->loadDeco();
	user->setHelpText(_("press ok to edit username"));

	lpass = new eLabel(this);
	lpass->resize(eSize(120, fd+4));
	lpass->setText(_("Pass:"));

	pass=new eTextInputField(this, lpass);
	pass->resize(eSize(320, fd+10));
	pass->setMaxChars(100);
	pass->loadDeco();
	pass->setHelpText(_("press ok to edit password"));

	doamount=new eCheckbox(this, 0, 1);
	doamount->resize(eSize(140, fd+4));
	doamount->setText(_("Automount"));
	doamount->setHelpText(_("enable/disable automount (ok)"));

	lstatus = new eLabel(this);
	lstatus->resize(eSize(220, fd+4));
	lstatus->move(ePoint(3, clientrect.height() - (55+fd+10+fd+10) ));
	lstatus->show();

	//buttons
	prev = new eButton(this);
	prev->move(ePoint(3, clientrect.height() - (55+fd+10) ));
	prev->resize(eSize(15, fd+10));
	prev->setText("<");
	prev->setHelpText(_("go to previous share"));
	prev->loadDeco();
	CONNECT(prev->selected, eNFSSetup::prevPressed);

	umount = new eButton(this);
	umount->move(ePoint(23, clientrect.height() - (55+fd+10) ));
	umount->resize(eSize(99, fd+10));
	umount->setShortcut("red");
	umount->setShortcutPixmap("red");
	umount->loadDeco();
	umount->setHelpText(_("press ok to (un)mount this share"));
	CONNECT(umount->selected, eNFSSetup::umountPressed);

	addmp = new eButton(this);
	addmp->move(ePoint(127, clientrect.height() - (55+fd+10) ));
	addmp->resize(eSize(98, fd+10));
	addmp->setText(_("new"));
	addmp->setHelpText(_("press ok to create/cancel a new share"));
	addmp->loadDeco();
	addmp->setShortcut("green");
	addmp->setShortcutPixmap("green");
	CONNECT(addmp->selected, eNFSSetup::addPressed);

	ok = new eButton(this);
	ok->move(ePoint(230, clientrect.height() - (55+fd+10) ));
	ok->resize(eSize(98, fd+10));
	ok->setText(_("save"));
	ok->setHelpText(_("press ok to save this share"));
	ok->loadDeco();
	ok->setShortcut("yellow");
	ok->setShortcutPixmap("yellow");
	CONNECT(ok->selected, eNFSSetup::okPressed);

	del = new eButton(this);
	del->move(ePoint(333, clientrect.height() - (55+fd+10) ));
	del->resize(eSize(99, fd+10));
	del->setText(_("remove"));
	del->setHelpText(_("press ok to remove this share"));
	del->loadDeco();
	del->setShortcut("blue");
	del->setShortcutPixmap("blue");
	CONNECT(del->selected, eNFSSetup::removePressed);

	next = new eButton(this);
	next->move(ePoint(437, clientrect.height() - (55+fd+10) ));
	next->resize(eSize(15, fd+10));
	next->setText(">");
	next->loadDeco();
	next->setHelpText(_("go to next share"));
	CONNECT(next->selected, eNFSSetup::nextPressed);

	//statusbar
	sbar = new eStatusBar(this);
	sbar->move( ePoint(0, clientrect.height()-50) );
	sbar->resize( eSize( clientrect.width(), 50) );
	sbar->loadDeco();

	mountMgr = eNetworkMountMgr::getInstance();
	cur_entry = -1; // Hack: setValidEntry will add 1 to it so we start with entry 0
	// eDebug("NFSSetup] loading config for entry %d", cur_entry+1);
	setValidEntry(1);
	load_config();

}

void eNFSSetup::changeWidgets(int fstype)
{
	// eDebug("NFSSetup] change widgets");
	int y=5;

	lip->move(ePoint(10,y));
	shost->move(ePoint(120, y));
	combo_fstype->move(ePoint(340,y));
	y += 36;

	lsdir->move(ePoint(10, y));
	sdir->move(ePoint(120, y));
	y += 36;

	lldir->move(ePoint(10, y));
	ldir->move(ePoint(120, y));
	y += 36;

	luser->hide();
	user->hide();
	lpass->hide();
	pass->hide();
	loptions->hide();
	combo_options->hide();
	lextras->hide();
	extraoptions->hide();

	switch( fstype )
	{
		case 0:  // NFS
		case 1:  // CIFS
			loptions->move(ePoint(10, y));
			combo_options->move(ePoint(120,y));
			y += 36;
			lextras->move(ePoint(10, y));
			extraoptions->move(ePoint(120, y));
			y += 36;
			loptions->show();
			lextras->show();
			combo_options->show();
			extraoptions->show();
			sdir->setHelpText(_("enter the name of the share"));
			break;
		case 2:  // SMBFS
			sdir->setHelpText(_("enter the name of the share (//DreamBOX/harddisk)"));
			break;
	}
	if ( fstype )
	{
		luser->move(ePoint(10, y));
		user->move(ePoint(120, y));
		y += 36;

		lpass->move(ePoint(10, y));
		pass->move(ePoint(120, y));
		y += 36;

		luser->show();
		lpass->show();
		user->show();
		pass->show();
	}
	doamount->move(ePoint(120, y));
	invalidate();
}
    
void eNFSSetup::fstypeChanged(eListBoxEntryText *le)
{
	if (le)
		changeWidgets((int)le->getKey());
}
    
void eNFSSetup::load_config()
{
	// eDebug("NFSSetup::load_config] start");

	shost->setText(mountinfo.remoteHost);

	if (combo_fstype->setCurrent((void*)mountinfo.fstype, true) != eComboBox::OK || !mountinfo.fstype)
	{
		changeWidgets(0);
	}

	sdir->setText(mountinfo.mountDir);
	ldir->setText(mountinfo.localDir);

	if(!ldir->getText())
		ldir->setText("/media/");

	// find correct options
	// Since the webif stores with checkboxes it is possible that we do not find
	// the correct match here. webif and remote control on screen should behave the same
	// we should discuss how we want it (dropdown with choices or checkboxes) on the forum
	for (int i = 0; i < size_options_table; i++)
	{
		if (mountinfo.options == options_table[i].szOption)
		{
			combo_options->setCurrent(i, true);
		}
	}

	extraoptions->setText(mountinfo.extra_options);	
	user->setText(mountinfo.userName);
	pass->setText(mountinfo.password);
	if (cur_entry >= 0)
	{
		umount->show();
		del->show();
		ismounted = mountMgr->isMounted(cur_entry);
		addmp->setText(_("New"));
	}
	else
	{
		umount->hide();
		del->hide();
		addmp->setText(_("Cancel"));
		ismounted = false;
	}
	if (ismounted)
	{
		umount->setText(_("umount"));
		lstatus->setText(_("Status: Mounted"));
	}
	else
	{
		umount->setText(_("mount"));
		lstatus->setText(_("Status: Not Mounted"));
	}

	doamount->setCheck(mountinfo.automount);
	max_nfs_entries = mountMgr->mountPointCount();
	headline.sprintf(_("External mounts settings (%d/%d)"), cur_entry + 1, max_nfs_entries);
	setText(headline);
}
    
void eNFSSetup::clearMountinfo()
{
       	mountinfo.id = -1;
       	mountinfo.userName = "";
       	mountinfo.password = "";
       	mountinfo.localDir = "";
       	mountinfo.mountDir = "";
       	mountinfo.remoteHost = "";
       	mountinfo.remoteIP = "";
       	mountinfo.fstype = -1;
       	mountinfo.automount = 0;
       	mountinfo.options = "";
       	mountinfo.extra_options = "";
       	mountinfo.setMounted(false);
       	mountinfo.description = "";
	cur_entry = -1;
}

bool eNFSSetup::possibleEntry(int entry)
{
	// Note: as a sideeffect set mountinfo!

	mountinfo = mountMgr->getMountPointData(entry);

	return (mountinfo.fstype != 3);
}

void eNFSSetup::setValidEntry(int dir)
{
	int tot = mountMgr->mountPointCount();
	max_nfs_entries = tot;
	while (tot > 0)
	{
		cur_entry = ((cur_entry + max_nfs_entries + dir) % max_nfs_entries);
		if (possibleEntry(cur_entry))
			break;
		tot--;
	}
	if (tot <= 0)
	{
		clearMountinfo();
	}
}
    
void eNFSSetup::addPressed()
{
	if (cur_entry == -1)  // add in progress, cancel it
	{
		cur_entry = 0;
		setValidEntry(1);
	}
	else
	{
		clearMountinfo();
		setFocus(shost);
	}
	load_config();
}

void eNFSSetup::prevPressed()
{
	setValidEntry(-1);
	load_config();
	setFocus(prev);
}
    
void eNFSSetup::nextPressed()
{
	setValidEntry(1);
	load_config();
	setFocus(next);
}
    
void eNFSSetup::okPressed()
{
	if(shost->getText().length() == 0 || sdir->getText().length()==0 || ldir->getText().length()==0)
	{
		errorMessage(_("invalid or missing host or dir or local dir"));
		return;
	}
	else
	{
		saveEntry();
	}

	int tmp = (int)combo_fstype->getCurrent()->getKey();
	eString tmp1 = tmp == 2 ? "SMBFS" : tmp == 1 ? "CIFS" : "NFS";
	tmp1 += _("-Entry stored. Further entry?\n");
	eString tmp2 = "NFS/CIFS/SMBFS";
	tmp2 += _("-Setup...");
	eMessageBox msg( tmp1, tmp2,
		eMessageBox::btYes|eMessageBox::btNo, eMessageBox::btNo);
	msg.show();
	int res=msg.exec();
	msg.hide();

	if (res == eMessageBox::btYes)
	{
		nextPressed();
		setFocus(shost);
	}
	else
		load_config();
}

void eNFSSetup::removePressed()
{
	mountMgr->removeMountPoint(cur_entry);
	max_nfs_entries = mountMgr->mountPointCount();
	if (cur_entry >= max_nfs_entries)
		cur_entry = max_nfs_entries - 1;
	if (!possibleEntry(cur_entry))
		setValidEntry(1);

	load_config();
	setFocus(next);
}

void eNFSSetup::mountPressed()
{
	if(shost->getText().length() == 0 || sdir->getText().length() == 0 || ldir->getText().length() == 0)
	{
		errorMessage(_("invalid or missing host or dir or local dir"));
		return;
	}
	else 
	{
		// eDebug("[NFSSetup::mountPressed] check mounted");
		if (mountMgr->isMountPointMounted(mountinfo.localDir))
		{
			eString error = mountinfo.fstype == 2 ? "SMBFS " : mountinfo.fstype == 1 ? "CIFS " : "NFS ";
			error += _("mount error already mounted");
			error += ldir->getText();
			errorMessage(error);
			return;
		}
		else
		{
			int rc = mountMgr->mountMountPoint(cur_entry);
			eString info;
			info.sprintf("Mount was %s", rc >= 0 ? "OK" : "NOT OK");
			errorMessage(info);
		}
	}
	// eDebug("[NFSSetup::mountPressed] load config mounted");
	load_config();
}
    
void eNFSSetup::umountPressed() 
{
	if (ismounted)
	{
		eString errorstring;

		int error = mountMgr->unmountMountPoint(cur_entry);

		int tmp = (int)combo_fstype->getCurrent()->getKey();

		errorstring.sprintf("%s umount '%s' %s",
			tmp == 2 ? "SMBFS" : tmp == 1 ? "CIFS" : "NFS",
			ldir->getText().c_str(),
			error ? "FAILED!" : "OK!");

		errorMessage(errorstring.c_str());
		load_config();
	}
	else
		mountPressed();
}

int eNFSSetup::eventHandler(const eWidgetEvent &e)
{
// eDebug("[eNFSSetup] eventHandler");
	if (e.type == eWidgetEvent::execBegin )
	{
		setFocus(shost);
		return 1;
	}
#ifdef KNOW_HOW_TO_CHECK_FOCUS
        else if (e.type == eWidgetEvent::evtAction)
        {
		if (e.action == &i_NFSSetupActions->prevShare)
		{
// eDebug("[eNFSSetup] eventHandler prev share");
			prevPressed();
			return 1;
		}
		else if (e.action == &i_NFSSetupActions->nextShare)
		{
//eDebug("[eNFSSetup] eventHandler next share");
			nextPressed();
			return 1;
		}
	}
#endif

	return eWindow::eventHandler(e);
}

eNFSSetup::~eNFSSetup()
{
	delete mountContainer;
}

void eNFSSetup::saveEntry()
{
	// mountinfo = mountMgr->getMountPointData(cur_entry);
	
	mountinfo.remoteHost = shost->getText();
	mountinfo.setIP();
	mountinfo.mountDir = sdir->getText();
	mountinfo.localDir = ldir->getText();
	mountinfo.fstype = (int)combo_fstype->getCurrent()->getKey();
	mountinfo.options = options_table[(int)combo_options->getCurrent()->getKey()].szOption;
	mountinfo.extra_options = extraoptions->getText();
	mountinfo.userName = user->getText();
	mountinfo.password = pass->getText();
	mountinfo.automount = (int)doamount->isChecked();
	
	//mountMgr->changeMountPoint(mountinfo, cur_entry == -1);
	if (cur_entry == -1) // this is a new entry, so it is added at the end of the list
		cur_entry = mountMgr->mountPointCount() - 1;
}
#endif

#endif // DISABLE_NFS
#endif // DISABLE_NETWORK
