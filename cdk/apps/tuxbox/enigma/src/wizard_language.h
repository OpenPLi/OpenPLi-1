#ifndef __src_wizard_language_h
#define __src_wizard_language_h

#include <lib/gui/listbox.h>

class eLanguageEntry;

class eWizardLanguage: public eWindow
{
	eListBox<eLanguageEntry> *list;
	eLabel *head, *help;
	void selchanged(eLanguageEntry *entry);
	void selected(eLanguageEntry *entry);
	int eventHandler( const eWidgetEvent &e );
	eString oldLanguage;
	void init_eWizardLanguage();
public:
	eWizardLanguage();
	static int run();
};

#endif
