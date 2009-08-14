#ifndef __echeckbox_h
#define __echeckbox_h

#include <lib/gui/ebutton.h>

class eCheckbox: public eButton
{
protected:
	int ischecked;
private:
	void sel();
	int eventHandler(const eWidgetEvent &event);
	int setProperty(const eString &prop, const eString &value);
	void gotFocus();
	void lostFocus();
	bool swapTxtPixmap;
	void init_eCheckbox(int checked);
public:
	Signal1<void, int> checked;
	eCheckbox(eWidget *parent, int checked=0, int takefocus=1, bool swapTxtPixmap=false, const char *deco="eCheckBox" );
	~eCheckbox();
	void setCheck(int c);
	int isChecked() { return ischecked; }
};

#endif
