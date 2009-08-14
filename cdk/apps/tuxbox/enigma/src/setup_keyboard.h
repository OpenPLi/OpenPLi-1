#ifdef ENABLE_KEYBOARD
#ifndef __setupkeyboard_h
#define __setupkeyboard_h

#include <lib/gui/ePLiWindow.h>
#include <callablemenu.h>

class eButton;
class eLabel;
class eComboBox;

class eZapKeyboardSetup: public ePLiWindow, public eCallableMenu
{
	eComboBox* mappings;
	void okPressed();
	void loadMappings();
	void init_eZapKeyboardSetup();
public:
	eZapKeyboardSetup();
	~eZapKeyboardSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

#endif // __setupkeyboard_h

#endif // ENABLE_KEYBOARD
