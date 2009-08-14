/***************************************************************/
/*
    * Copyright (C) 2004 Lee Wilmot <lee@dinglisch.net>

    This file is part of MV.

    MV is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    MV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You can find a copy of the GNU General Public License in the
    file gpl.txt distributed with this file.
*/
/*************************************************************/

#include "channelmanager.h"
#include <enigma.h>
#include <sstream>


#define COLUMN_WIDTH        ((clientrect.width()-10) / 3)

#define CM_TO_BOX_X      	180
#define CM_BUTTON_HEIGHT 	30
#define CM_BUTTON_WIDTH 	( clientrect.width() - CM_TO_BOX_X - 20) / 4 

#define CM_RED_BUTTON_X		CM_TO_BOX_X
#define CM_GREEN_BUTTON_X	CM_RED_BUTTON_X + CM_BUTTON_WIDTH + 3
#define CM_YELLOW_BUTTON_X	CM_GREEN_BUTTON_X + CM_BUTTON_WIDTH + 3
#define CM_BLUE_BUTTON_X	CM_YELLOW_BUTTON_X + CM_BUTTON_WIDTH + 3
#define CM_BUTTON_ROW_Y  	clientrect.height() - CM_BUTTON_HEIGHT - 10

ChannelManager::ChannelManager(FileMap& map) :
eWindow(1),
showFlags(CM_SHOW_UNKNOWN),
fromBoxP( 0 ),
toBoxP( 0),
toLabelP(0),
lbCurrentMode(0),
numberOfChanIdKeys(0),
fileMap(map)
{
    loadServices();
    setWidgetGeom( this, 55, 50, 615, 500 );
    setText( "Channels" );

    // RED, UNKNOWN
    eButton *bt_unknown = makeNewButton(
                            this, "Unknown",
                            CM_RED_BUTTON_X, CM_BUTTON_ROW_Y,
                            CM_BUTTON_WIDTH, CM_BUTTON_HEIGHT,
                            "red");

    // GREEN, DONE
    eButton *bt_ok = makeNewButton(
                            this, "Done",
                            CM_GREEN_BUTTON_X, CM_BUTTON_ROW_Y,
                            CM_BUTTON_WIDTH, CM_BUTTON_HEIGHT,
                            "green");

    // YELLOW, IGNORED
    eButton *bt_ignore = makeNewButton(
                            this, "Ignored",
                            CM_YELLOW_BUTTON_X, CM_BUTTON_ROW_Y,
                            CM_BUTTON_WIDTH, CM_BUTTON_HEIGHT,
                            "yellow");
    // BLUE, MAPPED
    eButton *bt_mapped = makeNewButton(
                            this, "Mapped",
                            CM_BLUE_BUTTON_X, CM_BUTTON_ROW_Y,
                            CM_BUTTON_WIDTH, CM_BUTTON_HEIGHT,
                            "blue");

    toLabelP = makeNewLabel(this, "UNSET",
                            5, CM_BUTTON_ROW_Y + 4,
                            100, font.pointSize);


    int xx = clientrect.width()/2;
    lbCurrentMode = makeNewLabel(this, "Display mode", 5, 5, xx -10, font.pointSize);
    fromBoxP = makeNewListbox(this, 5, 5 + 26 + 5, xx -10, clientrect.height()-CM_BUTTON_HEIGHT-10 -(5+5+font.pointSize));
    CONNECT( fromBoxP->selchanged, ChannelManager::fromSelectionChanged );
    CONNECT( fromBoxP->selected, ChannelManager::fromSelectedItem );

    toBoxP = makeNewListbox(this, xx, 5, clientrect.width()-xx-10, clientrect.height()-40);
    new eListBoxEntryText(toBoxP, name, (void*)0);
    CONNECT( toBoxP->selected, ChannelManager::toSelectedItem );

    build();

    CONNECT(bt_unknown->selected, ChannelManager::unknownButtonHandle );
    CONNECT(bt_ok->selected,      ChannelManager::doneButtonHandle );
    CONNECT(bt_ignore->selected,  ChannelManager::ignoreButtonHandle);
    CONNECT(bt_mapped->selected,  ChannelManager::mappedButtonHandle );
}


void ChannelManager::fromSelectedItem( eListBoxEntryText *item )
{
    if ( item )
		setFocus( toBoxP );
}

void ChannelManager::fromSelectionChanged( eListBoxEntryText *item )
{
    if ( item )
    {
        eString chanId = localChanIdMap[(int) item->getKey()];
        if (fileMap.isMapped(chanId))
        {
            eString ref = fileMap.getMappedChannel(chanId);
            toLabelP->setText( getMappedName(ref) );
        }
        else
        {
            toLabelP->setText( "Not mapped");
        }
    }
}

const eString ChannelManager::getMappedName(const eString& ref)
{
    return getServiceName(mapKey2Ref[ref]);
}

void ChannelManager::build()
{
	showFlags = CM_SHOW_UNKNOWN;
	rebuild(); 
}

void ChannelManager::rebuild( void )
{
    if (lbCurrentMode)
    {
        switch (showFlags)
        {
            case CM_SHOW_UNKNOWN:
                lbCurrentMode->setText("Unknown channels");
                break;
            case CM_SHOW_IGNORE:
                lbCurrentMode->setText("Ignored channels");
                break;
            case CM_SHOW_MAPPED:
                lbCurrentMode->setText("Mapped channels");
                break;
        }
    }
    rebuildFromBox();
    fromBoxP->show();

    rebuildToBox();
    toBoxP->show();

    setFocus( fromBoxP );
}

struct fillFromBox
{
    ChannelManager &m;
    fillFromBox(ChannelManager& man) : m(man){}
    void operator()(const eString& chanId, const eString& name)
    {
        m.newFromListBoxEntry(name, chanId);
    }
};


bool ChannelManager::showEntry(const eString& chanId)
{
    switch (showFlags)
    {
        case CM_SHOW_UNKNOWN:
            return !(fileMap.isMapped(chanId) || fileMap.isIgnored(chanId));
        case CM_SHOW_IGNORE:
            return (fileMap.isIgnored(chanId));
        case CM_SHOW_MAPPED:
            return (fileMap.isMapped(chanId));
    }
    std::cout << "Why am I here???" << std::endl;
    return true;
}

void ChannelManager::newFromListBoxEntry(const eString& name, const eString& chanId)
{
    //check if we need to show this one (is it mapped and showmode == mapped?) etc
    //not sure what happens with objects, references, pointers to references, so
    // I make a local list of integer keys to refer to the actual channel ids
    localChanIdMap[numberOfChanIdKeys] = chanId;
    if (showEntry(chanId))
    {
        new eListBoxEntryText(fromBoxP, name, (void*) numberOfChanIdKeys );
    }

    ++numberOfChanIdKeys;
}
void ChannelManager::rebuildFromBox( void )
{
    numberOfChanIdKeys = 0;
    localChanIdMap.clear();

    fromBoxP->hide();
    fromBoxP->clearList();
    // Add items
    fileMap.forEachChanAbc(fillFromBox(*this));
}

void ChannelManager::rebuildToBox( void )
{
    toBoxP->hide();
    toBoxP->clearList();    // I really hope that this also deletes its child entries....

    new eListBoxEntryText(toBoxP, "IGNORE", (void*) 0 );
    new eListBoxEntryText(toBoxP, "UNKNOWN", (void*) 1 );
	new eListBoxEntrySeparator( (eListBox<eListBoxEntry>*)toBoxP, getNamedPixmapP( "listbox.separator" ), 0, true );

	int entryNo = 3;
	// Fill box from bouquet
    serviceRefs.clear();
	std::list<eServiceReferenceDVB>::iterator curService = serviceList.begin();
    while ( curService != serviceList.end() )
    {
        eString name = getServiceName( *curService );
        new eListBoxEntryText(toBoxP, name, (void*) entryNo );
        serviceRefs[entryNo] = *curService;

        curService++;
        entryNo++;
    }
}

void ChannelManager::toSelectedItem( eListBoxEntryText *item )
{
    if ( item )
    {
        eListBoxEntryText *fromCurrentItemP = fromBoxP->getCurrent();
        int localId = (int) fromCurrentItemP->getKey();
        eString fromKey = localChanIdMap[localId];
        eString toText = item->getText();
        int toKey = (int) item->getKey();
        //handle toKey < 3
        if (toText == "IGNORE")
        {
            std::cout << "Adding ignore " << fromKey << std::endl;
            fileMap.addIgnoredChannel(fromKey);
        }
        else if (toText == "UNKNOWN")
        {
            std::cout << "Adding unknown" << fromKey << std::endl;
            fileMap.addUnknownChannel(fromKey);
        }
        else if (toKey >= 3)
        {
            eServiceReferenceDVB ref = serviceRefs[toKey];
            std::cout << "Adding map" << fromKey << ", " << ref << ", " << toText << std::endl;
            fileMap.addMappedChannel(fromKey, getRtServiceKey(ref));
        }

        if (!showEntry(fromKey))
        {
            //get next entry in list, and give focus
            //avoid a jumping back to first if we deleted last entry
            if (fromBoxP->getNext() != fromBoxP->getFirst())
            {
                fromBoxP->moveSelection(eListBoxBase::dirDown);
            }
            fromBoxP->remove(fromCurrentItemP, true);   //hold current position
        }
        setFocus( fromBoxP );
    }
}

void ChannelManager::doneButtonHandle(void)
{
    fileMap.writeMapFile();
    eWidget::accept();
}
void ChannelManager::mappedButtonHandle( void )
{
    showFlags = CM_SHOW_MAPPED;
    rebuild();
}

void ChannelManager::ignoreButtonHandle( void )
{
    showFlags = CM_SHOW_IGNORE;
    rebuild();
}

void ChannelManager::unknownButtonHandle( void )
{
    showFlags = CM_SHOW_UNKNOWN;
    rebuild();
}


void ChannelManager::loadServices()
{
    serviceList.clear();
    Signal1<void,const eServiceReference&> callback;
    CONNECT( callback, ChannelManager::addToList );
                // bool is 'from beginning'
    eZap::getInstance()->getServiceSelector()->forEachServiceRef( callback,  true );
}

void ChannelManager::addToList( const eServiceReference& ref )
{
    if ((ref.type == eServiceReference::idDVB)      &&
        ( !(ref.flags & eServiceReference::isMarker)) )
    {
        eString refString = getRtServiceKey((const eServiceReferenceDVB&) ref);
        mapKey2Ref[refString] = (const eServiceReferenceDVB&) ref;
        serviceList.push_back( (const eServiceReferenceDVB&) ref);
   }
}

eString ChannelManager::getRtServiceKey(const eServiceReferenceDVB& ref)
{
    std::stringstream ss;
    ss << std::hex << ref.getServiceID().get() << ":" << ref.getOriginalNetworkID().get() << ":" << ref.getTransportStreamID().get();

    return ss.str();
}
