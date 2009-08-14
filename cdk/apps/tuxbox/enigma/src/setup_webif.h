#ifndef __setup_webif_h
#define __setup_webif_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/enumber.h>
#include <lib/gui/echeckbox.h>
#include <callablemenu.h>

class WebIfSetup : public ePLiWindow, public eCallableMenu
{
	public:
		WebIfSetup();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);

	private:
		eLabel *lblPort;
		eNumber *numPort;
		eCheckbox *eShowChanEPG;
		eCheckbox *eLockWebIf;
		eCheckbox *eZapStream;
		
		void okPressed();
};

#endif // __setup_webif_h
