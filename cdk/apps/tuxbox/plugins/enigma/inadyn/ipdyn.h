/* Licence and copyright: see ipdyn.cpp */

#ifndef _INADYN_H
#define _INADYN_H

#include <plugin.h>
#include <stdio.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>
#include <lib/gui/textinput.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/eskin.h>
#include <lib/gui/combobox.h>
#include <lib/gui/statusbar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "fstream"
#include "iostream"

#define CONFIG "/var/etc/inadyn.config"

using namespace std;

class eInaDyn: public eWindow
{
	eButton *bt_abort, *bt_ok;
	eLabel *lpass , *luser, *ldomain, *mode_label, *l_update;
	eComboBox *mode, *cb_update;
	eStatusBar *sStatusbar;
	eTextInputField *user, *pass,*domain;
	int dd;
	char *UU;
	char *PP;
	char *AA;
	char *updatePeriodStr;

	void selectedItem(eListBoxEntryText *item);
	void selectionChanged(eListBoxEntryText *item);
	void abortPressed();
	void ok();
	int loadData();
	int counter;
	void domainChanged(eListBoxEntryText *item);
	int getDomain();
public:
	eInaDyn();
	~eInaDyn();
};

#endif /* _INADYN_H */
