/*
 *   libavs.h - audio/video switch library (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2002 Gillem gillem@berlios.de
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *   $Log: libavs.h,v $
 *   Revision 1.3  2002/03/06 14:00:35  gillem
 *   - more work on avslib
 *
 *   Revision 1.2  2002/03/06 08:53:55  gillem
 *   - some fixes
 *   - add testavs
 *
 *   Revision 1.1  2002/03/04 16:10:11  gillem
 *   - initial release
 *
 *
 *
 *   $Revision: 1.3 $
 *
 */

#ifndef _LIBAVS_H_
#define _LIBAVS_H_

#define AVS_DEVICE	"/dev/dbox/avs0"
#define SAA_DEVICE	"/dev/dbox/saa0"

typedef enum ePort
{
    epVCR,
    epTV,
    epCINCH
} ePort;

typedef enum eSource
{
    esDIGITALENCODER,
    esVCR,
    esMUTE
} eSource;

typedef enum eMode
{
    emNONE,
    emCVBS,
    emRGB,
    emYC
} eMode;

typedef enum eLevel
{
	elOFF,
	el4X3,
	el16X9
} eLevel;

typedef enum eSwitch
{
	esOFF = 0,
	esON
} eSwitch;

int avsInit( int debug );
void avsDeInit( void );

int avsSetVolume( int vol );
int avsGetVolume( void );
int avsSetMute( eSwitch mute );

int avsSetLevel( eLevel level );
int avsSetRoute( ePort port, eSource source, eMode mode );

#endif
