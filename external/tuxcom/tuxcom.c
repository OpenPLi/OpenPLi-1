/*
	TuxCom - TuxBox-Commander Plugin

	Copyright (C) 2004 'dbluelle' (dbluelle@blau-weissoedingen.de)

	Homepage: http://www.blau-weissoedingen.de/dreambox/

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

#include "tuxcom.h"
/******************************************************************************
 * GetRCCode  (Code from Tuxmail)
 ******************************************************************************/



int GetRCCode(int mode)
{
	static int count = 0;
	//get code
	struct input_event ev;
	static __u16 rc_last_key = KEY_RESERVED;
	static __u16 rc_last_code = KEY_RESERVED;
	if(read(rc, &ev, sizeof(ev)) == sizeof(ev))
	{
		if(ev.value)
		{
			if(ev.code == rc_last_key)
			{
				if (count < REPEAT_TIMER)
				{
					count++;
					rccode = -1;
					return 1;
				}
			}
			else
				count = 0;
			rc_last_key = ev.code;
			switch(ev.code)
			{
				case KEY_UP:		rccode = RC_UP;			break;
				case KEY_DOWN:		rccode = RC_DOWN;		break;
				case KEY_LEFT:		rccode = RC_LEFT;		break;
				case KEY_RIGHT:		rccode = RC_RIGHT;		break;
				case KEY_OK:		rccode = RC_OK;			break;
				case KEY_RED:		rccode = RC_RED;		break;
				case KEY_GREEN:		rccode = RC_GREEN;		break;
				case KEY_YELLOW:	rccode = RC_YELLOW;		break;
				case KEY_BLUE:		rccode = RC_BLUE;		break;
				case KEY_INFO:		rccode = RC_HELP;		break;
				case KEY_MENU:		rccode = RC_DBOX;		break;
				case KEY_EXIT:		rccode = RC_HOME;		break;
				case KEY_POWER:		rccode = RC_STANDBY;	break;
				default:
					if( ev.code > 0x7F )
					{
						rccode = 0;
						if( ev.code == 0x110 )
						{
							rccode = RC_ON;
						}
					}
					else
					{
						rccode = rctable[ev.code & 0x7F];
					}
					if( rc_last_code == RC_LSHIFT )
					{
						if( ev.code <= 0x56 )  //(sizeof(rcshifttable)/sizeof(int)-1)
						{
							rccode = rcshifttable[ev.code];
						}
					}
					else if( rc_last_code == RC_ALTGR )
					{
						if( ev.code <= 0x56 )  //(sizeof(rcaltgrtable)/sizeof(int)-1)
						{
							rccode = rcaltgrtable[ev.code];
						}
					}
					else if( rc_last_code == RC_ALT )
					{
						if((ev.code >=2) && ( ev.code <= 11 ))
						{
							rccode = (ev.code-1) | 0x0200;
						}
					}
//					if( !rccode )
					{
//						rccode = -1;
					}

			}
			rc_last_code = rccode;
			return 1;
		}
		else
		{
			rccode = -1;
			rc_last_key = KEY_RESERVED;
			rc_last_code = KEY_RESERVED;
		}
	}

	count=0;
	if(read(kb, &ev, sizeof(ev)) == sizeof(ev))
	{
		if(ev.value)
		{
			if(ev.code == rc_last_key)
			{
				if (count < REPEAT_TIMER)
				{
					count++;
					rccode = -1;
					return 1;
				}
			}
			else
				count = 0;
			rc_last_key = ev.code;
			switch(ev.code)
			{
				case KEY_UP:		rccode = RC_UP;			break;
				case KEY_DOWN:		rccode = RC_DOWN;		break;
				case KEY_LEFT:		rccode = RC_LEFT;		break;
				case KEY_RIGHT:		rccode = RC_RIGHT;		break;
				case KEY_OK:		rccode = RC_OK;			break;
				case KEY_RED:		rccode = RC_RED;		break;
				case KEY_GREEN:		rccode = RC_GREEN;		break;
				case KEY_YELLOW:	rccode = RC_YELLOW;		break;
				case KEY_BLUE:		rccode = RC_BLUE;		break;
				case KEY_INFO:		rccode = RC_HELP;		break;
				case KEY_MENU:		rccode = RC_DBOX;		break;
				case KEY_EXIT:		rccode = RC_HOME;		break;
				case KEY_POWER:		rccode = RC_STANDBY;	break;
				default:
					if( ev.code > 0x7F )
					{
						rccode = 0;
						if( ev.code == 0x110 )
						{
							rccode = RC_ON;
						}
					}
					else
					{
						rccode = rctable[ev.code & 0x7F];
					}
					if( rc_last_code == RC_LSHIFT )
					{
						if( ev.code <= 0x56 )  //(sizeof(rcshifttable)/sizeof(int)-1)
						{
							rccode = rcshifttable[ev.code];
						}
					}
					else if( rc_last_code == RC_ALTGR )
					{
						if( ev.code <= 0x56 )  //(sizeof(rcaltgrtable)/sizeof(int)-1)
						{
							rccode = rcaltgrtable[ev.code];
						}
					}
					else if( rc_last_code == RC_ALT )
					{
						if((ev.code >=2) && ( ev.code <= 11 ))
						{
							rccode = (ev.code-1) | 0x0200;
						}
					}
//					if( !rccode )
					{
//						rccode = -1;
					}

			}
			rc_last_code = rccode;
			return 1;
		}
		else
		{
			rccode = -1;
			rc_last_key = KEY_RESERVED;
			rc_last_code = KEY_RESERVED;
		}
	}

		rccode = -1;
		usleep(1000000/100);
		return 0;
}


/******************************************************************************
 * MyFaceRequester
 ******************************************************************************/

FT_Error MyFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	FT_Error result;

	result = FT_New_Face(library, face_id, 0, aface);

	if(!result) printf("TuxCom <Font \"%s\" loaded>\n", (char*)face_id);
	else        printf("TuxCom <Font \"%s\" failed>\n", (char*)face_id);

	return result;
}

/******************************************************************************
 * RenderChar
 ******************************************************************************/

int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, int color)
{
	int row, pitch, bit, x = 0, y = 0;
	FT_UInt glyphindex;
	FT_Vector kerning;
	FT_Error error;

	currentchar=currentchar & 0xFF;

	if (currentchar == '\r') // display \r in windows edited files
	{
		if(color != -1)
		{
			if (sx+10 < ex)
			{
				RenderBox(sx,sy-10, sx+10,sy,GRID,color);
			}
			else
				return -1;
		}
		return 10;
	}
	//load char

		if(!(glyphindex = FT_Get_Char_Index(face, (int)currentchar)))
		{
			printf("TuxCom <FT_Get_Char_Index for Char \"%c\" failed\n", (int)currentchar);
			return 0;
		}



		if((error = FTC_SBit_Cache_Lookup(cache, &desc, glyphindex, &sbit)))
		{
			printf("TuxCom <FTC_SBitCache_Lookup for Char \"%c\" failed with Errorcode 0x%.2X>\n", (int)currentchar, error);
			return 0;
		}

// no kerning used
/*
		if(use_kerning)
		{
			FT_Get_Kerning(face, prev_glyphindex, glyphindex, ft_kerning_default, &kerning);

			prev_glyphindex = glyphindex;
			kerning.x >>= 6;
		}
		else
*/
			kerning.x = 0;

	//render char

		if(color != -1) /* don't render char, return charwidth only */
		{
			if(sx + sbit->xadvance >= ex) return -1; /* limit to maxwidth */

			for(row = 0; row < sbit->height; row++)
			{
				for(pitch = 0; pitch < sbit->pitch; pitch++)
				{
					for(bit = 7; bit >= 0; bit--)
					{
						if(pitch*8 + 7-bit >= sbit->width) break; /* render needed bits only */

						if((sbit->buffer[row * sbit->pitch + pitch]) & 1<<bit) memcpy(lbb + StartX*4 + sx*4 + (sbit->left + kerning.x + x)*4 + fix_screeninfo.line_length*(StartY + sy - sbit->top + y),bgra[color],4);

						x++;
					}
				}

				x = 0;
				y++;
			}

		}

	//return charwidth

		return sbit->xadvance + kerning.x;
}

/******************************************************************************
 * GetStringLen
 ******************************************************************************/

int GetStringLen(const char *string, int size)
{
	int stringlen = 0;

	//set size

		switch (size)
		{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
		case VERY_SMALL: desc.width = desc.height = FONTHEIGHT_VERY_SMALL; break;
		case SMALL     : desc.width = desc.height = FONTHEIGHT_SMALL     ; break;
		case BIG       : desc.width = desc.height = FONTHEIGHT_BIG       ; break;
#else
		case VERY_SMALL: desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_VERY_SMALL; break;
		case SMALL     : desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_SMALL     ; break;
		case BIG       : desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_BIG       ; break;
#endif
	 	}

	//reset kerning

		prev_glyphindex = 0;

	//calc len

		while(*string != '\0')
		{
			stringlen += RenderChar(*string, -1, -1, -1, -1);
			string++;
		}

	return stringlen;
}

/******************************************************************************
 * RenderString
 ******************************************************************************/

void RenderString(const char *string, int sx, int sy, int maxwidth, int layout, int size, int color)
{
	int stringlen, ex, charwidth;

	//set size

		switch (size)
		{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
		case VERY_SMALL: desc.width = desc.height = FONTHEIGHT_VERY_SMALL; break;
		case SMALL     : desc.width = desc.height = FONTHEIGHT_SMALL     ; break;
		case BIG       : desc.width = desc.height = FONTHEIGHT_BIG       ; break;
#else
		case VERY_SMALL: desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_VERY_SMALL; break;
		case SMALL     : desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_SMALL     ; break;
		case BIG       : desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_BIG       ; break;
#endif
	 	}

	//set alignment

		if(layout != LEFT)
		{
			stringlen = GetStringLen(string, size);

			switch(layout)
			{
				case CENTER:	if(stringlen < maxwidth) sx += (maxwidth - stringlen)/2;
						break;

				case RIGHT:	if(stringlen < maxwidth) sx += maxwidth - stringlen;
			}
		}

	//reset kerning

		prev_glyphindex = 0;

	//render string

		ex = sx + maxwidth;

		while(*string != '\0' && *string != '\n')
		{
			if((charwidth = RenderChar(*string, sx, sy, ex, color)) == -1) return; /* string > maxwidth */

			sx += charwidth;
			string++;
		}
}

/******************************************************************************
 * RenderBox
 ******************************************************************************/

void RenderBox(int sx, int sy, int ex, int ey, int mode, int color)
{
	int loop;
	int tx;
	if(mode == FILL)
	{
		for(; sy <= ey; sy++)
		{
		for(tx=0; tx <= (ex-sx); tx++)	
			{
			memcpy(lbb + StartX*4 + sx*4 + (tx*4) + fix_screeninfo.line_length*(StartY + sy),bgra[color],4);
			}
		}
	}
	else
	{
		for(loop = sx; loop <= ex; loop++)
		{
			memcpy(lbb + StartX*4+loop*4 + fix_screeninfo.line_length*(sy+StartY), bgra[color], 4);
			memcpy(lbb + StartX*4+loop*4 + fix_screeninfo.line_length*(sy+1+StartY), bgra[color], 4);
			memcpy(lbb + StartX*4+loop*4 + fix_screeninfo.line_length*(ey-1+StartY), bgra[color], 4);
			memcpy(lbb + StartX*4+loop*4 + fix_screeninfo.line_length*(ey+StartY), bgra[color], 4);
		}
		for(loop = sy; loop <= ey; loop++)
		{
			memcpy(lbb + StartX*4+sx*4 + fix_screeninfo.line_length*(loop+StartY), bgra[color], 4);
			memcpy(lbb + StartX*4+(sx+1)*4 + fix_screeninfo.line_length*(loop+StartY), bgra[color], 4);
			memcpy(lbb + StartX*4+(ex-1)*4 + fix_screeninfo.line_length*(loop+StartY), bgra[color], 4);
			memcpy(lbb + StartX*4+ex*4 + fix_screeninfo.line_length*(loop+StartY), bgra[color], 4);
		}
	}
}

void SetLanguage()
{
	if (langselect == BTN_AUTO)
	{
		language=LANG_INT;
		if (strncmp(setlocale( LC_ALL, NULL ),"de",2) == 0) language=LANG_DE;
		if (strncmp(setlocale( LC_ALL, NULL ),"it",2) == 0) language=LANG_IT;
		if (strncmp(setlocale( LC_ALL, NULL ),"sv",2) == 0) language=LANG_SV;
		if (strncmp(setlocale( LC_ALL, NULL ),"pt",2) == 0) language=LANG_PT;
	}
	else
	{
		switch (langselect)
		{
			case BTN_GERMAN   : language = LANG_DE ; break;
			case BTN_ITALIAN  : language = LANG_IT ; break;
			case BTN_SWEDISH  : language = LANG_SV ; break;
			case BTN_PORTUGUES: language = LANG_PT ; break;
			default           : language = LANG_INT; break;
		}
	}
}

/******************************************************************************
 * plugin_exec                                                                *
 ******************************************************************************/

int main()
{
	FT_Error error;

	//show versioninfo

	printf(MSG_VERSION);
	char szMessage[400];

	//get params


	kb = fb = rc = sx = ex = sy = ey = -1;

	/* open Framebuffer */
	fb=open("/dev/fb/0", O_RDWR);

	/* open Remote Control */
	int cnt=0;
	while(1)
	{
		struct stat s;
		char tmp[128];
		sprintf(tmp, "/dev/input/event%d", cnt);
		if (stat(tmp, &s))
			break;
		/* open Remote Control */
		if ((rc=open(tmp, O_RDONLY)) == -1)
		{
			perror("TuxCom <open remote control>");
			return 0;
		}
		if (ioctl(rc, EVIOCGNAME(128), tmp) < 0)
			perror("EVIOCGNAME failed");
		if (strstr(tmp, "remote control"))
			break;
		close(rc);
		rc=-1;
		++cnt;
	}
	fcntl(rc, F_SETFL, fcntl(rc, F_GETFL) | O_EXCL | O_NONBLOCK);

	/* open dream ir keyboard */
	cnt=0;
	while(1)
	{
		struct stat s;
		char tmp[128];
		sprintf(tmp, "/dev/input/event%d", cnt);
		if (stat(tmp, &s))
			break;
		/* open keyboard */
		if ((kb=open(tmp, O_RDONLY)) == -1)
		{
			perror("TuxCom <open ir keyboard>");
			return 0;
		}
		if (ioctl(kb, EVIOCGNAME(128), tmp) < 0)
			perror("EVIOCGNAME failed");
		if (strstr(tmp, "dreambox ir keyboard"))
			break;
		close(kb);
		kb=-1;
		++cnt;
	}
	fcntl(kb, F_SETFL, fcntl(kb, F_GETFL) | O_EXCL | O_NONBLOCK);

	sx = 50;
	ex = 670;
	sy = 50;
	ey = 526;

/*	for(; par; par = par->next)
	{
		if	(!strcmp(par->id, P_ID_FBUFFER)) fb = atoi(par->val);
		else if	(!strcmp(par->id, P_ID_RCINPUT)) rc = atoi(par->val);
		else if	(!strcmp(par->id, P_ID_OFF_X))   sx = atoi(par->val);
		else if	(!strcmp(par->id, P_ID_END_X))   ex = atoi(par->val);
		else if	(!strcmp(par->id, P_ID_OFF_Y))   sy = atoi(par->val);
		else if	(!strcmp(par->id, P_ID_END_Y))   ey = atoi(par->val);
	}
*/

	if(fb == -1 || rc == -1 || sx == -1 || ex == -1 || sy == -1 || ey == -1)
	{
		printf("TuxCom <missing Param(s)>\n");
		return;
	}
	//init framebuffer

	if(ioctl(fb, FBIOGET_VSCREENINFO, &var_screeninfo) == -1)
	{
		printf("TuxCom <FBIOGET_VSCREENINFO failed>\n");
		return;
	}

	var_screeninfo_original = var_screeninfo;

	var_screeninfo.xres = 720;
	var_screeninfo.yres = 576;

	/* set variable screeninfo for double buffering */
	var_screeninfo.yres_virtual = 2*var_screeninfo.yres;
	var_screeninfo.xres_virtual = var_screeninfo.xres;
	var_screeninfo.xoffset = var_screeninfo.yoffset = 0;

	if(ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo) == -1)
	{
		printf("TuxCom <FBIOPUT_VSCREENINFO failed>\n");
		return;
	}

	if(ioctl(fb, FBIOGET_FSCREENINFO, &fix_screeninfo) == -1)
	{
		printf("TuxCom <FBIOGET_FSCREENINFO failed>\n");
		return;
	}

	if(ioctl(fb, FBIOPUTCMAP, &colormap) == -1)
	{
		printf("TuxCom <FBIOPUTCMAP failed>\n");
		return;
	}

	if(!(lfb = (unsigned char*)mmap(0, fix_screeninfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0)))
	{
		printf("TuxCom <mapping of Framebuffer failed>\n");
		return;
	}

	//init fontlibrary

	if((error = FT_Init_FreeType(&library)))
	{
		printf("TuxCom <FT_Init_FreeType failed with Errorcode 0x%.2X>", error);
		memset(lfb, 0, fix_screeninfo.smem_len);
		munmap(lfb, fix_screeninfo.smem_len);
		ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
		return;
	}

	if((error = FTC_Manager_New(library, 1, 2, 0, &MyFaceRequester, NULL, &manager)))
	{
		printf("TuxCom <FTC_Manager_New failed with Errorcode 0x%.2X>\n", error);
		FT_Done_FreeType(library);
		memset(lfb, 0, fix_screeninfo.smem_len);
		munmap(lfb, fix_screeninfo.smem_len);
		ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
		return;
	}

	if((error = FTC_SBitCache_New(manager, &cache)))
	{
		printf("TuxCom <FTC_SBitCache_New failed with Errorcode 0x%.2X>\n", error);
		FTC_Manager_Done(manager);
		FT_Done_FreeType(library);
		memset(lfb, 0, fix_screeninfo.smem_len);
		munmap(lfb, fix_screeninfo.smem_len);
		ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
		return;
	}

	if((error = FTC_Manager_Lookup_Face(manager, FONT, &face)))
	{
		if((error = FTC_Manager_Lookup_Face(manager, FONT2, &face)))
		{
			printf("TuxCom <FTC_Manager_Lookup_Face failed with Errorcode 0x%.2X>\n", error);
			FTC_Manager_Done(manager);
			FT_Done_FreeType(library);
			memset(lfb, 0, fix_screeninfo.smem_len);
			munmap(lfb, fix_screeninfo.smem_len);
			ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
			return;
		}
		else
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
			desc.face_id = FONT2;
#else
			desc.font.face_id = FONT2;
#endif
	}
	else
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
		desc.face_id = FONT;
#else
		desc.font.face_id = FONT;
#endif


	use_kerning = FT_HAS_KERNING(face);

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	desc.flags = FT_LOAD_MONOCHROME;
#else
	desc.image_type = ftc_image_mono;
#endif



	//init backbuffer

	if(!(lbb = malloc(fix_screeninfo.line_length*var_screeninfo.yres)))
	{
		printf("TuxCom <allocating of Backbuffer failed>\n");
		FTC_Manager_Done(manager);
		FT_Done_FreeType(library);
		memset(lfb, 0, fix_screeninfo.smem_len);
		munmap(lfb, fix_screeninfo.smem_len);
		ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);
		return;
	}
	memset(lbb, 0, fix_screeninfo.line_length*var_screeninfo.yres);
	RenderBox(0,0,var_screeninfo.xres,var_screeninfo.yres,FILL,BLACK);

	//open avs
/*	if((avs = open(AVS, O_RDWR)) == -1)
	{
		printf("TuxCom <open AVS>");
		return;
	}
	ioctl(avs, AVSIOGSCARTPIN8, &fnc_old);
	ioctl(avs, AVSIOSSCARTPIN8, &fncmodes[screenmode]);
	//open saa
	if((saa = open(SAA, O_RDWR)) == -1)
	{
		printf("TuxCom <open SAA>");
		return;
	}
*/


	//init data
	curframe = 0;
	cursort = SORT_UP;
	curvisibility = 0;
	textuppercase = 0;
	screenmode=0;
	langselect = BTN_AUTO; // automatic
	autosave = BTN_ASK; // ask on exit

	commandsize =sysconf(_SC_ARG_MAX )-100;
	szClipboard[0] = 0x00;
	szSearchstring[0] = 0x00;
	szTextSearchstring[0] = 0x00;
	memset(tool, ACTION_NOACTION, sizeof(tool));
	colortool[0] = ACTION_EXEC   ;
	colortool[1] = ACTION_MARKER ;
	colortool[2] = ACTION_SORT   ;
	colortool[3] = ACTION_REFRESH;

	memset(&finfo[0], 0, sizeof(finfo[0]));
	memset(&finfo[1], 0, sizeof(finfo[0]));


	// center output on screen
	StartX = sx;
	StartY = sy;
	viewx = ex - sx;
	viewy = ey - sy;
    menuitemwidth  = viewx / MENUITEMS;
    menuitemnumber = viewx / (MENUITEMS*6);

	framerows = (viewy-MENUSIZE - 3*BORDERSIZE - FONTHEIGHT_SMALL) / FONTHEIGHT_SMALL;

	FrameWidth = viewx/2;
	NameWidth = (FrameWidth / 3 ) * 2;
	SizeWidth = (FrameWidth / 3 ) - 3* BORDERSIZE;


	ReadSettings();

	SetLanguage();

//	ioctl(saa, SAAIOGWSS, &saa_old);
//	ioctl(saa, SAAIOSWSS, &saamodes[screenmode]);
	// setup screen
	RenderFrame(LEFTFRAME);
	RenderFrame(RIGHTFRAME);
	memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
	printf("TuxCom init successful\n");

//	fcntl(rc, F_SETFL, fcntl(rc, F_GETFL) &~ O_NONBLOCK);

	int dosave = autosave;
	int firstentry = 1;
 	struct fileentry *pfe;
	char action[256];
	char szSel [256];
	int pos, check;
	do{
		overwriteall = 0;
		skipall = 0;
		GetRCCode(RC_NORMAL);

		// hack to ignore the first OK press (from starting the plugin)
		if (firstentry == 1)
		{
			if (rccode == RC_OK) continue;

			// check password
			if (szPass[0] != 0x00)
			{
				char szP[20];
				*szP = 0x00;
				if (GetInputString(250,19,szP,info[INFO_PASS1*NUM_LANG+language], YES) != RC_OK) break;
				if (strcmp(szP,szPass) != 0) break;
				RenderFrame(LEFTFRAME);
				RenderFrame(RIGHTFRAME);
				memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
			}
		}
		firstentry = 0;

		switch(rccode)
		{
				case RC_HELP:
					MessageBox(MSG_VERSION,MSG_COPYRIGHT,OK);
					break;
				case RC_OK:
					pfe = GetSelected(curframe);
					if (pfe && (S_ISDIR(pfe->fentry.st_mode) ||  (finfo[curframe].zipfile[0] != 0x00 && S_ISLNK(pfe->fentry.st_mode))))
					{
						if (strcmp(pfe->name,"..") == 0)
						{
							ClearMarker(curframe);
							FillDir(curframe,SELECT_UPDIR);
						}
						else if (strcmp(pfe->name,"/") == 0)
						{
							ClearMarker(curframe);
							FillDir(curframe,SELECT_ROOTDIR);
						}
						else
						{
							if (finfo[curframe].zipfile[0] != 0x00)
							{
								strncat(finfo[curframe].zippath,pfe->name,256);
								strncat(finfo[curframe].zippath,"/",1);
							}
							else
							{
								strncat(finfo[curframe].path,pfe->name,256);
								strncat(finfo[curframe].path,"/",1);
							}
							finfo[curframe].selected =1;
							finfo[curframe].first    =0;
							ClearMarker(curframe);
							FillDir(curframe,SELECT_NOCHANGE);
						}
					}
					else if (pfe && S_ISLNK(pfe->fentry.st_mode))
					{
						struct stat fs;
						char fullfile[FILENAME_MAX];
						sprintf(fullfile,"%s%s",finfo[curframe].path,pfe->name);
						stat(fullfile,&fs);
						if (S_ISDIR(fs.st_mode))
						{
							strncat(finfo[curframe].path,pfe->name,256);
							strncat(finfo[curframe].path,"/",1);
							finfo[curframe].selected =0;
							finfo[curframe].first    =0;
							ClearMarker(curframe);
							FillDir(curframe,SELECT_NOCHANGE);
						}
						else
						{
							if (pfe && ((pfe->fentry.st_mode & S_IXUSR) == S_IXUSR))
							{
								sprintf(szMessage,msg[MSG_EXEC*NUM_LANG+language], pfe->name);
								switch (MessageBox(szMessage,info[INFO_EXEC*NUM_LANG+language],OKHIDDENCANCEL))
								{
									case YES:
										sprintf(action,"\"%s%s\"",finfo[curframe].path, pfe->name);
										DoExecute(action, SHOW_OUTPUT);
										FillDir(1-curframe,SELECT_NOCHANGE);
										FillDir(  curframe,SELECT_NOCHANGE);
										break;
									case HIDDEN:
										sprintf(action,"\"%s%s\" &",finfo[curframe].path, pfe->name);
										DoExecute(action, SHOW_NO_OUTPUT);
										break;
									default:
										rccode = 0;
								}
							}
						}

					}
					else if (pfe && ((pfe->fentry.st_mode & S_IRUSR) == S_IRUSR) && ((check = CheckZip(pfe->name))>= GZIP) && (finfo[curframe].zipfile[0] == 0x00))
					{
						ReadZip(check);
						FillDir(curframe,SELECT_NOCHANGE);
						SetSelected(curframe,"..");

					}
					else if (pfe && ((pfe->fentry.st_mode & S_IXUSR) == 0) && finfo[curframe].zipfile[0] == 0x00)
					{
						RenderMenuLine(ACTION_VIEW-1, YES);
						DoViewFile();
					}
					else if (pfe && ((pfe->fentry.st_mode & S_IXUSR) == S_IXUSR) && finfo[curframe].zipfile[0] == 0x00)
					{
						sprintf(szMessage,msg[MSG_EXEC*NUM_LANG+language], pfe->name);
						switch (MessageBox(szMessage,info[INFO_EXEC*NUM_LANG+language],OKHIDDENCANCEL))
						{
							case YES:
								sprintf(action,"\"%s%s\"",finfo[curframe].path, pfe->name);
								DoExecute(action, SHOW_OUTPUT);
								FillDir(1-curframe,SELECT_NOCHANGE);
								FillDir(  curframe,SELECT_NOCHANGE);
								break;
							case HIDDEN:
								sprintf(action,"\"%s%s\" &",finfo[curframe].path, pfe->name);
								DoExecute(action, SHOW_NO_OUTPUT);
								break;
							default:
								rccode = 0;
						}
					}
					break;
				case RC_LEFT:
					curframe = 0;
					break;
				case RC_RIGHT:
					curframe = 1;
					break;
				case RC_UP:
					finfo[curframe].selected--;
					if (finfo[curframe].selected  < 0)
						finfo[curframe].selected  = finfo[curframe].count -1;
					break;
				case RC_DOWN:
					finfo[curframe].selected++;
					if (finfo[curframe].selected >= finfo[curframe].count)
						finfo[curframe].selected  = 0;
					break;
				case RC_PLUS:
					finfo[curframe].selected-= framerows;
					break;
				case RC_MINUS:
					finfo[curframe].selected+= framerows;
					break;
				case RC_1:
					if (tool[ACTION_PROPS-1] == ACTION_PROPS)
					{
						RenderMenuLine(ACTION_PROPS-1, YES);
						if (ShowProperties() == YES)
						{
							FillDir(1-curframe,SELECT_NOCHANGE);
							FillDir(  curframe,SELECT_NOCHANGE);
						}
					}
					break;
				case RC_2:
					if (tool[ACTION_RENAME-1] == ACTION_RENAME)
					{
						RenderMenuLine(ACTION_RENAME-1, YES);
						pfe = GetSelected(curframe);
						char szBuf[256];
						char szMsg[356];
						strcpy(szBuf,pfe->name);
						sprintf(szMsg,msg[MSG_RENAME*NUM_LANG+language], pfe->name);
						int nok = 0;
						while (nok == 0)
						{
							switch (GetInputString(400,255,szBuf,szMsg, NO))
							{
								case RC_OK:
								{
									if (*szBuf == 0x00)
									{
										nok = 1;
										break;
									}
									if (FindFile(curframe,szBuf) != NULL)
									{
										char szMsg2[356];
										sprintf(szMsg2,msg[MSG_FILE_EXISTS*NUM_LANG+language], szBuf);
										MessageBox(szMsg2,"",OK);
										break;
									}
									else
									{
										char szOld[FILENAME_MAX],szNew[FILENAME_MAX];
										sprintf(szOld,"%s%s",finfo[curframe].path, pfe->name);
										sprintf(szNew,"%s%s",finfo[curframe].path, szBuf    );
										rename(szOld,szNew);
										RenameMarker(curframe,pfe->name,szBuf);
										if (strcmp(finfo[curframe].path,finfo[1-curframe].path) == 0)
											RenameMarker(1-curframe,pfe->name,szBuf);
										FillDir(1-curframe,SELECT_NOCHANGE);
										FillDir(  curframe,SELECT_NOCHANGE);
										SetSelected(curframe,szBuf);
										if (strcmp(finfo[curframe].path,finfo[1-curframe].path) == 0)
											SetSelected(1-curframe,szBuf);

										nok = 1;
									}
								}
								default:
									rccode = 0;
									nok = 1;
							}
						}
					}
					break;
				case RC_3:
					if (tool[ACTION_VIEW-1] == ACTION_VIEW)
					{
						RenderMenuLine(ACTION_VIEW-1, YES);
						DoViewFile();
					}
					break;
				case RC_4:
					if (tool[ACTION_EDIT-1] == ACTION_EDIT)
					{
						pfe = GetSelected(curframe);
						sprintf(action,"%s%s",finfo[curframe].path, pfe->name);
						if (CheckZip(pfe->name) == FTP)
						{
							colortool[0] = ACTION_NOACTION;
							colortool[1] = ACTION_NOACTION;
							colortool[2] = ACTION_NOACTION;
							colortool[3] = ACTION_NOACTION;
							RenderMenuLine(ACTION_EDIT-1, YES);
							DoEditFTP(action, pfe->name);
						}
						else
						{
							colortool[0] = ACTION_DELLINE ;
							colortool[1] = ACTION_INSLINE ;
							colortool[2] = ACTION_NOACTION;
							colortool[3] = ACTION_TOLINUX ;
							RenderMenuLine(-1, EDITOR);
							DoEditFile(action, action, YES);
						}
						FillDir(1-curframe,SELECT_NOCHANGE);
						FillDir(  curframe,SELECT_NOCHANGE);
					}
					break;
				case RC_5:
					if (tool[ACTION_COPY-1] == ACTION_COPY)
					{
						tmpzipdir[0] = 0x00;
						char* szZipCommand = (char*)malloc(commandsize);
						szZipCommand[0] = 0x00;
						RenderMenuLine(ACTION_COPY-1, YES);
						pfe = GetSelected(curframe);
						if ((finfo[curframe].zipfile[0] == 0x00) && (strcmp(finfo[curframe].path, finfo[1-curframe].path) == 0))
						{
							MessageBox(msg[MSG_COPY_NOT_POSSIBLE*NUM_LANG+language],"",OK);
						}
						else
						{
							if (finfo[curframe].markcount > 0)
							{
								sprintf(szMessage,msg[MSG_COPY_MULTI*NUM_LANG+language], finfo[curframe].markcount, finfo[1-curframe].path);
								switch (MessageBox(szMessage,(finfo[curframe].zipfile[0] == 0x00 ? info[INFO_COPY*NUM_LANG+language] :""),(finfo[curframe].zipfile[0] == 0x00 ? OKHIDDENCANCEL: OKCANCEL )))
								{
									case YES:
									    for (pos = 0; pos < finfo[curframe].count; pos++)
									    {
											if (IsMarked(curframe,pos))
											{
												pfe = getfileentry(curframe, pos);
												if (DoCopy(pfe,YES, OVERWRITESKIPCANCEL,szZipCommand) < 0) break;
											}
										}
										DoZipCopyEnd(szZipCommand);
										FillDir(1-curframe,SELECT_NOCHANGE);
										FillDir(  curframe,SELECT_NOCHANGE);
										break;
									case HIDDEN:
									    for (pos = 0; pos < finfo[curframe].count; pos++)
									    {
											if (IsMarked(curframe,pos))
											{
												pfe = getfileentry(curframe, pos);
												if (DoCopy(pfe,HIDDEN, OVERWRITESKIPCANCEL,szZipCommand) < 0) break;
											}
										}
										break;
									default:
										rccode = 0;
								}
							}
							else
							{
								sprintf(szMessage,msg[MSG_COPY*NUM_LANG+language], pfe->name, finfo[1-curframe].path);
								switch (MessageBox(szMessage,(finfo[curframe].zipfile[0] == 0x00 ? info[INFO_COPY*NUM_LANG+language]:""),(finfo[curframe].zipfile[0] == 0x00 ? OKHIDDENCANCEL : OKCANCEL )))
								{
									case YES:
										if (DoCopy(pfe,YES, OVERWRITECANCEL,szZipCommand) < 0) break;
										DoZipCopyEnd(szZipCommand);
										FillDir(1-curframe,SELECT_NOCHANGE);
										FillDir(  curframe,SELECT_NOCHANGE);
										break;
									case HIDDEN:
										DoCopy(pfe,HIDDEN, OVERWRITECANCEL,szZipCommand);
										break;
									default:
										rccode = 0;
								}
							}
						}
						free(szZipCommand);
					}
					break;
				case RC_6:
					if (tool[ACTION_MOVE-1] == ACTION_MOVE)
					{
						RenderMenuLine(ACTION_MOVE-1, YES);
						pfe = GetSelected(curframe);
						if (finfo[curframe].markcount > 0)
						{
							sprintf(szMessage,msg[MSG_MOVE_MULTI*NUM_LANG+language], finfo[curframe].markcount, finfo[1-curframe].path);
							switch (MessageBox(szMessage,info[INFO_MOVE*NUM_LANG+language],OKHIDDENCANCEL))
							{
								case YES:
									for (pos = 0; pos < finfo[curframe].count; pos++)
									{
										if (IsMarked(curframe,pos))
										{
											pfe = getfileentry(curframe, pos);
											if (DoMove(pfe, YES, OVERWRITESKIPCANCEL) < 0) break;
										}
									}
									ClearMarker(curframe);
									FillDir(1-curframe,SELECT_NOCHANGE);
									FillDir(  curframe,SELECT_NOCHANGE);
									break;
								case HIDDEN:
									for (pos = 0; pos < finfo[curframe].count; pos++)
									{
										if (IsMarked(curframe,pos))
										{
											pfe = getfileentry(curframe, pos);
											if (DoMove(pfe, HIDDEN, OVERWRITESKIPCANCEL) < 0) break;
										}
									}
									ClearMarker(curframe);
									break;
								default:
									rccode = 0;
							}
						}
						else
						{
							sprintf(szMessage,msg[MSG_MOVE*NUM_LANG+language], pfe->name, finfo[1-curframe].path);
							switch (MessageBox(szMessage,info[INFO_MOVE*NUM_LANG+language],OKHIDDENCANCEL))
							{
								case YES:
									if (DoMove(pfe,YES,OVERWRITECANCEL) < 0) break;
									FillDir(1-curframe,SELECT_NOCHANGE);
									FillDir(  curframe,SELECT_NOCHANGE);
									break;
								case HIDDEN:
									DoMove(pfe,HIDDEN,OVERWRITECANCEL);
									break;
								default:
									rccode = 0;
							}
						}
					}
					break;
				case RC_7:
					if (tool[ACTION_MKDIR-1] == ACTION_MKDIR)
					{
						RenderMenuLine(ACTION_MKDIR-1, YES);
						char szDir[FILENAME_MAX];
						szDir[0] = 0x00;
						char szMsg[1000];
						sprintf(szMsg,msg[MSG_MKDIR*NUM_LANG+language],finfo[curframe].path);
						switch (GetInputString(400,255,szDir,szMsg, NO))
						{
							case RC_OK:
							{
								if (*szDir != 0x00)
								{
									if (FindFile(curframe,szDir) != NULL)
									{
										sprintf(szMsg,msg[MSG_FILE_EXISTS*NUM_LANG+language],szDir);
										MessageBox(szMsg,"",OK);
									}
									else
									{
										sprintf(action,"mkdir -p \"%s%s\"",finfo[curframe].path, szDir);
										DoExecute(action, SHOW_NO_OUTPUT);
										FillDir(1-curframe,SELECT_NOCHANGE);
										FillDir(  curframe,SELECT_NOCHANGE);
										SetSelected(curframe,szDir);
									}
								}
							}
							default:
								rccode = 0;
						}
					}
					break;
				case RC_8:
					if (tool[ACTION_DELETE-1] == ACTION_DELETE)
					{
						RenderMenuLine(ACTION_DELETE-1, YES);
						pfe = GetSelected(curframe);
						if (finfo[curframe].markcount > 0)
						{
							sprintf(szMessage,msg[MSG_DELETE_MULTI*NUM_LANG+language], finfo[curframe].markcount);
							if (MessageBox(szMessage,"",OKCANCEL) == YES)
							{
								for (pos = 0; pos < finfo[curframe].count; pos++)
								{
									if (IsMarked(curframe,pos))
									{
										pfe = getfileentry(curframe, pos);
										sprintf(szMessage,msg[MSG_DELETE_PROGRESS*NUM_LANG+language], pfe->name);
										MessageBox(szMessage,"",NOBUTTON);
										sprintf(action,"rm -f -r \"%s%s\"",finfo[curframe].path,pfe->name);
										DoExecute(action, SHOW_NO_OUTPUT);
									}
								}
								ClearMarker(curframe);
								FillDir(1-curframe,SELECT_NOCHANGE);
								FillDir(  curframe,SELECT_NOCHANGE);
							}
						}
						else
						{
							sprintf(szMessage,msg[MSG_DELETE*NUM_LANG+language], pfe->name);
							if (MessageBox(szMessage,"",OKCANCEL) == YES)
							{
								sprintf(szMessage,msg[MSG_DELETE_PROGRESS*NUM_LANG+language], pfe->name);
								MessageBox(szMessage,"",NOBUTTON);
								sprintf(action,"rm -f -r \"%s%s\"",finfo[curframe].path,pfe->name);
								DoExecute(action, SHOW_NO_OUTPUT);
								FillDir(1-curframe,SELECT_NOCHANGE);
								FillDir(  curframe,SELECT_NOCHANGE);
							}
						}
						rccode = 0;
					}
					break;
				case RC_9:
					if (tool[ACTION_MKFILE-1] == ACTION_MKFILE)
					{
						RenderMenuLine(ACTION_MKFILE-1, YES);
						char szDir[FILENAME_MAX];
						szDir[0] = 0x00;
						char szMsg[356];
						sprintf(szMsg,msg[MSG_MKFILE*NUM_LANG+language], finfo[curframe].path);
						switch (GetInputString(400,255,szDir,szMsg, NO))
						{
							case RC_OK:
							{
								if (*szDir != 0x00)
								{
									sprintf(action,"touch \"%s%s\"",finfo[curframe].path, szDir);
									DoExecute(action, SHOW_NO_OUTPUT);
									FillDir(1-curframe,SELECT_NOCHANGE);
									FillDir(  curframe,SELECT_NOCHANGE);
									SetSelected(curframe,szDir);
								}
							}
							default:
								rccode = 0;
						}
					}
					break;
				case RC_0:
					if (tool[ACTION_MKLINK-1] == ACTION_MKLINK)
					{
						RenderMenuLine(ACTION_MKLINK-1, YES);
						char szDir[FILENAME_MAX];
						pfe = GetSelected(curframe);
						strcpy(szDir,pfe->name);
						char szMsg[356];
						sprintf(szMsg,msg[MSG_MKLINK*NUM_LANG+language], finfo[curframe].path, pfe->name, finfo[1-curframe].path);
						switch (GetInputString(400,255,szDir,szMsg, NO))
						{
							case RC_OK:
							{
								if (*szDir != 0x00)
								{
									sprintf(action,"ln -s \"%s%s\" \"%s%s\"",finfo[curframe].path, pfe->name,finfo[1-curframe].path, szDir);
									DoExecute(action, SHOW_NO_OUTPUT);
									FillDir(1-curframe,SELECT_NOCHANGE);
									FillDir(  curframe,SELECT_NOCHANGE);
								}
							}
							default:
								rccode = 0;
						}
					}
					break;
				case RC_RED:
					{
						char szMsg[356];
						sprintf(szMsg,msg[MSG_COMMAND*NUM_LANG+language]);
						char* szCommand = (char*)malloc(commandsize);
					    	szCommand [0]= 0x00;
						switch (GetInputString(400,commandsize,szCommand,szMsg, NO))
						{
							case RC_OK:
								DoExecute(szCommand, SHOW_OUTPUT);
								FillDir(1-curframe,SELECT_NOCHANGE);
								FillDir(  curframe,SELECT_NOCHANGE);
								break;
							default:
								rccode = 0;
						}
						free (szCommand);
					}
					break;
				case RC_GREEN: // toggle marker
				    ToggleMarker(curframe);
					finfo[curframe].selected++;
				    break;
				case RC_YELLOW:
					cursort = finfo[curframe].sort = finfo[curframe].sort * -1;
					strcpy(szSel,GetSelected(curframe)->name);
					sortframe(curframe, szSel);
					break;
				case RC_BLUE: // Refresh
					FillDir(1-curframe,SELECT_NOCHANGE);
					FillDir(  curframe,SELECT_NOCHANGE);
					break;
				case RC_MUTE: // toggle visibility
					curvisibility++;
					if (curvisibility > 2) curvisibility = 0;
					break;
				case RC_DBOX: // main menu
					DoMainMenu();
					break;
				case RC_HOME:
					if (autosave == BTN_ASK)
					{
						switch (MessageBox(msg[MSG_SAVESETTINGS*NUM_LANG+language],"",YESNOCANCEL))
						{
							case YES:
								dosave = BTN_YES;
								while (GetRCCode(RC_NORMAL) == 0);
								rccode = RC_HOME;
								break;
							case NO:
								dosave = BTN_NO;
								while (GetRCCode(RC_NORMAL) == 0);
								rccode = RC_HOME;
								break;
							case CANCEL:
								dosave = BTN_NO;
								rccode = -1;
								break;
						}
					}
					break;

				default:
					continue;
		}
		if (finfo[curframe].selected  < 0)
			finfo[curframe].selected  = 0;
		if (finfo[curframe].selected >= finfo[curframe].count)
			finfo[curframe].selected  = finfo[curframe].count -1;
		if (finfo[curframe].first     > finfo[curframe].selected)
			finfo[curframe].first     = finfo[curframe].selected;
		if (finfo[curframe].selected >= finfo[curframe].first + framerows)
			finfo[curframe].first     = finfo[curframe].selected - framerows+1;
		RenderFrame(LEFTFRAME);
		RenderFrame(RIGHTFRAME);
		memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);

	}while(rccode != RC_HOME);

	if (dosave == BTN_YES)
		WriteSettings();

	system("rm -f /tmp/tuxcom.out");
	rccode = -1;
	FTC_Manager_Done(manager);
	FT_Done_FreeType(library);

	free(lbb);

	memset(lfb, 0, fix_screeninfo.smem_len);
	munmap(lfb, fix_screeninfo.smem_len);
	ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo_original);

	//restore videoformat
//	ioctl(avs, AVSIOSSCARTPIN8, &fnc_old);
//	ioctl(saa, SAAIOSWSS, &saa_old);
//	close(avs);
//	close(saa);

// 	fcntl(rc, F_SETFL, O_NONBLOCK);
	close(rc);
	close(kb);

	ClearEntries   (LEFTFRAME );
	ClearEntries   (RIGHTFRAME);
	ClearMarker    (LEFTFRAME );
	ClearMarker    (RIGHTFRAME);
	ClearZipEntries(LEFTFRAME );
	ClearZipEntries(RIGHTFRAME);
	return;
}

/******************************************************************************
 * RenderMenuLine                                                              *
 ******************************************************************************/

void RenderMenuLine(int highlight, int refresh)
{
	char szEntry[20];
	int i,j;
	RenderBox(menuitemwidth * MENUITEMS                 ,viewy-MENUSIZE, viewx, viewy-MENUSIZE / 2 , FILL, (highlight == MENUITEMS-1 ? GREEN : BLUE2) );
	for (i = 0; i < MENUITEMS; i++)
	{
		RenderBox(menuitemwidth * i                 ,viewy-MENUSIZE, menuitemwidth *(i+1)                 , viewy-MENUSIZE / 2 , FILL, (i == highlight ? GREEN : BLUE2) );
		RenderBox(menuitemwidth * i                 ,viewy-MENUSIZE, menuitemwidth * i   + menuitemnumber , viewy-MENUSIZE / 2 , FILL, BLUE1);

		sprintf(szEntry,"%d",(i+1)%MENUITEMS);
		RenderString(szEntry          , menuitemwidth * i +1              , viewy-(MENUSIZE/2 + FONT_OFFSET_BIG) , menuitemnumber              , CENTER, SMALL, WHITE);
		if (refresh == EDIT)
		{
			if (textuppercase == 0)
				RenderString(numberchars[(i+1)%10], menuitemwidth * i + menuitemnumber, viewy-(MENUSIZE/2 + FONT_OFFSET_BIG) , menuitemwidth-menuitemnumber, CENTER, SMALL, WHITE);
			else
			{
				strcpy(szEntry,numberchars[(i+1)%10]);
				for (j = 0; j < strlen(szEntry); j++) szEntry[j] = toupper(szEntry[j]);
				RenderString(szEntry, menuitemwidth * i + menuitemnumber, viewy-(MENUSIZE/2 + FONT_OFFSET_BIG) , menuitemwidth-menuitemnumber, CENTER, SMALL, WHITE);
			}

		}
		else if (refresh == EDITOR)
			RenderString(editorline[tool[i]*NUM_LANG+language], menuitemwidth * i + menuitemnumber, viewy-(MENUSIZE/2 + FONT_OFFSET_BIG) , menuitemwidth-menuitemnumber, CENTER, SMALL, WHITE);
		else
			RenderString(menuline[tool[i]*NUM_LANG+language], menuitemwidth * i + menuitemnumber, viewy-(MENUSIZE/2 + FONT_OFFSET_BIG) , menuitemwidth-menuitemnumber, CENTER, SMALL, WHITE);


	}
	RenderBox( viewx-COLORBUTTONS ,viewy-MENUSIZE/2, viewx , viewy , FILL,  BLUE1);
	RenderBox( 0,viewy- MENUSIZE , viewx , viewy-MENUSIZE / 2 , GRID, WHITE);

	for (i = 0; i < COLORBUTTONS; i++)
	{

		RenderBox( (viewx/COLORBUTTONS) *i ,viewy-MENUSIZE/2, (viewx/COLORBUTTONS) *(i+1) , viewy , FILL,  (i == 0 ? RED    :
		                                                            					                   (i == 1 ? GREEN  :
		                                                            					                   (i == 2 ? YELLOW : BLUE1))));
		RenderBox( (viewx/COLORBUTTONS) *i ,viewy-MENUSIZE/2, (i < COLORBUTTONS-1 ? (viewx/COLORBUTTONS) *(i+1) : viewx) , viewy , GRID,  WHITE );
		RenderString(colorline[colortool[i]*NUM_LANG+language], (viewx/COLORBUTTONS) *i , viewy- FONT_OFFSET_BIG , viewx/COLORBUTTONS, CENTER, SMALL  , (i == 2 ? BLACK : WHITE));
	}
	if (refresh == YES)
		memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);

}


/******************************************************************************
 * RenderFrame                                                                *
 ******************************************************************************/

void RenderFrame(int frame)
{

	int row = 0;
	int bcolor, fcolor;
	char sizeString[100];
	short bselected;
	struct fileentry* pfe;


	int nBackColor;

	colortool[0] = ACTION_EXEC   ;
	colortool[1] = ACTION_MARKER ;
	colortool[2] = ACTION_SORT   ;
	colortool[3] = ACTION_REFRESH;

	if (curvisibility == 0)
		nBackColor = (finfo[frame].writable ? BLUE1: BLACK );
	else
		nBackColor = trans_map[curvisibility];

	if (curframe == frame)
	{
		memset(tool, ACTION_NOACTION, sizeof(tool));
	}

	if (curframe == frame)
	{
		if (finfo[frame].writable)
		{
			tool[ACTION_MKDIR-1 ] = ACTION_MKDIR; // mkdir allowed
			tool[ACTION_MKFILE-1] = ACTION_MKFILE; // mkfile allowed
		}
	}
	while (row < framerows && (finfo[frame].first + row < finfo[frame].count))
	{
		bselected =  ((finfo[frame].first + row == finfo[frame].selected) && (curframe == frame));
		bcolor = (bselected ? (IsMarked(frame,finfo[frame].first + row) ? BLUE3 : BLUE2)
		                    : (IsMarked(frame,finfo[frame].first + row) ? trans_map_mark[curvisibility] : nBackColor));
		pfe = getfileentry(frame, finfo[frame].first + row);
		if (bselected && strcmp(pfe->name,"..") != 0 && strcmp(pfe->name,"/") != 0)
		{
		    tool[ACTION_PROPS-1] = ACTION_PROPS; // view properties allowed if entry is not .. or /
		    if (finfo[1-frame].writable && finfo[frame].zipfile[0] == 0x00)
			tool[ACTION_MKLINK-1] = ACTION_MKLINK; // mklink allowed
		}
		*sizeString = 0x00;
		fcolor = WHITE;
		if ((pfe->fentry.st_mode & S_IRUSR) == S_IRUSR )
		{
			fcolor = GREEN2 ;
			if (bselected)
			{
				tool[ACTION_COPY-1] = (finfo[1-frame].writable  ? ACTION_COPY : ACTION_NOACTION); // copy allowed, if other frame writable;
				tool[ACTION_VIEW-1] = (finfo[frame].ftpconn != NULL ? ACTION_NOACTION : ACTION_VIEW); // view allowed, if not in FTP-Connection
			}
		}
		if ((pfe->fentry.st_mode & S_IWUSR) == S_IWUSR )
		{
			fcolor = GRAY   ;
			if (bselected)
			{
				tool[ACTION_MOVE-1] = (finfo[1-frame].writable && finfo[frame].writable ? ACTION_MOVE   : ACTION_NOACTION); // move   allowed, if both frames writable;
				tool[ACTION_DELETE-1] = (finfo[  frame].writable ? ACTION_DELETE : ACTION_NOACTION); // delete allowed, if current frame writable
				tool[ACTION_RENAME-1] = (finfo[  frame].writable ? ACTION_RENAME : ACTION_NOACTION); // rename allowed, if current frame writable
				tool[ACTION_EDIT-1] = ((pfe->fentry.st_size < FILEBUFFER_SIZE) && finfo[frame].writable ?  ACTION_EDIT : ACTION_NOACTION); // edit allowed, if size of current file < FILEBUFFER_SIZE;
			}
		}
		if      ((pfe->fentry.st_mode & S_IXUSR) == S_IXUSR )
		{
			fcolor = YELLOW ;
		}
		if     (S_ISDIR(pfe->fentry.st_mode))
		{
			fcolor = WHITE  ;
			sprintf(sizeString,"<DIR>");
			if (bselected)
			{
				tool[ACTION_VIEW-1] = ACTION_NOACTION; // view not allowed
				tool[ACTION_EDIT-1] = ACTION_NOACTION; // edit not allowed
			}
		}
		else if (S_ISLNK(pfe->fentry.st_mode))
		{
			fcolor = ORANGE ;
			sprintf(sizeString,"<LINK>");
			if (bselected)
				tool[ACTION_VIEW-1] = ACTION_NOACTION; // view not allowed
		}
		else
		{
			GetSizeString(sizeString,pfe->fentry.st_size);
		}
		if (bselected)
		{
			if (finfo[frame].markcount > 0) // files marked in current frame
			{
				tool[ACTION_COPY  -1] = (finfo[1-frame].writable  ? ACTION_COPY : ACTION_NOACTION); // copy allowed, if other frame writable;
				tool[ACTION_MOVE  -1] = (finfo[1-frame].writable && finfo[frame].writable ? ACTION_MOVE   : ACTION_NOACTION); // move   allowed, if both frames writable;
				tool[ACTION_DELETE-1] = (finfo[  frame].writable ? ACTION_DELETE : ACTION_NOACTION); // delete allowed, if current frame writable
			}
			RenderMenuLine(-1, NO);
		}

		PosY = (row+1) * FONTHEIGHT_SMALL + BORDERSIZE ;
		PosX = frame * FrameWidth + BORDERSIZE;

		RenderBox(PosX, PosY-FONTHEIGHT_SMALL,((1+frame)*FrameWidth), PosY, FILL, bcolor);
		RenderString(pfe->name, PosX+2, PosY-FONT_OFFSET, NameWidth-2, LEFT, SMALL,fcolor);
		RenderString(sizeString, (1+frame)*FrameWidth -2*BORDERSIZE - 2*SizeWidth, PosY-FONT_OFFSET, 2*SizeWidth, RIGHT, SMALL,fcolor);
		row++;
	}
	// fill empty rows
	RenderBox(PosX, PosY,((1+frame)*FrameWidth), PosY+ FONTHEIGHT_SMALL*(framerows-row+1) , FILL, nBackColor);

	// draw Rectangle
	RenderBox(   frame *FrameWidth              , 0                            ,    frame *FrameWidth          +BORDERSIZE, viewy-MENUSIZE                              , FILL, (curframe == frame ? WHITE : BLUE2));
	RenderBox((1+frame)*FrameWidth -BORDERSIZE  , 0                            , (1+frame)*FrameWidth                     , viewy-MENUSIZE                              , FILL, (curframe == frame ? WHITE : BLUE2));
	RenderBox(   frame *FrameWidth              , 0                            , (1+frame)*FrameWidth                     , BORDERSIZE                                  , FILL, (curframe == frame ? WHITE : BLUE2));
	RenderBox(   frame *FrameWidth              , viewy-2*BORDERSIZE-FONTHEIGHT_SMALL - MENUSIZE, (1+frame)*FrameWidth    , viewy-BORDERSIZE-FONTHEIGHT_SMALL - MENUSIZE, FILL, (curframe == frame ? WHITE : BLUE2));
	RenderBox(   frame *FrameWidth +NameWidth   , 0                            ,    frame *FrameWidth+NameWidth+BORDERSIZE, viewy-BORDERSIZE-FONTHEIGHT_SMALL - MENUSIZE, FILL, (curframe == frame ? WHITE : BLUE2));
	RenderBox(   frame *FrameWidth              , viewy-BORDERSIZE - MENUSIZE  , (1+frame)*FrameWidth                     , viewy-MENUSIZE                              , FILL, (curframe == frame ? WHITE : BLUE2));

	// Info line
	RenderBox(PosX   , viewy-BORDERSIZE-FONTHEIGHT_SMALL-MENUSIZE, PosX+FrameWidth-2*BORDERSIZE, viewy-BORDERSIZE-MENUSIZE, FILL, BLACK);
	if (finfo[frame].markcount > 0)
	{
		sprintf(sizeString,info[INFO_MARKER*NUM_LANG+language],finfo[frame].markcount);
		RenderString(sizeString, PosX+2, viewy-BORDERSIZE-FONT_OFFSET-MENUSIZE , NameWidth-2, LEFT, SMALL,(curframe == frame ? WHITE : BLUE2));
		GetSizeString(sizeString,finfo[frame].marksize);
	}
	else
	{
		RenderString(finfo[frame].zipfile[0] != 0x00 ? finfo[frame].zipfile : finfo[frame].path, PosX+2, viewy-BORDERSIZE-FONT_OFFSET-MENUSIZE , NameWidth-2, LEFT, SMALL,(curframe == frame ? WHITE : BLUE2));
		GetSizeString(sizeString,finfo[frame].size);
	}
	RenderString(sizeString, (1+frame)*FrameWidth -BORDERSIZE - 2*SizeWidth, viewy-BORDERSIZE-FONT_OFFSET-MENUSIZE , 2*SizeWidth, RIGHT, SMALL,(curframe == frame ? WHITE : BLUE2));
}

/******************************************************************************
 * MessageBox                                                                 *
 ******************************************************************************/

int MessageBox(const char* msg1, const char* msg2, int mode)
{

	int sel = 0, le1=0, le2=0 , wi, he, maxsel=0;
	int ps[5];

	switch (mode)
	{
		case OKCANCEL:
			ps[0] = YES;
			ps[1] = CANCEL;
			ps[2] = 0;
			ps[3] = 0;
			ps[4] = 0;
			sel = 1;
			maxsel = 1;
			break;
		case OKHIDDENCANCEL:
			ps[0] = YES;
			ps[1] = CANCEL;
			ps[2] = HIDDEN;
			ps[3] = 0;
			ps[4] = 0;
			sel = 1;
			maxsel = 2;
			break;
		case YESNOCANCEL:
			ps[0] = YES;
			ps[1] = CANCEL;
			ps[2] = NO;
			ps[3] = 0;
			ps[4] = 0;
			sel = 1;
			maxsel = 2;
			break;
		case OVERWRITECANCEL:
			ps[0] = OVERWRITE;
			ps[1] = CANCEL;
			ps[2] = RENAME;
			ps[3] = 0;
			ps[4] = 0;
			sel = 1;
			maxsel = 2;
			break;
		case OVERWRITESKIPCANCEL:
			ps[0] = OVERWRITE;
			ps[1] = CANCEL;
			ps[2] = SKIP;
			ps[3] = OVERWRITEALL;
			ps[4] = SKIPALL;
			sel = 1;
			maxsel = 4;
			break;
		case CANCELRUN:
			ps[0] = CANCEL;
			ps[1] = 0;
			ps[2] = 0;
			ps[3] = 0;
			ps[4] = 0;
			sel = 1;
			maxsel = 1;
			break;
	}

	le1 = GetStringLen(msg1, BIG);
	le2 = GetStringLen(msg2, BIG);
	wi = MINBOX;
	if (le1 > wi ) wi = le1;
	if (le2 > wi ) wi = le2;
	if (wi > viewx - 6* BORDERSIZE) wi = viewx - 6* BORDERSIZE;

	he = 4* BORDERSIZE+ BUTTONHEIGHT + (*msg2 == 0x00 ?  1 : 2) * FONTHEIGHT_BIG + (maxsel > 2 ? BORDERSIZE+BUTTONHEIGHT : 0);


	RenderBox((viewx-wi)/2 - 2*BORDERSIZE, (viewy-he) /2, viewx-(viewx-wi)/2+ 2*BORDERSIZE, viewy-(viewy-he)/2, FILL, trans_map[curvisibility]);
	RenderBox((viewx-wi)/2 - 2*BORDERSIZE, (viewy-he) /2, viewx-(viewx-wi)/2+ 2*BORDERSIZE, viewy-(viewy-he)/2, GRID, WHITE);
	RenderString(msg1,(viewx-wi)/2-BORDERSIZE , (viewy-he)/2 + BORDERSIZE + FONTHEIGHT_BIG-FONT_OFFSET , wi+2*BORDERSIZE, CENTER, BIG, WHITE);
	if (le2 > 0)
		RenderString(msg2,(viewx-wi)/2-BORDERSIZE , (viewy-he)/2 + BORDERSIZE + 2*FONTHEIGHT_BIG-FONT_OFFSET , wi+2*BORDERSIZE, CENTER, BIG, WHITE);


	RenderButtons(he, mode);
	if (mode == NOBUTTON) return 0;
	int drawsel = 0;
	do{
		if ((GetRCCode(RC_NORMAL) ==0) && mode == CANCELRUN) return NO;
		switch(rccode)
		{
				case RC_OK:
					rccode = -1;
					return ps[sel];
				case RC_LEFT:
					sel--;
					if (sel < 0 ) sel = 0;
					drawsel = 1;
					break;
				case RC_RIGHT:
					sel++;
					if (sel > maxsel) sel = maxsel;
					drawsel = 1;
					break;
				case RC_UP:
				    if (sel > 2) sel = 1;
					drawsel = 1;
					break;
				case RC_DOWN:
				    if (sel < 3) sel = 3;
					drawsel = 1;
					break;
				case RC_RED:
					rccode = -1;
					return ps[0];
				case RC_GREEN:
				case RC_HOME:
					rccode = -1;
					return ps[1];
				case RC_YELLOW:
					rccode = -1;
					if (maxsel > 1)
						return ps[2];
				default:
					continue;
		}
		if (drawsel)
		{
			switch(maxsel)
			{
				case 1:
					RenderBox(viewx/2 - 2* BORDERSIZE -BUTTONWIDTH  , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 - 2* BORDERSIZE                ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, (sel == 0 ? WHITE : RED  ));
					RenderBox(viewx/2 - 2* BORDERSIZE -BUTTONWIDTH+1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 - 2* BORDERSIZE              -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, (sel == 0 ? WHITE : RED  ));

					RenderBox(viewx/2 + 2* BORDERSIZE               , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 + 2* BORDERSIZE +BUTTONWIDTH   ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, (sel == 1 ? WHITE : GREEN));
					RenderBox(viewx/2 + 2* BORDERSIZE             +1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 + 2* BORDERSIZE +BUTTONWIDTH -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, (sel == 1 ? WHITE : GREEN));
					memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
					break;
				case 2:
					RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 - 4* BORDERSIZE  - BUTTONWIDTH/2              ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, (sel == 0 ? WHITE : RED  ));
					RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2+1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 - 4* BORDERSIZE  - BUTTONWIDTH/2            -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, (sel == 0 ? WHITE : RED  ));

					RenderBox(viewx/2 - BUTTONWIDTH/2                                , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 + BUTTONWIDTH/2                               ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, (sel == 1 ? WHITE : GREEN ));
					RenderBox(viewx/2 - BUTTONWIDTH/2                              +1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 + BUTTONWIDTH/2                             -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, (sel == 1 ? WHITE : GREEN ));

					RenderBox(viewx/2 + 4* BORDERSIZE + BUTTONWIDTH/2                , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2  ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, (sel == 2 ? BLACK : YELLOW ));
					RenderBox(viewx/2 + 4* BORDERSIZE + BUTTONWIDTH/2              +1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2-1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, (sel == 2 ? BLACK : YELLOW ));
					memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
					break;
				case 4:
					RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2  , viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT  , viewx/2 - 4* BORDERSIZE  - BUTTONWIDTH/2              ,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT  , GRID, (sel == 0 ? WHITE : RED    ));
					RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2+1, viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT+1, viewx/2 - 4* BORDERSIZE  - BUTTONWIDTH/2            -1,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT-1, GRID, (sel == 0 ? WHITE : RED    ));

					RenderBox(viewx/2 - BUTTONWIDTH/2                                , viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT  , viewx/2 + BUTTONWIDTH/2                               ,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT  , GRID, (sel == 1 ? WHITE : GREEN  ));
					RenderBox(viewx/2 - BUTTONWIDTH/2                              +1, viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT+1, viewx/2 + BUTTONWIDTH/2                             -1,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT-1, GRID, (sel == 1 ? WHITE : GREEN  ));

 					RenderBox(viewx/2 + 4* BORDERSIZE + BUTTONWIDTH/2                , viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT  , viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2  ,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT  , GRID, (sel == 2 ? BLACK : YELLOW ));
 					RenderBox(viewx/2 + 4* BORDERSIZE + BUTTONWIDTH/2              +1, viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT+1, viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2-1,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT-1, GRID, (sel == 2 ? BLACK : YELLOW ));

					RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE -   BUTTONHEIGHT  , viewx/2 - 2* BORDERSIZE                               ,viewy-(viewy-he)/2- 2* BORDERSIZE                 , GRID, (sel == 3 ? WHITE : BLUE2  ));
					RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2+1, viewy-(viewy-he)/2 - 2*BORDERSIZE -   BUTTONHEIGHT+1, viewx/2 - 2* BORDERSIZE                             -1,viewy-(viewy-he)/2- 2* BORDERSIZE               -1, GRID, (sel == 3 ? WHITE : BLUE2  ));

					RenderBox(viewx/2 + 2* BORDERSIZE                                , viewy-(viewy-he)/2 - 2*BORDERSIZE -   BUTTONHEIGHT  , viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2  ,viewy-(viewy-he)/2- 2* BORDERSIZE                 , GRID, (sel == 4 ? WHITE : BLUE2  ));
					RenderBox(viewx/2 + 2* BORDERSIZE                              +1, viewy-(viewy-he)/2 - 2*BORDERSIZE -   BUTTONHEIGHT+1, viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2-1,viewy-(viewy-he)/2- 2* BORDERSIZE               -1, GRID, (sel == 4 ? WHITE : BLUE2  ));
					memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
					break;
			}
			drawsel = 0;
		}

	}while(1);
	rccode = -1;
	return sel;

}
/******************************************************************************
 * RenderButtons                                                              *
 ******************************************************************************/

void RenderButtons(int he, int mode)
{
	switch(mode)
	{
		case OKCANCEL:
			RenderBox(viewx/2 - 2* BORDERSIZE -BUTTONWIDTH , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 - 2* BORDERSIZE              ,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, RED  );
			RenderBox(viewx/2 + 2* BORDERSIZE              , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 + 2* BORDERSIZE +BUTTONWIDTH ,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, GREEN);
			RenderString(mbox[BTN_OK    *NUM_LANG+language],viewx/2 - 2* BORDERSIZE -BUTTONWIDTH , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_CANCEL*NUM_LANG+language],viewx/2 + 2* BORDERSIZE              , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderBox(viewx/2 + 2* BORDERSIZE                , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 + 2* BORDERSIZE +BUTTONWIDTH   ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, WHITE);
			RenderBox(viewx/2 + 2* BORDERSIZE              +1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 + 2* BORDERSIZE +BUTTONWIDTH -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, WHITE);
			break;
		case OKHIDDENCANCEL:
			RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 - 4* BORDERSIZE - BUTTONWIDTH/2             ,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, RED   );
			RenderBox(viewx/2 - BUTTONWIDTH/2                              , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 + BUTTONWIDTH/2                             ,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, GREEN );
			RenderBox(viewx/2 + 4* BORDERSIZE + BUTTONWIDTH/2              , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, YELLOW);
			RenderString(mbox[BTN_OK    *NUM_LANG+language],viewx/2 - 4* BORDERSIZE -BUTTONWIDTH - BUTTONWIDTH/2 , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_CANCEL*NUM_LANG+language],(viewx-BUTTONWIDTH)/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_HIDDEN*NUM_LANG+language],viewx/2 + 4* BORDERSIZE +BUTTONWIDTH/2            , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, BLACK);
			RenderBox(viewx/2 -BUTTONWIDTH/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 + BUTTONWIDTH/2   ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, WHITE);
			RenderBox(viewx/2 -BUTTONWIDTH/2+1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 + BUTTONWIDTH/2 -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, WHITE);
			break;
		case YESNOCANCEL:
			RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 - 4* BORDERSIZE - BUTTONWIDTH/2             ,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, RED   );
			RenderBox(viewx/2 - BUTTONWIDTH/2                              , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 + BUTTONWIDTH/2                             ,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, GREEN );
			RenderBox(viewx/2 + 4* BORDERSIZE + BUTTONWIDTH/2              , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, YELLOW);
			RenderString(mbox[BTN_YES   *NUM_LANG+language],viewx/2 - 4* BORDERSIZE -BUTTONWIDTH - BUTTONWIDTH/2 , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_CANCEL*NUM_LANG+language],(viewx-BUTTONWIDTH)/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_NO    *NUM_LANG+language],viewx/2 + 4* BORDERSIZE +BUTTONWIDTH/2            , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, BLACK);
			RenderBox(viewx/2 -BUTTONWIDTH/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 + BUTTONWIDTH/2   ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, WHITE);
			RenderBox(viewx/2 -BUTTONWIDTH/2+1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 + BUTTONWIDTH/2 -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, WHITE);
			break;
		case OVERWRITECANCEL:
			RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 - 4* BORDERSIZE - BUTTONWIDTH/2             ,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, RED   );
			RenderBox(viewx/2 - BUTTONWIDTH/2                              , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 + BUTTONWIDTH/2                             ,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, GREEN );
			RenderBox(viewx/2 + 4* BORDERSIZE + BUTTONWIDTH/2              , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2,viewy-(viewy-he)/2- 2* BORDERSIZE, FILL, YELLOW);
			RenderString(mbox[BTN_OVERWRITE*NUM_LANG+language],viewx/2 - 4* BORDERSIZE -BUTTONWIDTH - BUTTONWIDTH/2 , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_CANCEL   *NUM_LANG+language],(viewx-BUTTONWIDTH)/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_RENAME   *NUM_LANG+language],viewx/2 + 4* BORDERSIZE +BUTTONWIDTH/2            , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, BLACK);
			RenderBox(viewx/2 -BUTTONWIDTH/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 + BUTTONWIDTH/2   ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, WHITE);
			RenderBox(viewx/2 -BUTTONWIDTH/2+1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 + BUTTONWIDTH/2 -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, WHITE);
			break;
		case OVERWRITESKIPCANCEL:
			RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2, viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT, viewx/2 - 4* BORDERSIZE  - BUTTONWIDTH/2            ,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT, FILL, RED    );
			RenderBox(viewx/2 - BUTTONWIDTH/2                              , viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT, viewx/2 + BUTTONWIDTH/2                             ,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT, FILL, GREEN  );
			RenderBox(viewx/2 + 4* BORDERSIZE + BUTTONWIDTH/2              , viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT, viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT, FILL, YELLOW );
			RenderBox(viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2, viewy-(viewy-he)/2 - 2*BORDERSIZE -   BUTTONHEIGHT, viewx/2 - 2* BORDERSIZE                             ,viewy-(viewy-he)/2- 2* BORDERSIZE               , FILL, BLUE2  );
			RenderBox(viewx/2 + 2* BORDERSIZE                              , viewy-(viewy-he)/2 - 2*BORDERSIZE -   BUTTONHEIGHT, viewx/2 + 4* BORDERSIZE +BUTTONWIDTH + BUTTONWIDTH/2,viewy-(viewy-he)/2- 2* BORDERSIZE               , FILL, BLUE2  );
			RenderString(mbox[BTN_OVERWRITE   *NUM_LANG+language],viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2, viewy-(viewy-he)/2 - 4*BORDERSIZE-FONT_OFFSET-BUTTONHEIGHT , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_CANCEL      *NUM_LANG+language],(viewx-BUTTONWIDTH)/2                                , viewy-(viewy-he)/2 - 4*BORDERSIZE-FONT_OFFSET-BUTTONHEIGHT , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_SKIP        *NUM_LANG+language],viewx/2 + 4* BORDERSIZE +BUTTONWIDTH/2               , viewy-(viewy-he)/2 - 4*BORDERSIZE-FONT_OFFSET-BUTTONHEIGHT , BUTTONWIDTH, CENTER, BIG, BLACK);
			RenderString(mbox[BTN_OVERWRITEALL*NUM_LANG+language],viewx/2 - 4* BORDERSIZE - BUTTONWIDTH - BUTTONWIDTH/2, viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET              , BUTTONWIDTH + BUTTONWIDTH/2 + 2* BORDERSIZE, CENTER, BIG, WHITE);
			RenderString(mbox[BTN_SKIPALL     *NUM_LANG+language],viewx/2 + 2* BORDERSIZE                              , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET              , BUTTONWIDTH + BUTTONWIDTH/2 + 2* BORDERSIZE, CENTER, BIG, WHITE);
			RenderBox(viewx/2 -BUTTONWIDTH/2  , viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT  , viewx/2 + BUTTONWIDTH/2  ,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT  , GRID, WHITE);
			RenderBox(viewx/2 -BUTTONWIDTH/2+1, viewy-(viewy-he)/2 - 4*BORDERSIZE - 2*BUTTONHEIGHT+1, viewx/2 + BUTTONWIDTH/2-1,viewy-(viewy-he)/2- 4* BORDERSIZE - BUTTONHEIGHT-1, GRID, WHITE);
			break;
		case CANCELRUN:
			RenderBox((viewx-BUTTONWIDTH)/2 , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx - (viewx-BUTTONWIDTH)/2,viewy-(viewy-he)/2 - 2*BORDERSIZE , FILL, RED  );
			RenderString(mbox[BTN_CANCEL*NUM_LANG+language],(viewx-BUTTONWIDTH)/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderBox((viewx-BUTTONWIDTH)/2 , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx - (viewx-BUTTONWIDTH)/2,viewy-(viewy-he)/2 - 2*BORDERSIZE , GRID, WHITE);
		case NOBUTTON:
		    break;
		default:
			RenderBox((viewx-BUTTONWIDTH)/2 , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx - (viewx-BUTTONWIDTH)/2,viewy-(viewy-he)/2 - 2*BORDERSIZE , FILL, RED  );
			RenderString(mbox[BTN_OK*NUM_LANG+language],(viewx-BUTTONWIDTH)/2  , viewy-(viewy-he)/2 - 2*BORDERSIZE-FONT_OFFSET , BUTTONWIDTH, CENTER, BIG, WHITE);
			RenderBox((viewx-BUTTONWIDTH)/2 , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT, viewx - (viewx-BUTTONWIDTH)/2,viewy-(viewy-he)/2 - 2*BORDERSIZE , GRID, WHITE);
			break;
	}
	memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
}

/******************************************************************************
 * ShowProperties                                                             *
 ******************************************************************************/

int ShowProperties()
{
	struct fileentry *pfe = GetSelected(curframe);

	int sel = NO, pos = -1, mode, i, le1, wi , he = 10 * BORDERSIZE + BUTTONHEIGHT + 7 * FONTHEIGHT_BIG;
	int ri[3];
	char action[FILENAME_MAX];

	ri[0] =  ((pfe->fentry.st_mode & S_IRUSR) == S_IRUSR ? 1 : 0);
	ri[1] =  ((pfe->fentry.st_mode & S_IWUSR) == S_IWUSR ? 1 : 0);
	ri[2] =  ((pfe->fentry.st_mode & S_IXUSR) == S_IXUSR ? 1 : 0);

	le1 = GetStringLen(pfe->name, BIG);
	wi = 400;
	if (le1 + 4*BORDERSIZE > wi  ) wi = le1 + 4*BORDERSIZE;
	if (wi > viewx - 4* BORDERSIZE) wi = viewx - 4* BORDERSIZE;

	mode  =  (finfo[curframe].writable ? OKCANCEL : OK);
	RenderBox((viewx-wi)/2 , (viewy-he) /2, viewx-(viewx-wi)/2, viewy-(viewy-he)/2, FILL, trans_map[curvisibility]);
	RenderBox((viewx-wi)/2 , (viewy-he) /2, viewx-(viewx-wi)/2, viewy-(viewy-he)/2, GRID, WHITE);
	RenderString(pfe->name,(viewx-wi)/2+  2* BORDERSIZE , (viewy-he)/2 + 2*BORDERSIZE + FONTHEIGHT_BIG-FONT_OFFSET , wi, CENTER, BIG, WHITE);

	RenderString(info[INFO_ACCESSED*NUM_LANG+language],(viewx-wi)/2+ 3* BORDERSIZE , (viewy-he)/2 + 6*BORDERSIZE + (2)*FONTHEIGHT_BIG-FONT_OFFSET , wi, LEFT, BIG, WHITE);
	RenderString(info[INFO_MODIFIED*NUM_LANG+language],(viewx-wi)/2+ 3* BORDERSIZE , (viewy-he)/2 + 6*BORDERSIZE + (3)*FONTHEIGHT_BIG-FONT_OFFSET , wi, LEFT, BIG, WHITE);
	RenderString(info[INFO_CREATED *NUM_LANG+language],(viewx-wi)/2+ 3* BORDERSIZE , (viewy-he)/2 + 6*BORDERSIZE + (4)*FONTHEIGHT_BIG-FONT_OFFSET , wi, LEFT, BIG, WHITE);
	char tm[100];
	strftime(tm,100,info[INFO_DATETIME *NUM_LANG+language],localtime(&pfe->fentry.st_atime));
	RenderString(tm,viewx/2- 2* BORDERSIZE , (viewy-he)/2 + 6*BORDERSIZE + (2)*FONTHEIGHT_BIG-FONT_OFFSET , wi/2, RIGHT, BIG, WHITE);
	strftime(tm,100,info[INFO_DATETIME *NUM_LANG+language],localtime(&pfe->fentry.st_mtime));
	RenderString(tm,viewx/2- 2* BORDERSIZE , (viewy-he)/2 + 6*BORDERSIZE + (3)*FONTHEIGHT_BIG-FONT_OFFSET , wi/2, RIGHT, BIG, WHITE);
	strftime(tm,100,info[INFO_DATETIME *NUM_LANG+language],localtime(&pfe->fentry.st_ctime));
	RenderString(tm,viewx/2- 2* BORDERSIZE , (viewy-he)/2 + 6*BORDERSIZE + (4)*FONTHEIGHT_BIG-FONT_OFFSET , wi/2, RIGHT, BIG, WHITE);

	for (i = 0; i < 3 ; i++)
	{
		RenderString(props[i*NUM_LANG+language],(viewx-wi)/2+ 3* BORDERSIZE , (viewy-he)/2 + 6*BORDERSIZE + (i+5)*FONTHEIGHT_BIG-FONT_OFFSET , wi, LEFT, BIG, WHITE);
		RenderBox(viewx-(viewx-wi)/2 - 2* BORDERSIZE - FONTHEIGHT_BIG+2, (viewy-he)/2 + 6*BORDERSIZE + (i+4)*FONTHEIGHT_BIG+2, viewx-(viewx-wi)/2 - 2*BORDERSIZE-2, (viewy-he)/2 + 6*BORDERSIZE + (i+5)*FONTHEIGHT_BIG-2, FILL, (ri[i] == 0 ? RED : GREEN));
		RenderBox(      (viewx-wi)/2 + 2* BORDERSIZE                 +1, (viewy-he)/2 + 6*BORDERSIZE + (i+4)*FONTHEIGHT_BIG+1, viewx-(viewx-wi)/2 - 2*BORDERSIZE-1, (viewy-he)/2 + 6*BORDERSIZE + (i+5)*FONTHEIGHT_BIG-1, GRID, (pos == i ? WHITE :trans_map[curvisibility]));
		RenderBox(      (viewx-wi)/2 + 2* BORDERSIZE                 +2, (viewy-he)/2 + 6*BORDERSIZE + (i+4)*FONTHEIGHT_BIG+2, viewx-(viewx-wi)/2 - 2*BORDERSIZE-2, (viewy-he)/2 + 6*BORDERSIZE + (i+5)*FONTHEIGHT_BIG-2, GRID, (pos == i ? WHITE :trans_map[curvisibility]));
	}
	RenderButtons(he,mode);
	int drawsel = 0;
	do{
		GetRCCode(RC_NORMAL);
		switch(rccode)
		{
				case RC_OK:
					if (sel == NO) return NO;
					if (sel == YES)
					{
						if(!finfo[curframe].writable) return NO;
						int m = (ri[0] << 2) | (ri[1] << 1) | ri[2];
						sprintf(action,"chmod -R %d%d%d \"%s%s\"",m,m,m, finfo[curframe].path, pfe->name);
						DoExecute(action, SHOW_NO_OUTPUT);
						rccode = -1;
						return YES;
					}
					if (pos != -1)
					{
						ri[pos] = 1- ri[pos];
						drawsel = 1;
					}
					break;
				case RC_LEFT:
					if (mode == OKCANCEL)
					{
						sel = YES;
						drawsel = 1;
					}
					break;
				case RC_RIGHT:
					if (mode == OKCANCEL)
					{
						sel = NO;
						drawsel = 1;
					}
					break;
				case RC_UP:
					if (mode == OKCANCEL)
					{
						if (sel != -1)
						{
							pos = 2;
						}
						else
						{
							if (pos > 0) pos--;
						}
						sel = -1;
						drawsel = 1;
					}
					break;
				case RC_DOWN:
					if (mode == OKCANCEL)
					{
						pos++;
						if (pos == 3)
							sel = YES;
						if (pos >= 4)
						{
							pos = 4;
							sel = NO;
						}
						drawsel = 1;

					}
					break;
				case RC_RED:
					rccode = -1;
					if(!finfo[curframe].writable) return NO;
					int m = (ri[0] << 2) | (ri[1] << 1) | ri[2];
					sprintf(action,"chmod -R %d%d%d \"%s%s\"",m,m,m, finfo[curframe].path, pfe->name);
					DoExecute(action, SHOW_NO_OUTPUT);
					rccode = -1;
					return YES;
				case RC_GREEN:
				case RC_HOME:
					rccode = -1;
					return NO;
				default:
					continue;
		}
		if (drawsel)
		{
			for (i = 0; i < 3 ; i++)
			{
				RenderBox(viewx-(viewx-wi)/2 - 2* BORDERSIZE - FONTHEIGHT_BIG+2, (viewy-he)/2 + 6*BORDERSIZE + (i+4)*FONTHEIGHT_BIG+2, viewx-(viewx-wi)/2 - 2*BORDERSIZE-2, (viewy-he)/2 + 6*BORDERSIZE + (i+5)*FONTHEIGHT_BIG-2, FILL, (ri[i] == 0 ? RED : GREEN));
				RenderBox(      (viewx-wi)/2 + 2* BORDERSIZE                 +2, (viewy-he)/2 + 6*BORDERSIZE + (i+4)*FONTHEIGHT_BIG+2, viewx-(viewx-wi)/2 - 2*BORDERSIZE-2, (viewy-he)/2 + 6*BORDERSIZE + (i+5)*FONTHEIGHT_BIG-2, GRID, (pos == i ? WHITE :trans_map[curvisibility]));
				RenderBox(      (viewx-wi)/2 + 2* BORDERSIZE                 +1, (viewy-he)/2 + 6*BORDERSIZE + (i+4)*FONTHEIGHT_BIG+1, viewx-(viewx-wi)/2 - 2*BORDERSIZE-1, (viewy-he)/2 + 6*BORDERSIZE + (i+5)*FONTHEIGHT_BIG-1, GRID, (pos == i ? WHITE :trans_map[curvisibility]));
			}
			RenderBox(viewx/2 - 2* BORDERSIZE -BUTTONWIDTH  , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 - 2* BORDERSIZE               ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, (sel == YES ? WHITE : RED  ));
			RenderBox(viewx/2 - 2* BORDERSIZE -BUTTONWIDTH+1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 - 2* BORDERSIZE             -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, (sel == YES ? WHITE : RED  ));
			RenderBox(viewx/2 + 2* BORDERSIZE               , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 + 2* BORDERSIZE +BUTTONWIDTH  ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, (sel == NO ? WHITE : GREEN));
			RenderBox(viewx/2 + 2* BORDERSIZE             +1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 + 2* BORDERSIZE +BUTTONWIDTH-1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, (sel == NO ? WHITE : GREEN));
			memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
			drawsel = 0;
		}

	}while(1);
	rccode = -1;
	return sel;

}

/******************************************************************************
 * DoEditFTP                                                                  *
 ******************************************************************************/

void DoEditFTP(char* szFile,char* szTitle)
{
	FILE *fp;
	char *p, *p1;
	char entries[5][FILENAME_MAX];
	char line[FILENAME_MAX];

	memset(entries,0,5*FILENAME_MAX);
	fp = fopen( szFile, "r" );
	if ( !fp )
	{
		printf("tuxcom: could not open ftpfile\n");
	}
	else
	{
		while( fgets( line, 128, fp ) )
		{
			if ( *line == '#' )	continue;
			if ( *line == ';' )	continue;
			p=strchr(line,'\n');
			if ( p )
				*p=0;
			p=strchr(line,'=');
			if ( !p )
				continue;
			*p=0;
			p++;
			p1=strchr(p,'\r'); // für Windows-Nutzer: '\r' überlesen
			if (p1 != NULL) *p1 = 0x00;
			if      ( !strcmp(line,"host") ) strcpy(entries[0], p);
			else if ( !strcmp(line,"port") ) strcpy(entries[1], p);
			else if ( !strcmp(line,"user") ) strcpy(entries[2], p);
			else if ( !strcmp(line,"pass") ) strcpy(entries[3], p);
			else if ( !strcmp(line,"dir" ) ) strcpy(entries[4], p);
		}
		fclose(fp);
	}


	int end = NO,sel = NO, pos = -1, i, le1, wi , he = 8 * BORDERSIZE + BUTTONHEIGHT + 6 * FONTHEIGHT_BIG;


	le1 = GetStringLen(szTitle, BIG);
	wi = 500;
	if (le1 + 4*BORDERSIZE > wi  ) wi = le1 + 4*BORDERSIZE;
	if (wi > viewx - 4* BORDERSIZE) wi = viewx - 4* BORDERSIZE;

	RenderBox((viewx-wi)/2 , (viewy-he) /2, viewx-(viewx-wi)/2, viewy-(viewy-he)/2, FILL, trans_map[curvisibility]);
	RenderBox((viewx-wi)/2 , (viewy-he) /2, viewx-(viewx-wi)/2, viewy-(viewy-he)/2, GRID, WHITE);
	RenderString(szFile,(viewx-wi)/2+  2* BORDERSIZE , (viewy-he)/2 + 2*BORDERSIZE + FONTHEIGHT_BIG-FONT_OFFSET , wi, CENTER, BIG, WHITE);

	for (i = 0; i < 5 ; i++)
	{
		RenderString(ftpstr[i*NUM_LANG+language],(viewx-wi)/2+ 3* BORDERSIZE , (viewy-he)/2 + 3*BORDERSIZE + (i+2)*FONTHEIGHT_BIG-FONT_OFFSET , wi, LEFT, BIG, WHITE);
		RenderString(entries[i],viewx/2 , (viewy-he)/2 + 3*BORDERSIZE + (i+2)*FONTHEIGHT_BIG-FONT_OFFSET , wi, LEFT, BIG, WHITE);
		RenderBox(viewx/2 - 2* BORDERSIZE  , (viewy-he)/2 + 3*BORDERSIZE + (i+1)*FONTHEIGHT_BIG  , viewx-(viewx-wi)/2 - 2*BORDERSIZE  , (viewy-he)/2 + 3*BORDERSIZE + (i+2)*FONTHEIGHT_BIG  , GRID, (pos == i ? WHITE :trans_map[curvisibility]));
		RenderBox(viewx/2 - 2* BORDERSIZE-1, (viewy-he)/2 + 3*BORDERSIZE + (i+1)*FONTHEIGHT_BIG-1, viewx-(viewx-wi)/2 - 2*BORDERSIZE+1, (viewy-he)/2 + 3*BORDERSIZE + (i+2)*FONTHEIGHT_BIG+1, GRID, (pos == i ? WHITE :trans_map[curvisibility]));
	}
	RenderButtons(he,OKCANCEL);
	int drawsel = 0;
	do{
		GetRCCode(RC_NORMAL);
		switch(rccode)
		{
				case RC_OK:
					if (sel == NO) return;
					if (sel == YES)
					{
						end = YES;
						break;
					}
					if (pos != -1)
					{
						DoEditString(viewx/2-2*BORDERSIZE,(viewy-he)/2 + 4*BORDERSIZE + (pos+1)*FONTHEIGHT_BIG-FONT_OFFSET, (viewx-(viewx-wi)/2) - (viewx/2), FILENAME_MAX,entries[pos], BIG,BLUE2, NO);
						RenderBox   (viewx/2-3*BORDERSIZE,(viewy-he)/2 + 3*BORDERSIZE + (pos+1)*FONTHEIGHT_BIG            , viewx-(viewx-wi)/2 - BORDERSIZE  , (viewy-he)/2 + 3*BORDERSIZE + (pos+2)*FONTHEIGHT_BIG  , FILL, BLUE1);
						drawsel = 1;
					}
					break;
				case RC_LEFT:
					sel = YES;
					drawsel = 1;
					break;
				case RC_RIGHT:
					sel = NO;
					drawsel = 1;
					break;
				case RC_UP:
					if (sel != -1)
							pos = 4;
					else
					{
						if (pos > 0) pos--;
					}
					sel = -1;
					drawsel = 1;
					break;
				case RC_DOWN:
					pos++;
					if (pos == 5)
						sel = YES;
					if (pos >= 6)
					{
						pos = 6;
						sel = NO;
					}
					drawsel = 1;
					break;
				case RC_RED:
					rccode = -1;
					end = YES;
					break;
				case RC_GREEN:
				case RC_HOME:
					rccode = -1;
					return;
				default:
					continue;
		}
		if (drawsel)
		{
			for (i = 0; i < 5; i++)
			{
				RenderString(entries[i],viewx/2 , (viewy-he)/2 + 3*BORDERSIZE + (i+2)*FONTHEIGHT_BIG-FONT_OFFSET , wi, LEFT, BIG, WHITE);
				RenderBox(viewx/2 - 2* BORDERSIZE+1, (viewy-he)/2 + 3*BORDERSIZE + (i+1)*FONTHEIGHT_BIG+1, viewx-(viewx-wi)/2 - 2*BORDERSIZE-1, (viewy-he)/2 + 3*BORDERSIZE + (i+2)*FONTHEIGHT_BIG-1, GRID, (pos == i ? WHITE :trans_map[curvisibility]));
				RenderBox(viewx/2 - 2* BORDERSIZE+2, (viewy-he)/2 + 3*BORDERSIZE + (i+1)*FONTHEIGHT_BIG+2, viewx-(viewx-wi)/2 - 2*BORDERSIZE-2, (viewy-he)/2 + 3*BORDERSIZE + (i+2)*FONTHEIGHT_BIG-2, GRID, (pos == i ? WHITE :trans_map[curvisibility]));
			}
			RenderBox(viewx/2 - 2* BORDERSIZE -BUTTONWIDTH  , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 - 2* BORDERSIZE               ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, (sel == YES ? WHITE : RED  ));
			RenderBox(viewx/2 - 2* BORDERSIZE -BUTTONWIDTH+1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 - 2* BORDERSIZE             -1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, (sel == YES ? WHITE : RED  ));
			RenderBox(viewx/2 + 2* BORDERSIZE               , viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT  , viewx/2 + 2* BORDERSIZE +BUTTONWIDTH  ,viewy-(viewy-he)/2- 2* BORDERSIZE  , GRID, (sel == NO ? WHITE : GREEN));
			RenderBox(viewx/2 + 2* BORDERSIZE             +1, viewy-(viewy-he)/2 - 2*BORDERSIZE - BUTTONHEIGHT+1, viewx/2 + 2* BORDERSIZE +BUTTONWIDTH-1,viewy-(viewy-he)/2- 2* BORDERSIZE-1, GRID, (sel == NO ? WHITE : GREEN));
			memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
			drawsel = 0;
		}
		if (end == YES)
		{
			fp = fopen( szFile, "w" );
			if ( !fp )
			{
				printf("tuxcom: could not open ftpfile\n");
			}
			else
			{
				fprintf(fp,"host=%s\n", entries[0]);
				fprintf(fp,"port=%s\n", entries[1]);
				fprintf(fp,"user=%s\n", entries[2]);
				fprintf(fp,"pass=%s\n", entries[3]);
				fprintf(fp,"dir=%s\n" , entries[4]);
				fclose(fp);
			}
			break;
		}


	}while(1);
	rccode = -1;
	return;

}
/******************************************************************************
 * DoMainMenu                                                                 *
 ******************************************************************************/

void DoMainMenu()
{
	int pos = 0, i, wi , he = (MAINMENU+1) * BORDERSIZE + MAINMENU * FONTHEIGHT_BIG;
	wi = 500;
	char szEntry[200];
	if (wi > viewx - 4* BORDERSIZE) wi = viewx - 4* BORDERSIZE;

	RenderBox((viewx-wi)/2 , (viewy-he) /2, viewx-(viewx-wi)/2, viewy-(viewy-he)/2, FILL, trans_map[curvisibility]);
	RenderBox((viewx-wi)/2 , (viewy-he) /2, viewx-(viewx-wi)/2, viewy-(viewy-he)/2, GRID, WHITE);

	for (i = 0; i < MAINMENU ; i++)
	{
		RenderBox((viewx-wi)/2+ BORDERSIZE   , (viewy-he)/2 + BORDERSIZE + (i)*FONTHEIGHT_BIG  , viewx-(viewx-wi)/2 - BORDERSIZE  , (viewy-he)/2 + BORDERSIZE + (i+1)*FONTHEIGHT_BIG  , FILL, (pos == i ? BLUE2 :trans_map[curvisibility]));
		switch(i)
		{
			case 4: // set language
				sprintf(szEntry,mainmenu[i*NUM_LANG+language],mbox[langselect*NUM_LANG+language]);
				break;
			case 5: // save settings
				sprintf(szEntry,mainmenu[i*NUM_LANG+language],mbox[autosave  *NUM_LANG+language]);
				break;
			default:
				strcpy(szEntry,mainmenu[i*NUM_LANG+language]);
				break;
		}
		RenderString(szEntry,(viewx-wi)/2+ BORDERSIZE , (viewy-he)/2 + BORDERSIZE + (i+1)*FONTHEIGHT_BIG-FONT_OFFSET , wi, CENTER, BIG, WHITE);
	}
	memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
	int drawsel = 0;
	do{
		GetRCCode(RC_NORMAL);
		switch(rccode)
		{
				case RC_OK:
					{
						rccode = -1;
						switch (pos)
						{
							case 0:
								DoSearchFiles();
								return;
							case 1:
								DoTaskManager();
								return;
							case 2:
								screenmode = 1-screenmode;
								//ioctl(avs, AVSIOSSCARTPIN8, &fncmodes[screenmode]);
								//ioctl(saa, SAAIOSWSS, &saamodes[screenmode]);
								return;
							case 3:
								SetPassword();
								return;
							case 6:
								WriteSettings();
								MessageBox(info[INFO_SAVED*NUM_LANG+language],"",OK);
								return;
						}
					}
					break;
				case RC_UP:
					pos--;
					if (pos < 0) pos = MAINMENU-1;
					drawsel = 1;
					break;
				case RC_DOWN:
					pos++;
					if (pos >= MAINMENU) pos = 0;
					drawsel = 1;
					break;
				case RC_LEFT:
					switch (pos)
					{
						case 4:
							switch (langselect)
							{
								case BTN_AUTO     : langselect = BTN_ENGLISH  ; break;
								case BTN_PORTUGUES: langselect = BTN_AUTO     ; break;
								case BTN_SWEDISH  : langselect = BTN_PORTUGUES; break;
								case BTN_ITALIAN  : langselect = BTN_SWEDISH  ; break;
								case BTN_GERMAN   : langselect = BTN_ITALIAN  ; break;
								case BTN_ENGLISH  : langselect = BTN_GERMAN   ; break;
							}
							SetLanguage();
							break;
						case 5:
							switch (autosave)
							{
								case BTN_ASK    : autosave   = BTN_NO     ; break;
								case BTN_YES    : autosave   = BTN_ASK    ; break;
								case BTN_NO     : autosave   = BTN_YES    ; break;
							}
							break;
					}
					drawsel = 1;
					break;
				case RC_RIGHT:
					switch (pos)
					{
						case 4:
							switch (langselect)
							{
								case BTN_AUTO     : langselect = BTN_PORTUGUES; break;
								case BTN_PORTUGUES: langselect = BTN_SWEDISH  ; break;
								case BTN_SWEDISH  : langselect = BTN_ITALIAN  ; break;
								case BTN_ITALIAN  : langselect = BTN_GERMAN   ; break;
								case BTN_GERMAN   : langselect = BTN_ENGLISH  ; break;
								case BTN_ENGLISH  : langselect = BTN_AUTO     ; break;
							}
							SetLanguage();
							break;
						case 5:
							switch (autosave)
							{
								case BTN_ASK    : autosave   = BTN_YES    ; break;
								case BTN_YES    : autosave   = BTN_NO     ; break;
								case BTN_NO     : autosave   = BTN_ASK    ; break;
							}
							break;
					}
					drawsel = 1;
					break;
				case RC_HOME:
					rccode = -1;
					return;
				default:
					continue;
		}
		if (drawsel)
		{
			for (i = 0; i < MAINMENU; i++)
			{
				RenderBox((viewx-wi)/2+ BORDERSIZE   , (viewy-he)/2 + BORDERSIZE + (i)*FONTHEIGHT_BIG  , viewx-(viewx-wi)/2 - BORDERSIZE  , (viewy-he)/2 + BORDERSIZE + (i+1)*FONTHEIGHT_BIG  , FILL, (pos == i ? BLUE2 :trans_map[curvisibility]));
				switch(i)
				{
					case 4: // set language
						sprintf(szEntry,mainmenu[i*NUM_LANG+language],mbox[langselect*NUM_LANG+language]);
						break;
					case 5: // save settings
						sprintf(szEntry,mainmenu[i*NUM_LANG+language],mbox[autosave  *NUM_LANG+language]);
						break;
					default:
						strcpy(szEntry,mainmenu[i*NUM_LANG+language]);
						break;
				}
				RenderString(szEntry,(viewx-wi)/2+ BORDERSIZE , (viewy-he)/2 + BORDERSIZE + (i+1)*FONTHEIGHT_BIG-FONT_OFFSET , wi, CENTER, BIG, WHITE);
			}
			memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
			drawsel = 0;
		}
	}while(1);
	rccode = -1;
	return;

}
/******************************************************************************
 * DoSearchFiles                                                              *
 ******************************************************************************/

void DoSearchFiles()
{
	char szMsg[FILENAME_MAX+256];
	char action[1000];
	sprintf(szMsg,msg[MSG_SEARCHFILES*NUM_LANG+language],finfo[curframe].path);
	if (GetInputString(400,FILENAME_MAX,szSearchstring,szMsg,NO) == RC_OK && (*szSearchstring != 0x00))
	{
		colortool[0] = ACTION_NOACTION;
		colortool[1] = ACTION_NOACTION;
		colortool[2] = ACTION_NOACTION;
		colortool[3] = ACTION_NOACTION;
		RenderMenuLine(-1, NO);

		sprintf(action,"find \"%s\" -name \"%s\"",finfo[curframe].path,szSearchstring);
		MessageBox(info[INFO_SEARCH1*NUM_LANG+language],"",NOBUTTON);
		DoExecute(action, SHOW_SEARCHRESULT);

	}
}
/******************************************************************************
 * GetInputString                                                             *
 ******************************************************************************/

int GetInputString(int width,int maxchars, char* str, char * msg, int pass)
{


	int le1, wi, he, x , y;



	le1 = GetStringLen(msg, BIG);
	wi = MINBOX;
	if (width > viewx - 8* BORDERSIZE) width = viewx - 8* BORDERSIZE;
	if (le1   > wi ) wi = le1 + 6*BORDERSIZE;
	if (width > wi ) wi = width + 6*BORDERSIZE;
	if (wi > viewx - 6* BORDERSIZE) wi = viewx - 6* BORDERSIZE;

	he = 6* BORDERSIZE+ 2*FONTHEIGHT_BIG;


	RenderBox((viewx-wi)/2 , (viewy-he) /2, viewx-(viewx-wi)/2, viewy-(viewy-he)/2, FILL, trans_map[curvisibility]);
	RenderBox((viewx-wi)/2 , (viewy-he) /2, viewx-(viewx-wi)/2, viewy-(viewy-he)/2, GRID, WHITE);
	RenderString(msg,(viewx-wi)/2-BORDERSIZE , (viewy-he)/2 + BORDERSIZE + FONTHEIGHT_BIG-FONT_OFFSET , wi+2*BORDERSIZE, CENTER, BIG, WHITE);

	x = (viewx-width)/2 - BORDERSIZE;
	y = (viewy-he)/2+ 2*BORDERSIZE + FONTHEIGHT_BIG;

	RenderBox(x,y, x+width +2*BORDERSIZE, y+FONTHEIGHT_BIG+2*BORDERSIZE, FILL, trans_map[curvisibility]);
	RenderBox(x,y, x+width              , y+FONTHEIGHT_BIG+2*BORDERSIZE, GRID, WHITE);

	return DoEditString(x+BORDERSIZE,y+BORDERSIZE,width-2*BORDERSIZE,maxchars,str, BIG,BLUE1, pass);
}
/******************************************************************************
 * DoEditString                                                               *
 ******************************************************************************/


int DoEditString(int x, int y, int width, int maxchars, char* str, int vsize, int back, int pass)
{

	int pos = 0, start = 0, slen, he = (vsize==BIG ? FONTHEIGHT_BIG : FONTHEIGHT_SMALL);
	int prev_key = -1;
	int markmode = 0, markpos=0;
	char szbuf[maxchars+1];
	char szdst[maxchars+1];
	char * pch;

	memset(szdst, 0,maxchars+1);
	strcpy(szdst,str);
	szdst[strlen(szdst)] = ' ';
	szdst[strlen(szdst)+1] = 0x00;
	strcpy(szbuf,str);
	szbuf[1] = 0x00;

	RenderBox(x-1,y, x+width+1, y+he, FILL, back);
	RenderBox(x-1,y, x+GetStringLen(szbuf, vsize)+1, y+he, FILL, RED);
	RenderString(szdst,x, y+he-FONT_OFFSET, width, LEFT, vsize, WHITE);

	colortool[0] = ACTION_CLEAR ;
	colortool[1] = (pass == NO ? ACTION_MARKTEXT : ACTION_NOACTION);
	colortool[2] = (textuppercase == 0 ? ACTION_UPPERCASE : ACTION_LOWERCASE);
	colortool[3] = (pass == NO ? ACTION_INSTEXT  : ACTION_NOACTION);
	RenderMenuLine(-1, EDIT);

	memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);

	do{
		while (GetRCCode(RC_EDIT) == 0);
#ifndef HAVE_DREAMBOX_HARDWARE
		if ((rccode >=0x20) && (rccode < 0x0100))
		{
		  kbcode=rccode;
		  if ((kbcode>=RC_0) && (kbcode<=RC_9))
		  {
		  	rccode-=RC_0;
		  	kbcode=0;
					if (markmode == 0)
					{
						if (prev_key != -1 && rccode != prev_key) // jump to next char when other number pressed
						{
							pos++;
							if (pos >= strlen(szdst))
							{
								if (pos > maxchars) pos = maxchars;
								else
									strcat(szdst," ");
							}
						}
						prev_key = rccode;
						pch = strchr(numberchars[rccode],tolower(szdst[pos]));
						if (pch == NULL) szdst[pos] = (textuppercase == 0 ? numberchars[rccode][0] : toupper(numberchars[rccode][0]));
						else
						{
							if (pch == &(numberchars[rccode][strlen(numberchars[rccode])-1])) szdst[pos] = (textuppercase == 0 ? numberchars[rccode][0]: toupper(numberchars[rccode][0]));
							else szdst[pos] = (textuppercase == 0 ? *((char*)pch+1) : toupper(*((char*)pch+1)));
						}
					}
		  }
		}
		else if ((rccode >= 0x0201) && (rccode <= 0x020A))
		{
		  kbcode = (rccode & 0x00FF) ;
		  if (kbcode == 10) kbcode = 0;
		  kbcode += RC_0;
		}
		else
		  kbcode = 0;
#endif
		if (kbcode != 0 && markmode == 0)
		{
			if (kbcode == 0x7f) // backspace
			{
				if (pos > 0)
				{
					if (pos== strlen(szdst)-1) // remove last char when at end of line
					{
						szdst[pos-1] = 0x00;
					}
					else if (szdst[pos] != 0x00)
					{
						strcpy(szbuf,(char*)(szdst+pos));
						strcpy((char*)(szdst+pos-1),szbuf);
					}
					pos--;
				}
			}
			else if (kbcode >= 0x20)
			{
				if (szdst[pos] != 0x00)
				{
					strcpy(szbuf,(char*)(szdst+pos));
					szdst[pos] = kbcode;
					strcpy((char*)(szdst+pos+1),szbuf);
				}
				pos++;
			}
			prev_key = -1;
			rccode = -1;
		}
		switch(rccode)
		{
				case RC_OK:
					if (markmode == 0)
					{
						*str = 0x00;
						strncpy(str,szdst,maxchars);
						if (strlen(szdst) > 0)
						{
							str[strlen(szdst)] = 0x00;
							while (str[strlen(str)-1] == ' ')
								str[strlen(str)-1] = 0x00;
						}
						rccode = -1;
						return RC_OK;
					}
					else
					{
						memset(szClipboard,0,256);
						if (pos != markpos)
						{
							if (pos > markpos)
								strncpy(szClipboard,(szdst+markpos),(pos-markpos));
							else
								strncpy(szClipboard,(szdst+pos),(markpos-pos));
						}
						markmode = 0;
						break;
					}
				case RC_LEFT:
					pos--;
					prev_key = -1;
					break;
				case RC_RIGHT:
					pos++;
					prev_key = -1;
					break;
				case RC_PLUS:
					if (markmode == 0)
					{
						if (szdst[pos] != 0x00)
						{
							strcpy(szbuf,(char*)(szdst+pos));
							szdst[pos] = ' ';
							strcpy((char*)(szdst+pos+1),szbuf);
						}
						prev_key = -1;
					}
					break;
				case RC_MINUS:
					if (markmode == 0)
					{
						if (pos== strlen(szdst)-1) // remove last char when at end of line
						{
							pos--;
							szdst[pos] = 0x00;
						}
						else if (szdst[pos] != 0x00)
						{
							strcpy(szbuf,(char*)(szdst+pos+1));
							strcpy((char*)(szdst+pos),szbuf);
						}
						prev_key = -1;
					}
					break;
				case RC_DOWN:
					if (markmode == 0)
					{
						pch = strchr(charset,szdst[pos]);
						if (pch == NULL) szdst[pos] = ' ';
						else
						{
							if (pch == charset) szdst[pos] = charset[strlen(charset)-1];
							else szdst[pos] = *((char*)pch-1);
						}
						prev_key = -1;
					}
					break;
				case RC_UP:
					if (markmode == 0)
					{
						pch = strchr(charset,szdst[pos]);
						if (pch == NULL) szdst[pos] = ' ';
						else
						{
							if (pch == &(charset[strlen(charset)-1])) szdst[pos] = charset[0];
							else szdst[pos] = *((char*)pch+1);
						}
						prev_key = -1;
					}
					break;
				case RC_0:
				case RC_1:
				case RC_2:
				case RC_3:
				case RC_4:
				case RC_5:
				case RC_6:
				case RC_7:
				case RC_8:
				case RC_9:
					if (markmode == 0)
					{
						if (prev_key != -1 && rccode != prev_key) // jump to next char when other number pressed
						{
							pos++;
							if (pos >= strlen(szdst))
							{
								if (pos > maxchars) pos = maxchars;
								else
									strcat(szdst," ");
							}
						}
						prev_key = rccode;
						pch = strchr(numberchars[rccode],tolower(szdst[pos]));
						if (pch == NULL) szdst[pos] = (textuppercase == 0 ? numberchars[rccode][0] : toupper(numberchars[rccode][0]));
						else
						{
							if (pch == &(numberchars[rccode][strlen(numberchars[rccode])-1])) szdst[pos] = (textuppercase == 0 ? numberchars[rccode][0]: toupper(numberchars[rccode][0]));
							else szdst[pos] = (textuppercase == 0 ? *((char*)pch+1) : toupper(*((char*)pch+1)));
						}
					}
					break;
				case RC_RED:
					if (markmode == 0)
					{
						szdst[0] = 0x00;
						pos = 0;
						prev_key = -1;
					}
					break;
				case RC_GREEN:
					if (markmode == 0 && pass == NO)
					{
						markmode = 1;
						markpos = pos;
					}
					break;
				case RC_YELLOW:
					if (markmode == 0)
					{
						textuppercase = 1-textuppercase;
						szdst[pos] = (textuppercase == 0 ? tolower(szdst[pos]) : toupper(szdst[pos]));
					}
					break;
				case RC_BLUE:
					if (markmode == 0 && pass == NO)
					{
						if (szdst[pos] != 0x00 && szClipboard[0] != 0x00)
						{
							strcpy(szbuf,(char*)(szdst+pos));
							strcpy((char*)(szdst+pos),szClipboard);
							strcpy((char*)(szdst+pos+strlen(szClipboard)),szbuf);
						}
					}
					break;
				case RC_HOME:
					rccode = -1;
					if (markmode == 0)
					{
						return RC_HOME;
					}
					else
					{
						markmode = 0;
						break;
					}

		}
		if (pos <  0            ) pos = 0;
		if (pos >= strlen(szdst))
		{
			if (pos > maxchars) pos = maxchars;
			else
			{
				char* c = strchr(szdst,'\n');
				if (c) *c = ' ';
				strcat(szdst," ");
			}
		}
		if (start > pos) start = pos;
		while (1)
		{
			strcpy(szbuf,(char*)(szdst+start));
			szbuf[pos+1-start] = 0x00;
			slen = GetStringLen(szbuf, vsize);
		 	if (slen >= width - 2*BORDERSIZE)
				start++;
			else
				break;
		}
		if (pass == NO)
			strcpy(szbuf,(char*)(szdst+start));
		else
			strcpy(szbuf,"******************");
		if (markmode == 0)
			szbuf[pos+1-start] = 0x00;
		else
		{
			if (pos > markpos)
				szbuf[pos -start] = 0x00;
			else
				szbuf[markpos -start] = 0x00;
		}
		slen = GetStringLen(szbuf, vsize);
		if (markmode == 0)
			szbuf[pos-start] = 0x00;
		else
		{
			if (pos > markpos)
				szbuf[markpos -start] = 0x00;
			else
				szbuf[pos -start] = 0x00;
		}
		int slen2=GetStringLen(szbuf, vsize);

		RenderBox(x-1,y, x+width+1, y+he, FILL, back);
		RenderBox(x+slen2-1,y, x+slen+1, y+he, FILL, (markmode == 0 ? RED : GREEN));
		if (pass == NO)
			RenderString((char*)(szdst+start),x, y+he-FONT_OFFSET, width, LEFT, vsize, WHITE);
		else
		{
			strcpy(szbuf,"******************");
			szbuf[strlen(szdst)]=0x00;
			RenderString(szbuf,x, y+he-FONT_OFFSET, width, LEFT, vsize, WHITE);
		}
		if (markmode == 0)
		{
			colortool[0] = ACTION_CLEAR ;
			colortool[1] = (pass == NO ? ACTION_MARKTEXT : ACTION_NOACTION);
			colortool[2] = (textuppercase == 0 ? ACTION_UPPERCASE : ACTION_LOWERCASE);
			colortool[3] = (pass == NO ? ACTION_INSTEXT  : ACTION_NOACTION);
		}
		else
		{
			colortool[0] = ACTION_NOACTION;
			colortool[1] = ACTION_NOACTION;
			colortool[2] = ACTION_NOACTION;
			colortool[3] = ACTION_NOACTION;
		}
		RenderMenuLine(-1, EDIT);
		memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
	}while(1);

	rccode = -1;
	return rccode;
}

/******************************************************************************
 * flistcmp                                                                   *
 ******************************************************************************/

int flistcmp(struct fileentry * p1, struct fileentry * p2)
{

	if (S_ISDIR(p1->fentry.st_mode) )
	{
		if (strcmp(p1->name,"..") == 0)
			return -1;
		if (strcmp(p1->name,"/") == 0)
			return -1;
		if (S_ISDIR(p2->fentry.st_mode) )
			return strcmp(p1->name,p2->name) * cursort;
		else
			return -1;
	}
	if (S_ISDIR(p2->fentry.st_mode) )
	{
		if (strcmp(p1->name,"..") == 0)
			return -1;
		if (strcmp(p1->name,"/") == 0)
			return -1;
		return 1;
	}
	else
		return strcmp(p1->name,p2->name) * cursort;
}

/******************************************************************************
 * sortframe                                                                  *
 ******************************************************************************/

void sortframe(int frame, char* szSel)
{
	int i;

	qsort(finfo[frame].flist,finfo[frame].count, sizeof(struct fileentry),(__compar_fn_t)flistcmp);
	for (i =0; i < finfo[frame].count; i++)
	{
		if (strcmp(getfileentry(frame, i)->name,szSel) == 0)
		{
			finfo[frame].selected = i;
			break;
		}
	}
}
/******************************************************************************
 * getfileentry                                                               *
 ******************************************************************************/

struct fileentry* getfileentry(int frame, int pos)
{
	return &finfo[frame].flist[pos];
}

/******************************************************************************
 * GetSelected                                                                *
 ******************************************************************************/

struct fileentry* GetSelected(int frame)
{
	return &finfo[frame].flist[finfo[frame].selected];
}
/******************************************************************************
 * SetSelected                                                                *
 ******************************************************************************/

void SetSelected(int frame, const char* szFile)
{
	int i;
	for (i = 0; i < finfo[frame].count; i++)
	{
		if (strcmp(finfo[frame].flist[i].name,szFile) == 0)
		{
			finfo[frame].selected = i;
			if (finfo[frame].selected  < 0)
				finfo[frame].selected  = 0;
			if (finfo[frame].selected >= finfo[frame].count)
				finfo[frame].selected  = finfo[frame].count -1;
			if (finfo[frame].first     > finfo[frame].selected)
				finfo[frame].first     = finfo[frame].selected;
			if (finfo[frame].selected >= finfo[frame].first + framerows)
				finfo[frame].first     = finfo[frame].selected - framerows+1;
			break;
		}
	}
}
/******************************************************************************
 * FindFile                                                                   *
 ******************************************************************************/

struct fileentry* FindFile(int frame, const char* szFile)
{
	int i;
	for (i = 0; i < finfo[frame].count; i++)
	{
		if (strcmp(finfo[frame].flist[i].name,szFile) == 0) return &finfo[frame].flist[i];
	}
	return NULL;
}

/******************************************************************************
 * ClearEntries                                                               *
 ******************************************************************************/

void ClearEntries(int frame)
{

	if (finfo[frame].flist != NULL)
	{
		free(finfo[frame].flist);
		finfo[frame].flist = NULL;
	}
}
/******************************************************************************
 * ClearZipEntries                                                            *
 ******************************************************************************/

void ClearZipEntries(int frame)
{
	struct zipfileentry * pzfe, *pzfe1 = finfo[frame].allziplist;
	while (pzfe1 != NULL)
	{
		pzfe  = pzfe1;
		pzfe1 = pzfe1->next;
		free(pzfe);
	}

	finfo[frame].allziplist = NULL;
	finfo[frame].zipfile[0] = 0x00;
	if (finfo[frame].ftpconn != NULL)
	{
		char buf[512];
		FTPcmd(frame, "QUIT",NULL,buf);
		fclose(finfo[frame].ftpconn);
		finfo[frame].ftpconn = NULL;
	}

}
/******************************************************************************
 * ClearMarker                                                                *
 ******************************************************************************/

void ClearMarker(int frame)
{
	struct marker * pmk, *pmk1 = finfo[frame].mlist;
	while (pmk1 != NULL)
	{
		pmk = pmk1;
		pmk1 = pmk1->next;
		free(pmk);
	}
	finfo[frame].mlist = NULL;
	finfo[frame].markcount = 0;
	finfo[frame].marksize  = 0;
}
/******************************************************************************
 * ToggleMarker                                                               *
 ******************************************************************************/

void ToggleMarker(int frame)
{
	struct fileentry *pfe = GetSelected(frame);
	struct marker * pmk = NULL, *pmk1 = finfo[frame].mlist;

	if (strcmp(pfe->name,"..") == 0) return;
	while (pmk1 != NULL)
	{
		if (strcmp(pmk1->name,pfe->name) == 0)
		{
		  if (pmk == NULL)
			  finfo[frame].mlist = pmk1->next;
		  else
			  pmk->next = pmk1->next;
		  free(pmk1);
		  finfo[frame].markcount--;
		  if (!S_ISDIR(pfe->fentry.st_mode))
		  	finfo[frame].marksize -= pfe->fentry.st_size;
		  return;
		}
		pmk = pmk1;
		pmk1 = pmk1->next;
	}
	pmk = malloc(sizeof(struct marker));
	strcpy(pmk->name,pfe->name);
	pmk->next = finfo[frame].mlist;
	finfo[frame].mlist = pmk;
	finfo[frame].markcount++;
	if (!S_ISDIR(pfe->fentry.st_mode))
		finfo[frame].marksize += pfe->fentry.st_size;
}
/******************************************************************************
 * RenameMarker                                                               *
 ******************************************************************************/

void RenameMarker(int frame, const char* szOld, const char* szNew )
{
	struct marker * pmk = finfo[frame].mlist;

	while (pmk != NULL)
	{
		if (strcmp(pmk->name,szOld) == 0)
		{
		  strcpy(pmk->name,szNew);
		  return;
		}
		pmk = pmk->next;
	}
}
/******************************************************************************
 * IsMarked                                                                   *
 ******************************************************************************/

int IsMarked(int frame, int pos)
{
	struct fileentry *pfe = getfileentry(frame, pos);
	struct marker * pmk = finfo[frame].mlist;
	while (pmk != NULL)
	{
		if (strcmp(pmk->name,pfe->name) == 0)
		{
		  return 1;
		}
		pmk = pmk->next;
	}
	return 0;
}

/******************************************************************************
 * CheckOverwrite                                                             *
 ******************************************************************************/
int CheckOverwrite(struct fileentry* pfe, int mode, char* szNew)
{
	char szMessage[356];

	strcpy(szNew,pfe->name);
	if (overwriteall != 0)
	{
	    if (S_ISDIR(pfe->fentry.st_mode)) *szNew = 0x00;
	    return overwriteall;
	}

	if (FindFile(1-curframe,pfe->name) != NULL)
	{
		if (skipall != 0)      return skipall;
		sprintf(szMessage,msg[MSG_FILE_EXISTS*NUM_LANG+language], pfe->name);
		switch (MessageBox(szMessage,"",mode))
		{
			case OVERWRITE:
				if (S_ISDIR(pfe->fentry.st_mode)) *szNew = 0x00;
				return OVERWRITE;
				break;
			case OVERWRITEALL:
				if (S_ISDIR(pfe->fentry.st_mode)) *szNew = 0x00;
				overwriteall = OVERWRITE;
				return OVERWRITE;
			case SKIP:
				return SKIP;
			case SKIPALL:
				skipall = SKIP;
				return SKIP;
			case RENAME:
				sprintf(szMessage,msg[MSG_RENAME*NUM_LANG+language], pfe->name);
				if (GetInputString(400,255,szNew,szMessage, NO) == RC_OK)
					return RENAME;
				else
					return -1;
			case CANCEL:
				overwriteall = -1;
				skipall = -1;
				return -1;

		}
	}
	return OVERWRITE;
}
/******************************************************************************
 * FillDir                                                                    *
 ******************************************************************************/

void FillDir(int frame, int selmode)
{
	char* pch;
	char selentry[256];
	*selentry = 0x00;
	char szSel[256];
	*szSel = 0x00;
	int npos = 0;
	struct fileentry* pfe = NULL;
	if (finfo[frame].selected > 0)
	{
	  strcpy(szSel,GetSelected(frame)->name);
	}
	if (finfo[frame].zipfile[0] != 0x00)
	{
		if ((selmode == SELECT_UPDIR) &&(frame == curframe))
		{
			if (strcmp(finfo[frame].zippath,"/") == 0)
			{
				strncat(finfo[curframe].path,finfo[frame].zipfile,256);
				strncat(finfo[curframe].path,"/",1);
				ClearZipEntries(frame);
			}
		}
	}
	ClearEntries(frame);
	if (finfo[frame].zipfile[0] != 0x00) // filling zipfile structure
	{
		char szDir[2000];
		if ((selmode == SELECT_UPDIR) &&(frame == curframe))
		{
			finfo[frame].zippath[strlen(finfo[frame].zippath)-1]=0x00;
			pch = strrchr(finfo[frame].zippath,'/');
			if (pch)
			{
				strcpy(selentry,(pch+1));
				*(pch+1) = 0x00;
			}
			ReadFTPDir(frame,"..");
		}
		else if (strcmp(finfo[frame].zippath,"/") == 0)
			ReadFTPDir(frame,"/");
		else
		{
			strcpy(szDir,finfo[frame].zippath);
			szDir[strlen(szDir)-1]=0x00;
			pch = strrchr(szDir,'/');
			ReadFTPDir(frame,pch+1);
		}

		finfo[frame].count = 1;
		finfo[frame].size  = 0;
		finfo[frame].writable = 0;
		struct zipfileentry * pzfe;
		int zlen = strlen(finfo[frame].zippath);

		sprintf(szDir,"%s.",finfo[frame].zippath);

		pzfe = finfo[frame].allziplist;
		while (pzfe != NULL)
		{
			if (strcmp(pzfe->name, szDir) != 0)
			{
				if ((strncmp(finfo[frame].zippath,pzfe->name,zlen) == 0) && (strrchr(pzfe->name,'/') == (char*)(pzfe->name+zlen-1)))
				{
					finfo[frame].count++;
					finfo[frame].size+= pzfe->fentry.st_size;
				}
			}
			pzfe = pzfe->next;
		}
		finfo[frame].flist = (struct fileentry*)calloc(finfo[frame].count, sizeof(struct fileentry));

		pfe = getfileentry(frame, npos);
		// create ".." entry
		strcpy(pfe->name,"..");
		memset(&pfe->fentry, 0, sizeof(struct stat));
		pfe->fentry.st_mode = S_IFDIR;
		npos++;
		pzfe = finfo[frame].allziplist;
		while (pzfe != NULL)
		{
			if ((strncmp(finfo[frame].zippath,pzfe->name,zlen) == 0)  && (strrchr(pzfe->name,'/') == (char*)(pzfe->name+zlen-1)))
			{
				pfe = getfileentry(frame, npos);
				int ppos = strcspn((char*)(pzfe->name+zlen),"/");
				if (ppos > 0)
				{
					strncpy(pfe->name,(char*)(pzfe->name+zlen),ppos);
					pfe->name[strlen(pfe->name)-1] = 0x00;
				}
				else
					strcpy(pfe->name,(char*)(pzfe->name+zlen));
				memcpy(&pfe->fentry,&pzfe->fentry,sizeof(struct stat));
				if ((selmode == SELECT_UPDIR) && (strcmp(pfe->name,selentry) == 0) && (frame==curframe))
				{
					finfo[frame].selected = npos;
					strcpy(szSel,GetSelected(frame)->name);
				}
				npos++;
			}
			pzfe = pzfe->next;
		}
	}
	else // filling normal directory
	{
		if (finfo[frame].path[0] != '/') strcpy(finfo[frame].path, DEFAULT_PATH);
		if ((selmode == SELECT_UPDIR) &&(frame == curframe))
		{
			finfo[curframe].path[strlen(finfo[curframe].path)-1]=0x00;
			pch = strrchr(finfo[frame].path,'/');
			if (pch)
			{
				strcpy(selentry,(pch+1));
				*(pch+1) = 0x00;
			}

		}
		else if ((selmode == SELECT_ROOTDIR) &&(frame == curframe))
		{
			pch = strchr(&(finfo[frame].path[1]),'/');
			if (pch)
			{
				strcpy(selentry,(pch+1));
				*(pch+1) = 0x00;
			}
			strcpy(finfo[frame].path, DEFAULT_PATH);

		}
#if DEBUG
		printf("filling directory structure:<%s>\n", finfo[frame].path);
#endif
		DIR* dp = opendir(finfo[frame].path);

		struct dirent *dentry;
		char fullfile[FILENAME_MAX];
		finfo[frame].count = (strcmp(finfo[frame].path,"/") == 0 ? 0 : 2);
		finfo[frame].size  = 0;
		finfo[frame].writable = 0;
		if (!dp)
		{
			printf("cannot read %s\n", finfo[frame].path);
			strcpy(finfo[frame].path, DEFAULT_PATH);
			dp = opendir(finfo[frame].path);
		}
		while((dentry = readdir(dp)) != NULL)
		{
			if (strcmp(dentry->d_name,".") == 0)
			{
				struct stat   st;
				sprintf(fullfile,"%s.",finfo[frame].path);
				lstat(fullfile,&st);
				if ((st.st_mode & S_IWUSR) == S_IWUSR )
					finfo[frame].writable = 1;
				continue;
			}
			if (strcmp(dentry->d_name,"..") == 0)
				continue;
			finfo[frame].count++;
		}

		finfo[frame].flist = (struct fileentry*)calloc(finfo[frame].count, sizeof(struct fileentry));


		// rewinddir not defined ?????????????????
		closedir(dp);
		dp = opendir(finfo[frame].path);

		if (strcmp(finfo[frame].path,"/") != 0)
		{
			// create "/" entry when not in root dir
			pfe = getfileentry(frame, npos);
			strcpy(pfe->name,"/");
			memset(&pfe->fentry, 0, sizeof(struct stat));
			pfe->fentry.st_mode = S_IFDIR;
			npos++;
			pfe = getfileentry(frame, npos);
			// create ".." entry when not in root dir
			strcpy(pfe->name,"..");
			memset(&pfe->fentry, 0, sizeof(struct stat));
			pfe->fentry.st_mode = S_IFDIR;
			npos++;
		}
		while((dentry = readdir(dp)) != NULL && npos < finfo[frame].count)
		{
			if (strcmp(dentry->d_name,".") == 0)
				continue;
			if (strcmp(dentry->d_name,"..") == 0)
				continue;
			pfe = getfileentry(frame, npos);
			strcpy(pfe->name,dentry->d_name);
			sprintf(fullfile,"%s%s",finfo[frame].path,pfe->name);
			lstat(fullfile,&(pfe->fentry));
			if     (!((S_ISDIR(pfe->fentry.st_mode)) || (S_ISLNK(pfe->fentry.st_mode))))
			{
				finfo[frame].size+= pfe->fentry.st_size;
			}
			if ((selmode == SELECT_UPDIR) && (strcmp(pfe->name,selentry) == 0) && (frame==curframe))
			{
				finfo[frame].selected = npos;
				strcpy(szSel,GetSelected(frame)->name);
			}
			npos++;
		}
		closedir(dp);
	}
	cursort = finfo[frame].sort;
	sortframe(frame, szSel);



}

/******************************************************************************
 * DoCopy                                                                     *
 ******************************************************************************/

int DoCopy(struct fileentry* pfe, int typ, int checktype, char* szZipCommand)
{
	int i = 1;
	char action[512], szFullFile[1000], tp;
	char szNew[FILENAME_MAX];

	int check = CheckOverwrite(pfe, checktype, szNew);
	if (check < 0 || check == SKIP) return check;

	if (finfo[curframe].ftpconn != NULL)
	{
		char szMessage[400],buf[512], szMessage2[400], xbuf[FTPBUFFER_SIZE];
		sprintf(szMessage,msg[MSG_COPY_PROGRESS*NUM_LANG+language], pfe->name, finfo[1-curframe].path);
		MessageBox(szMessage,"",NOBUTTON);


		struct sockaddr_in s_inlist;
		long size = 0, r, rg = 0;
		memcpy(&s_inlist,&finfo[curframe].s_in,sizeof(struct sockaddr_in));

		FTPcmd(curframe, "TYPE A", NULL, buf);

		if (FTPcmd(curframe, "SIZE ", pfe->name, buf) == 213) size = atol(buf+4);
		else
		{
			sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"SIZE ",pfe->name);
			MessageBox(szMessage,buf,OK);
			return check;
		}

		FTPcmd(curframe, "TYPE I", NULL, buf);

		if (FTPcmd(curframe, "PASV", NULL, buf) != 227)
		{
			sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"PASV","");
			MessageBox(szMessage,buf,OK);
			return check;
		}
		sprintf(szFullFile,"%s%s",finfo[1-curframe].path,szNew);
		FILE* fnewFile = NULL;
		struct stat st;
		if ((lstat(szFullFile,&st) != -1) && (st.st_size > 0))
		{
			// append to downloaded file
			sprintf(szMessage,msg[MSG_APPENDDOWNLOAD*NUM_LANG+language], szNew);
			int res = MessageBox(szMessage,"",YESNOCANCEL);
			switch (res)
			{
				case YES:
					{
						char szsize[50];
						sprintf(szsize, "%ld", st.st_size);
						if (FTPcmd(curframe,"REST", szsize, buf) == 350) {
							size -= st.st_size;
							fnewFile = fopen(szFullFile,"a");
						}
						else
						{
							sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"REST ",szsize);
							MessageBox(szMessage,buf,OK);
							return check;
						}
						break;
					}
				case NO:
					fnewFile = fopen(szFullFile,"w");
					break;
				case CANCEL:
					return check;
			}
		}
		else
			fnewFile = fopen(szFullFile,"w");

		if (fnewFile == NULL)
		{
			sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"RETR ",pfe->name);
			MessageBox(szMessage,"open file failure",OK);
			return check;
		}


		char* s = strrchr(buf, ',');
		*s = 0;
		int port = atoi(s+1);
		s = strrchr(buf, ',');
		port += atoi(s+1) * 256;
		s_inlist.sin_port = htons(port);
		int sControl = socket(AF_INET, SOCK_STREAM, 0);
		connect(sControl, (struct sockaddr *)&s_inlist, sizeof(struct sockaddr_in));
		FILE* fData = fdopen(sControl, "r+");
		if (FTPcmd(curframe, "RETR ", pfe->name, buf) > 150)
		{
			sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"RETR ",pfe->name);
			MessageBox(szMessage,buf,OK);
			return check;
		}
		if (fData == NULL)
		{
			sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"RETR ",pfe->name);
			MessageBox(szMessage,"open stream failure",OK);
			fclose(fnewFile);
			return check;
		}
		do
		{
			sprintf(szMessage,msg[MSG_COPY_PROGRESS*NUM_LANG+language], pfe->name, finfo[1-curframe].path);
			sprintf(szMessage2,"%lu/%lu Bytes (%d %%)", rg,size, (int)((((double)rg)/size)*100));
			if (MessageBox(szMessage,szMessage2,CANCELRUN) == CANCEL)
			{
				if (MessageBox(msg[MSG_CANCELDOWNLOAD*NUM_LANG+language],pfe->name,OKCANCEL) == OK)
				{
					fclose(fnewFile);
					fclose(fData);

					FTPcmd(curframe, NULL, NULL, buf);
					return check;
				}
			}

			r = 0;
			do
			{
				clearerr(fData);
				r += fread((char *)xbuf + r, 1, FTPBUFFER_SIZE-2 - r, fData);
			}
			while (r < FTPBUFFER_SIZE-2 && r+rg < size && ferror(fData) && errno == EINTR);
//			r = read (sControl,xbuf,FTPBUFFER_SIZE-2);

			rg += r;
//			if (ferror(fData) || (feof(fData) && (rg < size)) || (r == 0))
			if (((rg < size) && (r == 0))|| (r == -1) )
			{
				sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"RETR ",pfe->name);
				MessageBox(szMessage,"reading stream failure",OK);
				break;
			}
			if (fwrite(xbuf,1,r,fnewFile) != r)
			{
				sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"RETR ",pfe->name);
				MessageBox(szMessage,"writing file failure",OK);
				break;

			}

		}
		while (rg < size);
		fclose(fnewFile);
		fclose(fData);

		if (FTPcmd(curframe, NULL, NULL, buf) != 226)
		{
			sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"RETR",pfe->name);
			MessageBox(szMessage,buf,OK);
			return check;
		}
	}
	else if (finfo[curframe].zipfile[0] != 0x00)
	{
		if (tmpzipdir[0] == 0x00)
		while(1)
		{
			char szMessage[400];
			sprintf(szMessage ,msg[MSG_EXTRACT*NUM_LANG+language], finfo[curframe].zipfile);
			MessageBox(szMessage,"",NOBUTTON);

			sprintf(tmpzipdir,"ziptmp%d",i);
			if (FindFile(1-curframe,tmpzipdir) == NULL)
			{
				sprintf(action,"mkdir -p \"%s%s\"",finfo[1-curframe].path, tmpzipdir);
				DoExecute(action, SHOW_NO_OUTPUT);
				break;
			}
			i++;
		}
		int zlen = strlen(szZipCommand);
		if (zlen == 0)
		{
			switch(finfo[curframe].ziptype)
			{
				case GZIP     : tp = 'z'; break;
				case BZIP2    : tp = 'j'; break;
				case COMPRESS : tp = 'Z'; break;
				case TAR      : tp = ' '; break;
				default: return check;
			}
			sprintf(szZipCommand,"tar  -x%c -f \"%s%s\" -C \"%s%s\""
						,tp
						,finfo[curframe].path
						,finfo[curframe].zipfile
						,finfo[1-curframe].path
						,tmpzipdir);
			zlen = strlen(szZipCommand);

		}
		if (S_ISDIR(pfe->fentry.st_mode))
		{
			struct zipfileentry *pzfe = finfo[curframe].allziplist;
			while (pzfe != NULL)
			{
				if (zlen == 0)
				{
					switch(finfo[curframe].ziptype)
					{
						case GZIP     : tp = 'z'; break;
						case BZIP2    : tp = 'j'; break;
						case COMPRESS : tp = 'Z'; break;
						case TAR      : tp = ' '; break;
						default: return check;
					}
					sprintf(szZipCommand,"tar  -x%c -f \"%s%s\" -C \"%s%s\""
								,tp
								,finfo[curframe].path
								,finfo[curframe].zipfile
								,finfo[1-curframe].path
								,tmpzipdir);
					zlen = strlen(szZipCommand);

				}

				sprintf(szFullFile,"%s%s/",finfo[curframe].zippath,szNew);
				if (strncmp(pzfe->name,szFullFile,strlen(szFullFile))== 0)
				{
					int dlen = strlen(pzfe->name)+2;
					if (dlen + zlen >= commandsize)
					{
						DoExecute(szZipCommand, SHOW_NO_OUTPUT);
						szZipCommand[0]=0x00;
						zlen = strlen(szZipCommand);
					}
					else
					{
						sprintf(szFullFile," \"%s\"",(char*)(pzfe->name+1));
						strcat(szZipCommand,szFullFile);
					}
				}
				pzfe = pzfe->next;
			}

		}
		int elen = strlen(pfe->name)+strlen(finfo[curframe].zippath)+2;
		if (elen + zlen >= commandsize)
		{
			DoExecute(szZipCommand, SHOW_NO_OUTPUT);
			szZipCommand[0]=0x00;
		}
		else
		{
			sprintf(szFullFile," \"%s%s\"",(char*)(finfo[curframe].zippath+1),szNew);
			strcat(szZipCommand,szFullFile);
		}
	}
	else
	{
		if (typ != HIDDEN)
		{
			char szMessage[400];
			sprintf(szMessage,msg[MSG_COPY_PROGRESS*NUM_LANG+language], pfe->name, finfo[1-curframe].path);
			MessageBox(szMessage,"",NOBUTTON);
		}
		sprintf(action,"cp -dpR \"%s%s\" \"%s%s\"%s",finfo[curframe].path,pfe->name, finfo[1-curframe].path,szNew,typ == HIDDEN ? " &" : "");
		DoExecute(action, SHOW_NO_OUTPUT);
	}
	return check;
}

/******************************************************************************
 * DoZipCopyEnd                                                               *
 ******************************************************************************/

void DoZipCopyEnd(char* szZipCommand)
{
	if (finfo[curframe].zipfile[0] != 0x00 && finfo[curframe].ftpconn == NULL)
	{
		int zlen = strlen(szZipCommand);
		if (zlen > 0) DoExecute(szZipCommand, SHOW_NO_OUTPUT);
		sprintf(szZipCommand,"mv -f \"%s%s%s\"* \"%s\"",finfo[1-curframe].path,tmpzipdir,finfo[curframe].zippath, finfo[1-curframe].path);
		DoExecute(szZipCommand, SHOW_NO_OUTPUT);
		sprintf(szZipCommand,"rm -f -r \"%s%s\"",finfo[1-curframe].path,tmpzipdir);
		DoExecute(szZipCommand, SHOW_NO_OUTPUT);
	}
}
/******************************************************************************
 * DoMove                                                                     *
 ******************************************************************************/

int DoMove(struct fileentry* pfe, int typ, int checktype)
{
	char action[1000];
	char szMessage[400];
	char szNew[FILENAME_MAX];


	int check = CheckOverwrite(pfe, checktype, szNew);
	if (check < 0 || check == SKIP) return check;

	if (typ != HIDDEN)
	{
		sprintf(szMessage,msg[MSG_MOVE_PROGRESS*NUM_LANG+language], pfe->name, finfo[1-curframe].path, szNew);
		MessageBox(szMessage,"",NOBUTTON);
	}
	sprintf(action,"mv -f \"%s%s\" \"%s%s\"%s",finfo[curframe].path,pfe->name, finfo[1-curframe].path,szNew,typ == HIDDEN ? " &":"");
	DoExecute(action, SHOW_NO_OUTPUT);
	return check;
}
/******************************************************************************
 * DoViewFile                                                                 *
 ******************************************************************************/

void DoViewFile()
{
	char action[4000];
//	FILE* pFile;
	struct fileentry* pfe = GetSelected(curframe);
/*	if (pfe->fentry.st_size >= FILEBUFFER_SIZE)
	{
		if (finfo[curframe].zipfile[0] != 0x00)
		{
			sprintf(action,"tar  -x%cO -f \"%s%s\"  \"%s%s\"",finfo[curframe].ziptype == GZIP ? 'z' :'j',finfo[curframe].path,finfo[curframe].zipfile,(char*)(finfo[curframe].zippath+1),pfe->name);
			pFile = OpenPipe(action);
		}
		else
		{
			sprintf(action,"%s%s",finfo[curframe].path, pfe->name);
			pFile = fopen(action,"r");
		}
		if  (pFile != NULL)
		{
			ShowFile(pFile, pfe->name);
			fclose(pFile);
		}
	}
	else
*/
	{
		if (finfo[curframe].zipfile[0] != 0x00)
		{
			sprintf(action,"tar  -x%cO -f \"%s%s\"  \"%s%s\" > /tmp/tuxcom.out",finfo[curframe].ziptype == GZIP ? 'z' :'j',finfo[curframe].path,finfo[curframe].zipfile,(char*)(finfo[curframe].zippath+1),pfe->name);
			DoExecute(action, SHOW_NO_OUTPUT);
			DoEditFile("/tmp/tuxcom.out",pfe->name,NO);

		}
		else
		{
			sprintf(action,"%s%s",finfo[curframe].path, pfe->name);
			DoEditFile(action,action,NO);
		}
	}

}

/******************************************************************************
 * DoEditFile                                                                 *
 ******************************************************************************/
void InsertText(char* pStart, char* pEnd,char* szText, int sel, int* pcount)
{
	int oldlen = (pEnd ? pEnd-pStart: strlen(pStart));
	int newlen = strlen(szText);
	if (pEnd && szText[newlen-1] != '\n') {szText[newlen] = '\n'; newlen++;}
	int movlen = (pEnd ? (FILEBUFFER_SIZE-newlen <= strlen(pEnd) ? FILEBUFFER_SIZE-newlen-1 : strlen(pEnd)) : 0);
	int step = (sel > 0 && (*(pStart-1) != '\n') ? 1 : 0)+ (szText[newlen-1] != '\n' ? 1 : 0);
	if (pEnd && (oldlen != newlen+step) && movlen > 0)
		memmove((void*)(pStart+newlen+step),pEnd,movlen);
	if (sel > 0 && (*(pStart-1) != '\n')) {*pStart = '\n'; pStart++; (*pcount)++;}
	strncpy(pStart,szText,newlen);
	*(pStart+newlen+step+movlen) = 0x00;
}
void DoEditFile(char* szFile, char* szTitle,  int writable)
{
	FILE* pFile = fopen(szFile,"r");
	char* szFileBuffer = (char*)malloc(FILEBUFFER_SIZE);
	szFileBuffer [0]= 0x00;

	char *p = szFileBuffer, *p1, *pcur = szFileBuffer, *pStart = szFileBuffer, *pStop = NULL, *pMarkStart = NULL,*pMarkStop = NULL, *pMark = NULL;
	char szInputBuffer[1001];
	char szLineNumber[20];
	int count = 0, curcount = 0, totalcount = 0, selstart = -1, selstop = -1;
	int changed = 0;
	int readall = NO;
	int offset;
	long filepos = 0;

	struct stat st;

	offset = 0;
	memset(szFileBuffer,0,FILEBUFFER_SIZE);
	while( fgets( p, FILEBUFFER_SIZE - offset, pFile ) )
	{
	  offset+=strlen(p);
	  count++;
	  if (offset >= FILEBUFFER_SIZE ) break;
	  p = (char*)(p+strlen(p));
	}
	lstat(szFile,&st);

	if (st.st_size < FILEBUFFER_SIZE ) count++;
	totalcount = count;


	if (st.st_size < FILEBUFFER_SIZE)
	{
		readall = YES;
	}

	int i,row = 0, sel = 0, presel=0, strsize, markmode=0;



	while( 1 )
	{
		// Render output window
		RenderBox(               0, 0                          , viewx     , viewy-MENUSIZE             , FILL, trans_map[curvisibility]);
		RenderBox(               0, 0                          , BORDERSIZE, viewy-MENUSIZE             , FILL, WHITE);
		RenderBox(viewx-BORDERSIZE, 0                          , viewx     , viewy-MENUSIZE             , FILL, WHITE);
		RenderBox(               0, 0                          , viewx     , BORDERSIZE                 , FILL, WHITE);
		RenderBox(               0, BORDERSIZE+FONTHEIGHT_BIG  , viewx     , 2*BORDERSIZE+FONTHEIGHT_BIG, FILL, WHITE);
		RenderBox(               0, viewy-BORDERSIZE- MENUSIZE , viewx     , viewy-MENUSIZE             , FILL, WHITE);

		p = szFileBuffer;

		if (st.st_size >= FILEBUFFER_SIZE) // File is to big to fit in Buffer, we have to read next/previous part
		{
			if (sel+framerows >= count  && !feof(pFile)) // read next part
			{
				sel = sel-presel;
				row = sel;
				curcount += presel;
				filepos = ftell(pFile);
				filepos-= (long)((szFileBuffer+FILEBUFFER_SIZE-pStart)-1);
				fseek(pFile,filepos,SEEK_SET);
				count = 0;
				p = szFileBuffer;
				pcur = szFileBuffer;
				memset(szFileBuffer,0,FILEBUFFER_SIZE);
				offset = 0;
				while( fgets( p, FILEBUFFER_SIZE-offset, pFile ) )
				{
				  offset+=strlen(p);
				  count++;
				  if (offset >= FILEBUFFER_SIZE ) break;
				  p = (char*)(p+strlen(p));
				}
				if (readall == NO) totalcount = curcount + count;
				if (feof(pFile)) // EOF reached
					readall = YES;

			}
			if ((sel < 0 ) && (curcount > 0)) // read previous part
			{
				filepos = ftell(pFile);
				filepos-= (long)(strlen(szFileBuffer)+((szFileBuffer+FILEBUFFER_SIZE)-pStop));

				if (filepos < 0 ) filepos = 0;

				fseek(pFile,filepos,SEEK_SET);
				if (filepos > 0)  fgets( szFileBuffer, FILEBUFFER_SIZE, pFile ); // ignore first (probably incomplete) line
				count = 0;
				p = szFileBuffer;
				pcur = szFileBuffer;
				memset(szFileBuffer,0,FILEBUFFER_SIZE);
				offset = 0;
				while( fgets( p, FILEBUFFER_SIZE-offset, pFile ) )
				{
				  offset+=strlen(p);
				  count++;
				  if (offset >= FILEBUFFER_SIZE ) break;
				  p = (char*)(p+strlen(p));
				}
				if (filepos == 0)
				{
					sel = curcount + sel ;
					curcount = 0;
				}
				else
				{
					sel = count + sel - framerows -1;
					curcount -= (count - framerows);
				}
			}
		}



		if (sel >= count) sel = count-1;
		if (sel < 0 ) sel = 0;
		if (sel < row) row = sel;
		if (sel > row+(framerows-2)) row = sel-(framerows-2);
		presel = sel;
		if (writable == NO) row = sel;
		for (i =0; i < row; i++)
		{
          p1=strchr(p,'\n');
          if (p1 == NULL) break;
          p= p1+1;
		}
		sprintf(szLineNumber,msg[MSG_LINE*NUM_LANG+language],curcount+sel+1, totalcount,(readall == YES ? "":"+"));
		strsize = GetStringLen(szLineNumber, BIG);
		RenderString(szTitle     ,2*BORDERSIZE               , BORDERSIZE+FONTHEIGHT_BIG-FONT_OFFSET_BIG  , viewx-5*BORDERSIZE-strsize, LEFT, BIG, WHITE);
		RenderString(szLineNumber,viewx-2*BORDERSIZE-strsize , BORDERSIZE+FONTHEIGHT_BIG-FONT_OFFSET_BIG  , strsize+BORDERSIZE        , RIGHT, BIG, WHITE);

		if ( p )
		{
			pStart = p;
			for (i =0; i < framerows; i++)
			{
				if ((sel == row + i)&& writable!=NO)
				{
					pcur = p;
					RenderBox(BORDERSIZE, 2*BORDERSIZE+FONTHEIGHT_BIG+i*FONTHEIGHT_SMALL , viewx- BORDERSIZE , 2*BORDERSIZE+FONTHEIGHT_BIG+(i+1)*FONTHEIGHT_SMALL, FILL, BLUE2);
					if (markmode)
					{
						if (pcur >= pMark)
						{
							pMarkStart = pMark;
							pMarkStop  = pcur;
						}
						else
						{
							pMarkStop  = pMark;
							pMarkStart = pcur;
						}
					}
				}
				if (pMarkStop != NULL && pMarkStart != NULL && pMarkStop < pMarkStart)
				{
					char* ptmp = pMarkStart;
					pMarkStart = pMarkStop;
					pMarkStop = ptmp;
				}
				if (pMarkStop != NULL && pMarkStart != NULL && pMarkStart <=  p && pMarkStop >= p)
					RenderBox(BORDERSIZE, 2*BORDERSIZE+FONTHEIGHT_BIG+i*FONTHEIGHT_SMALL , viewx- BORDERSIZE , 2*BORDERSIZE+FONTHEIGHT_BIG+(i+1)*FONTHEIGHT_SMALL, FILL, (sel == row + i)? GRAY : GRAY2);

          		RenderString(p,2*BORDERSIZE, 2*BORDERSIZE+FONTHEIGHT_BIG+(i+1)*FONTHEIGHT_SMALL -FONT_OFFSET, viewx-4*BORDERSIZE, LEFT, SMALL, WHITE);
          		p1=strchr(p,'\n');
	            if (p1 == NULL)
	            {
					i++;
					if ((sel == row + i) && writable!=NO)
					{
						pcur+=strlen(pcur);
						RenderBox(BORDERSIZE, 2*BORDERSIZE+FONTHEIGHT_BIG+i*FONTHEIGHT_SMALL , viewx- BORDERSIZE , 2*BORDERSIZE+FONTHEIGHT_BIG+(i+1)*FONTHEIGHT_SMALL, FILL, BLUE2);
					}
					break;
				}
	            p = p1+1;
			}
			pStop = p;
			memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
			while (GetRCCode(RC_NORMAL) == 0);
			switch (rccode)
			{
				case RC_3:
					if (writable == YES)
					{
						if (markmode)
						{
							selstop = sel;
							pMarkStop = pcur;
							pMark = NULL;
						}
						else
						{
							selstart = sel;
							pMarkStart = pcur;
							pMark = pcur;
						}
						markmode=1-markmode;
						RenderMenuLine(markmode ? ACTION_VIEW-1:-1, EDITOR);
					}
					break;
				case RC_5:
					if (writable == YES)
					{
						if (markmode)
						{
							selstop = sel;
							pMarkStop = pcur;
							markmode=1-markmode;
							RenderMenuLine(-1, EDITOR);
						}
						else
						{
							if (pMarkStart && pMarkStop)
							{
								p1 = strchr(pMarkStop,'\n');
								if (p1 == NULL) p1 = pMarkStop+strlen(pMarkStop);
								char * pTmpBuf = (char*)malloc(p1-pMarkStart+2); // temporary buffer for marked segment
								strncpy(pTmpBuf,pMarkStart, p1-pMarkStart);
								*(pTmpBuf+(p1-pMarkStart)) = '\n';
								*(pTmpBuf+(p1-pMarkStart)+1) = 0x00;
								InsertText(pcur, pcur,pTmpBuf, sel, &count);
								count +=(selstop > selstart ?  selstop - selstart : selstart - selstop);
								totalcount = count;
								free(pTmpBuf);
								changed = 1;
							}
						}
					}
					break;
				case RC_6:
					if (writable == YES)
					{
						if (markmode)
						{
							selstop = sel;
							pMarkStop = pcur;
							markmode=1-markmode;
							RenderMenuLine(-1, EDITOR);
						}
						else
						{
							if (pMarkStart && pMarkStop)
							{
								p1 = strchr(pMarkStop,'\n');
								if (p1 == NULL) p1 = pMarkStop+strlen(pMarkStop);
								char * pTmpBuf = (char*)malloc(p1-pMarkStart+2); // temporary buffer for marked segment
								strncpy(pTmpBuf,pMarkStart, p1-pMarkStart);
								*(pTmpBuf+(p1-pMarkStart)) = '\n';
								*(pTmpBuf+(p1-pMarkStart)+1) = 0x00;
								if (pMarkStart > pcur && *(p1+1) != 0x00)
									memmove(pMarkStart,p1+1,FILEBUFFER_SIZE-(p1+1-szFileBuffer));
								int ctmp = count;
								InsertText(pcur, pcur,pTmpBuf, sel, &ctmp);
								free(pTmpBuf);
								if (pMarkStart <= pcur && *(p1+1) != 0x00)
									memmove(pMarkStart,p1+1,FILEBUFFER_SIZE-(p1+1-szFileBuffer));
								if (pMarkStart < pcur) sel -=  (selstop > selstart ?  selstop - selstart : selstart - selstop)+1;
								selstop  = sel + (selstop > selstart ?  selstop - selstart : selstart - selstop);
								selstart = sel;
								if (pMarkStart < pcur)
								{
									int marksize = (p1+1-pMarkStart);
									pMarkStop  = pcur-marksize+(pMarkStop-pMarkStart) + (*(p1+1) != 0x00 ? 1 : 0);
									pMarkStart = pcur-marksize;
								}
								else
								{
									pMarkStop  = pcur+(pMarkStop-pMarkStart);
									pMarkStart = pcur;
								}

								changed = 1;
							}
						}
					}
					break;
				case RC_8:
					if (writable == YES)
					{
						if (markmode)
						{
							selstop = sel;
							pMarkStop = pcur;
							pMark = NULL;
							markmode=1-markmode;
							RenderMenuLine(-1, EDITOR);
						}
						if (pMarkStart && pMarkStop)
						{
							p1 = strchr(pMarkStop,'\n');
							count -= (selstop > selstart ?  selstop - selstart : selstart - selstop)+1;
							totalcount = count;
							if (p1 == NULL) p1 = pMarkStop+strlen(pMarkStop);
							memmove(pMarkStart,p1+1,FILEBUFFER_SIZE-(p1+1-szFileBuffer));
							if (pMarkStart < pcur) sel -=  (selstop > selstart ?  selstop - selstart : selstart - selstop)+1;
							pMarkStart = pMarkStop = NULL;
							changed = 1;
						}
					}
					break;
				case RC_HOME :
					break;
				case RC_OK :
				{
					if (writable == YES)
					{
						if (markmode)
						{
							selstop = sel;
							pMarkStop = pcur;
							pMark = NULL;
							markmode = 0;
							RenderMenuLine(-1, EDITOR);
							break;
						}
						markmode = 0;
						pMarkStart = pMarkStop = NULL;
						p1 = strchr(pcur,'\n');
						if (p1) p1++;
						int plen = (p1 ? p1-pcur: strlen(pcur));
						strncpy(szInputBuffer,pcur,plen);
						szInputBuffer[plen]=0x00;
						RenderBox(0, 2*BORDERSIZE+FONTHEIGHT_BIG+(sel-row)*FONTHEIGHT_SMALL-1 , viewx, 2*BORDERSIZE+FONTHEIGHT_BIG+(sel-row+1)*FONTHEIGHT_SMALL+1, GRID, WHITE);
						switch (DoEditString(BORDERSIZE,2*BORDERSIZE+FONTHEIGHT_BIG+(sel-row)*FONTHEIGHT_SMALL, viewx- 2*BORDERSIZE ,1000,szInputBuffer,BIG/*SMALL*/,BLUE2, NO))
						{
							case RC_OK:
							{
								InsertText(pcur, p1,szInputBuffer, sel, &count);
								changed = 1;
								break;
							}
							default:
								rccode = 0;
								break;
						}
						colortool[0] = ACTION_DELLINE ;
						colortool[1] = ACTION_INSLINE ;
						colortool[2] = ACTION_NOACTION;
						colortool[3] = ACTION_TOLINUX ;
						RenderMenuLine(-1, EDITOR);
					}
					else if (writable == SEARCHRESULT)
					{
						p1 = strchr(pcur,'\n');
						int plen = (p1 ? p1-pcur: strlen(pcur));
						strncpy(szInputBuffer,pcur,plen);
						szInputBuffer[plen]=0x00;
						char* plast = strrchr(szInputBuffer,'/');
						if (plast != NULL)
						{
							plast++;
							char x = *plast;
							*plast=0x00;
							strcpy(finfo[curframe].path, szInputBuffer);
							*plast = x;
							FillDir(curframe,SELECT_NOCHANGE);
							SetSelected(curframe,plast);
						}
						return;
					}
					else
					{
						sel+= framerows-1;
						if (sel < count) row = sel;
					}
					break;
				}
				case RC_UP:
					sel--;
					break;
				case RC_DOWN:
					sel++;
					break;
				case RC_LEFT:
					sel-= framerows-1;
					if (sel >= 0) row = sel;
					break;
				case RC_RIGHT:
					sel+= framerows-1;
					if (sel < count) row = sel;
					break;
				case RC_PLUS:
					sel = 0;
					if (curcount >0)
					{
						fseek(pFile,0,SEEK_SET);
						count = 0;
						p = szFileBuffer;
						pcur = szFileBuffer;
						memset(szFileBuffer,0,FILEBUFFER_SIZE);
						offset = 0;
						while( fgets( p, FILEBUFFER_SIZE-offset, pFile ) )
						{
						  offset+=strlen(p);
						  count++;
						  if (offset >= FILEBUFFER_SIZE ) break;
						  p = (char*)(p+strlen(p));
						}
						curcount = 0;
					}
					break;
				case RC_MINUS:
					sel = totalcount;
					break;
				case RC_RED:
					if (writable == YES)
					{
						p1 = strchr(pcur,'\n');
						if (p1)
						{
							memmove(pcur,p1+1,FILEBUFFER_SIZE-(pcur-szFileBuffer)-1);
						}
						else
							*pcur = 0x00;
						if (count > 0 ) count--;
						if (totalcount > 0 ) totalcount--;
						changed = 1;
					}
					break;
				case RC_GREEN:
					if (writable == YES)
					{
						memmove(pcur+1,pcur,FILEBUFFER_SIZE-(pcur-szFileBuffer)-1);
						*pcur = '\n';
						count++;
						totalcount++;
						changed = 1;
					}
					break;
				case RC_BLUE:
					if (writable == YES)
					{
						p1 = szFileBuffer;
						while (p1)
						{
							p1 = strchr(p1,'\r');
							if (p1)
							{
								memmove(p1,p1+1,FILEBUFFER_SIZE-(p1-szFileBuffer)-1);
								changed = 1;
							}
						}
					}
					break;
			}
			if (rccode == RC_HOME)
			{
				if (changed)
				{
					char szMessage[400];
					sprintf(szMessage,msg[MSG_SAVE*NUM_LANG+language], szFile);
					switch (MessageBox(szMessage,"",YESNOCANCEL))
					{
						case YES:
							fclose(pFile);
							pFile = fopen(szFile,"w");
							if (pFile)
							{
								fputs(szFileBuffer,pFile);
								fclose(pFile);
							}
							free(szFileBuffer);
							rccode = -1;
							return;
						case NO:
							rccode = -1;
							fclose(pFile);
							free(szFileBuffer);
							return;
					}
				}
				else
				{
					rccode = -1;
					fclose(pFile);
					free(szFileBuffer);
					return;
				}

			}
		}
	}
	rccode = -1;
	fclose(pFile);
	free(szFileBuffer);
}
/******************************************************************************
 * DoTaskManager                                                              *
 ******************************************************************************/

void DoTaskManager()
{
	FILE* pFile = OpenPipe("ps -aux");
	char* szFileBuffer = (char*)malloc(FILEBUFFER_SIZE);
	szFileBuffer [0]= 0x00;
	char *p = szFileBuffer, *p1, *p2, *pcur = szFileBuffer;
	char szMsg[2000], procname[256], prid[20]="",uid[100]="";
	int offset = 0, count = 0;
	if (pFile == NULL)
	{
		MessageBox(MSG_VERSION,MSG_COPYRIGHT,OK);
		return;
	}
	memset(szFileBuffer,0,FILEBUFFER_SIZE);
	while( fgets( p, FILEBUFFER_SIZE - offset, pFile ) )
	{
	  if (offset == 0) // ignore first line
	  {
		  offset+=strlen(p);
		  continue;
	  }


	  count++;
	  offset+=strlen(p);
	  if (offset >= FILEBUFFER_SIZE ) break;
	  p = (char*)(p+strlen(p));
	}
	fclose(pFile);


	int i,row = 0, sel = 0, strsize;

	colortool[0] = ACTION_KILLPROC;
	colortool[1] = ACTION_NOACTION;
	colortool[2] = ACTION_NOACTION;
	colortool[3] = ACTION_NOACTION;
	memset(tool, ACTION_NOACTION, sizeof(tool));

	RenderMenuLine(-1, NO);


	while( 1 )
	{
		// Render output window
		RenderBox(               0, 0                          , viewx     , viewy-MENUSIZE             , FILL, trans_map[curvisibility]);
		RenderBox(               0, 0                          , BORDERSIZE, viewy-MENUSIZE             , FILL, WHITE);
		RenderBox(viewx-BORDERSIZE, 0                          , viewx     , viewy-MENUSIZE             , FILL, WHITE);
		RenderBox(               0, 0                          , viewx     , BORDERSIZE                 , FILL, WHITE);
		RenderBox(               0, BORDERSIZE+FONTHEIGHT_BIG  , viewx     , 2*BORDERSIZE+FONTHEIGHT_BIG, FILL, WHITE);
		RenderBox(               0, viewy-BORDERSIZE- MENUSIZE , viewx     , viewy-MENUSIZE             , FILL, WHITE);

		p = szFileBuffer;

		if (sel < 0 ) sel = 0;
		if (sel >= count) sel = count-1;
		if (sel < row) row = sel;
		if (sel > row+(framerows-4)) row = sel-(framerows-4);
		for (i =0; i < row; i++)
		{
          p1=strchr(p,'\n');
          if (p1 == NULL) break;
          p= p1+1;
		}
		strsize = GetStringLen(MSG_COPYRIGHT, BIG);
		RenderString(MSG_VERSION  ,2*BORDERSIZE               , BORDERSIZE+FONTHEIGHT_BIG-FONT_OFFSET_BIG  , viewx-5*BORDERSIZE-strsize, LEFT, BIG, WHITE);
		RenderString(MSG_COPYRIGHT,viewx-2*BORDERSIZE-strsize , BORDERSIZE+FONTHEIGHT_BIG-FONT_OFFSET_BIG  , strsize+BORDERSIZE        , RIGHT, BIG, WHITE);
		RenderString(msg[MSG_PROCESSID  *NUM_LANG+language]   , 2*BORDERSIZE               , 2*BORDERSIZE+FONTHEIGHT_BIG+FONTHEIGHT_SMALL-FONT_OFFSET, viewx/6, RIGHT, SMALL, WHITE);
		RenderString(msg[MSG_PROCESSUSER*NUM_LANG+language]   , 5*BORDERSIZE + viewx/6     , 2*BORDERSIZE+FONTHEIGHT_BIG+FONTHEIGHT_SMALL-FONT_OFFSET, viewx/6, LEFT , SMALL, WHITE);
		RenderString(msg[MSG_PROCESSNAME*NUM_LANG+language]   ,                viewx/3     , 2*BORDERSIZE+FONTHEIGHT_BIG+FONTHEIGHT_SMALL-FONT_OFFSET, viewx/3, LEFT , SMALL, WHITE);
		RenderBox(               0, BORDERSIZE+FONTHEIGHT_BIG + 2*FONTHEIGHT_SMALL-FONT_OFFSET  , viewx     , 2*BORDERSIZE+FONTHEIGHT_BIG + 2*FONTHEIGHT_SMALL-FONT_OFFSET, FILL, WHITE);

		if ( p )
		{
			for (i =0; i < framerows-2; i++)
			{
				if (sel == row + i)
				{
					pcur = p;
					RenderBox(BORDERSIZE, 2*BORDERSIZE+FONTHEIGHT_BIG+2*FONTHEIGHT_SMALL+i*FONTHEIGHT_SMALL , viewx- BORDERSIZE , 2*BORDERSIZE+FONTHEIGHT_BIG+2*FONTHEIGHT_SMALL+(i+1)*FONTHEIGHT_SMALL, FILL, BLUE2);
				}
				if (*p == 0x00) break;
				sscanf(p,"%s%s",prid,uid);

				memset(procname,0,256);
				strncpy(procname,(char*)(p+26),255);
          		p2=strchr(procname,'\n');
          		if (p2 != NULL) *p2 = 0x00;
          		RenderString(    prid,2*BORDERSIZE           , 2*BORDERSIZE+FONTHEIGHT_BIG+2*FONTHEIGHT_SMALL+(i+1)*FONTHEIGHT_SMALL -FONT_OFFSET,   viewx/6 , RIGHT, SMALL, WHITE);
          		RenderString(     uid,5*BORDERSIZE+  viewx/6 , 2*BORDERSIZE+FONTHEIGHT_BIG+2*FONTHEIGHT_SMALL+(i+1)*FONTHEIGHT_SMALL -FONT_OFFSET,   viewx/6 , LEFT , SMALL, WHITE);
          		RenderString(procname,               viewx/3 , 2*BORDERSIZE+FONTHEIGHT_BIG+2*FONTHEIGHT_SMALL+(i+1)*FONTHEIGHT_SMALL -FONT_OFFSET, 2*viewx/3 , LEFT , SMALL, WHITE);
          		p1=strchr(p,'\n');
	            if (p1 == NULL)
	            {
					i++;
					if (sel == row + i)
					{
						pcur+=strlen(pcur);
						RenderBox(BORDERSIZE, 2*BORDERSIZE+FONTHEIGHT_BIG+2*FONTHEIGHT_SMALL+i*FONTHEIGHT_SMALL , viewx- BORDERSIZE , 2*BORDERSIZE+FONTHEIGHT_BIG+2*FONTHEIGHT_SMALL+(i+1)*FONTHEIGHT_SMALL, FILL, BLUE2);
					}
					break;
				}
	            p = p1+1;
			}
			RenderBox(  viewx/6 +3*BORDERSIZE, BORDERSIZE+FONTHEIGHT_BIG  ,   viewx/6 + 4*BORDERSIZE, viewy-MENUSIZE             , FILL, WHITE);
			RenderBox(  viewx/3 -2*BORDERSIZE, BORDERSIZE+FONTHEIGHT_BIG  ,   viewx/3 -   BORDERSIZE, viewy-MENUSIZE             , FILL, WHITE);
			memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
			while (GetRCCode(RC_NORMAL) == 0);
			switch (rccode)
			{
				case RC_HOME :
					break;
				case RC_UP:
					sel--;
					break;
				case RC_DOWN:
					sel++;
					break;
				case RC_LEFT:
					sel-= framerows-3;
					if (sel >= 0) row = sel;
					break;
				case RC_RIGHT:
					sel+= framerows-3;
					if (sel < count) row = sel;
					break;
				case RC_PLUS:
					sel = 0;
					break;
				case RC_MINUS:
					sel = count;
					break;
				case RC_RED:
					sscanf(pcur,"%s%s",prid,uid);
					memset(procname,0,256);
					strncpy(procname,(char*)(pcur+26),255);
	          		p2=strchr(procname,'\n');
	          		if (p2 != NULL) *p2 = 0x00;
					sprintf(szMsg,msg[MSG_KILLPROC*NUM_LANG+language],procname);
					if (MessageBox(szMsg, info[INFO_PROC*NUM_LANG+language],OKCANCEL) == OK)
					{
						char szCmd[2000];
						sprintf(szCmd,"kill -9 %s", prid);
						system(szCmd);
						FILE* pFile = OpenPipe("ps -aux");
						if (pFile == NULL)
						{
							MessageBox(MSG_VERSION,MSG_COPYRIGHT,OK);
							return;
						}
						sel = 0;
						p = szFileBuffer;
						count = 0;
						memset(szFileBuffer,0,FILEBUFFER_SIZE);
						while( fgets( p, FILEBUFFER_SIZE - offset, pFile ) )
						{
						  count++;
						  if (offset == 0) // ignore first line
						  {
							  offset+=strlen(p);
							  continue;
						  }
						  offset+=strlen(p);
						  count++;
						  if (offset >= FILEBUFFER_SIZE ) break;
						  p = (char*)(p+strlen(p));
						}
						fclose(pFile);
					}
					break;

			}
			if (rccode == RC_HOME)
			{
				rccode = -1;
				free(szFileBuffer);
				return;
			}
		}
	}
	free(szFileBuffer);
	rccode = -1;
}
/******************************************************************************
 * DoExecute                                                                  *
 ******************************************************************************/

void DoExecute(char* szAction, int showoutput)
{
	printf("executing: %s\n", szAction);

	if (showoutput == SHOW_NO_OUTPUT)
	{
		int x = system(szAction);
		printf("result %d \n",x);

		if (x != 0)
			MessageBox("Error",strerror(x),OK);
	}
	else
	{
		FILE* pipe = OpenPipe(szAction);
		if (pipe== NULL)
		{
			printf("tuxcom: could not open pipe\n");
			char message[1000];
			sprintf(message,msg[MSG_EXEC_NOT_POSSIBLE*NUM_LANG+language],szAction);
			MessageBox(message,"",OK);

			return;
		}
//		ShowFile(pipe, szAction);
		fclose(pipe);
		DoEditFile("/tmp/tuxcom.out", (showoutput== SHOW_SEARCHRESULT ? info[INFO_SEARCH2*NUM_LANG+language] : szAction) ,(showoutput== SHOW_SEARCHRESULT ? SEARCHRESULT : NO));
	}
	rccode = -1;
}
/******************************************************************************
 * CheckZip                                                                   *
 ******************************************************************************/

int CheckZip(char* szName)
{
	int len = strlen(szName);
	if (len < 4) return -1;
	if (strcmp((char*)(szName+len-4),".tar") == 0) return TAR;
	if (strcmp((char*)(szName+len-4),".ftp") == 0) return FTP;
	if (len < 6) return -1;
	if (strcmp((char*)(szName+len-6),".tar.Z") == 0) return COMPRESS;
	if (len < 7) return -1;
	if (strcmp((char*)(szName+len-7),".tar.gz") == 0) return GZIP;
	if (len < 8) return -1;
	if (strcmp((char*)(szName+len-8),".tar.bz2") == 0) return BZIP2;
	return -1;
}
/******************************************************************************
 * ReadZip                                                                    *
 ******************************************************************************/

void ReadZip(int typ)
{
	if (typ == FTP) { OpenFTP(); return;}
	MessageBox(msg[MSG_READ_ZIP_DIR*NUM_LANG+language],"",NOBUTTON);
	FILE* pipe;
	char szAction[400], szLine[400], name[FILENAME_MAX];
	char* p;
	char d,r,w,x;
	struct zipfileentry* pzfe1 = NULL, *pzfe2 = NULL;
	long size=0;

	ClearZipEntries(curframe);
	if      (typ == GZIP)
		sprintf(szAction,"tar -tzv -f \"%s%s\"",finfo[curframe].path,GetSelected(curframe)->name);
	else if (typ == BZIP2)
		sprintf(szAction,"tar -tjv -f \"%s%s\"",finfo[curframe].path,GetSelected(curframe)->name);
	else if (typ == COMPRESS)
		sprintf(szAction,"tar -tZv -f \"%s%s\"",finfo[curframe].path,GetSelected(curframe)->name);
	else if (typ == TAR)
		sprintf(szAction,"tar -tv -f \"%s%s\"",finfo[curframe].path,GetSelected(curframe)->name);
	else
	    return;
	strcpy(finfo[curframe].zipfile,GetSelected(curframe)->name);
	strcpy(finfo[curframe].zippath,"/");
	finfo[curframe].ziptype = typ;
	pipe = OpenPipe(szAction);
	if (pipe== NULL)
	{
		printf("tuxcom: could not open pipe\n");
		char message[1000];
		sprintf(message,msg[MSG_EXEC_NOT_POSSIBLE*NUM_LANG+language],szAction);
		MessageBox(message,"",OK);

		return;
	}
	while( fgets( szLine, 400, pipe ) )
	{
		p=strchr(szLine,'\n');
		if ( p )
			*p=0;
		sscanf(szLine,"%c%c%c%c%*s%*s%lu%*s%*s%s",&d,&r,&w,&x,&size,name);


		if (name[0] != 0x00)
		{
			pzfe1 = malloc(sizeof(struct zipfileentry));

			if (name[strlen(name)-1] == '/') name[strlen(name)-1]=0x00;
			if (name[0] != '/')
				sprintf(pzfe1->name,"/%s",name);
			else
				strcpy(pzfe1->name,name);
			pzfe1->next = NULL;
			memset(&pzfe1->fentry, 0, sizeof(struct stat));
			pzfe1->fentry.st_size = size;
			if (d == 'd') { pzfe1->fentry.st_mode |= S_IFDIR;pzfe1->fentry.st_size = 0;}
			if (r == 'r')   pzfe1->fentry.st_mode |= S_IRUSR;
			if (w == 'w')   pzfe1->fentry.st_mode |= S_IWUSR;
			if (x == 'x')   pzfe1->fentry.st_mode |= S_IXUSR;

			if (pzfe2 == NULL)
			  finfo[curframe].allziplist = pzfe1;
			else
			  pzfe2->next = pzfe1;
			pzfe2 = pzfe1;
		}
	}


	fclose(pipe);

}

/******************************************************************************
 * OpenFTP                                                                   *
 ******************************************************************************/

void OpenFTP()
{
	// read connection info from selected file
	FILE *fp;
	char *p, *p1;
	char line[256];
	char file[FILENAME_MAX];
	char dir [FILENAME_MAX];

	strcpy(dir,"/");
	strcpy(finfo[curframe].ftpuser,"anonymous");
	strcpy(finfo[curframe].ftppass,"tuxcom");
	finfo[curframe].ftpport = 21;
	sprintf(file,"%s%s",finfo[curframe].path,GetSelected(curframe)->name);
	fp = fopen( file, "r" );
	if ( !fp )
	{
		printf("tuxcom: could not open ftpfile\n");
	}
	else
	{
		while( fgets( line, 128, fp ) )
		{
			if ( *line == '#' )	continue;
			if ( *line == ';' )	continue;
			p=strchr(line,'\n');
			if ( p )
				*p=0;
			p=strchr(line,'=');
			if ( !p )
				continue;
			*p=0;
			p++;
			p1=strchr(p,'\r'); // für Windows-Nutzer: '\r' überlesen
			if (p1 != NULL) *p1 = 0x00;
			if      ( !strcmp(line,"host") ) strcpy(finfo[curframe].ftphost, p);
			else if ( !strcmp(line,"port") ) finfo[curframe].ftpport = atoi(p);
			else if ( !strcmp(line,"user") ) strcpy(finfo[curframe].ftpuser, p);
			else if ( !strcmp(line,"pass") ) strcpy(finfo[curframe].ftppass, p);
			else if ( !strcmp(line,"dir" ) ) strcpy(dir, p);
		}
		fclose(fp);
	}
	MessageBox(msg[MSG_FTP_CONN*NUM_LANG+language],finfo[curframe].ftphost,NOBUTTON);


	// try to connect to ftp-server
	struct hostent *he;
	char szMessage[400];

	memset(&finfo[curframe].s_in, 0, sizeof(struct sockaddr_in));
	finfo[curframe].s_in.sin_family = AF_INET;
	he = gethostbyname(finfo[curframe].ftphost);
	if (he == NULL)
	{
		MessageBox(msg[MSG_FTP_NOCONN*NUM_LANG+language],finfo[curframe].ftphost,OK);
		return;
	}
	memcpy(&(finfo[curframe].s_in.sin_addr), he->h_addr_list[0], he->h_length);

	finfo[curframe].s_in.sin_port = finfo[curframe].ftpport;


	int sControl = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(sControl, (struct sockaddr *)&finfo[curframe].s_in, sizeof(struct sockaddr_in)) < 0)
	{
		MessageBox(msg[MSG_FTP_NOCONN*NUM_LANG+language],finfo[curframe].ftphost,OK);
		return;
	}
	sprintf(szMessage, "%s[%s]:%d",finfo[curframe].ftphost,inet_ntoa(finfo[curframe].s_in.sin_addr), ntohs(finfo[curframe].s_in.sin_port));
	MessageBox(msg[MSG_FTP_CONN*NUM_LANG+language],szMessage,NOBUTTON);

	finfo[curframe].ftpconn = fdopen(sControl, "r+");
	if (finfo[curframe].ftpconn == NULL)
	{
		MessageBox("open stream failure",finfo[curframe].ftphost,OK);
		return;
	}
	char buf[512];


	if (FTPcmd(curframe, NULL, NULL, buf) != 220)
	{
		fclose(finfo[curframe].ftpconn);
		finfo[curframe].ftpconn = NULL;
		MessageBox(msg[MSG_FTP_NOCONN*NUM_LANG+language],finfo[curframe].ftphost,OK);
		return;
	}

	switch (FTPcmd(curframe, "USER ", finfo[curframe].ftpuser, buf))
	{
		case 230:
			break;
		case 331:
			if (FTPcmd(curframe, "PASS ", finfo[curframe].ftppass, buf) != 230)
			{
				fclose(finfo[curframe].ftpconn);
				finfo[curframe].ftpconn = NULL;
				sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"PASS","");
				MessageBox(szMessage,buf,OK);
				return;
			}
			break;
		default:
			fclose(finfo[curframe].ftpconn);
			finfo[curframe].ftpconn = NULL;
			sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"USER","");
			MessageBox(szMessage,buf,OK);
			return;
	}
	sprintf(finfo[curframe].zippath,"%s%s%s",(dir[0] != '/' ? "/":""), dir, (dir[strlen(dir)-1] != '/' ? "/":""));
	strcpy(finfo[curframe].zipfile,GetSelected(curframe)->name);
}

/******************************************************************************
 * ReadFTPDir                                                                 *
 ******************************************************************************/

void ReadFTPDir(int frame, char* seldir)
{
	if (finfo[frame].ftpconn == NULL) return;
	struct zipfileentry * pzfe = finfo[frame].allziplist;
	char buf[512], *p, name[FILENAME_MAX];
	char d,r,w,x;
	char szMessage[400];
	struct sockaddr_in s_inlist;
	struct zipfileentry* pzfe1 = NULL, *pzfe2 = finfo[frame].allziplist;
	char szDir[2000];
	sprintf(szDir,"%s.",finfo[frame].zippath);
	if (strcmp(seldir,"..") == 0)
	{
		if (FTPcmd(frame, "CDUP", NULL, buf) != 250)
		{
			sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"CDUP","");
			MessageBox(szMessage,buf,OK);
			return;
		}
	}
	while (pzfe != NULL)
	{
		if (strcmp(pzfe->name,szDir) == 0) return; // directory already read
		pzfe = pzfe->next;
	}

	long size = 0;
	memcpy(&s_inlist,&finfo[frame].s_in,sizeof(struct sockaddr_in));

	MessageBox(msg[MSG_FTP_READDIR*NUM_LANG+language],seldir,NOBUTTON);

	FTPcmd(frame, "TYPE A", NULL, buf);

	if (*seldir != 0x00 && strcmp(seldir,"..") != 0)
	{
		if (FTPcmd(frame, "CWD ", seldir, buf) != 250)
		{
			sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"CWD ",seldir);
			MessageBox(szMessage,buf,OK);
			return;
		}
	}


	if (FTPcmd(frame, "PASV", NULL, buf) != 227)
	{
		fclose(finfo[frame].ftpconn);
		finfo[frame].ftpconn = NULL;
		sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"PASV","");
		MessageBox(szMessage,buf,OK);
		return;
	}
    char* s = strrchr(buf, ',');
	*s = 0;
	int port = atoi(s+1);
	s = strrchr(buf, ',');
	port += atoi(s+1) * 256;
	s_inlist.sin_port = htons(port);
	int sControl = socket(AF_INET, SOCK_STREAM, 0);
	connect(sControl, (struct sockaddr *)&s_inlist, sizeof(struct sockaddr_in));
	FILE* fData = fdopen(sControl, "r+");


	FTPcmd(frame, "LIST", NULL, buf);


	if (fData == NULL)
	{
		sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"LIST","");
		MessageBox(szMessage,"open stream failure",OK);
		return;
	}
	pzfe1 = malloc(sizeof(struct zipfileentry));

	strcpy(pzfe1->name,szDir);
	pzfe1->next = pzfe2;
	memset(&pzfe1->fentry, 0, sizeof(struct stat));
	finfo[frame].allziplist = pzfe1;
	pzfe2 = pzfe1;

	do
	{
		if (fgets(buf, 510,fData) == NULL) break;
		p=strchr(buf,'\n');
		if ( p )
			*p=0;
		sscanf(buf,"%c%c%c%c%*s%*d%*s%*s%lu%*s%*s%*s%[^\n]s",&d,&r,&w,&x,&size,name);


		if (name[0] != 0x00)
		{
			char* pname = name;
			while (*pname == ' ') pname++;
			while (!isspace(pname[strlen(pname)-1])) pname[strlen(pname)-1] = 0x00;

			if (pname[0] == '/') pname++;
			if (pname[strlen(pname)-1] == '/') pname[strlen(pname)-1]=0x00;
			if (strcmp(pname,"..") == 0) continue;
			pzfe1 = malloc(sizeof(struct zipfileentry));
			sprintf(pzfe1->name,"%s%s",finfo[frame].zippath,pname);
			pzfe1->next = pzfe2;
			memset(&pzfe1->fentry, 0, sizeof(struct stat));
			pzfe1->fentry.st_size = size;
			if (d == 'd') { pzfe1->fentry.st_mode |= S_IFDIR;pzfe1->fentry.st_size = 0;}
			if (d == 'l') { pzfe1->fentry.st_mode |= S_IFLNK;pzfe1->fentry.st_size = 0;}
			if (r == 'r')   pzfe1->fentry.st_mode |= S_IRUSR;
			if (w == 'w')   pzfe1->fentry.st_mode |= S_IWUSR;
			if (x == 'x')   pzfe1->fentry.st_mode |= S_IXUSR;

			finfo[frame].allziplist = pzfe1;
			pzfe2 = pzfe1;
		}
	} while (! isdigit(buf[0]) || buf[3] != ' ');
	fclose(fData);
	if (FTPcmd(frame, NULL, NULL, buf) != 226)
	{
		sprintf(szMessage, msg[MSG_FTP_ERROR*NUM_LANG+language],"LIST","");
		MessageBox(szMessage,buf,OK);
		return;
	}

}

/******************************************************************************
 * FTPcmd                                                                     *
 ******************************************************************************/

int FTPcmd(int frame, const char *s1, const char *s2, char *buf)
{
	if (s1) {
		if (s2) {
			fprintf(finfo[frame].ftpconn, "%s%s\n", s1, s2);
		} else {
			fprintf(finfo[frame].ftpconn, "%s\n", s1);
		}
		fflush(finfo[frame].ftpconn);
	}
	do {
		if ((fgets(buf, 510,finfo[curframe].ftpconn)) == NULL) {
			return -1;
		}
	} while (! isdigit(buf[0]) || buf[3] != ' ');

	return atoi(buf);
}

/******************************************************************************
 * ShowFile                                                                   *
 ******************************************************************************/

void ShowFile(FILE* pipe, char* szAction)
{
	// Code from splugin (with little modifications...)
	char *p;
	char line[256];



	// Render output window
	RenderBox(               0, 0                          , viewx     , viewy-MENUSIZE             , FILL, trans_map[curvisibility]);
	RenderBox(               0, 0                          , BORDERSIZE, viewy-MENUSIZE             , FILL, WHITE);
	RenderBox(viewx-BORDERSIZE, 0                          , viewx     , viewy-MENUSIZE             , FILL, WHITE);
	RenderBox(               0, 0                          , viewx     , BORDERSIZE                 , FILL, WHITE);
	RenderBox(               0, BORDERSIZE+FONTHEIGHT_BIG  , viewx     , 2*BORDERSIZE+FONTHEIGHT_BIG, FILL, WHITE);
	RenderBox(               0, viewy-BORDERSIZE- MENUSIZE , viewx     , viewy-MENUSIZE             , FILL, WHITE);
	RenderString(szAction,2*BORDERSIZE, BORDERSIZE+FONTHEIGHT_BIG-FONT_OFFSET_BIG  , viewx-4*BORDERSIZE, CENTER, BIG, WHITE);

	int row = 0;
	while( fgets( line, 128, pipe ) )
	{
		p=strchr(line,'\n');
		if ( p )
			*p=0;
		row++;
		RenderString(line,2*BORDERSIZE, 2*BORDERSIZE+FONTHEIGHT_BIG+row*FONTHEIGHT_SMALL -FONT_OFFSET, viewx-4*BORDERSIZE, LEFT, SMALL, WHITE);

		if (row > framerows - 2)
		{
			memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
			while (1)
			{
				GetRCCode(RC_NORMAL);
				if (rccode == RC_HOME || rccode == RC_OK || rccode == RC_RIGHT)
					break;
			}
			row = 0;
			if (rccode == RC_HOME) break;
			// Render output window
			RenderBox(               0, 0                          , viewx     , viewy-MENUSIZE             , FILL, trans_map[curvisibility]);
			RenderBox(               0, 0                          , BORDERSIZE, viewy-MENUSIZE             , FILL, WHITE);
			RenderBox(viewx-BORDERSIZE, 0                          , viewx     , viewy-MENUSIZE             , FILL, WHITE);
			RenderBox(               0, 0                          , viewx     , BORDERSIZE                 , FILL, WHITE);
			RenderBox(               0, BORDERSIZE+FONTHEIGHT_BIG  , viewx     , 2*BORDERSIZE+FONTHEIGHT_BIG, FILL, WHITE);
			RenderBox(               0, viewy-BORDERSIZE- MENUSIZE , viewx     , viewy-MENUSIZE             , FILL, WHITE);
			RenderString(szAction,2*BORDERSIZE, BORDERSIZE+FONTHEIGHT_BIG-FONT_OFFSET_BIG  , viewx-4*BORDERSIZE, CENTER, BIG, WHITE);
		}
	}
	if (row>0)
	{
		memcpy(lfb, lbb, fix_screeninfo.line_length*var_screeninfo.yres);
		while (1)
		{
			GetRCCode(RC_NORMAL);
			if (rccode == RC_HOME || rccode == RC_OK)
				break;
		}
	}
	rccode = -1;
}

/******************************************************************************
 * SetPassword                                                                *
 ******************************************************************************/

void SetPassword()
{
	char szP1[20];
	char szP2[20];
	*szP1 = 0x00;
	*szP2 = 0x00;
	if (GetInputString(150,19,szP1,info[INFO_PASS2*NUM_LANG+language], NO) != RC_OK) return;
	if (GetInputString(150,19,szP2,info[INFO_PASS3*NUM_LANG+language], NO) != RC_OK) return;
	if (strcmp(szP1,szP2) == 0)
	{
		strcpy(szPass,szP1);
		MessageBox(info[INFO_PASS4*NUM_LANG+language],"",OK);
	}

}

/******************************************************************************
 * OpenPipe                                                                   *
 ******************************************************************************/

FILE* OpenPipe(char* szAction)
{
	FILE *pipe;
	char szCommand[4000];
	system("rm -f /tmp/tuxcom.out");
	sprintf(szCommand,"%s > /tmp/tuxcom.out",szAction);
	system(szCommand);
	pipe = fopen("/tmp/tuxcom.out","r");
	return pipe;
}

/******************************************************************************
 * GetSizeString                                                              *
 ******************************************************************************/

void GetSizeString(char* sizeString, unsigned long long size)
{
	unsigned long long tmp = size;
	char sztmp[100];
	*sztmp = 0x00;


	while (tmp > 1000)
	{
		sprintf(sizeString,".%03lu%s",(unsigned long)(tmp % (unsigned long long)1000), sztmp);
		strcpy(sztmp,sizeString);
		tmp /= (unsigned long long)1000;
	}
	sprintf(sizeString,"%lu%s",(unsigned long)tmp,sztmp);

}

/******************************************************************************
 * ReadSettings                                                               *
 ******************************************************************************/

void ReadSettings()
{
	FILE *fp;
	char *p;
	char line[256];
	char lfile[256] = "";
	char rfile[256] = "";
	int  lfirst = 0;
	int  rfirst = 0;

	printf("tuxcom: reading settings \n");

	finfo[LEFTFRAME].sort = SORT_UP;
	finfo[RIGHTFRAME].sort = SORT_UP;

	fp = fopen( "/etc/tuxbox/tuxcom.conf", "r" );
	if ( !fp )
	{
		printf("tuxcom: could not open /etc/tuxbox/tuxcom.conf !!!\n");
	}
	else
	{
		while( fgets( line, 128, fp ) )
		{
			if ( *line == '#' )
				continue;
			if ( *line == ';' )
				continue;
			p=strchr(line,'\n');
			if ( p )
				*p=0;
			p=strchr(line,'=');
			if ( !p )
				continue;
			*p=0;
			p++;
			if ( !strcmp(line,"version") )
			{
				continue;
			}
			else if ( !strcmp(line,"curframe") )
			{
				curframe = atoi(p);
			}
			else if ( !strcmp(line,"curvisibility") )
			{
				curvisibility = atoi(p);
			}
			else if ( !strcmp(line,"ldir") )
			{
				strcpy(finfo[LEFTFRAME].path, p);
			}
			else if ( !strcmp(line,"rdir") )
			{
				strcpy(finfo[RIGHTFRAME].path, p);
			}
			else if ( !strcmp(line,"lsort") )
			{
				finfo[LEFTFRAME].sort = atoi(p);
				if (finfo[LEFTFRAME].sort == 0) finfo[LEFTFRAME].sort = SORT_UP;

			}
			else if ( !strcmp(line,"rsort") )
			{
				finfo[RIGHTFRAME].sort = atoi(p);
				if (finfo[RIGHTFRAME].sort == 0) finfo[RIGHTFRAME].sort = SORT_UP;
			}
			else if ( !strcmp(line,"screenmode") )
			{
				screenmode = atoi(p)%2;
			}
			else if ( !strcmp(line,"lfile") )
			{
				strcpy(lfile, p);
			}
			else if ( !strcmp(line,"rfile") )
			{
				strcpy(rfile, p);
			}
			else if ( !strcmp(line,"lfirst") )
			{
				lfirst = atoi(p);
			}
			else if ( !strcmp(line,"rfirst") )
			{
				rfirst = atoi(p);
			}
			else if ( !strcmp(line,"clip") )
			{
				strcpy(szClipboard, p);
			}
			else if ( !strcmp(line,"pass") )
			{
				strcpy(szPass, p);
			}
			else if ( !strcmp(line,"search") )
			{
				strcpy(szSearchstring, p);
			}
			else if ( !strcmp(line,"langselect") )
			{
				langselect = atoi(p);
			}
			else if ( !strcmp(line,"autosave") )
			{
				autosave = atoi(p);
			}
			else if ( !strcmp(line,"searchtext") )
			{
				strcpy(szTextSearchstring, p);
			}
		}
		fclose(fp);
	}
	FillDir(1-curframe,SELECT_NOCHANGE);
	FillDir(  curframe,SELECT_NOCHANGE);
	if (lfirst < finfo[LEFTFRAME ].count) finfo[LEFTFRAME ].first = lfirst;
	if (rfirst < finfo[RIGHTFRAME].count) finfo[RIGHTFRAME].first = rfirst;
	SetSelected(LEFTFRAME ,lfile);
	SetSelected(RIGHTFRAME ,rfile);

}
/******************************************************************************
 * WriteSettings                                                              *
 ******************************************************************************/

void WriteSettings()
{

	FILE *fp;


	fp = fopen( "/etc/tuxbox/tuxcom.conf", "w" );
	if ( !fp )
	{
		printf("tuxcom: could not open /etc/tuxbox/tuxcom.conf !!!\n");
	}
	else
	{
		fprintf(fp,"version=%d\n", INI_VERSION);
		fprintf(fp,"curframe=%d\n", curframe);
		fprintf(fp,"curvisibility=%d\n", curvisibility);
		fprintf(fp,"ldir=%s\n",finfo[LEFTFRAME ].path);
		fprintf(fp,"rdir=%s\n",finfo[RIGHTFRAME].path);
		fprintf(fp,"lsort=%d\n",finfo[LEFTFRAME ].sort);
		fprintf(fp,"rsort=%d\n",finfo[RIGHTFRAME].sort);
		fprintf(fp,"screenmode=%d\n",screenmode);
		fprintf(fp,"lfile=%s\n",(finfo[LEFTFRAME ].zipfile[0] != 0x00 ? finfo[LEFTFRAME ].zipfile : GetSelected(LEFTFRAME )->name));
		fprintf(fp,"rfile=%s\n",(finfo[RIGHTFRAME].zipfile[0] != 0x00 ? finfo[RIGHTFRAME].zipfile : GetSelected(RIGHTFRAME)->name));
		fprintf(fp,"lfirst=%lu\n",finfo[LEFTFRAME ].first);
		fprintf(fp,"rfirst=%lu\n",finfo[RIGHTFRAME].first);
		fprintf(fp,"clip=%s\n",szClipboard);
		fprintf(fp,"pass=%s\n",szPass);
		fprintf(fp,"search=%s\n",szSearchstring);
		fprintf(fp,"langselect=%d\n",langselect);
		fprintf(fp,"autosave=%d\n",autosave);
		fprintf(fp,"searchtext=%s\n",szTextSearchstring);
		fclose(fp);
	}
}
