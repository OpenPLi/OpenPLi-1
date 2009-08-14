/*
	DBSwitch - Enigma Plugin

	Copyright (C) 2005 'mechatron' (mechatron@gmx.net)

	Homepage: http://mechatron.6x.to/

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#include <plugin.h>

#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/textinput.h>
#include <lib/gui/eskin.h>
#include <lib/gdi/lcd.h>
#include <lib/system/econfig.h>

#include "lcddisplay.h"

class dbswitch: public eWindow
{
	eTextInputField *code;
	eButton *con_yellow, *con_green;
	eString codeentry, color_bt;

	void start();
	void col_press();
	void status();

	eTimer rc_status;
public:
	dbswitch();
	~dbswitch();
};

class rcplay
{
	pthread_t thrPlay;
#ifndef DISABLE_LCD
	CLCDDisplay display;
#endif
	static void* PlayThread(void*);
	int rcloop(const char *code_str);
public:
	enum State {STOP = 0, PLAY};
	State state;
	static rcplay* getInstance();
	bool play(const char *str);
	rcplay();
	~rcplay();
};

