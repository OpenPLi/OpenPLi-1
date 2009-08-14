/*
 *   libavs.c - audio/video switch library (dbox-II-project)
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
 *   $Log: libavs.c,v $
 *   Revision 1.4  2002/08/21 09:14:52  obi
 *   no more compile warnings
 *
 *   Revision 1.3  2002/03/06 14:00:35  gillem
 *   - more work on avslib
 *
 *   Revision 1.2  2002/03/06 08:54:06  gillem
 *   - some fixes
 *   - add testavs
 *
 *   Revision 1.1  2002/03/04 16:10:11  gillem
 *   - initial release
 *
 *
 *
 *   $Revision: 1.4 $
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <dbox/avs_core.h>
#include <dbox/saa7126_core.h>

#include "libavs.h"

int dbg;
int avs_fd=-1;
int saa_fd=-1;
int avs_type;

void sys_error( char * text )
{
    if(dbg)
	perror(text);
}

int avsInit( int debug )
{
    int ret = -1;
    
    dbg = debug;
    
    if ( (saa_fd = open( SAA_DEVICE, O_RDWR )) == -1 )
    {
	sys_error("saa open: ");
    }
    else if ( (avs_fd = open( AVS_DEVICE, O_RDWR )) == -1 )
    {
	sys_error("avs open: ");
    }
    else
    {
	if ( ioctl( avs_fd, AVSIOGTYPE, &avs_type ) != 0 )
	{
	    sys_error("ioctl: ");
	}
	else
	{
	    ret = 0;
	}
    }
    
    return ret;
}

void avsDeInit()
{
    if( avs_fd != -1 )
    {
	close(avs_fd);
    }
    
    if( saa_fd != -1 )
    {
	close(saa_fd);
    }
}

int avsSetVolume( int vol )
{
    int ret = -1;
    
    if ( (vol >= 0) && (vol < 63) )
    {
	if ( ioctl( avs_fd, AVSIOSVOL, &vol ) != 0 )
	{
	    sys_error("ioctl: ");
	}
	else
	{
	    ret = 0;
	}
    }
    
    return ret;
}

int avsGetVolume( void )
{
    int ret = -1;
    int vol;
    
    if ( ioctl( avs_fd, AVSIOGVOL, &vol ) != 0 )
    {
	sys_error("ioctl: ");
    }
    else
    {
	ret = vol;
    }
    
    return ret;
}

int avsSetMute( eSwitch mute )
{
    int ret = -1;
    
    if ( ioctl( avs_fd, AVSIOSMUTE, &mute ) != 0 )
    {
	sys_error("ioctl: ");
    }
    else
    {
	ret = 0;
    }
    
    return ret;
}

int avsSetLevel( eLevel level )
{
	int ret = -1;
	int videolevel;

	switch(level)
	{
		case elOFF:
			videolevel = 0;
			break;
		case el4X3:
			videolevel = 3;
			break;
		case el16X9:
			switch(avs_type)
			{
				case CXA2092:
				case CXA2126:
					videolevel = 1;
					break;
				case STV6412:
					videolevel = 2;
					break;
			}
			break;
		default:
			return ret;
			break;
	}

	if ( ioctl( avs_fd, AVSIOSFNC, &videolevel ) != 0 )
	{
		sys_error("ioctl: ");
	}
	else
	{
		ret = 0;
	}

	return ret;
}

/*
 * the very nice routing, please bug report and NO dau report ...
 *
 * 1. port (vcr,tv,cinch)
 * 2. source (mute,encoder,vcr)
 * 3. mode (only encoder)
 * 4. set avs/saa(mode) specific values and route
 *
 */
int avsSetRoute( ePort port, eSource source, eMode mode )
{
	int ret = -1;
	int videoport = 0;
	int audioport = 0;
	int videosource,audiosource;
	int video = 0;
	int audio = 0;
	int videomode;
	int videofblk;

	switch(port)
	{
		case epVCR:
		{
			switch(source)
			{
				case esVCR:
				{
					video = 1;
					audio = 1;

					switch(avs_type)
					{
						case CXA2092:
						case CXA2126:
						{
							videoport = AVSIOSVSW2;
							videosource = 3;
							audioport = AVSIOSASW2;
							audiosource = 2;
							ret = 0;
							break;
						}
					}

					break;
				}

				case esMUTE:
				{
					video = 1;
					audio = 0;

					switch(avs_type)
					{
						case CXA2092:
						case CXA2126:
						{
							videoport = AVSIOSVSW2;
							videosource = 7;
							ret = 0;
							break;
						}
					}

					break;
				}

				case esDIGITALENCODER:
				{
					/*    C   | CVBS/Y
					 *
					 * 0: R/C | CVBS/Y
					 * 1: C   | CVBS/Y (spare inputs)
					 * 4: R/C | G/CVBS/Y
					 *
					 */
					video = 1;
					audio = 1;

					switch(avs_type)
					{
						case CXA2092:
						case CXA2126:
						{
							videoport = AVSIOSVSW2;
							videosource = 1;
							audioport = AVSIOSASW2;
							audiosource = 1;
							ret = 0;
							break;
						}
					}

					break;
				}

				default:
				{
					break;
				}
			}
		}

		case epTV:
		{
			switch(source)
			{
				case esVCR:
				{
					video = 1;
					audio = 1;

					switch(avs_type)
					{
						case CXA2092:
						case CXA2126:
						{
							videoport = AVSIOSVSW1;
							videosource = 3;
							audioport = AVSIOSASW1;
							audiosource = 2;
							ret = 0;
							break;
						}
					}

					break;
				}

				case esMUTE:
				{
					video = 1;
					audio = 0;

					switch(avs_type)
					{
						case CXA2092:
						case CXA2126:
						{
							videoport = AVSIOSVSW1;
							videosource = 7;
							ret = 0;
							break;
						}
					}

					break;
				}

				case esDIGITALENCODER:
				{
					video = 1;
					audio = 1;

					switch(avs_type)
					{
						case CXA2092:
						case CXA2126:
						{
							videoport = AVSIOSVSW1;
							videosource = 1;
							audioport = AVSIOSASW1;
							audiosource = 1;
							ret = 0;
							break;
						}
					}
					break;
				}

				default:
				{
					break;
				}
			}
		}

		case epCINCH:
		{
			break;
		}

		default:
		{
			break;
		}
	}

	if ( ret == 0 )
	{
		if ( video == 1 )
		{
			if ( ioctl( avs_fd, videoport, &videosource ) != 0 )
			{
				sys_error("ioctl: ");
				ret = -1;
			}
		}
    
		if ( audio == 1 )
		{
			if ( ioctl( avs_fd, audioport, &audiosource ) != 0 )
			{
				sys_error("ioctl: ");
				ret = -1;
			}
		}

		// fblk bad mode here (0:0V(cvbs),1:5V(rgb),2:DE(auto),3:VCR(auto))
		if ( (mode != emNONE) && (source == esDIGITALENCODER) )
		{
			switch(mode)
			{
				case emCVBS:
					videomode = SAA_MODE_FBAS;
					videofblk = 0;
					break;
				case emRGB:
					videomode = SAA_MODE_RGB;
					videofblk = 1;
					break;
				case emYC:
					videomode = SAA_MODE_SVIDEO;
					videofblk = 0;
					break;
				default:
					videomode = -1;
					break;
			}

			if ( videomode != -1 )
			{
				if ( ioctl( saa_fd, SAAIOSMODE, &videomode ) != 0 )
				{
					sys_error("ioctl: ");
					ret = -1;
				}
				else if ( ioctl( avs_fd, AVSIOGFBLK, &videofblk ) != 0 )
				{
					sys_error("ioctl: ");
					ret = -1;
				}
			}
		}
	}

	return ret;
}
