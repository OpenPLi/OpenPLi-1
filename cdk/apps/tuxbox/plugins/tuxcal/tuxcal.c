/******************************************************************************
 *                        <<< TuxCal - Calendar Plugin >>>
 *                (c) Robert "robspr1" Spreitzer 2006 (robert.spreitzer@inode.at)
 *  
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *-----------------------------------------------------------------------------
 * $Log: tuxcal.c,v $
 * Revision 1.08  2007/05/17 16:19:47  dbluelle
 * Make plugins compile with freeetype 2.1.x on dreambox (as needed for Neutrino on Dreambox)
 *
 * Revision 1.07  2006/03/05 15:59:37  robspr1
 * - use /tmp/keyboard.lck to signal decoding of the keyboard
 *
 * Revision 1.06  2006/02/24 08:13:29  robspr1
 * - bugfix Editor background
 *
 * Revision 1.05  2006/02/23 23:07:25  robspr1
 * - change SKIN2, signal up to 5 days, toggle clock-display file
 *
 * Revision 1.04  2006/02/18 14:57:13  robspr1
 * add signaling at fixed times, some small fixes
 *
 * Revision 1.03  2006/02/17 21:29:36  robspr1
 * -add command to switch/hide the clock
 *
 * Revision 1.02  2006/02/15 19:11:33  robspr1
 * first version in CVS
 *
 * Revision 1.01  2006/02/12 23:10:00  robspr1
 * - bugfix reading params POS_X and POS_Y (needed by the daemon)
 *
 * Revision 1.00  2006/02/06 20:00:00  robspr1
 * - first version
 ******************************************************************************/
// lots of code is from the tuxmail-project

#include "tuxcal.h"


/******************************************************************************
 * ReadConf
 ******************************************************************************/
/*!
 * read configuration-file 

 \param			: none
 \return 		: none
*/
void ReadConf()
{
	FILE *fd_conf;
	char *ptr;
	char line_buffer[256];

	// open config-file
	if (!(fd_conf = fopen(CFGPATH CFGFILE, "r")))
	{
		printf("TuxCal <Config not found, using defaults>\n");
		return;
	}

	// read config-file line-by-line
	while(fgets(line_buffer, sizeof(line_buffer), fd_conf))
	{
		if ((ptr = strstr(line_buffer, "STARTDELAY=")))
		{
			sscanf(ptr + 11, "%d", &startdelay);
		}
		else if((ptr = strstr(line_buffer, "INTERVALL=")))
		{
			sscanf(ptr + 10, "%d", &intervall);
		}
		else if((ptr = strstr(line_buffer, "LOGGING=")))
		{
			sscanf(ptr + 8, "%c", &logging);
		}
		else if((ptr = strstr(line_buffer, "AUDIO=")))
		{
			sscanf(ptr + 6, "%c", &audio);
		}
		else if((ptr = strstr(line_buffer, "VIDEO=")))
		{
			sscanf(ptr + 6, "%d", &video);
		}
		else if((ptr = strstr(line_buffer, "SIGNAL=")))
		{
			sscanf(ptr + 7, "%d", &sigtype);
		}
		else if((ptr = strstr(line_buffer, "SIGMODE=")))
		{
			sscanf(ptr + 8, "%d", &sigmode);
		}
		else if((ptr = strstr(line_buffer, "SIGTIME=")))
		{
			sscanf(ptr + 8, "%s", &sigtime[0]);
		}
		else if((ptr = strstr(line_buffer, "OSD=")))
		{
			sscanf(ptr + 4, "%c", &osd);
		}
		else if((ptr = strstr(line_buffer, "SKIN=")))
		{
			sscanf(ptr + 5, "%d", &skin);
		}
		else if((ptr = strstr(line_buffer, "WEBPORT=")))
		{
			sscanf(ptr + 8, "%d", &webport);
		}
		else if((ptr = strstr(line_buffer, "WEBUSER=")))
		{
			sscanf(ptr + 8, "%s", &webuser[0]);
		}
		else if((ptr = strstr(line_buffer, "WEBPASS=")))
		{
			sscanf(ptr + 8, "%s", &webpass[0]);
		}
		else if((ptr = strstr(line_buffer, "POS_X=")))
		{
			sscanf(ptr + 6, "%d", &cstartx);
		}
		else if((ptr = strstr(line_buffer, "POS_Y=")))
		{
			sscanf(ptr + 6, "%d", &cstarty);
		}
		else if((ptr = strstr(line_buffer, "SHOW=")))
		{
			sscanf(ptr + 5, "%c", &show_clock);
		}
		else if((ptr = strstr(line_buffer, "DATE=")))
		{
			sscanf(ptr + 5, "%c", &disp_date);
		}
		else if((ptr = strstr(line_buffer, "CLOCK=")))
		{
			sscanf(ptr + 6, "%c", &disp_clock);
		}
		else if((ptr = strstr(line_buffer, "SEC=")))
		{
			sscanf(ptr + 4, "%c", &disp_sec);
		}
		else if((ptr = strstr(line_buffer, "SIZE=")))
		{
			sscanf(ptr + 5, "%c", &disp_size);
		}
		else if((ptr = strstr(line_buffer, "COLOR=")))
		{
			sscanf(ptr + 6, "%d", &disp_color);
		}
		else if((ptr = strstr(line_buffer, "BACK=")))
		{
			sscanf(ptr + 5, "%d", &disp_back);
		}
		else if((ptr = strstr(line_buffer, "DETECT=")))
		{
			sscanf(ptr + 7, "%d", &disp_detect);
		}
		else if((ptr = strstr(line_buffer, "MAIL=")))
		{
			sscanf(ptr + 5, "%c", &disp_mail);
		}
	}

	// close config-file
	fclose(fd_conf);

	// ----------------
	// check config
	// on-screen-display german or english
	if (osd != 'G' && osd != 'E')
	{
		printf("TuxCal <OSD=%c invalid, set to \"G\">\n", osd);
		osd = 'G';
	}
	switch (osd)
	{
		case 'G': osdidx=0; break;
		case 'E': osdidx=1; break;
	}

	if (!startdelay) startdelay = 30;												// default 30 seconds delay
	if (!intervall) intervall = 1;													// default check every 1 second

	// we have different skins
	if (skin != 1 && skin != 2 && skin != 3)
	{
		printf("TuxCal <SKIN=%d invalid, set to \"1\">\n", skin);
		skin = 1;
	}
}

/******************************************************************************
 * WriteConf
 ******************************************************************************/
/*!
 * write the configuration back to the file
 
 \param			: none
 \return 		: 1:OK    0:FAILED    
*/
int WriteConf()
{
	FILE *fd_conf;

	// open config-file
	if (!(fd_conf = fopen(CFGPATH CFGFILE , "w")))
	{
		return 0;
	}

	fprintf(fd_conf, "STARTDELAY=%d\n", startdelay);
	fprintf(fd_conf, "INTERVALL=%d\n", intervall);
	fprintf(fd_conf, "LOGGING=%c\n", logging);
	fprintf(fd_conf, "AUDIO=%c\n", audio);
	fprintf(fd_conf, "VIDEO=%d\n", video);
	fprintf(fd_conf, "SIGNAL=%d\n", sigtype);
	fprintf(fd_conf, "SIGMODE=%d\n", sigmode);
	fprintf(fd_conf, "SIGTIME=%s\n\n", sigtime);
	fprintf(fd_conf, "OSD=%c\n", osd);
	fprintf(fd_conf, "SKIN=%d\n\n", skin);
	fprintf(fd_conf, "WEBPORT=%d\n", webport);
	fprintf(fd_conf, "WEBUSER=%s\n", webuser);
	fprintf(fd_conf, "WEBPASS=%s\n\n", webpass);
	fprintf(fd_conf, "POS_X=%d\n", cstartx);
	fprintf(fd_conf, "POS_Y=%d\n", cstarty);
	fprintf(fd_conf, "SHOW=%c\n", show_clock);
	fprintf(fd_conf, "DATE=%c\n", disp_date);
	fprintf(fd_conf, "CLOCK=%c\n", disp_clock);
	fprintf(fd_conf, "SEC=%c\n", disp_sec);
	fprintf(fd_conf, "SIZE=%c\n", disp_size);
	fprintf(fd_conf, "COLOR=%d\n", disp_color);
	fprintf(fd_conf, "BACK=%d\n", disp_back);
	fprintf(fd_conf, "DETECT=%d\n", disp_detect);
	fprintf(fd_conf, "MAIL=%c\n\n", disp_mail);

	fclose(fd_conf);
	return 1;
}

/******************************************************************************
 * ControlDaemon 
 ******************************************************************************/
/*
 * we do have a connection to a daemon, this daemon will signal an alarm
 
 \param	command	: different command-codes, e.g. GET_VERSION
 \return 				: 1:OK   0:FAILED
*/
int ControlDaemon(int command)
{
	int fd_sock;																													// socket to daemon
	struct sockaddr_un srvaddr;
	socklen_t addrlen;

	// setup connection
	srvaddr.sun_family = AF_UNIX;
	strcpy(srvaddr.sun_path, SCKFILE);
	addrlen = sizeof(srvaddr.sun_family) + strlen(srvaddr.sun_path);

	// open socket
	if ((fd_sock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		printf("TuxCal <Socketerror: socket failed>\n");
		return 0;
	}

	// connect socket
	if (connect(fd_sock, (struct sockaddr*)&srvaddr, addrlen) == -1)
	{
		printf("TuxCal <Socketerror: connect failed>\n");
		close(fd_sock);
		return 0;
	}

	// send command
	// sending is very simple: we send a command and (perhaps) we wait for an answer
	switch(command)
	{
		case GET_STATUS:
		{
			send(fd_sock, "G", 1, 0);
			recv(fd_sock, &online, 1, 0);
		}	break;

		case SET_STATUS:
		{
			send(fd_sock, "S", 1, 0);
			send(fd_sock, &online, 1, 0);
		}	break;

		case GET_VERSION:
		{
			send(fd_sock, "V", 1, 0);
			recv(fd_sock, &versioninfo_d, sizeof(versioninfo_d), 0);
		}	break;
		
		case RELOAD_DB:
		{
			send(fd_sock, "R", 1, 0);
		} break;			
		
		case TOGGLE_CLOCK:
		{
			send(fd_sock, "C", 1, 0);
		} break;
	}

	// close connection
	close(fd_sock);
	return 1;
}

/******************************************************************************
 * GetRCCode
 ******************************************************************************/
/*!
 * this is the remote-control function, similar one is used in TuxMail
 * very different for dBox and Dreambox
 
 \param			: none
 \return 		: remote-control or keyboard-code
*/

#if HAVE_DVB_API_VERSION == 3									// dBox

int GetRCCode()
{
	static __u16 rc_last_key = KEY_RESERVED;
	static __u16 rc_last_code = KEY_RESERVED;

	int iEnterSecond = at->tm_sec;								// remember second when entering the function
		
	do
	{
		rccode = 0xFFFF;
  
  	// we read exactly one input-event
  	if (read(rc, &ev, sizeof(ev)) == sizeof(ev))
  	{
  		// do we have a valid input
  		if (ev.value)
  		{
  			// is it the same keystroke than last time
  			if (ev.code != rc_last_key)
  			{
  				// remember the key pressed, this is not the same as the last keycode
  				rc_last_key = ev.code;
  
  				switch (ev.code)
  				{
  					case KEY_OK: 			rccode = RC_OK;			break;
  					case KEY_RED:			rccode = RC_RED;		break;
  					case KEY_GREEN:		rccode = RC_GREEN;	break;
  					case KEY_YELLOW:	rccode = RC_YELLOW;	break;
  					case KEY_BLUE:		rccode = RC_BLUE;		break;
  					case KEY_HELP:		rccode = RC_HELP;		break;
  					case KEY_SETUP:		rccode = RC_DBOX;		break;
  
  					// here we handle keystrokes from a keyboard
  					default:						
  						// no keystroke with code > 127
  						// except POWER_ON
  						// log keys to file
/*  						
							{  						
								FILE* pipe;  	
								if ((pipe = fopen("/tmp/keys.log", "a"))!=NULL)
								{
									fprintf(pipe,"key: %04x\r\n",ev.code);
									fclose(pipe);
								}
							}
*/							
  						if ( ev.code > 0x7F )
  						{
  							rccode = 0;
  							if ( ev.code == 0x110 )
  							{
  								rccode = RC_STANDBY;
  							}
  						}
  						else
  						{
  							// get keystroke from table
  							rccode = rctable[ev.code & 0x7F];
  						}
  						// if last-key was the LEFT_SHIFT, we use another table
  						if ( rc_last_code == RC_LSHIFT )
  						{
  							if ( ev.code <= (sizeof(rcshifttable)/sizeof(int)-1))
  							{
  								rccode = rcshifttable[ev.code];
  							}
  						}
  						// if last-key was the ALT key, we use a different tyble
  						else if ( rc_last_code == RC_ALTGR )
  						{
  							if ( ev.code <= (sizeof(rcaltgrtable)/sizeof(int)-1))
  							{
  								rccode = rcaltgrtable[ev.code];
  							}
  						}
  						// if we didn't find a valid key, we return the error-code
  						if ( !rccode )
  						{
  							rccode = 0xFFFF;
  						}
  				}
  				// remember the last keycode
  				rc_last_code = rccode;
  			}
  			else  // same key than last time -> ignore
  			{				
  				rccode = 0xFFFF;
  			}
  		}
  		else		// no valid key -> ignore and invalidate last pressed key
  		{
  			rccode = 0xFFFF;
  			rc_last_key = KEY_RESERVED;
  			rc_last_code = KEY_RESERVED;
  		}
			usleep(1000000/100);
	
		}	

//		ftime(&tb);																// read the actual time
//		gettimeofday(&tv,NULL);
//		tb.time=tv.tv_sec;
		time(&tt);																	// read the actual time	
		at = localtime(&tt);
		if (at->tm_sec != iEnterSecond) break;			// leave if we have a different second
			
 	} while (rccode == 0xFFFF);

	return rccode;
}

#else	// Dreambox

int GetRCCode()
{
	static unsigned short LastKey = -1;
	static char LastKBCode = 0x00;

	int iEnterSecond = at->tm_sec;								// remember second when entering the function

	// rc is in non-blocking mode, so it is possible to read either
	// the rc or the kb
	// we return if we receive any key pressed
	do
	{
		rccode = -1;

		// first check if we have a key pressed on the remote-control	
		int bytesavail = 0;
		int bytesread = read(rc, &rccode, sizeof(rccode));
		
		kbcode = 0;

		// keyboard
		if( kb != -1 )
		{
			ioctl(kb, FIONREAD, &bytesavail);
		}	
		if( bytesavail>0 )
		{
//			char tch[100];
			char end=0;

			if( bytesavail > 99 ) 
			{
				bytesavail = 99;
			}
			read(kb,tch,bytesavail);
			tch[bytesavail] = 0x00;
			kbcode = tch[0];
			LastKBCode = kbcode;
			// one code keys
			if (bytesavail == 1 && kbcode == 0x1b) { rccode = RC_ESC; LastKey = rccode; return rccode;} // ESC-Taste
			if (bytesavail == 1 && kbcode == 0x7F) { rccode = RC_BS; LastKey = rccode; return rccode;} 	// BS-Taste
			if (bytesavail == 1 && kbcode == '\n') { rccode = RC_RET; LastKey = rccode; return rccode;} // Enter-Taste
			// keys with at least 3 codes
			if (bytesavail >= 3 && tch[0] == 0x1b && tch[1] == 0x5b)
			{
				if (tch[2] == 0x41 )                                    { kbcode = LastKBCode = 0x00; rccode = RC_UP        ; end=1;}// Cursortasten
				if (tch[2] == 0x42 )                                    { kbcode = LastKBCode = 0x00; rccode = RC_DOWN      ; end=1;}// Cursortasten
				if (tch[2] == 0x43 )                                    { kbcode = LastKBCode = 0x00; rccode = RC_RIGHT     ; end=1;}// Cursortasten
				if (tch[2] == 0x44 )                                    { kbcode = LastKBCode = 0x00; rccode = RC_LEFT      ; end=1;}// Cursortasten
				if (tch[2] == 0x33 && tch[3] == 0x7e)                   { kbcode = LastKBCode = 0x00; rccode = RC_ENTF      ; end=1;}// entf-Taste
				if (tch[2] == 0x32 && tch[3] == 0x7e)                   { kbcode = LastKBCode = 0x00; rccode = RC_INS       ; end=1;}// einf-Taste
				if (tch[2] == 0x35 && tch[3] == 0x7e)                   { kbcode = LastKBCode = 0x00; rccode = RC_PLUS      ; end=1;}// PgUp-Taste
				if (tch[2] == 0x36 && tch[3] == 0x7e)                   { kbcode = LastKBCode = 0x00; rccode = RC_MINUS     ; end=1;}// PgDn-Taste
				if (tch[2] == 0x5b && tch[3] == 0x41)                   { kbcode = LastKBCode = 0x00; rccode = RC_F10       ; end=1;}// F1-Taste
				if (tch[2] == 0x5b && tch[3] == 0x42)                   { kbcode = LastKBCode = 0x00; rccode = RC_F5        ; end=1;}// F2-Taste
				if (tch[2] == 0x5b && tch[3] == 0x43)                   { kbcode = LastKBCode = 0x00; rccode = RC_F6        ; end=1;}// F3-Taste
				if (tch[2] == 0x5b && tch[3] == 0x44)                   { kbcode = LastKBCode = 0x00; rccode = RC_F7        ; end=1;}// F4-Taste
				if (tch[2] == 0x5b && tch[3] == 0x45)                   { kbcode = LastKBCode = 0x00; rccode = RC_F8        ; end=1;}// F5-Taste
				if (tch[2] == 0x31 && tch[3] == 0x37 && tch[4] == 0x7e) { kbcode = LastKBCode = 0x00; rccode = RC_F6        ; end=1;}// WEB-Taste
				if (tch[2] == 0x31 && tch[3] == 0x38 && tch[4] == 0x7e) { kbcode = LastKBCode = 0x00; rccode = RC_F5        ; end=1;}// Mail-Taste
				if (tch[2] == 0x31 && tch[3] == 0x39 && tch[4] == 0x7e) { kbcode = LastKBCode = 0x00; rccode = RC_F5        ; end=1;}// F8-Taste
				if (tch[2] == 0x32 && tch[3] == 0x30 && tch[4] == 0x7e) { kbcode = LastKBCode = 0x00; rccode = RC_F5        ; end=1;}// F9-Taste
				if (tch[2] == 0x32 && tch[3] == 0x31 && tch[4] == 0x7e) { kbcode = LastKBCode = 0x00; rccode = RC_RET1      ; end=1;}// M2-Taste
				if (tch[2] == 0x32 && tch[3] == 0x33 && tch[4] == 0x7e) { kbcode = LastKBCode = 0x00; rccode = RC_F9        ; end=1;}// M1-Taste
			}
			// if we didn't read a key from remote-control
			if( bytesread == 0 )
			{
				if (kbcode == '0') { kbcode = 0x00;rccode = RC_0  ; end=1;}
				if (kbcode == '1') { kbcode = 0x00;rccode = RC_1  ; end=1;}
				if (kbcode == '2') { kbcode = 0x00;rccode = RC_2  ; end=1;}
				if (kbcode == '3') { kbcode = 0x00;rccode = RC_3  ; end=1;}
				if (kbcode == '4') { kbcode = 0x00;rccode = RC_4  ; end=1;}
				if (kbcode == '5') { kbcode = 0x00;rccode = RC_5  ; end=1;}
				if (kbcode == '6') { kbcode = 0x00;rccode = RC_6  ; end=1;}
				if (kbcode == '7') { kbcode = 0x00;rccode = RC_7  ; end=1;}
				if (kbcode == '8') { kbcode = 0x00;rccode = RC_8  ; end=1;}
				if (kbcode == '9') { kbcode = 0x00;rccode = RC_9  ; end=1;}
			}
			if( end ) 
			{ 
				LastKey = rccode; return rccode; 
			}
			if( bytesavail == 1 )
			{
				rccode = kbcode ; return rccode; 
			}
		}
		// if a key on the remote-control has been pressed
		if( bytesread == 2 )
		{
			if( rccode == LastKey && LastKBCode != 0x00 && LastKBCode == kbcode )
			{
				return rccode;
			}
			LastKBCode = 0x00;
			if( rccode == LastKey )
			{
				rccode = -1;
				return rccode;
			}

			LastKey = rccode;
			if((rccode & 0xFF00) == 0x5C00)
			{
				kbcode = 0;
				switch(rccode)
				{
					case KEY_UP:		rccode = RC_UP;			break;
					case KEY_DOWN:		rccode = RC_DOWN;		break;
					case KEY_LEFT:		rccode = RC_LEFT;		break;
					case KEY_RIGHT:		rccode = RC_RIGHT;		break;
					case KEY_OK:		rccode = RC_OK;			break;
					case KEY_0:			rccode = RC_0;			break;
					case KEY_1:			rccode = RC_1;			break;
					case KEY_2:			rccode = RC_2;			break;
					case KEY_3:			rccode = RC_3;			break;
					case KEY_4:			rccode = RC_4;			break;
					case KEY_5:			rccode = RC_5;			break;
					case KEY_6:			rccode = RC_6;			break;
					case KEY_7:			rccode = RC_7;			break;
					case KEY_8:			rccode = RC_8;			break;
					case KEY_9:			rccode = RC_9;			break;
					case KEY_RED:		rccode = RC_RED;		break;
					case KEY_GREEN:		rccode = RC_GREEN;		break;
					case KEY_YELLOW:	rccode = RC_YELLOW;		break;
					case KEY_BLUE:		rccode = RC_BLUE;		break;
					case KEY_VOLUMEUP:	rccode = RC_PLUS;		break;
					case KEY_VOLUMEDOWN:rccode = RC_MINUS;		break;
					case KEY_MUTE:		rccode = RC_MUTE;		break;
					case KEY_HELP:		rccode = RC_HELP;		break;
					case KEY_SETUP:		rccode = RC_DBOX;		break;
					case KEY_HOME:		rccode = RC_HOME;		break;
					case KEY_POWER:		rccode = RC_STANDBY;	break;
					default: 			rccode = -1;
				}
				return rccode;
			}
			else
			{
				if( rccode != 0xFFFF)
				{
					rccode &= 0x003F;
				}
			}
			return rccode;
		}

		usleep(1000000/100);
		
//		ftime(&tb);																// read the actual time
//		gettimeofday(&tv,NULL);
//		tb.time=tv.tv_sec;
//		at = localtime(&tb.time);
		time(&tt);																	// read the actual time	
		at = localtime(&tt);
		if (at->tm_sec != iEnterSecond) break;			// leave if we have a different second
	}
	while( rccode == 0xFFFF);
	return rccode;
}
#endif

/******************************************************************************
 * MyFaceRequester
 ******************************************************************************/
/*!
 * load font
 
 \param	face_id				: FTC_FaceID
 \param library				: FT_Library
 \param request_data	: FT_Pointer
 \param afacs					: FT_Face*
 \return 							: FT_Error
*/
FT_Error MyFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	FT_Error result;

	result = FT_New_Face(library, face_id, 0, aface);

	if (!result) printf("TuxCal <Font \"%s\" loaded>\n", (char*)face_id);
	else printf("TuxCal <Font \"%s\" failed>\n", (char*)face_id);
	
	return result;
}


/******************************************************************************
 * RenderChar
 ******************************************************************************/
/*!
 * render a character to the screen
 
 \param currentchar	: FT_ULong
 \param sx					: startposition x
 \param sy					: startposition y
 \param ex					: endposition x
 \param color				: color
 \return 						: charwidth
*/
int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, int color)
{
	int row, pitch, bit, x = 0, y = 0;
	FT_Error error;
	FT_UInt glyphindex;
	FT_Vector kerning;
	FTC_Node anode;

	//load char
	if (!(glyphindex = FT_Get_Char_Index(face, currentchar)))
	{
		printf("TuxCal <FT_Get_Char_Index for Char \"0x%x\" failed: \"undefined character code\">\n", (int)currentchar);
		return 0;
	}

	if ((error = FTC_SBitCache_Lookup(cache, &desc, glyphindex, &sbit, &anode)))
	{
		printf("TuxCal <FTC_SBitCache_Lookup for Char \"%c\" failed with Errorcode 0x%.2X>\n", (int)currentchar, error);
		return 0;
	}

	if (use_kerning)
	{
		FT_Get_Kerning(face, prev_glyphindex, glyphindex, ft_kerning_default, &kerning);
		prev_glyphindex = glyphindex;
		kerning.x >>= 6;
	}
	else
	{
		kerning.x = 0;
	}

	// render char
	if (color != -1) 																			// don't render char, return charwidth only 
	{
		if (sx + sbit->xadvance >= ex) return -1; 					// limit to maxwidth 

		for (row = 0; row < sbit->height; row++)
		{
			for (pitch = 0; pitch < sbit->pitch; pitch++)
			{
				for (bit = 7; bit >= 0; bit--)
				{
					if (pitch*8 + 7-bit >= sbit->width)
					{
						break; 																			// render needed bits only 
					}

					if ((sbit->buffer[row * sbit->pitch + pitch]) & 1<<bit)
					{
						*(lbb + startx + sx + sbit->left + kerning.x + x + var_screeninfo.xres*(starty + sy - sbit->top + y)) = color;
					}

					x++;
				}
			}
			x = 0;
			y++;
		}
	}

	// return charwidth
	return sbit->xadvance + kerning.x;
}

/******************************************************************************
 * GetStringLen
 ******************************************************************************/
/*!
 * calculate used pixels on screen for output
 
*/
int GetStringLen(unsigned char *string)
{
	int stringlen = 0;

	// reset kerning
	prev_glyphindex = 0;

	// calc len
	while (*string != '\0')
	{
		stringlen += RenderChar(*string, -1, -1, -1, -1);
		string++;
	}

	return stringlen;
}

/******************************************************************************
 * RenderString
 ******************************************************************************/
/*!
 * render a string to the screen
 
*/
void RenderString(unsigned char *string, int sx, int sy, int maxwidth, int layout, int size, int color)
{
	int stringlen, ex, charwidth;

	// set size
	if(size == SMALL)
	{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
		desc.width = desc.height = FONTSIZE_SMALL;
#else
		desc.font.pix_width = desc.font.pix_height = FONTSIZE_SMALL;
#endif
	}
	else if (size == NORMAL)
	{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
		desc.width = desc.height = FONTSIZE_NORMAL;
#else
		desc.font.pix_width = desc.font.pix_height = FONTSIZE_NORMAL;
#endif
	}
	else
	{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
		desc.width = desc.height = FONTSIZE_BIG;
#else
		desc.font.pix_width = desc.font.pix_height = FONTSIZE_BIG;
#endif
	}

	// set alignment
	if ((layout != LEFT) && (layout != FIXEDLEFT))
	{
		stringlen = GetStringLen(string);

		switch(layout)
		{
			case FIXEDCENTER:
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
				stringlen = (desc.width/2) * strlen(string);
#else
				stringlen = (desc.font.pix_width/2) * strlen(string);
#endif
			case CENTER:
			{
				if(stringlen < maxwidth)
				{
					sx += (maxwidth - stringlen)/2;
				} 
			} break;

			case FIXEDRIGHT:
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
				stringlen = (desc.width/2) * strlen(string);
#else
				stringlen = (desc.font.pix_width/2) * strlen(string);
#endif
			case RIGHT:
			{
				if(stringlen < maxwidth)
				{
					sx += maxwidth - stringlen;
				}
			} break;		
		}
	}

	// reset kerning
	prev_glyphindex = 0;

	// render string
	ex = sx + maxwidth;

	while (*string != '\0')
	{
		if ((charwidth = RenderChar(*string, sx, sy, ex, color)) == -1)  return; // string > maxwidth 

		if ((layout == FIXEDLEFT) || (layout == FIXEDCENTER) || (layout == FIXEDRIGHT))
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
			sx += (desc.width/2);
#else
			sx += (desc.font.pix_width/2);
#endif
		else 
			sx += charwidth;
		string++;
	}
}

/******************************************************************************
 * RenderBox
 ******************************************************************************/
/*!
 * render a box to the screen

 \param sx		: startposition x
 \param sy		: startposition y
 \param ex		: endposition x
 \param ey		: endposition y
 \param mode	: mode for painting (FILL or GRID)
 \param color	: color to paint with
 \return      : none
*/
void RenderBox(int sx, int sy, int ex, int ey, int mode, int color)
{
	int loop;

	// paint a box with solid color
	if (mode == FILL)
	{
		// for all lines of the box
		for (; sy <= ey; sy++)
		{
			// fill the line with the color
			memset(lbb + startx + sx + var_screeninfo.xres*(starty + sy), color, ex-sx + 1);
		}
	}
	// paint a grid around the rectangle
	else
	{
		// lines
		for (loop = sx; loop <= ex; loop++)
		{
			*(lbb + startx+loop + var_screeninfo.xres*(sy+starty)) = color;
			*(lbb + startx+loop + var_screeninfo.xres*(sy+1+starty)) = color;
			*(lbb + startx+loop + var_screeninfo.xres*(ey-1+starty)) = color;
			*(lbb + startx+loop + var_screeninfo.xres*(ey+starty)) = color;
		}

		// columns

		for (loop = sy; loop <= ey; loop++)
		{
			*(lbb + startx+sx + var_screeninfo.xres*(loop+starty)) = color;
			*(lbb + startx+sx+1 + var_screeninfo.xres*(loop+starty)) = color;
			*(lbb + startx+ex-1 + var_screeninfo.xres*(loop+starty)) = color;
			*(lbb + startx+ex + var_screeninfo.xres*(loop+starty)) = color;
		}
	}
}

/******************************************************************************
 * RenderInt
 ******************************************************************************/
/*!
 * render a integer to the screen
 
*/
void RenderInt(unsigned char *string, int sx, int sy, int maxwidth, int layout, int size, int color, int colorgrid, int colorfill)
{
	int x,y,cx,cy;
	int sizey=FONTSIZE_NORMAL;
	
	if (size==SMALL) sizey=FONTSIZE_SMALL;
	else if (size==BIG) sizey=FONTSIZE_BIG;
	
	x=sx-5;
	y=sy-sizey+8;
	cx=x+maxwidth+3;
	cy=y+sizey;	
	if (colorfill!=-1) RenderBox(x,y,cx,cy,FILL,colorfill);
	if (colorgrid!=-1) RenderBox(x,y,cx,cy,GRID,colorgrid);

	RenderString(string,sx,sy,maxwidth,layout,size,color);
}

/******************************************************************************
 * RenderSObject
 ******************************************************************************/
/*!
 * render an object to the screen paint only the pixels which are set
 
 \param sx		: startposition x
 \param sy		: startposition y
 \param color	: color to paint with
 \param iType	: index for the object to paint
 \return      : none
*/
void RenderSObject(int sx, int sy, int color, int iType)
{
	int x, y;
  char* pObj=circle;

	// choose the object
	if (iType == OBJ_CIRCLE) pObj=circle;
	if (iType == OBJ_HEART) pObj=heart;
	if (iType == OBJ_MARKER) pObj=marker;
	if (iType == OBJ_SCROLLUP) pObj=scroll_up;
	if (iType == OBJ_SCROLLDN) pObj=scroll_dn;
	if (iType == OBJ_CLOCK) pObj=symbolclock;

	// render
	for (y = 0; y < OBJ_SY; y++)					// for all columns
	{	
		for (x = 0; x < OBJ_SX; x++)				// for all lines
		{
			if (*pObj++)											// only paint if mask-value set
				memset(lbb + startx + sx + x + var_screeninfo.xres*(starty + sy + y), color, 1);
		}
	}
}

/******************************************************************************
 * MessageBox
 ******************************************************************************/
/*!
 * message-box which asks a question
 * use can choose OK or NO/Exit
 
 \param	header		:	text which is displayed in the header of the messagebox
 \param	question	:	the text (question)
 \return  				: 1:OK/Yes  0:Exit/No
*/
int MessageBox(char* header, char* question)
{
	// paint the box
	RenderBox(MSGBOX_SX, MSGBOX_SY, MSGBOX_EX, HEADERSTART, FILL, SKIN0);
	RenderBox(MSGBOX_SX, HEADERSTART, MSGBOX_EX, MSGBOX_EY, FILL, SKIN1);
	RenderBox(MSGBOX_SX, MSGBOX_SY, MSGBOX_EX, MSGBOX_EY, GRID, SKIN2);
	RenderBox(MSGBOX_SX, HEADERSTART, MSGBOX_EX, MSGBOX_EY, GRID, SKIN2);
  
	// render the strings
	RenderString(header, MSGBOX_SX+2, HEADERTEXTSTART, TEXTWIDTH, CENTER, BIG, ORANGE);
	RenderString(question, MSGBOX_SX+2, TEXTSTART, TEXTWIDTH, CENTER, BIG, WHITE);
   
  // paint the buttons
  RenderBox(BUTTONSX, BUTTONSY, BUTTONSX+BUTTONX, BUTTONSY+FONTSIZE_SMALL, FILL, SKIN2);
  RenderString("OK", BUTTONSX+2, BUTTONSY+FONTSIZE_SMALL-5, BUTTONX-4, CENTER, SMALL, WHITE);
	RenderBox(BUTTONSX+2*BUTTONX, BUTTONSY, BUTTONSX+3*BUTTONX, BUTTONSY+FONTSIZE_SMALL, FILL, SKIN2);
	RenderString("EXIT", BUTTONSX+2*BUTTONX+2, BUTTONSY+FONTSIZE_SMALL-5, BUTTONX-4, CENTER, SMALL, WHITE);
  
	// output to framebuffer
	memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);

	// now wait for key
	while ( GetRCCode() )
	{
		// OK or RETURN
		if ( rccode == RC_OK )
		{
			rccode = -1;											// forget the pressed key
			return 1;													// return YES
		}
		// HOME or EXIT == NO
		if ( rccode == RC_HOME )
		{
			rccode = -1;											// forget the pressed key
			return 0;													// return NO
		}
	}
	return 0;
}

/******************************************************************************
 * ShowMessage
 ******************************************************************************/
/*!
 * output a message in a window on the screen
 
*/
void ShowMessage(int message)
{
	char info[80];

	// layout
	RenderBox(MSGBOX_SX, MSGBOX_SY, MSGBOX_EX, HEADERSTART, FILL, SKIN0);
	RenderBox(MSGBOX_SX, HEADERSTART, MSGBOX_EX, MSGBOX_EY, FILL, SKIN1);
	RenderBox(MSGBOX_SX, MSGBOX_SY, MSGBOX_EX, MSGBOX_EY, GRID, SKIN2);
	RenderBox(MSGBOX_SX, HEADERSTART, MSGBOX_EX, MSGBOX_EY, GRID, SKIN2);

	// message
	if (message != INFO)
	{
		RenderString("TuxCal Statusinfo", MSGBOX_SX+2, HEADERTEXTSTART, TEXTWIDTH, CENTER, BIG, ORANGE);
		strcpy(info,infomsg[message][osdidx]);
	}
	else
	{
		ControlDaemon(GET_VERSION);
		sprintf(info, "TuxCal (P%s/D%s)", versioninfo_p, versioninfo_d);
		RenderString(info, MSGBOX_SX+2, HEADERTEXTSTART, TEXTWIDTH, CENTER, BIG, ORANGE);
		sprintf(info,"(c) 2006 Robert \"robspr1\" Spreitzer");
	}

	RenderString(info, MSGBOX_SX+2, TEXTSTART, TEXTWIDTH, CENTER, NORMAL, WHITE);

	memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);

	// wait for OK keystroke
	while ( GetRCCode() )
	{
		if (( rccode == RC_OK ) || ( rccode == RC_HOME ))
		{
			break;
		}
		if ((message == INFO) && (rccode == RC_HELP))
		{
			//TODO: open and show help-file
			break;
		}
	}
	rccode = -1;
}


/******************************************************************************
 * PaintEdit
 ******************************************************************************/
/*!
 * paint the edit-window
 
 \param pEvt			: EVT_DB* pointer to event-object
 \param iEditLine	: line just being edited
 \param iEditCol	: column in line just being edited
 \return					: none
*/
int* PaintEdit(EVT_DB* pEvt, int iEditLine, int iEditCol)
{
	char info[80];	
	int x, y, l, t, r, b;
	int iColor=RED;
	int* pIEdit=NULL;
	
	// background (just for testing)
	RenderBox(0, 0, MAXSCREEN_X, MAXSCREEN_Y, FILL, GREY);
  // sprintf(info,"%d %d %d %d %d %d",last,start,end,akt,sel,infolines);
  // RenderString(info,0,GRIDLINE-2,MAXSCREEN_X-4, LEFT, SMALL, GREY);
  // return;
	
	// header (paint date)
	RenderBox(0, 0, MAXSCREEN_X,GRIDLINE, FILL, SKIN3);
	RenderBox(0, 0, MAXSCREEN_X,GRIDLINE, GRID, SKIN2);
	RenderBox(MAXSCREEN_X/2, 0, MAXSCREEN_X/2,GRIDLINE, GRID, SKIN2);
	if (osdidx == 0) sprintf(info,"%u. %s %u",tShow_day,monthmsg[tShow_mon-1][osdidx],tShow_year);
	else sprintf(info,"%s %u, %u",monthmsg[tShow_mon-1][osdidx],tShow_day,tShow_year);
	RenderString(info,0+4,GRIDLINE-4,MAXSCREEN_X-4, LEFT, NORMAL, BLACK);

	int iY=2*GRIDLINE;
	int iX=EDITX;
	
	RenderString(infotype[pEvt->type-1][osdidx],20,iY,MAXSCREEN_X-4,LEFT,NORMAL,RED);
	iY+=GRIDLINE;
	
	int i;
	int *pint=NULL;
	int colorfill=WHITE;
	int iStrLen=0;
	
	for (i=1;i<11;i++)
	{
		switch (i)
		{
			case 1:
				iX=EDITX;
				RenderString(szEditBoxInfo[5][osdidx], iX ,iY,MAXSCREEN_X-2*iX, LEFT, NORMAL, BLUE);
				RenderString(szEditBoxInfo[4][osdidx], iX ,iY+GRIDLINE,MAXSCREEN_X-2*iX, FIXEDLEFT, NORMAL, BLUE);
				iX+=5;
				iY+=2*GRIDLINE;
				iStrLen=0;
				pint=&pEvt->day;
				break;
			case 2:
				iX+=FONTSIZE_NORMAL*3;
				pint=&pEvt->month;
				break;
			case 3:
				iX+=FONTSIZE_NORMAL*3;
				if (pEvt->year)
				{
					pint=&pEvt->year;
					iStrLen=1;
				}
				else pint=NULL;
				break;	
			case 4:
				iX+=FONTSIZE_NORMAL*4;
				if (pEvt->hour!=-1)
				{
					pint=&pEvt->hour;
					iStrLen=0;
				}
				else pint=NULL;
				break;
			case 5:
				iX+=FONTSIZE_NORMAL*3;
				if (pEvt->hour!=-1)	pint=&pEvt->min;
				else pint=NULL;
				break;
								
			case 6:				
				iX=EDITX;
				if (pEvt->type == PERIOD)
				{
					RenderString(szEditBoxInfo[6][osdidx], iX ,iY+GRIDLINE,MAXSCREEN_X-2*iX, LEFT, NORMAL, BLUE);
					RenderString(szEditBoxInfo[4][osdidx], iX ,iY+2*GRIDLINE,MAXSCREEN_X-2*iX, FIXEDLEFT, NORMAL, BLUE);
				}
				iX+=5;
				iY+=3*GRIDLINE;
				iStrLen=0;
				if (pEvt->type != PERIOD)
				{
					i=10;
					pint=NULL;
					break;
				}				
				iStrLen=0;
				pint=&pEvt->eday;
				break;
			case 7:
				iX+=FONTSIZE_NORMAL*3;
				pint=&pEvt->emonth;
				break;
			case 8:
				iX+=FONTSIZE_NORMAL*3;
				if (pEvt->eyear)
				{
					pint=&pEvt->eyear;
					iStrLen=1;
				}
				else pint=NULL;
				break;	
			case 9:
				iX+=FONTSIZE_NORMAL*4;
				if (pEvt->ehour!=-1)
				{
					pint=&pEvt->ehour;
					iStrLen=0;
				}
				else pint=NULL;
				break;
			case 10:
				iX+=FONTSIZE_NORMAL*3;
				if (pEvt->ehour!=-1)	pint=&pEvt->emin;
				else pint=NULL;
				break;
		}

		if (pint)
		{
			sprintf(info,szFmtStr[iStrLen],*pint);

			if (iEditLine==i) colorfill=YELLOW;
			else colorfill=WHITE;
			RenderInt(info, iX ,iY,(iStrLen+1)*FONTSIZE_NORMAL, FIXEDLEFT, NORMAL, BLACK, SKIN2, colorfill);
			if (iEditLine==i)
			{
				info[iEditCol+1]=0;
				RenderInt(&info[iEditCol], iX+(iEditCol*FONTSIZE_NORMAL/2), iY, FONTSIZE_NORMAL/2, FIXEDLEFT, NORMAL, WHITE, TRANSP, BLUE);
				pIEdit=pint;
			}
		}
	}
		
	iY+=2*GRIDLINE;
	if (iEditLine==11) colorfill=YELLOW;
	else colorfill=WHITE;
	RenderInt(pEvt->info,EDITX+5,iY,FONTSIZE_NORMAL*16, FIXEDLEFT, NORMAL, BLACK, SKIN2,colorfill);
	if (strlen(pEvt->info)>MAXINFOEDITLEN/2)
		RenderInt(&pEvt->info[MAXINFOEDITLEN/2],EDITX+5,iY+GRIDLINE,FONTSIZE_NORMAL*16, FIXEDLEFT, NORMAL, BLACK,SKIN2,colorfill);
	if (iEditLine==11)
	{
		info[0]=pEvt->info[iEditCol];
		info[1]=0;	
		RenderInt(info, EDITX+5+((iEditCol-((iEditCol>=MAXINFOEDITLEN/2)?MAXINFOEDITLEN/2:0))*FONTSIZE_NORMAL/2), iY+((iEditCol>=MAXINFOEDITLEN/2)?GRIDLINE:0), FONTSIZE_NORMAL/2, FIXEDLEFT, NORMAL, WHITE, TRANSP, BLUE);
	}
	
	// footer (paint buttons, function-keys)
	RenderBox(0, EDITFOOTER_Y, MAXSCREEN_X,MAXSCREEN_Y, FILL, SKIN3);
	RenderBox(0, EDITFOOTER_Y, MAXSCREEN_X,MAXSCREEN_Y, GRID, SKIN2);
	for ( y = 0; y < 4; y++ )
	{
		t=EDITFOOTER_Y + KEYBOX_SPACE + y*(KEYBOX_HEIGHT+KEYBOX_SPACE);
		b=EDITFOOTER_Y +  KEYBOX_SPACE + y*(KEYBOX_HEIGHT+KEYBOX_SPACE)+KEYBOX_HEIGHT;

		l=20 + 3*(KEYBOX_WIDTH+KEYBOX_SPACE);
		r=20 + 3*(KEYBOX_WIDTH+KEYBOX_SPACE)+KEYBOX_WIDTH;
		
		switch (y)
		{
			case 0: iColor=RED; break;
			case 1: iColor=GREEN; break;
			case 2: iColor=YELLOW; break;
			case 3: iColor=BLUE; break;
		}

		RenderSObject(l,t+6,iColor,OBJ_CIRCLE);
		RenderString(szEditBoxInfo[y][osdidx],20 + l, b - 4,MAXSCREEN_X/2, LEFT, SMALL, ORANGE);
		
		// paint function-buttons
		for ( x = 0; x < 3; x++)
		{
			l=10 + x*(KEYBOX_WIDTH+KEYBOX_SPACE);
			r=10 + x*(KEYBOX_WIDTH+KEYBOX_SPACE)+KEYBOX_WIDTH;
			
			RenderBox(l, t, r, b, FILL, SKIN1);
			RenderBox(l, t, r, b, GRID, SKIN2);

			if( nEditStyle == 3 )
			{
				if (szKeyBBoxKey[x+y*3][0])
				{
					RenderString(szKeyBBoxKey[x+y*3],5 + l, b - 4 ,25,LEFT, SMALL, WHITE);
					RenderString(szKeyBBoxInfo[x+y*3][osdidx],20 + l, b - 4,60, RIGHT, SMALL, ORANGE);
				}
			}
			else
			{
				RenderString(szKeyBoxKey[x+y*3],5 + l, b - 4, 25,LEFT, SMALL, WHITE);
				RenderString(szKeyBoxInfo[x+y*3],20 + l, b - 4, 60, RIGHT, SMALL, ORANGE);
			}
		}
	}

	// output to framebuffer
	memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);
	return pIEdit;
}

/******************************************************************************
 * CheckEvent
 ******************************************************************************/
/*!
 * check the entry
 
 \param pEvt	: EVT_DB* pointer to event-object
 \return  		: 1:OK/Yes  0:Exit/No - changes made
*/
int CheckEvent(EVT_DB* pEvt)
{
	EVT_DB evtsic;
	memcpy(&evtsic,pEvt,sizeof(EVT_DB)-MAXINFOLEN);
	
	int iLeapYear=0;
	if (pEvt->month<1) pEvt->month=1;
	if (pEvt->month>12) pEvt->month=12;
	if (pEvt->year!=0)
	{
		if (pEvt->year<1700) pEvt->year=1700;
		if (pEvt->year>2299) pEvt->year=2299;
		iLeapYear=LeapYear(pEvt->year);
	}
	if (pEvt->day<1) pEvt->day=1;
	if (pEvt->day>monthdays[iLeapYear][pEvt->month-1]) pEvt->day=monthdays[iLeapYear][pEvt->month-1];
	if (pEvt->hour>=0)
	{
		if (pEvt->hour>23) pEvt->hour=23;
		if (pEvt->min<0) pEvt->min=0;
		if (pEvt->min>59) pEvt->min=59;
	}
	
	if (pEvt->type==PERIOD)
	{
		iLeapYear=0;
  	if (pEvt->emonth<1) pEvt->emonth=1;
  	if (pEvt->emonth>12) pEvt->emonth=12;
  	if (pEvt->eyear!=0)
  	{
  		if (pEvt->eyear<1700) pEvt->eyear=1700;
  		if (pEvt->eyear>2299) pEvt->eyear=2299;
  		iLeapYear=LeapYear(pEvt->eyear);
  	}
  	if (pEvt->eday<1) pEvt->eday=1;
  	if (pEvt->eday>monthdays[iLeapYear][pEvt->emonth-1]) pEvt->eday=monthdays[iLeapYear][pEvt->emonth-1];
  	if (pEvt->ehour>=0)
  	{
  		if (pEvt->ehour>23) pEvt->ehour=23;
  		if (pEvt->emin<0) pEvt->emin=0;
  		if (pEvt->emin>59) pEvt->emin=59;
  	}
	}
	
	if (memcmp(&evtsic,pEvt,sizeof(EVT_DB)-MAXINFOLEN)) return 0;
	return 1;
}

/******************************************************************************
 * Edit
 ******************************************************************************/
/*!
 * edit an entry
 
 \param pEvt	: EVT_DB* pointer to event-object
 \return  		: 1:OK/Yes  0:Exit/No
*/
int Edit(EVT_DB* pEvt)
{
	int iEditLine=1;
	int iEditCol=0;
	int tmprc=0xFFFF;
	int iTypeInt=2;
	int iMultipl=1;
	int iFmtIdx=0;
	int* pEditInt=NULL;
	int iGotoRight=0;
	int iKeyboardMode=0;
	char info[80];
	int iShortTimer=0;
	
	// render the output for the current event, return the pointer to the selected
	// integer-value, NULL if none selected
	pEditInt=PaintEdit(pEvt, iEditLine, iEditCol);	
	
	// act on key-input
	while (GetRCCode())
	{
		tmprc=rccode;	
		rccode = 0xFFFF;	
		if (iShortTimer>1) iShortTimer--;											// timer for going one pos to the right
		if ((tmprc == 0xFFFF) && (iShortTimer!=1)) continue;	// only do something if a key was pressed
		
		// first calculate how many characters we have in the selected field
		// also calculate the multiplier needed for changing an integer value
		if ((iEditLine==3) || (iEditLine==8)) 								// field 3 and 8 are for the years
		{
			iTypeInt=4;																					// 4 char value
			iFmtIdx=1;
			if (iEditCol==0) iMultipl=1000;
			if (iEditCol==1) iMultipl=100;
			if (iEditCol==2) iMultipl=10;
			if (iEditCol==3) iMultipl=1;
		}
		else if (iEditLine==11) iTypeInt=strlen(pEvt->info);	// filed 11 represents the info-string
		else 																									// all others are 2-character integer values
		{
			iTypeInt=2;																					// 2 char value
			iFmtIdx=0;
			if (iEditCol==0) iMultipl=10;
			if (iEditCol==1) iMultipl=1;
		}
		
		iGotoRight=0;																					// the procedure go stepping one position to the right
																													// is outside the next switch, so we use a flag to trigger it
		
		// no act on the pressed key
		switch (tmprc)
		{
			case RC_CAPSLOCK:																							// toogle the mode for using the keyboard
			case RC_ON:
				if (iKeyboardMode) iKeyboardMode=0;
				else iKeyboardMode=1;
			break;

			// cursor-up, moves the selected line up
			case RC_UP:
				if ((iEditLine==11) && (pEvt->type==PERIOD)) iEditLine=6;		// if we have an end-date, select this line
				else iEditLine=1;																						// else select first field
				iEditCol=0;
			break;

			// cursor-down, moves the selected line up
			case RC_DOWN:
				if ((iEditLine==1) && (pEvt->type==PERIOD)) iEditLine=6;		// if we have an end-date, select this line
				else iEditLine=11;																					// else select last field
				iEditCol=0;
			break;

			// cursor right, selects the next character or the next field
			case RC_RIGHT:
				iGotoRight=1;
			break;

			// cursor left, selects previous character or the previous field
			case RC_LEFT:		
				if (--iEditCol<0) 																					// we steped outside the field
				{
					iEditLine--;																							// go one-field back
					if (iEditLine==10) iEditLine=11;													// but stay in the info-field
					if ((iEditLine==8) && (pEvt->eyear==0)) iEditLine=7;			// if we have no end-year
					if (iEditLine==5) iEditLine=6;														// don't leave the end-date line
					if ((iEditLine==3) && (pEvt->year==0)) iEditLine=2;				// if we have no start-year
					if (iEditLine<1) iEditLine=1;															// don't leave outside or index
					iEditCol=0;																								// start at first column in the new field
				}
			break;

			// plus, increment the currently selected field
			case RC_PLUS:
				if (pEditInt!=NULL)																					// do we have an integer selected
				{
					*pEditInt+=iMultipl;
					if ((iTypeInt==4) && (*pEditInt>=10000)) *pEditInt-=10000;// check for maximum
					if ((iTypeInt==2) && (*pEditInt>=100)) *pEditInt-=100;		// check for maximum
				} 
				else
				{
					if (++pEvt->info[iEditCol]>126) pEvt->info[iEditCol]=32;	// cycle through 32-126
				}
			break;

			// minus, decrement the currently selected field
			case RC_MINUS:
				if (pEditInt!=NULL)																					// do we have an integer selected
				{
					*pEditInt-=iMultipl;
					if (*pEditInt<0) *pEditInt+=(iMultipl*10);								// check for minimum
				}
				else
				{
					if (--pEvt->info[iEditCol]<32) pEvt->info[iEditCol]=126;	// cycle through 32-126
				}
			break;			

			// toogle through different types, select and deselect the needed fields
			// cycle: BRITHDAY -> EVENT -> PERIOD -> HOLIDAY -> BIRTHDAY
			case RC_RED:
			{
				iEditCol=0;																									// in any case select the first char
				if (pEvt->type==BIRTHDAY) pEvt->type=EVENT;									// change to EVENT, we have all fieds we need
				else if (pEvt->type==EVENT) 																// change to period, we have to enable the end-date/time
				{
					pEvt->type=PERIOD;
					pEvt->eyear=pEvt->year;
					pEvt->emonth=pEvt->month;
					pEvt->eday=pEvt->day;
					pEvt->ehour=pEvt->hour;
					pEvt->emin=pEvt->min;
				}
				else if (pEvt->type==PERIOD)																// change to HOLIDAY, we have no end-date/time and no starttime
				{
					pEvt->type=HOLIDAY;
					if (pEvt->year==0) pEvt->year=tShow_year;
					pEvt->eyear=0;
					pEvt->hour=-1;
					pEvt->ehour=-1;
				}
				else if (pEvt->type==HOLIDAY)																// change to BIRTHDAY, birthdays should have a year
				{
					pEvt->type=BIRTHDAY;
					if (pEvt->year==0) pEvt->year=tShow_year;
				}
			} break;
			
			// toogle using the year for the events
			// thus we switch on or off the using of the year(s)
			case RC_GREEN:
			{
				iEditCol=0;																									// in any case select the first char
				if (pEvt->year) 																						// if we used the year, unuse it
				{
					pEvt->year=0;
					pEvt->eyear=0;
				}
				else 																												// if we didn't use the year
				{
					pEvt->year=tShow_year;																		// insert current selected year
					if (pEvt->type==PERIOD) pEvt->eyear=pEvt->year;						// if type is PERIOD fill in end-year
				}
			} break;
			
			// toogle using the time for the events
			// this we switch on or off the times
			case RC_YELLOW:
			{
				if ((pEvt->type==BIRTHDAY) || (pEvt->type==HOLIDAY)) break;
				iEditCol=0;																									// in any case select the first char
				if (pEvt->hour!=-1)																					// if we used the time, unselect the times
				{
					pEvt->hour=-1;
					pEvt->ehour=-1;
				}
				else																												// if we didn't use the times
				{
					pEvt->hour=at->tm_hour;																		// fill in actual time
					pEvt->min=at->tm_min;
					if (pEvt->type==PERIOD)																		// if we have an end-date, fill in the end-time
					{
						pEvt->ehour=at->tm_hour;
						pEvt->emin=at->tm_min;
					}
				}
			} break;

			// toogle using an end-date
			// thus switch between EVENT and PERIOD
			case RC_BLUE:
			{
				iEditCol=0;																									// in any case select the first char
				if (pEvt->type==PERIOD)																			// if we do use an end-date, switch this off
				{
					pEvt->eyear=0;																						
					pEvt->ehour=-1;
					pEvt->type=EVENT;
				}
				else 																												// if we wand enable a PERIOD
				{
					pEvt->type=PERIOD;
					pEvt->emonth=pEvt->month;																	// day and month is always used
					pEvt->eday=pEvt->day;
					if (pEvt->year) pEvt->eyear=pEvt->year;										// if we have a start-year, also use end-year
					if (pEvt->hour!=-1)																				// if we have a start-time, also use end-time
					{
						pEvt->ehour=pEvt->hour;
						pEvt->emin=pEvt->min;
					}
				}
			} break;

			// an number has been entered
			case RC_0: case RC_1: case RC_2: case RC_3: case RC_4:
			case RC_5: case RC_6: case RC_7: case RC_8: case RC_9:
			{
				if (pEditInt!=NULL)																					// do we have an integer selected
				{
					
					sprintf(info,szFmtStr[iFmtIdx],*pEditInt);
					info[iEditCol]=tmprc;
					*pEditInt=atoi(info);
					iGotoRight=1;																							// goto next position
				}
				else
				{
					if ((iKeyboardMode) || (tmprc==RC_0))											// using a keyboard or '0'
					{
						pEvt->info[iEditCol]=tmprc;															// set character
						iGotoRight=1;																						// goto next position
					}
					else																											// not using a keyboard					
					{						
						char* p;
						p=strchr(szKeyBoxInfo[tmprc-RC_1],pEvt->info[iEditCol]);// is the actual character a subset of this key-sequence
						if (p==NULL)
						{
							if (iShortTimer==0)
								pEvt->info[iEditCol]=szKeyBoxInfo[tmprc-RC_1][0];		// add the first char
							else
							{
								if (iEditCol<MAXINFOEDITLEN+1)
								{
									if (pEvt->info[iEditCol+1]==0) pEvt->info[iEditCol+2]=0;
									iEditCol++;
								}
								pEvt->info[iEditCol]=szKeyBoxInfo[tmprc-RC_1][0];		// add the first char
							}
						}
						else 
						{
							if (*(p+1)==0) pEvt->info[iEditCol]=szKeyBoxInfo[tmprc-RC_1][0];
							else pEvt->info[iEditCol]=*(p+1);
						}
						iShortTimer=REPKEYDELAY;
					}
				}
			}
			break;

			// delete previous character
			case RC_BS:
				if ((pEditInt==NULL) && (iEditCol))													// do we have the info selected and at least 2 chars
				{
					iEditCol--;																								// step one position left an fallthrough to RC_MUTE
				}
				else break;
			
			// delete actual character
			case RC_ENTF:			
			case RC_MUTE:			
				if (pEditInt==NULL)																					// do we have the info selected
				{
					if (pEvt->info[iEditCol+1]!=0)														// only delete if there is something to delete
					{
						strcpy(info,&pEvt->info[iEditCol+1]);
						strcpy(&pEvt->info[iEditCol],info);
					}
				}
			break;
			
			// finish the edit-dialog
			// TODO: security-question
			case RC_OK:
				if (CheckEvent(pEvt))
				{
					if (MessageBox(infomsg1[0][osdidx],infomsg1[3][osdidx]))	// save changes?
						return 1;																								// return YES
				}
				break;
			case RC_HOME:	
				if (MessageBox(infomsg1[0][osdidx],infomsg1[4][osdidx]))		// lose changes?
					return 0;							// return NO
				break;
			
			default:
				if (pEditInt==NULL)																					// do we have the info selected			
				{
					if ((tmprc>=' ') && (tmprc<=255))													// printable?
					{
						iKeyboardMode=1;
						pEvt->info[iEditCol]=tmprc;															// set character
						iGotoRight=1;																						// goto next position
					}
				}
		}

		if ((iShortTimer) && ((tmprc<RC_0) || (tmprc>RC_9)) && (tmprc != 0xFFFF))
		{
			iShortTimer=0;
		}
		if (iShortTimer==1)
		{
			iShortTimer=0;
			iGotoRight=1;
		}
		
		if (iGotoRight)																								// we step one position to the right
		{
			if (++iEditCol==iTypeInt) 																	// when we steped past the last char of a field
			{
				iEditLine++;																							// goto the next field, but
				if ((iEditLine==3) && (pEvt->year==0)) iEditLine=4;				// if no start-year, goto the next field
				if ((iEditLine==4) && (pEvt->hour==-1)) iEditLine=1;			// if no start-time, goto the first field
				if (iEditLine==6) iEditLine=1;														// if we have been at the last field in this line, goto the first
				if ((iEditLine==8) && (pEvt->eyear==0)) iEditLine=9;			// if no end-year, goto the next field
				if ((iEditLine==9) && (pEvt->ehour==-1)) iEditLine=6;			// if no end-time, goto the first field in this line
				if (iEditLine==12) 																				// if last char of info-line
				{
					if (iTypeInt<(MAXINFOEDITLEN-1))												// we can still add a char to the info
					{
						pEvt->info[iEditCol]=' ';															// add the space
						pEvt->info[iEditCol+1]=0;															// end the string
					}
					else iEditCol=0;																				// else, we step to the first char
					iEditLine=11;																						// and of course we stay at the info-line
				}
				else iEditCol=0;																					// first char of any field (except info-field)
			}
		}

	// render the output for the current event, return the pointer to the selected
	// integer-value, NULL if none selected
		pEditInt=PaintEdit(pEvt, iEditLine, iEditCol);
	}
	return 0;
}

/******************************************************************************
 * PaintGrid
 ******************************************************************************/
/*!
 * paint the calendar-grid
 

*/
void PaintGrid(int last, int start, int end, int akt, int sel, int infolines, int* iSelInfo)
{
	int x,y;
	int iCnt=0;
	int cy;
	int iDate;
	char info[80];	
	int cols=4;
	int iEvt;
	int iTmpMonth, iTmpYear;
	int l,t,r,b;
		
	// how many calendar-lines do we need
	if (end>5*7) cols=6;
	else if (end>4*7) cols=5;
	
	// height of calendar-grid depends on showing infolines and calendar-lines
	if (infolines) cy=GRIDBOX_CY2/cols;
	else cy=GRIDBOX_CY1/cols;
	
	// background (just for testing)
  // RenderBox(0, 0, MAXSCREEN_X, MAXSCREEN_Y, FILL, WHITE);
  // sprintf(info,"%d %d %d %d %d %d",last,start,end,akt,sel,infolines);
  // RenderString(info,0,GRIDLINE-2,MAXSCREEN_X-4, LEFT, SMALL, GREY);
  // return;
	
	// header (paint date and time)
	RenderBox(0, 0, MAXSCREEN_X,GRIDLINE, FILL, SKIN3);
	RenderBox(0, 0, MAXSCREEN_X,GRIDLINE, GRID, SKIN2);
	RenderBox(MAXSCREEN_X/2, 0, MAXSCREEN_X/2,GRIDLINE, GRID, SKIN2);
	strftime(info,80,infomsg[DATE][osdidx],at);
	RenderString(info,0,GRIDLINE-4,MAXSCREEN_X-4, FIXEDRIGHT, SMALL, GREY);
	sprintf(info,"%s %u",monthmsg[tShow_mon-1][osdidx],tShow_year);
	RenderString(info,0+4, GRIDLINE-4,MAXSCREEN_X/2, CENTER, NORMAL, BLACK);
	
	// only paint if a key has been pressed
//	if (rccode!=0xFFFF) return;
	
	// paint monday to sunday
	for (x=0; x<7; x++)
	{
		RenderBox(0+x*GRIDBOX_X, GRIDLINE, (x+1)*GRIDBOX_X,GRIDCAL, FILL, SKIN0);
		RenderBox(0+x*GRIDBOX_X, GRIDLINE, (x+1)*GRIDBOX_X,GRIDCAL, GRID, SKIN2);
		RenderString(days[x][osdidx], x*GRIDBOX_X, GRIDCAL-2, GRIDBOX_X, CENTER, SMALL, WHITE);
	}

	// paint calendar squares
	for (y=0; y<cols; y++)
	{
		for (x=0; x<7; x++)
		{
			iCnt++;
			l=0+x*GRIDBOX_X;
			r=(x+1)*GRIDBOX_X;
			b=GRIDCAL+(y+1)*cy;
			t=GRIDCAL+y*cy;
			
			// fill background depending on type of day
			if (iCnt==akt) 													RenderBox(l, t, r, b, FILL, DAY1);		// actual day
			else if (iCnt==sel)											RenderBox(l, t, r, b, FILL, DAY2);		// selected day
			else if ((iCnt<start) || (iCnt>end))		RenderBox(l, t, r, b, FILL, DAY3);		// day outside month
			else if (x>4)														RenderBox(l, t, r, b, FILL, DAY4);		// saturday or sunday
			else 																		RenderBox(l, t, r, b, FILL, DAY5);	 // all other days

			// draw border around the day
			if (iCnt==sel)	RenderBox(l, t, r, b, GRID, SKIN1);
			else						RenderBox(l, t, r, b, GRID, SKIN2);

			// calculate the day/month/year for the selected field
			iTmpMonth=tShow_mon;
			iTmpYear=tShow_year;
			if (iCnt<start) 
			{
				iDate=last-(start-iCnt)+1;	
				iTmpMonth--;
				if (iTmpMonth==0)
				{
					iTmpMonth=12;
					iTmpYear--;
				}
			}
			else if (iCnt>end) 
			{
				iDate=iCnt-end;
				iTmpMonth++;
				if (iTmpMonth==13)
				{
					iTmpMonth=1;
					iTmpYear++;
				}
			}
			else 
			{
				iDate=iCnt-start+1;
			}
			
			// get all event for this day
			iEvt=IsEvent(iDate,iTmpMonth,iTmpYear);

			// paint date in the left-top corner of the box
			sprintf(info,"%u",iDate);
			RenderString(info,0+x*GRIDBOX_X+4, GRIDCAL+y*cy+FONTSIZE_SMALL, GRIDBOX_X, LEFT, SMALL, BLACK);
			
			// if we have found some events, we will paint markers and/or lines
			if (iEvt)	
			{
				int i;
				for (i=0; i<iEvt; i++)
				{
					
					if (iEventType[i]>MAXENTRYS)		// special variable/fixed holidays
					{
						RenderSObject(0+x*GRIDBOX_X+25,GRIDCAL+y*cy+5,MAGENTA,OBJ_MARKER);
  					RenderBox(0+x*GRIDBOX_X+2, GRIDCAL+(y+1)*cy-10, (x+1)*GRIDBOX_X-2,GRIDCAL+(y+1)*cy-6, FILL, BLUE);
					}
					else
					{
  					switch (eventdb[iEventType[i]].type)
  					{
  						case BIRTHDAY: RenderSObject(0+x*GRIDBOX_X+45,GRIDCAL+y*cy+5,RED,OBJ_HEART); break;
  						case EVENT: RenderSObject(0+x*GRIDBOX_X+63,GRIDCAL+y*cy+5,BLUE,OBJ_CLOCK); break;
  						case PERIOD: RenderBox(0+x*GRIDBOX_X+2, GRIDCAL+(y+1)*cy-20, (x+1)*GRIDBOX_X-2,GRIDCAL+(y+1)*cy-16, FILL, RED); break;
  						case HOLIDAY: RenderBox(0+x*GRIDBOX_X+2, GRIDCAL+(y+1)*cy-10, (x+1)*GRIDBOX_X-2,GRIDCAL+(y+1)*cy-6, FILL, BLUE); break;
  					}
					}
				}
			}
		}
	}

	// paint thick lines to seperate month
	if (start>1)
	{
		RenderBox(0, GRIDCAL+cy-LNWIDTH, (start-1)*GRIDBOX_X+LNWIDTH,GRIDCAL+cy+LNWIDTH, FILL, BLACK);
		RenderBox((start-1)*GRIDBOX_X-LNWIDTH, GRIDCAL, (start-1)*GRIDBOX_X+LNWIDTH,GRIDCAL+cy, FILL, BLACK);
	}
	if (end < cols*7)
	{
		x=cols*7-end;
		RenderBox(MAXSCREEN_X-x*GRIDBOX_X, GRIDCAL+(cols-1)*cy-LNWIDTH, MAXSCREEN_X,GRIDCAL+(cols-1)*cy+LNWIDTH, FILL, BLACK);
		RenderBox(MAXSCREEN_X-x*GRIDBOX_X-LNWIDTH, GRIDCAL+(cols-1)*cy, MAXSCREEN_X-x*GRIDBOX_X+LNWIDTH,GRIDCAL+cols*cy, FILL, BLACK);
	}
	
	// do we want to see some (max. 3) event on the screen
	if (infolines)
	{
		// paint selected date and info-lines
		RenderBox(0, GRIDCAL+GRIDBOX_CY2, MAXSCREEN_X,GRIDCAL+GRIDBOX_CY2+GRIDLINE_INFO, FILL, SKIN3);
		RenderBox(0, GRIDCAL+GRIDBOX_CY2, MAXSCREEN_X,GRIDCAL+GRIDBOX_CY2+GRIDLINE_INFO, GRID, SKIN2);

		iEvt=IsEvent(tShow_day,tShow_mon,tShow_year);
		// set maximum for selected entry
		if (iEvt<*iSelInfo) *iSelInfo=iEvt;
		
		int iW;
		iW=WeekNumber(tShow_year, tShow_mon, tShow_day);
		
		if (osdidx == 0)
			sprintf(info,"W %d    %u. %s %u     %u %s",iW,tShow_day,monthmsg[tShow_mon-1][osdidx],tShow_year,iEvt,infotype[1][osdidx]);
		else 
			sprintf(info,"W %d    %s %u, %u     %u %s",iW,monthmsg[tShow_mon-1][osdidx],tShow_day,tShow_year,iEvt,infotype[1][osdidx]);
		RenderString(info,0+4, GRIDCAL+GRIDBOX_CY2+GRIDLINE_INFO-4,MAXSCREEN_X, CENTER, NORMAL, BLACK);

		int iMarkline=*iSelInfo;
		if (*iSelInfo>3) iMarkline=3;
		PEVT_DB pEvt=NULL;			
		for (y=1;y<=3;y++)
		{
			int iSelEvt=y-1;																	// iSelEvent is the index for the event we print
			if (*iSelInfo>3) iSelEvt+=(*iSelInfo-3);
			
			int iType;
			RenderBox(0, GRIDCAL+GRIDBOX_CY2+y*GRIDLINE_INFO, MAXSCREEN_X,GRIDCAL+GRIDBOX_CY2+(y+1)*GRIDLINE_INFO, FILL, (y==iMarkline) ? LGREY : WHITE);
			RenderBox(0, GRIDCAL+GRIDBOX_CY2+y*GRIDLINE_INFO, MAXSCREEN_X,GRIDCAL+GRIDBOX_CY2+(y+1)*GRIDLINE_INFO, GRID, SKIN2);
			
			// variable/fixed holidays
			if ((iEvt>=y) && (iEventType[iSelEvt]>MAXENTRYS))
			{
				RenderSObject(0,GRIDCAL+GRIDBOX_CY2+(y)*GRIDLINE_INFO+8,MAGENTA,OBJ_MARKER);
				RenderString(vdaysnames[iEventType[iSelEvt]-MAXENTRYS-1][osdidx],20,GRIDCAL+GRIDBOX_CY2+(y+1)*GRIDLINE_INFO-4,MAXSCREEN_X-4,LEFT,SMALL,BLACK);
			}			
			// do we have an event in this line?
			else if ((iEvt>=y) && (iType=eventdb[iEventType[iSelEvt]].type))
			{
				pEvt=&eventdb[iEventType[iSelEvt]];
				
				// write info for birthday
				if (iType == BIRTHDAY)
				{
					sprintf(info,"%s: %s (%u)",infotype[0][osdidx],pEvt->info,tShow_year-pEvt->year);
					RenderSObject(2,GRIDCAL+GRIDBOX_CY2+(y)*GRIDLINE_INFO+8,RED,OBJ_HEART);
					RenderString(info,20,GRIDCAL+GRIDBOX_CY2+(y+1)*GRIDLINE_INFO-4,MAXSCREEN_X-4,LEFT,SMALL,BLACK);
				}
				// write info for an event
				else if (iType == EVENT)
				{
					if (pEvt->hour!=-1)
						sprintf(info,"%02u:%02u %s",pEvt->hour,pEvt->min,pEvt->info);
					else 
						sprintf(info,"%s",pEvt->info);				
					RenderSObject(2,GRIDCAL+GRIDBOX_CY2+(y)*GRIDLINE_INFO+8,BLUE,OBJ_CLOCK);
					RenderString(info,20,GRIDCAL+GRIDBOX_CY2+(y+1)*GRIDLINE_INFO-4,MAXSCREEN_X-4,LEFT,SMALL,BLACK);
				}
				// write info for a holiday
				else if (iType == HOLIDAY)
				{
					sprintf(info,"%s: %s",infotype[3][osdidx],pEvt->info);
					RenderString(info,5,GRIDCAL+GRIDBOX_CY2+(y+1)*GRIDLINE_INFO-4,MAXSCREEN_X-4,LEFT,SMALL,BLACK);
				}
				// write info for a timeperiod
				else if (iType == PERIOD)
				{
					if ((pEvt->year!=0) && (pEvt->hour!=-1))
					{
						char info1[8];
						char info2[8];
						if ((pEvt->year==tShow_year) && (pEvt->month==tShow_mon) && (pEvt->day==tShow_day))
							sprintf(info1,"%02u:%02u >",pEvt->hour,pEvt->min);
						else info1[0]=0;
						if ((pEvt->eyear==tShow_year) && (pEvt->emonth==tShow_mon) && (pEvt->eday==tShow_day))
							sprintf(info2,"> %02u:%02u",pEvt->ehour,pEvt->emin);
						else info2[0]=0;
						
						sprintf(info,"%s: %s %s%s",infotype[2][osdidx],pEvt->info,info1,info2);
					}
					else
						sprintf(info,"%s: %s",infotype[2][osdidx],pEvt->info);
					RenderString(info,5,GRIDCAL+GRIDBOX_CY2+(y+1)*GRIDLINE_INFO-4,MAXSCREEN_X-4,LEFT,SMALL,BLACK);
				}
			}
		}

		// if we have more than 3 event, we paint arrow(s) to signal this
		if (iEvt>3)
		{
			if (*iSelInfo!=iEvt)
				RenderSObject(MAXSCREEN_X-18,GRIDCAL+GRIDBOX_CY2+3*GRIDLINE_INFO+10,SKIN1,OBJ_SCROLLDN);
			if (*iSelInfo>3)
				RenderSObject(MAXSCREEN_X-18,GRIDCAL+GRIDBOX_CY2+1*GRIDLINE_INFO+10,SKIN1,OBJ_SCROLLUP);
		}				
		
	}
		
	// footer 
	RenderBox(0, MAXSCREEN_Y-GRIDLINE_SMALL, MAXSCREEN_X,MAXSCREEN_Y, FILL, SKIN3);
	RenderBox(0, MAXSCREEN_Y-GRIDLINE_SMALL, MAXSCREEN_X,MAXSCREEN_Y, GRID, SKIN2); 
	
	if (infolines)
	{
		RenderSObject(4,MAXSCREEN_Y-GRIDLINE_SMALL+4,RED,OBJ_CIRCLE);	
		RenderString(infohelp[0][osdidx],25,MAXSCREEN_Y-4,MAXSCREEN_X/4,LEFT,SMALL,BLACK);	
		RenderSObject(4+MAXSCREEN_X/4,MAXSCREEN_Y-GRIDLINE_SMALL+4,GREEN,OBJ_CIRCLE);
		RenderString(infohelp[1][osdidx],25+MAXSCREEN_X/4,MAXSCREEN_Y-4,MAXSCREEN_X/4,LEFT,SMALL,BLACK);	
		RenderSObject(4+MAXSCREEN_X/2,MAXSCREEN_Y-GRIDLINE_SMALL+4,YELLOW,OBJ_CIRCLE);	
		RenderString(infohelp[2][osdidx],25+MAXSCREEN_X/2,MAXSCREEN_Y-4,MAXSCREEN_X/4,LEFT,SMALL,BLACK);	
		RenderSObject(4+3*MAXSCREEN_X/4,MAXSCREEN_Y-GRIDLINE_SMALL+4,BLUE,OBJ_CIRCLE);	
		RenderString(infohelp[3][osdidx],25+3*MAXSCREEN_X/4,MAXSCREEN_Y-4,MAXSCREEN_X/4,LEFT,SMALL,BLACK);	
	}
	else
	{
		RenderString(infohelp[4][osdidx],4,MAXSCREEN_Y-4,MAXSCREEN_X-8,LEFT,SMALL,BLACK);	
	}

	// output to framebuffer
	memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);
}

/******************************************************************************
 * LeapYear
 ******************************************************************************/
/*!
 * calculate if we have a leap-year
 
\return 1 if leap-year
*/
int LeapYear(int year)
{
	if (((year % 4 == 0) && (!( year % 100 == 0))) && (year % 400 != 0))
	{
		return 1;
	}
	return 0;
}

/*
The basic steps for a date in the years 2000-2099 are as follows:

Example date July 13th, 2004

   1. Take the last 2 digits of the year and add a quarter onto itself. (04 + 1 = 5)
   2. Get the corresponding code for the month. (January = 6, February = 2, March = 2, etc. See month codes for details). July = 5
   3. Take the day. (=13)
   4. Add the numbers together (5 + 5 + 13 = 23)
   5. Take away 7 (or multiples of 7) until a number from 1-7 is left. (23 - 21 =2)
   6. This number corresponds to the day of the week. (1 = Monday, 2 = Tuesday, etc.) In this case 2 = Tuesday

Apart from the basic steps, other elements have to be taken into account:

    * When adding a quarter of the year onto itself, If the quarter of the year is not a whole number, simply ignore the decimals. Do not round up. Therefore 27/4 = 6.75 = 6, and 2/4 = 0.5 = 0.
       
    * Leap years: subtract 1 from the total if the month is January or February.
       
    * Negative numbers. During the calculation you get 0 or negative numbers, just add seven until you get a number from 1-7.
       
    * Different "centuries" *.
          o 1700s add 5
          o 1800s add 3
          o 1900s add 1
          o 2100s subtract 2
          o 2200s subtract 4


Jan  	Feb  	Mar  	Apr.  May  	Jun  	Jul  	Ago  	Sept  Oct  	Nov  	Dec
6 	  2 	  2 	  5  	  0 	  3 	  5 	  1 	  4 	  6 	  2 	  4
*/
/******************************************************************************
 * DayOfWeek
 ******************************************************************************/
/*!
 * return the day-of-the-week
 * monday=1, tueday=2, etc. 

*/
int DayOfWeek(int day, int month, int year)
{
	int iCalc;

	// no calculations out of range, return monday :)
	if ((year<1700) || (year>=2300)) return 1;
		
	// Take the last 2 digits of the year and add a quarter onto itself. (04 + 1 = 5)
	iCalc=(year % 100) + (year % 100)/4;
	
	// Get the corresponding code for the month. (January = 6, February = 2, March = 2, etc. See month codes for details). July = 5
	// Take the day. (=13)
	// Add the numbers together (5 + 5 + 13 = 23)
	iCalc+=monthcode[month-1]+day;

	// Leap years: subtract 1 from the total if the month is January or February.
	if ((LeapYear(year)) && (month<3)) iCalc--;

	// Different "centuries" *.
	//           o 1700s add 5
	//           o 1800s add 3
	//           o 1900s add 1
	//           o 2100s subtract 2
	//           o 2200s subtract 4
	if (year<1800)       iCalc+=5;
	else if (year<1900)  iCalc+=3;
	else if (year<2000)  iCalc+=1;
	else if (year>=2200) iCalc-=4;
	else if (year>=2100) iCalc-=2;

	// Negative numbers. During the calculation you get 0 or negative numbers, just add seven until you get a number from 1-7.
	while (iCalc<1) iCalc+=7;
			
	// Take away 7 (or multiples of 7) until a number from 1-7 is left. (23 - 21 =2)
	while (iCalc>7) iCalc-=7;
	
	return iCalc;
}

/******************************************************************************
 * AddDays
 ******************************************************************************/
/*!
 * add days (max. +/- 28) to date

*/
void AddDays(int* pday, int* pmonth, int* pyear, int adddays)
{
	if ((adddays>28) || (adddays<-28)) return;
	
	int day = *pday;
	int month = *pmonth;
	int year = *pyear;
	int iLeapYear;
	
	iLeapYear=LeapYear(year);
	
	// substract days
	if (adddays<0)
	{
		day+=adddays;
		if (day<1)
		{
			month--;
			if (month<1)
			{
				month=12;
				year--;
			}
			day+=monthdays[iLeapYear][month-1];
		}
	}
	
	// add days
	if (adddays>0)
	{
		day+=adddays;
		if (day > monthdays[iLeapYear][month-1])
		{
			day-=monthdays[iLeapYear][month-1];
			month++;
			if (month>12)
			{
				month=1;
				year++;
			}
		}
	}

	*pday=day;
	*pmonth=month;
	*pyear=year;
}

/******************************************************************************
 * WeekNumber
 ******************************************************************************/
/*!
 * calculate the ISO Weeknumber
 * code from http://www.proesite.com/timex/wkcalc.htm
 
*/
int WeekNumber( int y, int m, int d )
{
	int days;
	int dow0401;
	int offset;
	int week;
	
  dow0401 = DayOfWeek( 4, 1, y );										// weekday for 4th january, this day is in the first week
  
  days = __mon_yday[LeapYear(y)?1:0][m-1] + d;			// days in this year

	offset = dow0401 - 4;															// offset for 1.1 for monday of week 1
	days = days + offset -1;
	if (days<0)
	{
		int iWD=DayOfWeek( 1, 1, y-1);
		if ((iWD==4) || ((LeapYear(y-1)) && (iWD==3))) week = 53;
		else week = 52; 
	}
	else week = (days / 7) +1;

	if ((days > 360) && (week > 52)) 
	{
		int iWD=DayOfWeek( 1, 1, y);
		if ((iWD==4) || ((LeapYear(y)) && (iWD==3))) week = 53;
		else week = 1; 
   }
	
	return week;
	
/*	
  dow     = DayOfWeek( d, m, y ) -1 ;
  dow0101 = DayOfWeek( 1, 1, y ) -1 ;

  if ( m == 1  &&  3 < dow0101 < 7 - (d-1) )
  {
    // days before week 1 of the current year have the same week number as
    // the last day of the last week of the previous year

    dow     = dow0101 - 1;
    dow0101 = DayOfWeek( 1, 1, y-1 ) -1 ;
    m       = 12;
    d       = 31;
  }
  else if ( m == 12  &&  30 - (d-1) < (DayOfWeek( 1, 1, y+1 ) -1) < 4 )
  {
    // days after the last week of the current year have the same week number as
    // the first day of the next year, (i.e. 1)

     return 1;
  }

  return ( (DayOfWeek( 1, 1, y ) -1) < 4 ) + 4 * (m-1) + ( 2 * (m-1) + (d-1) + dow0101 - dow + 6 ) * 36 / 256;


	
	int Woche=0;
	int Wochtag1Jan=CurrentDoomDay(jahr);
	int Tage=tagesnummer(tag, monat, jahr)-1;


	if (Wochtag1Jan > 3)
      		Tage = Tage - (7 - Wochtag1Jan);
   	else Tage = Tage + Wochtag1Jan;
	
	if (Tage < 0)
      		if (   (Wochtag1Jan == 4) || (CurrentDoomDay(jahr-1) == 3))
        		 Woche = 53;
      		else Woche = 52;
 	else Woche = (Tage/7) + 1;
	
	if ((Tage > 360) && (Woche > 52)) {
      		if (Wochtag1Jan == 3)
          		Woche = 53;
      		else if (CurrentDoomDay(jahr+1) == 4)
              		Woche = 53;
        	  else Woche = 1;
   }

   return Woche;
*/
}

/******************************************************************************
 * CalcEastern
 ******************************************************************************/
/*!
 * 
 - Christi Himmelfahrt (+39), 
 - Pfingsten (+49), 
 - Fronleichnam (+60), 

Der Muttertag ist der zweite Sonntag im Mai, 
das Erntedankfest der erste Sonntag im Oktober (jedoch nicht überall!). 
Der 1. Advent ist der Sonntag nach dem 26. November; 
der Buß- und Bettag liegt 11 Tage vor dem 1. Advent. 

 
*/
void CalcEastern(int year, int* month, int* day)
{
	int a,b,c,d,e;
	int dM, dA;
	int M = 24, N=5;
	
	if (year<1800)      	{	M=23; N=3; }
	else if (year<1900)		{	M=23; N=4; }
	else if (year<2100)		{	M=24; N=5; }
	else if (year<2200)		{	M=24; N=6; }
	
	a= year % 19;
	b= year % 4;
	c= year % 7;
	d= (19*a + M) % 30;
	e= (2*b + 4*c + 6*d + N) % 7;
	
	dM = (22 + d + e);									// eastern in march dM >= 22
	dA = (d + e - 9);										// eastern in april dA <= 25

	*month=3; 
	*day=dM; 														// we think eastern is in march
	
	if (dM > 31)	
	{
		if (dA == 26)											// special rule
		{ *month=4; *day=19; }
		else if ((dA == 25) && (d == 28) && (a > 10))	// another special rule
		{ *month=4; *day=18; }
		else
		{ *month=4; *day=dA; }
	}
}

/******************************************************************************
 * FillEasternDays
 ******************************************************************************/
/*!
 * fill the structure for this years for eastern etc.
 - Christi Himmelfahrt (+39), 
 - Pfingsten (+49), 
 - Fronleichnam (+60), 

Der Muttertag ist der zweite Sonntag im Mai, 
das Erntedankfest der erste Sonntag im Oktober (jedoch nicht überall!). 
Der 1. Advent ist der Sonntag nach dem 26. November; 
der Buß- und Bettag liegt 11 Tage vor dem 1. Advent. 

 
*/
void FillEasternDays(int year)
{
	int mon, day;
	int mon1, day1;
	
	CalcEastern(year, &mon, &day);
	mon1=mon; day1=day;
	varaibledays[OFFSET_E-1].mon = mon;			// eastern
	varaibledays[OFFSET_E-1].day = day;
	AddDays(&day, &mon, &year, 1);
	varaibledays[OFFSET_EM-1].mon = mon;		// eastern monday
	varaibledays[OFFSET_EM-1].day = day;
	
	AddDays(&day, &mon, &year, 19);					// +39 days from eastern
	AddDays(&day, &mon, &year, 19);
	varaibledays[OFFSET_H-1].mon = mon;			// christi himmelfahrt
	varaibledays[OFFSET_H-1].day = day;
	
	AddDays(&day, &mon, &year, 10);
	varaibledays[OFFSET_P-1].mon = mon;			// pfingsten
	varaibledays[OFFSET_P-1].day = day;
	AddDays(&day, &mon, &year, 1);
	varaibledays[OFFSET_PM-1].mon = mon;		// pfingsten montag
	varaibledays[OFFSET_PM-1].day = day;
	
	AddDays(&day, &mon, &year, 10);
	varaibledays[OFFSET_F-1].mon = mon;			// fronleichnam
	varaibledays[OFFSET_F-1].day = day;
	
	AddDays(&day1, &mon1, &year, -2);
	varaibledays[OFFSET_KF-1].mon = mon1;		// karfreitag
	varaibledays[OFFSET_KF-1].day = day1;
	AddDays(&day1, &mon1, &year, -1);
	varaibledays[OFFSET_GD-1].mon = mon1;		// gruendonnerstag
	varaibledays[OFFSET_GD-1].day = day1;

	AddDays(&day1, &mon1, &year, -23);
	AddDays(&day1, &mon1, &year, -20);
	varaibledays[OFFSET_A-1].mon = mon1;		// aschermittwoch
	varaibledays[OFFSET_A-1].day = day1;
	AddDays(&day1, &mon1, &year, -2);
	varaibledays[OFFSET_RM-1].mon = mon1;		// rosenmontag
	varaibledays[OFFSET_RM-1].day = day1;

	//mothersday is 2. sunday in may
	int iWDay;
	iWDay=DayOfWeek(1,5,year);
	varaibledays[OFFSET_M-1].mon = 5;
	varaibledays[OFFSET_M-1].day = 15-iWDay;
	
	// sommertime
	iWDay=DayOfWeek(31,3,year);
	if (iWDay!=7)	varaibledays[OFFSET_SZ-1].day = 31-iWDay;
	else varaibledays[OFFSET_SZ-1].day = 31;
	varaibledays[OFFSET_SZ-1].mon = 3;

	// wintertime	
	iWDay=DayOfWeek(31,10,year);
	if (iWDay!=7)	varaibledays[OFFSET_WZ-1].day = 31-iWDay;
	else varaibledays[OFFSET_WZ-1].day = 31;
	varaibledays[OFFSET_WZ-1].mon = 10;
	
	varaibledays[OFFSET_W0-1].day=24;					// heiliger abend
	varaibledays[OFFSET_W0-1].mon=12;
	varaibledays[OFFSET_W1-1].day=25;					// 1. weihnachtstag
	varaibledays[OFFSET_W1-1].mon=12;
	varaibledays[OFFSET_W2-1].day=26;					// 2. weihnachtstag
	varaibledays[OFFSET_W2-1].mon=12;
	varaibledays[OFFSET_3K-1].day=6;					// hl. 3 koenige
	varaibledays[OFFSET_3K-1].mon=1;
	varaibledays[OFFSET_N-1].day=1;						// neujahr
	varaibledays[OFFSET_N-1].mon=1;
	varaibledays[OFFSET_S-1].day=31;					// sylvester
	varaibledays[OFFSET_S-1].mon=12;
	varaibledays[OFFSET_V-1].day=14;					// valentinstag
	varaibledays[OFFSET_V-1].mon=2;
	varaibledays[OFFSET_1M-1].day=1;					// 1. may
	varaibledays[OFFSET_1M-1].mon=5;
	varaibledays[OFFSET_MH-1].day=15;					// maria himmelfahrt
	varaibledays[OFFSET_MH-1].mon=8;
	varaibledays[OFFSET_NI-1].day=6;					// nikolaus
	varaibledays[OFFSET_NI-1].mon=12;
	varaibledays[OFFSET_ND-1].day=3;					// tag der deutschen einheit
	varaibledays[OFFSET_ND-1].mon=10;
	varaibledays[OFFSET_NA-1].day=26;					// nationalfeiertag oesterreich
	varaibledays[OFFSET_NA-1].mon=10;
}

/******************************************************************************
 * IsEvent
 ******************************************************************************/
/*!
 * is there an event at this day

*/
int IsEvent(int day, int month, int year)
{
	int iEntry=0;
	int iCnt=0;
	int iFound;
	int days = __mon_yday[LeapYear(year) ? 1 : 0][month-1]+day;
	
	// first check for any of the holidays: eastern, etc.
	int i;
	for (i=1;i<=NOF_VDAYS;i++)
		if ((varaibledays[i-1].mon==month) && (varaibledays[i-1].day==day))
			iEventType[iCnt++]=MAXENTRYS+i;
	
	while (iEntry<MAXENTRYS)
	{
		// abort if no entry
		if (eventdb[iEntry].type == FREE) break;
		
		iFound=0;
		// check for holiday or event
		if ((eventdb[iEntry].type == HOLIDAY) || (eventdb[iEntry].type == EVENT))
		{
			if ((eventdb[iEntry].day == day) && (eventdb[iEntry].month == month) && ((eventdb[iEntry].year==0) || (eventdb[iEntry].year == year)))
				iFound=1;
		}

		// check for birthday
		else if ((eventdb[iEntry].type == BIRTHDAY) && (eventdb[iEntry].day == day) && (eventdb[iEntry].month == month))
		{
			iFound=1;		
		}
	
		// check for period
		else if (eventdb[iEntry].type == PERIOD)
		{		
			// don't check the year
			if (eventdb[iEntry].year==0)
			{
				if (LeapYear(year))
				{
					int iTmpDays1=eventdb[iEntry].days;
					int iTmpDays2=eventdb[iEntry].edays;
					
					if (iTmpDays1 >= __mon_yday[0][2]) iTmpDays1++;
					if (iTmpDays2 >= __mon_yday[0][2]) iTmpDays2++;
					if ((iTmpDays1 <= days) && (iTmpDays2 >= days))
						iFound=1;
				}
				else
				{
					if ((eventdb[iEntry].days <= days) && (eventdb[iEntry].edays >= days))
						iFound=1;
				}
			}
			else
			{
					if (((eventdb[iEntry].year < year) || ((eventdb[iEntry].year == year) && (eventdb[iEntry].days <= days))) && 
						  ((eventdb[iEntry].eyear > year) || ((eventdb[iEntry].eyear == year) && (eventdb[iEntry].edays >= days))))
						iFound=1;					 
			}
		}

		if (iFound)
		{
			iEventType[iCnt]=iEntry;
			iCnt++;
		}
		
		iEntry++;
		if (iCnt==MAXPERDAY) break;
	}
	
	return iCnt;
}

/******************************************************************************
 * LoadDatabase
 ******************************************************************************/
/*!
 * load the database from file

*/
void LoadDatabase(void)
{
	// clear database
	memset(eventdb,0,sizeof(eventdb));
	
	char linebuffer[1024];
	FILE *fd_evt;
	int iEntry=0;
	int iLen;
	char* p1;
	char* p2;
	
	// read the tuxcal-event-file
	if ((fd_evt = fopen(CFGPATH EVTFILE, "r"))!=NULL)
	{
		// read line by line
		while (fgets(linebuffer, sizeof(linebuffer), fd_evt))
		{			
			p1=linebuffer;
			
			// first check for type
			if ((p2=strchr(p1,';')) == NULL) 
			{
				if ((p2=strchr(p1,'#')) != NULL)								// comment ?
				{
					eventdb[iEntry].type = COMMENT;								
					strcpy(eventdb[iEntry].info,linebuffer);
				}				
				else
				{
					eventdb[iEntry].type = SPACE;									// empty line
				}
				iEntry++;
				if (iEntry >= MAXENTRYS) break;		
				continue;			
			}
			
			switch (p1[0])
			{
				case 'Z':
				case 'z': eventdb[iEntry].type = PERIOD; break;
				case 'G':
				case 'g': eventdb[iEntry].type = BIRTHDAY; break;
				case 'T':
				case 't': eventdb[iEntry].type = EVENT; break;				
				case 'F':
				case 'f': eventdb[iEntry].type = HOLIDAY; break;				
				default:  eventdb[iEntry].type = COMMENT;
			}
			
			
			p1=p2;
			*p1=' ';
			//second check for date
			if ((p2=strchr(p1,';')) == NULL) 
				continue;			
			
			// we have 6 different formats, we use the length for recognition
			//    dd.mm.(6) or dd.mm.yyyy(10) or dd.mm.-dd.mm(13) or dd.mm.yyyy-dd.mm.yyyy(21)
			// or dd.mm.yyyy hh:mm(16) or dd.mm.yyyy hh:mm-dd.mm.yy hh:mm(33)
			*p2=0;
			iLen=strlen(p1)-1;		// stringlen without leading space
			switch (iLen)
			{
				// dd.mm.   all day event/holiday
				case 6: 
					sscanf(p1," %02u.%02u.",&eventdb[iEntry].day,&eventdb[iEntry].month); 
					eventdb[iEntry].days=__mon_yday[0][eventdb[iEntry].month-1]+eventdb[iEntry].day;
					eventdb[iEntry].hour=-1; 
					eventdb[iEntry].ehour=-1; 
				break;
				
				// dd.mm.yyyy   all day event
				case 10:
					sscanf(p1," %02u.%02u.%04u",&eventdb[iEntry].day,&eventdb[iEntry].month,&eventdb[iEntry].year); 
					eventdb[iEntry].days=__mon_yday[LeapYear(eventdb[iEntry].year) ? 1 : 0][eventdb[iEntry].month-1]+eventdb[iEntry].day;
					eventdb[iEntry].hour=-1; 
					eventdb[iEntry].ehour=-1; 
				break;
				
				// dd.mm.-dd.mm.   all day event-period
				case 13:
					sscanf(p1," %02u.%02u.-%02u.%02u.",&eventdb[iEntry].day,&eventdb[iEntry].month,&eventdb[iEntry].eday,&eventdb[iEntry].emonth); 
					eventdb[iEntry].days=__mon_yday[0][eventdb[iEntry].month-1]+eventdb[iEntry].day;
					eventdb[iEntry].edays=__mon_yday[0][eventdb[iEntry].emonth-1]+eventdb[iEntry].eday;
					eventdb[iEntry].hour=-1; 
					eventdb[iEntry].ehour=-1; 
				break;

				// dd.mm.yyyy hh:mm   event at certain time
				case 16:
					sscanf(p1," %02u.%02u.%04u %02u:%02u",&eventdb[iEntry].day,&eventdb[iEntry].month,&eventdb[iEntry].year,&eventdb[iEntry].hour,&eventdb[iEntry].min); 
					eventdb[iEntry].days=__mon_yday[LeapYear(eventdb[iEntry].year) ? 1 : 0][eventdb[iEntry].month-1]+eventdb[iEntry].day;
				break;

				// dd.mm.yyyy-dd.mm.yyyy   all day event period
				case 21:
					sscanf(p1," %02u.%02u.%04u-%02u.%02u.%04u",&eventdb[iEntry].day,&eventdb[iEntry].month,&eventdb[iEntry].year,&eventdb[iEntry].eday,&eventdb[iEntry].emonth,&eventdb[iEntry].eyear); 
					eventdb[iEntry].days=__mon_yday[LeapYear(eventdb[iEntry].year) ? 1 : 0][eventdb[iEntry].month-1]+eventdb[iEntry].day;
					eventdb[iEntry].edays=__mon_yday[LeapYear(eventdb[iEntry].year) ? 1 : 0][eventdb[iEntry].emonth-1]+eventdb[iEntry].eday;
					eventdb[iEntry].hour=-1; 
					eventdb[iEntry].ehour=-1; 
				break;

				// dd.mm.yyyy hh:mm-dd.mm.yyyy hh:mm   event-period
				case 33:
					sscanf(p1," %02u.%02u.%04u %02u:%02u-%02u.%02u.%04u %02u:%02u",&eventdb[iEntry].day,&eventdb[iEntry].month,&eventdb[iEntry].year,&eventdb[iEntry].hour,&eventdb[iEntry].min,&eventdb[iEntry].eday,&eventdb[iEntry].emonth,&eventdb[iEntry].eyear,&eventdb[iEntry].ehour,&eventdb[iEntry].emin); 
					eventdb[iEntry].days=__mon_yday[LeapYear(eventdb[iEntry].year) ? 1 : 0][eventdb[iEntry].month-1]+eventdb[iEntry].day;
					eventdb[iEntry].edays=__mon_yday[LeapYear(eventdb[iEntry].year) ? 1 : 0][eventdb[iEntry].emonth-1]+eventdb[iEntry].eday;
				break;
			}

			p1=p2;
			*p2=' ';
			//third check for info
			if ((p2=strchr(p1,';')) == NULL)
				if ((p2=strchr(p1,'\n')) == NULL)
					continue;			
			
			*p2=0;
			strcpy(eventdb[iEntry].info,&p1[1]);
			
			iEntry++;
			if (iEntry >= MAXENTRYS) break;		
		}
		
		iCntEntries=iEntry;
		
		fclose(fd_evt);
	}
}

/******************************************************************************
 * SaveDatabase
 ******************************************************************************/
/*!
 * save the database to the file

*/
void SaveDatabase(void)
{
	FILE *fd_evt;
	int iEntry=0;
	char info_tm1[7];
	char info_tm2[7];
	char info_yr1[5];
	char info_yr2[5];
		
	// open the tuxcal-event-file
	if ((fd_evt = fopen(CFGPATH EVTFILE, "w"))!=NULL)
	{
		while (iEntry<MAXENTRYS)
		{
			// abort if no entry
			if (eventdb[iEntry].type == FREE) break;
			
			info_tm1[0]=0;
			info_tm2[0]=0;
			if (eventdb[iEntry].hour!=-1) sprintf(info_tm1," %02u:%02u",eventdb[iEntry].hour,eventdb[iEntry].min);
			if (eventdb[iEntry].ehour!=-1) sprintf(info_tm2," %02u:%02u",eventdb[iEntry].ehour,eventdb[iEntry].emin);
			info_yr1[0]=0;
			info_yr2[0]=0;
			if (eventdb[iEntry].year!=0)	sprintf(info_yr1,"%04u",eventdb[iEntry].year);
			if (eventdb[iEntry].eyear!=0) sprintf(info_yr2,"%04u",eventdb[iEntry].eyear);
			
			// check for holiday
			if (eventdb[iEntry].type == HOLIDAY)
			{
				fprintf(fd_evt,"f;%02u.%02u.%s;%s;\n", eventdb[iEntry].day,eventdb[iEntry].month,info_yr1,eventdb[iEntry].info);
			}
			
			// check for event
 			if (eventdb[iEntry].type == EVENT)
			{
				fprintf(fd_evt,"t;%02u.%02u.%s%s;%s;\n", eventdb[iEntry].day,eventdb[iEntry].month,info_yr1,info_tm1,eventdb[iEntry].info);
			}
 
			// check for birthday
			if (eventdb[iEntry].type == BIRTHDAY)
			{
				fprintf(fd_evt,"g;%02u.%02u.%s;%s;\n", eventdb[iEntry].day,eventdb[iEntry].month,info_yr1,eventdb[iEntry].info);
			}

			// check for period
			else if (eventdb[iEntry].type == PERIOD)
			{
					fprintf(fd_evt,"z;%02u.%02u.%s%s-%02u.%02u.%s%s;%s;\n", eventdb[iEntry].day,eventdb[iEntry].month,info_yr1,info_tm1,eventdb[iEntry].eday,eventdb[iEntry].emonth,info_yr2,info_tm2,eventdb[iEntry].info);
			}				
			
			// check if comment
			else if (eventdb[iEntry].type == COMMENT)
			{
				fprintf(fd_evt,"%s", eventdb[iEntry].info);			
			}

			// check if space
			else if (eventdb[iEntry].type == SPACE)
			{
				fprintf(fd_evt,"\n");
			}

			iEntry++;
		}		
		fclose(fd_evt);
	}
	iCntEntries=iEntry;
	LoadDatabase();
}

/******************************************************************************
 * plugin_exec
 ******************************************************************************/

void resetBpp(void)
{
	if (var_screeninfo.bits_per_pixel != prev_bpp)
	{
		var_screeninfo.bits_per_pixel = prev_bpp;
		if (ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo) == -1)
			perror("TuxCal <FBIOPUT_VSCREENINFO>");
	}

}
/*!
 * start the plugin

*/
void plugin_exec(PluginParam *par)
{
	char cvs_revision[] = "$Revision: 1.08 $";
	FILE *fd_run;
	FT_Error error;

	// show versioninfo
	sscanf(cvs_revision, "%*s %s", versioninfo_p);
	printf("TuxCal %s\n", versioninfo_p);

	// get params
	fb = rc = lcd = sx = ex = sy = ey = -1;

	for(; par; par = par->next)
	{
		if (!strcmp(par->id, P_ID_FBUFFER))
		{
			fb = atoi(par->val);
		}
		else if (!strcmp(par->id, P_ID_RCINPUT))
		{
			rc = atoi(par->val);
		}
		else if (!strcmp(par->id, P_ID_LCD))
		{
			lcd = atoi(par->val);
		}
		else if (!strcmp(par->id, P_ID_OFF_X))
		{
			sx = atoi(par->val);
		}
		else if (!strcmp(par->id, P_ID_END_X))
		{
			ex = atoi(par->val);
		}
		else if (!strcmp(par->id, P_ID_OFF_Y))
		{
			sy = atoi(par->val);
		}
		else if (!strcmp(par->id, P_ID_END_Y))
		{
			ey = atoi(par->val);
		}
	}

	if (fb == -1 || rc == -1 || sx == -1 || ex == -1 || sy == -1 || ey == -1)
	{
		printf("TuxCal <missing Param(s)>\n");
		return;
	}

	// keyboard
	kb = open("/dev/vc/0", O_RDONLY);

	// read config
	ReadConf();
	WriteConf();
	
	// init framebuffer
	if (ioctl(fb, FBIOGET_FSCREENINFO, &fix_screeninfo) == -1)
	{
		printf("TuxCal <FBIOGET_FSCREENINFO failed>\n");
		return;
	}

	if (ioctl(fb, FBIOGET_VSCREENINFO, &var_screeninfo) == -1)
	{
		printf("TuxCal <FBIOGET_VSCREENINFO failed>\n");
		return;
	}

	prev_bpp = var_screeninfo.bits_per_pixel;
	if (prev_bpp != 8)
	{
		var_screeninfo.bits_per_pixel = 8;

		if (ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo) == -1)
		{
			perror("TuxCal <FBIOPUT_VSCREENINFO>");
			return;
		}
	}


	if (ioctl(fb, FBIOPUTCMAP, (skin == 1) ? &colormap1 : (skin == 2) ? &colormap2 : &colormap3) == -1)
	{
		printf("TuxCal <FBIOPUTCMAP failed>\n");
		resetBpp();
		return;
	}

	if (!(lfb = (unsigned char*)mmap(0, fix_screeninfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0)))
	{
		printf("TuxCal <mapping of Framebuffer failed>\n");
		resetBpp();
		return;
	}

	// init fontlibrary
	if ((error = FT_Init_FreeType(&library)))
	{
		printf("TuxCal <FT_Init_FreeType failed with Errorcode 0x%.2X>", error);
		resetBpp();
		munmap(lfb, fix_screeninfo.smem_len);
		return;
	}

	if ((error = FTC_Manager_New(library, 1, 2, 0, &MyFaceRequester, NULL, &manager)))
	{
		printf("TuxCal <FTC_Manager_New failed with Errorcode 0x%.2X>\n", error);
		FT_Done_FreeType(library);
		resetBpp();
		munmap(lfb, fix_screeninfo.smem_len);
		return;
	}

	if ((error = FTC_SBitCache_New(manager, &cache)))
	{
		printf("TuxCal <FTC_SBitCache_New failed with Errorcode 0x%.2X>\n", error);
		FTC_Manager_Done(manager);
		FT_Done_FreeType(library);
		resetBpp();
		munmap(lfb, fix_screeninfo.smem_len);
		return;
	}

	if ((error = FTC_Manager_Lookup_Face(manager, FONT, &face)))
	{
		printf("TuxCal <FTC_Manager_Lookup_Face failed with Errorcode 0x%.2X>\n", error);
		FTC_Manager_Done(manager);
		FT_Done_FreeType(library);
		resetBpp();
		munmap(lfb, fix_screeninfo.smem_len);
		return;
	}

	use_kerning = FT_HAS_KERNING(face);

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	desc.face_id = FONT;
	desc.flags = FT_LOAD_MONOCHROME;
#else
	desc.font.face_id = FONT;
	desc.type = ftc_image_mono;
#endif

	// init backbuffer
	if (!(lbb = malloc(var_screeninfo.xres*var_screeninfo.yres)))
	{
		printf("TuxCal <allocating of Backbuffer failed>\n");
		FTC_Manager_Done(manager);
		FT_Done_FreeType(library);
		resetBpp();
		munmap(lfb, fix_screeninfo.smem_len);
		return;
	}

	memset(lbb, 0, var_screeninfo.xres*var_screeninfo.yres);

	startx = sx + (((ex-sx) - MAXSCREEN_X)/2);
	starty = sy + (((ey-sy) - MAXSCREEN_Y)/2);

	// lock keyboard-conversions, this is done by the plugin itself
	fclose(fopen(KBLCKFILE,"w"));
		
	// get daemon status
	if (!ControlDaemon(GET_STATUS))
	{
		online = DAEMON_OFF;
	}

	// remove last key & set rc to blocking mode
	// dBox2 code
#if HAVE_DVB_API_VERSION == 3
	read(rc, &ev, sizeof(ev));
//	fcntl(rc, F_SETFL, fcntl(rc, F_GETFL) &~ O_NONBLOCK);
	fcntl(rc, F_SETFL, fcntl(rc, F_GETFL) | O_NONBLOCK);
#else
	// dreambox code
	read(rc, &rccode, sizeof(rccode));
 	fcntl(rc, F_SETFL, O_NONBLOCK);
#endif

	// read tuxcal.list file
	LoadDatabase();
	
//	ftime(&tb);
//	gettimeofday(&tv,NULL);
//	tb.time=tv.tv_sec;
//	at = localtime(&tb.time);
	time(&tt);																	// read the actual time	
	at = localtime(&tt);

	tShow_year = at->tm_year+1900;
	tShow_mon = at->tm_mon+1;
	tShow_day = at->tm_mday;

	int iSel=0;
	int iSelInfo=0;
	int iDayOfWeek;
	int iLastPreMonth;
	int iMonthDays;
	int iActDayPos=0;
	int year,mon;	
	int oldyear=0;
	int iChanged=0;	
	rccode = 0;
	
	
	// main loop
	do
	{
	
		// calculate the christian holidays
		if (oldyear!=tShow_year)
		{
			FillEasternDays(tShow_year);
			oldyear = tShow_year;
		}
		
		// calculate the weekday for the first of the selected month
		iDayOfWeek=DayOfWeek( 1, tShow_mon, tShow_year);	
		
		// calculate the last day of the previous month
		year=tShow_year;
		mon=tShow_mon;
		iLastPreMonth=1;
		AddDays( &iLastPreMonth, &mon, &year, -1);
		
		// calculate days in this month
		iMonthDays=monthdays[LeapYear(tShow_year)][tShow_mon-1];
		// calculate position of actual day
		if ((tShow_mon == at->tm_mon+1) && (tShow_year == at->tm_year+1900))
			iActDayPos=iDayOfWeek+at->tm_mday-1;
		else
			iActDayPos=0;
		
		PaintGrid(iLastPreMonth, iDayOfWeek, iDayOfWeek+iMonthDays-1, iActDayPos, iDayOfWeek+tShow_day-1, iSel, &iSelInfo);

		switch ((rccode = GetRCCode()))
		{
			case RC_DBOX:
			{
				if (!ControlDaemon(TOGGLE_CLOCK))														// send hide/show clock to daemon
					ShowMessage(CLOCKFAIL);																		// we didn't reach the daemon, show error
				else ShowMessage(CLOCKOK);
			} break;
			
			case RC_0:	
			{
				tShow_year = at->tm_year+1900;
				tShow_mon = at->tm_mon+1;
				tShow_day = at->tm_mday;
			} break;				
			
			case RC_1:	
			case RC_2:  
			case RC_3:
			case RC_4:	
			case RC_5:  
			case RC_6:  
			case RC_7:	
			case RC_8:  
			case RC_9:  
			{
		  }	break;

			case RC_MINUS:
			{
				AddDays( &tShow_day, &tShow_mon, &tShow_year, -28);		
		  }	break;

			case RC_PLUS:
			{
				AddDays( &tShow_day, &tShow_mon, &tShow_year, 28);		
		  }	break;

			case RC_UP:
			{
				if (iSelInfo==0) AddDays( &tShow_day, &tShow_mon, &tShow_year, -7);		
				if (iSelInfo>1) iSelInfo--;
		  }	break;

			case RC_DOWN:
			{
				if (iSelInfo==0) AddDays( &tShow_day, &tShow_mon, &tShow_year, +7);		
				if (iSelInfo) iSelInfo++;
		  }	break;

			case RC_LEFT:
			{
				AddDays( &tShow_day, &tShow_mon, &tShow_year, -1);		
				iSelInfo=0;
		  }	break;

			case RC_RIGHT:
			{
				AddDays( &tShow_day, &tShow_mon, &tShow_year, 1);		
				iSelInfo=0;
		  }	break;

			case RC_OK:
			{
				if (iSelInfo)
				{
					int iEvt;
					iEvt=IsEvent(tShow_day,tShow_mon,tShow_year);
					if (iEvt)
					{
						if (iEventType[iSelInfo-1]<MAXENTRYS)
						{
							EVT_DB evt;
							memcpy(&evt,&eventdb[iEventType[iSelInfo-1]],sizeof(evt));
							if (Edit(&evt))
							{
								memcpy(&eventdb[iEventType[iSelInfo-1]],&evt,sizeof(evt));
								SaveDatabase();
								iChanged=1;
							}
						}
					}
				}
				else
				{
					if (iSel) iSel=0;
					else iSel=1;
				}
		  }	break;

			// delete currentla selected entry
			case RC_RED:
			{
				// do we have something selected which can be deleted
				if ((iSel) && (iSelInfo))
				{
					int iEvt;
					iEvt=IsEvent(tShow_day,tShow_mon,tShow_year);
					if (iEvt)
					{
						if ((iEventType[iSelInfo-1]<MAXENTRYS) && (MessageBox(infomsg1[0][osdidx],infomsg1[1][osdidx])))
						{
							eventdb[iEventType[iSelInfo-1]].type=UNUSED;
							SaveDatabase();
							iChanged=1;
						}
					}
				}
		  }	break;

			// change cursor to info-lines
			case RC_GREEN:
			{
				if (iSel) 
				{
					if (iSelInfo==0) iSelInfo=1;
					else iSelInfo=0;
				}
		  }	break;

			// add new entry at current day
			case RC_YELLOW:
			{
				if ((iSel) && (iCntEntries<MAXENTRYS))
				{
						if (MessageBox(infomsg1[0][osdidx],infomsg1[2][osdidx]))
						{
							eventdb[iCntEntries].type=EVENT;
							eventdb[iCntEntries].day=tShow_day;
							eventdb[iCntEntries].month=tShow_mon;
							eventdb[iCntEntries].year=tShow_year;
							eventdb[iCntEntries].hour=-1;
							strcpy(eventdb[iCntEntries].info,infomsg1[0][osdidx]);
							SaveDatabase();
							iChanged=1;
							iSelInfo=255;		
						}
				}
		  }	break;

			case RC_BLUE:
			{
				// do we have something selected which can be edited
				if ((iSel) && (iSelInfo))
				{
					int iEvt;
					iEvt=IsEvent(tShow_day,tShow_mon,tShow_year);
					if (iEvt)
					{
						if (iEventType[iSelInfo-1]<MAXENTRYS)
						{
							EVT_DB evt;
							memcpy(&evt,&eventdb[iEventType[iSelInfo-1]],sizeof(evt));
							if (Edit(&evt))
							{
								memcpy(&eventdb[iEventType[iSelInfo-1]],&evt,sizeof(evt));
								SaveDatabase();
								iChanged=1;
							}
						}
					}
				}
		  }	break;

			// switch signaling of an event on or off
			case RC_MUTE:
			{			
				if (!ControlDaemon(GET_STATUS))															// request state of daemon
				{
					online = DAEMON_OFF;																			// we didn't reach the daemon
					ShowMessage(NODAEMON);
				}
				else
				{
					online++;																									// toogle actual daemon-state
					online &= 1;

					if (!ControlDaemon(SET_STATUS))														// send new state to daemon
					{
						if (online) ShowMessage(STARTFAIL);											// we didn't reach the daemon, show error
						else ShowMessage(STOPFAIL);

						online++;																								// toogle state back
						online &= 1;
					}
					else
					{
						if(online) ShowMessage(STARTDONE);											// show message: signaling is on now
						else ShowMessage(STOPDONE);															//               signaling is off now
					}
				}
		  }	break;

			case RC_HELP:
			{
				ShowMessage(INFO);
		  }	break;

			// switch autostart of daemon on or off
			case RC_STANDBY:
			{
				if((fd_run = fopen(RUNFILE, "r")))													//! autostart is on
				{
					fclose(fd_run);
					unlink(RUNFILE);																					//! delete autostart-file
					ShowMessage(BOOTOFF);																			//! show that autostart is disabled
				}
				else
				{
					fclose(fopen(RUNFILE, "w"));															//! generate the autostart-file
					ShowMessage(BOOTON);																			//! show message that autostart is enabled
				}
			} break;
			
			default:
			;
		}
	}
	while (rccode != RC_HOME);

	// signal daemon to reread the database
	ControlDaemon(RELOAD_DB);														

	// enable keyboard-conversion again
	unlink(KBLCKFILE);
	
	// cleanup
	FTC_Manager_Done(manager);
	FT_Done_FreeType(library);
	free(lbb);

	resetBpp();
	munmap(lfb, fix_screeninfo.smem_len);
	fcntl(rc, F_SETFL, O_NONBLOCK);
	close(kb);

	return;
}
