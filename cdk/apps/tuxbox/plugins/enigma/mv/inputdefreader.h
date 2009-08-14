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

#ifndef __INPUTDEFREADER_H__
#define __INPUTDEFREADER_H__

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include "defs.h"
#include "util.h"
#include "files.h"
#include <enigma.h>
#define INPUTDEF_PATH                     "/var/tuxbox/config/mv/inputs.txt"
#define ENIGMA_CACHE_NAME		"EnigmaCache"
#define ENIGMA_CACHE_BACK_NAME		"SavedCache"

#define MAX_INPUT_PREFIX_LENGTH 12
#define MAX_INPUT_AFFIX_LENGTH 12
#define MAX_INPUT_FILENAME_LENGTH MAX_PATH_LENGTH-MAX_INPUT_PREFIX_LENGTH-MAX_INPUT_AFFIX_LENGTH-8

#define CACHE_INPUT_FORMAT 'c'
#define SAVED_CACHE_INPUT_FORMAT 's'
#define XMLTV_INPUT_FORMAT 'x'
#define EPGUI_INPUT_FORMAT 'e'

#define FIXED_INPUT_TYPE   'f'
#define ROTATING_INPUT_TYPE 'r'

enum {
        inputDefRemoteTypeNone = 0,
        inputDefRemoteTypeHTTP,
        inputDefRemoteTypeOTV
};

void makeInputDefFileIfNotExist( void );
int remoteType( eString spec );


struct inputDef {
	char name[MAX_INPUT_NAME_LENGTH+1];
	char format;
	char encoding;
	char type;
	int offsetToGMT;
	char localDir[MAX_INPUT_FILENAME_LENGTH+1];
	int days;
	char prefix[MAX_INPUT_PREFIX_LENGTH+1];
	char affix[MAX_INPUT_AFFIX_LENGTH+1];
	char remoteDir[MAX_INPUT_FILENAME_LENGTH+1];

	eString getFixedFilename( void ) {
		return eString(prefix) + eString(affix);
	}
	eString getFilename( eString middle ) {
		return eString(prefix) + middle + eString( affix );
	}
	eString getFixedLocalPath( void ) {
		return prefixDir( eString(localDir), getFixedFilename());
	}
	eString getLocalPath( eString middle ) {
		return prefixDir( eString(localDir), getFilename(middle));
	}
};

class InputDefReader {
	std::list<struct inputDef *> defList;
public:
	InputDefReader();
	~InputDefReader();
	struct inputDef *getDefP( char *name );
	struct inputDef *getNextDefP( bool fromStart );
	int noDefs( void );
};

#endif
