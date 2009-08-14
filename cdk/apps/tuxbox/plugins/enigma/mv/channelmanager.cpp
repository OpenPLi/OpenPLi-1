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

#define CM_SHOW_UNKNOWN  1
#define CM_SHOW_IGNORE	 2
#define CM_SHOW_MAPPED   4

#define CM_TO_BOX_X      	180
#define CM_BUTTON_HEIGHT 	30
#define CM_BUTTON_WIDTH 	( clientrect.width() - CM_TO_BOX_X - 20) / 4 

#define CM_RED_BUTTON_X		CM_TO_BOX_X
#define CM_GREEN_BUTTON_X	CM_RED_BUTTON_X + CM_BUTTON_WIDTH + 3
#define CM_YELLOW_BUTTON_X	CM_GREEN_BUTTON_X + CM_BUTTON_WIDTH + 3
#define CM_BLUE_BUTTON_X	CM_YELLOW_BUTTON_X + CM_BUTTON_WIDTH + 3
#define CM_BUTTON_ROW_Y  	clientrect.height() - CM_BUTTON_HEIGHT - 10

ChannelManager::ChannelManager( eString path ) : 
	FileMap( path )
#ifndef MVLITE
	,eWindow(1), fromBoxP( NULL ), toBoxP( NULL )
#endif
{
#ifndef MVLITE
	setWidgetGeom( this, 55, 50, 615, 500 );
	setText( getStr( strAliasManagerTitle ) );
	
	gFont font = getNamedFont( "epg.time" );
	font.pointSize = 16;
	setFont( font );

	// RED, UNKNOWN
	eButton *bt_unknown = makeNewButton(
                this, strButtonUnknown,
		CM_RED_BUTTON_X, CM_BUTTON_ROW_Y,
		CM_BUTTON_WIDTH, CM_BUTTON_HEIGHT,
                "red"
        );

	// GREEN, DONE
	eButton *bt_ok = makeNewButton(
                this, strButtonDone,
		CM_GREEN_BUTTON_X, CM_BUTTON_ROW_Y,
		CM_BUTTON_WIDTH, CM_BUTTON_HEIGHT,
                "green"
        );

	// YELLOW, IGNORE
	eButton *bt_ignore = makeNewButton(
                this, strButtonIgnore,
		CM_YELLOW_BUTTON_X, CM_BUTTON_ROW_Y,
		CM_BUTTON_WIDTH, CM_BUTTON_HEIGHT,
                "yellow"
        );
	// BLUE, MAPPED
	eButton *bt_mapped = makeNewButton(
                this, strButtonMapped,
		CM_BLUE_BUTTON_X, CM_BUTTON_ROW_Y,
		CM_BUTTON_WIDTH, CM_BUTTON_HEIGHT,
                "blue"
        );

	toLabelP = makeNewLabel( 
		this, "UNSET", 
		5, CM_BUTTON_ROW_Y + 4,
		100, font.pointSize
	);

	CONNECT(bt_unknown->selected, ChannelManager::unknownButtonHandle );
	CONNECT(bt_ok->selected, eWidget::accept);
	CONNECT(bt_ignore->selected, ChannelManager::ignoreButtonHandle);
	CONNECT(bt_mapped->selected, ChannelManager::mappedButtonHandle );
#endif
}

// ------------------- THESE ARE THE NON-LITE WINDOW FUNCTIONS ------------- //

#ifndef MVLITE

void ChannelManager::fromSelectedItem( eListBoxEntryText *item )
{
        if ( item )
		setFocus( toBoxP );
}

void ChannelManager::fromSelectionChanged( eListBoxEntryText *item )
{
        if ( item ) {
		eString selString = item->getText();
		toLabelP->setText( getMappedName( selString ) );
	}
}
void ChannelManager::build( std::list<eServiceReferenceDVB> *services )
{
	serviceList = services;
	showFlags = 0;
	rebuild(); 
}

void ChannelManager::rebuild( void )
{
	rebuildFromBox();
	fromBoxP->show();

	rebuildToBox();
	toBoxP->show();

	setFocus( fromBoxP );
}


void ChannelManager::rebuildFromBox( void )
{
	if ( fromBoxP != NULL )
		fromBoxP->hide();

	fromBoxP =  makeNewListbox(
                this,
                5,5,
		CM_TO_BOX_X - 10, clientrect.height() - CM_BUTTON_HEIGHT - 20
        );


	// Fill box from bouquet

	int entryNo = 0;
	std::list<eServiceReferenceDVB>::iterator curService = serviceList->begin();

        while ( curService != serviceList->end() ) {
                eString name = getServiceName( *curService );
                eString mapped = getMappedName( name );
		if(
		    (showFlags & CM_SHOW_UNKNOWN)
		    || mapped.length()==0
		  )
		{
		new eListBoxEntryText(fromBoxP, name, (void*) entryNo );
		if(entryNo==0)
		  toLabelP->setText( mapped);
		entryNo++;
		}
                curService++;
        }

	CONNECT( fromBoxP->selchanged, ChannelManager::fromSelectionChanged );
	CONNECT( fromBoxP->selected, ChannelManager::fromSelectedItem );
}

void ChannelManager::rebuildToBox( void )
{
	if ( toBoxP != NULL )
		toBoxP->hide();

	toBoxP =  makeNewListbox(
                this,
                CM_TO_BOX_X,5,
                clientrect.width() - CM_TO_BOX_X - 10, clientrect.height() - 40
        );
	toBoxP->setColumns( 3 );

	// Add items

	int itemCount = 0;
	std::set<eString>::iterator it = inputs.begin();
        while ( it != inputs.end() ) {
		new eListBoxEntryText(toBoxP, *it, (void*) itemCount );
		itemCount++;		
		it++;
	}

	
	// Select currently mapped item
	
	CONNECT( toBoxP->selected, ChannelManager::toSelectedItem );
}

void ChannelManager::toSelectedItem( eListBoxEntryText *item )
{
        if ( item ) {
		eString newToName = item->getText();
		eListBoxEntryText *fromCurrentItemP = fromBoxP->getCurrent();
		eString fromName = fromCurrentItemP->getText();

		eString oldToName = getMappedName( fromName );
//mylog(eString().sprintf("from=%s,to=%s\n",fromName.c_str(),newToName.c_str()));
		addMappedName( newToName, fromName, true );

		if ( ! matchesCurrentShowFlags( newToName ) ) {
			fromBoxP->remove( fromCurrentItemP );
			setFocus( fromBoxP );
		}
        }
}

void ChannelManager::ignoreButtonHandle( void )
{
	showFlags = CM_SHOW_IGNORE;
	rebuild();
}

void ChannelManager::mappedButtonHandle( void )
{
	showFlags = CM_SHOW_MAPPED;
	rebuild();
}

void ChannelManager::unknownButtonHandle( void )
{
	showFlags ^= CM_SHOW_UNKNOWN;
	rebuild();
}

bool ChannelManager::matchesCurrentShowFlags( eString name )
{
	if ( name == eString( "UNKNOWN" ) )
		return ( showFlags & CM_SHOW_UNKNOWN );
	else if ( name == eString( "IGNORE" ) )
		return ( showFlags & CM_SHOW_IGNORE );
	else if ( name.length() > 0 )
                return ( showFlags & CM_SHOW_MAPPED );
	else
		return false;
}

#endif

// ------------------- THESE ARE THE NON- WINDOW FUNCTIONS ------------- //

// Returns IGNORE if to ignore, "" for unknown or the mapped name
//
void ChannelManager::addInput( eString originalName )
{
	inputs.insert(originalName);
}
