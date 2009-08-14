/******************************************************************************
 *                        <<< TuxCal - Calendar Plugin >>>
 *                (c) Robert "robspr1" Spreitzer 2006 (robert.spreitzer@inode.at)
 *-----------------------------------------------------------------------------
 * $Log: tuxcal.h,v $
 * Revision 1.08  2009/03/11 20:42:21  rhabarber1848
 * remove SuSv3 legacy functions: http://tuxbox-forum.dreambox-fan.de/forum/viewtopic.php?p=364177#p364177
 *
 * Revision 1.07  2007/05/17 16:19:47  dbluelle
 * Make plugins compile with freeetype 2.1.x on dreambox (as needed for Neutrino on Dreambox)
 *
 * Revision 1.06  2006/03/05 15:59:37  robspr1
 * - use /tmp/keyboard.lck to signal decoding of the keyboard
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
 * Revision 1.02  2006/02/15 19:12:30  robspr1
 * first version in CVS
 *
 * Revision 1.01  2006/02/12 23:10:00  robspr1
 * - bugfix reading params POS_X and POS_Y (needed by the daemon)
 *
 * Revision 1.00  2006/01/31 14:37:48  robspr1
 * - first version
 ******************************************************************************/
// lots of code is from the tuxmail-project

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
#include <sys/time.h>
#include <time.h>
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H

#include <plugin.h>

#define SCKFILE "/tmp/tuxcald.socket"									//! socket-file, connection to daemon
#define RUNFILE "/var/etc/.tuxcald"										//! autostart-file for daemon
#define CFGPATH "/var/tuxbox/config/tuxcal/"					//! config-path
#define CFGFILE "tuxcal.conf"													//! config-file
#define EVTFILE "tuxcal.list"													//! database-file
#define KBLCKFILE "/tmp/keyboard.lck"									//! file to lock keyboard-conversion

//----------------------------------------------------
// OSD   different languages

int osdidx = 0;																				// actual used language

#define MAXOSD	2
 
char *days[7][MAXOSD] = {
	{ "Mo", "Mo" },
	{ "Di", "Tu" },
	{ "Mi", "We" },
	{ "Do", "Th" },
	{ "Fr", "Fr" },
	{ "Sa", "Sa" },
	{ "So", "So" }
};

char *monthmsg[12][MAXOSD] = {
	{ "Januar"    , "January" },
	{ "Februar"   , "February" },
	{ "März"      , "March" },
	{ "April"     , "April" },
	{ "Mai"       , "May" },
	{ "Juni"      , "Juni" },
	{ "Juli"      , "July" },
	{ "August"    , "August" },
	{ "September" , "September" },
	{ "Oktober"   , "October" },
	{ "November"  , "November" },
	{ "Dezember"  , "December" }
};

char* infomsg1[][MAXOSD] = {
	{ "Eintrag"               	, "entry" },
	{ "selektierten löschen?" 	, "delete selected?" },
	{ "neuen hinzufügen?"     	, "add new?" },
	{ "Änderungen übernehmen?"  , "save changes" },
	{ "Änderungen verwerfen?"  	, "lose changes" }
};

char *infotype[][MAXOSD] = {
	{ "Geburtstag"    , 	"Birthday" },
	{ "Einträge"      ,   "Entries" },
	{ "Zeitraum"      ,   "Period" },
	{ "Feiertag"      , 	"Holiday" }
};

char *infohelp[][MAXOSD] = {
	{ "löschen"       , "delete" },
	{ "markieren"     , "select" },
	{ "einfügen"      , "insert" },
	{ "bearbeiten"    , "edit" },
	{ "[OK]  Einträge anzeigen    [dBox/Menü]  Uhrzeit ein/ausblenden    [0]  heute" ,  "[OK]  show entrys    [dBox/menu]  show/hide clock    [0]  today" }
};

char *szEditBoxInfo[][MAXOSD] ={
	{ "Typ auswählen"							, "select event type" },
	{ "Jahreszahl ein/ausblenden" , "toogle year" },
	{ "Uhrzeit ein/ausblenden"    , "toogle time" },
	{ "Enddatum ein/ausblenden" 	, "toogle end-date" },
	{ "Tag  Monat  Jahr  Stunde Minute", "day  month  year   hour  minute" },
	{ "Startdatum"								, "start date" },
	{ "Enddatum"								, "end date" }
};

char *vdaysnames[][MAXOSD] = {
	{ "Ostersonntag"  				, "eastern sunday" },
	{ "Ostermontag"  			  	, "eastern monday" },
	{ "Christi Himmelfahrt" 	, "Christi Himmelfahrt" },
	{ "Pfingstsonntag"				, "Pfingsten" },
	{ "Pfingstmontag"					, "Pfingsten" },
	{ "Fronleichnam"					, "Fronleichnam" },
	{ "Aschermittwoch"				, "Aschermittwoch" },
	{ "Muttertag"							, "Muttertag" },
	{ "Sommerzeit"						, "summer time" },
	{ "Winterzeit"						, "winter time" },
	{ "heiliger Abend"				, "christmas" },
	{ "1. Weihnachtsfeiertag"	, "christmas" },
	{ "2. Weihnachtsfeiertag"	, "christmas" },
	{ "heil. 3 Könige"				, "three kings" },
	{ "Neujahr"								, "new year" },
	{ "Sylvester"							, "sylvester" },
	{ "Valentinstag"					, "valentine" },
	{ "Mai-/Staatsfeiertag"		, "first may" },
	{ "Maria Himmelfahrt"			, "Maria Himmelfahrt" },
	{ "Nikolaus"							, "st. claus" },
	{ "Rosenmontag"						, "Rosenmontag" },
	{ "Gründonnerstag"				, "Gründonnerstag" },
	{ "Karfreitag"						, "Karfreitag" },
	{ "D: Tag der Einheit"		, "D: Tag der Einheit" },
	{ "Ö: Nationalfeiertag"		, "Ö: Nationalfeiertag" }
};

// ShowMessage output
enum {NODAEMON, STARTDONE, STARTFAIL, STOPDONE, STOPFAIL, BOOTON, BOOTOFF, DATE, CLOCKFAIL, CLOCKOK, INFO};
char *infomsg[][MAXOSD] = {
	{ "Daemon ist nicht geladen!" , "Daemon not running!" },
	{ "Abfrage wurde gestartet."  , "Polling started." },
	{ "Start ist fehlgeschlagen!" , "Start failed!" },
	{ "Abfrage wurde gestoppt."   , "Polling stopped." },
	{ "Stop ist fehlgeschlagen!"  , "Stop failed!" },
	{ "Autostart aktiviert."      , "Autostart enabled." },
	{ "Autostart deaktiviert."    , "Autostart disabled." },
	{ "%d.%m.%Y %H:%M:%S"         , "%m/%d/%Y %H:%M:%S" },
	{ "Uhr ist fehlgeschlagen!"   , "Clock failed!" },
	{ "Uhranzeige umgeschaltet."  , "displaying clock changed" }
};


//----------------------------------------------------
// remote-control and keyboard

#if HAVE_DVB_API_VERSION == 3
struct input_event ev;													//! input event for dBox
#else
unsigned char kbcode;														//! keyboard-input for Dreambox
char tch[100];																	//! variable for keyboard-reading
#endif

unsigned short rccode;													//! remote-control code

// rc codes

#if HAVE_DVB_API_VERSION == 1										// Dreambox-codes

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

// defines for pressing 0 - 9 on remote-control
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

// defines for remote-control and keyboard
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

#define REPKEYDELAY	4

#if HAVE_DVB_API_VERSION == 3												// only for dBox
// conversion-tables for infrared-keyboard 
// normal key
const int rctable[] = 
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
// shift-key also pressed
const int rcshifttable[] = 
{
   0x00, RC_ESC, '!', '"', '§', '$', '%', '&', '/', '(', ')', '=', '?', '`', 0x08, 0x09,
   'Q',  'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 'Ü', '*', RC_RET1, RC_STRG, 'A', 'S',
   'D',  'F', 'G', 'H', 'J', 'K', 'L', 'Ö', 'Ä', '°', RC_LSHIFT, 0x27, 'Y', 'X', 'C', 'V',
   'B',  'N', 'M', ';', ':', '_', RC_RSHIFT, 0x00, RC_ALT, 0x20, RC_CAPSLOCK,RC_F1,RC_F2,RC_F3,RC_F4,RC_F5,
   RC_F6,RC_F7,RC_F8,RC_F9,RC_F10,RC_NUM,RC_ROLLEN,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, RC_STANDBY, 0x00, 0x00, 0x00, 0x00, '>'
};
// alt-gr-key also pressed
const int rcaltgrtable[] = 
{
   0x00, RC_ESC, 0x00, '²', '³', 0x00, 0x00, 0x00, '{', '[', ']', '}', '\\', 0x00, 0x00, 0x00,
   '@',  0x00, '€', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '~', RC_RET1, RC_STRG, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, RC_LSHIFT, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00,  0x00, 'µ', 0x00, 0x00, 0x00, RC_RSHIFT, 0x00, RC_ALT, 0x20, RC_CAPSLOCK,RC_F1,RC_F2,RC_F3,RC_F4,RC_F5,
   RC_F6,RC_F7,RC_F8,RC_F9,RC_F10,RC_NUM,RC_ROLLEN,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, RC_STANDBY, 0x00, 0x00, 0x00, 0x00, '|'
};
#endif

// displaying function-keys
#define KEYBOX_KEYS 12

char *szKeyBoxInfo[KEYBOX_KEYS] = {
  " @!""#$%&'()*+-./[]\\1"   , "ABC2abc" , "DEF3def" ,
  "GHI4ghi" , "JKL5jkl" , "MNO6mno" ,
  "PQRS7pqrs", "TUV8tuv" , "WXYZ9wxyz",
  "0"    ,     "" ,  "" } ;

char *szKeyBoxKey[KEYBOX_KEYS] = {
  "1" , "2" , "3" ,
  "4" , "5" , "6" ,
  "7" , "8" , "9",
  "0" , "+" , "-" } ;

char *szKeyBBoxInfo[KEYBOX_KEYS][MAXOSD] = {
  { "red"   , "ROT" }  , { "OK"  , "OK" }   , { "entf." , "clr ln" },
  { "green" , "GRÜN" } , { "HOME", "HOME" } , { "leeren", "clr all" } ,
  { "yellow", "GELB" } , { "Anf.", "pos1" } , { "plus"  , "plus" },
  { "blue"  , "BLAU" } , { "Ende", "end" }  , { "minus" , "minus"}
} ;

#if HAVE_DVB_API_VERSION == 1
char *szKeyBBoxKey[KEYBOX_KEYS] = {
  "F1" , "" , "" ,
  "F2" , "" , "" ,
  "M1" , "" , "P+",
  "M2" , "" , "P-" } ;
#else
char *szKeyBBoxKey[KEYBOX_KEYS] = {
  "F1" , "F5" , "F9" ,
  "F2" , "F6" , "F10" ,
  "F3" , "F7" , "Pg+",
  "F4" , "F8" , "Pg-" } ;
#endif

const char *szDirectStyle[4] = {
"ABC", "Abc", "abc", "keyboard" };



//----------------------------------------------------
// calendar calculations

// for calculation the day of the week
const int monthcode[12] = {
	6, 	2, 	2, 	5, 	0, 	3, 	5, 	1, 	4, 	6, 	2, 	4
};

// days per month
const int monthdays[2][12] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },								// Normal years.
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }								// Leap years. 
};

// How many days come before each month (0-12).
const int __mon_yday[2][13] =
  {    
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },  // Normal years.
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }	 // Leap years.  
  };
  

//----------------------------------------------------
// functions

void ShowMessage(int message);
int IsEvent(int day, int month, int year);
int WeekNumber( int y, int m, int d );
int LeapYear(int year);

//----------------------------------------------------
// freetype stuff

#define FONT FONTDIR "/pakenham.ttf"

// definitions for string-rendering and size
enum {LEFT, CENTER, RIGHT, FIXEDLEFT, FIXEDCENTER, FIXEDRIGHT};
enum {SMALL, NORMAL, BIG};

FT_Library		library;
FTC_Manager		manager;
FTC_SBitCache		cache;
FTC_SBit		sbit;
#if FREETYPE_MAJOR  == 2 && FREETYPE_MINOR == 0
FTC_ImageDesc		desc;
#else
FTC_ImageTypeRec	desc;
#endif
FT_Face			face;
FT_UInt			prev_glyphindex;
FT_Bool			use_kerning;

//----------------------------------------------------
// config
char osd = 'G';														//! OSD language
int skin = 1;															//! which skin to use
int startdelay = 30;											//! startdelay for daemon to check event
char logging = 'Y';												//! logging to file
char audio = 'Y';													//! signal event per audio
int video=1;															//! signal event per video (different types)
int sigtype=1;														//! signal type 
int sigmode=0;														//! signal mode
char sigtime[80];													//! fix signal-times
int webport=80;														//! webport for using webinterface
char webuser[32] = "";										//! for using webinterface
char webpass[32] = "";										//! for using webinterface
char disp_date = 'N';											//! display the date
char disp_clock = 'Y';										//! display the clock
char disp_sec = 'Y';											//! display the second
char disp_size = 'S';											//! display size 'S'mall, 'N'ormal, 'B'ig
int disp_color = 1;												//! display color
int disp_back = 2;												//! display back-color
int disp_detect = 1;											//! detect color-map
char disp_mail = 'Y';											//! display mail notification
int cstartx = 500;												//! x position for displaying daemon-clock
int cstarty = 30;													//! y position for displaying daemon-clock
char show_clock = 'Y';										//! show the clock

char *szFmtStr[] = {
	"%02u" , "%04u"
};

//----------------------------------------------------
//----------------------------------------------------
// defines for setting the output
#define FONTSIZE_SMALL  24
#define FONTSIZE_NORMAL 32
#define FONTSIZE_BIG 		40

#define MAXSCREEN_X	560
#define MAXSCREEN_Y	500
#define MSGBOX_SX		145
#define MSGBOX_SY		175
#define MSGBOX_EX		455
#define MSGBOX_EY		325

#define GRIDLINE				32
#define GRIDLINE_SMALL	24
#define GRIDBOX_X				(MAXSCREEN_X/7)
#define GRIDBOX_CY1			420
#define GRIDBOX_CY2			300

#define LNWIDTH					2

#define TEXTWIDTH  				(MSGBOX_EX-MSGBOX_SX-4)
#define HEADERSTART  			(MSGBOX_SY+FONTSIZE_BIG+2)
#define HEADERTEXTSTART  	(HEADERSTART-7)
#define TEXTSTART 				((MSGBOX_EY-HEADERSTART)/2-7+HEADERSTART)
#define BUTTONSY  				(MSGBOX_EY-FONTSIZE_SMALL-14)
#define BUTTONX   				50
#define BUTTONSX  				(((MSGBOX_EX-MSGBOX_SX)-3*BUTTONX)/2 + MSGBOX_SX)
#define GRIDCAL						(GRIDLINE+GRIDLINE_SMALL)
#define GRIDLINE_INFO			((GRIDBOX_CY1-GRIDBOX_CY2)/4)

#define KEYBOX_SPACE  5
#define KEYBOX_HEIGHT 25
#define KEYBOX_WIDTH  90

#define EDITFOOTER_Y	(MAXSCREEN_Y-4*(KEYBOX_HEIGHT+KEYBOX_SPACE)-2*KEYBOX_SPACE)
#define EDITX					20

//----------------------------------------------------
//----------------------------------------------------
// defines for database
#define MAXINFOLEN				80
#define MAXENTRYS					500
#define MAXPERDAY					10
#define MAXINFOEDITLEN		64

//----------------------------------------------------
#define DAEMON_ON_NOSIGNAL		0
#define DAEMON_ON_SIGNAL			1
#define DAEMON_OFF						2
//----------------------------------------------------
// variables
struct tm *at;												//! actual time
time_t tt;														//! actual time
int tShow_year;												//! year to show
int tShow_mon;												//! month to show
int tShow_day;												//! day to show
int iEventType[MAXPERDAY];						//! structure filled with event-index by IsEvent()
int iCntEntries;											//! total number of entries in database
int nEditStyle = 1;										//! style for editing (RC, KB)
int intervall;												//! update clock-info every x seconds
char online;													//! are we connected to the daemon
char versioninfo_p[12];								//! plugin version
char versioninfo_d[12] = "?.??";			//! daemon version

//----------------------------------------------------
// database for all events
enum {FREE, BIRTHDAY, EVENT, PERIOD, HOLIDAY, COMMENT, UNUSED, SPACE};
typedef struct tagEVT_DB
{
	int type;										//! type of event: BIRTHDAY, EVENT, PERIOD, HOLIDAY or FREE (not used)
	int year;										//! year for the event , for birthday the birth-year, 0 for all years
	int month;									//! month of the event
	int day;										//! day of the event
	int hour;										//! hour of the event, -1 for all-day event
	int min;										//! minute of the event, ignore if hour == -1
	int days;										//! days since 1.1.
	int eyear;									//! end-year for the event 
	int emonth;									//! end-month of the event
	int eday;										//! end-day of the event
	int ehour;									//! end-hour of the event, -1 for all-day event
	int emin;										//! end-minute of the event, ignore if hour == -1
	int edays;									//! days since 1.1.
	char info[MAXINFOLEN];			//! info for the event
} EVT_DB, *PEVT_DB;

EVT_DB eventdb[MAXENTRYS];

#define OFFSET_E	1						//! index for eastern
#define OFFSET_EM	2						//! index for eastern
#define OFFSET_H	3						//! index for "christi himmelfahrt"
#define OFFSET_P	4						//! index for "pfingsten"
#define OFFSET_PM	5						//! index for "pfingsten"
#define OFFSET_F	6						//! index for "fronleichnam"
#define OFFSET_A	7						//! aschermittwoch
#define OFFSET_M	8						//! Muttertag
#define OFFSET_SZ 9						//! Sommerzeit
#define OFFSET_WZ 10					//! Winterzeit
#define OFFSET_W0 11					//! heiliger abend
#define OFFSET_W1 12					//! 1. Weihnachtstag
#define OFFSET_W2 13					//! 2. Weihnachtstag
#define OFFSET_3K 14					//! hl. 3 Koenige
#define OFFSET_N 	15					//! Neujahr
#define OFFSET_S 	16					//! Silvester
#define OFFSET_V	17					//! Valentinstag
#define OFFSET_1M	18					//! 1. may
#define OFFSET_MH	19					//! maria himmelfahrt
#define OFFSET_NI	20					//! nikolaus
#define OFFSET_RM	21					//! rosenmontag
#define OFFSET_GD	22					//! gruendonnerstag
#define OFFSET_KF	23					//! karfreitag
#define OFFSET_ND	24					//! tag der deutschen einheit
#define OFFSET_NA	25					//! nationalfeiertag oesterreich
#define NOF_VDAYS	25


// structure for the christian holidays in a year
typedef struct tagVariableDays
{
	int mon; 										// month
	int day;										// year
} VARIABLEDAY, *PVARIABLEDAY;

VARIABLEDAY varaibledays[NOF_VDAYS];

// daemon commands
enum {GET_STATUS, SET_STATUS, GET_VERSION, RELOAD_DB, TOGGLE_CLOCK};

//----------------------------------------------------
// devs
int fb, rc, kb, lcd;

//----------------------------------------------------
// framebuffer stuff
enum {FILL, GRID};
enum {TRANSP, WHITE, SKIN0, SKIN1, SKIN2, ORANGE, GREEN, YELLOW, RED, BLUE, GREY, DAY1, DAY2, DAY3, DAY4, DAY5, SKIN3, BLACK, LGREY, MAGENTA};

unsigned char *lfb = 0, *lbb = 0;
int prev_bpp;

struct fb_fix_screeninfo fix_screeninfo;
struct fb_var_screeninfo var_screeninfo;

//                      WHITE     SKIN0    SKIN1    SKIN2   ORANGE   GREEN    YELLOW     RED     BLUE     GREY    DAY1     DAY2     DAY3     DAY4     DAY5     SKIN3    BLACK    LGREY   MAGENTA
unsigned short rd1[] = {0xFF<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0x00<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0xB0<<8, 0xFF<<8, 0xFF<<8, 0xB0<<8, 0x00<<8, 0xFF<<8, 0x50<<8, 0x00<<8, 0x50<<8, 0xFF<<8};
unsigned short gn1[] = {0xFF<<8, 0x00<<8, 0x40<<8, 0x80<<8, 0xC0<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0x00<<8, 0xB0<<8, 0x50<<8, 0xC0<<8, 0xB0<<8, 0xFF<<8, 0xFF<<8, 0x50<<8, 0x00<<8, 0x50<<8, 0x00<<8};
unsigned short bl1[] = {0xFF<<8, 0x80<<8, 0x80<<8, 0xFF<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0xB0<<8, 0x50<<8, 0x00<<8, 0xB0<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0x00<<8, 0x50<<8, 0xFF<<8};
unsigned short tr1[] = {0x0000,  0x0A00,  0x0A00,  0x0A00,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000 };
struct fb_cmap colormap1 = {1, 19, rd1, gn1, bl1, tr1};

unsigned short rd2[] = {0xFF<<8, 0x25<<8, 0x4A<<8, 0x97<<8, 0xFF<<8, 0x00<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0xB0<<8, 0xFF<<8, 0xB0<<8, 0x50<<8, 0x50<<8, 0x50<<8, 0x50<<8, 0x00<<8, 0x50<<8, 0xFF<<8};
unsigned short gn2[] = {0xFF<<8, 0x3A<<8, 0x63<<8, 0xAC<<8, 0xC0<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0x00<<8, 0xB0<<8, 0xB0<<8, 0xB0<<8, 0x50<<8, 0x75<<8, 0x98<<8, 0x50<<8, 0x00<<8, 0x50<<8, 0x00<<8};
unsigned short bl2[] = {0xFF<<8, 0x4D<<8, 0x77<<8, 0xC1<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0xB0<<8, 0xFF<<8, 0xFF<<8, 0xB0<<8, 0xFF<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0x50<<8, 0xFF<<8};
unsigned short tr2[] = {0x0000,  0x0A00,  0x0A00,  0x0A00,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000 };
struct fb_cmap colormap2 = {1, 19, rd2, gn2, bl2, tr2};

unsigned short rd3[] = {0xFF<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0x00<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0xB0<<8, 0xFF<<8, 0x50<<8, 0xB0<<8, 0x00<<8, 0xFF<<8, 0x50<<8, 0x00<<8, 0x50<<8, 0xFF<<8};
unsigned short gn3[] = {0xFF<<8, 0x00<<8, 0x00<<8, 0x80<<8, 0xC0<<8, 0xFF<<8, 0xFF<<8, 0x00<<8, 0x00<<8, 0xB0<<8, 0x50<<8, 0xFF<<8, 0xB0<<8, 0xFF<<8, 0xFF<<8, 0x50<<8, 0x00<<8, 0x50<<8, 0x00<<8};
unsigned short bl3[] = {0xFF<<8, 0x00<<8, 0x80<<8, 0xFF<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0xB0<<8, 0x50<<8, 0x50<<8, 0xB0<<8, 0x00<<8, 0x00<<8, 0xFF<<8, 0x00<<8, 0x50<<8, 0xFF<<8};
unsigned short tr3[] = {0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000,  0x0000 };
struct fb_cmap colormap3 = {1, 19, rd3, gn3, bl3, tr3};

int startx, starty, sx, ex, sy, ey;

//----------------------------------------------------
// object to render
enum {OBJ_CIRCLE, OBJ_HEART, OBJ_MARKER, OBJ_SCROLLUP, OBJ_SCROLLDN, OBJ_CLOCK};
#define OBJ_SX	15											// lines for object
#define OBJ_SY	15											// columns for object

char scroll_up[] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

char scroll_dn[] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,1,0,0,0,0,0,0
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

char heart[] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,1,1,1,0,0,0,1,1,1,0,0,0,
	0,0,1,1,1,1,1,0,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

char marker[] =
{
	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0
};

char symbolclock[] =
{
//	0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
	0,1,1,0,0,0,1,1,1,0,0,0,1,1,0,
	0,1,1,1,0,1,1,1,1,1,0,1,1,1,0,
	0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,0,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,0,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,
	1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,0,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,0,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0
//	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0
};

