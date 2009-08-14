/******************************************************************************
 *                        <<< TuxMail - Mail Plugin >>>
 *                (c) Thomas "LazyT" Loewe 2003 (LazyT@gmx.net)
 *-----------------------------------------------------------------------------
 * $Log: tuxmail.c,v $
 * Revision 1.48  2008/08/28 20:29:51  robspr1
 * increase maximum startup delay to 120
 *
 * Revision 1.47  2007/05/17 16:19:47  dbluelle
 * Make plugins compile with freeetype 2.1.x on dreambox (as needed for Neutrino on Dreambox)
 *
 * Revision 1.46  2007/01/01 21:22:20  robspr1
 * -delete tuxmail.new before leaving plugin, not when entering
 *
 * Revision 1.45  2006/10/27 19:36:32  robspr1
 * - bugfix viewing mails in emtpy mailbox
 *
 * Revision 1.44  2006/09/27 18:59:00  robspr1
 * -faster switching between viewed emails using + and -
 *
 * Revision 1.43  2006/03/05 16:02:13  robspr1
 * - use /tmp/keyboard.lck to signal decoding of the keyboard
 *
 * Revision 1.42  2005/12/12 18:59:51  robspr1
 * -bugfix USER/SUSER and PASS/SPASS extraction
 *
 * Revision 1.41  2005/11/19 14:37:38  robspr1
 * - add different behaviour in marking mails green in the plugin
 *
 * Revision 1.40  2005/11/11 18:40:15  robspr1
 * - /tmp/tuxmail.new holds number of new files /  reread tuxmail.conf after writing
 *
 * Revision 1.39  2005/11/04 15:59:32  robspr1
 * - adding IMAP support
 *
 * Revision 1.38  2005/08/19 19:00:54  robspr1
 * - add pin protection for config GUI
 *
 * Revision 1.37  2005/08/19 09:00:04  robspr1
 * - add 3rd skin, bugfix config GUI
 *
 * Revision 1.36  2005/08/18 23:20:54  robspr1
 * - add config GUI (DBOX key)
 *
 * Revision 1.35  2005/07/03 18:39:17  robspr1
 * bugbix PIN code after database reload
 *
 * Revision 1.34  2005/06/27 19:49:54  robspr1
 * - reload database after read/write
 *
 * Revision 1.33  2005/06/19 17:21:36  robspr1
 * - dreambox tastatur now working
 *
 * Revision 1.32  2005/06/08 21:56:49  robspr1
 * - minor fixes for mail writing; - using dreambox keyboard?
 *
 * Revision 1.31  2005/06/07 19:17:51  robspr1
 * -change ENTER for dBox Keyboard; -simple zoom for mail viewer
 *
 * Revision 1.30  2005/06/06 21:27:48  robspr1
 * using dBox Keyboard
 *
 * Revision 1.29  2005/05/24 16:37:22  lazyt
 * - fix WebIF Auth
 * - add SMTP Auth
 *
 * Revision 1.28  2005/05/20 18:01:24  lazyt
 * - Preparation for Keyboard
 * - don't try add to Spamlist for empty Account
 *
 * Revision 1.27  2005/05/19 21:56:19  robspr1
 * - bugfix cached mailreading
 *
 * Revision 1.26  2005/05/19 10:04:31  robspr1
 * - add cached mailreading
 *
 * Revision 1.25  2005/05/17 20:40:34  robspr1
 * - add addressbook to mailwriter
 *
 * Revision 1.24  2005/05/14 18:54:28  robspr1
 * - Bugfix Mailreader - Mailwriter SMS style
 *
 * Revision 1.23  2005/05/14 08:59:51  lazyt
 * - fix Spamfunction
 * - new Keydefinitions: RED=delete Mail, GREEN=send Mail, YELLOW=read Mail, ?=About (DBOX reserved for Configmenu)
 *
 * Revision 1.22  2005/05/13 23:17:14  robspr1
 * - first Mail writing GUI\n- add parameters for Mail sending
 *
 * Revision 1.21  2005/05/12 22:24:23  lazyt
 * - PIN-Fix
 * - add Messageboxes for send Mail done/fail
 *
 * Revision 1.20  2005/05/12 14:28:28  lazyt
 * - PIN-Protection for complete Account
 * - Preparation for sending Mails ;-)
 *
 * Revision 1.19  2005/05/11 19:00:21  robspr1
 * minor Mailreader changes / add to Spamlist undo
 *
 * Revision 1.18  2005/05/11 12:01:21  lazyt
 * Protect Mailreader with optional PIN-Code
 *
 * Revision 1.17  2005/05/10 12:55:15  lazyt
 * - LCD-Fix for DM500
 * - Autostart for DM7020 (use -DOE, put Init-Script to /etc/init.d/tuxmail)
 * - try again after 10s if first DNS-Lookup failed
 * - don't try to read Mails on empty Accounts
 *
 * Revision 1.16  2005/05/09 19:30:20  robspr1
 * support for mail reading
 *
 * Revision 1.15  2005/04/29 17:24:00  lazyt
 * use 8bit audiodata, fix skin and osd
 *
 * Revision 1.14  2005/03/28 14:14:14  lazyt
 * support for userdefined audio notify (put your 12/24/48KHz pcm wavefile to /var/tuxbox/config/tuxmail/tuxmail.wav)
 *
 * Revision 1.13  2005/03/24 13:15:20  lazyt
 * cosmetics, add SKIN=0/1 for different osd-colors
 *
 * Revision 1.12  2005/03/22 13:31:46  lazyt
 * support for english osd (OSD=G/E)
 *
 * Revision 1.11  2005/03/22 09:35:20  lazyt
 * lcd support for daemon (LCD=Y/N, GUI should support /tmp/lcd.locked)
 *
 * Revision 1.10  2005/02/26 10:23:48  lazyt
 * workaround for corrupt mail-db
 * add ADMIN=Y/N to conf (N to disable mail deletion via plugin)
 * show versioninfo via "?" button
 * limit display to last 100 mails (increase MAXMAIL if you need more)
 *
 * Revision 1.9  2004/07/10 11:38:14  lazyt
 * use -DOLDFT for older FreeType versions
 * replaced all remove() with unlink()
 *
 * Revision 1.8  2004/04/24 22:24:23  carjay
 * fix compiler warnings
 *
 * Revision 1.7  2004/04/14 17:46:00  metallica
 *  fix2: seg fault (better)
 *
 * Revision 1.6  2004/04/14 17:05:10  metallica
 *  fix: seg fault
 *
 * Revision 1.5  2003/06/24 07:19:35  alexw
 * bugfix & cleanup
 *
 * Revision 1.4  2003/06/23 15:18:49  alexw
 * hmpf, oldapi
 *
 * Revision 1.3  2003/06/23 15:16:45  alexw
 * rc fixed
 *
 * Revision 1.2  2003/05/16 15:07:23  lazyt
 * skip unused accounts via "plus/minus", add mailaddress to spamlist via "blue"
 *
 * Revision 1.1  2003/04/21 09:24:52  lazyt
 * add tuxmail, todo: sync (filelocking?) between daemon and plugin
 ******************************************************************************/

#include "tuxmail.h"

/******************************************************************************
 * ReadConf
 ******************************************************************************/

void ReadConf()
{
	FILE *fd_conf;
	char *ptr;
	char line_buffer[256];

	// open config

		if(!(fd_conf = fopen(CFGPATH CFGFILE, "r")))
		{
			printf("TuxMail <Config not found, using defaults>\n");

			return;
		}

	// read config

		while(fgets(line_buffer, sizeof(line_buffer), fd_conf))
		{
			if((ptr = strstr(line_buffer, "STARTDELAY=")))
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
			else if((ptr = strstr(line_buffer, "LOGMODE=")))
			{
				sscanf(ptr + 8, "%c", &logmode);
			}
			else if((ptr = strstr(line_buffer, "SAVEDB=")))
			{
				sscanf(ptr + 7, "%c", &savedb);
			}
			else if((ptr = strstr(line_buffer, "AUDIO=")))
			{
				sscanf(ptr + 6, "%c", &audio);
			}
			else if((ptr = strstr(line_buffer, "VIDEO=")))
			{
				sscanf(ptr + 6, "%d", &video);
			}
			else if((ptr = strstr(line_buffer, "LCD=")))
			{
				sscanf(ptr + 4, "%c", &lcdc);
			}
			else if((ptr = strstr(line_buffer, "CONFIGCODE=")))
			{
				sscanf(ptr + 11, "%s", &configcode[0]);
			}
			else if((ptr = strstr(line_buffer, "ADMIN=")))
			{
				sscanf(ptr + 6, "%c", &admin);
			}
			else if((ptr = strstr(line_buffer, "MAILCACHE=")))
			{
				sscanf(ptr + 10, "%d", &mailcache);
			}
			else if((ptr = strstr(line_buffer, "MAILDIR=")))
			{
				sscanf(ptr + 8, "%s", &maildir[0]);
			}
			else if((ptr = strstr(line_buffer, "OSD=")))
			{
				sscanf(ptr + 4, "%c", &osd);
			}
			else if((ptr = strstr(line_buffer, "SKIN=")))
			{
				sscanf(ptr + 5, "%d", &skin);
			}
			else if((ptr = strstr(line_buffer, "TYPEFLAG=")))
			{
				sscanf(ptr + 9, "%d", &typeflag);
			}
			else if((ptr = strstr(line_buffer, "SECURITY=")))
			{
				sscanf(ptr + 9, "%s", &security[0]);
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
			else if((ptr = strstr(line_buffer, "NAME")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", maildb[index-'0'].namebox);
				}
			}
			else if((ptr = strstr(line_buffer, "SMTP")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", maildb[index-'0'].smtp);
				}
			}
			else if((ptr = strstr(line_buffer, "FROM")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
//					strncpy(maildb[index-'0'].from,ptr+6,63);
					sscanf(ptr + 6, "%s", maildb[index-'0'].from);
				}
			}
			else if((ptr = strstr(line_buffer, "CODE")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%4s", maildb[index-'0'].code);
				}
			}
			else if((ptr = strstr(line_buffer, "POP3")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", maildb[index-'0'].pop3);
				}
			}
			else if((ptr = strstr(line_buffer, "IMAP")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", maildb[index-'0'].imap);
				}
			}
			else if((ptr = strstr(line_buffer, "SUSER")) && (*(ptr+6) == '='))
			{
				char index = *(ptr+5);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 7, "%s", maildb[index-'0'].suser);
				}
			}
			else if((ptr = strstr(line_buffer, "SPASS")) && (*(ptr+6) == '='))
			{
				char index = *(ptr+5);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 7, "%s", maildb[index-'0'].spass);
				}
			}
			else if((ptr = strstr(line_buffer, "USER")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", maildb[index-'0'].user);
				}
			}
			else if((ptr = strstr(line_buffer, "PASS")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", maildb[index-'0'].pass);
				}
			}
			else if((ptr = strstr(line_buffer, "AUTH")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%d", &maildb[index-'0'].auth);
				}
			}
			else if((ptr = strstr(line_buffer, "INBOX")) && (*(ptr+6) == '='))
			{
				char index = *(ptr+5);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 7, "%s", maildb[index-'0'].inbox);
				}
			}
		}

		fclose(fd_conf);

	// check config

		if(admin != 'Y' && admin != 'N')
		{
			printf("TuxMail <ADMIN=%c invalid, set to \"Y\">\n", admin);

			admin = 'Y';
		}

		if(osd != 'G' && osd != 'E')
		{
			printf("TuxMail <OSD=%c invalid, set to \"G\">\n", osd);

			osd = 'G';
		}

		if(skin != 1 && skin != 2 && skin != 3)
		{
			printf("TuxMail <SKIN=%d invalid, set to \"1\">\n", skin);

			skin = 1;
		}
		
		if(typeflag < 1 || typeflag > 4)
		{
			printf("TuxMail <TYPEFLAG=%d invalid, set to \"1\">\n", typeflag);

			typeflag = 1;
		}
		
}

/******************************************************************************
 * WriteConf
 ******************************************************************************/

int WriteConf()
{
	FILE *fd_conf;
	int loop;

	// open config

	if(!(fd_conf = fopen(CFGPATH CFGFILE , "w")))
	{
		return 0;
	}

	fprintf(fd_conf, "STARTDELAY=%d\n", startdelay);
	fprintf(fd_conf, "INTERVALL=%d\n\n", intervall);
	fprintf(fd_conf, "LOGGING=%c\n", logging);
	fprintf(fd_conf, "LOGMODE=%c\n\n", logmode);
	fprintf(fd_conf, "SAVEDB=%c\n\n", savedb);
	fprintf(fd_conf, "AUDIO=%c\n", audio);
	fprintf(fd_conf, "VIDEO=%d\n\n", video);
	fprintf(fd_conf, "LCD=%c\n", lcdc);
	fprintf(fd_conf, "OSD=%c\n\n", osd);
	fprintf(fd_conf, "SKIN=%d\n\n", skin);
	fprintf(fd_conf, "TYPEFLAG=%d\n\n", typeflag);
	fprintf(fd_conf, "CONFIGCODE=%s\n", configcode);
	fprintf(fd_conf, "ADMIN=%c\n\n", admin);
	fprintf(fd_conf, "MAILCACHE=%d\n", mailcache);
	fprintf(fd_conf, "MAILDIR=%s\n", maildir);
	fprintf(fd_conf, "SECURITY=%s\n\n", security);
	fprintf(fd_conf, "WEBPORT=%d\n", webport);
	fprintf(fd_conf, "WEBUSER=%s\n", webuser);
	fprintf(fd_conf, "WEBPASS=%s\n", webpass);

	for(loop = 0; loop < 10; loop++)
	{
		fprintf(fd_conf, "\nNAME%d=%s\n", loop, maildb[loop].namebox);
		fprintf(fd_conf, "POP3%d=%s\n", loop, maildb[loop].pop3);
		fprintf(fd_conf, "IMAP%d=%s\n", loop, maildb[loop].imap);
		fprintf(fd_conf, "USER%d=%s\n", loop, maildb[loop].user);
		fprintf(fd_conf, "PASS%d=%s\n", loop, maildb[loop].pass);
		fprintf(fd_conf, "SMTP%d=%s\n", loop, maildb[loop].smtp);
		fprintf(fd_conf, "FROM%d=%s\n", loop, maildb[loop].from);
		fprintf(fd_conf, "CODE%d=%s\n", loop, maildb[loop].code);
		fprintf(fd_conf, "AUTH%d=%d\n", loop, maildb[loop].auth);
		fprintf(fd_conf, "SUSER%d=%s\n", loop, maildb[loop].suser);
		fprintf(fd_conf, "SPASS%d=%s\n", loop, maildb[loop].spass);
		fprintf(fd_conf, "INBOX%d=%s\n", loop, maildb[loop].inbox);
		if(!maildb[loop + 1].user[0])
		{
			break;
		}
	}

	fclose(fd_conf);
	return 1;
}

/******************************************************************************
 * ControlDaemon (0=fail, 1=done)
 ******************************************************************************/

int ControlDaemon(int command, int account, int mailindex)
{
	int fd_sock;
	struct sockaddr_un srvaddr;
	socklen_t addrlen;
	char sendcmd[88];
	char mailsend;

	// setup connection

		srvaddr.sun_family = AF_UNIX;
		strcpy(srvaddr.sun_path, SCKFILE);
		addrlen = sizeof(srvaddr.sun_family) + strlen(srvaddr.sun_path);

		if((fd_sock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
		{
			printf("TuxMail <Socketerror: socket failed>\n");

			return 0;
		}

		if(connect(fd_sock, (struct sockaddr*)&srvaddr, addrlen) == -1)
		{
			printf("TuxMail <Socketerror: connect failed>\n");

			close(fd_sock);

			return 0;
		}

	// send command

		switch(command)
		{
			case GET_STATUS:

				send(fd_sock, "G", 1, 0);
				recv(fd_sock, &online, 1, 0);

				break;

			case SET_STATUS:

				send(fd_sock, "S", 1, 0);
				send(fd_sock, &online, 1, 0);

				break;

			case RELOAD_SPAMLIST:

				send(fd_sock, "L", 1, 0);

				break;

			case RELOAD_CONFIG:

				send(fd_sock, "R", 1, 0);

				break;

			case GET_VERSION:

				send(fd_sock, "V", 1, 0);
				recv(fd_sock, &versioninfo_d, sizeof(versioninfo_d), 0);

				break;

			case GET_MAIL:

				ShowMessage(GETMAIL);

				send(fd_sock, "M", 1, 0);
				sprintf(sendcmd, "%d-%02d-%s", account, mailindex, maildb[account].mailinfo[mailindex].uid);
				send(fd_sock, sendcmd, 5 + sizeof(maildb[account].mailinfo[mailindex].uid), 0);
				recv(fd_sock, &mailfile, 1, 0);

				break;

			case SEND_MAIL:

				sprintf(sendcmd, "W%d", account);
				send(fd_sock, sendcmd, 2, 0);
				recv(fd_sock, &mailsend, 1, 0);

				mailsend ? ShowMessage(SENDMAILDONE) : ShowMessage(SENDMAILFAIL);
		}

		close(fd_sock);

	return 1;
}

/******************************************************************************
 * GetRCCode
 ******************************************************************************/

#if HAVE_DVB_API_VERSION == 3

int GetRCCode()
{
	static __u16 rc_last_key = KEY_RESERVED;
	static __u16 rc_last_code = KEY_RESERVED;

	if(sim_key)
	{
		sim_key = 0;

		return rccode;
	}

	if(read(rc, &ev, sizeof(ev)) == sizeof(ev))
	{
		if(ev.value)
		{
			if(ev.code != rc_last_key)
			{
				rc_last_key = ev.code;

				switch(ev.code)
				{
					case KEY_OK:

						rccode = RC_OK;

						break;

					case KEY_RED:

						rccode = RC_RED;

						break;

					case KEY_GREEN:

						rccode = RC_GREEN;

						break;

					case KEY_YELLOW:

						rccode = RC_YELLOW;

						break;

					case KEY_BLUE:

						rccode = RC_BLUE;

						break;

					case KEY_HELP:

						rccode = RC_HELP;

						break;

					case KEY_SETUP:

						rccode = RC_DBOX;

						break;

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
						if( !rccode )
						{
							rccode = -1;
						}
				}
				rc_last_code = rccode;
			}
			else
			{
				rccode = -1;
			}
		}
		else
		{
			rccode = -1;
			rc_last_key = KEY_RESERVED;
			rc_last_code = KEY_RESERVED;
		}
	}

	return rccode;
}

#else

int GetRCCode()
{
	static unsigned short LastKey = -1;
	static char LastKBCode = 0x00;

	if(sim_key)
	{
		sim_key = 0;

		return rccode;
	}
	
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
	}
	while( rccode == 0xFFFF);
	return rccode;
}
#endif

/******************************************************************************
 * MyFaceRequester
 ******************************************************************************/

FT_Error MyFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	FT_Error result;

	result = FT_New_Face(library, face_id, 0, aface);

	if(!result)
	{
		printf("TuxMail <Font \"%s\" loaded>\n", (char*)face_id);
	}
	else
	{
		printf("TuxMail <Font \"%s\" failed>\n", (char*)face_id);
	}

	return result;
}

/******************************************************************************
 * RenderLCDDigit
 ******************************************************************************/

void RenderLCDDigit(int digit, int sx, int sy)
{
	int x, y;

	for(y = 0; y < 15; y++)
	{
		for(x = 0; x < 10; x++)
		{
			if(lcd_digits[digit*15*10 + x + y*10])
			{
				lcd_buffer[sx + x + ((sy + y)/8)*120] |= 1 << ((sy + y)%8);
			}
			else
			{
				lcd_buffer[sx + x + ((sy + y)/8)*120] &= ~(1 << ((sy + y)%8));
			}
		}
	}
}

/******************************************************************************
 * UpdateLCD
 ******************************************************************************/

void UpdateLCD(int account)
{
	int x, y;

	// set online status

		for(y = 0; y < 19; y++)
		{
			for(x = 0; x < 17; x++)
			{
				if(lcd_status[online*17*19 + x + y*17])
				{
					lcd_buffer[4 + x + ((18 + y)/8)*120] |= 1 << ((18 + y)%8);
				}
				else
				{
					lcd_buffer[4 + x + ((18 + y)/8)*120] &= ~(1 << ((18 + y)%8));
				}
			}
		}

	// set digits

		RenderLCDDigit(maildb[account].nr[0] - '0', 41, 20);

		RenderLCDDigit(maildb[account].time[0] - '0', 58, 20);
		RenderLCDDigit(maildb[account].time[1] - '0', 71, 20);
		RenderLCDDigit(maildb[account].time[3] - '0', 93, 20);
		RenderLCDDigit(maildb[account].time[4] - '0', 106, 20);

		RenderLCDDigit(maildb[account].status[0] - '0', 28, 44);
		RenderLCDDigit(maildb[account].status[1] - '0', 41, 44);
		RenderLCDDigit(maildb[account].status[2] - '0', 54, 44);
		RenderLCDDigit(maildb[account].status[4] - '0', 80, 44);
		RenderLCDDigit(maildb[account].status[5] - '0', 93, 44);
		RenderLCDDigit(maildb[account].status[6] - '0', 106, 44);

	// copy to lcd

		write(lcd, &lcd_buffer, sizeof(lcd_buffer));
}

/******************************************************************************
 * RenderChar
 ******************************************************************************/

int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, int color)
{
	int row, pitch, bit, x = 0, y = 0;
	FT_Error error;
	FT_UInt glyphindex;
	FT_Vector kerning;
	FTC_Node anode;

	//load char

		if(!(glyphindex = FT_Get_Char_Index(face, currentchar)))
		{
			printf("TuxMail <FT_Get_Char_Index for Char \"0x%x\" failed: \"undefined character code\">\n", (int)currentchar);

			return 0;
		}

		if((error = FTC_SBitCache_Lookup(cache, &desc, glyphindex, &sbit, &anode)))
		{
			printf("TuxMail <FTC_SBitCache_Lookup for Char \"%c\" failed with Errorcode 0x%.2X>\n", (int)currentchar, error);

			return 0;
		}

		if(use_kerning)
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

		if(color != -1) /* don't render char, return charwidth only */
		{
			if(sx + sbit->xadvance >= ex)
			{
				return -1; /* limit to maxwidth */
			}

			for(row = 0; row < sbit->height; row++)
			{
				for(pitch = 0; pitch < sbit->pitch; pitch++)
				{
					for(bit = 7; bit >= 0; bit--)
					{
						if(pitch*8 + 7-bit >= sbit->width)
						{
							break; /* render needed bits only */
						}

						if((sbit->buffer[row * sbit->pitch + pitch]) & 1<<bit)
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

int GetStringLen(unsigned char *string)
{
	int stringlen = 0;

	// reset kerning

		prev_glyphindex = 0;

	// calc len

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

void RenderString(unsigned char *string, int sx, int sy, int maxwidth, int layout, int size, int color)
{
	int stringlen, ex, charwidth;

	// set size

		if(size == SMALL)
		{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
			desc.width = desc.height = 24;
#else
			desc.font.pix_width = desc.font.pix_height = 24;
#endif
		}
		else if(size == NORMAL)
		{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
			desc.width = desc.height = 32;
#else
			desc.font.pix_width = desc.font.pix_height = 32;
#endif
		}
		else
		{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
			desc.width = desc.height = 40;
#else
			desc.font.pix_width = desc.font.pix_height = 40;
#endif
		}

	// set alignment

		if(layout != LEFT)
		{
			stringlen = GetStringLen(string);

			switch(layout)
			{
				case CENTER:
					if(stringlen < maxwidth)
					{
						sx += (maxwidth - stringlen)/2;
					}

					break;

				case RIGHT:

					if(stringlen < maxwidth)
					{
						sx += maxwidth - stringlen;
					}
			}
		}

	// reset kerning

		prev_glyphindex = 0;

	// render string

		ex = sx + maxwidth;

		while(*string != '\0')
		{
			if((charwidth = RenderChar(*string, sx, sy, ex, color)) == -1)
			{
				return; /* string > maxwidth */
			}

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

	if(mode == FILL)
	{
		for(; sy <= ey; sy++)
		{
			memset(lbb + startx + sx + var_screeninfo.xres*(starty + sy), color, ex-sx + 1);
		}
	}
	else
	{
		// hor lines

			for(loop = sx; loop <= ex; loop++)
			{
				*(lbb + startx+loop + var_screeninfo.xres*(sy+starty)) = color;
				*(lbb + startx+loop + var_screeninfo.xres*(sy+1+starty)) = color;

				*(lbb + startx+loop + var_screeninfo.xres*(ey-1+starty)) = color;
				*(lbb + startx+loop + var_screeninfo.xres*(ey+starty)) = color;
			}

		// ver lines

			for(loop = sy; loop <= ey; loop++)
			{
				*(lbb + startx+sx + var_screeninfo.xres*(loop+starty)) = color;
				*(lbb + startx+sx+1 + var_screeninfo.xres*(loop+starty)) = color;

				*(lbb + startx+ex-1 + var_screeninfo.xres*(loop+starty)) = color;
				*(lbb + startx+ex + var_screeninfo.xres*(loop+starty)) = color;
			}
	}
}

/******************************************************************************
 * RenderCircle
 ******************************************************************************/

void RenderCircle(int sx, int sy, char type)
{
	int x, y, color;

	// set color

		switch(type)
		{
			case 'n':
			case 'N':
			case 'o':

				color = GREEN;

				break;

			case 'O':

				color = YELLOW;

				break;

			case 'D':

				color = RED;

				break;

			default:
				
				return;
		}

	// render

		for(y = 0; y < 15; y++)
		{
			for(x = 0; x < 15; x++)
			{
				if(circle[x + y*15])
				{
					memset(lbb + startx + sx + x + var_screeninfo.xres*(starty + sy + y), color, 1);
				}
			}
		}
}


/******************************************************************************
 * MessageBox
 ******************************************************************************/

void MessageBox(char* header, char* question)
{
	RenderBox(155, 178, 464, 220, FILL, SKIN0);
	RenderBox(155, 220, 464, 327, FILL, SKIN1);
	RenderBox(155, 178, 464, 327, GRID, SKIN2);
	RenderBox(155, 220, 464, 327, GRID, SKIN2);
  
	RenderString(header, 157, 213, 306, CENTER, BIG, ORANGE);
	RenderString(question, 157, 265, 306, CENTER, BIG, WHITE);
  
  RenderBox(235, 286, 284, 310, FILL, SKIN2);
  RenderString("OK", 237, 305, 46, CENTER, SMALL, WHITE);
	RenderBox(335, 286, 384, 310, FILL, SKIN2);
	RenderString("EXIT", 337, 305, 46, CENTER, SMALL, WHITE);
  
	memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);

	while( GetRCCode() )
	{
		if(( rccode == RC_OK ) || ( rccode == RC_HOME ) || ( rccode == RC_RET ))
		{
			break;
		}
	}
}

/******************************************************************************
 * PaintMailHeader
 ******************************************************************************/

void PaintMailHeader( void )
{
	RenderBox(0, 0, VIEWX, 3*FONTHEIGHT_SMALL+2*BORDERSIZE+2, FILL, SKIN0);
	RenderBox(0, 3*FONTHEIGHT_SMALL+2*BORDERSIZE, VIEWX,VIEWY, FILL, SKIN1);
	RenderBox(0, 0, VIEWX, 3*FONTHEIGHT_SMALL+2*BORDERSIZE+2, GRID, SKIN2);
	RenderBox(0, 3*FONTHEIGHT_SMALL+2*BORDERSIZE, VIEWX, VIEWY, GRID, SKIN2);
}

/******************************************************************************
 * ShowMailHeader
 ******************************************************************************/

void ShowMailHeader(char* szAction)
{
	char *p;
	
	
	p=strchr(szAction, '\n');
	if(p)
	{
		*p = 0;
		RenderString((osd == 'G') ? "Absender:" : "From:", 2*BORDERSIZE, BORDERSIZE+FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
		RenderString(szAction, 2*BORDERSIZE+100, BORDERSIZE+FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE-100, LEFT, SMALL, WHITE);
		szAction = ++p;
	}
	if(p) 
	{
		p = strchr(szAction, '\n');
	}
	if(p)
	{
		*p = 0;
		RenderString((osd == 'G') ? "Betreff:" : "Subject:", 2*BORDERSIZE, BORDERSIZE+2*FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
		RenderString(szAction, 2*BORDERSIZE+100, BORDERSIZE+2*FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE-100, LEFT, SMALL, WHITE);
		szAction = ++p;
	}
	if(p)
	{
		p = strchr(szAction,'\n');
	}
	if(p)
	{
		*p = 0;
		RenderString((osd == 'G') ? "Datum:" : "Date:", 2*BORDERSIZE, BORDERSIZE+3*FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
		RenderString(szAction, 2*BORDERSIZE+100, BORDERSIZE+3*FONTHEIGHT_SMALL-2 , VIEWX-4*BORDERSIZE-100, LEFT, SMALL, WHITE);
		szAction = ++p;
	}
	if(p)
	{
		p = strchr(szAction,'\n');
	}
	if(p)
	{
		*p = 0;
		RenderString((osd == 'G') ? "Seite:" : "Page:", 2*BORDERSIZE+450, BORDERSIZE+3*FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
		RenderString(szAction, 2*BORDERSIZE+500, BORDERSIZE+3*FONTHEIGHT_SMALL-2 , VIEWX-4*BORDERSIZE-500, LEFT, SMALL, WHITE);
		szAction = ++p;
	}
	
}

/******************************************************************************
 * ShowMailFile
 ******************************************************************************/

void ShowMailFile(char* filename, char* szAction)
{
	// Code from splugin (with some modifications...)
	char *p;
	char line[256];
	int iPage;
	int iMaxPages = 0;
	long iPagePos[100];
	char szHeader[256];
	int fSize = SMALL;
	int fLines = 15;
	int fChars = 83;
	int fHeight = FONTHEIGHT_SMALL;
	int fAssign = LEFT;
  int iRetfgets;
	FILE* pipe;
	pipe = fopen(filename,"r");

	if  (pipe == NULL)
	{
		return;
	}


	// Render output window
	PaintMailHeader();
	int row = 0;
	
	// calculate number of pages
	while( fgets( line, fChars, pipe ))
	{
		if ( ++row > (fLines) )
		{
			row = 0;
			if( !feof(pipe) )
			{
				iMaxPages ++;
			}
		}
	}
	iMaxPages ++;
	
	row = 0;
	iPage = 0;
	// position of 1. byte of all the pages
	fseek(pipe, 0, SEEK_SET);
	iPagePos[0] = ftell(pipe);
	while(1)
	{
		iRetfgets=(int)fgets( line, fChars, pipe );
		if( iRetfgets )
		{
			p = strchr(line,'\n');
			if( p )
			{
				*p = 0;
			}
			row++;
			
			RenderString(line, 2*BORDERSIZE+2, 2*BORDERSIZE+3*FONTHEIGHT_SMALL+2+row*fHeight -FONT_OFFSET, VIEWX-4*BORDERSIZE, fAssign, fSize, WHITE);
		}
		
		if((row > fLines) || (!iRetfgets))
		{
			// Render output window
			sprintf(szHeader,"%s%u / %u\n", szAction, iPage+1, iMaxPages);
			ShowMailHeader(szHeader);
			memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);

			while(1)
			{
				GetRCCode();
				if((rccode == RC_HOME) || (rccode == RC_PLUS) || (rccode == RC_MINUS)) 
				{
					break;
				}
				else if( rccode == RC_OK || rccode == RC_DOWN )
				{
					if((iPage < 99) && ((iPage +1) < iMaxPages) && (!feof(pipe)))
					{
						iPage++;
					}
					else
					{
//						fseek(pipe, 0, SEEK_SET);
					}
					break;
				}
				else if( rccode == RC_UP )
				{
					if(iPage) 
					{
						iPage--;
						fseek(pipe, iPagePos[iPage], SEEK_SET);
					}
					else
					{
						fseek(pipe, 0, SEEK_SET);
					}
					break;
				}
				else if( rccode == RC_YELLOW )
				{
					fseek(pipe, 0, SEEK_SET);
					iPage=0;
					switch( fSize )
					{
						case SMALL:
							fSize = NORMAL;
							fLines = 12;
							fChars = 62;
							fHeight = FONTHEIGHT_NORMAL;
							iMaxPages*=2;
							break;
						case NORMAL:
							fSize = BIG;
							fLines = 9;
							fChars = 47;
							fHeight = FONTHEIGHT_BIG;
							break;
						case BIG:
							fSize = SMALL;
							fLines = 15;
							fChars = 83;
							fHeight = FONTHEIGHT_SMALL;
							iMaxPages/=2;
							break;
					}
					break;
				}
			}
			row = 0;
			if( !feof(pipe) )
			{
				iPagePos[iPage] = ftell(pipe);
				if(iPagePos[iPage] == -1)
				{
					iPagePos[iPage] = 0;
				}
			}
			if((rccode == RC_HOME) || (rccode == RC_PLUS) || (rccode == RC_MINUS)) 
			{
				break;
			}
			// Render output window
			PaintMailHeader();
		}
	}
	if (rccode==RC_HOME) rccode=0;
	fclose(pipe);
}

/******************************************************************************
 * PaintSmtpMailHeader
 * nEditDirectStyle: 0: ABC, 1: Abc, 2: abc, 3: keyboard
 * nConfigPage:      -1: no Configpage, 0-9: mailboxes, >=10: main-settings
 ******************************************************************************/

void PaintSmtpMailHeader( int nEditDirectStyle , int nConfigPage)
{
	RenderBox(0, 0, VIEWX, 3*FONTHEIGHT_SMALL+2*BORDERSIZE+2, FILL, SKIN0);
	RenderBox(0, 3*FONTHEIGHT_SMALL+2*BORDERSIZE, VIEWX,VIEWY-INFOBOXY, FILL, SKIN1);
	RenderBox(0, VIEWY-INFOBOXY, VIEWX,VIEWY, FILL, SKIN0);
	RenderBox(0, 0, VIEWX, 3*FONTHEIGHT_SMALL+2*BORDERSIZE+2, GRID, SKIN2);
	RenderBox(0, 3*FONTHEIGHT_SMALL+2*BORDERSIZE, VIEWX,VIEWY-INFOBOXY, GRID, SKIN2);
	RenderBox(0, VIEWY-INFOBOXY, VIEWX,VIEWY, GRID, SKIN2);	
	RenderBox(300, VIEWY-INFOBOXY, VIEWX,VIEWY, GRID, SKIN2);	

	if( nConfigPage == -1 )
	{
		RenderString((osd == 'G') ? "Absender:" : "From:", 2*BORDERSIZE, BORDERSIZE+FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
		RenderString((osd == 'G') ? "Empfänger:" : "To:", 2*BORDERSIZE, BORDERSIZE+2*FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
		RenderString((osd == 'G') ? "Betreff:" : "Subject:", 2*BORDERSIZE, BORDERSIZE+3*FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
	}
	else
	{
		char linebuffer[80];

		RenderString((osd == 'G') ? "Konfiguration für TuxMail" : "Config for tuxmail", 2*BORDERSIZE, BORDERSIZE+FONTHEIGHT_BIG-2  , VIEWX-4*BORDERSIZE, CENTER, BIG, RED);
		sprintf(linebuffer,"%d / 12",nConfigPage+1);
		RenderString(linebuffer, 2*BORDERSIZE+550, BORDERSIZE+3*FONTHEIGHT_SMALL-2 , VIEWX-4*BORDERSIZE-500, LEFT, SMALL, WHITE);

		if( nConfigPage < 10 )
		{			
			RenderString((osd == 'G') ? "POP3/IMAP/SMTP Einstellungen Mailbox:" : "POP3/IMAP/SMTP  setting mailbox:", 2*BORDERSIZE, BORDERSIZE+3*FONTHEIGHT_SMALL-6  , VIEWX-4*BORDERSIZE, LEFT, NORMAL, ORANGE);
			sprintf(linebuffer,"%d",nConfigPage);
			RenderString(linebuffer, 2*BORDERSIZE+380, BORDERSIZE+3*FONTHEIGHT_SMALL-6  , VIEWX-4*BORDERSIZE, LEFT, NORMAL, WHITE);			
				
			int i;
			for( i=2; i<14; i++)
			{
				switch( i )
				{
					case 2: 
						strcpy(linebuffer, "NAME:"); 
						strcpy(szInfo[i],maildb[nConfigPage].namebox);
						if( maildb[nConfigPage].user[0]=='\0' )
						{
							szInfo[i][0]='\0';
						}
						break; 
					case 3: 
						strcpy(linebuffer, "POP3:"); 
						strcpy(szInfo[i],maildb[nConfigPage].pop3);
						break; 
					case 4: 
						strcpy(linebuffer, "USER:"); 
						strcpy(szInfo[i],maildb[nConfigPage].user);
						break; 
					case 5: 
						strcpy(linebuffer, "PASS:"); 
						strcpy(szInfo[i],maildb[nConfigPage].pass);
						break; 
					case 6: 
						strcpy(linebuffer, "SMTP:"); 
						strcpy(szInfo[i],maildb[nConfigPage].smtp);
						break; 
					case 7: 
						strcpy(linebuffer, "FROM:"); 
						strcpy(szInfo[i],maildb[nConfigPage].from);
						break; 
					case 8: 
						strcpy(linebuffer, "CODE:"); 
						strcpy(szInfo[i],maildb[nConfigPage].code);
						break; 
					case 9: 
						strcpy(linebuffer, "AUTH:"); 
						sprintf(szInfo[i],"%d",maildb[nConfigPage].auth);
						break; 
					case 10: 
						strcpy(linebuffer, "SUSER:"); 
						strcpy(szInfo[i],maildb[nConfigPage].suser);
						break; 
					case 11: 
						strcpy(linebuffer, "SPASS:"); 
						strcpy(szInfo[i],maildb[nConfigPage].spass);
						break; 
					case 12: 
						strcpy(linebuffer, "IMAP:"); 
						strcpy(szInfo[i],maildb[nConfigPage].imap);
						break; 
					case 13: 
						strcpy(linebuffer, "INBOX:"); 
						strcpy(szInfo[i],maildb[nConfigPage].inbox);
						break; 
				}
				RenderString( linebuffer, 2*BORDERSIZE, BORDERSIZE+(2+i)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
			}

		}
		else
		{
			RenderString((osd == 'G') ? "Grundeinstellungen:" : "main setting:", 2*BORDERSIZE, BORDERSIZE+3*FONTHEIGHT_SMALL-6  , VIEWX-4*BORDERSIZE, LEFT, NORMAL, ORANGE);
			
			if( nConfigPage == 10 )
			{
				int i;
				for( i=2; i<13; i++)
				{
					switch( i )
					{
						case 2: 
							strcpy(linebuffer, "STARTDELAY:"); 
							sprintf(szInfo[i],"%d",startdelay);
							break; 
						case 3: 
							strcpy(linebuffer, "INTERVALL:"); 
							sprintf(szInfo[i],"%d",intervall);
							break; 
						case 4: 
							strcpy(linebuffer, "LOGGING:"); 
							sprintf(szInfo[i],"%c",logging);
							break; 
						case 5: 
							strcpy(linebuffer, "LOGMODE:"); 
							sprintf(szInfo[i],"%c",logmode);
							break; 
						case 6: 
							strcpy(linebuffer, "SAVEDB:"); 
							sprintf(szInfo[i],"%c",savedb);
							break; 
						case 7: 
							strcpy(linebuffer, "AUDIO:"); 
							sprintf(szInfo[i],"%c",audio);
							break; 
						case 8: 
							strcpy(linebuffer, "VIDEO:"); 
							sprintf(szInfo[i],"%d",video);
							break; 
						case 9: 
							strcpy(linebuffer, "LCD:"); 
							sprintf(szInfo[i],"%c",lcdc);
							break; 
						case 10: 
							strcpy(linebuffer, "OSD:"); 
							sprintf(szInfo[i],"%c",osd);
							break; 
						case 11: 
							strcpy(linebuffer, "SKIN:"); 
							sprintf(szInfo[i],"%d",skin);
							break; 
						case 12: 
							strcpy(linebuffer, "TYPEFLAG:"); 
							sprintf(szInfo[i],"%d",typeflag);
							break; 
					}
					RenderString( linebuffer, 2*BORDERSIZE, BORDERSIZE+(2+i)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
				}
			}
			else
			{
				int i;
				for( i=2; i<12; i++)
				{
					switch( i )
					{
						case 2: 
							strcpy(linebuffer, "ADMIN:"); 
							sprintf(szInfo[i],"%c",admin);
							break; 
						case 3: 
							strcpy(linebuffer, "MAILCACHE:"); 
							sprintf(szInfo[i],"%d",mailcache);
							break; 
						case 4: 
							strcpy(linebuffer, "MAILDIR:"); 
							strcpy(szInfo[i],maildir);
							break; 
						case 5: 
							strcpy(linebuffer, "SECURITY:"); 
							strcpy(szInfo[i],security);
							break; 
						case 6: 
							strcpy(linebuffer, "WEBPORT:"); 
							sprintf(szInfo[i],"%d",webport);
							break; 
						case 7: 
							strcpy(linebuffer, "WEBUSER:"); 
							strcpy(szInfo[i],webuser);
							break; 
						case 8: 
							strcpy(linebuffer, "WEBPASS:"); 
							strcpy(szInfo[i],webpass);
							break; 
						case 9: 
							strcpy(linebuffer, "CONFIGCODE:"); 
							strcpy(szInfo[i],configcode);
							break; 
						default:
							linebuffer[0]='\0';
							szInfo[i][0]='\0';
					}
					RenderString( linebuffer, 2*BORDERSIZE, BORDERSIZE+(2+i)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE, LEFT, SMALL, ORANGE);
				}
			}
		}
	}
	
	int x, y;
	for( x = 0; x < 3; x++ )
	{
		for( y = 0; y < 4; y++)
		{
			RenderBox(10 + x*(KEYBOX_WIDTH+KEYBOX_SPACE),              VIEWY-INFOBOXY + KEYBOX_SPACE + y*(KEYBOX_HEIGHT+KEYBOX_SPACE), 
			          10 + x*(KEYBOX_WIDTH+KEYBOX_SPACE)+KEYBOX_WIDTH, VIEWY-INFOBOXY + KEYBOX_SPACE + y*(KEYBOX_HEIGHT+KEYBOX_SPACE)+KEYBOX_HEIGHT, FILL, SKIN1);
			RenderBox(10 + x*(KEYBOX_WIDTH+KEYBOX_SPACE),              VIEWY-INFOBOXY + KEYBOX_SPACE + y*(KEYBOX_HEIGHT+KEYBOX_SPACE), 
			          10 + x*(KEYBOX_WIDTH+KEYBOX_SPACE)+KEYBOX_WIDTH, VIEWY-INFOBOXY + KEYBOX_SPACE + y*(KEYBOX_HEIGHT+KEYBOX_SPACE)+KEYBOX_HEIGHT, GRID, SKIN2);
			if( nEditDirectStyle == 3 )
			{
				RenderString(szKeyBBoxKey[x+y*3],15 + x*(KEYBOX_WIDTH+KEYBOX_SPACE),VIEWY-INFOBOXY + KEYBOX_SPACE + FONTHEIGHT_SMALL - 3 + y*(KEYBOX_HEIGHT+KEYBOX_SPACE),25,LEFT, SMALL, WHITE);
				RenderString((osd == 'G') ? szKeyBBoxInfoDe[x+y*3] : szKeyBBoxInfoEn[x+y*3],30 + x*(KEYBOX_WIDTH+KEYBOX_SPACE),VIEWY-INFOBOXY + KEYBOX_SPACE + FONTHEIGHT_SMALL - 3+ y*(KEYBOX_HEIGHT+KEYBOX_SPACE),60, RIGHT, SMALL, ORANGE);
			}
			else
			{
				RenderString(szKeyBoxKey[x+y*3],15 + x*(KEYBOX_WIDTH+KEYBOX_SPACE),VIEWY-INFOBOXY + KEYBOX_SPACE + FONTHEIGHT_SMALL - 3 + y*(KEYBOX_HEIGHT+KEYBOX_SPACE),25,LEFT, SMALL, WHITE);
				RenderString(szKeyBoxInfo[x+y*3],30 + x*(KEYBOX_WIDTH+KEYBOX_SPACE),VIEWY-INFOBOXY + KEYBOX_SPACE + FONTHEIGHT_SMALL - 3+ y*(KEYBOX_HEIGHT+KEYBOX_SPACE),60, RIGHT, SMALL, ORANGE);
			}
		}
	}
}

/******************************************************************************
 * SaveConfigMailBox
 ******************************************************************************/

void SaveConfigMailBox(int nConfigPage)
{
	if( nConfigPage < 10 )
	{				
		int i;
		for( i=2; i<14; i++)
		{
			switch( i )
			{
				case 2: 
					strncpy(maildb[nConfigPage].namebox,szInfo[i],31);
					break; 
				case 3: 
					strncpy(maildb[nConfigPage].pop3,szInfo[i],63);
					break; 
				case 4: 
					strncpy(maildb[nConfigPage].user,szInfo[i],63);
					break; 
				case 5: 
					strncpy(maildb[nConfigPage].pass,szInfo[i],63);
					break; 
				case 6: 
					strncpy(maildb[nConfigPage].smtp,szInfo[i],63);
					break; 
				case 7: 
					strncpy(maildb[nConfigPage].from,szInfo[i],63);
					break; 
				case 8: 
					strncpy(maildb[nConfigPage].code,szInfo[i],4);
					break; 
				case 9: 
					maildb[nConfigPage].auth=atoi(szInfo[i]);
					if( maildb[nConfigPage].auth < 0 )
					{
						maildb[nConfigPage].auth=0;
					}
					if( maildb[nConfigPage].auth > 2 )
					{
						maildb[nConfigPage].auth=2;
					}
					break; 
				case 10: 
					strncpy(maildb[nConfigPage].suser,szInfo[i],63);
					break; 
				case 11: 
					strncpy(maildb[nConfigPage].spass,szInfo[i],63);
					break; 
				case 12: 
					strncpy(maildb[nConfigPage].imap,szInfo[i],63);
					break; 
				case 13: 
					strncpy(maildb[nConfigPage].inbox,szInfo[i],63);
					break; 
			}
		}
	}
	else if( nConfigPage == 10 )
	{
		int i, iTest;
		for( i=2; i<13; i++)
		{
			switch( i )
			{
				case 2: 
					iTest = atoi(szInfo[i]);
					if( startdelay == iTest )
					{
						if( startdelay < 15 )
						{ 
							startdelay=15;
						}
						if( startdelay > 120 )
						{ 
							startdelay=120;
						}
					}
					else
					{
						startdelay = iTest;
					}
					break; 
				case 3: 
					iTest = atoi(szInfo[i]);
					if( intervall == iTest )
					{
						if( intervall < 5 )
						{ 
							intervall=5;
						}
						if( intervall > 60 )
						{ 
							intervall=60;
						}
					}
					else
					{
						intervall = iTest;
					}
					break; 
				case 4: 
					if( logging != szInfo[i][0] )
					{
						if( logging == 'Y')
						{
							logging = 'N';
						}
						else
						{
							logging = 'Y';
						}
					}
					break; 
				case 5: 
					if( logmode != szInfo[i][0] )
					{
						if( logmode == 'S')
						{
							logmode = 'A';
						}
						else
						{
							logmode = 'S';
						}
					}
					break; 
				case 6: 
					if( savedb != szInfo[i][0] )
					{
						if( savedb == 'Y')
						{
							savedb = 'N';
						}
						else
						{
							savedb = 'Y';
						}
					}
					break; 
				case 7: 
					if( audio != szInfo[i][0] )
					{
						if( audio == 'Y')
						{
							audio = 'N';
						}
						else
						{
							audio = 'Y';
						}
					}
					break; 
				case 8: 
					video=atoi(szInfo[i]);
					if( video < 1 )
					{
						video = 1;
					}
					if( video > 5 )
					{
						video = 5;
					}
					break; 
				case 9: 
					if( lcdc != szInfo[i][0] )
					{
						if( lcdc == 'Y')
						{
							lcdc = 'N';
						}
						else
						{
							lcdc = 'Y';
						}
					}
					break; 
				case 10: 
					if( osd != szInfo[i][0] )
					{
						if( osd == 'G')
						{
							osd = 'E';
						}
						else
						{
							osd = 'G';
						}
					}
					break; 
				case 11: 
					skin=atoi(szInfo[i]);
					if( skin < 1 )
					{
						skin = 1;
					}
					if( skin > 3 )
					{
						skin = 3;
					}
					break; 
				case 12: 
					typeflag=atoi(szInfo[i]);
					if( typeflag < 1 )
					{
						typeflag = 1;
					}
					if( typeflag > 4 )
					{
						typeflag = 4;
					}
					break; 
			}
		}
	}
	else
	{
		int i;
		for( i=2; i<10; i++)
		{
			switch( i )
			{
				case 2: 
					if( admin != szInfo[i][0] )
					{
						if( admin == 'Y')
						{
							admin = 'N';
						}
						else
						{
							admin = 'Y';
						}
					}
					break; 
				case 3: 
					mailcache=atoi(szInfo[i]);
					if( mailcache < 0 )
					{
						mailcache = 0;
					}
					break; 
				case 4: 
					strncpy(maildir,szInfo[i],255);
					break; 
				case 5: 
					strncpy(security,szInfo[i],79);
					break; 
				case 6: 
					webport=atoi(szInfo[i]);
					if( webport < 0 )
					{
						webport = 80;
					}
					break; 
				case 7: 
					strncpy(webuser,szInfo[i],31);
					break; 
				case 8: 
					strncpy(webpass,szInfo[i],31);
					break; 
				case 9: 
					strncpy(configcode,szInfo[i],4);
					break; 

			}
		}
	}
	
}
/******************************************************************************
 * EditMailFile
 * account: 0-9: mailbox-account, -1: config
 ******************************************************************************/

void EditMailFile(char* filename, int account, int mailindex )
{
	char szSmtp[MAXLINELEN];
	char szFrom[MAXLINELEN];
	int nEditLine = 2;				// start to edit at body
	int nEditPos = 0;				// start to edit at body
	char nEditType = 0;				// type of edit: 0:letters, 1:T9, 2:direct
	int nEditDirectStyle = 0;		// 0: ABC, 1: Abc, 2: abc, 3: keyboard
	int nConfigPage = -1;			// -1: no Configpage, 0-9: mailboxes, >=10: main-settings
	int  nTextFileIdx = 0;
	int  nAddrFileIdx = 0;
	char szTextFile[MAXLINELEN];
	char TextFileValid = 0;
	char linebuffer[MAXLINELEN];
//	FILE* pipeT9 = NULL;
//	char szT9Code[11];
	char cChar;
	unsigned short cLastKey=RC_UP;
	
	FILE* pipe = NULL;
	
	if( account!= -1 )				// not editing the config
	{
		pipe = fopen(filename,"w");

		if(pipe == NULL)
		{
			return;
		}
	
		// prepare main strings
		strcpy(szSmtp,maildb[account].smtp);
		strcpy(szFrom,maildb[account].from);
		strncpy(szInfo[0],maildb[account].mailinfo[mailindex].from,MAXLINELEN-1);
		szInfo[0][MAXLINELEN-1]='\0';
		strcpy(szInfo[1],"Re: ");
		strncpy(&szInfo[1][4],maildb[account].mailinfo[mailindex].subj,MAXLINELEN-5);
		szInfo[1][MAXLINELEN-1]='\0';
		
	}
	else
	{
		nConfigPage = 0;
		nEditType = 2;
		int i;
		for( i=0; i<MAXINFOLINES; i++)
		{
			szInfo[i][0]='\0';
		}
	}
	
	while(1)
	{
		PaintSmtpMailHeader(nEditDirectStyle,nConfigPage);

		int y = nEditType * 30;
		RenderCircle( 310, VIEWY-INFOBOXY+10, 'D');
		RenderCircle( 310, VIEWY-INFOBOXY+40, 'N');	
		RenderCircle( 310, VIEWY-INFOBOXY+70, 'O');

		if( nConfigPage == -1)
		{
			RenderString( szFrom, 2*BORDERSIZE+100, BORDERSIZE+FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE-100, LEFT, SMALL, WHITE);
			
			if( nEditType < 2 )
			{
				RenderString( szInfo[0], 2*BORDERSIZE+100, BORDERSIZE+2*FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE-100, LEFT, SMALL, WHITE);
				RenderString( szInfo[1], 2*BORDERSIZE+100, BORDERSIZE+3*FONTHEIGHT_SMALL-2  , VIEWX-4*BORDERSIZE-100, LEFT, SMALL, WHITE);
			}
			RenderString((osd == 'G') ? "Texte" : "Letters", 325, VIEWY-INFOBOXY+30  , 100, CENTER, (nEditType == 0) ? NORMAL : SMALL, (nEditType == 0) ? ORANGE : WHITE);
			RenderString((osd == 'G') ? "T9" : "T9", 325, VIEWY-INFOBOXY+60  , 100, CENTER,  (nEditType == 1) ? NORMAL : SMALL, (nEditType == 1) ? ORANGE : WHITE);
		}
		else
		{
			RenderString((osd == 'G') ? "vor" : "next", 325, VIEWY-INFOBOXY+30  , 100, CENTER, SMALL,  WHITE);
			RenderString((osd == 'G') ? "zurück" : "back", 325, VIEWY-INFOBOXY+60  , 100, CENTER,  SMALL, WHITE);
		}
		
		RenderString(szDirectStyle[nEditDirectStyle], 325, VIEWY-INFOBOXY+90  , 100,  CENTER, (nEditType == 2) ? NORMAL : SMALL, (nEditType == 2) ? ORANGE : WHITE);
		RenderBox(330, VIEWY-INFOBOXY+y+5  , 430, VIEWY-INFOBOXY+35+y, GRID, SKIN2);
	
		// print text-blocks
		if( nEditType == 0 )
		{			
			// for index 0 we use the previously read mail file
			if( !nTextFileIdx )
			{
				strcpy(szTextFile, POP3FILE);
				
				if( mailcache )
				{
					char *stored_uids = 0, *ptr = 0;
					int idx1 = 0;
					int filesize = 0;
					char idxfile[256];
					FILE *fd_mailidx;

					sprintf(idxfile,"%stuxmail.idx%u",maildir,account);

					if((fd_mailidx = fopen(idxfile,"r")))
					{
						fseek(fd_mailidx, 0, SEEK_END);

						if((filesize = ftell(fd_mailidx)))
						{
							stored_uids = malloc(filesize + 1);
							memset(stored_uids, 0, filesize + 1);
		
							rewind(fd_mailidx);
							fread(stored_uids, filesize, 1, fd_mailidx);
						}
							
						if((filesize) && (ptr = strstr(stored_uids, maildb[account].mailinfo[mailindex].uid)))
						{
							// we already have this mail read
							sscanf(ptr-3,"%02u",&idx1);
							if( idx1 )
							{
								sprintf(szTextFile,"%stuxmail.idx%u.%u",maildir,account,idx1);
							}
						}
						free(stored_uids);
						fclose(fd_mailidx);
					}
				}
			}
			else
			{
				sprintf(szTextFile,"%s.%02u",TEXTFILE,nTextFileIdx);
			}

			FILE* pipe;
			pipe = fopen(szTextFile,"r");
			int i;
			for( i=2; i<MAXINFOLINES; i++ )
			{
				szInfo[i][0] = '\0';
			}
			if(pipe == NULL)
			{
				if( !nTextFileIdx )
				{
					RenderString((osd == 'G') ? "keine gelesene email" : "no read email", 2*BORDERSIZE,  BORDERSIZE+5*FONTHEIGHT_SMALL  , 500,  LEFT, BIG, ORANGE);				
				}
				else
				{
					RenderString((osd == 'G') ? "kein Textblock" : "no Letter", 2*BORDERSIZE,  BORDERSIZE+5*FONTHEIGHT_SMALL  , 500,  LEFT, BIG, ORANGE);				
				}
				TextFileValid = 0;
			}
			else
			{
				nEditLine = 2;
				memset(linebuffer, 0, sizeof(linebuffer));
				while(fgets(linebuffer, sizeof(linebuffer), pipe))
				{
					strcpy(szInfo[nEditLine], linebuffer);
					if( szInfo[nEditLine][strlen(linebuffer)-1] == '\n' )
					{
						szInfo[nEditLine][strlen(linebuffer)-1] = '\0';
					}
					RenderString( linebuffer, 2*BORDERSIZE, BORDERSIZE+(2+nEditLine)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE, LEFT, SMALL, WHITE);	
					if( ++nEditLine == MAXINFOLINES)
					{
						break;
					}
					memset(linebuffer, 0, sizeof(linebuffer));
				}
				
				fclose(pipe);
				TextFileValid = 1;
			}
		}
		else if( nEditType == 1 )
		{
	/*
			if( pipeT9 )
			{
				int i=nEditPos;
				while( (i) && (szInfo[nEditLine][i]>='A') && (szInfo[nEditLine][i]>='z') )
				{
					i--;
				}
				int iStart=i;
				while( i!=nEditPos )
				{
					char x = szInfo[nEditLine][i] & 0x5F;
					int j,k,l=0;
					
					for( k = 1; k < 9; k++ )
					{
						for( j = 0; j < strlen(szKeyBoxInfo[k]); j++)
						{
							if( szKeyBoxInfo[k][j] == x )
							{
								l = k + '0';
								break;
							}
						}
					}					
					szT9Code[i-iStart]=k;
					i++;
				}
				szT9Code[i-iStart]='\0';
			}
			else
*/
			{
				RenderString((osd == 'G') ? "kein T9 Wörterbuch gefunden" : "no T9 dictionary found", 2*BORDERSIZE,  BORDERSIZE+5*FONTHEIGHT_SMALL  , 500,  LEFT, BIG, ORANGE);				
			}
		}
		else if( nEditType == 2 )
		{
			int i;
			int xoff;
			for( i=0; i<MAXINFOLINES; i++ )
			{
				// the first two lines are 100 pixels to the right
				if( i<2 )
				{
					xoff = 100;
				}
				else
				{
					xoff = 0;
				}
				
				if( nConfigPage != -1 )
				{
					xoff = 100;
				}
								
				if( nEditLine ==  i )
				{
					if( szInfo[i][0] == '\0' )
					{
						RenderString( "_", 2*BORDERSIZE+xoff, BORDERSIZE+(2+i)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE-xoff, LEFT, NORMAL, ORANGE);					
					}
					else
					{
						int linelen, x;
						char linepart[80];
						
						linelen = strlen( szInfo[i] );
						
						if( nEditPos > linelen )
						{
							nEditPos = linelen;
						}
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
						desc.width = desc.height = 32;
#else
						desc.font.pix_width = desc.font.pix_height = 32;
#endif
						x = 0;
						
						if( nEditPos )
						{
							strncpy(linepart, szInfo[i], nEditPos);
							linepart[nEditPos] = '\0';
							x = GetStringLen( linepart );
							RenderString( linepart, 2*BORDERSIZE+xoff, BORDERSIZE+(2+i)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE-xoff, LEFT, NORMAL, WHITE);
						}
						
						linepart[0] = szInfo[i][nEditPos];
						if(( linepart[0] < 0x21 ) || ( linepart[0] > 0x7E ))
						{
							linepart[0] = '_';
						}
						linepart[1] = '\0';
						RenderString( linepart, 2*BORDERSIZE+xoff+x, BORDERSIZE+(2+i)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE-xoff, LEFT, NORMAL, ORANGE);
						x += GetStringLen( linepart );
						
						if( nEditPos != linelen )
						{
							strncpy(linepart, &szInfo[i][nEditPos+1], linelen - nEditPos - 1);
							linepart[linelen - nEditPos - 1] = '\0';
							RenderString( linepart, 2*BORDERSIZE+xoff+x, BORDERSIZE+(2+i)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE-xoff, LEFT, NORMAL, WHITE);
						}
					}
				}
				else
				{
					RenderString( szInfo[i], 2*BORDERSIZE+xoff, BORDERSIZE+(2+i)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE-xoff, LEFT, SMALL, WHITE);
				}
			}
		}
	
/*
		char szTmpOut[80];
		sprintf(szTmpOut,"Line:%d Pos:%d Type:%d T9:<%x>",nEditLine,nEditPos,nEditType,cLastKey);
		RenderString( szTmpOut, 2*BORDERSIZE, BORDERSIZE+(14)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE, LEFT, SMALL, WHITE);
*/
		
/*
		char szTmpOut[80];
		sprintf(szTmpOut,"Key:%x    Tast:%x-%x-%x-%x-%x-%x  ",rccode, tch[0], tch[1], tch[2], tch[3], tch[4], tch[5]);
		RenderString( szTmpOut, 2*BORDERSIZE, BORDERSIZE+(14)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE, LEFT, SMALL, WHITE);
*/

		memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);
		
		int valid = 1;
		do
		{
			GetRCCode();
			// don't show error-key, shift or alt
			switch(rccode)
			{
				case RC_CAPSLOCK:
				case RC_ON:
					if( nEditDirectStyle != 3 )
					{
						nEditLine = 2;
					}
					nEditDirectStyle = 3;
					nEditType = 2;
					rccode = -1;
					valid = 1;
					break;
				case 0xFFFF:
				case 0xFFF:
				case RC_LSHIFT:
				case RC_ALT:
				case RC_ALTGR:
					valid = 0;
					break;
				default:
					valid = 1;
			}
		}while( !valid ); 
	
		int iIndex = 0;
		cChar = 'A';
		if( ((rccode < RC_0) || (rccode > RC_9)) && (rccode != 0xFFFF) )
		{
			cLastKey = rccode;
		} 
		
		// for keyboard-input
		if( nEditDirectStyle == 3 )	
		{
			if( (rccode>=0x20) && (rccode<0x100) )
			{
				if( nEditPos < (MAXLINELEN-1) )
				{
					if( nEditPos < strlen(szInfo[nEditLine]))
					{
						nEditPos ++;
					}
					else
					{
						szInfo[nEditLine][nEditPos++]=' ';
						szInfo[nEditLine][nEditPos]='\0';
					}
				}
				szInfo[nEditLine][nEditPos-1]=rccode;
				rccode = -1;
			}
			else if( rccode == RC_BS )
			{
				if( nEditPos )
				{
					nEditPos--;
				}
				rccode = RC_MUTE;	
			}
			else if( rccode == RC_RET )
			{
				rccode = RC_DOWN;				
			}
			else if( rccode == RC_RET1 )
			{
				if( nEditLine > 1)
				{
					int i=MAXINFOLINES-1;
					while( nEditLine < i )
					{
						strcpy(szInfo[i],szInfo[i-1]);
						i--;
					}
					szInfo[nEditLine][0]='\0';
				}
				rccode = -1;				
			}
			else if( rccode == RC_ENTF )
			{
				rccode = RC_MUTE;				
			}
			else if( rccode == RC_INS )
			{
				rccode = RC_BLUE;		
				cChar = ' ';		
			}
			else if( rccode == RC_F5 )
			{
				rccode = RC_OK;				
			}
			else if( rccode == RC_F6 )
			{
				rccode = RC_HOME;				
			}
			else if( rccode == RC_F7 )
			{
				nEditPos = 0;
				rccode = -1;				
			}
			else if(( rccode == RC_F8 ) || ( rccode == RC_END ))
			{
				nEditPos = strlen(szInfo[nEditLine]);
				rccode = -1;				
			}
			else if(( rccode == RC_F9 ) && ( nConfigPage == -1))
			{
				if( nEditLine > 1)
				{
					int i;
					for( i=nEditLine; i<MAXINFOLINES-1; i++)
					{
						strcpy(szInfo[i],szInfo[i+1]);
					}
					szInfo[MAXINFOLINES-1][0]='\0';
					nEditPos = 0;
				}
				else
				{
					szInfo[nEditLine][0] = '\0';
					nEditPos = 0;
				}
				rccode = -1;
			}
			else if(( rccode == RC_F10 ) && ( nConfigPage == -1))
			{
 				MessageBox((osd == 'G') ? "Löschen" : "clear",(osd == 'G') ? "Text löschen?" : "clear all?");
				if(( rccode == RC_OK ) || ( rccode == RC_RET ))
				{
					int i;
					for( i=2; i<MAXINFOLINES; i++ )
					{
						szInfo[i][0] = '\0';
					}
					nEditPos = 0;
					nEditLine = 2;
				}
				rccode = -1;
			}
		}
		
		{
  		switch ( rccode )
  		{
  			case RC_9: iIndex++;
  			case RC_8: iIndex++;
  			case RC_7: iIndex++;
  			case RC_6: iIndex++;
  			case RC_5: iIndex++;
  			case RC_4: iIndex++;
  			case RC_3: iIndex++;
  			case RC_2: iIndex++;
  			case RC_1: iIndex++;
  			case RC_0: 
  				if(nEditType == 2)
  				{
  					char cNew, cAkt;
  				
  					if( (cLastKey != rccode) && ((cLastKey>= RC_0) && (cLastKey <= RC_9)) )
  					{
  						if( nEditPos < (MAXLINELEN-1) )
  						{
  							if( nEditPos < strlen(szInfo[nEditLine]))
  							{
  								nEditPos ++;
  							}
  							else
  							{
  								szInfo[nEditLine][nEditPos++]=' ';
  								szInfo[nEditLine][nEditPos]='\0';
  							}
  						}
  					}
  					cLastKey = rccode;
  					cAkt = szInfo[nEditLine][nEditPos];
  					
  					if( !iIndex )
  					{
  						iIndex = 9;
  					}
  					else
  					{
  						iIndex --;
  					}
  					cNew = szKeyBoxInfo[iIndex][0];
  
  					if( !cAkt )
  					{
  						if(((cNew >= 'A') && (cNew <= 'Z')) &&
  						   ((nEditDirectStyle == 2) ||
  						    ((nEditDirectStyle == 1) && (nEditPos>1) && (((szInfo[nEditLine][nEditPos-1]>='A') && (szInfo[nEditLine][nEditPos-1]<='z')) 
  						       || ((szInfo[nEditLine][nEditPos-1]==' ') && (szInfo[nEditLine][nEditPos-2]!='.') && (szInfo[nEditLine][nEditPos-2]!='!') && (szInfo[nEditLine][nEditPos-2]!='?'))
  						     ))
  						   )) 
  						{
  							cNew += ('a' - 'A');
  						}
  						if( strlen(szInfo[nEditLine]) < (MAXLINELEN-2) )
  						{
  							szInfo[nEditLine][nEditPos] = cNew;
  							szInfo[nEditLine][nEditPos+1] = '\0';
  						}
  					}
  					else
  					{
  						// now we do some SMS style calculation
  						int j;
  						char bFound = 0;
  						char bLowerCase = 0;
  						
  						if( (cAkt >= 'a') && (cAkt <= 'z') )
  						{
  							bLowerCase = 1;
  							cAkt -= ('a'-'A');
  						}
  						
  						for( j = 0; j < strlen(szKeyBoxInfo[iIndex]); j++)
  						{
  							if( szKeyBoxInfo[iIndex][j] == cAkt )
  							{
  								if( (j+1) < strlen(szKeyBoxInfo[iIndex]) )
  								{
  									j++;
  								}
  								else
  								{
  									j=0;
  								}
  								cNew = szKeyBoxInfo[iIndex][j];
  								bFound = 1;
  								break;
  							}
  							if( bFound )
  							{
  								break;
  							}
  						}
  						
  						if(((cNew >= 'A') && (cNew <= 'Z')) && 
  							 (((nEditDirectStyle == 1) && (nEditPos) && 
  							   (((szInfo[nEditLine][nEditPos-1]>='A') && (szInfo[nEditLine][nEditPos-1]<='z')) || (szInfo[nEditLine][nEditPos-1]==' '))) ||
  							  (nEditDirectStyle == 2)
  							 ))
  						{
  							cNew += ('a' - 'A');
  						}
  						szInfo[nEditLine][nEditPos] = cNew;
  					}
  				}			
  				break;
  				
  			case RC_OK:
  				if( nConfigPage == -1)
  				{	
  					MessageBox((osd == 'G') ? "Mail senden?" : "send mail?",(osd == 'G') ? "Mail jetzt senden?" : "send mail now?");
  				}
  				else
  				{
  					MessageBox((osd == 'G') ? "Konfiguration" : "config",(osd == 'G') ? "jetzt sichern?" : "save now?");
  				}
				if(( rccode == RC_OK ) || ( rccode == RC_RET ))
				{
  					cChar = '\0';
				}
  				break;
  
  			case RC_HOME:	
  			case RC_DBOX:
	 			MessageBox((osd == 'G') ? "Beenden" : "end",(osd == 'G') ? "Jetzt beenden?" : "end now?");
				if(( rccode == RC_OK ) || ( rccode == RC_RET ))
				{
					rccode = RC_HOME;
  					cChar = '\0';
				}
  				break;
  
  			case RC_RED:
  				if( nConfigPage == -1)
  				{
  					nEditType = 0;
  					nEditPos = 0;
  /*
	  				if( pipeT9 )
  					{
  						fclose (pipeT9 );
  						pipeT9 = NULL;
  					}
  */
  				}
  				else
  				{
  					SaveConfigMailBox(nConfigPage);
  					if( nConfigPage<11 )
  					{
  						nConfigPage++;
  					}
  				}
  				break;
  				
  			case RC_GREEN:
  				if( nConfigPage == -1)
  				{
	  				nEditType = 1;
  					nEditPos = 0;
  //					pipeT9 = fopen(T9FILE,"r");
  				}
  				else
  				{
  					SaveConfigMailBox(nConfigPage);
  					if( nConfigPage )
  					{
  						nConfigPage--;
  					}
  				}
  				break;
  				
  			case RC_YELLOW:
  /*
  				if( pipeT9 )
  				{
  					fclose (pipeT9 );
  					pipeT9 = NULL;
  				}
  */	
  				if( nEditType == 2 )
  				{
  					if( ++nEditDirectStyle > 3 )
  					{
  						nEditDirectStyle = 0;
  					}
  					char c = szInfo[nEditLine][nEditPos];
  					if( (nEditDirectStyle == 0) && (c >= 'a') && (c <= 'z'))
  					{
  						szInfo[nEditLine][nEditPos] = c - ('a'-'A');
  					}
  					else if( (nEditDirectStyle == 2) && (c >= 'A') && (c <= 'Z'))
  					{
  						szInfo[nEditLine][nEditPos] = c + ('a'-'A');
  					}
  				}
  				else
  				{
  					nEditType = 2;
  					nEditLine = 2;
  					nEditPos = 0;
  				}
  				break;
  				
  			case RC_BLUE:
  				if(nEditType == 2)
  				{
  					int len;
  							
  					len = strlen( szInfo[nEditLine] );
  				
  					if( !len )
  					{
  						szInfo[nEditLine][0] = cChar;
  						szInfo[nEditLine][1] = '\0';
  					}
  					else
  					{
  						if( len == MAXLINELEN )
  						{
  							break;
  						}
  						
  						char linepart[MAXLINELEN];
  						
  						if( !nEditPos )
  						{
  							strcpy( linepart, szInfo[nEditLine]);
  							sprintf( szInfo[nEditLine], "%c%s", cChar, linepart);
  						}
  						else if( nEditPos < len )
  						{
  							strcpy( linepart, &szInfo[nEditLine][nEditPos] );
  							szInfo[nEditLine][nEditPos] = cChar;
  							szInfo[nEditLine][nEditPos+1] = '\0';
  							strcat( szInfo[nEditLine], linepart );
  						}
  					}				
  				}
  				break;
  				
  			case RC_PLUS:
  				if((nEditType == 0) && (( TextFileValid ) || ( !nTextFileIdx )))
  				{
  					nTextFileIdx++;	
  				}
  				else if(nEditType == 2)
  				{
  					if( !nEditLine )
  					{
  						FILE* pipe;
  						pipe = fopen(ADDRFILE,"r");
  						if(pipe != NULL)
  						{
  							nAddrFileIdx++;
  							int i=0;
  							while(fgets(linebuffer, sizeof(linebuffer), pipe))
  							{
  								if( nAddrFileIdx==i )
  								{
  									strcpy( szInfo[nEditLine], linebuffer );
  									if( szInfo[nEditLine][strlen(linebuffer)-1] == '\n' )
  									{
  										szInfo[nEditLine][strlen(linebuffer)-1] = '\0';
  									}
  									break;
  								}
  								i++;
  							}
  							if( i < nAddrFileIdx )
  							{
  								nAddrFileIdx = i;
  							}
  							fclose( pipe );
  						}
  						break;
  					}
  					
  					char c;
  					
  					if(( c = szInfo[nEditLine][nEditPos] ))
  					{
  						if( c < 0x7D )
  						{
  							szInfo[nEditLine][nEditPos] = ++c;
  						}
  						else
  						{
  							szInfo[nEditLine][nEditPos] = '~';
  						}
  					}
  					else
  					{
  						szInfo[nEditLine][nEditPos] = cChar;
  						szInfo[nEditLine][nEditPos+1] = '\0';
  					}
  				}
  				break;
  				
  			case RC_MINUS:
  				if((nEditType == 0) && ( nTextFileIdx ))
  				{
  					nTextFileIdx--;
  				}
  				else if(nEditType == 2) 
  				{
  					if( !nEditLine )
  					{
  						FILE* pipe;
  						pipe = fopen(ADDRFILE,"r");
  						if(pipe != NULL)
  						{
  							if( nAddrFileIdx )
  							{
  								nAddrFileIdx--;
  							}
  							
  							int i=0;
  							while(fgets(linebuffer, sizeof(linebuffer), pipe))
  							{
  								if( nAddrFileIdx==i )
  								{
  									strcpy( szInfo[nEditLine], linebuffer );
  									if( szInfo[nEditLine][strlen(linebuffer)-1] == '\n' )
  									{
  										szInfo[nEditLine][strlen(linebuffer)-1] = '\0';
  									}
  									break;
  								}
  								i++;
  							}
  							fclose( pipe );
  						}
  						break;
  					}
  
  					char c;
  				
  					if(( c = szInfo[nEditLine][nEditPos] ))
  					{
  						if( c > 0x21 )
  						{
  							szInfo[nEditLine][nEditPos] = --c;
  						}
  						else
  						{
  							szInfo[nEditLine][nEditPos] = ' ';
  						}
  					}
  					else
  					{
  						szInfo[nEditLine][nEditPos] = cChar;
  						szInfo[nEditLine][nEditPos+1] = '\0';
  					}
  				}
  				break;
  				
  			case RC_DOWN:
  				if((nEditType == 2) && ( nEditLine < (MAXINFOLINES-1) ))
  				{
					if(( nConfigPage == -1) || (nEditLine<(MAXINFOLINES-2)))
					{
						nEditLine ++;
					}
  					int len=strlen( szInfo[nEditLine] );
  					if( len < (nEditPos + 1) )
  					{
  						if( len )
  						{
  							nEditPos = len - 1;
  						}
  						else
  						{ 
  							nEditPos = 0;
  						}
  					}
  				}
  				break;
  					
  			case RC_UP:
  				if((nEditType == 2) && ( nEditLine ))
  				{
					if(( nConfigPage == -1) || (nEditLine>2))
					{
						nEditLine --;
					}
					
  					int len=strlen( szInfo[nEditLine] );
  					if( len < (nEditPos + 1) )
  					{
  						if( len )
  						{
  							nEditPos = len - 1;
  						}
  						else
  						{ 
  							nEditPos = 0;
  						}
  					}
  				}
  				break;
  				
  			case RC_LEFT:
  				if((nEditType == 2) && ( nEditPos ))
  				{
  					nEditPos --;
  				}
  				break;
  				
  			case RC_RIGHT:
  				if(nEditType == 2)
  				{
  					if( nEditPos < (MAXLINELEN-1) )
  					{
  						if( nEditPos < strlen(szInfo[nEditLine]))
  						{
  							nEditPos ++;
  						}
  						else
  						{
  							szInfo[nEditLine][nEditPos++]=' ';
  							szInfo[nEditLine][nEditPos]='\0';
  						}
  					}
  				}
  				break;
  				
  			case RC_MUTE:
  				if(nEditType == 2)
  				{
  					if( szInfo[nEditLine][nEditPos] )
  					{
  						char linepart[MAXLINELEN];
  						int len;
  						
  						len = strlen( szInfo[nEditLine] );
  						szInfo[nEditLine][nEditPos] = '\0';
  						
  						if( !nEditPos )
  						{		
  							strcpy(	linepart, &szInfo[nEditLine][1] );
  							strcpy( szInfo[nEditLine], linepart );
  						}
  						else if( nEditPos < (len - 1) )
  						{
  							strcpy( linepart, &szInfo[nEditLine][ nEditPos + 1 ] );
  							strcat( szInfo[nEditLine], linepart );							
  						}
  					}
  				}
  				break;
	  		}
		}
		
		if( cChar == '\0' )
		{
			break;
		}

		if(( nConfigPage != -1 ) && ( rccode != RC_RED ) && ( rccode != RC_GREEN ))
		{
			SaveConfigMailBox(nConfigPage);
			if( nConfigPage<10 )
			{
				if(( maildb[nConfigPage].namebox[0] ) && (maildb[nConfigPage].user[0] == '\0'))
				{
					maildb[nConfigPage].user[0] = ' ';
				}
			}
		}
	}
	
	if( rccode == RC_OK)
	{
		if( account!= -1 )				// not editing the config
		{
			// check from:
			char *ptr1, *ptr2;
			if( (ptr1=strchr(maildb[account].from,'<')) )
			{
				if( (ptr2=strchr(ptr1,'>')) )
				{
					*ptr2 = '\0';
					strcpy(szFrom,ptr1);	
				}
			}		
			// check to: (and ptr1 to result)
			if( (ptr1=strchr(szInfo[0],'<')) )
			{
				if( (ptr2=strchr(ptr1++,'>')) )
				{
					*ptr2 = '\0';
				}
			}		
			else
			{
				ptr1 = szInfo[0];
			}
			fprintf(pipe,"%s\n",szSmtp);
			fputs("tuxmaild\n",pipe);
			fprintf(pipe,"<%s>\n",szFrom);
			fprintf(pipe,"<%s>\n",ptr1);
			fprintf(pipe,"From: %s\n",szFrom);
			fprintf(pipe,"To: %s\n",szInfo[0]);
			fprintf(pipe,"Subject: %s\n\n",szInfo[1]);
/*
			char szTmpOut[80];
			sprintf(szTmpOut,"from:<%s> to:<%s>",szFrom,ptr1);
			RenderString( szTmpOut, 2*BORDERSIZE, BORDERSIZE+(14)*FONTHEIGHT_SMALL  , VIEWX-4*BORDERSIZE, LEFT, SMALL, WHITE);
			memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);
*/		
			int i,j;
			j=MAXINFOLINES-1;
			// skip empty lines
			while( szInfo[j][0] == '\0' )
			{
				j--;
			}
			for( i=2; i<=j; i++)
			{
				fprintf(pipe,"%s\n",szInfo[i]);
			}
		}
		else
		{
			// save config
			WriteConf();
			ControlDaemon(RELOAD_CONFIG,0,0);
		}
	}
	else
	{
		rccode = 0;
	}
/*
	if( pipeT9 )
	{
		fclose (pipeT9 );
	}
*/
	if( pipe )
	{
		fclose(pipe);
	}

	if( account == -1)
	{
		SaveAndReloadDB(0);
	}

}

/******************************************************************************
 * ShowMessage
 ******************************************************************************/

void ShowMessage(int message)
{
    char info[32];

	// layout

		RenderBox(155, 178, 464, 220, FILL, SKIN0);
		RenderBox(155, 220, 464, 327, FILL, SKIN1);
		RenderBox(155, 178, 464, 327, GRID, SKIN2);
		RenderBox(155, 220, 464, 327, GRID, SKIN2);

	// message

		if(message != INFO)
		{
			RenderString("TuxMail Statusinfo", 157, 213, 306, CENTER, BIG, ORANGE);
		}

		switch(message)
		{
			case NODAEMON:

				RenderString((osd == 'G') ? "Daemon ist nicht geladen!" : "Daemon not running!", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case STARTDONE:

				RenderString((osd == 'G') ? "Abfrage wurde gestartet." : "Polling started.", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case STARTFAIL:

				RenderString((osd == 'G') ? "Start ist fehlgeschlagen!" : "Start failed!", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case STOPDONE:

				RenderString((osd == 'G') ? "Abfrage wurde gestoppt." : "Polling stopped.", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case STOPFAIL:

				RenderString((osd == 'G') ? "Stop ist fehlgeschlagen!" : "Stop failed!", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case BOOTON:

				RenderString((osd == 'G') ? "Autostart aktiviert." : "Autostart enabled.", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case BOOTOFF:

				RenderString((osd == 'G') ? "Autostart deaktiviert." : "Autostart disabled.", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case ADD2SPAM:

				RenderString((osd == 'G') ? "Spamliste wurde erweitert." : "Added to Spamlist.", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case DELSPAM:

				RenderString((osd == 'G') ? "von Spamliste entfernt." : "Removed from Spamlist.", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case SPAMFAIL:

				RenderString((osd == 'G') ? "Update fehlgeschlagen!" : "Update failed!", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case GETMAIL:

				RenderString((osd == 'G') ? "Mail wird gelesen." : "Reading Mail.", 157, 265, 306, CENTER, BIG, WHITE);
				RenderString((osd == 'G') ? "Moment bitte..." : "Please wait...", 157, 305, 306, CENTER, BIG, WHITE);

				break;

			case GETMAILFAIL:

				RenderString((osd == 'G') ? "Mail lesen fehlgeschlagen!" : "Reading Mail failed!", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case SENDMAILDONE:

				RenderString((osd == 'G') ? "Mail wurde gesendet." : "Mailing successful.", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case SENDMAILFAIL:

				RenderString((osd == 'G') ? "Mail nicht gesendet!" : "Mailing failed!", 157, 265, 306, CENTER, BIG, WHITE);

				break;

			case INFO:

				ControlDaemon(GET_VERSION,0,0);

				sprintf(info, "TuxMail (P%s/D%s)", versioninfo_p, versioninfo_d);

				RenderString(info, 157, 213, 306, CENTER, BIG, ORANGE);
				RenderString("(c) 2003-2005 Thomas \"LazyT\" Loewe", 157, 247, 306, CENTER, SMALL, WHITE);
				RenderString("(c) 2005-2006 Robert \"robspr1\" Spreitzer", 157, 273, 306, CENTER, SMALL, WHITE);
		}

		if(message != GETMAIL)
		{
		    RenderBox(285, 286, 334, 310, FILL, SKIN2);
		    RenderString("OK", 287, 305, 46, CENTER, SMALL, WHITE);
		}

		memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);

		if(message != GETMAIL)
		{
			while(GetRCCode() != RC_OK);
		}
}

/******************************************************************************
 * ShowMailInfo
 ******************************************************************************/

void ShowMailInfo(int account, int mailindex)
{
	int scrollbar_len, scrollbar_ofs, scrollbar_cor, loop;
	int sy = 61;
	int selectbar_mailindex = mailindex;

	// lcd

		if(lcd != -1)
		{
		    UpdateLCD(account);
		}
		else
		{
		    printf("TuxMail <no LCD found>\n");
		}

	// layout

		RenderBox(0, 0, 619, 504, FILL, SKIN0);
		RenderBox(0, 42, 593, 504, FILL, SKIN1);
		RenderBox(0, 0, 619, 504, GRID, SKIN2);
		RenderBox(0, 42, 593, 504, GRID, SKIN2);
		RenderBox(592, 42, 619, 69, GRID, SKIN2);
		RenderBox(592, 477, 619, 504, GRID, SKIN2);

	// status

		RenderString(maildb[account].nr, 12, 34, 20, LEFT, BIG, ORANGE);
		RenderString(maildb[account].time, 32, 34, 75, RIGHT, BIG, ORANGE);
		RenderString(maildb[account].name, 122, 34, 371, CENTER, BIG, ORANGE);
		RenderString(maildb[account].status, 503, 34, 105, RIGHT, BIG, ORANGE);

	// scrollbar

		mailindex = (mailindex/10)*10;

		for(loop = 0; loop < 14; loop++)
		{
			memcpy(lbb + startx + 599 + var_screeninfo.xres*(starty +  49 + loop), scroll_up + loop*14, 14);
			memcpy(lbb + startx + 599 + var_screeninfo.xres*(starty + 484 + loop), scroll_dn + loop*14, 14);
		}

		scrollbar_len = 403 / ((maildb[account].mails - 1)/10 + 1);
		scrollbar_ofs = scrollbar_len*mailindex / 10;
		scrollbar_cor = 403 - ((403/scrollbar_len)*scrollbar_len);
		RenderBox(596, 72 + scrollbar_ofs, 615, 72 + scrollbar_ofs + scrollbar_len + scrollbar_cor - 1, FILL, SKIN2);

	// check pin

		if(!CheckPIN(account))
		{
			do
			{
				GetRCCode();
			}
			while(rccode != RC_HOME && rccode != RC_0 && rccode != RC_1 && rccode != RC_2 && rccode != RC_3 && rccode != RC_4 && rccode != RC_5 && rccode != RC_6 && rccode != RC_7 && rccode != RC_8 && rccode != RC_9 && rccode != RC_PLUS && rccode != RC_MINUS);

			sim_key = 1;

			return;
		}
		else
		{
			RenderBox(155, 178, 464, 327, FILL, SKIN1);
		}

	// selectbar

		if(maildb[account].mails)
		{
			RenderBox(2, 44 + (selectbar_mailindex%10)*46, 591, 44 + (selectbar_mailindex%10)*46 + 44, FILL, SKIN2);
		}

	// mails

		for(loop = 0; loop < 10; loop++)
		{
			RenderString((char*)&maildb[account].mailinfo[mailindex + loop].date, 2, sy, 50, RIGHT, SMALL, WHITE);
			RenderString((char*)&maildb[account].mailinfo[mailindex + loop].time, 2, sy + 22, 50, RIGHT, SMALL, WHITE);
			RenderString((char*)&maildb[account].mailinfo[mailindex + loop].from, 75, sy, 517, LEFT, SMALL, WHITE);
			RenderString((char*)&maildb[account].mailinfo[mailindex + loop].subj, 75, sy + 22, 517, LEFT, SMALL, WHITE);

			RenderCircle( 56, sy - 4, maildb[account].mailinfo[mailindex + loop].type[0]);

			sy += 46;
		}

	// copy backbuffer to framebuffer

		memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);
}

/******************************************************************************
 * ViewMail
 ******************************************************************************/

void ViewMail(int account, int mailindex)
{
	rccode = 0;
	
	if(maildb[account].mails)
	{				
		if( mailcache )
		{
			char *stored_uids = 0, *ptr = 0;
			int idx1 = 0;
			int filesize = 0;
			char idxfile[256];
			char mailfile[256];
			FILE *fd_mailidx;

			sprintf(idxfile,"%stuxmail.idx%u",maildir,account);
	
			if((fd_mailidx = fopen(idxfile,"r")))
			{
				fseek(fd_mailidx, 0, SEEK_END);

				if((filesize = ftell(fd_mailidx)))
				{
					stored_uids = malloc(filesize + 1);
					memset(stored_uids, 0, filesize + 1);
	
					rewind(fd_mailidx);
					fread(stored_uids, filesize, 1, fd_mailidx);
				}
							
				if((filesize) && (ptr = strstr(stored_uids, maildb[account].mailinfo[mailindex].uid)))
				{
					// we already have this mail read
					sscanf(ptr-3,"%02u",&idx1);
					if( idx1 )
					{
						char szInfo[256];
						sprintf(mailfile,"%stuxmail.idx%u.%u",maildir,account,idx1);
						sprintf(szInfo, "%s\n%s\n%s %s\n", maildb[account].mailinfo[mailindex].from, maildb[account].mailinfo[mailindex].subj, maildb[account].mailinfo[mailindex].date, maildb[account].mailinfo[mailindex].time);
//						printf("cached file: %s",mailfile);
						ShowMailFile(mailfile, szInfo);
						free(stored_uids);
						fclose(fd_mailidx);
						SaveAndReloadDB(0);
						return;
					}
				}
				free(stored_uids);
				fclose(fd_mailidx);
			}
		}
		ControlDaemon(GET_MAIL, account, mailindex);

		if(!mailfile)
		{
			ShowMessage(GETMAILFAIL);
		}
		else
		{
			char szInfo[256];
			sprintf(szInfo, "%s\n%s\n%s %s\n", maildb[account].mailinfo[mailindex].from, maildb[account].mailinfo[mailindex].subj, maildb[account].mailinfo[mailindex].date, maildb[account].mailinfo[mailindex].time);
			ShowMailFile(POP3FILE, szInfo);
		}
		SaveAndReloadDB(0);
	}
}

/******************************************************************************
 * FillDB
 ******************************************************************************/

void FillDB(int account)
{
	char msg_file[] = "/tmp/tuxmail.?";
	char linebuffer[1024];
	FILE *fd_msg;
	int line = 0;

	// open status file

		msg_file[sizeof(msg_file) - 2] = account | '0';

		if(!(fd_msg = fopen(msg_file, "r")))
		{
			printf("TuxMail <no Status for Account %d>\n", account);

			maildb[account].nr[0] = account | '0';
			memcpy(maildb[account].time, "00:00", 5);
			memcpy(maildb[account].name, (osd == 'G') ? "keine Info verfügbar" : "Info isn't available", 20);
			memcpy(maildb[account].status, "000/000", 7);
			maildb[account].mails = 0;
			maildb[account].inactive = 1;

			return;
		}

	// get account, timestamp, name and mailstatus

		fgets(linebuffer, sizeof(linebuffer), fd_msg);
		sscanf(linebuffer, "%s %s %s %s", maildb[account].nr, maildb[account].time, maildb[account].name, maildb[account].status);

	// get date, time, from and subject for every mail

		while(fgets(linebuffer, sizeof(linebuffer), fd_msg))
		{
			char *entrystart;

			maildb[account].mails++;

			if((entrystart = strtok(linebuffer, "|")))
			{
				strncpy(maildb[account].mailinfo[line].type, entrystart, sizeof(maildb[account].mailinfo[line].type));
			}
			else
			{
				strcpy(maildb[account].mailinfo[line].type, "?");
			}

			if((entrystart = strtok(NULL, "|")))
			{
				strncpy(maildb[account].mailinfo[line].uid, entrystart, sizeof(maildb[account].mailinfo[line].uid));
			}
			else
			{
				strcpy(maildb[account].mailinfo[line].uid, "?");
			}

			if((entrystart = strtok(NULL, "|")))
			{
				strncpy(maildb[account].mailinfo[line].date, entrystart, sizeof(maildb[account].mailinfo[line].date));
			}
			else
			{
				strcpy(maildb[account].mailinfo[line].date, "??.???");
			}

			if((entrystart = strtok(NULL, "|")))
			{
				strncpy(maildb[account].mailinfo[line].time, entrystart, sizeof(maildb[account].mailinfo[line].time));
			}
			else
			{
				strcpy(maildb[account].mailinfo[line].time, "??:??");
			}

			if((entrystart = strtok(NULL, "|")))
			{
				strncpy(maildb[account].mailinfo[line].from, entrystart, sizeof(maildb[account].mailinfo[line].from));
			}
			else
			{
				strcpy(maildb[account].mailinfo[line].from, (osd == 'G') ? "TuxMail: DB-Eintrag defekt!" : "TuxMail: DB-Entry broken!");
			}

			if((entrystart = strtok(NULL, "|")))
			{
				strncpy(maildb[account].mailinfo[line].subj, entrystart, sizeof(maildb[account].mailinfo[line].subj));
			}
			else
			{
				strcpy(maildb[account].mailinfo[line].subj, (osd == 'G') ? "TuxMail: DB-Eintrag defekt!" : "TuxMail: DB-Entry broken!");
			}

//			maildb[account].mailinfo[line].save[0] = maildb[account].mailinfo[line].type[0];

			if(++line >= MAXMAIL)
			{
				break;
			}
		}

	fclose(fd_msg);
}

/******************************************************************************
 * UpdateDB
 ******************************************************************************/

void UpdateDB(int account)
{
	char msg_file[] = "/tmp/tuxmail.?";
	char linebuffer[1024];
	FILE *fd_msg;
	int loop, pos1, pos2;

	// set current file

		msg_file[sizeof(msg_file) - 2] = account | '0';

	// update

		if((fd_msg = fopen(msg_file, "r+")))
		{
			fgets(linebuffer, sizeof(linebuffer), fd_msg);

			for(loop = 0; loop < maildb[account].mails; loop++)
			{
				while(!feof(fd_msg))
				{
					pos1 = ftell(fd_msg);
					fgets(linebuffer, sizeof(linebuffer), fd_msg);
					pos2 = ftell(fd_msg);

					if(strstr(linebuffer, maildb[account].mailinfo[loop].uid))
					{
						fseek(fd_msg, pos1, SEEK_SET);
						fprintf(fd_msg, "|%c|", maildb[account].mailinfo[loop].type[0]);
						fseek(fd_msg, pos2, SEEK_SET);

						break;
					}
				}
			}

			fclose(fd_msg);
		}
		else
		{
			printf("TuxMail <could not update Status for Account %d>\n", account);
		}
}

/******************************************************************************
 * Add2SpamList (0=fail, 1=added, 2=removed)
 ******************************************************************************/

int Add2SpamList(int account, int mailindex)
{
	FILE *fd_spam;
	char *ptr1, *ptr2;
	char mailaddress[256];
	char linebuffer[256];
	char filebuffer[4096];
	char bRemove = 0;
	
	// find address

		if((ptr1 = strchr(maildb[account].mailinfo[mailindex].from, '@')))
		{
			while(*(ptr1 - 1) != '\0' && *(ptr1 - 1) != '<')
			{
				ptr1--;
			}

			ptr2 = ptr1;

			while(*(ptr2) != '\0' && *(ptr2) != '>')
			{
				ptr2++;
			}

			strncpy(mailaddress, ptr1, ptr2 - ptr1);
			mailaddress[ptr2 - ptr1] = '\0';
		}
		else
		{
			printf("TuxMail <Mailaddress \"%s\" invalid, not added to Spamlist>\n", maildb[account].mailinfo[mailindex].from);

			return 0;
		}

	// now we check if this address is already in the spamlist, if so we remove it

		if((fd_spam = fopen(CFGPATH SPMFILE, "r")))
		{
			memset(filebuffer, 0, sizeof(filebuffer));

			while(fgets(linebuffer, sizeof(linebuffer), fd_spam))
			{
				if(!strstr(linebuffer, mailaddress))
				{
					strcat(filebuffer, linebuffer);
				}
				else
				{
					bRemove = 1;
				}
			}

			fclose(fd_spam);
		}

	// add or remove

		if(!(fd_spam = fopen(CFGPATH SPMFILE, bRemove ? "w" : "a")))
		{
			printf("TuxMail <could not create Spamlist: %s>\n", strerror(errno));

			return 0;
		}

		if(!bRemove)
		{		
			// add address to spamlist
	
			printf("TuxMail <Mailaddress \"%s\" added to Spamlist>\n", mailaddress);

			fprintf(fd_spam, "%s\n", mailaddress);

			fclose(fd_spam);

			return 1;
		}
		else
		{
			// remove address from spamlist

			printf("TuxMail <Mailaddress \"%s\" removed from Spamlist>\n", mailaddress);

			fprintf(fd_spam, "%s", filebuffer);

			fclose(fd_spam);

			return 2;
		}
}

/******************************************************************************
 * CheckPIN (0=wrong, 1=ok)
 ******************************************************************************/

int CheckPIN(int Account)
{
	int result = 0;
	int skip;
	int count = 0;
	char code[4];
	char* pcode;
	int*  ppincount;
	
	if( Account == -1 )
	{
		pcode = &configcode[0];
		ppincount = &configpincount;
	}
	else
	{
		pcode = &maildb[Account].code[0];
		ppincount = &maildb[Account].pincount;
	}

	// pin active?
	
		if(*ppincount == -1 || pcode[0] == 0)
		{
			return 1;
		}

	// account locked?

		if(*ppincount == 3)
		{
			RenderBox(155, 178, 464, 220, FILL, SKIN0);
			RenderBox(155, 220, 464, 327, FILL, SKIN1);
			RenderBox(155, 178, 464, 327, GRID, SKIN2);
			RenderBox(155, 220, 464, 327, GRID, SKIN2);

			RenderString((osd == 'G') ? "Sicherheitsabfrage" : "Security Check", 157, 213, 306, CENTER, BIG, ORANGE);
			if( Account == -1 )
			{
				RenderString((osd == 'G') ? "Konfiguration gesperrt!" : "config locked!", 157, 265, 306, CENTER, BIG, WHITE);
			}
			else
			{
				RenderString((osd == 'G') ? "Konto gesperrt!" : "Account locked!", 157, 265, 306, CENTER, BIG, WHITE);
				RenderString((osd == 'G') ? "Bitte anderes Konto wählen..." : "Try another Account...", 157, 305, 306, CENTER, SMALL, WHITE);
			}

			memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);

			return 0;
		}

	// layout

		RenderBox(155, 178, 464, 220, FILL, SKIN0);
		RenderBox(155, 220, 464, 327, FILL, SKIN1);
		RenderBox(155, 178, 464, 327, GRID, SKIN2);
		RenderBox(155, 220, 464, 327, GRID, SKIN2);

		RenderString((osd == 'G') ? "Sicherheitsabfrage" : "Security Check", 157, 213, 306, CENTER, BIG, ORANGE);
		RenderString("PIN-Code :", 219, 265, 120, RIGHT, BIG, WHITE);
		RenderString("????", 346, 265, 100, LEFT, BIG, WHITE);

		RenderBox(285, 286, 334, 310, FILL, SKIN2);
		RenderString("EXIT", 287, 305, 46, CENTER, SMALL, WHITE);

		memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);

    	// get pin

		do
		{
			skip = 0;

			switch((rccode = GetRCCode()))
			{
				case RC_0:
					code[count] = '0';
					break;

				case RC_1:
					code[count] = '1';
					break;

				case RC_2:
					code[count] = '2';
					break;

				case RC_3:
					code[count] = '3';
					break;

				case RC_4:
					code[count] = '4';
					break;

				case RC_5:
					code[count] = '5';
					break;

				case RC_6:
					code[count] = '6';
					break;

				case RC_7:
					code[count] = '7';
					break;

				case RC_8:
					code[count] = '8';
					break;

				case RC_9:
					code[count] = '9';
					break;

				default:
					skip = 1;
			}

			if(!skip)
			{
				RenderBox(341, 236, 393, 270, FILL, SKIN1);

				switch(++count)
				{
					case 1:
						RenderString("*???", 346, 265, 100, LEFT, BIG, WHITE);
						break;

					case 2:
						RenderString("**??", 346, 265, 100, LEFT, BIG, WHITE);
						break;

					case 3:
						RenderString("***?", 346, 265, 100, LEFT, BIG, WHITE);
						break;

					case 4:
						RenderString("****", 346, 265, 100, LEFT, BIG, WHITE);
				}

				memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);
			}
		}
		while(rccode != RC_HOME && count < 4);

	// check pin

		if(count == 4)
		{
			if(strncmp(code, pcode, 4))
			{
				(*ppincount)++;

				RenderBox(157, 222, 462, 325, FILL, SKIN1);
				RenderString((osd == 'G') ? "Falsche PIN!" : "Wrong PIN!", 157, 265, 306, CENTER, BIG, WHITE);
				RenderString((osd == 'G') ? "Nächster Versuch in 5 Sekunden..." : "Try again in 5 Seconds...", 157, 305, 306, CENTER, SMALL, WHITE);
				memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);

				sleep(5);
			}
			else
			{
				*ppincount = -1;
				result = 1;
			}
		}

		if(!result)
		{
			RenderBox(157, 222, 462, 325, FILL, SKIN1);
			RenderString((osd == 'G') ? "Zugriff verweigert!" : "Access denied!", 157, 265, 306, CENTER, BIG, WHITE);
			if( Account != -1 )
			{
				RenderString((osd == 'G') ? "Bitte anderes Konto wählen..." : "Try another Account...", 157, 305, 306, CENTER, SMALL, WHITE);
			}

			memcpy(lfb, lbb, var_screeninfo.xres*var_screeninfo.yres);
		}

		return result;
}

/******************************************************************************
 * SaveAndReloadDB(int iSave)
 ******************************************************************************/

void SaveAndReloadDB(int iSave)
{
	int loop;
	
	if( iSave )
	{
		for(loop = 0; loop < 10; loop++)
		{
			UpdateDB(loop);
		}
	}
	
	int pincount[10];
	char code[10];
	
	// save already done pin info
	
	for( loop = 0; loop < 10; loop++)
	{
		pincount[loop]=maildb[loop].pincount;
		code[loop]=maildb[loop].code[0];
	}
	
	// fill database
	memset(maildb, 0, sizeof(maildb));

	for(loop = 0; loop < 10; loop++)
	{	
		FillDB(loop);
	}

	// read config

	ReadConf();

	// restore pin info
	
	for(loop = 0; loop < 10; loop++)
	{	
		maildb[loop].pincount=pincount[loop];
		maildb[loop].code[0]=code[loop];
	}

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
				perror("TuxMail <FBIOPUT_VSCREENINFO>");
		}
}

void plugin_exec(PluginParam *par)
{
	char cvs_revision[] = "$Revision: 1.48 $";
	int loop, account, mailindex;
	FILE *fd_run;
	FT_Error error;

	// show versioninfo

		sscanf(cvs_revision, "%*s %s", versioninfo_p);
		printf("TuxMail %s\n", versioninfo_p);

	// get params

		fb = rc = lcd = sx = ex = sy = ey = -1;

		for(; par; par = par->next)
		{
			if(!strcmp(par->id, P_ID_FBUFFER))
			{
				fb = atoi(par->val);
			}
			else if(!strcmp(par->id, P_ID_RCINPUT))
			{
				rc = atoi(par->val);
			}
			else if(!strcmp(par->id, P_ID_LCD))
			{
				lcd = atoi(par->val);
			}
			else if(!strcmp(par->id, P_ID_OFF_X))
			{
				sx = atoi(par->val);
			}
			else if(!strcmp(par->id, P_ID_END_X))
			{
				ex = atoi(par->val);
			}
			else if(!strcmp(par->id, P_ID_OFF_Y))
			{
				sy = atoi(par->val);
			}
			else if(!strcmp(par->id, P_ID_END_Y))
			{
				ey = atoi(par->val);
			}
		}

		if(fb == -1 || rc == -1 || sx == -1 || ex == -1 || sy == -1 || ey == -1)
		{
			printf("TuxMail <missing Param(s)>\n");

			return;
		}

	// keyboard

		kb = open("/dev/vc/0", O_RDONLY);

	// fill database

		memset(maildb, 0, sizeof(maildb));

		for(loop = 0; loop < 10; loop++)
		{
			FillDB(loop);
		}

	// read config

		ReadConf();

	// init framebuffer

		if(ioctl(fb, FBIOGET_FSCREENINFO, &fix_screeninfo) == -1)
		{
			printf("TuxMail <FBIOGET_FSCREENINFO failed>\n");

			return;
		}

		if(ioctl(fb, FBIOGET_VSCREENINFO, &var_screeninfo) == -1)
		{
			printf("TuxMail <FBIOGET_VSCREENINFO failed>\n");

			return;
		}

		prev_bpp = var_screeninfo.bits_per_pixel;
		if (prev_bpp != 8)
		{
			var_screeninfo.bits_per_pixel = 8;

			if (ioctl(fb, FBIOPUT_VSCREENINFO, &var_screeninfo) == -1)
			{
				perror("TuxMail <FBIOPUT_VSCREENINFO>");
				return;
			}
		}


		if(ioctl(fb, FBIOPUTCMAP, (skin == 1) ? &colormap1 : (skin == 2) ? &colormap2 : &colormap3) == -1)
		{
			printf("TuxMail <FBIOPUTCMAP failed>\n");
			resetBpp();

			return;
		}

		if(!(lfb = (unsigned char*)mmap(0, fix_screeninfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0)))
		{
			printf("TuxMail <mapping of Framebuffer failed>\n");
			resetBpp();

			return;
		}

	// init fontlibrary

		if((error = FT_Init_FreeType(&library)))
		{
			printf("TuxMail <FT_Init_FreeType failed with Errorcode 0x%.2X>", error);

			resetBpp();
			munmap(lfb, fix_screeninfo.smem_len);

			return;
		}

		if((error = FTC_Manager_New(library, 1, 2, 0, &MyFaceRequester, NULL, &manager)))
		{
			printf("TuxMail <FTC_Manager_New failed with Errorcode 0x%.2X>\n", error);

			FT_Done_FreeType(library);

			resetBpp();
			munmap(lfb, fix_screeninfo.smem_len);

			return;
		}

		if((error = FTC_SBitCache_New(manager, &cache)))
		{
			printf("TuxMail <FTC_SBitCache_New failed with Errorcode 0x%.2X>\n", error);

			FTC_Manager_Done(manager);

			FT_Done_FreeType(library);

			resetBpp();
			munmap(lfb, fix_screeninfo.smem_len);

			return;
		}

		if((error = FTC_Manager_Lookup_Face(manager, FONT, &face)))
		{
			printf("TuxMail <FTC_Manager_Lookup_Face failed with Errorcode 0x%.2X>\n", error);

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

		if(!(lbb = malloc(var_screeninfo.xres*var_screeninfo.yres)))
		{
			printf("TuxMail <allocating of Backbuffer failed>\n");

			FTC_Manager_Done(manager);

			FT_Done_FreeType(library);

			resetBpp();
			munmap(lfb, fix_screeninfo.smem_len);

			return;
		}

		memset(lbb, 0, var_screeninfo.xres*var_screeninfo.yres);

		startx = sx + (((ex-sx) - 620)/2);
		starty = sy + (((ey-sy) - 505)/2);

	// lock keyboard-conversions, this is done by the plugin itself
		fclose(fopen(KBLCKFILE,"w"));
	
	// get daemon status

		if(!ControlDaemon(GET_STATUS,0,0))
		{
			online = 2;
		}

	// remove last key & set rc to blocking mode

#if HAVE_DVB_API_VERSION == 3

		read(rc, &ev, sizeof(ev));
		fcntl(rc, F_SETFL, fcntl(rc, F_GETFL) &~ O_NONBLOCK);
#else
		read(rc, &rccode, sizeof(rccode));
 	fcntl(rc, F_SETFL, O_NONBLOCK);
#endif

	// show first account with new mail or account 0

		account = mailindex = 0;

		for(loop = 0; loop < 10; loop++)
		{
			if(maildb[loop].status[2] > '0' || maildb[loop].status[1] > '0' || maildb[loop].status[0] > '0')
			{
				account = loop;

				break;
			}
		}

		ShowMailInfo(account, mailindex);

	// main loop

		do
		{
			switch((rccode = GetRCCode()))
			{
				case RC_0:

					account = 0;
					mailindex = 0;

					break;

				case RC_1:

					account = 1;
					mailindex = 0;

					break;

				case RC_2:

					account = 2;
					mailindex = 0;

					break;

				case RC_3:

					account = 3;
					mailindex = 0;

					break;

				case RC_4:

					account = 4;
					mailindex = 0;

					break;

				case RC_5:

					account = 5;
					mailindex = 0;

					break;

				case RC_6:

					account = 6;
					mailindex = 0;

					break;

				case RC_7:

					account = 7;
					mailindex = 0;

					break;

				case RC_8:

					account = 8;
					mailindex = 0;

					break;

				case RC_9:

					account = 9;
					mailindex = 0;

					break;

				case RC_MINUS:

					mailindex = 0;

					for(loop = 0; loop < 10; loop++)
					{
						if(account > 0)
						{
							account--;
						}
						else
						{
							account = 9;
						}

						if(!maildb[account].inactive)
						{
							break;
						}
					}

					break;

				case RC_PLUS:

					mailindex = 0;

					for(loop = 0; loop < 10; loop++)
					{
						if(account < 9)
						{
							account++;
						}
						else
						{
							account = 0;
						}

						if(!maildb[account].inactive)
						{
							break;
						}
					}

					break;


				case RC_UP:

					if(mailindex > 0)
					{
						mailindex--;
					}

					break;

				case RC_DOWN:

					if(mailindex < maildb[account].mails-1)
					{
						mailindex++;
					}

					break;

				case RC_LEFT:

					if(!(mailindex%10) && mailindex)
					{
						mailindex--;
					}
					else
					{
						mailindex = (mailindex/10)*10;
					}

					break;

				case RC_RIGHT:

					if(mailindex%10 == 9)
					{
						mailindex++;
					}
					else
					{
						mailindex = (mailindex/10)*10 + 9;
					}

					if(mailindex >= maildb[account].mails)
					{
						if(maildb[account].mails)
						{
							mailindex = maildb[account].mails - 1;
						}
						else
						{
							mailindex = 0;
						}
					}

					break;

//				case RC_OK:

//					break;

				case RC_RED:

					if(admin == 'Y')
					{
						if(maildb[account].mails)
						{
							if(maildb[account].mailinfo[mailindex].type[0] != 'D')
							{
						    		maildb[account].mailinfo[mailindex].type[0] = 'D';
						    }
							else
							{
//								maildb[account].mailinfo[mailindex].type[0] = maildb[account].mailinfo[mailindex].save[0];
								
								if(maildb[account].mailinfo[mailindex].type[0] == 'D')
								{
									maildb[account].mailinfo[mailindex].type[0] = 'O';
								}
							}
						}
						SaveAndReloadDB(1);
					}

					break;

				case RC_GREEN:

					if(maildb[account].smtp[0] != '\0')
					{
						EditMailFile(SMTPFILE, account, mailindex);
						
						if( rccode == RC_OK )
						{
							ControlDaemon(SEND_MAIL, account, mailindex);
						}
						SaveAndReloadDB(0);
					}

					break;

				case RC_YELLOW:

					do
					{
						
						ViewMail(account, mailindex);
					
						if( rccode == RC_MINUS )
						{
							if(mailindex > 0)
							{
								mailindex--;
								rccode = RC_YELLOW;
							}
						}
						if( rccode == RC_PLUS)
						{
							if(mailindex < maildb[account].mails-1)
							{
								mailindex++;
								rccode = RC_YELLOW;
							}
						}
					} while( rccode == RC_YELLOW );
						
					break;

				case RC_BLUE:

					if(admin == 'Y' && maildb[account].mails)
					{
						int iRet = Add2SpamList(account, mailindex);

						if(iRet == 1)
						{
							maildb[account].mailinfo[mailindex].type[0] = 'D';
							ControlDaemon(RELOAD_SPAMLIST, 0, 0);
							ShowMessage(ADD2SPAM);
						}
						else if(iRet == 2)
						{
							maildb[account].mailinfo[mailindex].type[0] = 'O';
							ControlDaemon(RELOAD_SPAMLIST, 0, 0);
							ShowMessage(DELSPAM);
						}
						else
						{
							ShowMessage(SPAMFAIL);
						}
					}

					break;

				case RC_MUTE:

					if(!ControlDaemon(GET_STATUS, 0, 0))
					{
						online = 2;
						ShowMessage(NODAEMON);
					}
					else
					{
						online++;
						online &= 1;

						if(!ControlDaemon(SET_STATUS, 0, 0))
						{
							if(online)
							{
								ShowMessage(STARTFAIL);
							}
							else
							{
								ShowMessage(STOPFAIL);
							}

							online++;
							online &= 1;
						}
						else
						{
							if(online)
							{
								ShowMessage(STARTDONE);
							}
							else
							{
								ShowMessage(STOPDONE);
							}
						}
					}

					break;

				case RC_STANDBY:

					if((fd_run = fopen(RUNFILE, "r")))
					{
						fclose(fd_run);
						unlink(RUNFILE);
#ifdef OE
						unlink(OE_START);
						unlink(OE_KILL0);
						unlink(OE_KILL6);
#endif
						ShowMessage(BOOTOFF);
					}
					else
					{
						fclose(fopen(RUNFILE, "w"));
#ifdef OE
		        symlink("../init.d/tuxmail", OE_START);
						symlink("../init.d/tuxmail", OE_KILL0);
						symlink("../init.d/tuxmail", OE_KILL6);
#endif
						ShowMessage(BOOTON);
					}

					break;

				case RC_HELP:

					ShowMessage(INFO);

					break;

				case RC_DBOX:

					if( CheckPIN(-1) )
					{
						EditMailFile(NULL, -1, 0);
					}

					break;

				default:

					continue;
			}

			ShowMailInfo(account, mailindex);
		}
		while(rccode != RC_HOME);

	// reset lcd lock

    	unlink(LCKFILE);
    	
	// update database

		for(loop = 0; loop < 10; loop++)
		{
			if( typeflag > 1 )
			{
				if( maildb[loop].mails )
				{
					for( mailindex = 0; mailindex < maildb[loop].mails; mailindex++)
					{
						if(( maildb[loop].mailinfo[mailindex].type[0] == 'N' ) || ( maildb[loop].mailinfo[mailindex].type[0] == 'n' ))
						{
							if( typeflag == 2 )
							{
								maildb[loop].mailinfo[mailindex].type[0] = 'o';
							}
							else
							{
								maildb[loop].mailinfo[mailindex].type[0] = 'O';
							}
						}
					}
					if( typeflag >= 3 )
					{
						maildb[loop].status[0] = '0';
						maildb[loop].status[1] = '0';
						maildb[loop].status[2] = '0';
					}
				}
			}
			UpdateDB(loop);
		}

	// enable keyboard-conversion again
		unlink(KBLCKFILE);

	// remove notify file
		unlink(NOTIFILE);

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
