#include <callablemenu.h>

std::map<eString, struct eCallableMenuFactory::MenuEntry> eCallableMenuFactory::menus;

eCallableMenuFactory::eCallableMenuFactory(const eString& shortcutName, const eString& menuName)
	: shortcutName(shortcutName)
{
		/*
		* we shouldn't actually use our this pointer in the constructor...
		* but since all factories are created globally, we should be
		* sure that nobody uses our this pointer till we've been fully constructed
		*
		* and for the same reason, we don't need a mutex round the menu list,
		* nobody will use the list till it has been completely filled
		*/

	struct MenuEntry menuEntry(this, menuName);
	eCallableMenuFactory::menus[shortcutName] = menuEntry;
}

eCallableMenuFactory::~eCallableMenuFactory()
{
	struct MenuEntry menuEntry(NULL, "");
	eCallableMenuFactory::menus[shortcutName] = menuEntry;
}

int eCallableMenuFactory::showMenu(
	const eString& shortcutName,
	eWidget* lcdTitle,
	eWidget* lcdElement)
{
	eCallableMenuFactory* factory = eCallableMenuFactory::menus[shortcutName].factory;
	if(factory && factory->isAvailable())
	{
		eCallableMenu* menu = factory->createMenu();
		if(menu)
		{
			menu->doMenu(lcdTitle, lcdElement);
			delete menu;
			return 0;
		}
	}
	return -1;
}

bool eCallableMenuFactory::isAvailable()
{
	return true;
}

void eCallableMenuFactory::getMenuEntries(std::map<eString, struct MenuEntry>& menuEntries)
{
	for(std::map<eString, struct MenuEntry>::iterator i = eCallableMenuFactory::menus.begin();
		i != eCallableMenuFactory::menus.end(); ++i)
	{
		eCallableMenuFactory* factory = i->second.factory;
		if(factory && factory->isAvailable())
		{
			menuEntries[factory->shortcutName] = eCallableMenuFactory::menus[factory->shortcutName];
		}
	}
}

struct eCallableMenuFactory::MenuEntry& eCallableMenuFactory::getMenuEntry(const eString& shortcutName)
{
	return eCallableMenuFactory::menus[shortcutName];
}
