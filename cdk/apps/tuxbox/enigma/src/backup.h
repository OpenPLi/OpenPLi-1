#ifndef __backup_h
#define __backup_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/listbox.h>
#include <enigma_mount.h>
#include <callablemenu.h>

class BackupScreen : public eListBoxWindow<eListBoxEntryMenu>, public eCallableMenu
{
	public:
		BackupScreen();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);
		
	private:
		void selectedBackupRestore(eListBoxEntryMenu* item);
};

class BackupScreenBackup : public ePLiWindow
{
	public:
		BackupScreenBackup();
		
	private:
		eMountSelectionComboBox* comLocation;
		void okPressed();
		void locationChanged(eListBoxEntryText *sel);
};

class BackupScreenRestore : public ePLiWindow
{
	public:
		struct RestoreDescriptions
		{
			char* description;
			char* keyword;
		};
	
		BackupScreenRestore();

	private:
		eMountSelectionComboBox* comLocation;
		eComboBox* comWhat;
		void okPressed();
		void reloadPressed();
		void locationChanged(eListBoxEntryText *sel);
};

#endif
