#ifndef __setup_teletext_h
#define __setup_teletext_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/echeckbox.h>
#include <callablemenu.h>

class TeletextSetup : public ePLiWindow, public eCallableMenu
{
	public:
		TeletextSetup();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
		
	private:
		eCheckbox *chk_caching;
		void okPressed();
};

#endif
