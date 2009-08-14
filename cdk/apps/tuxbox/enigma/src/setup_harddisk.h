#ifndef DISABLE_HDD
#ifndef DISABLE_FILE

#ifndef __lib_apps_enigma_setup_harddisk_h
#define __lib_apps_enigma_setup_harddisk_h

#include <setup_window.h>
#include <lib/dvb/edvb.h>
#include <lib/gui/listbox.h>
#include <lib/gui/ePLiWindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/enumber.h>
#include <lib/base/console.h>
#include <callablemenu.h>

#include "setup_trc.h"

class eButton;
class eComboBox;
class eStatusBar;

class eHarddiskSetup: public eSetupWindow, public eCallableMenu
{
	private:
	void generalSettings();
	void specificSettings();
	void init_eHarddiskSetup();
	
	public:
	eHarddiskSetup();

	/* eCallableMenu functions */
	void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

class eGeneralHarddiskSetup: public ePLiWindow
{
	private:
	int iHDSleepTime;
	int iHDAcoustics;

	eLabel* hd_sleeptime_label;
	eNumber* hd_sleeptime;
	eLabel* hd_acoustics_label;
	eNumber* hd_acoustics;

	void okPressed();

	public:
	eGeneralHarddiskSetup();
};

class eSpecificHarddiskSetup: public eListBoxWindow<eListBoxEntryText>
{
	int nr;
	void selectedHarddisk(eListBoxEntryText *sel);
	void init_eHarddiskSetup();
public:
	eSpecificHarddiskSetup();
	int getNr() const { return nr; }
};

class eHarddiskMenu: public eWindow
{
	eButton *ext, *format, *bcheck;
	eLabel *status, *model, *capacity, *bus, *lfs;
	eComboBox *fs;
	eStatusBar *sbar;
	int dev;
	bool restartNet;
	int numpart;
	int visible;
	void s_format();
	void extPressed();
	void check();
	void readStatus();
	void init_eHarddiskMenu();
public:
	eHarddiskMenu(int dev);
	~eHarddiskMenu()
	{
		if ( restartNet )
			eDVB::getInstance()->configureNetwork();
	}
};

class ePartitionCheck: public eWindow
{
	eLabel *lState;
	eButton *bCancel, *bClose;
	int dev;
	void onCancel();
	void fsckClosed(int);
	int eventHandler( const eWidgetEvent &e );
	void getData( eString );
	eConsoleAppContainer *fsck;
	void init_ePartitionCheck();
public:
	ePartitionCheck( int dev );
};

#endif

#endif //DISABLE_FILE
#endif //DISABLE_HDD
