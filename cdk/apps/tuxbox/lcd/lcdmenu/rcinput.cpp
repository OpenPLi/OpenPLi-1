/*
 * $Id: rcinput.cpp,v 1.3.2.1 2003/05/24 07:46:45 alexw Exp $
 * 
 * Remote Control Handling Class
 *
 * Copyright (C) 2001, 2002 Andreas Oberritter <obi@saftware.de>
 *
 * based on rcinput.cpp from Neutrino which is
 *
 * Copyright (C) 2001 Steffen Hehn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Log: rcinput.cpp,v $
 * Revision 1.3.2.1  2003/05/24 07:46:45  alexw
 * update
 *
 * Revision 1.3  2002/01/03 17:18:59  obi
 * some reorganization.
 * removed buffer classes.
 * removed threading from rcinput.
 * moved timer from rc-class to menu-class.
 * manually add fonts to fontRendererClass.
 * fixed small bugs that noone would ever find and added new ones.
 *
 *
 */

#include "rcinput.h"

/**************************************************************************
 *	Constructor - opens rc-input device and starts threads
 **************************************************************************/
CRCInput::CRCInput()
{
	fd=open(RC_DEVICE, O_RDONLY);
	if (fd<0)
	{
		perror(RC_DEVICE);
		exit(-1);
	}
	ioctl(fd, RC_IOCTL_BCODES, 1);
	prevrccode = 0xffff;

    tv_prev.tv_sec = 0;
    repeat_block = 150000; // 150ms
}

/**************************************************************************
 *	Destructor - close the input-device
 **************************************************************************/
CRCInput::~CRCInput()
{
	if (fd>=0)
		close(fd);
}

/**************************************************************************
 *	get rc-key from the rcdevice
 **************************************************************************/
int CRCInput::getKey()
{
	struct timeval tv;
	long long td;
	__u16 rccode;
	bool repeat = true;
	int erg = RC_nokey;
	while (repeat)
	{
		if (read(fd, &rccode, 2)!=2)
		{
			printf("key: empty\n");
			//return -1; error!!!
		}
		else
		{
			gettimeofday( &tv, NULL );

			td = ( tv.tv_usec - tv_prev.tv_usec );
			td+= ( tv.tv_sec - tv_prev.tv_sec )* 1000000;

			if ( ( ( prevrccode&0x1F ) != ( rccode&0x1F ) ) || ( td > repeat_block ) )
			{
				tv_prev = tv;
				//printf("got key native key: %04x %d\n", rccode, tv.tv_sec );

				if( prevrccode==rccode )
				{
					// key-repeat - cursors are okay
					int tkey = translate(rccode);
					if ((tkey==RC_up)
						|| (tkey==RC_down)
						|| (tkey==RC_left)
						|| (tkey==RC_right))
					{
						erg = tkey;
						repeat = false;
					}
				}
    				else
    				{
					erg = translate(rccode);
    					prevrccode=rccode;
				    	if(erg!=RC_nokey)
    					{
				    		repeat=false;
				    	}
    				}
            		}
		}
	}
	return erg;
}


/**************************************************************************
 *	transforms the rc-key to generic - internal use only!
 **************************************************************************/
int CRCInput::translate(int code)
{
	if ((code&0xFF00)==0x5C00)
	{
		switch (code&0xFF)
		{
		case 0x0C: return RC_standby;
		case 0x20: return RC_home;
		case 0x27: return RC_setup;
		case 0x00: return RC_0;
		case 0x01: return RC_1;
		case 0x02: return RC_2;
		case 0x03: return RC_3;
		case 0x04: return RC_4;
		case 0x05: return RC_5;
		case 0x06: return RC_6;
		case 0x07: return RC_7;
		case 0x08: return RC_8;
		case 0x09: return RC_9;
		case 0x3B: return RC_blue;
		case 0x52: return RC_yellow;
		case 0x55: return RC_green;
		case 0x2D: return RC_red;
		case 0x54: return RC_page_up;
		case 0x53: return RC_page_down;
		case 0x0E: return RC_up;
 		case 0x0F: return RC_down;
		case 0x2F: return RC_left;
 		case 0x2E: return RC_right;
		case 0x30: return RC_ok;
 		case 0x16: return RC_plus;
 		case 0x17: return RC_minus;
 		case 0x28: return RC_spkr;
 		case 0x82: return RC_help;
		default:
			return RC_nokey;
		}
	} else if (!(code&0x00))
		return code&0x3F;

	return RC_nokey;
}

