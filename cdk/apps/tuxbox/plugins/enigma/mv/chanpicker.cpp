#include "chanpicker.h"

ChannelPicker::ChannelPicker() : eListBoxWindow<eListBoxEntryText>()
{ 
	setText( eString( getStr( strChannelPickerWindowTitle ) ) );
	setWidgetGeom( this, CP_X, CP_Y, CP_WIDTH, CP_HEIGHT );

	setWidgetGeom( &list, 0, 0, clientrect.width(), clientrect.height() );
	list.setColumns( CP_NO_COLS );
	CONNECT( list.selected, ChannelPicker::selected );
}

void ChannelPicker::selected( eListBoxEntryText *entryP )
{
	if ( entryP )
		close( (int) (entryP->getKey()) );
}

eListBoxEntryText *ChannelPicker::addEntry( eString &toAdd, int index )
{
	return new eListBoxEntryText( &list, toAdd.c_str(), (void *)index, 16  );
}

void ChannelPicker::setCur( eListBoxEntryText *toSet )
{
	list.setCurrent( toSet );
}

int ChannelPicker::eventHandler(const eWidgetEvent &event)
{
	int handled = 0;
        switch (event.type) {
                case eWidgetEvent::evtKey:
		{
			if ( (event.key)->flags == KEY_STATE_UP )
				return 0;
			switch ( (event.key)->code ) {
				case VIEW_PICKER_UP:
					list.moveSelection( eListBoxBase::dirUp );
					break;
				case VIEW_PICKER_DN:
					list.moveSelection( eListBoxBase::dirDown );
					break;
			}
		}
		default:
			break;
	}

	return handled ? 1 :eListBoxWindow<eListBoxEntryText>::eventHandler(event);
}
