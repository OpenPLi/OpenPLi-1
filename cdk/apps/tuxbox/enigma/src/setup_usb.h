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

#ifndef __setup_usb_h
#define __setup_usb_h

#include <lib/base/console.h>
#include <lib/gui/ePLiWindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/elabel.h>
#include <lib/gui/enumber.h>
#include "setup_trc.h"
#include <callablemenu.h>

class UsbSetup:public ePLiWindow, public eCallableMenu
{
private:
	TRC_Config oRc_Config;
	int iHDSleepTime;

	eLabel *lblMountWait;
	eNumber *numMountWait;
	eButton *btFormat, *btMount, *btUnmount;

	void format_usb();
	void mount_usb();
	void umount_usb();
	void mountWaitChanged();
	void okPressed();
	
public:
	UsbSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#if 0
class eMyPartitionCheck:public eWindow
{
  eLabel *lState;
  eButton *bCancel, *bClose;
  int dev;
  void onCancel ();
  void fsckClosed (int);
  int eventHandler (const eWidgetEvent & e);
  void getData (eString);
  eConsoleAppContainer *fsck;
public:
    eMyPartitionCheck (int dev);
};
#endif

#endif /* __setup_usb_h */
