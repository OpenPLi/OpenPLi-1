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

void MV::changeSbarValue( int *toChange, int amount )
{
	*toChange += amount;
	sBarP->hide();
	setStatusBarGeom();
	sBarP->show();
}

void MV::changeIndValue( int *toChange, int amount )
{
	*toChange += amount;
	setTimeLabelGeom();
}

#define SBAR_CHANGE_AMOUNT 4
#define IND_CHANGE_AMOUNT 2
#define PROGRAM_LIST_CHANGE_AMOUNT 4

int MV::eventHandleEditMode( int keyState, int key )
{
	if ( keyState == KEY_STATE_UP )
		return 0;

	int handled = 1;
	switch ( key ) {
		case EDIT_SWAP_FOCUS:
			if ( currentMode == MV_MODE_EDIT )
				currentMode = MV_MODE_EDIT_IND;
			else if ( currentMode == MV_MODE_EDIT_IND ) {
				views[0].feat.showStatusBar = true;
				setStatusBarGeom();
				sBarP->show();
				currentMode = MV_MODE_EDIT_SBAR;
			}
			else {
				currentMode = MV_MODE_EDIT;
			}
			break;
		case EDIT_DEC_X:
			if ( currentMode == MV_MODE_EDIT_SBAR )
				changeSbarValue( &(views[0].geom.statusBarX), -SBAR_CHANGE_AMOUNT );
			else if ( currentMode == MV_MODE_EDIT_IND )
				changeIndValue( &(views[0].geom.timeLabelX), -IND_CHANGE_AMOUNT );
			else
                        	changeTopLeftX( -10 );
			break;
		case EDIT_INC_X:  
			if ( currentMode == MV_MODE_EDIT_SBAR )
				changeSbarValue( &(views[0].geom.statusBarX), +SBAR_CHANGE_AMOUNT );
			else if ( currentMode == MV_MODE_EDIT_IND )
				changeIndValue( &(views[0].geom.timeLabelX), IND_CHANGE_AMOUNT );
			else
                        	changeTopLeftX( +10 );
			break;
		case EDIT_INC_Y: 
			if ( currentMode == MV_MODE_EDIT_SBAR )
				changeSbarValue( &(views[0].geom.statusBarY), +SBAR_CHANGE_AMOUNT );
			else if ( currentMode == MV_MODE_EDIT_IND )
				changeIndValue( &(views[0].geom.timeLabelY), IND_CHANGE_AMOUNT );
			else
                        	changeTopLeftY( +10 );
			break;
		case EDIT_DEC_Y:
			if ( currentMode == MV_MODE_EDIT_SBAR )
				changeSbarValue( &(views[0].geom.statusBarY), -SBAR_CHANGE_AMOUNT );
			else if ( currentMode == MV_MODE_EDIT_IND )
				changeIndValue( &(views[0].geom.timeLabelY), -IND_CHANGE_AMOUNT );
			else
                        	changeTopLeftY( -10 );
			break;
		case EDIT_INC_WIDTH:
			if ( currentMode == MV_MODE_EDIT_SBAR )
				changeSbarValue( &(views[0].geom.statusBarWidthPixels), +SBAR_CHANGE_AMOUNT );
			else
				changeWidthPixels( +1 );
			break;
		case EDIT_DEC_WIDTH:
			if ( currentMode == MV_MODE_EDIT_SBAR )
				changeSbarValue( &(views[0].geom.statusBarWidthPixels), -SBAR_CHANGE_AMOUNT );
			else
				changeWidthPixels( -1 );
			break;
		case EDIT_INC_HEIGHT:
			if ( currentMode == MV_MODE_EDIT_SBAR )
				changeSbarValue( &(views[0].geom.statusBarHeightPixels), +SBAR_CHANGE_AMOUNT );
			else
                        	changeHeightPixels( +1 );
			break;
		case EDIT_DEC_HEIGHT:
			if ( currentMode == MV_MODE_EDIT_SBAR )
				changeSbarValue( &(views[0].geom.statusBarHeightPixels), -SBAR_CHANGE_AMOUNT );
			else
                        	changeHeightPixels( -1 );
			break;
		case EDIT_DEC_CHANNEL_HEIGHT:
                        changeChannelHeight( -1 );
			break;
		case EDIT_INC_CHANNEL_HEIGHT:
                        changeChannelHeight( +1 );
			break;
		case EDIT_STRETCH:
                        changeWindowSeconds( +1 );
			break;
		case EDIT_SHRINK: 
                        changeWindowSeconds( -1 );
			break;
		case EDIT_SHOW_HELP:
                        showInfoBox( this, strEditHelpTitle, strEditHelp );
			break;
		case EDIT_SHOW_MENU:
			stopEdit();
                        doMenu();
			break;
		case EDIT_DEC_HEADER_WIDTH:
			changeHeaderWidth( -1 );
			break;
		case EDIT_INC_HEADER_WIDTH:
			changeHeaderWidth( +1 );
			break;
		case EDIT_FINISHED:
			stopEdit();
			break;
		case EDIT_EXIT:
			stopEdit();
			handled = inputMgrP->downloading();
			if ( handled )
				flashMessage( getStr( strDownloadInProgress ) );
			else if ( execFlag )
				close(0);
			break;
		default:
			handled = 0;
			break;
	}
	return handled;
}

void MV::stopEdit( void )
{
	currentMode = MV_MODE_VIEW;
	copyView( 0, currentView );

	rebuildAll();

	setIndicator( indEdit, false );
}

int MV::doMenu( void )
{
	if ( isGraphicalView( currentView ) )
		hideShowAll(true);

	bool doneFlag = false;

	bool needDataReloadFlag = false;

	int result;

	storeRestoreIndicators( true );

	while ( ! doneFlag ) {
		
		MainMenu menu;
		eString sysCom;
		InputDefReader defs;

		result = showExecHide( &menu );

		doneFlag = false;
       		switch ( result ) {
                case MENU_ITEM_TIPS:
				showInfoBox( NULL, strTipsTitle, strTips );
	                       	break;
                case MENU_ITEM_ABOUT:
				showInfoBox( NULL, strVersion, strAbout );
                        	break;
                case MENU_ITEM_FEEDBACK:
				showInfoBox( NULL, strFeedbackTitle, strFeedback );
                       	 	break;
               	case MENU_ITEM_ALIAS_MANAGER:
               	        	doMenuItemManageAliases();
				needDataReloadFlag = true;
				break;
               	 case MENU_ITEM_FAVOURITES_MANAGER:
               	        	doMenuItemManageFavourites();
				needDataReloadFlag = true;
				break;
               	 case MENU_ITEM_MISC:
               	        	doMenuItemMiscSettings();
				break;
               	 case MENU_ITEM_EDIT_INPUTS:
               	        	doMenuItemEditInputs();
				needDataReloadFlag = true;
				break;
               	 case MENU_ITEM_EDIT_VIEW_GEOM:
               	        	doMenuItemEditViewGeom( MV_MODE_EDIT );
				doneFlag = true;
                        	break;
               	 case MENU_ITEM_EDIT_VIEW_PRESENTATION:
                        	doMenuItemEditViewPresentation();
				break;
               	 case MENU_ITEM_EDIT_VIEW_FEATURES:
                        	doMenuItemEditViewFeatures();
				needDataReloadFlag = true;
				break;
               	 case MENU_ITEM_RESET:
				doMenuItemReset();
				needDataReloadFlag = true;
				break;
               	 default:
				doneFlag = true;
                        	break;
        	}
	}

	storeRestoreIndicators( false );

	if ( needDataReloadFlag )
		reload();

	if ( isGraphicalView( currentView ) ) {
		// Rebuild takes place with a reload
		if ( ! needDataReloadFlag )
			rebuildAll();
		hideShowAll(false);
	}

	return result;
}

void MV::doMenuItemReset( void )
{
	if ( userConfirms( strResetConfirmText ) ) {
		setConfigDefaults();
		writeConfig();
	}
}

void MV::doMenuItemManageFavourites( void )
{
	FavManager fav( formConfigPath( FAVOURITES_FILENAME ) );

	showExecHide( &fav );

	setIndicator( indUnknownChannels, inputMgrP->haveUnknownChannels() );
}

void MV::doMenuItemManageAliases( void )
{
	inputMgrP->runChannelManager();
	setIndicator( indUnknownChannels, inputMgrP->haveUnknownChannels() );
}

void MV::doMenuItemMiscSettings( void )
{
	MiscMenu menu( conf );
	
	showExecHide( &menu );
}

void MV::doMenuItemEditInputs( void )
{
	InputMenu menu( inputs );
	
	showExecHide( &menu );
}

void MV::doMenuItemEditViewFeatures( void )
{
	bool doneFlag=false;

	// Loop code from Alex Shtol
	// Modified to keep cursor position
	// when returning to it

	MainMenuFeatured mf_menu( currentView );

	while ( ! doneFlag ) {
		int iResult = showExecHide(&mf_menu);
		if ( iResult != (int) -1 ) {
			FeatureMenu fmenu( views[0].feat, currentView, iResult );
			showExecHide( &fmenu );
		}	
		else {
			doneFlag=true;
		}
	}

	stopEdit();
}

void MV::doMenuItemEditViewPresentation( void )
{
	PresentationMenu vmenu( views[0].pres, currentView );
	
	vmenu.display();
	showExecHide( &vmenu );

	stopEdit();
}

void MV::doMenuItemEditViewGeom( int mode )
{
	setIndicator( indEdit, true );
        currentMode = mode;
	copyView( currentView, 0 );
}


void MV::changeHeaderWidth( int posNeg )
{
	views[0].geom.headerWidthPixels += ( posNeg * 5 );

	hideVisibleChannels();
        rebuildChannels();
}

void MV::changeChannelHeight( int posNeg )
{
	views[0].geom.channelHeightPixels += ( 5 * posNeg );
	if ( views[0].geom.channelHeightPixels < MIN_CHANNEL_HEIGHT )
		views[0].geom.channelHeightPixels = MIN_CHANNEL_HEIGHT;	
	if ( views[0].geom.channelHeightPixels >= views[0].geom.heightPixels )
		views[0].geom.channelHeightPixels = views[0].geom.heightPixels - 1;

	hideVisibleChannels();
        rebuildChannels();
}

void MV::changeWidthPixels( int posNeg )
{
        if ( posNeg > 0 )
                views[0].geom.widthPixels += 10;
        else {
                views[0].geom.widthPixels -= 10;

        	// Get pixels left over if we don't hide/move/show
		hideShowAll(true);
	}
	
	setMainWindowGeom();
	hideShowAll(false);
}
void MV::changeHeightPixels( int posNeg )
{
        if ( posNeg > 0 )
                views[0].geom.heightPixels += 10;
        else {
                views[0].geom.heightPixels -= 10;
        	// Get pixels left over if we don't hide/move/show
		hideShowAll(true);
	}

	setMainWindowGeom();
	hideVisibleChannels();
	rebuildChannels();
	hideShowAll(false);
}

void MV::changeTopLeftY( int posNeg )
{
        if ( posNeg > 0 )
                views[0].geom.topLeftY += 10;
        else
                views[0].geom.topLeftY -= 10;

	hideShowAll(true);
	setMainWindowGeom();
	hideShowAll(false);
}

void MV::changeTopLeftX( int posNeg )
{
        if ( posNeg > 0 )
                views[0].geom.topLeftX += 10;
        else
                views[0].geom.topLeftX -= 10;

	hideShowAll(true);
	setMainWindowGeom();
	hideShowAll(false);
}
