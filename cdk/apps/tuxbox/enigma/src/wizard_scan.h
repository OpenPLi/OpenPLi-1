#ifndef __wizard_scan_h
#define __wizard_scan_h

#include <lib/gui/listbox.h>
#include <lib/gui/combobox.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ePLiWindow.h>

class eDiseqcChoice;

class eWizardSelectDiseqc: public eWindow
{
	eListBox<eDiseqcChoice> *diseqclist;
	eLabel *description;
	void selected(eDiseqcChoice *choice);
	void selchanged(eDiseqcChoice *choice);
	void init_eWizardSelectDiseqc();
public:
	eWizardSelectDiseqc();
	static int run();
};

class eWizardSelectLNB: public ePLiWindow
{
	private:
		eComboBox* lnbType;
		void okPressed();
		
	public:
		eWizardSelectLNB();
};

#endif
