#ifndef __setup_swapandvar_h
#define __setup_swapandvar_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/combobox.h>
#include <callablemenu.h>

#include "setup_trc.h"

class SwapAndVarSetup : public ePLiWindow, public eCallableMenu
{
	public:
		SwapAndVarSetup();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);

	private:
		TRC_Config oRc_Config;

		enum { NO_ACTION, TO_HDD, TO_USB, TO_CF, TO_NETWORK, FROM_HDD, FROM_USB, FROM_CF, FROM_NETWORK };

		eComboBox *swap_on, *swap_size, *comVarLocation;
	
		void okPressed();
		void move_to_hdd();
		void remove_from_hdd();
		void move_to_usb();
		void remove_from_usb();
		void move_to_cf();
		void remove_from_cf();
		void move_to_network();
		void remove_from_network();
		void moveVar(
			const eString& medium,
			const eString& action,
			const eString& mountpoint);
		void addSwapToFstab(const eString& swapFile);
		void createSwapFile(const eString& swapFile, int swapSize);
};

#endif
