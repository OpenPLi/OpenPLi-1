/*
 * PLi's setup plugin for dreambox
 * Copyright (c) 2005 PLi <peter@dreamvcr.com>
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

#ifdef SETUP_FIREWALL

#include "setup_firewall.h"

//-----------------------------------------------------------------------------

FirewallSetup::FirewallSetup() : eWindow(0), pOk(0), pLabel(0), pStatusbar(0)
{
	setText (dgettext("plisetup", "Configure firewall"));
	cmove (ePoint (130, 140));
	cresize (eSize (450, 290));

	int fd = eSkin::getActive ()->queryValue ("fontsize", 16);


	for (int i = 0; i < iMaxIp; i++) {
		strHostNames[i] = "";
		for (int j = 0; j < 4; j++) {
			aiFirewallIp[i][j] = 0;
		}
	}

	ReadUserfile();

	pLabel = new eLabel (this);
	pLabel->setText ("IP address or hostname");
	pLabel->move (ePoint (10, 10));
	pLabel->resize (eSize (150, fd + 4));
	int iXpoint = 10;
	int iYpoint = 10;
	for (int i = 0; i < iMaxIp; i++) {
		if (i == 5) {
			iXpoint += 225;
			iYpoint = 10;
		}
		iYpoint += 30;
		pIpFields[i] = new eNumber(this, 4, 0, 255, 3, aiFirewallIp[i], 0, pLabel);
		pIpFields[i]->move(ePoint (iXpoint, iYpoint));
		pIpFields[i]->resize(eSize (200, fd + 10));
		pIpFields[i]->setFlags(eNumber::flagDrawPoints);
		pIpFields[i]->setHelpText(dgettext("plisetup", "Enter IP address to allow access"));
		pIpFields[i]->loadDeco();
		if (aiFirewallIp[i][0] == 0) {
			pIpFields[i]->hide();
		}

      pHostFields[i] = new eTextInputField(this);
      pHostFields[i]->move(ePoint (iXpoint, iYpoint));
      pHostFields[i]->resize(eSize (200, fd + 10));
      pHostFields[i]->setHelpText(dgettext("plisetup", "Enter hostname to allow access"));
      pHostFields[i]->setUseableChars ("01234567890abcdefghijklmnopqrstuvwxyz._-");
      pHostFields[i]->setMaxChars(64);
      pHostFields[i]->loadDeco();
      pHostFields[i]->setText(strHostNames[i]);
      if (strHostNames[i] == "") {
			pHostFields[i]->hide();
		}
	}

	pOk = new eButton(this);
	pOk->setShortcut("green");
	pOk->setShortcutPixmap("green");
	pOk->loadDeco();
	pOk->setText(dgettext("plisetup", "Save"));
	pOk->move(ePoint (10, 190));
	pOk->resize (eSize (170, 40));
	pOk->setHelpText(dgettext("plisetup", "save changes and restart firewall"));
	CONNECT (pOk->selected, FirewallSetup::okPressed);


	pStatusbar = new eStatusBar(this);
	pStatusbar->move(ePoint(0, clientrect.height () - 40));
	pStatusbar->resize(eSize(clientrect.width (), 50));
	pStatusbar->loadDeco();
}

//-----------------------------------------------------------------------------

FirewallSetup::~FirewallSetup()
{
	if (pStatusbar) delete pStatusbar;
	if (pOk) delete pOk;
	for (int i = 0; i < iMaxIp; i++) {
		if (pIpFields[i]) delete pIpFields[i];
		if (pHostFields[i]) delete pHostFields[i];
	}
	if (pLabel) delete pLabel;
}
//-----------------------------------------------------------------------------

void FirewallSetup::ReadUserfile()
{
	// try to read /var/etc/firewall.users and store the found data in the correct variables
	char line[256];
	FILE* fUserfile = fopen("/var/etc/firewall.users", "r");

	if (fUserfile) {
		int iIpIdx = 0;
		while (fgets (line, 256, fUserfile) != NULL && iIpIdx < iMaxIp) {
			// check if line is an IP adress or hostname
			if (sscanf(line, "%d.%d.%d.%d", &aiFirewallIp[iIpIdx][0], &aiFirewallIp[iIpIdx][1], &aiFirewallIp[iIpIdx][2], &aiFirewallIp[iIpIdx][3]) != 4) {
				// not an IP address
				aiFirewallIp[iIpIdx][0] = 0;
				aiFirewallIp[iIpIdx][1] = 0;
				aiFirewallIp[iIpIdx][2] = 0;
				aiFirewallIp[iIpIdx][3] = 0;
				strHostNames[iIpIdx] = line;
			} else {
				strHostNames[iIpIdx] = "";
			}
			iIpIdx++;
		}
		fclose(fUserfile);
	}
}
//-----------------------------------------------------------------------------

void FirewallSetup::WriteUserfile()
{
	// try to write a new /var/etc/firewall.users
	FILE* fUserfile = fopen("/var/etc/firewall.users", "w");

	if (fUserfile) {
		eString strLineOut;
		for (int iIpIdx = 0; iIpIdx < iMaxIp; iIpIdx++) {
			strLineOut = "";
			if (pHostFields[iIpIdx]->getText() != "") {
				strLineOut = pHostFields[iIpIdx]->getText();
			} else {
				if (pIpFields[iIpIdx]->getNumber(0) != 0) {
          		strLineOut = eString().sprintf ("%d.%d.%d.%d", pIpFields[iIpIdx]->getNumber(0), pIpFields[iIpIdx]->getNumber(1), pIpFields[iIpIdx]->getNumber(2), pIpFields[iIpIdx]->getNumber(3));
				}
			}

			if (strLineOut.length() != 0) {
				fprintf(fUserfile, "%s\n", strLineOut.c_str());
			}
		}
		fclose(fUserfile);
	}

}
//-----------------------------------------------------------------------------

void FirewallSetup::okPressed()
{
#ifdef DEBUG
	printf ("okPressed\n");
#endif
	WriteUserfile();
	close(0);
}
//-----------------------------------------------------------------------------


#endif
