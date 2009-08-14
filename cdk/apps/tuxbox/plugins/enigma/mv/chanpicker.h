#ifndef __CHANPICKER_H__
#define __CHANPICKER_H__

#include <lib/gui/listbox.h>

#include "keys.h"
#include "text.h"
#include "util.h"

#define CP_X	60
#define CP_Y	50
#define CP_WIDTH 600
#define CP_HEIGHT 450
#define CP_NO_COLS 4

class ChannelPicker : public eListBoxWindow<eListBoxEntryText> {
	void selected( eListBoxEntryText *entryP );
	int eventHandler(const eWidgetEvent &event);
public:
	ChannelPicker();
	eListBoxEntryText *addEntry( eString &toAdd, int index );
	void setCur( eListBoxEntryText *toset);
};

#endif

