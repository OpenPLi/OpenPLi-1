#ifndef __setup_mounts_h
#define __setup_mounts_h

#ifndef DISABLE_NETWORK
#ifndef DISABLE_NFS

#include <lib/gui/ewindow.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/elabel.h>
#include <lib/gui/enumber.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/textinput.h>
#include <lib/gui/combobox.h>
#include <lib/gui/eskin.h>
#include <enigma_mount.h>
#include <lib/gui/listbox.h>
#include <lib/gui/ePLiWindow.h>
#include <callablemenu.h>

class eMountSetupWrapper : public eCallableMenu
{
	public:
		eMountSetupWrapper() {};

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
};

class eMountSetup : public eListBoxWindow<eListBoxEntryMenu>
{
	public:
		eMountSetup(int nrOfMounts);
		
	private:
		void selectedMount(eListBoxEntryMenu* item);
		
		eNetworkMountMgr* networkMountMgr;
};

class eMountDetails : public ePLiWindow
{
	public:
		eMountDetails(int mountIndex);
		
	private:
		void hostIPChanged(eListBoxEntryText *sel);
		void deletePressed();
		eString saveMount();
		void savePressed();
		void mountPressed();
		void optionsPressed();
		void mountpointChanged(eListBoxEntryText *sel);
		
		eNetworkMountMgr* networkMountMgr;
		int currentMountId;
		eMountPoint currentMount;
		eTextInputField* tiDesc;
		eComboBox* cbFS;
		eCheckbox* chAutoMount;
		eComboBox* cbIP;
		eTextInputField* tiHost;
		eNumber* nuIP;
		eMountSelectionComboBox* cbMountPoint;
		eTextInputField* tiServerDir;
		eTextInputField* tiUser;
		eTextInputField* tiPass;
};

class eMountOptions : public ePLiWindow
{
	public:
		eMountOptions(eMountPoint* currentMount);
		
	private:
		void okPressed();

		eMountPoint* currentMount;
		eCheckbox* chOption[11];
		eCheckbox* chLinuxExt;
		eComboBox* cbRsize;
		eComboBox* cbWsize;
		eTextInputField* tiOptions;
};

/*** All below this line is the old mount manager ***/

#if 0
class eConsoleAppContainer;

class eNFSSetup: public eWindow
{
	eTimer timeout;
	eButton *ok, *del, *addmp, *umount, *prev, *next;
	eCheckbox *doamount;
	eComboBox *combo_fstype, *combo_options;
	eLabel *lpass , *luser, *loptions, *lextras, *lip, *lldir, *lsdir, *lstatus;
	eStatusBar *sbar;
	eString cmd,headline;
	eTextInputField *shost, *sdir, *ldir, *user, *pass, *extraoptions;
	eMountPoint mountinfo;
	eNetworkMountMgr *mountMgr;
	int cur_entry, max_nfs_entries;
	bool ismounted;
	eConsoleAppContainer *mountContainer;
     
	void fieldSelected(int *number) { focusNext(eWidget::focusDirNext); }
	void fstypeChanged(eListBoxEntryText *le);
	void changeWidgets(int fstype);
    
	void load_config();

	int eventHandler(const eWidgetEvent &e);
	void okPressed();
	void removePressed();
	void prevPressed();
	void nextPressed();
	void addPressed();
	void umountPressed();
	void mountPressed();
	void saveEntry();
	bool possibleEntry(int entry);
	void clearMountinfo();
	void setValidEntry(int dir);

public:
	eNFSSetup();
	~eNFSSetup();
    
};
#endif

#endif // DISABLE_NFS
#endif // DISABLE_NETWORK
#endif // __setup_mounts_h
