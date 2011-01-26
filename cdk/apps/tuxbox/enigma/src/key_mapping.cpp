/*
 *  PLi extension to Enigma: key mapping
 *
 *  Copyright (C) 2008 dAF2000 <David@daf2000.nl>
 *  Copyright (C) 2008 PLi team <http://www.pli-images.org>
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

#include <key_mapping.h>
#include <enigma_plugins.h>

#define MENUNAME N_("User-defined buttons")

class KeyMappingFactory : public eCallableMenuFactory
{
public:
	KeyMappingFactory() : eCallableMenuFactory("KeyMapping", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new KeyMappingPreselection;
	}
};

KeyMappingFactory KeyMapping_factory;

KeyMappingPreselection::KeyMappingPreselection()
	:eListBoxWindow<eListBoxEntryMenu>(_(MENUNAME), 3, 300, true)
{
	init_KeyMappingPreselection();
}
void KeyMappingPreselection::init_KeyMappingPreselection()
{
	valign();

	new eListBoxEntryMenu(&list, _("Assign green button"), _("Assign the green button to a menu, function or plugin"), 
		0, (void*)"/pli/keyMapping/greenButton");	
	new eListBoxEntryMenu(&list, _("Assign yellow button"), _("Assign the yellow button to a menu, function or plugin"), 
		0, (void*)"/pli/keyMapping/yellowButton");
	new eListBoxEntryMenu(&list, _("Assign blue button"), _("Assign the blue button to a menu, function or plugin"), 
		0, (void*)"/pli/keyMapping/blueButton");
	
	CONNECT(list.selected, KeyMappingPreselection::selectedButton);
}

void KeyMappingPreselection::selectedButton(eListBoxEntryMenu *item)
{
	if(item)
	{
		KeyMapping keymapping(eString((char*)item->getKey()));
		keymapping.setLCD(LCDTitle, LCDElement);
		keymapping.show();
		keymapping.exec();
		keymapping.hide();
	}
}

void KeyMappingPreselection::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

KeyMapping::KeyMapping(const eString& buttonKey)
	: ePLiWindow(_(MENUNAME), 500), buttonKey(buttonKey)
{
	init_KeyMapping(buttonKey);
}
void KeyMapping::init_KeyMapping(const eString& buttonKey)
{
	eLabel* lblKey = new eLabel(this);
	lblKey->setText(_("Assign button as:"));
	lblKey->move(ePoint(10, yPos()));
	lblKey->resize(eSize(480, widgetHeight()));
	lblKey->loadDeco();

	nextYPos(35);
	comAction = new eComboBox(this, 7, lblKey);
	comAction->move(ePoint(10, yPos()));
	comAction->resize(eSize(480, widgetHeight()));
	comAction->setHelpText(_("Select a menu, function or plugin"));
	comAction->loadDeco();
	
	new eListBoxEntryText(*comAction, _("Audio selection"), (void*)AUDIOSELECTION);
	new eListBoxEntryText(*comAction, _("Timeshift pause"), (void*)TIMESHIFTPAUSE);
	new eListBoxEntryText(*comAction, _("Plugin screen"), (void*)PLUGINSCREEN);
	new eListBoxEntryText(*comAction, _("SubServices"), (void*)SUBSERVICES);
	new eListBoxEntryText(*comAction, _("Teletext"), (void*)TELETEXT);
	new eListBoxEntryText(*comAction, _("Plugin"), (void*)PLUGIN);
	new eListBoxEntryText(*comAction, _("Menu"), (void*)MENU);

	int actionIndex = 0;
	eString enigmaKey = "AudioSelection";
	if(buttonKey.find("greenButton") != eString::npos)
		enigmaKey = "SubServices";
	eConfig::getInstance()->getKey(buttonKey.c_str(), enigmaKey);
	
	if(enigmaKey == "AudioSelection")
	{
		actionIndex = AUDIOSELECTION;
	}
	else if(enigmaKey == "TimeshiftPause")
	{
		actionIndex = TIMESHIFTPAUSE;
	}
	else if(enigmaKey == "PluginScreen")
	{
		actionIndex = PLUGINSCREEN;
	}
	else if(enigmaKey == "SubServices")
	{
		actionIndex = SUBSERVICES;
	}
	else if(enigmaKey == "Teletext")
	{
		actionIndex = TELETEXT;
	}
	else if(enigmaKey.left(7) == "Plugin:")
	{
		actionIndex = PLUGIN;
	}
	else if(enigmaKey.left(5) == "Menu:")
	{
		actionIndex = MENU;
	}
	
	comAction->setCurrent((void*)actionIndex);
	CONNECT(comAction->selchanged, KeyMapping::typeChanged);

	nextYPos(35);	
	comMenus = new eComboBox(this, 5, 0);
	comMenus->move(ePoint(10, yPos()));
	comMenus->resize(eSize(480, widgetHeight()));
	comMenus->setHelpText(_("Select a menu"));
	comMenus->loadDeco();
	
	std::map<eString, struct eCallableMenuFactory::MenuEntry> menus;
	eCallableMenuFactory::getMenuEntries(menus);
	
	void* menuIndex = 0;
	
	for(std::map<eString, struct eCallableMenuFactory::MenuEntry>::iterator i = menus.begin();
		i != menus.end(); ++i)
	{
		new eListBoxEntryText(*comMenus, _(i->second.menuName.c_str()), (void*)i->first.c_str(), 0, "", eListBoxEntryText::ptr);
		
		if(actionIndex == MENU)
		{
			if(enigmaKey.mid(5) == i->first)
			{
				menuIndex = (void*)i->first.c_str();
			}
		}
	}
	
	comMenus->sort();
	if(menuIndex == 0)
	{
		// Choose first entry by default
		comMenus->setCurrent(0);
	}
	else
	{
		comMenus->setCurrent(menuIndex);
	}
	
	comPlugins = new eComboBox(this, 5, 0);
	comPlugins->move(ePoint(10, yPos()));
	comPlugins->resize(eSize(480, widgetHeight()));
	comPlugins->setHelpText(_("Select a plugin"));
	comPlugins->loadDeco();
	
	std::vector<eString> pluginNames;
	eZapPlugins::listPlugins(eZapPlugins::StandardPlugin, &pluginFilenames, &pluginNames);

	void* pluginIndex = 0;

	for(unsigned int i = 0; i < pluginFilenames.size(); ++i)
	{
		new eListBoxEntryText(*comPlugins, pluginNames[i], (void*)pluginFilenames[i].c_str(), 0, "", eListBoxEntryText::ptr);
		
		if(actionIndex == PLUGIN)
		{
			if(enigmaKey.mid(7) == pluginFilenames[i])
			{
				pluginIndex = (void*)pluginFilenames[i].c_str();
			}
		}
	}
	
	comPlugins->sort();
	if(pluginIndex == 0)
	{
		// Choose first entry by default
		comPlugins->setCurrent(0);
	}
	else
	{
		comPlugins->setCurrent(pluginIndex);
	}

	buildWindow();
	CONNECT(bOK->selected, KeyMapping::okPressed);
	
	typeChanged(comAction->getCurrent());
}

void KeyMapping::okPressed()
{
	eString enigmaKey;
	int actionType = (int)comAction->getCurrent()->getKey();
	
	switch(actionType)
	{
		case AUDIOSELECTION:
			enigmaKey = "AudioSelection";
			break;
			
		case TIMESHIFTPAUSE:
			enigmaKey = "TimeshiftPause";
			break;
			
		case PLUGINSCREEN:
			enigmaKey = "PluginScreen";
			break;

		case SUBSERVICES:
			enigmaKey = "SubServices";
			break;
			
		case TELETEXT:
			enigmaKey = "Teletext";
			break;
			
		case PLUGIN:
			enigmaKey = "Plugin:" + eString((char*)comPlugins->getCurrent()->getKey());
			break;
			
		case MENU:
			enigmaKey = "Menu:" + eString((char*)comMenus->getCurrent()->getKey());
			break;
	}
	
	eConfig::getInstance()->setKey(buttonKey.c_str(), enigmaKey);
	accept();
}

void KeyMapping::typeChanged(eListBoxEntryText *sel)
{
	if(sel)
	{
		int executeType = (int)sel->getKey();
		
		switch(executeType)
		{
			case AUDIOSELECTION:
			case TIMESHIFTPAUSE:
			case PLUGINSCREEN:
			case SUBSERVICES:
			case TELETEXT:
				comPlugins->hide();
				comMenus->hide();
				break;
				
			case PLUGIN:
				comPlugins->show();
				comMenus->hide();
				setFocus(comPlugins);
				break;
				
			case MENU:
				comPlugins->hide();
				comMenus->show();
				setFocus(comMenus);
		}
	}
}

eString KeyMapping::getShortButtonDescription(const eString& buttonKey)
{
	eString enigmaKey = "AudioSelection";
	if(buttonKey.find("greenButton") != eString::npos)
		enigmaKey = "SubServices";
	eConfig::getInstance()->getKey(buttonKey.c_str(), enigmaKey);
	
	if(enigmaKey == "AudioSelection")
	{
		return _("Aud");
	}
	else if(enigmaKey == "TimeshiftPause")
	{
		return _("Pau");
	}
	else if(enigmaKey == "PluginScreen")
	{
		return _("Plug");
	}
	else if(enigmaKey == "SubServices")
    {
        return _("Sub");
    }    
	else if(enigmaKey == "Teletext")
	{
		return _("Txt");
	}
	else if(enigmaKey.left(7) == "Plugin:")
	{
		return _("Plug");
	}
	else if(enigmaKey.left(5) == "Menu:")
	{
		return _("Menu");
	}
	
	return "???";
}
