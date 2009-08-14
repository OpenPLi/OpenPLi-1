/*
 * enigma_setup.cpp
 *
 * Copyright (C) 2002 Felix Domke <tmbinc@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: setupskin.cpp,v 1.25 2009/02/07 10:06:31 dbluelle Exp $
 */

#include <setupskin.h>

#include <dirent.h>
#include <enigma.h>
#include <enigma_main.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/system/econfig.h>

#define MENUNAME N_("OSD skin")

class eSkinSetupFactory : public eCallableMenuFactory
{
public:
	eSkinSetupFactory() : eCallableMenuFactory("eSkinSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eSkinSetup;
	}
};

eSkinSetupFactory eSkinSetup_factory;

void eSkinSetup::loadSkins()
{
	eListBoxEntrySkin* selection=0;

	const char *skinPaths[] =
	{
		CONFIGDIR "/enigma/skins/",
		TUXBOXDATADIR "/enigma/skins/",
		0
	};

	char *current_skin=0;
	eConfig::getInstance()->getKey("/ezap/ui/skin", current_skin);

	std::set<eString> parsedSkins;

	for (int i=0; skinPaths[i]; ++i)
	{
		DIR *d=opendir(skinPaths[i]);
		if (!d)
		{
			if (i)
			{
				eDebug("error reading skin directory");
				eMessageBox::ShowBox("error reading skin directory", "error");
			}
			continue;
		}

		while(struct dirent *e=readdir(d))
		{
			if (i && parsedSkins.find(e->d_name) != parsedSkins.end() )
				// ignore loaded skins in var... (jffs2)
				continue;

			eString fileName=skinPaths[i];
			fileName+=e->d_name;

			if (fileName.find(".info") != eString::npos)
			{
				eSimpleConfigFile config(fileName.c_str());
				eString esml = skinPaths[i] + config.getInfo("esml");
				eString name = config.getInfo("name");
				eDebug("esml = %s, name = %s", esml.c_str(), name.c_str());
				if (esml.size() && name.size())
				{
					parsedSkins.insert(e->d_name);
					eListBoxEntrySkin *s=new eListBoxEntrySkin(lskins, name, esml);
					if (current_skin && esml == current_skin)
						selection=s;
				}
			}
		}
		closedir(d);
	}
	if ( lskins->getCount() )
		lskins->sort();
	if (selection)
		lskins->setCurrent(selection);
	if ( current_skin )
		free(current_skin);
}

void eSkinSetup::accept()
{
	skinSelected(lskins->getCurrent());
}

void eSkinSetup::skinSelected(eListBoxEntrySkin *skin)
{
	if (!skin)
		close(1);
	else
	{
		eConfig::getInstance()->setKey("/ezap/ui/skin", skin->getESML().c_str());
		eConfig::getInstance()->flush();
		close(0);
	}
}

eSkinSetup::eSkinSetup()
{
	init_eSkinSetup();
}
void eSkinSetup::init_eSkinSetup()
{
	lb3 = new eLabel(this);
	move(ePoint(50,50));
	cresize(eSize(590,376));
	setText("Skin Selector");
	baccept=new eButton(this);
	baccept->setName("accept");
	lskins=new eListBox<eListBoxEntrySkin>(this);
	lskins->setName("skins");
	lskins->setFlags(eListBoxBase::flagLostFocusOnLast);
	statusbar=new eStatusBar(this);
	statusbar->setName("statusbar");

	CONNECT(baccept->selected, eSkinSetup::accept);
	CONNECT(lskins->selected, eSkinSetup::skinSelected);
	
	setFocus(lskins);

	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "setup.skins"))
		eFatal("skin load of \"setup.skins\" failed");

	loadSkins();

	/* help text for skin setup */
	setHelpText(_("\tChange Skin\n\n>>> [MENU] >>> [6] Setup >>> [3] System Settings\n>>> [6] OSD Settings >>> [BLUE] Change Skin\n. . . . . . . . . .\n\n" \
								"Your Dreambox supports custom made skins. PLi offers additional skin downloads and previews from the download menu automating skin installation.\n" \
								"Note: Changing a skin requires a reboot.\n. . . . . . . . . .\n\nUsage:\n\n[UP]/[DOWN]\tSelect Inputfield, Button, or skin\n\n" \
								"[GREEN]/[OK]\tSave Settings and Close Window\n\n[EXIT]\tClose window without saving changes"));
}

eSkinSetup::~eSkinSetup()
{
}

int eSkinSetup::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
		case eWidgetEvent::evtAction:
			if (event.action == &i_cursorActions->left)
				focusNext(eWidget::focusDirW);
			else if (event.action == &i_cursorActions->right)
				focusNext(eWidget::focusDirE);
			else
				break;
			return 1;
		default:
			break;
	}
	return eWindow::eventHandler(event);
}

void eSkinSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	int skinRes = exec();
	hide();
	if(!skinRes)
	{
		eMessageBox msg(_("You have to restart enigma to apply the new skin\nRestart now?"), _("Skin changed"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion, eMessageBox::btYes );
		msg.show();
		int ret = msg.exec();
		msg.hide();
		if(ret == eMessageBox::btYes)
		{
			if (eZapMain::getInstance()->checkRecordState())
			{
				eZap::getInstance()->quit(2);
			}
		}
		msg.hide();
	}
}
