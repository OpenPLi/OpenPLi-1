#ifndef __enigma_info_h
#define __enigma_info_h

#include <lib/gui/ewidget.h>
#include <lib/gui/listbox.h>

class eZapInfo: public eListBoxWindow<eListBoxEntryMenu>
{
private:
	void sel_streaminfo();
	void sel_about();	
	void sel_aboutPLi();	
public:
	eZapInfo();
	~eZapInfo();
};

#endif /* __enigma_info_h */
