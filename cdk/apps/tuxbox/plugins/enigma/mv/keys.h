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

#ifndef __KEYS_H__
#define __KEYS_H__

#define RC_DREAM	32
#define RC_VOLUP	10
#define RC_VOLDN	11
#define RC_BOUQUP	13
#define RC_BOUQDN	14
#define RC_OK		37	
#define RC_LEFT		35
#define RC_INFO		40
#define RC_VIDEO	39
#define RC_AUDIO	38
#define RC_RIGHT	36
#define RC_UP		33
#define RC_DN		34
#define RC_TV		12
#define RC_RADIO	83
#define RC_TEXT		69
#define RC_LEFT_ARROW	81
#define RC_RIGHT_ARROW	80
#define RC_0		0
#define RC_1		1
#define RC_2		2
#define RC_3		3
#define RC_4		4
#define RC_5		5
#define RC_6		6
#define RC_7		7
#define RC_8		8
#define RC_9		9
#define RC_RED		64
#define RC_GREEN	65
#define RC_YELLOW	66
#define RC_BLUE		67
#define RC_SHIFT_RED	48
#define RC_SHIFT_GREEN	49
#define RC_SHIFT_YELLOW	50
#define RC_SHIFT_BLUE	51
#define RC_SHIFT_TV	52
#define RC_SHIFT_RADIO	53
#define RC_MUTE		68

#ifdef DM5620  	 
#define RC_LAME 84 	 
#define RC_HELP 82 	 
#else 	 
#ifdef DM500 	 
#define RC_LAME 84 	 
#define RC_HELP 82
#else
#ifdef DM500PLUS
#define RC_LAME 84 	 
#define RC_HELP 82 	 
#else 	 
// For DM70xx 	 
#define RC_LAME 82 	 
#define RC_HELP 84 	 
#endif 	 
#endif
#endif

/**** MAIN WINDOW VIEW MODE KEYS ****/

#define VIEW_BOUQUET_SELECTOR	RC_AUDIO
#define VIEW_PROG_LEFT		RC_LEFT
#define VIEW_PROG_RIGHT		RC_RIGHT
#define VIEW_INC_CHANNEL	RC_DN
#define VIEW_DEC_CHANNEL	RC_UP
#define	VIEW_SWITCH_1			RC_1
#define	VIEW_SWITCH_2			RC_2
#define	VIEW_SWITCH_3			RC_3
#define VIEW_SWITCH_4			RC_4
#define VIEW_SWITCH_5			RC_5
#define VIEW_SWITCH_6			RC_6
#define VIEW_SWITCH_7			RC_7
#define VIEW_SWITCH_8			RC_8
#define VIEW_SWITCH_9			RC_9
#define VIEW_DAY_LEFT			RC_LEFT_ARROW
#define VIEW_DAY_CENTRE			RC_0
#define VIEW_DAY_RIGHT			RC_RIGHT_ARROW
#define VIEW_PICKER_UP			RC_BOUQUP
#define VIEW_PICKER_DN			RC_BOUQDN
#define	VIEW_SHOW_MENU			RC_DREAM
#define VIEW_SELECT			RC_OK
#define VIEW_SHOW_HELP			RC_HELP
#define VIEW_SHOW_INFO			RC_INFO
#define VIEW_EXIT			RC_LAME
#define VIEW_SHOW_TIMER_LIST		RC_SHIFT_RED
#define VIEW_ADD_FAVOURITE		RC_SHIFT_GREEN
#define VIEW_SHOW_TIMER_EDIT_REC	RC_GREEN
#define VIEW_SHOW_TIMER_EDIT_NGRAB	RC_YELLOW
#define VIEW_SHOW_TIMER_EDIT_SWITCH	RC_BLUE
#define VIEW_SHOW_TIMER_EDIT_DELETE	RC_RED
#define VIEW_FILL_CACHE			RC_TEXT
#define VIEW_FILL_CACHE_PLAYING		RC_RADIO
#define VIEW_FILL_CACHE_CHANNEL		RC_TV
#define VIEW_HIDE_WINDOW		RC_VIDEO

/**** MAIN WINDOW EDIT MODE KEYS ****/

#define EDIT_DEC_X	RC_LEFT
#define EDIT_INC_X	RC_RIGHT
#define EDIT_INC_Y	RC_DN
#define EDIT_DEC_Y	RC_UP
#define EDIT_INC_WIDTH	RC_6
#define EDIT_DEC_WIDTH	RC_4
#define EDIT_DEC_CHANNEL_HEIGHT	RC_BOUQDN
#define EDIT_INC_CHANNEL_HEIGHT	RC_BOUQUP
#define EDIT_INC_HEIGHT	RC_8
#define EDIT_DEC_HEIGHT	RC_2
#define EDIT_STRETCH	RC_RIGHT_ARROW
#define EDIT_SHRINK	RC_LEFT_ARROW
#define	EDIT_SHOW_MENU	RC_DREAM
#define	EDIT_FINISHED	RC_OK
#define EDIT_SHOW_HELP	RC_HELP
#define EDIT_DEC_HEADER_WIDTH 	RC_7
#define EDIT_INC_HEADER_WIDTH 	RC_9
#define EDIT_DEC_STATUS_BAR_HEIGHT	RC_VOLDN	
#define EDIT_INC_STATUS_BAR_HEIGHT	RC_VOLUP
#define EDIT_EXIT			RC_LAME
#define EDIT_SWAP_FOCUS			RC_0

#define EXT_INFO_UP			RC_UP
#define EXT_INFO_DN			RC_DN

/**** KEY STATES ****/

#define KEY_STATE_DOWN		0
#define KEY_STATE_REPEAT	eRCKey::flagRepeat
#define KEY_STATE_UP		eRCKey::flagBreak

#endif
