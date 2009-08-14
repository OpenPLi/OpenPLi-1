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

#include "setup_usb.h"

#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>
#include <enigma_plugins.h>
#include <plugin.h>

#define MENUNAME N_("USB stick")

class UsbSetupFactory : public eCallableMenuFactory
{
public:
	UsbSetupFactory() : eCallableMenuFactory("UsbSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new UsbSetup;
	}

	bool isAvailable()
	{
		int UsbAvailable(0);
		int MountedOnUSB(0);

		eConfig::getInstance()->getKey("/pli/UsbAvailable", UsbAvailable);
		eConfig::getInstance()->getKey("/pli/MountedOnUSB", MountedOnUSB);
		
		return (MountedOnUSB == 0) && (UsbAvailable != 0);
	}
};

UsbSetupFactory UsbSetup_factory;

int usb_mounted()
{
	int mounted = 0;
	int ret = system("mount | grep -c /media/usb");
	if ((ret >> 8) == 0)
	{
		mounted = 1;
	}
	
  return mounted;
}

UsbSetup::UsbSetup():
	ePLiWindow(_(MENUNAME), 350),
	lblMountWait(0), numMountWait(0), btFormat(0), btMount(0), btUnmount(0)
{
	oRc_Config.ReadConfig();

	lblMountWait = new eLabel(this);
	lblMountWait->setText(_("USB mount wait"));
	lblMountWait->move(ePoint(10, yPos()));
	lblMountWait->resize(eSize(250, widgetHeight()));

	int iSleep = oRc_Config.getSleep();
	numMountWait = new eNumber(this, 1, 0, 9, 2, &iSleep, 0, lblMountWait);
	numMountWait->move(ePoint(290, yPos()));
	numMountWait->resize(eSize (50, widgetHeight()));
	numMountWait->setHelpText(_("Number of seconds to wait before mounting USB stick"));
	numMountWait->loadDeco();
	CONNECT(numMountWait->numberChanged, UsbSetup::mountWaitChanged);

	nextYPos(35);
	btMount = new eButton(this);
	btMount->setText(_("Mount USB stick"));
	btMount->move(ePoint(10, yPos()));
	btMount->resize(eSize(330, 40));
	btMount->setHelpText(_("Mount USB stick"));
	btMount->loadDeco();
	CONNECT(btMount->selected, UsbSetup::mount_usb);
	
	btUnmount = new eButton(this);
	btUnmount->setText(_("Unmount USB stick"));
	btUnmount->move(ePoint(10, yPos()));
	btUnmount->resize(eSize(330, 40));
	btUnmount->setHelpText(_("Unmount USB stick"));
	btUnmount->loadDeco();
	CONNECT(btUnmount->selected, UsbSetup::umount_usb);
	
	nextYPos(50);
	btFormat = new eButton(this);
	btFormat->setText(_("Format USB stick"));
	btFormat->move(ePoint(10, yPos()));
	btFormat->resize(eSize(330, 40));
	btFormat->setHelpText(_("Format USB stick, this will remove ALL data on the stick"));
	btFormat->loadDeco();
	CONNECT(btFormat->selected, UsbSetup::format_usb);

	if(usb_mounted())
	{
		btMount->hide();
	}
	else
	{
		btUnmount->hide();
	}
	
	buildWindow();
	CONNECT(bOK->selected, UsbSetup::okPressed);
	
	// Don't show save and cancel button until mount wait time has been changed
	bOK->hide();
	bCancel->hide();
}

void UsbSetup::mountWaitChanged()
{
	bOK->show();
	bCancel->show();
	setFocus(bOK);
}

void UsbSetup::okPressed()
{
	if(numMountWait)
	{
		oRc_Config.setSleep(numMountWait->getNumber());
		oRc_Config.WriteConfig();
	}
	
	close(0);
}

void UsbSetup::format_usb ()
{
  do
    {
      {
        eMessageBox msg (_("Are you SURE that you want to format this USB disk?\n"),
                         _("formatting USB disk..."), eMessageBox::btYes | eMessageBox::btCancel, eMessageBox::btCancel);
        msg.show ();
        int res = msg.exec ();
        msg.hide ();
        if (res != eMessageBox::btYes)
          break;
      }

      system (eString ().sprintf ("/bin/umount /media/usb").c_str ());

      if (usb_mounted ())
        {
          eMessageBox::ShowBox(_("sorry, Cannot format a mounted USB device."), _("formatting USB disk..."), eMessageBox::btOK | eMessageBox::iconError);
          break;
        }

      eMessageBox msg (_("Please wait while formatting the USB disk.\nThis might take some minutes.\n"), _("formatting USB disk..."), 0);
      msg.show ();

      FILE *f = popen (eString ().sprintf ("/sbin/sfdisk -f /dev/scsi/host0/bus0/target0/lun0/disc").c_str (), "w");
      if (!f)
        {
          eMessageBox::ShowBox(_("sorry, couldn't find sfdisk utility to partition USB disk."),
                           _("formatting USB disk..."), eMessageBox::btOK | eMessageBox::iconError);
          break;
        }
      fprintf (f, "0,\n;\n;\n;\ny\n");
      fclose (f);
      {
        ::sync ();
        if (system (eString ().sprintf ("/sbin/mkfs.ext3 -b 4096 /dev/scsi/host0/bus0/target0/lun0/part1").c_str ()) >> 8)
          goto err;
        ::sync ();
        if (system (eString ().sprintf ("/bin/mount_usb.sh").c_str ()) >> 8)

          goto err;
        ::sync ();
        goto noerr;
      }
    err:
      {
        eMessageBox::ShowBox(_("creating filesystem failed."), _("formatting USB disk..."), eMessageBox::btOK | eMessageBox::iconError);
        break;
      }
    noerr:
      {
        eMessageBox::ShowBox(_("successfully formatted your USB disk!"), _("formatting USB disk..."), eMessageBox::btOK | eMessageBox::iconInfo);
      }
    }
  while (0);
}

void UsbSetup::mount_usb ()
{
	eMessageBox *msg;
	system ("/bin/mount_usb.sh");
	if (usb_mounted ())
	{
		msg = new eMessageBox (
			_("USB Stick successfully mounted"),
			_("USB Stick successfully mounted"),
			eMessageBox::btOK | eMessageBox::iconInfo);
	}
	else
	{
		msg = new eMessageBox (
			_("USB Stick NOT MOUNTED, unknown error"),
			_("USB Stick NOT MOUNTED, unknown error"),
			eMessageBox::btOK | eMessageBox::iconError);
	}

	msg->show ();
	msg->exec ();
	msg->hide ();

	if (usb_mounted())
	{
		btMount->hide();
		btUnmount->show();
	}
	else
	{
		btMount->show();
		btUnmount->hide();
	}
}

void UsbSetup::umount_usb ()
{
	eMessageBox *msg;
	system ("umount /media/usb");
	if (usb_mounted ())
	{
		msg = new eMessageBox(
			_("USB Stick IS STILL MOUNTED, unknown error"),
			_("USB Stick IS STILL MOUNTED, unknown error"),
			eMessageBox::btOK | eMessageBox::iconError);
	}
	else
	{
		msg = new eMessageBox (
			_("USB Stick successfully un-mounted"),
			_("USB Stick successfully un-mounted"),
			eMessageBox::btOK | eMessageBox::iconInfo);
	}

	msg->show ();
	msg->exec ();
	msg->hide ();

	if (usb_mounted())
	{
		btMount->hide();
		btUnmount->show();
	}
	else
	{
		btMount->show();
		btUnmount->hide();
	}
}

void UsbSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

#if 0
void
eMyPartitionCheck::getData (eString str)
{
  str.removeChars ('\x8');
  if (str.find ("<y>") != eString::npos)
    fsck->write ("y", 1);
  else if (str.find ("[N/Yes]") != eString::npos)
    fsck->write ("Yes", 3);

  lState->setText (str);
}
#endif
