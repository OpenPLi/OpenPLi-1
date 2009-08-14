/******************************************************************************
 *                        <<< TuxMail - Mail Plugin >>>
 *                (c) Thomas "LazyT" Loewe 2003 (LazyT@gmx.net)
 *-----------------------------------------------------------------------------
 * $Log: tuxmail.h,v $
 * Revision 1.35  2007/05/17 16:19:48  dbluelle
 * Make plugins compile with freeetype 2.1.x on dreambox (as needed for Neutrino on Dreambox)
 *
 * Revision 1.34  2006/03/05 16:02:13  robspr1
 * - use /tmp/keyboard.lck to signal decoding of the keyboard
 *
 * Revision 1.33  2005/11/19 14:37:48  robspr1
 * - add different behaviour in marking mails green in the plugin
 *
 * Revision 1.32  2005/11/11 18:40:34  robspr1
 * - /tmp/tuxmail.new holds number of new files /  reread tuxmail.conf after writing
 *
 * Revision 1.31  2005/11/04 16:00:06  robspr1
 * - adding IMAP support
 *
 * Revision 1.30  2005/08/19 19:00:35  robspr1
 * - add pin protection for config GUI
 *
 * Revision 1.29  2005/08/19 09:00:23  robspr1
 * - add 3rd skin, bugfix config GUI
 *
 * Revision 1.28  2005/08/18 23:21:05  robspr1
 * - add config GUI (DBOX key)
 *
 * Revision 1.27  2005/06/19 17:21:50  robspr1
 * - dreambox tastatur now working
 *
 * Revision 1.26  2005/06/08 21:56:31  robspr1
 * - minor fixes for mail writing; - using dreambox keyboard?
 *
 * Revision 1.25  2005/06/07 19:18:07  robspr1
 * -change ENTER for dBox Keyboard; -simple zoom for mail viewer
 *
 * Revision 1.24  2005/06/06 21:28:03  robspr1
 * using dBox Keyboard
 *
 * Revision 1.23  2005/05/20 18:01:24  lazyt
 * - Preparation for Keyboard
 * - don't try add to Spamlist for empty Account
 *
 * Revision 1.22  2005/05/19 10:04:16  robspr1
 * - add cached mailreading
 *
 * Revision 1.21  2005/05/17 20:40:17  robspr1
 * - add addressbook to mailwriter
 *
 * Revision 1.20  2005/05/14 18:54:40  robspr1
 * - Bugfix Mailreader - Mailwriter SMS style
 *
 * Revision 1.19  2005/05/14 08:59:51  lazyt
 * - fix Spamfunction
 * - new Keydefinitions: RED=delete Mail, GREEN=send Mail, YELLOW=read Mail, ?=About (DBOX reserved for Configmenu)
 *
 * Revision 1.18  2005/05/13 23:16:57  robspr1
 * - first Mail writing GUI\n- add parameters for Mail sending
 *
 * Revision 1.17  2005/05/12 22:24:23  lazyt
 * - PIN-Fix
 * - add Messageboxes for send Mail done/fail
 *
 * Revision 1.16  2005/05/12 14:28:28  lazyt
 * - PIN-Protection for complete Account
 * - Preparation for sending Mails ;-)
 *
 * Revision 1.15  2005/05/11 19:00:38  robspr1
 * minor Mailreader changes / add to Spamlist undo
 *
 * Revision 1.14  2005/05/11 12:01:21  lazyt
 * Protect Mailreader with optional PIN-Code
 *
 * Revision 1.13  2005/05/10 12:55:15  lazyt
 * - LCD-Fix for DM500
 * - Autostart for DM7020 (use -DOE, put Init-Script to /etc/init.d/tuxmail)
 * - try again after 10s if first DNS-Lookup failed
 * - don't try to read Mails on empty Accounts
 *
 * Revision 1.12  2005/05/09 19:32:32  robspr1
 * support for mail reading
 *
 * Revision 1.11  2005/04/29 17:24:00  lazyt
 * use 8bit audiodata, fix skin and osd
 *
 * Revision 1.10  2005/03/28 14:14:14  lazyt
 * support for userdefined audio notify (put your 12/24/48KHz pcm wavefile to /var/tuxbox/config/tuxmail/tuxmail.wav)
 *
 * Revision 1.9  2005/03/24 13:15:21  lazyt
 * cosmetics, add SKIN=0/1 for different osd-colors
 *
 * Revision 1.8  2005/03/22 13:31:46  lazyt
 * support for english osd (OSD=G/E)
 *
 * Revision 1.7  2005/03/22 09:35:20  lazyt
 * lcd support for daemon (LCD=Y/N, GUI should support /tmp/lcd.locked)
 *
 * Revision 1.6  2005/02/26 10:23:48  lazyt
 * workaround for corrupt mail-db
 * add ADMIN=Y/N to conf (N to disable mail deletion via plugin)
 * show versioninfo via "?" button
 * limit display to last 100 mails (increase MAXMAIL if you need more)
 *
 * Revision 1.5  2004/07/10 11:38:14  lazyt
 * use -DOLDFT for older FreeType versions
 * replaced all remove() with unlink()
 *
 * Revision 1.4  2004/06/23 11:05:04  obi
 * compile fix
 *
 * Revision 1.3  2004/04/24 22:24:23  carjay
 * fix compiler warnings
 *
 * Revision 1.2  2003/05/16 15:07:23  lazyt
 * skip unused accounts via "plus/minus", add mailaddress to spamlist via "blue"
 *
 * Revision 1.1  2003/04/21 09:24:52  lazyt
 * add tuxmail, todo: sync (filelocking?) between daemon and plugin
 ******************************************************************************/

#include "config.h"

#if !defined(HAVE_DVB_API_VERSION) && defined(HAVE_OST_DMX_H)
#define HAVE_DVB_API_VERSION 1
#endif

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/fb.h>
#if HAVE_DVB_API_VERSION == 3
#include <linux/input.h>
#endif
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H

#include <plugin.h>

#define SCKFILE "/tmp/tuxmaild.socket"
#define LCKFILE "/tmp/lcd.locked"
#define RUNFILE "/var/etc/.tuxmaild"
#define CFGPATH "/var/tuxbox/config/tuxmail/"
#define CFGFILE "tuxmail.conf"
#define SPMFILE "spamlist"
#define POP3FILE "/tmp/tuxmail.pop3"
#define SMTPFILE "/tmp/tuxmail.smtp"
#define NOTIFILE "/tmp/tuxmail.new"
#define TEXTFILE "/var/tuxbox/config/tuxmail/mailtext"
#define ADDRFILE "/var/tuxbox/config/tuxmail/tuxmail.addr"
#define T9FILE   "/var/tuxbox/config/tuxmail/tuxmail.t9"
#define KBLCKFILE "/tmp/keyboard.lck"										//! file to lock keyboard-conversion

#define OE_START "/etc/rc2.d/S99tuxmail"
#define OE_KILL0 "/etc/rc0.d/K00tuxmail"
#define OE_KILL6 "/etc/rc6.d/K00tuxmail"

#define MAXMAIL 100

// rc codes

#if HAVE_DVB_API_VERSION == 1

#define	RC1_0		0x5C00
#define	RC1_1		0x5C01
#define	RC1_2		0x5C02
#define	RC1_3		0x5C03
#define	RC1_4		0x5C04
#define	RC1_5		0x5C05
#define	RC1_6		0x5C06
#define	RC1_7		0x5C07
#define	RC1_8		0x5C08
#define	RC1_9		0x5C09
#define	RC1_STANDBY	0x5C0C
#define	RC1_UP		0x5C0E
#define	RC1_DOWN	0x5C0F
#define	RC1_PLUS	0x5C16
#define	RC1_MINUS	0x5C17
#define	RC1_HOME	0x5C20
#define	RC1_DBOX	0x5C27
#define	RC1_MUTE	0x5C28
#define	RC1_RED		0x5C2D
#define	RC1_RIGHT	0x5C2E
#define	RC1_LEFT	0x5C2F
#define	RC1_OK		0x5C30
#define	RC1_BLUE	0x5C3B
#define	RC1_YELLOW	0x5C52
#define	RC1_GREEN	0x5C55
#define	RC1_HELP	0x5C82

// kb codes

#define KEY_0		0x5C00
#define KEY_1		0x5C01
#define KEY_2		0x5C02
#define KEY_3		0x5C03
#define KEY_4		0x5C04
#define KEY_5		0x5C05
#define KEY_6		0x5C06
#define KEY_7		0x5C07
#define KEY_8		0x5C08
#define KEY_9		0x5C09
#define KEY_POWER	0x5C0C
#define KEY_UP		0x5C0E
#define KEY_DOWN	0x5C0F
#define KEY_VOLUMEUP	0x5C16
#define KEY_VOLUMEDOWN	0x5C17
#define KEY_HOME	0x5C20
#define KEY_SETUP	0x5C27
#define KEY_MUTE	0x5C28
#define KEY_RED		0x5C2D
#define KEY_RIGHT	0x5C2E
#define KEY_LEFT	0x5C2F
#define KEY_OK		0x5C30
#define KEY_BLUE	0x5C3B
#define KEY_YELLOW	0x5C52
#define KEY_GREEN	0x5C55
#define KEY_HELP	0x5C82

#endif

#define	RC_0			'0'
#define	RC_1			'1'
#define	RC_2			'2'
#define	RC_3			'3'
#define	RC_4			'4'
#define	RC_5			'5'
#define	RC_6			'6'
#define	RC_7			'7'
#define	RC_8			'8'
#define	RC_9			'9'

#define	RC_RIGHT	0x0191
#define	RC_LEFT		0x0192
#define	RC_UP			0x0193
#define	RC_DOWN		0x0194
#define	RC_PLUS		0x0195
#define	RC_MINUS	0x0196

#define	RC_OK				0x0D
#define	RC_STANDBY	0x1C
#define RC_ESC			RC_HOME

#define	RC_HOME			0x01B1
#define	RC_MUTE			0x01B2
#define	RC_HELP			0x01B3
#define	RC_DBOX			0x01B4

#define	RC_GREEN	0x01A1
#define	RC_YELLOW	0x01A2
#define	RC_RED		0x01A3
#define	RC_BLUE		0x01A4

#define RC_PAUSE	RC_HELP
#define RC_ALTGR	0x12
#define RC_BS			0x08
#define RC_POS1		RC_HOME
#define RC_END		0x13
#define RC_INS		0x10
#define RC_ENTF		0x11
#define RC_STRG		0x00
#define RC_LSHIFT	0x0E
#define RC_RSHIFT	0x0E
#define RC_ALT		0x0F
#define RC_NUM		RC_DBOX
#define RC_ROLLEN	0x00
#define RC_F5			0x01C5
#define RC_F6			0x01C6
#define RC_F7			0x01C7
#define RC_F8			0x01C8
#define RC_F9			0x01C9
#define RC_F10		0x01CA
#define RC_RET		0x0D
#define RC_RET1		0x01CC
#define RC_CAPSLOCK	0x01CD
#define RC_ON			0x01CE

#define RC_F1		RC_RED
#define RC_F2		RC_GREEN
#define RC_F3		RC_YELLOW
#define RC_F4		RC_BLUE
#define RC_PAGEUP	RC_PLUS
#define RC_PAGEDOWN	RC_MINUS

int rctable[] = 
{
   0x00, RC_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'ß', '´', RC_BS, 0x09,
   'q',  'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'ü', '+', RC_RET, RC_STRG, 'a', 's',
   'd',  'f', 'g', 'h', 'j', 'k', 'l', 'ö', 'ä', '^', RC_LSHIFT, '#', 'y', 'x', 'c', 'v',
   'b',  'n', 'm', ',', '.', '-', RC_RSHIFT, 0x00, RC_ALT, 0x20, RC_CAPSLOCK,RC_F1,RC_F2,RC_F3,RC_F4,RC_F5,
   RC_F6,RC_F7,RC_F8,RC_F9,RC_F10,RC_NUM,RC_ROLLEN,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, RC_STANDBY, 0x00, 0x00, 0x00, 0x00, '<', RC_OK, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, RC_ALTGR, 0x00, RC_POS1, RC_UP, RC_PAGEUP, RC_LEFT, RC_RIGHT, RC_END, RC_DOWN,RC_PAGEDOWN,RC_INS,RC_ENTF,
   0x00, RC_MUTE, RC_MINUS, RC_PLUS, RC_STANDBY, 0x00, 0x00, RC_PAUSE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
int rcshifttable[] = 
{
   0x00, RC_ESC, '!', '"', '§', '$', '%', '&', '/', '(', ')', '=', '?', '`', 0x08, 0x09,
   'Q',  'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 'Ü', '*', RC_RET1, RC_STRG, 'A', 'S',
   'D',  'F', 'G', 'H', 'J', 'K', 'L', 'Ö', 'Ä', '°', RC_LSHIFT, 0x27, 'Y', 'X', 'C', 'V',
   'B',  'N', 'M', ';', ':', '_', RC_RSHIFT, 0x00, RC_ALT, 0x20, RC_CAPSLOCK,RC_F1,RC_F2,RC_F3,RC_F4,RC_F5,
   RC_F6,RC_F7,RC_F8,RC_F9,RC_F10,RC_NUM,RC_ROLLEN,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, RC_STANDBY, 0x00, 0x00, 0x00, 0x00, '>'
};
int rcaltgrtable[] = 
{
   0x00, RC_ESC, 0x00, '²', '³', 0x00, 0x00, 0x00, '{', '[', ']', '}', '\\', 0x00, 0x00, 0x00,
   '@',  0x00, '€', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '~', RC_RET1, RC_STRG, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, RC_LSHIFT, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00,  0x00, 'µ', 0x00, 0x00, 0x00, RC_RSHIFT, 0x00, RC_ALT, 0x20, RC_CAPSLOCK,RC_F1,RC_F2,RC_F3,RC_F4,RC_F5,
   RC_F6,RC_F7,RC_F8,RC_F9,RC_F10,RC_NUM,RC_ROLLEN,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, RC_STANDBY, 0x00, 0x00, 0x00, 0x00, '|'
};
	
// kb codes

#define KBC_UP		0x01
#define KBC_DOWN	0x02
#define KBC_RIGHT	0x03
#define KBC_LEFT	0x04
#define KBC_INS		0x05
#define KBC_DEL		0x06
#define KBC_POS1	0x07
#define KBC_BACKSPACE	0x08
#define KBC_END		0x0A
#define KBC_PAGEUP	0x0B
#define KBC_PAGEDOWN	0x0C
#define KBC_RETURN	0x0D

// defines for mail-reading/writing
 
#define BORDERSIZE 2
#define FONTHEIGHT_BIG 40
#define FONTHEIGHT_NORMAL 32
#define FONTHEIGHT_SMALL 24
#define FONT_OFFSET_BIG 10
#define FONT_OFFSET 5
#define VIEWX  619
#define VIEWY  504
#define INFOBOXY   125
#define KEYBOX_SPACE  5
#define KEYBOX_HEIGHT 25
#define KEYBOX_WIDTH  90


#define MAXINFOLINES 15
#define MAXLINELEN	 80

#define KEYBOX_KEYS 12

char *szKeyBoxInfo[KEYBOX_KEYS] = {
  " @!""#$%&'()*+-./[]\\1"   , "ABC2" , "DEF3" ,
  "GHI4" , "JKL5" , "MNO6" ,
  "PQRS7", "TUV8" , "WXYZ9",
  "0"    ,     "" ,  "" } ;

char *szKeyBoxKey[KEYBOX_KEYS] = {
  "1" , "2" , "3" ,
  "4" , "5" , "6" ,
  "7" , "8" , "9",
  "0" , "+" , "-" } ;

char *szKeyBBoxInfoEn[KEYBOX_KEYS] = {
  "red"   , "OK"   , "clr ln" ,
  "green" , "HOME" , "clr all" ,
  "yellow", "pos1" , "plus",
  "blue"  , "end"  ,  "minus" } ;

#if HAVE_DVB_API_VERSION == 1
char *szKeyBBoxInfoDe[KEYBOX_KEYS] = {
  "leeren"   , ""   , "" ,
  "senden" , "" , "" ,
  "entf.", "" , "plus",
  "einf."  , ""  ,  "minus" } ;

char *szKeyBBoxKey[KEYBOX_KEYS] = {
  "F1" , "" , "" ,
  "F2" , "" , "" ,
  "M1" , "" , "P+",
  "M2" , "" , "P-" } ;
#else
char *szKeyBBoxInfoDe[KEYBOX_KEYS] = {
  "ROT"   , "OK"   , "entf." ,
  "GRÜN" , "HOME" , "leeren" ,
  "GELB", "Anf." , "plus",
  "BLAU"  , "Ende"  ,  "minus" } ;

char *szKeyBBoxKey[KEYBOX_KEYS] = {
  "F1" , "F5" , "F9" ,
  "F2" , "F6" , "F10" ,
  "F3" , "F7" , "Pg+",
  "F4" , "F8" , "Pg-" } ;
#endif

char *szDirectStyle[4] = {
"ABC", "Abc", "abc", "keyboard" };

// functions

void ShowMessage(int message);
int CheckPIN(int Account);
void SaveAndReloadDB(int iSave);

// freetype stuff

#define FONT FONTDIR "/pakenham.ttf"

enum {LEFT, CENTER, RIGHT};
enum {SMALL, NORMAL, BIG};

FT_Library		library;
FTC_Manager		manager;
FTC_SBitCache		cache;
FTC_SBit		sbit;
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
FTC_ImageTypeRec	desc;
#else
FTC_ImageDesc		desc;
#endif
FT_Face			face;
FT_UInt			prev_glyphindex;
FT_Bool			use_kerning;

char versioninfo_p[12], versioninfo_d[12] = "?.??";

// config

char admin = 'Y';
char osd = 'G';
int skin = 1;
int startdelay = 30;
int intervall = 15;
char logging = 'Y';
char logmode = 'S';
char audio = 'Y';
char savedb = 'Y';
char lcdc = 'Y';
int typeflag = 1;
int video=1;
int webport=80;
char webuser[32] = "";
char webpass[32] = "";
char security[80] = "";
char configcode[5] = "";
int  configpincount = 0;

// mail database

struct mi
{
	char type[2];	/* N=new, O=old, D=del */
	char save[2];	/* save type for restore */
	char uid[80];
	char date[8];	/* 01.Jan */
	char time[6];	/* 00:00 */
	char from[256];
	char subj[256];
};

struct
{
	int inactive;
	int mails;
	char nr[2];	/* 0...9 */
	char time[6];	/* 00:00 */
	char name[32];
	char namebox[32];
	char status[8];	/* 000/000 */
	int pincount;
	char pop3[64];
	char imap[64];
	char user[64];
	char pass[64];
	char smtp[64];
	char from[64];
	char code[5];
	int  auth;
	char suser[64];
	char spass[64];
	char inbox[64];
	struct mi mailinfo[MAXMAIL];

}maildb[10];

// devs

int fb, rc, kb, lcd;

// daemon commands

enum {GET_STATUS, SET_STATUS, RELOAD_SPAMLIST, GET_VERSION, GET_MAIL, SEND_MAIL, RELOAD_CONFIG};

// framebuffer stuff

enum {FILL, GRID};
enum {TRANSP, WHITE, SKIN0, SKIN1, SKIN2, ORANGE, GREEN, YELLOW, RED};
enum {NODAEMON, STARTDONE, STARTFAIL, STOPDONE, STOPFAIL, BOOTON, BOOTOFF, ADD2SPAM, DELSPAM, SPAMFAIL, INFO, GETMAIL, GETMAILFAIL, SENDMAILDONE, SENDMAILFAIL};

unsigned char *lfb = 0, *lbb = 0;
int prev_bpp;

struct fb_fix_screeninfo fix_screeninfo;
struct fb_var_screeninfo var_screeninfo;


unsigned short rd1[] = {0xFF<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0x00<<8, 0xFF<<8, 0xFF<<8};
unsigned short gn1[] = {0xFF<<8, 0x00<<8, 0x40<<8, 0x80<<8, 0xC0<<8, 0xFF<<8, 0xFF<<8, 0x00<<8};
unsigned short bl1[] = {0xFF<<8, 0x80<<8, 0x80<<8, 0xFF<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0x00<<8};
unsigned short tr1[] = {0x0000,  0x0A00,  0x0A00,  0x0A00,  0x0000,  0x0000,  0x0000,  0x0000 };

struct fb_cmap colormap1 = {1, 8, rd1, gn1, bl1, tr1};

unsigned short rd2[] = {0xFF<<8, 0x25<<8, 0x4A<<8, 0x97<<8, 0xFF<<8, 0x00<<8, 0xFF<<8, 0xFF<<8};
unsigned short gn2[] = {0xFF<<8, 0x3A<<8, 0x63<<8, 0xAC<<8, 0xC0<<8, 0xFF<<8, 0xFF<<8, 0x00<<8};
unsigned short bl2[] = {0xFF<<8, 0x4D<<8, 0x77<<8, 0xC1<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0x00<<8};
unsigned short tr2[] = {0x0000,  0x0A00,  0x0A00,  0x0A00,  0x0000,  0x0000,  0x0000,  0x0000 };

struct fb_cmap colormap2 = {1, 8, rd2, gn2, bl2, tr2};

unsigned short rd3[] = {0xFF<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0x00<<8, 0xE8<<8, 0xFF<<8, 0xb0<<8, 0x00<<8, 0x50<<8, 0x00<<8, 0x50<<8, 0x00<<8};
unsigned short gn3[] = {0xFF<<8, 0x00<<8, 0x00<<8, 0x80<<8, 0xC0<<8, 0xd0<<8, 0xE8<<8, 0x00<<8, 0xb0<<8, 0xff<<8, 0x50<<8, 0x00<<8, 0x50<<8, 0x40<<8};
unsigned short bl3[] = {0xFF<<8, 0x00<<8, 0x80<<8, 0xFF<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0xb0<<8, 0x00<<8, 0x50<<8, 0x80<<8, 0x50<<8, 0xff<<8};
unsigned short tr3[] = {0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000 , 0x0000 , 0x0000 , 0x80ff , 0x80ff , 0x0000 };

struct fb_cmap colormap3 = {1, 14, rd3, gn3, bl3, tr3};

int startx, starty, sx, ex, sy, ey;
char online;
char mailfile;
char mailsend;
char maildir[256];
int mailcache = 0;
char szInfo[MAXINFOLINES][MAXLINELEN];

#if HAVE_DVB_API_VERSION == 3

struct input_event ev;

#endif

unsigned short rccode;
unsigned char kbcode;
int sim_key = 0;
char tch[100];


char scroll_up[] =
{
	SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0,
	SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0,
	SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0,
	SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0,
	WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
	WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE
};

char scroll_dn[] =
{
	WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
	WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
	SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0,
	SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0,
	SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0,
	SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0,
	SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, WHITE, WHITE, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0, SKIN0
};

char circle[] =
{
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0
};

// lcd stuff

unsigned char lcd_buffer[] =
{
	0xE0, 0xF8, 0xFC, 0xFE, 0xFE, 0xFF, 0x7F, 0x7F, 0x7F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0xFF, 0xFF, 0xFF, 0xC7, 0xBB, 0x3B, 0xFB, 0xFB, 0x3B, 0xBB, 0xC7, 0xFF, 0x07, 0xFB, 0xFB, 0x07, 0xFB, 0xFB, 0xFB, 0x07, 0xFF, 0x87, 0x7B, 0xFB, 0xC7, 0xFB, 0x7B, 0x7B, 0x87, 0xFF, 0x07, 0xFB, 0xFB, 0x3B, 0xFB, 0xFB, 0xFB, 0x3B, 0xFB, 0xFB, 0x07, 0xFF, 0x07, 0xFB, 0xFB, 0xBB, 0xFB, 0xFB, 0xFB, 0x07, 0xFF, 0x27, 0xDB, 0xDB, 0x27, 0xFF, 0x07, 0xFB, 0xFB, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFE, 0xFE, 0xFC, 0xF8, 0xE0,
	0xFF, 0x7F, 0x7F, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x70, 0x6F, 0x6F, 0x70, 0x7F, 0x7F, 0x7F, 0x70, 0x6F, 0x6F, 0x6C, 0x6F, 0x6F, 0x6F, 0x70, 0x7F, 0x70, 0x6F, 0x6F, 0x71, 0x6F, 0x6F, 0x6F, 0x70, 0x7F, 0x70, 0x6F, 0x6F, 0x70, 0x6F, 0x6F, 0x6F, 0x70, 0x6F, 0x6F, 0x70, 0x7F, 0x70, 0x6F, 0x6F, 0x71, 0x6F, 0x6F, 0x6F, 0x70, 0x7F, 0x70, 0x6F, 0x6F, 0x70, 0x7F, 0x70, 0x6F, 0x6F, 0x6C, 0x6D, 0x6D, 0x6D, 0x73, 0x7F, 0x7F, 0x7F, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7B, 0x7F, 0x7F, 0xFF,
	0xFF, 0x00, 0x00, 0x00, 0xF8, 0xFC, 0x04, 0xFC, 0xF8, 0x00, 0xFC, 0xFC, 0x30, 0x60, 0xFC, 0xFC, 0x00, 0xFC, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x40, 0xC0, 0x00, 0xC0, 0x40, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xC0, 0x20, 0x20, 0x20, 0x20, 0xC0, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x80, 0xC0, 0xE1, 0x31, 0x18, 0x09, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x77, 0x55, 0xDD, 0x00, 0xDD, 0x55, 0xDD, 0x00, 0xDD, 0x55, 0x77, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xE3, 0x14, 0x14, 0x14, 0x14, 0xE3, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x81, 0x83, 0x9F, 0x9E, 0x8F, 0x87, 0x81, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x81, 0x81, 0x81, 0x80, 0x81, 0x81, 0x81, 0x80, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x80, 0x80, 0xFF, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x80, 0x80, 0x81, 0x82, 0x82, 0x82, 0x82, 0x81, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0x00, 0xE0, 0x60, 0x60, 0xA0, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xA0, 0x60, 0x60, 0xE0, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x10, 0x10, 0x10, 0x10, 0xE0, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xE0, 0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x30, 0xE0, 0x00, 0x00, 0x00, 0xFF,
	0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x80, 0x40, 0x21, 0x11, 0x0A, 0x06, 0x04, 0x08, 0x08, 0x04, 0x06, 0x0A, 0x11, 0x21, 0x40, 0x80, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x3E, 0x01, 0x00, 0x00, 0xC0, 0x3E, 0x01, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF,
	0x07, 0x18, 0x20, 0x40, 0x43, 0x83, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x83, 0x83, 0x80, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x80, 0x80, 0x80, 0x83, 0x84, 0x84, 0x84, 0x84, 0x83, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x83, 0x80, 0x80, 0x80, 0x83, 0x86, 0x84, 0x84, 0x84, 0x84, 0x84, 0x84, 0x86, 0x43, 0x40, 0x20, 0x18, 0x07
};

char lcd_status[] =
{
	1,1,1,1,0,0,1,1,1,1,1,0,1,1,0,1,1,	/* PAU */
	1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,
	1,1,0,1,1,0,1,1,0,1,1,0,1,1,0,1,1,
	1,1,1,1,0,0,1,1,1,1,1,0,1,1,0,1,1,
	1,1,0,0,0,0,1,1,0,1,1,0,1,1,0,1,1,
	1,1,0,0,0,0,1,1,0,1,1,0,1,1,0,1,1,
	1,1,0,0,0,0,1,1,0,1,1,0,1,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
	0,0,0,0,0,0,0,1,1,0,1,1,1,0,0,0,0,
	0,0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,
	0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,

	0,1,1,1,0,0,1,1,0,0,1,1,0,1,1,0,0,	/* ONL */
	1,1,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,
	1,1,0,1,1,0,1,1,1,0,1,1,0,1,1,0,0,
	1,1,0,1,1,0,1,1,1,1,1,1,0,1,1,0,0,
	1,1,0,1,1,0,1,1,0,1,1,1,0,1,1,0,0,
	1,1,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,
	0,1,1,1,0,0,1,1,0,0,1,1,0,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
	0,0,0,1,1,1,0,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,

	0,1,1,1,0,0,1,1,1,1,1,1,0,1,1,0,0,	/* OFL */
	1,1,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,
	1,1,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,
	1,1,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,
	1,1,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,
	1,1,0,1,1,0,0,1,1,0,0,0,0,1,1,0,0,
	0,1,1,1,0,0,0,1,1,0,0,0,0,1,1,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,0,1,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,1,0,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,1,1,1,0,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

char lcd_digits[] =
{
	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,0,0,1,1,1,1,0,0,0,
	0,0,1,1,0,0,1,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,1,0,0,1,1,0,0,
	0,0,0,1,1,1,1,0,0,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,1,1,1,1,0,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,0,1,1,1,1,0,
	1,1,0,1,1,1,0,0,1,1,
	1,0,0,0,1,0,0,0,0,1,
	1,0,0,0,1,0,0,0,0,1,
	1,0,0,0,1,0,0,0,0,1,
	1,0,0,0,1,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,1,0,0,1,1,
	0,0,0,0,0,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,1,1,1,1,0,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,1,1,1,1,0,
	1,0,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,0,0,0,0,1,
	0,0,0,0,1,1,0,0,1,1,
	0,0,0,0,0,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,0,0,0,0,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,

	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,1,1,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,1,1,
	0,1,1,1,1,1,1,1,1,0,
};
