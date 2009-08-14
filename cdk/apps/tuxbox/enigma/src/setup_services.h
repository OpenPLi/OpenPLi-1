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

#ifndef __setup_services_h
#define __setup_services_h

#include <lib/gui/listbox.h>
#include <lib/base/console.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/combobox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/textinput.h>
#include <lib/gui/ePLiWindow.h>
#include <callablemenu.h>

#include "setup_trc.h"

class ServicesSetup : public ePLiWindow, public eCallableMenu
{
	public:
		ServicesSetup();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);

	private:
		eButton *pConfigureFirewall;
		eCheckbox *eStartCron, *eStartNfsServer, *eStartApache,
			*eStartGsub, *eStartInadyn, *eStartDropbear, *eEnableFirewall, *eEnableSamba;

		TRC_Config oRc_Config;

		bool fStartSambaChanged;
		bool fStartNfsServerChanged;
		bool fStartApacheChanged;
		bool fEnableFirewallChanged;
		bool fStartInadynChanged;
		bool fStartDropbearChanged;
		bool fStartCronChanged;
		bool fStartGsubChanged;

#ifndef DISABLE_NETWORK
		void EnableSambaChanged(int i);
#endif
		void StartNfsServerChanged(int i);
		void StartApacheChanged(int i);
		void StartCronChanged (int i);
		void StartGsubChanged(int i);
		void StartInadynChanged(int i);
		void StartDropbearChanged(int i);
		void EnableFirewallChanged(int i);
#ifdef SETUP_FIREWALL
		void configureFirewall();
#endif // SETUP_FIREWALL
		void okPressed();
};

#endif
