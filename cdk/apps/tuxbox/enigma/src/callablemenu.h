#ifndef _callable_menu_h
#define _callable_menu_h

#include <map>
#include <lib/base/estring.h>
#include <lib/gui/ewidget.h>

// #define N_("text") to mark window titles as translatable for
// xgettext without calling dgettext.
#define N_(text) (text)

class eCallableMenu
{
public:
	eCallableMenu() {}
	virtual ~eCallableMenu() {}

	virtual void doMenu(eWidget* lcdTitle, eWidget* lcdElement) = 0;
};

class eCallableMenuFactory
{
	public:
		struct MenuEntry
		{
			eCallableMenuFactory* factory;
			eString menuName;
		
			MenuEntry(eCallableMenuFactory* factory, const eString& menuName)
				: factory(factory), menuName(menuName) {}
			MenuEntry() : factory(0), menuName("") {}
		};
	
		eCallableMenuFactory(const eString& shortcutName, const eString& menuName);
		virtual ~eCallableMenuFactory();

		static int showMenu(
			const eString& shortcutName,
			eWidget* lcdTitle,
			eWidget* lcdElement);
		
		static void getMenuEntries(std::map<eString, struct MenuEntry>& menuEntries);
		static struct MenuEntry& getMenuEntry(const eString& shortcutName);

		virtual eCallableMenu* createMenu() = 0;
		virtual bool isAvailable();
			
		eString shortcutName;

	private:
		static std::map<eString, struct MenuEntry> menus;
};

#endif
