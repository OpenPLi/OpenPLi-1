/******************************************************************************
 *                       <<< TuxMailD - POP3 Daemon >>>
 *                (c) Thomas "LazyT" Loewe 2003 (LazyT@gmx.net)
 *-----------------------------------------------------------------------------
 * $Log: tuxmaild.c,v $
 * Revision 1.49  2008/08/28 20:30:10  robspr1
 * increase maximum startup delay to 120
 *
 * Revision 1.48  2007/01/06 16:27:03  robspr1
 * - accept unknown chunks in wave header
 *
 * Revision 1.47  2007/01/01 19:54:03  robspr1
 * -execute tuxmail.onreadmail in cfg-dir before new mail is read to cache
 *
 * Revision 1.46  2006/10/29 20:10:15  robspr1
 * - bugfix viewing deleted mails from IMAP box; - add body-scanning to spamfilter
 *
 * Revision 1.45  2006/09/28 19:06:33  robspr1
 * -subject-scanning of spamfiler also for a sentence, not only for one word
 *
 * Revision 1.44  2006/09/28 18:05:10  robspr1
 * -bugfix and add subject-scanning for spamfilter
 *
 * Revision 1.43  2006/06/20 21:25:36  robspr1
 * -bugfix for param -send
 *
 * Revision 1.42  2006/01/28 10:29:23  robspr1
 * - bugfix: close IMAP connection in any case
 *
 * Revision 1.41  2006/01/23 20:46:13  robspr1
 * - wait before receiving answer on SMTP commands
 *
 * Revision 1.40  2006/01/08 22:34:41  robspr1
 * - show correct unread-mails notification
 *
 * Revision 1.39  2005/12/12 19:00:04  robspr1
 * -bugfix USER/SUSER and PASS/SPASS extraction
 *
 * Revision 1.38  2005/11/19 14:38:24  robspr1
 * - add different behaviour in marking mails green in the plugin
 *
 * Revision 1.37  2005/11/13 14:18:30  robspr1
 * - don't clear audio-flag in cause of DSP error
 *
 * Revision 1.36  2005/11/12 17:44:06  robspr1
 * - IMAP \Seen Flag bugfix (also clear \Seen Flag after reading mail-header)
 *
 * Revision 1.35  2005/11/11 18:41:56  robspr1
 * - /tmp/tuxmail.new holds number of new files /  restrict reset flags for unseen mails in IMAP
 *
 * Revision 1.34  2005/11/05 17:29:08  robspr1
 * - IMAP bugfix delete mails and restore Seen flag
 *
 * Revision 1.33  2005/11/04 16:00:37  robspr1
 * - adding IMAP support
 *
 * Revision 1.32  2005/10/29 12:53:01  robspr1
 * - bugfix for too long mail-headers
 *
 * Revision 1.31  2005/08/13 18:47:37  robspr1
 * - correct RSET command, change USR1 / USR2 meaning
 *
 * Revision 1.30  2005/07/06 19:55:56  robspr1
 * - add cmd-line interface for sending mails
 *
 * Revision 1.29  2005/07/05 19:59:22  robspr1
 * - add execution of special mail
 *
 * Revision 1.28  2005/06/27 19:50:48  robspr1
 * - reset read-flag
 *
 * Revision 1.27  2005/05/26 09:37:35  robspr1
 * - add param for SMTP AUTH  - support just playing audio (call tuxmaild -play wavfile)
 *
 * Revision 1.26  2005/05/24 16:37:22  lazyt
 * - fix WebIF Auth
 * - add SMTP Auth
 *
 * Revision 1.25  2005/05/19 21:55:58  robspr1
 * - bugfix cached mailreading
 *
 * Revision 1.24  2005/05/19 10:03:50  robspr1
 * - add cached mailreading
 *
 * Revision 1.23  2005/05/15 10:16:19  lazyt
 * - add SMTP-Logging
 * - change Parameters (POP3LOG now LOGGING, HOST? now POP3?)
 *
 * Revision 1.22  2005/05/13 23:16:19  robspr1
 * - first Mail writing GUI\n- add parameters for Mail sending
 *
 * Revision 1.21  2005/05/12 14:28:28  lazyt
 * - PIN-Protection for complete Account
 * - Preparation for sending Mails ;-)
 *
 * Revision 1.20  2005/05/11 19:01:35  robspr1
 * minor Mailreader changes / add to Spamlist undo
 *
 * Revision 1.19  2005/05/11 12:01:23  lazyt
 * Protect Mailreader with optional PIN-Code
 *
 * Revision 1.18  2005/05/10 12:55:16  lazyt
 * - LCD-Fix for DM500
 * - Autostart for DM7020 (use -DOE, put Init-Script to /etc/init.d/tuxmail)
 * - try again after 10s if first DNS-Lookup failed
 * - don't try to read Mails on empty Accounts
 *
 * Revision 1.17  2005/05/09 19:41:53  robspr1
 * support for mail reading
 *
 * Revision 1.16  2005/04/29 17:24:01  lazyt
 * use 8bit audiodata, fix skin and osd
 *
 * Revision 1.15  2005/03/28 14:14:15  lazyt
 * support for userdefined audio notify (put your 12/24/48KHz pcm wavefile to /var/tuxbox/config/tuxmail/tuxmail.wav)
 *
 * Revision 1.14  2005/03/24 13:12:11  lazyt
 * cosmetics, support for syslog-server (start with -syslog)
 *
 * Revision 1.13  2005/03/22 13:31:48  lazyt
 * support for english osd (OSD=G/E)
 *
 * Revision 1.12  2005/03/22 09:35:21  lazyt
 * lcd support for daemon (LCD=Y/N, GUI should support /tmp/lcd.locked)
 *
 * Revision 1.11  2005/03/14 17:45:27  lazyt
 * simple base64 & quotedprintable decoding
 *
 * Revision 1.10  2005/02/26 10:23:49  lazyt
 * workaround for corrupt mail-db
 * add ADMIN=Y/N to conf (N to disable mail deletion via plugin)
 * show versioninfo via "?" button
 * limit display to last 100 mails (increase MAXMAIL if you need more)
 *
 * Revision 1.9  2004/08/20 14:57:37  lazyt
 * add http-auth support for password protected webinterface
 *
 * Revision 1.8  2004/07/10 11:38:15  lazyt
 * use -DOLDFT for older FreeType versions
 * replaced all remove() with unlink()
 *
 * Revision 1.7  2004/06/29 16:33:10  lazyt
 * fix commandline interface
 *
 * Revision 1.6  2004/04/03 17:33:08  lazyt
 * remove curl stuff
 * fix audio
 * add new options PORT=n, SAVEDB=Y/N
 *
 * Revision 1.5  2004/03/31 13:55:36  thegoodguy
 * use UTF-8 encoding for ü (\xC3\xBC)
 *
 * Revision 1.4  2003/05/16 15:07:23  lazyt
 * skip unused accounts via "plus/minus", add mailaddress to spamlist via "blue"
 *
 * Revision 1.3  2003/05/10 08:24:35  lazyt
 * add simple spamfilter, show account details in message/popup
 *
 * Revision 1.2  2003/04/29 10:36:43  lazyt
 * enable/disable audio via .conf
 *
 * Revision 1.1  2003/04/21 09:24:52  lazyt
 * add tuxmail, todo: sync (filelocking?) between daemon and plugin
 ******************************************************************************/

#include "tuxmaild.h"

/******************************************************************************
 * ReadConf (0=fail, 1=done)
 ******************************************************************************/

int ReadConf()
{
	FILE *fd_conf;
	char *ptr;
	char line_buffer[256];
	int loop;

	// open config

		if(!(fd_conf = fopen(CFGPATH CFGFILE, "r+")))
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "generate new Config, please modify and restart Daemon") : printf("TuxMailD <generate new Config, please modify and restart Daemon>\n");

			if(mkdir(CFGPATH, S_IRWXU) == -1 && errno != EEXIST)
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "could not create ConfigDir") : printf("TuxMailD <could not create ConfigDir>\n");

				return 0;
			}

			if(!(fd_conf = fopen(CFGPATH CFGFILE, "w")))
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "could not create Config") : printf("TuxMailD <could not create Config>\n");

				return 0;
			}

			fprintf(fd_conf, "STARTDELAY=30\n");
			fprintf(fd_conf, "INTERVALL=15\n\n");
			fprintf(fd_conf, "LOGGING=Y\n");
			fprintf(fd_conf, "LOGMODE=S\n\n");
			fprintf(fd_conf, "SAVEDB=Y\n\n");
			fprintf(fd_conf, "AUDIO=Y\n");
			fprintf(fd_conf, "VIDEO=1\n\n");
			fprintf(fd_conf, "LCD=Y\n");
			fprintf(fd_conf, "OSD=G\n\n");
			fprintf(fd_conf, "SKIN=1\n\n");
			fprintf(fd_conf, "TYPEFLAG=1\n\n");
			fprintf(fd_conf, "ADMIN=Y\n\n");
			fprintf(fd_conf, "MAILCACHE=0\n");
			fprintf(fd_conf, "MAILDIR=\\tmp\\\n");		
			fprintf(fd_conf, "SECURITY=\n\n");		
			fprintf(fd_conf, "WEBPORT=80\n");
			fprintf(fd_conf, "WEBUSER=\n");
			fprintf(fd_conf, "WEBPASS=\n\n");
			fprintf(fd_conf, "NAME0=\n");
			fprintf(fd_conf, "POP30=\n");
			fprintf(fd_conf, "IMAP0=\n");
			fprintf(fd_conf, "USER0=\n");
			fprintf(fd_conf, "PASS0=\n");
			fprintf(fd_conf, "SMTP0=\n");
			fprintf(fd_conf, "FROM0=\n");
			fprintf(fd_conf, "CODE0=\n");
			fprintf(fd_conf, "AUTH0=0\n");
			fprintf(fd_conf, "SUSER0=\n");
			fprintf(fd_conf, "SPASS0=\n");
			fprintf(fd_conf, "INBOX0=\n");

			fclose(fd_conf);

			return 0;
		}

	// clear database

		memset(account_db, 0, sizeof(account_db));

		startdelay = intervall = logging = logmode = savedb = audio = video = lcd = osd = skin = admin = maildir[0] = webport = webuser[0] = webpass[0] = 0;

	// fill database

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
				sscanf(ptr + 4, "%c", &lcd);
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
					sscanf(ptr + 6, "%s", account_db[index-'0'].name);
				}
			}
			else if((ptr = strstr(line_buffer, "POP3")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", account_db[index-'0'].pop3);
				}
			}
			else if((ptr = strstr(line_buffer, "IMAP")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", account_db[index-'0'].imap);
				}
			}
			else if((ptr = strstr(line_buffer, "SUSER")) && (*(ptr+6) == '='))
			{
				char index = *(ptr+5);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 7, "%s", account_db[index-'0'].suser);
				}
			}
			else if((ptr = strstr(line_buffer, "SPASS")) && (*(ptr+6) == '='))
			{
				char index = *(ptr+5);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 7, "%s", account_db[index-'0'].spass);
				}
			}
			else if((ptr = strstr(line_buffer, "USER")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", account_db[index-'0'].user);
				}
			}
			else if((ptr = strstr(line_buffer, "PASS")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", account_db[index-'0'].pass);
				}
			}
			else if((ptr = strstr(line_buffer, "SMTP")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%s", account_db[index-'0'].smtp);
				}
			}
			else if((ptr = strstr(line_buffer, "FROM")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
//					strncpy(account_db[index-'0'].from,ptr+6,63);
					sscanf(ptr + 6, "%s", account_db[index-'0'].from);
				}
			}
			else if((ptr = strstr(line_buffer, "CODE")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%4s", account_db[index-'0'].code);
				}
			}
			else if((ptr = strstr(line_buffer, "AUTH")) && (*(ptr+5) == '='))
			{
				char index = *(ptr+4);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 6, "%d", &account_db[index-'0'].auth);
				}
			}
			else if((ptr = strstr(line_buffer, "INBOX")) && (*(ptr+6) == '='))
			{
				char index = *(ptr+5);
				if((index >= '0') && (index <= '9'))
				{
					sscanf(ptr + 7, "%s", account_db[index-'0'].inbox);
				}
			}
		}

	// check for update

		if(!startdelay || !intervall || !logging || !logmode || !savedb || !audio || !video || !lcd || !osd || !skin || !typeflag || !admin || !webport)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "missing Param(s), update Config") : printf("TuxMailD <missing Param(s), update Config>\n");

			rewind(fd_conf);

			if(!startdelay)
			{
				startdelay = 30;
			}

			if(!intervall)
			{
				intervall = 15;
			}

			if(!logging)
			{
				logging = 'Y';
			}

			if(!logmode)
			{
				logmode = 'S';
			}

			if(!savedb)
			{
				savedb = 'Y';
			}

			if(!audio)
			{
				audio = 'Y';
			}

			if(!video)
			{
				video = 1;
			}

			if(!lcd)
			{
				lcd = 'Y';
			}

			if(!osd)
			{
				osd = 'G';
			}

			if(!skin)
			{
				skin = 1;
			}
			
			if(!typeflag)
			{
				typeflag = 1;
			}

			if(!admin)
			{
				admin = 'Y';
			}

			if(!webport)
			{
				webport = 80;
			}

			fprintf(fd_conf, "STARTDELAY=%d\n", startdelay);
			fprintf(fd_conf, "INTERVALL=%d\n\n", intervall);
			fprintf(fd_conf, "LOGGING=%c\n", logging);
			fprintf(fd_conf, "LOGMODE=%c\n\n", logmode);
			fprintf(fd_conf, "SAVEDB=%c\n\n", savedb);
			fprintf(fd_conf, "AUDIO=%c\n", audio);
			fprintf(fd_conf, "VIDEO=%d\n\n", video);
			fprintf(fd_conf, "LCD=%c\n", lcd);
			fprintf(fd_conf, "OSD=%c\n\n", osd);
			fprintf(fd_conf, "SKIN=%d\n\n", skin);
			fprintf(fd_conf, "TYPEFLAG=%d\n\n", typeflag);
			fprintf(fd_conf, "ADMIN=%c\n\n", admin);
			fprintf(fd_conf, "MAILCACHE=%d\n", mailcache);
			fprintf(fd_conf, "MAILDIR=%s\n", maildir);
			fprintf(fd_conf, "SECURITY=%s\n\n", security);
			fprintf(fd_conf, "WEBPORT=%d\n", webport);
			fprintf(fd_conf, "WEBUSER=%s\n", webuser);
			fprintf(fd_conf, "WEBPASS=%s\n", webpass);

			for(loop = 0; loop < 10; loop++)
			{
				fprintf(fd_conf, "\nNAME%d=%s\n", loop, account_db[loop].name);
				fprintf(fd_conf, "POP3%d=%s\n", loop, account_db[loop].pop3);
				fprintf(fd_conf, "IMAP%d=%s\n", loop, account_db[loop].imap);
				fprintf(fd_conf, "USER%d=%s\n", loop, account_db[loop].user);
				fprintf(fd_conf, "PASS%d=%s\n", loop, account_db[loop].pass);
				fprintf(fd_conf, "SMTP%d=%s\n", loop, account_db[loop].smtp);
				fprintf(fd_conf, "FROM%d=%s\n", loop, account_db[loop].from);
				fprintf(fd_conf, "CODE%d=%s\n", loop, account_db[loop].code);
				fprintf(fd_conf, "AUTH%d=%d\n", loop, account_db[loop].auth);
				fprintf(fd_conf, "SUSER%d=%s\n", loop, account_db[loop].suser);
				fprintf(fd_conf, "SPASS%d=%s\n", loop, account_db[loop].spass);
				fprintf(fd_conf, "INBOX%d=%s\n", loop, account_db[loop].inbox);

				if(!account_db[loop + 1].name[0])
				{
					break;
				}
			}
		}

		fclose(fd_conf);

	// check config

		if(startdelay < 15 || startdelay > 120)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "STARTDELAY=%d out of Range, set to \"30\"", startdelay) : printf("TuxMailD <STARTDELAY=%d out of Range, set to \"30\">\n", startdelay);

			startdelay = 30;
		}

		if(!intervall)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "INTERVALL=0, check Account(s) and Exit") : printf("TuxMailD <INTERVALL=0, check Account(s) and Exit>\n");
		}
		else if(intervall < 5 || intervall > 60)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "INTERVALL=%d out of Range, set to \"15\"", intervall) : printf("TuxMailD <INTERVALL=%d out of Range, set to \"15\">\n", intervall);

			intervall = 15;
		}

		if(logging != 'Y' && logging != 'N')
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "LOGGING=%c invalid, set to \"N\"", logging) : printf("TuxMailD <LOGGING=%c invalid, set to \"N\">\n", logging);

			logging = 'N';
		}

		if(logmode != 'A' && logmode != 'S')
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "LOGMODE=%c invalid, set to \"S\"", logmode) : printf("TuxMailD <LOGMODE=%c invalid, set to \"S\">\n", logmode);

			logmode = 'S';
		}

		if(savedb != 'Y' && savedb != 'N')
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "SAVEDB=%c invalid, set to \"Y\"", savedb) : printf("TuxMailD <SAVEDB=%c invalid, set to \"Y\">\n", savedb);

			savedb = 'Y';
		}

		if(audio != 'Y' && audio != 'N')
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "AUDIO=%c invalid, set to \"Y\"", audio) : printf("TuxMailD <AUDIO=%c invalid, set to \"Y\">\n", audio);

			audio = 'Y';
		}

		if(video < 1 || video > 5)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "VIDEO=%d invalid, set to \"1\"", video) : printf("TuxMailD <VIDEO=%d invalid, set to \"1\">\n", video);

			video = 1;
		}

		if(lcd != 'Y' && lcd != 'N')
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "LCD=%c invalid, set to \"Y\"", lcd) : printf("TuxMailD <LCD=%c invalid, set to \"Y\">\n", lcd);

			lcd = 'Y';
		}

		if(osd != 'G' && osd != 'E')
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "OSD=%c invalid, set to \"G\"", osd) : printf("TuxMailD <OSD=%c invalid, set to \"G\">\n", osd);

			osd = 'G';
		}

		if(skin != 1 && skin != 2 && skin != 3)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "SKIN=%d invalid, set to \"1\"", skin) : printf("TuxMailD <SKIN=%d invalid, set to \"1\">\n", skin);

			skin = 1;
		}

		if(typeflag < 1 || typeflag > 4)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "TYPEFLAG=%d invalid, set to \"1\"", typeflag) : printf("TuxMailD <TYPEFLAG=%d invalid, set to \"1\">\n", typeflag);

			typeflag = 1;
		}

		if(admin != 'Y' && admin != 'N')
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "ADMIN=%c invalid, set to \"Y\"", admin) : printf("TuxMailD <ADMIN=%c invalid, set to \"Y\">\n", admin);

			admin = 'Y';
		}

		if(webport < 1 || webport > 65535)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "WEBPORT=%d invalid, set to \"80\"", webport) : printf("TuxMailD <WEBPORT=%d invalid, set to \"80\">\n", webport);

			webport = 80;
		}

		accounts = 0;

		for(loop = 0; loop <= 9; loop++)
		{
			if(account_db[loop].name[0] && (account_db[loop].pop3[0] || account_db[loop].imap[0]) && account_db[loop].user[0] && account_db[loop].pass[0])
			{
				accounts++;
			}
			else
			{
				break;
			}
		}

		if(maildir[0])
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "store max. %d mails in %s\n", mailcache,maildir) : printf("TuxMailD <store max. %d mails in %s>\n", mailcache, maildir);
		}
		
		if(security[0])
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "security for executing commands defined\n") : printf("TuxMailD <security for executing commands defined>\n");
		}

		if(accounts)
		{
			if(logging == 'N')
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "check %d Account(s) every %dmin without Logging", accounts, intervall) : printf("TuxMailD <check %d Account(s) every %dmin without Logging>\n", accounts, intervall);
			}
			else if(logmode == 'A')
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "check %d Account(s) every %dmin with Logging in Append-Mode", accounts, intervall) : printf("TuxMailD <check %d Account(s) every %dmin with Logging in Append-Mode>\n", accounts, intervall);
			}
			else
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "check %d Account(s) every %dmin with Logging in Single-Mode", accounts, intervall) : printf("TuxMailD <check %d Account(s) every %dmin with Logging in Single-Mode>\n", accounts, intervall);
			}

			return 1;
		}
		else
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "no valid Accounts found") : printf("TuxMailD <no valid Accounts found>\n");

			return 0;
		}
}

/******************************************************************************
 * ReadSpamList
 ******************************************************************************/

void ReadSpamList()
{
	FILE *fd_spam;
	char line_buffer[64];
  char *ptr;
  
	spam_entries = use_spamfilter = 0;

	if(!(fd_spam = fopen(CFGPATH SPMFILE, "r")))
	{
		slog ? syslog(LOG_DAEMON | LOG_INFO, "no Spamlist found, Filter disabled") : printf("TuxMailD <no Spamlist found, Filter disabled>\n");

		return;
	}
	else
	{
		memset(spamfilter, 0, sizeof(spamfilter));

		while(fgets(line_buffer, sizeof(line_buffer), fd_spam) && spam_entries < 100)
		{
			if(sscanf(line_buffer, "%s", spamfilter[spam_entries].address) == 1)
			{
				// for prefix ! or # or & take the whole line, not just the first string
				if((line_buffer[0] == '!') || (line_buffer[0] == '#') || (line_buffer[0] == '&'))
				{
					strcpy(spamfilter[spam_entries].address,line_buffer);
					if((ptr = strchr(spamfilter[spam_entries].address, '\r')) || (ptr = strchr(spamfilter[spam_entries].address, '\n')))
					{
						*ptr = 0;
					}						
				}

				slog ? syslog(LOG_DAEMON | LOG_INFO, "Spamlist Filter <%s>>", spamfilter[spam_entries].address) : printf("TuxMailD <Spamlist Filter <%s>>\n", spamfilter[spam_entries].address);

				spam_entries++;				
			}
		}

		if(spam_entries)
		{
			use_spamfilter = 1;

			slog ? syslog(LOG_DAEMON | LOG_INFO, "Spamlist contains %d Entries, Filter enabled", spam_entries) : printf("TuxMailD <Spamlist contains %d Entries, Filter enabled>\n", spam_entries);
		}
		else 
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "empty Spamlist, Filter disabled") : printf("TuxMailD <empty Spamlist, Filter disabled>\n");
		}

		fclose(fd_spam);
	}
}

/******************************************************************************
 * InterfaceThread
 ******************************************************************************/

void *InterfaceThread(void *arg)
{
	int fd_sock, fd_conn;
	struct sockaddr_un srvaddr;
	socklen_t addrlen;
	char command;
	char mailsend;
	char mailcmd[86];
	int mailidx;

	// setup connection

		unlink(SCKFILE);

		srvaddr.sun_family = AF_UNIX;
		strcpy(srvaddr.sun_path, SCKFILE);
		addrlen = sizeof(srvaddr.sun_family) + strlen(srvaddr.sun_path);

		if((fd_sock = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "Interface: socket() failed") : printf("TuxMailD <Interface: socket() failed>\n");

			return 0;
		}

		if(bind(fd_sock, (struct sockaddr*)&srvaddr, addrlen) == -1)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "Interface: bind() failed") : printf("TuxMailD <Interface: bind() failed>\n");

			return 0;
		}

		if(listen(fd_sock, 0) == -1)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "Interface: listen() failed") : printf("TuxMailD <Interface: listen() failed>\n");

			return 0;
		}

	// communication loop

		while(1)
		{
			if((fd_conn = accept(fd_sock, (struct sockaddr*)&srvaddr, &addrlen)) == -1)
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "Interface: accept() failed") : printf("TuxMailD <Interface: accept() failed>\n");

				continue;
			}

			recv(fd_conn, &command, 1, 0);

			switch(command)
			{
				case 'G':

					send(fd_conn, &online, 1, 0);

					break;

				case 'S':

					recv(fd_conn, &online, 1, 0);

					if( online )
					{
						kill(pid, SIGUSR1);
					}
					else
					{
						kill(pid, SIGUSR2);
					}

					break;

				case 'R':

					slog ? syslog(LOG_DAEMON | LOG_INFO, "update") : printf("TuxMailD <update>\n");
					if(!ReadConf())
					{
						intervall = 0;
					}

				case 'L':

					ReadSpamList();

					break;

				case 'M':

					recv(fd_conn, &mailcmd, 85, 0);
					mailidx = mailcmd[0] - '0';
					mailread = 0;

					if (!inPOPCmd)
					{
						inPOPCmd = 1;
						mailread = SaveMail(mailidx, &mailcmd[5]);
						inPOPCmd = 0;
					}

					send(fd_conn, &mailread, 1, 0);

					break;

				case 'W':

					recv(fd_conn, &mailcmd, 1, 0);

					mailsend = SendMail(mailcmd[0] - '0');

					send(fd_conn, &mailsend, 1, 0);

					break;

				case 'V':

					send(fd_conn, &versioninfo, 12, 0);
			}

			close(fd_conn);
		}

	return 0;
}

/******************************************************************************
 * EncodeBase64
 ******************************************************************************/

void EncodeBase64(char *decodedstring, int decodedlen)
{
	char encodingtable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int src_index, dst_index;

	memset(encodedstring, 0, sizeof(encodedstring));

	for(src_index = dst_index = 0; src_index < decodedlen; src_index += 3, dst_index += 4)
	{
		encodedstring[0 + dst_index] = encodingtable[decodedstring[src_index] >> 2];
		encodedstring[1 + dst_index] = encodingtable[(decodedstring[src_index] & 3) << 4 | decodedstring[1 + src_index] >> 4];
		encodedstring[2 + dst_index] = encodingtable[(decodedstring[1 + src_index] & 15) << 2 | decodedstring[2 + src_index] >> 6];
		encodedstring[3 + dst_index] = encodingtable[decodedstring[2 + src_index] & 63];
	}

	if(decodedlen % 3)
	{
		switch(3 - decodedlen%3)
		{
			case 2:

				encodedstring[strlen(encodedstring) - 2] = '=';

			case 1:

				encodedstring[strlen(encodedstring) - 1] = '=';
		}
	}
}

/******************************************************************************
 * DecodeBase64
 ******************************************************************************/

void DecodeBase64(char *encodedstring, int encodedlen)
{
	int src_index, dst_index;
	char decodingtable[] = {62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51};

	memset(decodedstring, 0, sizeof(decodedstring));

	for(src_index = dst_index = 0; src_index < encodedlen; src_index += 4, dst_index += 3)
	{
		decodedstring[dst_index] = decodingtable[encodedstring[src_index] - 43] << 2 | ((decodingtable[encodedstring[1 + src_index] - 43] >> 4) & 3);

		if(encodedstring[2 + src_index] == '=')
		{
			break;
		}

		decodedstring[1 + dst_index] = decodingtable[encodedstring[1 + src_index] - 43] << 4 | ((decodingtable[encodedstring[2 + src_index] - 43] >> 2) & 15);

		if(encodedstring[3 + src_index] == '=')
		{
			break;
		}

		decodedstring[2 + dst_index] = decodingtable[encodedstring[2 + src_index] - 43] << 6 | decodingtable[encodedstring[3 + src_index] - 43];
	}

	memcpy(&header[stringindex], decodedstring, strlen(decodedstring));

	stringindex += strlen(decodedstring);
}

/******************************************************************************
 * DecodeQuotedPrintable
 ******************************************************************************/

void DecodeQuotedPrintable(char *encodedstring, int encodedlen)
{
	int src_index = 0, dst_index = 0;

	memset(decodedstring, 0, sizeof(decodedstring));

	while(src_index < encodedlen)
	{
		if(encodedstring[src_index] == '_')
		{
			decodedstring[dst_index++] = ' ';
		}
		else if(encodedstring[src_index] == '=')
		{
			int value;

			sscanf(&encodedstring[++src_index], "%2X", &value);

			src_index++;

			sprintf(&decodedstring[dst_index++], "%c", value);
		}
		else
		{
			decodedstring[dst_index++] = encodedstring[src_index];
		}

		src_index++;
	}

	memcpy(&header[stringindex], decodedstring, strlen(decodedstring));

	stringindex += strlen(decodedstring);
}

/******************************************************************************
 * DecodeHeader
 ******************************************************************************/
 
int DecodeHeader(char *encodedstring)
{
	char *ptrS, *ptrE;

	if((ptrS = strstr(encodedstring, "?B?")))
	{
		ptrS += 3;

		if((ptrE = strstr(ptrS, "?=")))
		{
			DecodeBase64(ptrS, ptrE - ptrS);

			return ptrE+2 - encodedstring;
		}
	}
	else if((ptrS = strstr(encodedstring, "?Q?")))
	{
		ptrS += 3;

		if((ptrE = strstr(ptrS, "?=")))
		{
			DecodeQuotedPrintable(ptrS, ptrE - ptrS);

			return ptrE+2 - encodedstring;
		}
	}

	return 1;
}

/******************************************************************************
 * void AddChar2Mail( char c)
 *
 * output whole words in text-only mode
 ******************************************************************************/

void AddChar2Mail( char c)
{
	if (c == '\0')
	{
	    return;
	}

	if ((c != ' ') && (c != '\n') && (c != 10) && (c != '.') && (c != ':') && (c != '-'))
	{
		if ((nCharInLine + nCharInWord + nStartSpalte) > cnRAND)
		{
			if (nCharInWord < cnMaxWordLen)
			{
				fputc('\n', fd_mail);

				if (nStartSpalte)
				{
					char i;

					for (i = 0; i < nStartSpalte; i++)
					{
						fputc(' ', fd_mail);
					}
				}

				nCharInLine=0;	
				sWord[nCharInWord++]=c;
			}
			else
			{
				sWord[nCharInWord++]='\n';

				if (nStartSpalte)
				{
					char i;

					for (i = 0; i < nStartSpalte; i++)
					{
						sWord[nCharInWord++] = ' ';
					}
				}

				sWord[nCharInWord] = 0;
				fputs(sWord, fd_mail);
				sWord[0] = c;
				nCharInWord = 1;
				nCharInLine = 0;							
			}
		} 
		else
		{
		    sWord[nCharInWord++] = c;
		}
	}
	else 
	{
		sWord[nCharInWord++] = c;
		sWord[nCharInWord] = 0;
		nCharInLine += nCharInWord;
		fputs(sWord, fd_mail);
		nCharInWord = 0;

		if ((c == 10) || (c == '\n')) 
		{	
			nCharInLine = 0;

			if (nStartSpalte)
			{
				char i;

				for (i = 0; i < nStartSpalte; i++)
				{
					fputc(' ', fd_mail);
				}
			}
		}
	}
}

/******************************************************************************
 * void doOneChar (char c)
 *
 * before we print a character we wait for a whole word/tag
 ******************************************************************************/

void doOneChar( char c )
{
	char  sHack[4];
	bool  fDo;

//	printf("N: (%c:%u) word:%u line:%u bytes:%lu\r\n",c,c,nCharInWord,nCharInLine,nRead);
	switch(state) 
	{
		// normal, not in a tag, translation or special char    	
		case cNorm :
           		switch (c) 
           		{
           			// first check if a special char, a tag or a translation starts
              			case '<' : 	state = cInTag; 
              					nIn = 0; 
              					break;
              					
              			case '=' : 	sWord[nCharInWord++] = c; 
              					state = cTrans; 
              					nTr = 0; 
              					break;
              					
              			case '&' : 	if(fHtml) 
              					{ 
              						state = cSond;  
              						nSo = 0; 
              						break; 
              					}
              					// if not in HTML mode fall-through to default handling
              					
              			default  : 	fDo = 0;

                         			if( !fPre ) 
                         			{
                         				if(fHtml)
                         				{
                         					// we can do some conversions
                            					if( c == '\t' )
                            					{
                            						c = ' ';
                            					}
                            					if( c == 10 )
                            					{
                            						c = '\n';
                            					}
                            					if( c == '\n' ) 
                            					{
									if( cLast != ' ' )
									{
										c = ' ';
									}
									else
									{
										fDo = 0;
									}
                        					}
                        				}

                        				AddChar2Mail(c);
                        			}

						cLast = c;
			} //switch
           		break; // case cNorm 
	
		// normal, not in a tag, translation or special char    	
		case cTrans :
			{
//				printf("U: (%c:%u) nTr:%u\r\n",c,c,nTr);
				if(nTr == 0)
				{
					sWord[nCharInWord++] = c;
					nTr++;
				}
				else
				{
					char c1 = sWord[nCharInWord - 1];
					sWord[nCharInWord++] = c;
					nTr++;
					char i;
					char* ptable = ttable;

					for (i = 0; i < ttsize; i++)
					{
						char t1 = *ptable++;
						char t2 = *ptable++;

						if(( t1 == c1 ) && ( t2 == c ))
						{
							nCharInWord -= 3;
							state = cNorm;

							if (*ptable) 
							{
								AddChar2Mail(*ptable);
							}
							break;
						}

						ptable++;
					}

					state = cNorm;	
				}
			} break;
		
		// check for html-tag
		case cInTag:
			nIn++;
		
//			printf("T: (%c:%u) In:%u sSond:%s\r\n",c,c,nIn,sSond);

			// for the first tag prepare variables
			if( nIn == 1 )
			{
				sSond[0] = '\0'; 
				nStrich = 0; 
			}

			// check if it is a html-comment
			if( (nIn == 4) && (!strcmp(sSond, "!--")) ) 
			{
				state = cInComment;
				nHyp = 0;
			}
			
			
			if( (c == '>') || (nIn >= sizeof(sSond)) ) 
			{
				char *pc, sArgs[400];

				// search for the first SPACE
				pc = strstr(sSond," ");
				if( pc != NULL ) 
				{
					*pc = '\0';
					strcpy(sArgs, ++pc);
				}

				if( !strcmp(sSond, "HTML"))		
				{ 
					strcpy(sSond, "\n"); 
					fHtml = 1; 
				}
				else if( !strcmp(sSond, "/HTML"))	
				{ 
					strcpy(sSond, "\n"); 
					fHtml = 0; 
				}
				else if( !strcmp(sSond, "BR") )
				{
					strcpy(sSond, "\n");
				}
				else if( !strcmp(sSond, "P") )		
				{
					strcpy(sSond, "\n\n");
				}
				else if( !strcmp(sSond, "LI") )		
				{
					strcpy(sSond, "\n* ");
				}
				else if( !strcmp(sSond, "/UL") )	
				{
					strcpy(sSond, "\n");
				}
				else if( !strcmp(sSond, "/OL") )	
				{
					strcpy(sSond, "\n");
				}
				else if( !strcmp(sSond, "DL") )		
				{
					strcpy(sSond, "\n");
				}
				else if( !strcmp(sSond, "/DL") )	
				{ 
					strcpy(sSond, "\n"); 
					nStartSpalte = 0; 
				}
				else if( !strcmp(sSond, "DT") )		
				{ 
					strcpy(sSond, "\n* "); 
					nStartSpalte = 0; 
				}
				else if( !strcmp(sSond, "DD") )		
				{ 
					strcpy(sSond, "\n "); 
					nStartSpalte = 8; 
				}
				else if( !strcmp(sSond, "PRE"))		
				{ 
					strcpy(sSond, "\n"); 
					fPre = 1; 
				}
				else if( !strcmp(sSond, "/PRE"))	
				{ 
					strcpy(sSond, "\n"); 
					fPre = 0; 
				}
				else if( !strcmp(sSond, "TR") )		
				{ 
					strcpy(sSond, "\n"); 
					nStartSpalte = 0;
				}
				else if( !strcmp(sSond, "TD") )		
				{ 
					strcpy(sSond, "  "); 
				}
				else if( !strcmp(sSond, "TABLE") )	
				{ 
					strcpy(sSond, "\n"); 
					nStartSpalte = 4;
				}
				else if( !strcmp(sSond, "/TABLE") )	
				{ 
					strcpy(sSond, "\n"); 
					nStartSpalte = 0;
				}
				else if( !strcmp(sSond, "A") ) 
				{ 
					// href= analysieren 
					// test auf # 
					strcpy(sRef, sArgs); 
					strcpy(sSond, "");
				}
				else if( !strcmp(sSond, "/A") ) 
				{ 
					//  suchen von sRef im Speicher 
					//  nRef,sRef 
					// sprintf(sSond," [%d] ",nRef); 
					sSond[0] = '\0';
					nRef++;
					sRef[0] = '\0';
				}  
				else if( !strcmp(sSond, "HR" ) ) 
				{
					strcpy(sSond, "\n---------------------------------------------------------------------\n");
				}
				else if( !strcmp(sSond, "H1")
					|| !strcmp(sSond, "H2")
					|| !strcmp(sSond, "H3")
					|| !strcmp(sSond, "H4")
					|| !strcmp(sSond, "H5")
					|| !strcmp(sSond, "H6") ) 
				{
					strcpy(sSond, "\n\n");
				}
				else if( ((sSond[0] == '/') && (sSond[1] == 'H') && (sSond[2] >= '0')&& (sSond[2] <= '6'))
					|| !strcmp(sSond, "/TITLE") ) 
				{
					// Einen Strich unterm Titel 
					strcpy(sSond, "\n\n");
					nStrich = nCharInLine + 1;
				}
				else if( !strcmp(sSond, "/TD") )
				{ 
					int i;
					sSond[0] = '\0';
					for( i = 1; i <= (nCharInLine % 10); i++)
						strncat(sSond, " ", 1);
					nStartSpalte = nCharInLine + strlen(sSond);
				}
           			else 
				{
					// strstr(sArgs,"ALT=");
					if(!fHtml)
					{
						int i;		
						int iLen = strlen(sSond);
						state = cNorm;
						sSond[iLen++] = '>';
						sSond[iLen++] = '\0';
						AddChar2Mail('<');
						for (i = 0; i < iLen; i++)
						{
							if (sSond[i]) doOneChar(sSond[i]);
						}
					} 
					sSond[0] = '\0';
				}
//				printf("H: (%c:%u) In:%u spalte:%u\r\n",sSond[0],sSond[0],nIn,nStartSpalte);
				if(sSond[0] != '\0') 
				{
					writeFOut(sSond);
				}
				state = cNorm;
			} 
			else 
			{
				if(( c >= 'a' ) && ( c <= 'z' )) 
				{
					c -= ('a'-'A');
				}
				sHack[0] = c; 
				sHack[1] = '\0';
				strcat(sSond, sHack);
			}
			break; // InTag 

		// character conversion
		case cSond :
//			printf("S: (%c:%u) nSo:%u sSond:%s\r\n",c,c,nSo,sSond);

			nSo++;
			if( nSo == 1 )  
			{
				sSond[0] = '\0';
			}

			if( (c == '&') || (c == ' ') || (c == ';') || (nSo > 7) ) 
			{
				int i = 0;
				int fFound = 0;

				if( sSond[0] =='#' ) 
				{
					i = atoi( &sSond[1] );
					if( i == 153 ) strcpy(sSond, "(TM)");
					else 
					{
						sSond[0] = (char)i;
						sSond[1] = '\0';
					}
					fFound = 1;
				} 
				else 
				{
					fFound = 1;
					if( !strcmp(sSond, "lt"  ) )
					{
						strcpy( sSond, "<");
					}
					else if( !strcmp(sSond, "gt"  ) )  
					{
						strcpy( sSond, ">");
					}
					else if( !strcmp(sSond, "quot") )  
					{
						strcpy( sSond, "\"");
					}
					else if( !strcmp(sSond, "amp" ) )  
					{
						strcpy( sSond, "&");
					}
					else if( !strcmp(sSond, "nbsp") )  
					{
						strcpy( sSond, " ");
					}
					else if( !strcmp(sSond, "copy") )  
					{
						strcpy( sSond, "(c)");
					}
					else if( !strcmp(sSond, "reg" ) )  
					{
						strcpy( sSond, "(R)");
					}
					else 
					{
						fFound = 0;
						for ( i = 0; i < szsize; i++) 
						{
							if( !strcmp(sSond, szTab[i]) ) 
							{
								sSond[0] = (char)i+192;
								sSond[1] = '\0';
								fFound = 1;
								break;
							}
						}
					}
				}

				state = cNorm;

				if( fFound ) 
				{
					int iLen = strlen(sSond);
					for (i = 0; i < iLen; i++)
					{
						if ((sSond[i]) && (sSond[i] != '&'))
						{
							doOneChar(sSond[i]);
						}
						else
						{
							AddChar2Mail(sSond[i]);
						}	
					}
				} 
				else 
				{ // we didn't find a conversion
					char sTmp[300];

					sprintf(sTmp,"&%s%c",sSond,c);					
					int iLen = strlen(sTmp);
					for( i = 0 ; i < iLen; i++)
					{
						if(( sTmp[i] ) && ( sTmp[i] != '&' ))
						{
							doOneChar(sTmp[i]);
						}
						else
						{
							AddChar2Mail(sTmp[i]);
						}
					}
				}
			} 
			else
			{
				sHack[0] = c; 
				sHack[1] = '\0';
				strcat(sSond, sHack);
			}
			break;

		case cInComment:
			if( (nHyp == 2) & (c == '>') ) 
			{
				state = cNorm;
			} 
			else 
			{
				if( c == '-' )
				{
					nHyp++;
				}
				else            
				{
					nHyp = 0;
				}
			}
			break;

		}  //case
}

/******************************************************************************
 * void writeFOut (char* s)
 ******************************************************************************/

void writeFOut( char *s)
{
	char sSond[255];

	// paint a line ?
	if( s[0] == '\n' ) 
	{
		nCharInLine = 0;
		if( nStrich > 0 ) 
		{
			/* Eine Zeile mit Strichen erzeugen */
			if( nStrich > 80 )  nStrich = 75;
			{ 
				int i=0; char* p = sSond;
				for(; i < nStrich; i++, p++)
				{
					*p = '-';
				}
			}
			sSond[nStrich] = '\0';
			fprintf(fd_mail, "\n%s", sSond);
			nLine++;
//			nWrite += nStrich;
			nStrich = 0;
		}
		if( !strcmp(s, "\n" ) ) 
		{
			nCRLF++;
		}
	} 
	else 
	{
		nCRLF = 0;
	}

	if( nCRLF < 3 ) 
	{ 
		int i,l;
		l = strlen(s);
		for( i = 0 ; i < l ; i++)
		{
//			s[i] = iso2ascii[(int)s[i]];
			if( s[i] == '\n' )
			{
				nLine++;
			}
		}
		fputs(s, fd_mail);
	}

	// Einrueckung durchfuehren 
	if( s[0] == '\n' ) 
	{
		if( nStartSpalte >0 ) 
		{
			int i;
			for( i = 0 ; i < nStartSpalte; i++)
			{
				fputc(' ', fd_mail);
			}
		}
	}
}

/******************************************************************************
 * SendPOPCommand (0=fail, 1=done)
 ******************************************************************************/

int SendPOPCommand(int command, char *param)
{
	struct hostent *server;
	struct sockaddr_in SockAddr;
	FILE *fd_log;
	char send_buffer[128], recv_buffer[8192], month[4];
	char *ptr, *ptr1, *ptr2;
	int loop, day, hour, minute;
	int linelen;
	char* portpos;

	// build commandstring

		switch(command)
		{
			case INIT:

				strcpy(send_buffer,param);
				// check if port is given
				portpos = strchr( send_buffer, ':');
				if( portpos )
				{
					*portpos = '\0';
					portpos++;
				}
						
				if(!(server = gethostbyname(send_buffer)))
				{
				    slog ? syslog(LOG_DAEMON | LOG_INFO, "could not resolve Host \"%s\", will try again in 10s", send_buffer) : printf("TuxMailD <could not resolve Host \"%s\", will try again in 10s>\n", send_buffer);

				    sleep(10);	/* give some routers a second chance */

				    if(!(server = gethostbyname(send_buffer)))
				    {
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not resolve Host \"%s\"", send_buffer) : printf("TuxMailD <could not resolve Host \"%s\">\n", send_buffer);

					return 0;
				    }
				}

				if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not create Socket") : printf("TuxMailD <could not create Socket>\n");

					return 0;
				}

				SockAddr.sin_family = AF_INET;
				if( portpos )
				{
					SockAddr.sin_port = htons(atoi(portpos));
				}
				else
				{
					SockAddr.sin_port = htons(110);
				}
				SockAddr.sin_addr = *(struct in_addr*) server->h_addr_list[0];

				if(connect(sock, (struct sockaddr*)&SockAddr, sizeof(SockAddr)))
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not connect to Host \"%s\"", param) : printf("TuxMailD <could not connect to Host \"%s\">\n", param);

					close(sock);

					return 0;
				}

				break;

			case USER:

				sprintf(send_buffer, "USER %s\r\n", param);

				break;

			case PASS:

				sprintf(send_buffer, "PASS %s\r\n", param);

				break;

			case STAT:

				sprintf(send_buffer, "STAT\r\n");

				break;

			case UIDL:

				sprintf(send_buffer, "UIDL %s\r\n", param);

				break;

			case TOP:

				sprintf(send_buffer, "TOP %s 0\r\n", param);

				break;

			case RETR:

				sprintf(send_buffer, "TOP %s 5000\r\n", param);

				break;

			case DELE:

				sprintf(send_buffer, "DELE %s\r\n", param);

				break;

			case RSET:

				sprintf(send_buffer, "RSET\r\n");

				break;

			case QUIT:
				sprintf(send_buffer, "QUIT\r\n");
		}

	// send command to server

		if(command != INIT)
		{
			if(logging == 'Y')
			{
				if((fd_log = fopen(LOGFILE, "a")))
				{
					fprintf(fd_log, "POP3 <- %s", send_buffer);

					fclose(fd_log);
				}
				else
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not log POP3-Command") : printf("TuxMailD <could not log POP3-Command>\n");
				}
			}

			send(sock, send_buffer, strlen(send_buffer), 0);
		}

		// get server response

		stringindex = 0;
		linelen = 0;
    		state = cNorm;
    		nStrich = 0;
    		nStartSpalte = 1;
    		nCharInLine = 0;
    		nCharInWord = 0;
    		cLast = 0;
    		nRead = nWrite = 0;
    		fPre = 0;
    		fHtml = 0;
    		
		if(command == RETR)
		{
   		while(recv(sock, &recv_buffer[3], 1, 0) > 0)
			{
				if((nRead) && (nRead < 75000)) 
				{
					nRead++;
					doOneChar( recv_buffer[3] );
				}

				// scan for header-end
  			if(!nRead && recv_buffer[3] == '\n' && recv_buffer[1] == '\n')
  			{
					nRead++;
				}
			
				// this is normally the end of an email
				if(recv_buffer[3] == '\n' && recv_buffer[1] == 46 && recv_buffer[0] == '\n')
				{
					strcpy(recv_buffer, "+OK\r\n");
					break;
				}

				recv_buffer[0] = recv_buffer[1];
				recv_buffer[1] = recv_buffer[2];
				recv_buffer[2] = recv_buffer[3];
			}
		}
		else
		{
			while(recv(sock, &recv_buffer[stringindex], 1, 0) > 0)
			{
				if(command == TOP)
				{
					if((recv_buffer[stringindex] == '\n') || (recv_buffer[stringindex] == '\r'))
					{
						linelen = 0;
					}
					linelen ++;
					if(recv_buffer[stringindex] == '\n' && recv_buffer[stringindex - 3] == '\n')
					{
						recv_buffer[stringindex+1] = '\0';

						break;
					}
				}
				else if(recv_buffer[stringindex] == '\n')
				{
					recv_buffer[stringindex+1] = '\0';

					break;
				}

				if(stringindex < sizeof(recv_buffer) - 4)
				{
					if((linelen < 100) || (command != TOP))		// restrict linelen
					{
						stringindex++;
					} 
				}
				else
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "Buffer Overflow") : printf("TuxMailD <Buffer Overflow>\n");
					recv_buffer[stringindex + 1] = '\0';
					break;
				}
			}
		}

		if(logging == 'Y')
		{
			if((fd_log = fopen(LOGFILE, "a")))
			{
				fprintf(fd_log, "POP3 -> %s", recv_buffer);

				fclose(fd_log);
			}
			else
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "could not log POP3-Response") : printf("TuxMailD <could not log POP3-Response>\n");
			}
		}

	// check server response

		if(!strncmp(recv_buffer, "+OK", 3))
		{
			switch(command)
			{
				case STAT:

					sscanf(recv_buffer, "+OK %d", &messages);

					break;

				case UIDL:

					sscanf(recv_buffer, "+OK %*d %s", uid);

					break;

				case TOP:

					stringindex = 0;
					headersize = strlen(recv_buffer);
					memset(header, 0, sizeof(header));

					if((ptr = strstr(recv_buffer, "\nDate:")))
					{
						ptr += 6;

						while(*ptr == ' ')
						{
							ptr++;
						}

						if(*ptr < '0' || *ptr > '9')
						{
							sscanf(ptr, "%*s %d %s %*d %d:%d", &day, &month[0], &hour, &minute);
						}
						else
						{
							sscanf(ptr, "%d %s %*d %d:%d", &day, &month[0], &hour, &minute);
						}

						sprintf(header, "%.2d.%.3s|%.2d:%.2d|", day, month, hour, minute);
						stringindex += 13;
					}
					else
					{
						memcpy(header, "??.???|??:??|", 13);
						stringindex += 13;
					}
		
					if((ptr = strstr(recv_buffer, "\nFrom:")))
					{
						ptr += 6;

						while(*ptr == ' ')
						{
							ptr++;
						}

						ptr1 = &header[stringindex];

						while(*ptr != '\r')
						{
							if(*ptr == '=' && *(ptr + 1) == '?')
							{
								ptr += DecodeHeader(ptr);

								/* skip space(s) between encoded words */

								ptr2 = ptr;

								while(*ptr2 == ' ')
								{
									ptr2++;
								}

								if(*ptr2 == '?' && *(ptr2 + 1) == '=')
								{
									ptr = ptr2;
								}
							}
							else
							{
								memcpy(&header[stringindex++], ptr++, 1);
							}
						}

						if(use_spamfilter)
						{
							spam_detected = 0;

							for(loop = 0; loop < spam_entries; loop++)
							{
								if(strstr(ptr1, spamfilter[loop].address))
								{
									slog ? syslog(LOG_DAEMON | LOG_INFO, "Spamfilter active, delete Mail from \"%s\"", ptr1) : printf("TuxMailD <Spamfilter active, delete Mail from \"%s\">\n", ptr1);

									spam_detected = 1;

									break;
								}
							}
						}

						header[stringindex++] = '|';
					}
					else
					{
						memcpy(&header[stringindex], "-?-|", 4);
						stringindex += 4;
					}

					if((ptr = strstr(recv_buffer, "\nSubject:")))
					{
						ptr += 9;

						while(*ptr == ' ')
						{
							ptr++;
						}

						ptr1 = &header[stringindex];

						while(*ptr != '\r')
						{
							if(*ptr == '=' && *(ptr + 1) == '?')
							{
								ptr += DecodeHeader(ptr);

								/* skip space(s) between encoded words */

								ptr2 = ptr;

								while(*ptr2 == ' ')
								{
									ptr2++;
								}

								if(*ptr2 == '?' && *(ptr2 + 1) == '=')
								{
									ptr = ptr2;
								}
							}
							else
							{
								memcpy(&header[stringindex++], ptr++, 1);
							}
						}

						if(use_spamfilter)
						{
							for(loop = 0; loop < spam_entries; loop++)
							{
								// check if we should look for a subject
								if(((spamfilter[loop].address[0]=='#')	&& (strstr(ptr1, &spamfilter[loop].address[1]))) ||
									 ((spamfilter[loop].address[0]=='!')	&& (!strcmp(ptr1, &spamfilter[loop].address[1]))))
								{									
									slog ? syslog(LOG_DAEMON | LOG_INFO, "Spamfilter active, delete Mail with Subj. \"%s\"", ptr1) : printf("TuxMailD <Spamfilter active, delete Mail with Subj. \"%s\">\n", ptr1);

									spam_detected = 1;

									break;
								}
							}						
						}

						header[stringindex++] = '|';
					}
					else
					{
						memcpy(&header[stringindex], "-?-|", 4);
						stringindex += 4;
					}

					header[stringindex] = '\0';

					break;
					
				case DELE:
				case RETR:
					break;

				case QUIT:

					close(sock);
			}
		}
		else
		{
			if((ptr = strchr(recv_buffer, '\r')))
			{
				*ptr = 0;
			}

			slog ? syslog(LOG_DAEMON | LOG_INFO, "Server Error (%s)", recv_buffer + 5) : printf("TuxMailD <Server Error (%s)>\n", recv_buffer + 5);

			close(sock);

			return 0;
		}

	return 1;
}

/******************************************************************************
 * SendIMAPCommand (0=fail, 1=done)
 ******************************************************************************/

int SendIMAPCommand(int command, char *param, char *param2)
{
	struct hostent *server;
	struct sockaddr_in SockAddr;
	FILE *fd_log;
	char send_buffer[128], recv_buffer[8192], month[4];
	char *ptr, *ptr1, *ptr2;
	int loop, day, hour, minute;
	int linelen;
	char* portpos;
	
	// build commandstring

		switch(command)
		{
			case INIT:

				strcpy(send_buffer,param);
				// check if port is given
				portpos = strchr( send_buffer, ':');
				if( portpos )
				{
					*portpos = '\0';
					portpos++;
				}
						
				if(!(server = gethostbyname(send_buffer)))
				{
				    slog ? syslog(LOG_DAEMON | LOG_INFO, "could not resolve Host \"%s\", will try again in 10s", param) : printf("TuxMailD <could not resolve Host \"%s\", will try again in 10s>\n", param);

				    sleep(10);	/* give some routers a second chance */

				    if(!(server = gethostbyname(send_buffer)))
				    {
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not resolve Host \"%s\"", send_buffer) : printf("TuxMailD <could not resolve Host \"%s\">\n", send_buffer);

					return 0;
				    }
				}

				if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not create Socket") : printf("TuxMailD <could not create Socket>\n");

					return 0;
				}

				SockAddr.sin_family = AF_INET;
				if( portpos )
				{
					SockAddr.sin_port = htons(atoi(portpos));
				}
				else
				{
					SockAddr.sin_port = htons(143);
				}
				SockAddr.sin_addr = *(struct in_addr*) server->h_addr_list[0];

				if(connect(sock, (struct sockaddr*)&SockAddr, sizeof(SockAddr)))
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not connect to Host \"%s\"", param) : printf("TuxMailD <could not connect to Host \"%s\">\n", param);

					close(sock);

					return 0;
				}

				break;

			case LOGIN:

				sprintf(send_buffer, "? LOGIN %s %s\r\n",param, param2);

				break;

			case SELECT:

				if( param[0]=='\0' )
				{
					sprintf(send_buffer, "? SELECT INBOX\r\n");
				}
				else
				{
					sprintf(send_buffer, "? SELECT %s\r\n", param);
				}

				break;

			case UIDL:

				sprintf(send_buffer, "? FETCH %s UID\r\n",param);

				break;

			case FLAGS:

				sprintf(send_buffer, "? FETCH %s FLAGS\r\n",param);

				break;

			case UNSEEN:

				sprintf(send_buffer, "? STORE %s -FLAGS (\\Seen)\r\n",param);

				break;

			case FETCH:

				sprintf(send_buffer, "? FETCH %s (FLAGS BODY[HEADER.FIELDS (DATE FROM SUBJECT)])\r\n",param);

				break;

			case DELE:

				sprintf(send_buffer, "? STORE %s +FLAGS (\\Deleted)\r\n",param);

				break;

			case EXPUNGE:

				sprintf(send_buffer, "? EXPUNGE\r\n");

				break;

			case RETR:
				
				sprintf(send_buffer, "? FETCH %s BODY[TEXT]<0.75000>\r\n",param);

				break;

			case CLOSE:

				sprintf(send_buffer, "? CLOSE\r\n");

				break;

			case LOGOUT:

				sprintf(send_buffer, "? LOGOUT\r\n");

				break;
		}

	// send command to server

		if(command != INIT)
		{
			if(logging == 'Y')
			{
				if((fd_log = fopen(LOGFILE, "a")))
				{
					fprintf(fd_log, "IMAP <- %s", send_buffer);

					fclose(fd_log);
				}
				else
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not log IMAP-Command") : printf("TuxMailD <could not log IMAP-Command>\n");
				}
			}

			send(sock, send_buffer, strlen(send_buffer), 0);
		}

		// get server response

		stringindex = 0;
		linelen = 0;
   		state = cNorm;
   		nStrich = 0;
   		nStartSpalte = 1;
   		nCharInLine = 0;
   		nCharInWord = 0;
   		cLast = 0;
   		nRead = nWrite = 0;
   		fPre = 0;
   		fHtml = 0;
    		
		if(command == RETR)
		{
	   		while(recv(sock, &recv_buffer[stringindex], 1, 0) > 0)
			{
				// read line by line
				if((nRead) && (nRead < 75000)) 
				{
					nRead++;
					doOneChar( recv_buffer[stringindex] );
				}
				
				if(recv_buffer[stringindex] == '\n')
				{
					if( !nRead )
					{
						ptr2 = strstr(recv_buffer, "BODY[TEXT]<0> ");					
						sscanf(ptr2, "BODY[TEXT]<0> {%d", &linelen);
						stringindex=0;
						nRead++;
					}
					else
					{
						recv_buffer[stringindex + 1] = '\0';
//						printf("linelen: read:%d max:%d <%s>",nRead,linelen,recv_buffer);
						if( nRead >= linelen)
						{
							if(!strncmp(recv_buffer, "? OK", 4))
							{
								return 1;
							}
							if(!strncmp(recv_buffer, "? NO", 4))
							{
								close(sock);
								return 0;
							}
						}
						nRead++;
						stringindex=0;
					}
				}
				else
				{
					if(stringindex < sizeof(recv_buffer) - 4)
					{	
						stringindex++;
					}
					else
					{
						slog ? syslog(LOG_DAEMON | LOG_INFO, "Buffer Overflow") : printf("TuxMailD <Buffer Overflow>\n");
						recv_buffer[stringindex + 1] = '\0';
						break;
					}
				}
			}
		}
		else
		{
			while(recv(sock, &recv_buffer[stringindex], 1, 0) > 0)
			{
				// read line by line and end if leading character hast been '?' or after one line after INIT
				if(recv_buffer[stringindex] == '\n')
				{
					if((recv_buffer[nRead] == '?') || (command == INIT))
					{
						recv_buffer[stringindex + 1] = '\0';
						break;
					}
					else
					{
						nRead=stringindex + 1;
					}
				}
	
				if(stringindex < sizeof(recv_buffer) - 4)
				{
					stringindex++;
				}
				else
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "Buffer Overflow") : printf("TuxMailD <Buffer Overflow>\n");
					recv_buffer[stringindex + 1] = '\0';
					break;
				}
			}
		}

		if(logging == 'Y')
		{
			if((fd_log = fopen(LOGFILE, "a")))
			{
				fprintf(fd_log, "IMAP -> %s", recv_buffer);

				fclose(fd_log);
			}
			else
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "could not log IMAP-Response") : printf("TuxMailD <could not log IMAP-Response>\n");
			}
		}

	// check server response

//		if(!strncmp(recv_buffer, "? OK", 4))
//		{
			switch(command)
			{
				case INIT:
					
					if(!strncmp(recv_buffer, "* OK", 4))
					{
						return 1;
					}
					break;
					
				case LOGIN:
				case DELE:
				case CLOSE:
				case EXPUNGE:
				case UNSEEN:

					if(!strncmp(&recv_buffer[nRead], "? OK", 4))
					{
						return 1;
					}
					break;

				case SELECT:
					
					ptr1 = &recv_buffer[0];
					ptr2 = strstr(recv_buffer, "EXISTS");					
					do
					{
						ptr = strchr(ptr1,'\n');					
						if( ptr2 < ptr )
						{
							sscanf(ptr1, "* %d", &messages);
							break;
						}
						ptr1 = ptr + 1;
					} while( ptr );
			
					ptr2 = strstr(recv_buffer, "UIDVALIDITY");					
					sscanf(ptr2, "UIDVALIDITY %ld", &v_uid);

					if(!strncmp(&recv_buffer[nRead], "? OK", 4))
					{
						return 1;
					}
					break;

				case UIDL:

					ptr2 = strstr(recv_buffer, "UID");					
					sscanf(ptr2, "UID %ld", &m_uid);
					sprintf(uid,"%08lX%08lX",v_uid,m_uid);

					if(!strncmp(&recv_buffer[nRead], "? OK", 4))
					{
						return 1;
					}
					break;

				case FLAGS:

					// check if already seen
					if( strstr(recv_buffer, "\\Seen") != NULL )
					{
						*param2='S';
					}
					else
					{
						*param2='U';
					}					
					if( strstr(recv_buffer, "\\Deleted") != NULL )
					{
						*param2=0;
					}
					
					if(!strncmp(&recv_buffer[nRead], "? OK", 4))
					{
						return 1;
					}
					break;

				case FETCH:

					stringindex = 0;
					headersize = strlen(recv_buffer);
					memset(header, 0, sizeof(header));

					if((ptr = strstr(recv_buffer, "\nDate:")))
					{
						ptr += 6;

						while(*ptr == ' ')
						{
							ptr++;
						}

						if(*ptr < '0' || *ptr > '9')
						{
							sscanf(ptr, "%*s %d %s %*d %d:%d", &day, &month[0], &hour, &minute);
						}
						else
						{
							sscanf(ptr, "%d %s %*d %d:%d", &day, &month[0], &hour, &minute);
						}

						sprintf(header, "%.2d.%.3s|%.2d:%.2d|", day, month, hour, minute);
						stringindex += 13;
					}
					else
					{
						memcpy(header, "??.???|??:??|", 13);
						stringindex += 13;
					}
		
					if((ptr = strstr(recv_buffer, "\nFrom:")))
					{
						ptr += 6;

						while(*ptr == ' ')
						{
							ptr++;
						}

						ptr1 = &header[stringindex];

						while(*ptr != '\r')
						{
							if(*ptr == '=' && *(ptr + 1) == '?')
							{
								ptr += DecodeHeader(ptr);

								/* skip space(s) between encoded words */

								ptr2 = ptr;

								while(*ptr2 == ' ')
								{
									ptr2++;
								}

								if(*ptr2 == '?' && *(ptr2 + 1) == '=')
								{
									ptr = ptr2;
								}
							}
							else
							{
								memcpy(&header[stringindex++], ptr++, 1);
							}
						}

						if(use_spamfilter)
						{
							spam_detected = 0;

							for(loop = 0; loop < spam_entries; loop++)
							{
								if(strstr(ptr1, spamfilter[loop].address))
								{
									slog ? syslog(LOG_DAEMON | LOG_INFO, "Spamfilter active, delete Mail from \"%s\"", ptr1) : printf("TuxMailD <Spamfilter active, delete Mail from \"%s\">\n", ptr1);

									spam_detected = 1;

									break;
								}
							}
						}

						header[stringindex++] = '|';
					}
					else
					{
						memcpy(&header[stringindex], "-?-|", 4);
						stringindex += 4;
					}

					if((ptr = strstr(recv_buffer, "\nSubject:")))
					{
						ptr += 9;

						while(*ptr == ' ')
						{
							ptr++;
						}

						ptr1 = &header[stringindex];

						while(*ptr != '\r')
						{
							if(*ptr == '=' && *(ptr + 1) == '?')
							{
								ptr += DecodeHeader(ptr);

								/* skip space(s) between encoded words */

								ptr2 = ptr;

								while(*ptr2 == ' ')
								{
									ptr2++;
								}

								if(*ptr2 == '?' && *(ptr2 + 1) == '=')
								{
									ptr = ptr2;
								}
							}
							else
							{
								memcpy(&header[stringindex++], ptr++, 1);
							}
						}
						
						if(use_spamfilter)
						{
							for(loop = 0; loop < spam_entries; loop++)
							{
								// check if we should look for a subject
								if(((spamfilter[loop].address[0]=='#')	&& (strstr(ptr1, &spamfilter[loop].address[1]))) ||
									 ((spamfilter[loop].address[0]=='!')	&& (!strcmp(ptr1, &spamfilter[loop].address[1]))))
								{									
									slog ? syslog(LOG_DAEMON | LOG_INFO, "Spamfilter active, delete Mail with Subj. \"%s\"", ptr1) : printf("TuxMailD <Spamfilter active, delete Mail with Subj. \"%s\">\n", ptr1);

									spam_detected = 1;

									break;
								}
							}						
						}

						header[stringindex++] = '|';
					}
					else
					{
						memcpy(&header[stringindex], "-?-|", 4);
						stringindex += 4;
					}

					header[stringindex] = '\0';

					if(!strncmp(&recv_buffer[nRead], "? OK", 4))
					{
						return 1;
					}
					break;
					
				case LOGOUT:

					close(sock);
					return 1;
					
				default:
					slog ? syslog(LOG_DAEMON | LOG_INFO, "IMAP Server (%s)", recv_buffer) : printf("TuxMailD <IMAP Server (%s)>\n", recv_buffer);
				
			}

	slog ? syslog(LOG_DAEMON | LOG_INFO, "IMAP Server (%s)", recv_buffer) : printf("TuxMailD <IMAP Server (%s)>\n", recv_buffer);
	close(sock);
	return 0;
}

/******************************************************************************
 * SendSMTPCommand (0=fail, 1=done)
 ******************************************************************************/

int SendSMTPCommand(int command, char *param)
{
	FILE *fd_log;
	static int sock;
	struct hostent *server;
	struct sockaddr_in SockAddr;
	char send_buffer[128], recv_buffer[4096];
	bool cancel = false;
	char* portpos;

	// clear buffers

		memset(send_buffer, 0, sizeof(send_buffer));
		memset(recv_buffer, 0, sizeof(recv_buffer));

	// build commandstring

		switch(command)
		{
			case INIT:

				strcpy(send_buffer,param);
				// check if port is given
				portpos = strchr( send_buffer, ':');
				if( portpos )
				{
					*portpos = '\0';
					portpos++;
				}
						
				if(!(server = gethostbyname(send_buffer)))
				{
				    sleep(10);	/* give some routers a second chance */

				    if(!(server = gethostbyname(send_buffer)))
				    {
					return 0;
				    }
				}

				if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
				{
					return 0;
				}

				SockAddr.sin_family = AF_INET;
				if( portpos )
				{
					SockAddr.sin_port = htons(atoi(portpos));
				}
				else
				{
					SockAddr.sin_port = htons(25);
				}
				SockAddr.sin_addr = *(struct in_addr*) server->h_addr_list[0];

				if(connect(sock, (struct sockaddr*)&SockAddr, sizeof(SockAddr)))
				{
					close(sock);

					return 0;
				}

				break;

			case EHLO:

				sprintf(send_buffer, "EHLO %s\r\n", param);
				break;

			case AUTH:

				sprintf(send_buffer, "AUTH PLAIN %s\r\n", param);
				break;

			case MAIL:

				sprintf(send_buffer, "MAIL FROM: %s\r\n", param);
				break;

			case RCPT:

				sprintf(send_buffer, "RCPT TO: %s\r\n", param);
				break;

			case DATA1:

				sprintf(send_buffer, "DATA\r\n");
				break;

			case DATA2:

				sprintf(send_buffer, "%s\r\n", param);
				break;

			case DATA3:

				sprintf(send_buffer, ".\r\n");
				break;

			case QUIT:

				sprintf(send_buffer, "QUIT\r\n");
		}

	// send command to server

		if(command != INIT)
		{

			if(logging == 'Y')
			{
				if((fd_log = fopen(LOGFILE, "a")))
				{
					if(command == AUTH)
					{
						fprintf(fd_log, "SMTP <- be sure to edit the next line before you post it in a forum");
					}
					fprintf(fd_log, "SMTP <- %s", send_buffer);

					fclose(fd_log);
				}
				else
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not log SMTP-Command") : printf("TuxMailD <could not log SMTP-Command>\n");
				}
			}

			send(sock, &send_buffer, strlen(send_buffer), 0);
		}

	// get server response
	
	// wait a second to give some servers the chance to prepare the answer
		sleep(1);

		if(command == DATA2)
		{
			return 1;
		}

		recv(sock, &recv_buffer, sizeof(recv_buffer), 0);

		if(logging == 'Y')
		{
			if((fd_log = fopen(LOGFILE, "a")))
			{
				fprintf(fd_log, "SMTP -> %s", recv_buffer);

				fclose(fd_log);
			}
			else
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "could not log SMTP-Command") : printf("TuxMailD <could not log SMTP-Command>\n");
			}
		}

	// check server response

		switch(command)
		{
			case INIT:

				if(strncmp(recv_buffer, "220", 3))
				{
					cancel = true;
				}

				break;

			case AUTH:

				if(strncmp(recv_buffer, "235", 3))
				{
					cancel = true;
				}				

				break;

			case EHLO:
			case MAIL:
			case RCPT:
			case DATA3:

				if(strncmp(recv_buffer, "250", 3))
				{
					cancel = true;
				}

				break;

			case DATA1:

				if(strncmp(recv_buffer, "354", 3))
				{
					cancel = true;
				}

				break;

			case QUIT:

				close(sock);

				if(strncmp(recv_buffer, "221", 3))
				{
					return 0;
				}
		}

	// cancel operation?

		if(cancel)
		{
			sprintf(send_buffer, "QUIT\r\n");

			if(logging == 'Y')
			{
				if((fd_log = fopen(LOGFILE, "a")))
				{
					fprintf(fd_log, "SMTP <- %s", send_buffer);

					fclose(fd_log);
				}
				else
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not log SMTP-Command") : printf("TuxMailD <could not log SMTP-Command>\n");
				}
			}

			send(sock, &send_buffer, strlen(send_buffer), 0);

			memset(recv_buffer, 0, sizeof(recv_buffer));

			recv(sock, &recv_buffer, sizeof(recv_buffer), 0);

			if(logging == 'Y')
			{
				if((fd_log = fopen(LOGFILE, "a")))
				{
					fprintf(fd_log, "SMTP -> %s", recv_buffer);

					fclose(fd_log);
				}
				else
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not log SMTP-Command") : printf("TuxMailD <could not log SMTP-Command>\n");
				}
			}

			close(sock);

			return 0;
		}

	// success

		return 1;
}

/******************************************************************************
 * SendMail (0=fail, 1=done)
 ******************************************************************************/

int SendMail(int account)
{
	FILE *mail;
	char linebuffer[1024];

	// open mailfile

		if(!(mail = fopen(SMTPFILE, "r")))
		{
			return 0;
		}

	// send init

		memset(linebuffer, 0, sizeof(linebuffer));

		if(fgets(linebuffer, sizeof(linebuffer), mail))
		{
			linebuffer[strlen(linebuffer) - 1] = '\0';

			if(!SendSMTPCommand(INIT, linebuffer))
			{
				fclose(mail);

				return 0;
			}
		}

	// send ehlo

		memset(linebuffer, 0, sizeof(linebuffer));

		if(fgets(linebuffer, sizeof(linebuffer), mail))
		{
			linebuffer[strlen(linebuffer) - 1] = '\0';

			if(!SendSMTPCommand(EHLO, linebuffer))
			{
				fclose(mail);

				return 0;
			}
		}

		if(account_db[account].auth == 1 || account_db[account].auth == 2)
		{
	// send auth

			memset(decodedstring, 0, sizeof(decodedstring));
			if(account_db[account].auth == 1)
			{
				memcpy(decodedstring + 1, account_db[account].user, strlen(account_db[account].user));
				memcpy(decodedstring + 1 + strlen(account_db[account].user) + 1, account_db[account].pass, strlen(account_db[account].pass));
				EncodeBase64(decodedstring, strlen(account_db[account].user) + strlen(account_db[account].pass) + 2);
			}
			if(account_db[account].auth == 2)
			{
				memcpy(decodedstring + 1, account_db[account].suser, strlen(account_db[account].suser));
				memcpy(decodedstring + 1 + strlen(account_db[account].suser) + 1, account_db[account].spass, strlen(account_db[account].spass));
				EncodeBase64(decodedstring, strlen(account_db[account].suser) + strlen(account_db[account].spass) + 2);
			}

			if(!SendSMTPCommand(AUTH, encodedstring))
			{
				fclose(mail);

				return 0;
			}
		}
	// send mail

		memset(linebuffer, 0, sizeof(linebuffer));

		if(fgets(linebuffer, sizeof(linebuffer), mail))
		{
			linebuffer[strlen(linebuffer) - 1] = '\0';

			if(!SendSMTPCommand(MAIL, linebuffer))
			{
				fclose(mail);

				return 0;
			}
		}

	// send rcpt

		memset(linebuffer, 0, sizeof(linebuffer));

		if(fgets(linebuffer, sizeof(linebuffer), mail))
		{
			linebuffer[strlen(linebuffer) - 1] = '\0';

			if(!SendSMTPCommand(RCPT, linebuffer))
			{
				fclose(mail);

				return 0;
			}
		}

	// send data

		if(!SendSMTPCommand(DATA1, ""))
		{
			fclose(mail);

			return 0;
		}

		memset(linebuffer, 0, sizeof(linebuffer));

		while(fgets(linebuffer, sizeof(linebuffer), mail))
		{
			linebuffer[strlen(linebuffer) - 1] = '\0';

			if(!SendSMTPCommand(DATA2, linebuffer))
			{
				fclose(mail);

				return 0;
			}

			memset(linebuffer, 0, sizeof(linebuffer));
		}

		if(!SendSMTPCommand(DATA3, ""))
		{
			fclose(mail);

			return 0;
		}

	// send quit

		if(!SendSMTPCommand(QUIT, ""))
		{
			fclose(mail);

			return 0;
		}

	// success

		fclose(mail);

		return 1;
}

/******************************************************************************
 * SaveMail (0 = fail)
 ******************************************************************************/

int SaveMail(int account, char* mailuid)
{
	int loop;
	char mailnumber[12];
	char imap1 = 0;
	
	if( account_db[account].imap[0] != '\0' )
	{
		imap1 = 1;
	}
	
	if((fd_mail = fopen(POP3FILE, "w")))
	{

	// timestamp

	// get mail count

		if( !imap1 )
		{
			if(!SendPOPCommand(INIT, account_db[account].pop3))
			{
				if(fd_mail)
				{
					fclose(fd_mail);
				}

				return 0;
			}

			if(!SendPOPCommand(USER, account_db[account].user))
			{
				if(fd_mail)
				{
					fclose(fd_mail);
				}
	
				return 0;
			}

			if(!SendPOPCommand(PASS, account_db[account].pass))
			{
				if(fd_mail)
				{
					fclose(fd_mail);
				}
	
				return 0;
			}

			if(!SendPOPCommand(STAT, ""))
			{
				if(fd_mail)
				{
					fclose(fd_mail);
				}

				return 0;
			}
		}
		else
		{
			if(!SendIMAPCommand(INIT, account_db[account].imap, ""))
			{
				if(fd_mail)
				{
					fclose(fd_mail);
				}

				return 0;
			}

			// login to mail server
			if(!SendIMAPCommand(LOGIN, account_db[account].user, account_db[account].pass))
			{
				if(fd_mail)
				{
					fclose(fd_mail);
				}

				return 0;
			}
		
			// select folder, get mail count and UID
			if(!SendIMAPCommand(SELECT, account_db[account].inbox, ""))
			{
				if(fd_mail)
				{
					fclose(fd_mail);
				}

				return 0;
			}
		}



		if (!messages) return 0;

		for(loop = messages; loop != 0; loop--)
		{
			sprintf(mailnumber, "%d", loop);

			if( !imap1 )
			{
				if(!SendPOPCommand(UIDL, mailnumber))
				{
					if(fd_mail)
					{
						fclose(fd_mail);
					}

					return 0;
				}
			}
			else
			{
				if(!SendIMAPCommand(UIDL, mailnumber, ""))
				{
					if(fd_mail)
					{
						fclose(fd_mail);
					}

					return 0;
				}
			}
			
//			printf("TuxMailD <SaveFile idx(%u) uid(%s)>\n", loop,uid);
			if(!strcmp(uid,mailuid))
			{
				printf("TuxMailD <SaveFile idx(%u) uid(%s)>\n", loop,uid);

				if( !imap1 )
				{ 
					if(!SendPOPCommand(RETR, mailnumber))
					{
						if(fd_mail)
						{
							fclose(fd_mail);
						}

						return 0;
					}					
				
					fclose(fd_mail);
					SendPOPCommand(QUIT, "");
				}
				else
				{
					char seen = 0;
					if(!SendIMAPCommand(FLAGS, mailnumber, &seen))
					{
						if(fd_mail)
						{
							fclose(fd_mail);
						}

						return 0;
					}
					
					if((!seen) || (!SendIMAPCommand(RETR, mailnumber, "")))
					{
						if(fd_mail)
						{
							fclose(fd_mail);
						}

						return 0;
					}					

					if( seen == 'U' )
					{
						if(!SendIMAPCommand(UNSEEN, mailnumber, ""))
						{
							if(fd_mail)
							{
								fclose(fd_mail);
							}

							return 0;
						}					
					}
									
					fclose(fd_mail);
					SendIMAPCommand(LOGOUT, "", "");
				}
				return 1;
				
			}
		}

		fclose(fd_mail);
		
		if( !imap1 )
		{
			SendPOPCommand(QUIT, "");
		}
		else
		{
			SendIMAPCommand(LOGOUT, "", "");
		}
	}

	return 0;
}

/******************************************************************************
 * int ExecuteMail(char* mailfile) (0=not executed, 1=exectuted)
 ******************************************************************************/

int ExecuteMail(char* mailfile)
{
	// only execute if a securitystring is defined
	if( strlen(security)==0 ) 
	{
		return 0;
	}
	
	char exit = -1;
	FILE* fd_mail;
	fd_mail = fopen(mailfile, "r");

	char linebuffer[256];
	char executeline[1024];
	
	// read first line of mail to check if we should execute it
	if( fgets(linebuffer,sizeof(linebuffer),fd_mail) )
	{
		while(( linebuffer[strlen(linebuffer)-1] == '\n' ) || ( linebuffer[strlen(linebuffer)-1] == '\r') )
		{
			linebuffer[strlen(linebuffer)-1] = '\0';
			if( linebuffer[0] == '\0' )
			{
				break;
			}
		}
		if( strcmp(linebuffer,security) )
		{
			exit = 0;
		}
	}
	else
	{
		exit = 0;
	}

	if( exit )
	{	
		executeline[0] = '\0';
		
		while(fgets(linebuffer,sizeof(linebuffer),fd_mail))
		{
			while(( linebuffer[strlen(linebuffer)-1] == '\n' ) || ( linebuffer[strlen(linebuffer)-1] == '\r') )
			{
				linebuffer[strlen(linebuffer)-1] = '\0';
				if( linebuffer[0] == '\0' )
				{
					break;
				}
			}
			if( strcmp(&linebuffer[1],".") )
			{
				strcat(executeline,&linebuffer[1]);
				if( executeline[strlen(executeline)-1] == '&' )
				{
					executeline[strlen(executeline)-1] = '\0';	
				}
				else
				{
					if( executeline[0] )
					{
						printf("tuxmaild execute: '%s'\n",executeline);
						system(executeline);
						executeline[0] = '\0';
					}
				}
			}
			else
			{	
				exit = 1;
				break;
			}
		}
	}					
	fclose(fd_mail);
	return exit;
}

/******************************************************************************
 * int ScanMail(char* mailfile,int account,char* mailnumber, FILE* fd_status) (0=ok, 1=marked as SPAM)
 ******************************************************************************/

int ScanMail(char* mailfile,int account,char* mailnumber, FILE* fd_status)
{
	if(!use_spamfilter)
	{
		return 0;
	}
	
  int loop;
	char spamcheck = 0;

	// check the spamfilter fpr mail-text scanning
	for(loop = 0; loop < spam_entries; loop++)
	{
		// check if we should look for inside the mailtext
		if(spamfilter[loop].address[0]=='&')	
		{
			spamcheck=1;
			break;
		}
	}		

	if(!spamcheck)
	{
		return 0;
	}
	
	int scancnt = 20;
	char spam = 0;
	FILE* fd_mail;
	fd_mail = fopen(mailfile, "r");
	char linebuffer[256];
  int filesize;
  char *known_uids = 0;
	char *pointer;
  	
	// scan the first x lines of th file for a spam-tag
	do
	{	
		while((fgets(linebuffer,sizeof(linebuffer),fd_mail)) && (!spam))
		{
			for(loop = 0; loop < spam_entries; loop++)
			{
				// check if we should look for inside the mailtext
				if((spamfilter[loop].address[0]=='&')	&& (strstr(linebuffer, &spamfilter[loop].address[1])))
				{
					while(( linebuffer[strlen(linebuffer)-1] == '\n' ) || ( linebuffer[strlen(linebuffer)-1] == '\r') )
					{
						linebuffer[strlen(linebuffer)-1] = '\0';
					}
												
					slog ? syslog(LOG_DAEMON | LOG_INFO, "Spamfilter active, delete Mail with line \"%s\"", linebuffer) : printf("TuxMailD <Spamfilter active, delete Mail with line \"%s\" box:%d mail:<%s>>\n", linebuffer,account,mailnumber);

					spam = 1;

					break;
				}
			}						
		}
	}
	while((--scancnt) && (!spam));
	
	fclose(fd_mail);
	
	// if we found a spam-tag, mark the state of the mail
	if(spam)
	{
		// uid
		if(fd_status)
		{
			fseek(fd_status, 0, SEEK_END);

			if((filesize = ftell(fd_status)))
			{
				known_uids = malloc(filesize + 1);
				memset(known_uids, 0, filesize + 1);

				rewind(fd_status);
				fread(known_uids, filesize, 1, fd_status);
				
        pointer = strstr(known_uids, uid);
        if(pointer != 0)
        {
        	*(pointer-2)='D';
        }
        
				rewind(fd_status);
				fwrite(known_uids, filesize, 1, fd_status);		
			}
		}
	}
	
	return spam;
}

/******************************************************************************
 * AddNewMailFile (0=fail, 1=done)
 ******************************************************************************/

int AddNewMailFile(int account, char *mailnumber, FILE *fd_status)
{
	char *stored_uids = 0, *ptr = 0;
	int filesize = 0;
	int idx1 = 0;
	char idxfile[256];
	char mailfile[256];
	FILE *fd_mailidx;
	
	// if we do not store the mails
	if( !mailcache )
	{
		return 0;
	}
	
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
		fclose(fd_mailidx);
		fd_mailidx = NULL;
	}

	if((filesize) && (ptr = strstr(stored_uids, uid)))
	{
		// we already have this mail read
		free(stored_uids);
		return 1;
	}
	
	if(!(fd_mailidx = fopen(idxfile, "w")))
	{
		slog ? syslog(LOG_DAEMON | LOG_INFO, "could not create Idx-File for Account %d", account) : printf("TuxMailD <could not create Idx-File for Account %d>\n", account);
	}

	if(fd_mailidx)
	{
	  if( !filesize	)
	  {
	  	idx1 = 1;
	  }
	  else
	  {
			sscanf(&stored_uids[1],"%02u",&idx1);
			if( ++idx1 > mailcache )
			{
				idx1 = 1;
			}
	  }
 	  
		sprintf(mailfile,"%stuxmail.idx%u.%u",maildir,account,idx1);
//		printf("%stuxmail.idx%u.%u\n",maildir,account,idx1);
		fd_mail=fopen(mailfile,"w");
		
		if(fd_mail)
		{
			if( !imap )
			{
				if(!SendPOPCommand(RETR, mailnumber))
				{
					idx1 = 0;
					fclose(fd_mail);
				}
				else
				{
					// printf("write email nr: %s at %stuxmail.idx%u.%u\n",mailnumber,maildir,account,idx1);
					fclose(fd_mail);
					ExecuteMail(mailfile);
					ScanMail(mailfile,account,mailnumber,fd_status);
				}
			}
			else
			{				
				char seen=0;

				if(!SendIMAPCommand(FLAGS, mailnumber, &seen))
				{
					seen=0;
				}
									
				if((!seen) || (!SendIMAPCommand(RETR, mailnumber, "")))
				{
					// printf("error write email nr: %s at %stuxmail.idx%u.%u\n",mailnumber,maildir,account,idx1);
					idx1 = 0;
					fclose(fd_mail);
				}
				else
				{
					// printf("write email nr: %s at %stuxmail.idx%u.%u\n",mailnumber,maildir,account,idx1);
					if( seen == 'U' )
					{
						SendIMAPCommand(UNSEEN, mailnumber, "");
					}

					fclose(fd_mail);
					ExecuteMail(mailfile);
					ScanMail(mailfile,account,mailnumber,fd_status);
				}
			}
		}

		if( idx1 )
		{
			fprintf(fd_mailidx, "|%02u|%s\n", idx1, uid);
			
			char cComp[5];
			sprintf(cComp,"|%02u|",idx1);
			if( filesize	)
			{
				if( (ptr = strstr(stored_uids,cComp)) )
				{
					*ptr = '\0';
				}
				fprintf(fd_mailidx, "%s", stored_uids);
			}
		}

		fclose(fd_mailidx);	
	}
	
	free(stored_uids);
	if( idx1 )
	{
		return 1;
	}
	return 0;
}

/******************************************************************************
 * CheckAccount (0=fail, 1=done)
 ******************************************************************************/

int CheckAccount(int account)
{
	int loop, minloop = 0;
	FILE *fd_status, *fd_idx;
	int filesize, skip_uid_check = 0;
	char statusfile[] = "/tmp/tuxmail.?";
	char *known_uids = 0, *ptr = 0;
	char mailnumber[12];
	int readmails = 0;
	int knownmails = 0;

	imap = 0;
		
	// timestamp

	time(&tt);
	strftime(timeinfo, 22, "%R", localtime(&tt));

	// check if we should use an IMAP server
	if( account_db[account].imap[0] != '\0' )
	{
		imap = 1;

		// init connection to server
		if(!SendIMAPCommand(INIT, account_db[account].imap, ""))
		{
			return 0;
		}

		// login to mail server
		if(!SendIMAPCommand(LOGIN, account_db[account].user, account_db[account].pass))
		{
			return 0;
		}
		
		// select folder, get mail count and UID
		if(!SendIMAPCommand(SELECT, account_db[account].inbox, ""))
		{
			return 0;
		}
	}
	else
	{
		// get mail count
		if(!SendPOPCommand(INIT, account_db[account].pop3))
		{
			return 0;
		}

		if(!SendPOPCommand(USER, account_db[account].user))
		{
			return 0;
		}

		if(!SendPOPCommand(PASS, account_db[account].pass))
		{
			return 0;
		}

		if(!SendPOPCommand(STAT, ""))
		{
			return 0;
		}

		if(!SendPOPCommand(RSET, ""))
		{
			return 0;
		}
	}
	
		account_db[account].mail_all = messages;
		account_db[account].mail_new = 0;
		account_db[account].mail_unread = 0;
		account_db[account].mail_read = 0;
		deleted_messages = 0;

	// get mail info

		statusfile[sizeof(statusfile) - 2] = account | '0';

		if(messages)
		{
			// load last status from file

				if((fd_status = fopen(statusfile, "r")))
				{
					fseek(fd_status, 0, SEEK_END);

					if((filesize = ftell(fd_status)))
					{
						known_uids = malloc(filesize + 1);
						memset(known_uids, 0, filesize + 1);

						rewind(fd_status);
						fread(known_uids, filesize, 1, fd_status);
					}
					else
					{
						slog ? syslog(LOG_DAEMON | LOG_INFO, "empty Status for Account %d", account) : printf("TuxMailD <empty Status for Account %d>\n", account);

						skip_uid_check = 1;
					}

					fclose(fd_status);
				}
				else
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "no Status for Account %d", account) : printf("TuxMailD <no Status for Account %d>\n", account);

					skip_uid_check = 1;
				}

			// clear status

				if(!(fd_status = fopen(statusfile, "w+")))
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "could not create Status for Account %d", account) : printf("TuxMailD <could not create Status for Account %d>\n", account);
				}

				fd_idx = fopen("/tmp/tuxmaild.idx", "w+");
				
			// generate listing

				if(fd_status)
				{
					fprintf(fd_status, "- --:-- %s ---/---\n", account_db[account].name); /* reserve space */
				}

				if((messages > 2*MAXMAIL) && ( typeflag > 1 ))
				{
					minloop = messages - 2*MAXMAIL;
				}
				for(loop = messages; loop != minloop; loop--)
				{
					sprintf(mailnumber, "%d", loop);

					if( !imap )
					{
						if(!SendPOPCommand(UIDL, mailnumber))
						{
							free(known_uids);
	
							if(fd_status)
							{
								fclose(fd_status);
							}
	
							return 0;
						}
					}
					else
					{
						if(!SendIMAPCommand(UIDL, mailnumber, ""))
						{
							free(known_uids);
	
							if(fd_status)
							{
								fclose(fd_status);
							}
	
							return 0;
						}
					}

					if(skip_uid_check)
					{
						if( !imap )
						{
							if(!SendPOPCommand(TOP, mailnumber))
							{
								free(known_uids);
	
								if(fd_status)
								{
									fclose(fd_status);
								}
	
								return 0;
							}
						}
						else
						{
							char seen;
							if(!SendIMAPCommand(FLAGS, mailnumber, &seen))
							{
								free(known_uids);
	
								if(fd_status)
								{
									fclose(fd_status);
								}
	
								return 0;
							}

							if(!SendIMAPCommand(FETCH, mailnumber, ""))
							{
								free(known_uids);
	
								if(fd_status)
								{
									fclose(fd_status);
								}
	
								return 0;
							}
							
							if( seen == 'U' )
							{
								if(!SendIMAPCommand(UNSEEN, mailnumber, ""))
								{
									free(known_uids);
		
									if(fd_status)
									{
										fclose(fd_status);
									}
	
									return 0;
								}					
							}
							
						}

						if(use_spamfilter && spam_detected)
						{
							if( !imap )
							{
								
								if(!SendPOPCommand(DELE, mailnumber))
								{
									free(known_uids);
	
									if(fd_status)
									{
										fclose(fd_status);
									}
	
									return 0;
								}
							}
							else
							{
								if(!SendIMAPCommand(DELE, mailnumber, ""))
								{
									free(known_uids);
	
									if(fd_status)
									{
										fclose(fd_status);
									}
	
									return 0;
								}
							}
							deleted_messages++;
						}
						else
						{
							account_db[account].mail_new++;

							if((fd_idx) && (readmails < mailcache))
							{
								fprintf(fd_idx,"|%4d|%s\n",loop,uid);
								readmails++;
							}
							
							if(fd_status)
							{
								fprintf(fd_status, "|N|%s|%s\n", uid, header);
							}
						}
					}
					else
					{
						if((ptr = strstr(known_uids, uid)))
						{
							knownmails++;
							
							if(*(ptr - 2) == 'D')
							{
								if( !imap )
								{
									if(!SendPOPCommand(DELE, mailnumber))
									{
										free(known_uids);
	
										if(fd_status)
										{
											fclose(fd_status);
										}
	
										return 0;
									}
								}
								else
								{
									if(!SendIMAPCommand(DELE, mailnumber, ""))
									{
										free(known_uids);
	
										if(fd_status)
										{
											fclose(fd_status);
										}
	
										return 0;
									}
								}

								deleted_messages++;
							}
							else 
							{
								if(fd_status)
								{
									if( typeflag > 1 )
									{
										if((*(ptr - 2) == 'N') || (*(ptr - 2) == 'n'))
										{
											fprintf(fd_status, "|n|");
											if( (account_db[account].mail_read + account_db[account].mail_new + account_db[account].mail_unread) < MAXMAIL )
											{
												account_db[account].mail_unread++;
											}
										}
										else
										{
											fprintf(fd_status, "|O|");
											account_db[account].mail_read++;
										}
									}
									else
									{
										fprintf(fd_status, "|O|");
									}
									
									while(*ptr != '\n')
									{
										fprintf(fd_status, "%c", *ptr++);
									}

									fprintf(fd_status, "\n");
								}
							}
						}
						else
						{
							if( !imap )
							{
								if(!SendPOPCommand(TOP, mailnumber))
								{
									free(known_uids);
	
									if(fd_status)
									{
										fclose(fd_status);
									}
	
									return 0;
								}
							}
							else
							{
								char seen;
								if(!SendIMAPCommand(FLAGS, mailnumber, &seen))
								{
									free(known_uids);
		
									if(fd_status)
									{
										fclose(fd_status);
									}
		
									return 0;
								}

								if(!SendIMAPCommand(FETCH, mailnumber, ""))
								{
									free(known_uids);
	
									if(fd_status)
									{
										fclose(fd_status);
									}
	
									return 0;
								}

								if( seen == 'U' )
								{
									if(!SendIMAPCommand(UNSEEN, mailnumber, ""))
									{
										free(known_uids);
			
										if(fd_status)
										{
											fclose(fd_status);
										}
		
										return 0;
									}					
								}

							}
							if(use_spamfilter && spam_detected)
							{
								if( !imap )
								{
									if(!SendPOPCommand(DELE, mailnumber))
									{
										free(known_uids);
	
										if(fd_status)
										{
											fclose(fd_status);
										}
	
										return 0;
									}
								}
								else
								{
									if(!SendIMAPCommand(DELE, mailnumber, ""))
									{
										free(known_uids);
	
										if(fd_status)
										{
											fclose(fd_status);
										}
	
										return 0;
									}
								}
								deleted_messages++;
							}
							else
							{
								if((!knownmails) || ( typeflag == 1 ))
								{
									account_db[account].mail_new++;
	
									if((fd_idx) && (readmails < mailcache))
									{
										fprintf(fd_idx,"|%4d|%s\n",loop,uid);
										readmails++;
									}

									if(fd_status)
									{
										fprintf(fd_status, "|N|%s|%s\n", uid, header);
									}
								}
								else
								{				
									// if new mail have been found after already found mails, mark the mails as unread, but not new			
									account_db[account].mail_unread++;
	
									if((fd_idx) && (readmails < mailcache))
									{
										fprintf(fd_idx,"|%4d|%s\n",loop,uid);
										readmails++;
									}
	
									if(fd_status)
									{
										fprintf(fd_status, "|n|%s|%s\n", uid, header);
									}
								}
							}
						}
					}
				}

				if((fd_status) && (fd_idx) && (account_db[account].mail_new))
				{
					char linebuffer[256];
					int i, j;

					// reopen status-file and read content to known_uids
/*
					free(known_uids);
					fclose(fd_status);
					if((fd_status = fopen(statusfile, "r+")))
					{
						fseek(fd_status, 0, SEEK_END);

						if((filesize = ftell(fd_status)))
						{
							known_uids = malloc(filesize + 1);
							memset(known_uids, 0, filesize + 1);
							rewind(fd_status);
							fread(known_uids, filesize, 1, fd_status);
						}
					}
*/
					
					// if we have a hard-disk, we have to wake-up the disk and to wait until the disk is awaken
					if( readmails )
					{
						system(CFGPATH WAKEFILE);
					}
					
					for( i=0; i<readmails; i++)
					{
						rewind(fd_idx);
						j = readmails-i;
						while(fgets(linebuffer,sizeof(linebuffer),fd_idx))
						{
	//						printf("idx:%d j:%d line:%s\n",i,j,linebuffer);
							j--;
							if( !j ) break;							
						}				
						linebuffer[strlen(linebuffer)-1]='\0';
						linebuffer[5]='\0';
						sscanf(&linebuffer[1],"%s",&mailnumber[0]);
						sscanf(&linebuffer[6],"%s",&uid[0]);
						// printf("mail: %d  number: %s  uid: %s \n",i,mailnumber,uid);
						if( !AddNewMailFile(account, mailnumber,fd_status) )
						{
						// printf("not found\n");
						}
					}
				}
				
				if(( !deleted_messages ) && ( !imap ))
				{
					if(!SendPOPCommand(RSET, ""))
					{
						return 0;
					}
				}
				
				if(fd_status)
				{
					rewind(fd_status);

					fprintf(fd_status, "%.1d %s %s %.3d/%.3d\n", account, timeinfo, account_db[account].name, account_db[account].mail_new + account_db[account].mail_unread, account_db[account].mail_all - deleted_messages);
				}

				free(known_uids);

				if(fd_idx)
				{
					fclose(fd_idx);
				}
				
				if(fd_status)
				{
					fclose(fd_status);
				}
		}
		else
		{
			account_db[account].mail_new = 0;

			if((fd_status = fopen(statusfile, "w")))
			{
				fprintf(fd_status, "%.1d %s %s 000/000\n", account, timeinfo, account_db[account].name);

				fclose(fd_status);
			}
			else
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "could not create Status for Account %d", account) : printf("TuxMailD <could not create Status for Account %d>\n", account);
			}
		}

	// close session

		if( !imap )
		{
			if(!SendPOPCommand(QUIT, ""))
			{
				return 0;
			}	
		}
		else
		{
			if(!SendIMAPCommand(EXPUNGE, "", ""))
			{
				return 0;
			}	
			if(!SendIMAPCommand(CLOSE, "", ""))
			{
				return 0;
			}	
			if(!SendIMAPCommand(LOGOUT, "", ""))
			{
				return 0;
			}	
		}
		
	return 1;
}

/******************************************************************************
 * SwapEndian
 ******************************************************************************/

void SwapEndian(unsigned char *header)
{
	/* wrote the PlaySound() on my pc not in mind that dbox is big endian. so this was the lazy way to make it work, sorry... */

	struct WAVEHEADER *wave = (struct WAVEHEADER*)header;

	wave->ChunkID1 = (wave->ChunkID1 << 24) | ((wave->ChunkID1 & 0x0000ff00) << 8) | ((wave->ChunkID1 & 0x00ff0000) >> 8) | (wave->ChunkID1 >> 24);
//	wave->ChunkSize1 = (wave->ChunkSize1 << 24) | ((wave->ChunkSize1 & 0x0000ff00) << 8) | ((wave->ChunkSize1 & 0x00ff0000) >> 8) | (wave->ChunkSize1 >> 24);
	wave->ChunkType = (wave->ChunkType << 24) | ((wave->ChunkType & 0x0000ff00) << 8) | ((wave->ChunkType & 0x00ff0000) >> 8) | (wave->ChunkType >> 24);

	wave->ChunkID2 = (wave->ChunkID2 << 24) | ((wave->ChunkID2 & 0x0000ff00) << 8) | ((wave->ChunkID2 & 0x00ff0000) >> 8) | (wave->ChunkID2 >> 24);
	wave->ChunkSize2 = (wave->ChunkSize2 << 24) | ((wave->ChunkSize2 & 0x0000ff00) << 8) | ((wave->ChunkSize2 & 0x00ff0000) >> 8) | (wave->ChunkSize2 >> 24);
	wave->Format = (wave->Format >> 8) | (wave->Format << 8);
	wave->Channels = (wave->Channels >> 8) | (wave->Channels << 8);
	wave->SampleRate = (wave->SampleRate << 24) | ((wave->SampleRate & 0x0000ff00) << 8) | ((wave->SampleRate & 0x00ff0000) >> 8) | (wave->SampleRate >> 24);
//	wave->BytesPerSecond = (wave->BytesPerSecond << 24) | ((wave->BytesPerSecond & 0x0000ff00) << 8) | ((wave->BytesPerSecond & 0x00ff0000) >> 8) | (wave->BytesPerSecond >> 24);
//	wave->BlockAlign = (wave->BlockAlign >> 8) | (wave->BlockAlign << 8);
	wave->BitsPerSample = (wave->BitsPerSample >> 8) | (wave->BitsPerSample << 8);

	wave->ChunkID3 = (wave->ChunkID3 << 24) | ((wave->ChunkID3 & 0x0000ff00) << 8) | ((wave->ChunkID3 & 0x00ff0000) >> 8) | (wave->ChunkID3 >> 24);
	wave->ChunkSize3 = (wave->ChunkSize3 << 24) | ((wave->ChunkSize3 & 0x0000ff00) << 8) | ((wave->ChunkSize3 & 0x00ff0000) >> 8) | (wave->ChunkSize3 >> 24);
}

/******************************************************************************
 * SwapEndianChunk
 ******************************************************************************/

void SwapEndianChunk(unsigned char *chunk)
{
	/* wrote the PlaySound() on my pc not in mind that dbox is big endian. so this was the lazy way to make it work, sorry... */

	struct CHUNK *wave = (struct CHUNK*)chunk;

	wave->ChunkID = (wave->ChunkID << 24) | ((wave->ChunkID & 0x0000ff00) << 8) | ((wave->ChunkID & 0x00ff0000) >> 8) | (wave->ChunkID >> 24);
	wave->ChunkSize = (wave->ChunkSize << 24) | ((wave->ChunkSize & 0x0000ff00) << 8) | ((wave->ChunkSize & 0x00ff0000) >> 8) | (wave->ChunkSize >> 24);
}

/******************************************************************************
 * PlaySound
 ******************************************************************************/

void PlaySound(unsigned char *file)
{
	FILE *fd_wav;
	unsigned char header[sizeof(struct WAVEHEADER)];
	int dsp, format, channels, speed, blocksize, readcount, count = 0;
	unsigned char tmp;
	unsigned char *samples;
	struct WAVEHEADER *wave = (struct WAVEHEADER*)header;

	// check for userdefined soundfile

		if(!(fd_wav = fopen(file, "rb")))
		{
			format = AFMT_U8;
			channels = 1;
			speed = 12000;
			wave->ChunkSize3 = sizeof(audiodata);
			samples = audiodata;
		}
		else
		{
			// read header and detect format

				fread(header, 1, sizeof(header)-8, fd_wav);

				SwapEndian(header);

				if(wave->ChunkID1 != RIFF || wave->ChunkType != WAVE || wave->ChunkID2 != FMT)
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "unsupported Soundfile (WAVE only)") : printf("TuxMailD <unsupported Soundfile (WAVE only)>\n");

					fclose(fd_wav);

					return;
				}

				if(wave->Format != PCM)
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "unsupported Soundfile (PCM only)") : printf("TuxMailD <unsupported Soundfile (PCM only)>\n");

					fclose(fd_wav);

					return;
				}

				if(wave->SampleRate != 12000 && wave->SampleRate != 24000 && wave->SampleRate != 48000)
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "unsupported Soundfile (12/24/48KHz only)") : printf("TuxMailD <unsupported Soundfile (12/24/48KHz only)>\n");

					fclose(fd_wav);

					return;
				}

				format = (wave->BitsPerSample == 8) ? AFMT_U8 : AFMT_S16_LE;
				channels = wave->Channels;
				speed = wave->SampleRate;

				// step over unsed bytes in fmt-chunk
				count = wave->ChunkSize2 - 16;
				while(count)
				{
					fread(&tmp, 1, 1, fd_wav);
					count--;	
				}
				
				// find data-chunk
				do
				{
				  readcount=fread(&wave->ChunkID3, 1, 8, fd_wav);
				  SwapEndianChunk((char *)&wave->ChunkID3);
					if(readcount < 8)
					{
						slog ? syslog(LOG_DAEMON | LOG_INFO, "could not find Sounddata") : printf("TuxMailD <could not find Sounddata>\n");
	
						fclose(fd_wav);
	
						return;
					}
				  if(wave->ChunkID3 != DATA)
				  {
						count = wave->ChunkSize3;
						while(count)
						{
							fread(&tmp, 1, 1, fd_wav);
							count--;	
						}				  	
				  }
				}while(wave->ChunkID3 != DATA);
				
			// get samples

				if(!(samples = (unsigned char*)malloc(wave->ChunkSize3)))
				{
					slog ? syslog(LOG_DAEMON | LOG_INFO, "not enough Memory for Sounddata") : printf("TuxMailD <not enough Memory for Sounddata>\n");

					fclose(fd_wav);

					return;
				}

				fread(samples, 1, wave->ChunkSize3, fd_wav);

				fclose(fd_wav);
		}

	// play sound

		if((dsp = open(DSP, O_WRONLY)) == -1)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "could not open DSP") : printf("TuxMailD <could not open DSP>\n");

//			audio = 'N';
// don't disable audio-notification, because DSP could also be locked from Audioplayer
// and if we disable audio from now on, we would never have a audio-notification until
// configuration is reread

			return;
		}

		if(ioctl(dsp, SNDCTL_DSP_SETFMT, &format) == -1)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "could not set DSP-Format") : printf("TuxMailD <could not set DSP-Format>\n");

			close(dsp);

			return;
		}

		if(ioctl(dsp, SNDCTL_DSP_CHANNELS, &channels) == -1)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "could not set DSP-Channels") : printf("TuxMailD <could not set DSP-Channels>\n");

			close(dsp);

			return;
		}

		if(ioctl(dsp, SNDCTL_DSP_SPEED, &speed) == -1)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "could not set DSP-Samplerate") : printf("TuxMailD <could not set DSP-Samplerate>\n");

			close(dsp);

			return;
		}

		if(ioctl(dsp, SNDCTL_DSP_GETBLKSIZE, &blocksize) == -1)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "could not get DSP-Blocksize") : printf("TuxMailD <could not get DSP-Blocksize>\n");

			close(dsp);

			return;
		}

		while(count < wave->ChunkSize3)
		{
			write(dsp, samples + count, (count + blocksize > wave->ChunkSize3) ?  wave->ChunkSize3 - count : blocksize);

			count += blocksize;
		}

		ioctl(dsp, SNDCTL_DSP_SYNC);

	// cleanup

		if(samples != audiodata)
		{
			free(samples);
		}

		close(dsp);
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
 * ShowLCD
 ******************************************************************************/

void ShowLCD(int mails)
{
	int fd_lcd;
	int x, y;
    static int sum = 0;
    
    // mark lcd as locked
    
	    if(unlink(LCKFILE))
    	{
    		sum = mails;
    	}
    	else
    	{
    		if( typeflag == 1 )
    		{
    			sum += mails;
    		}
    		else
    		{
    			sum = mails;
    		}
    	}
    
    	fclose(fopen(LCKFILE, "w"));
    
    // clear counter area
    
    	for(y = 0; y < 15; y++)
    	{	
        	for(x = 0; x < 34; x++)
    		{
    			lcd_buffer[74 + x + ((23 + y)/8)*120] &= ~(1 << ((23 + y)%8));
    		}
    	}
    
    // set new counter
    
    	if(sum > 99)
    	{
            RenderLCDDigit(sum/100, 74, 23);
            RenderLCDDigit(sum%100/10, 86, 23);
            RenderLCDDigit(sum%10, 98, 23);
    	}
    	else if(sum > 9)
    	{
            RenderLCDDigit(sum/10, 80, 23);
            RenderLCDDigit(sum%10, 92, 23);
    	}
    	else
    	{
    		RenderLCDDigit(sum, 86, 23);
    	}

	// copy to lcd

		if((fd_lcd = open(LCD, O_RDWR)) == -1)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "could not open LCD") : printf("TuxMailD <could not open LCD>\n");

			lcd = 'N';
		}
		else
		{
			write(fd_lcd, &lcd_buffer, sizeof(lcd_buffer));

			close(fd_lcd);
		}
}

/******************************************************************************
 * NotifyUser
 ******************************************************************************/

void NotifyUser(int newmails, int oldmails)
{
	int loop;
	struct sockaddr_in SockAddr;
	char http_cmd[1024], tmp_buffer[128];
	char *http_cmd1 = "GET /cgi-bin/startPlugin?name=tuxmail.cfg HTTP/1.1\n\n";
	char *http_cmd2 = "GET /cgi-bin/xmessage?timeout=10&caption=TuxMail%20Information&body=";
	char *http_cmd3 = "GET /control/message?nmsg=";
	char *http_cmd4 = "GET /control/message?popup=";
	static int sum = 0;
	int mails = newmails + oldmails;

	// first check if we have to remove old notifications
	
	if(newmails == 0)									// we do not have new mails
	{
		if((typeflag != 1) && (oldmails == 0))			// we have no unread mails
		{
			unlink(LCKFILE);							// clear LCD output
			unlink(NOTIFILE);							// clear notify-file
			return;
		}
		if(typeflag == 1)								// no further processing in this mode
		{
			return;
		}
	}
	
	// write notify-file (used by other programs to know the number of unread messages)
	
	    if(unlink(NOTIFILE))
   		{
	   		sum = mails;
   		}
    	else
    	{
    		if( typeflag == 1 )
	    	{
    			sum += mails;
    		}
    		else
    		{
	    		sum = mails;
    		}
    	}
		FILE* pipe;
		pipe = fopen(NOTIFILE,"w");
		if( pipe != NULL)
		{
			fprintf(pipe, "%d", sum);
			fclose(pipe);
		}

	// lcd notify

		if(lcd == 'Y')
		{
			ShowLCD(mails);
		}

	// further notification only if new mails have be detected
	
		if(newmails == 0)
		{
			return;
		}
		
	// audio notify

		if(audio == 'Y')
		{
			PlaySound(CFGPATH SNDFILE);
		}

	// video notify

		if(video != 5)
		{
 			switch(video)
			{
				case 4:
					strcpy(http_cmd, http_cmd4);

					break;

				case 3:
					strcpy(http_cmd, http_cmd3);

					break;

				case 2:
					strcpy(http_cmd, http_cmd2);

					break;

				default:

					strcpy(http_cmd, http_cmd1);
			}

			if(video > 1)
			{
				for(loop = 0; loop < 10; loop++)
				{
					if(account_db[loop].mail_new)
					{
						if(video == 2)
						{
							sprintf(tmp_buffer, (osd == 'G') ? "Konto%%20#%d:%%20%.3d%%20Mail(s)%%20f\xC3\xBCr%%20%s%%0A" : "Account%%20#%d:%%20%.3d%%20Mail(s)%%20for%%20%s%%0A", loop, account_db[loop].mail_new + account_db[loop].mail_unread, account_db[loop].name);
						}

						if(video == 3 || video == 4)
						{
							sprintf(tmp_buffer, (osd == 'G') ? "Konto%%20#%d:%%20%.3d%%20Mail(s)%%20f\xC3\xBCr%%20%s%%0A" : "Account%%20#%d:%%20%.3d%%20Mail(s)%%20for%%20%s%%0A", loop, account_db[loop].mail_new + account_db[loop].mail_unread, account_db[loop].name);
						}

						strcat(http_cmd, tmp_buffer);
					}
				}

				strcat(http_cmd, " HTTP/1.1\n");
			
				if(webuser[0])
				{
					strcat(http_cmd, "Authorization: Basic ");

					strcpy(decodedstring, webuser);
					strcat(decodedstring, ":");
					strcat(decodedstring, webpass);
					EncodeBase64(decodedstring, strlen(decodedstring));

					strcat(http_cmd, encodedstring);
					strcat(http_cmd, "\n\n");			
				}
				else
				{
					strcat(http_cmd, "\n");
				}
			}

			if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "could not create Socket") : printf("TuxMailD <could not create Socket>\n");

				return;
			}

			SockAddr.sin_family = AF_INET;
			SockAddr.sin_port = htons(webport);
			inet_aton("127.0.0.1", &SockAddr.sin_addr);

			if(connect(sock, (struct sockaddr*)&SockAddr, sizeof(SockAddr)))
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "could not connect to WebServer") : printf("TuxMailD <could not connect to WebServer>\n");

				close(sock);

				return;
			}

			send(sock, http_cmd, strlen(http_cmd), 0);

			close(sock);
		}		
}		

/******************************************************************************
 * SigHandler
 ******************************************************************************/

void SigHandler(int signal)
{
	switch(signal)
	{
		case SIGTERM:

			slog ? syslog(LOG_DAEMON | LOG_INFO, "shutdown") : printf("TuxMailD <shutdown>\n");

			intervall = 0;

			break;

		case SIGHUP:

			slog ? syslog(LOG_DAEMON | LOG_INFO, "update") : printf("TuxMailD <update>\n");

			if(!ReadConf())
			{
				intervall = 0;
			}

			ReadSpamList();

			break;

		case SIGUSR1:

			online = 1;

			if(slog)
			{
				syslog(LOG_DAEMON | LOG_INFO, "wakeup");
			}
			else
			{
				printf("TuxMailD <wakeup>\n");
			}
			
			break;
			
		case SIGUSR2:

			online = 0;
			
			if(slog)
			{
				syslog(LOG_DAEMON | LOG_INFO, "sleep");
			}
			else
			{
				printf("TuxMailD <sleep>\n");
			}
	}
}

/******************************************************************************
 * MainProgram
 ******************************************************************************/

int main(int argc, char **argv)
{
	char cvs_revision[] = "$Revision: 1.49 $";
	int param, nodelay = 0, account, mailstatus, unread_mailstatus;
	pthread_t thread_id;
	void *thread_result = 0;

	// check commandline parameter

		if(argc > 1)
		{
			for(param = 1; param < argc; param++)
			{
				if(!strcmp(argv[param], "-nodelay"))
				{
					nodelay = 1;
				}
				else if(!strcmp(argv[param], "-syslog"))
				{
					slog = 1;

					openlog("TuxMailD", LOG_ODELAY, LOG_DAEMON);
				}
				else if(!strcmp(argv[param], "-play"))
				{
					param++;
					PlaySound(argv[param]);
					return 0;
				}
				else if(!strcmp(argv[param], "-send"))
				{
					slog = 1;

					if(!ReadConf())
					{
						return -1;
					}
					param++;
					int account = atoi(argv[param]);
					param++;
					FILE* pipeout;
					pipeout = fopen(SMTPFILE,"w");
					if(pipeout == NULL)
					{
						return -1;
					}
					FILE* pipein;
					pipein = fopen(argv[param+1],"r");
					if(pipeout == NULL)
					{
						fclose(pipeout);
						return -1;
					}
	
					// check from:
					char *ptr1, *ptr2;
					char szFrom[80];
					char szSmtp[80];
					strcpy(szSmtp,account_db[account].smtp);
					strcpy(szFrom,account_db[account].from);

					if( (ptr1=strchr(account_db[account].from,'<')) )
					{
						if( (ptr2=strchr(ptr1,'>')) )
						{
							*ptr2 = '\0';
							strcpy(szFrom,ptr1);	
						}
					}		
					// check to: (and ptr1 to result)
					if( (ptr1=strchr(argv[param],'<')) )
					{
						if( (ptr2=strchr(ptr1++,'>')) )
						{
							*ptr2 = '\0';
						}
					}		
					else
					{
						ptr1 = argv[param];
					}
					fprintf(pipeout,"%s\n",szSmtp);
					fputs("tuxmaild\n",pipeout);
					fprintf(pipeout,"<%s>\n",szFrom);
					fprintf(pipeout,"<%s>\n",ptr1);
					fprintf(pipeout,"From: %s\n",szFrom);
					fprintf(pipeout,"To: %s\n",argv[param]);
					
					char linebuffer[250];
					if( fgets(linebuffer, sizeof(linebuffer), pipein))
					{
						fprintf(pipeout,"Subject: %s\n",linebuffer);
					}
					while(fgets(linebuffer, sizeof(linebuffer), pipein))
					{
						fputs(linebuffer,pipeout);
					}	
					fputs("\n",pipeout);
					fclose(pipein);
					fclose(pipeout);
					SendMail(account);
					return 0;
				}
			}
		}

	// create daemon
		
		sscanf(cvs_revision, "%*s %s", versioninfo);

		time(&tt);
		strftime(timeinfo, 22, "%d.%m.%Y - %T", localtime(&tt));

		switch(fork())
		{
			case 0:

				slog ? syslog(LOG_DAEMON | LOG_INFO, "%s started [%s]", versioninfo, timeinfo) : printf("TuxMailD %s started [%s]\n", versioninfo, timeinfo);

				setsid();
				chdir("/");

				break;

			case -1:

				slog ? syslog(LOG_DAEMON | LOG_INFO, "%s aborted!", versioninfo) : printf("TuxMailD %s aborted!\n", versioninfo);

				return -1;

			default:

				exit(0);
		}

	// read, update or create config

		if(!ReadConf())
		{
			return -1;
		}

	// read spamlist

		ReadSpamList();

	// check for running daemon

		if((fd_pid = fopen(PIDFILE, "r+")))
		{
			fscanf(fd_pid, "%d", &pid);

			if(kill(pid, 0) == -1 && errno == ESRCH)
			{
				pid = getpid();

				rewind(fd_pid);
				fprintf(fd_pid, "%d", pid);
				fclose(fd_pid);
			}
			else
			{
				slog ? syslog(LOG_DAEMON | LOG_INFO, "Daemon already running with PID %d", pid) : printf("TuxMailD <Daemon already running with PID %d>\n", pid);

				fclose(fd_pid);

				return -1;
			}
		}
		else
		{
			pid = getpid();

			fd_pid = fopen(PIDFILE, "w");
			fprintf(fd_pid, "%d", pid);
			fclose(fd_pid);
		}

	// install sighandler

		if(signal(SIGTERM, SigHandler) == SIG_ERR)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "Installation of Signalhandler for TERM failed") : printf("TuxMailD <Installation of Signalhandler for TERM failed>\n");

			return -1;
		}

		if(signal(SIGHUP, SigHandler) == SIG_ERR)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "Installation of Signalhandler for HUP failed") : printf("TuxMailD <Installation of Signalhandler for HUP failed>\n");

			return -1;
		}

		if(signal(SIGUSR1, SigHandler) == SIG_ERR)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "Installation of Signalhandler for USR1 failed") : printf("TuxMailD <Installation of Signalhandler for USR1 failed>\n");

			return -1;
		}

		if(signal(SIGUSR2, SigHandler) == SIG_ERR)
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "Installation of Signalhandler for USR2 failed") : printf("TuxMailD <Installation of Signalhandler for USR2 failed>\n");

			return -1;
		}

	// install communication interface

		if(pthread_create(&thread_id, NULL, InterfaceThread, NULL))
		{
			slog ? syslog(LOG_DAEMON | LOG_INFO, "Interface-Thread failed") : printf("TuxMailD <Interface-Thread failed>\n");

			return -1;
		}

	// restore database

		if(savedb == 'Y')
		{
		    slog ? syslog(LOG_DAEMON | LOG_INFO, "restore Mail-DB") : printf("TuxMailD <restore Mail-DB>\n");

		    system("cp /var/tuxbox/config/tuxmail/tuxmail.[0-9] /tmp 2>/dev/null");
		}

	// remove any notification file
	
		unlink(NOTIFILE);
		
	// check accounts

		if(!nodelay)
		{
			sleep(startdelay);
		}

		do
		{
			if(online)
			{
				if(logging == 'Y' && logmode == 'S')
				{
					fclose(fopen(LOGFILE, "w"));
				}

				mailstatus = 0;
				unread_mailstatus = 0;
				if (!inPOPCmd)
				{
					inPOPCmd = 1;
					for(account = 0; account < accounts; account++)
					{
						if(CheckAccount(account))
						{
							slog ? syslog(LOG_DAEMON | LOG_INFO, "Account %d = %.3d(%.3d)/%.3d Mail(s) for %s", account, account_db[account].mail_new, account_db[account].mail_unread, account_db[account].mail_all - deleted_messages, account_db[account].name) : printf("TuxMailD <Account %d = %.3d(%.3d)/%.3d Mail(s) for %s>\n", account, account_db[account].mail_new, account_db[account].mail_unread, account_db[account].mail_all - deleted_messages, account_db[account].name);
	
							mailstatus += account_db[account].mail_new;
							unread_mailstatus += account_db[account].mail_unread;
						}
						else
						{
							slog ? syslog(LOG_DAEMON | LOG_INFO, "Account %d skipped", account) : printf("TuxMailD <Account %d skipped>\n", account);
						}
					}
					inPOPCmd = 0;
				}

				NotifyUser(mailstatus,unread_mailstatus);

			}

			sleep(intervall * 60);
		}
		while(intervall);

	// cleanup

		pthread_cancel(thread_id);
		pthread_join(thread_id, thread_result);

		if(savedb == 'Y')
		{
		    slog ? syslog(LOG_DAEMON | LOG_INFO, "backup Mail-DB") : printf("TuxMailD <backup Mail-DB>\n");

		    system("cp /tmp/tuxmail.[0-9] /var/tuxbox/config/tuxmail 2>/dev/null");
		}
		
		unlink(PIDFILE);

		time(&tt);
		strftime(timeinfo, 22, "%d.%m.%Y - %T", localtime(&tt));

		slog ? syslog(LOG_DAEMON | LOG_INFO, "%s closed [%s]", versioninfo, timeinfo) : printf("TuxMailD %s closed [%s]\n", versioninfo, timeinfo);

		closelog();

		return 0;
}
