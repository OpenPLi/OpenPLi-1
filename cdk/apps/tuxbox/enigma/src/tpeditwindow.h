#ifndef __SRC_TPEDITWINDOW_H_
#define __SRC_TPEDITWINDOW_H_

#include <lib/gui/listbox.h>

class eButton;
class eCheckbox;
class eComboBox;
class eNumber;
class eTextInputField;
class eTransponder;
class eTransponderWidget;
class tpPacket;

class eListBoxEntryTransponder: public eListBoxEntry
{
	friend class eListBox<eListBoxEntryTransponder>;
	eTransponder *tp;
	const eString &redraw(gPainter *, const eRect&, gColor, gColor, gColor, gColor, int);
	static gFont font;
public:
	bool operator < ( const eListBoxEntry& e )const;
	static int getEntryHeight();
	eListBoxEntryTransponder( eListBox<eListBoxEntryTransponder>*, eTransponder* );
	eTransponder *getTransponder() { return tp; }
};

class eTransponderEditWindow: public eWindow
{
	eListBox<eListBoxEntryText> *satellites;
	eListBox<eListBoxEntryTransponder> *transponders;
	eButton *sat, *add, *edit, *remove, *tpscan;
	int changed;
	void init_eTransponderEditWindow();
public:
	eTransponderEditWindow();
	~eTransponderEditWindow();
	void satSelChanged( eListBoxEntryText* );
	void satPressed();
	void addPressed();
	void editPressed();
	void removePressed();
	void tpscanPressed();
	void focusChanged( const eWidget* w );
	void addNetwork();
	void removeNetwork();
	int eventHandler( const eWidgetEvent & e );
};

class eTPEditDialog: public eWindow
{
	eTransponderWidget *tpWidget;
	eButton *save;
	eTransponder *tp;
	void savePressed();
	void init_eTPEditDialog();
public:
	eTPEditDialog( eTransponder *tp );
};

class eSatEditDialog: public eWindow
{
	eTextInputField *name;
	eNumber *OrbitalPos;
	eCheckbox *useBAT, *useONIT, *doNetworkSearch;
	eComboBox *direction;
	eButton *save;
	eStatusBar *sbar;
	tpPacket *tp;
	void savePressed();
	void init_eSatEditDialog();
public:
	eSatEditDialog( tpPacket *tp );
};

#endif // __SRC_TPEDITWINDOW_H_
