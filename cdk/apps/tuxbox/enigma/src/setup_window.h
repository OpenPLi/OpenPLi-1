#ifndef __setup_window_h
#define __setup_window_h

#include <lib/gui/listbox.h>

class eSetupWindow:public eListBoxWindow<eListBoxEntryMenu>
{
	void sel_num(int n);
protected:
	int eventHandler(const eWidgetEvent &event);
public:
	eListBox<eListBoxEntryMenu>* getList() { return &list; }
	static Signal2<void,eSetupWindow*,int*> setupHook;
	eSetupWindow( const char *titlemm, int entries, int width );
	void addCallableMenuEntry(
		const eString& shortcutName,
		const eString& helptext = "",
		int* entry = NULL);
};

#endif // __setup_window_h
