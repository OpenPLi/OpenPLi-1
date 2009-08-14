#ifndef __key_mapping_h
#define __key_mapping_h

#include <vector>
#include <lib/gui/listbox.h>
#include <lib/gui/combobox.h>
#include <lib/gui/ePLiWindow.h>
#include <callablemenu.h>

class KeyMappingPreselection : public eListBoxWindow<eListBoxEntryMenu>, public eCallableMenu
{
	public:
		KeyMappingPreselection();
		
		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);

	private:
		void selectedButton(eListBoxEntryMenu* item);
};

class KeyMapping : public ePLiWindow
{
	public:
		KeyMapping(const eString& buttonKey);
		static eString getShortButtonDescription(const eString& buttonKey);
	
	private:
		enum { AUDIOSELECTION = 0, TIMESHIFTPAUSE, PLUGINSCREEN, TELETEXT, PLUGIN, MENU };
	
		void okPressed();
		void typeChanged(eListBoxEntryText *sel);
		
		eString buttonKey;
		eComboBox* comAction;
		eComboBox* comPlugins;
		eComboBox* comMenus;
		std::vector<eString> pluginFilenames;
};

#endif
