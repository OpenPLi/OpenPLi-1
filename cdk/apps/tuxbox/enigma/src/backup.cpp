/*
 *  PLi backup/restore screen
 *
 *  Copyright (C) 2008 dAF2000, David@daf2000.nl
 *  Copyright (C) 2008 PLi team, www.pli-images.org
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

#include <backup.h>
#include <parentallock.h>
#include <lib/gui/ExecuteOutput.h>

#define MENUNAME N_("Backup/restore")

class BackupScreenFactory : public eCallableMenuFactory
{
public:
	BackupScreenFactory() : eCallableMenuFactory("BackupScreen", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new BackupScreen;
	}
};

BackupScreenFactory BackupScreen_factory;

BackupScreen::BackupScreen() :
	eListBoxWindow<eListBoxEntryMenu>(_(MENUNAME), 2, 300, true)
{
	valign();
	
	new eListBoxEntryCheck(&list, _("Backup Dreambox"), 
		_("Make a backup of your Dreambox"), 0, (void*)0);
	new eListBoxEntryCheck(&list, _("Restore Dreambox"), 
		_("Restore a backup to your Dreambox"), 0, (void*)1);

	CONNECT(list.selected, BackupScreen::selectedBackupRestore);
}

void BackupScreen::selectedBackupRestore(eListBoxEntryMenu* item)
{
	if(item)
	{
		if((int)item->getKey() == 0)
		{
			BackupScreenBackup screen;
			screen.show();
			screen.exec();
			screen.hide();
		}
		else
		{
			BackupScreenRestore screen;
			screen.show();
			screen.exec();
			screen.hide();
		}
	}
}

void BackupScreen::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	if(pinCheck::getInstance()->checkPin(pinCheck::setup))
	{
		setLCD(lcdTitle, lcdElement);
		show();
		exec();
		hide();
	}
}

BackupScreenBackup::BackupScreenBackup() :
	ePLiWindow(_("Backup"), 450)
{
	eLabel* lblLocation = new eLabel(this);
	lblLocation->move(ePoint(10, yPos()));
	lblLocation->resize(eSize(200, widgetHeight()));
	lblLocation->loadDeco();
	lblLocation->setText(_("Backup device:"));

	comLocation = new eMountSelectionComboBox(
		this, 4, lblLocation, 
		eMountSelectionComboBox::ShowDevices | 
		eMountSelectionComboBox::ShowNetwork |
		eMountSelectionComboBox::ShowCustomLocation);

	comLocation->move(ePoint(190, yPos()));
	comLocation->resize(eSize(250, widgetHeight()));
	comLocation->setHelpText(_("Select a backup device or mount"));
	comLocation->setCurrentLocation("/media/hdd");
	CONNECT(comLocation->selchanged, BackupScreenBackup::locationChanged);

	buildWindow();
	
	bOK->setText(_("Backup"));
	bOK->setHelpText(_("Make a backup now"));
	bOK->resize(eSize(210, 40));
	bCancel->move(ePoint(230, yPos()));
	bCancel->resize(eSize(210, 40));
	bCancel->setHelpText(_("Exit this screen"));
	CONNECT(bOK->selected, BackupScreenBackup::okPressed);
}

void BackupScreenBackup::okPressed()
{
	eString command = "backup.sh backup " + comLocation->getCurrentLocation();
	
	ExecuteOutput backup(
		_("Making backup, please wait"),
		command);
	backup.show();
	backup.exec();
	backup.hide();
}

void BackupScreenBackup::locationChanged(eListBoxEntryText *sel)
{
	comLocation->locationChanged(sel);
}

const static struct BackupScreenRestore::RestoreDescriptions whatToRestore[] =
{
	{N_("All"), "all"},
	{N_("Addons/plugins"), "addons"},
	{N_("Complete config"), "config"},
	{N_("Settings"), "settings"},
	{N_("Enigma"), "enigma"},
	{N_("Softcams"), "cams"},
	{N_("Samba"), "samba"},
	{N_("Bin"), "bin"},
	{N_("Etc"), "etc"},
	{N_("Library"), "lib"},
	{N_("Share"), "share"},
	{N_("Spool"), "spool"},
	{N_("Keys"), "keys"}
};

BackupScreenRestore::BackupScreenRestore() :
	ePLiWindow(_("Restore"), 450)
{
	eLabel* lblLocation = new eLabel(this);
	lblLocation->move(ePoint(10, yPos()));
	lblLocation->resize(eSize(200, widgetHeight()));
	lblLocation->loadDeco();
	lblLocation->setText(_("Restore device:"));

	comLocation = new eMountSelectionComboBox(
		this, 4, lblLocation, 
		eMountSelectionComboBox::ShowDevices | 
		eMountSelectionComboBox::ShowNetwork |
		eMountSelectionComboBox::ShowCustomLocation);

	comLocation->move(ePoint(190, yPos()));
	comLocation->resize(eSize(250, widgetHeight()));
	comLocation->setHelpText(_("Select the restore device or mount"));
	comLocation->setCurrentLocation("/media/hdd");
	CONNECT(comLocation->selchanged, BackupScreenRestore::locationChanged);

	nextYPos(35);

	eLabel* lblWhat = new eLabel(this);
	lblWhat->move(ePoint(10, yPos()));
	lblWhat->resize(eSize(200, widgetHeight()));
	lblWhat->loadDeco();
	lblWhat->setText(_("What to restore:"));

	comWhat = new eComboBox(this, 4, lblWhat);
	comWhat->move(ePoint(190, yPos()));
	comWhat->resize(eSize(250, widgetHeight()));
	comWhat->setHelpText(_("Select what you want to restore"));
	comWhat->loadDeco();

	for(unsigned int i=0; i<sizeof(whatToRestore) / sizeof(whatToRestore[0]); ++i)
	{
		new eListBoxEntryText(*comWhat, _(whatToRestore[i].description), (void*)whatToRestore[i].keyword);
	}
	comWhat->setCurrent(0);

	buildOKButton();

	eButton* btReload = new eButton(this);
	btReload->move(ePoint(140, yPos()));
	btReload->resize(eSize(170, 40));
	btReload->setShortcut("blue");
	btReload->setShortcutPixmap("blue");
	btReload->setText(_("Reload Enigma"));
	btReload->setHelpText(_("Reload Enigma settings now"));
	btReload->loadDeco();
	CONNECT(btReload->selected, BackupScreenRestore::reloadPressed);
	
	buildWindow();
	
	bOK->setText(_("Restore"));
	bOK->setHelpText(_("Restore Dreambox now"));
	bCancel->setHelpText(_("Exit this screen"));
	CONNECT(bOK->selected, BackupScreenRestore::okPressed);
}

void BackupScreenRestore::okPressed()
{
	eString command = "backup.sh " +
		eString((char*)comWhat->getCurrent()->getKey()) + " " +
		comLocation->getCurrentLocation();
	
	ExecuteOutput restore(
		_("Restoring, please wait"),
		command);
	restore.show();
	restore.exec();
	restore.hide();
}

void BackupScreenRestore::reloadPressed()
{
	ExecuteOutput reload(
		_("Reloading Enigma settings, please wait"),
		"backup.sh reloadenigma db");
	reload.show();
	reload.exec();
	reload.hide();
}

void BackupScreenRestore::locationChanged(eListBoxEntryText *sel)
{
	comLocation->locationChanged(sel);
}

