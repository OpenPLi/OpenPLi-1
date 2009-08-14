/******************************************************************
 * THIS FILE DOES NOT HAVE ANY CODE FOR THE DREAMBOX ANYMORE.     *
 * THE WHOLE MENU HAS BEEN REMOVED. KEEP THIS FILE FOR EASIER CVS *
 * MERGING ONLY.                                                  *
 ******************************************************************/

#ifndef __setup_extra_h
#define __setup_extra_h

#include <setup_window.h>

class eExpertSetup: public eSetupWindow
{
	void init_eExpertSetup();
#ifndef HAVE_DREAMBOX_HARDWARE
	void fileToggle(bool newState, const char* filename);
#endif
public:
	eExpertSetup();
};

#endif
