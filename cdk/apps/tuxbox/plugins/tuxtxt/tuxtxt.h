/******************************************************************************
 *                      <<< TuxTxt - Teletext Plugin >>>                      *
 *                                                                            *
 *             (c) Thomas "LazyT" Loewe 2002-2003 (LazyT@gmx.net)             *
 *                                                                            *
 *    continued 2004-2005 by Roland Meier <RolandMeier@Siemens.com>           *
 *                       and DBLuelle <dbluelle@blau-weissoedingen.de>        *
 *	russian and arabic support by Leonid Protasov <Lprot@mail.ru>         *
 *                                                                            *
 ******************************************************************************/

#define TUXTXT_CFG_STANDALONE 0  // 1:plugin only 0:use library
#define TUXTXT_DEBUG 0

#include <config.h>

#ifndef DREAMBOX
#include <tuxbox.h>
#endif
#if HAVE_DVB_API_VERSION >= 3
#include <linux/input.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>

#include <linux/fb.h>


#include <sys/ioctl.h>
#include <sys/mman.h>

#include <dbox/fp.h>
#include <plugin.h>
#include <dbox/lcd-ks0713.h>


#include "tuxtxt_def.h"


#if TUXTXT_CFG_STANDALONE
#include "tuxtxt_common.h"
#else
/* variables and functions from libtuxtxt */
extern tuxtxt_cache_struct tuxtxt_cache;
extern tstPageAttr tuxtxt_atrtable[];
extern int tuxtxt_init();
extern void tuxtxt_close();
extern int  tuxtxt_start(int tpid);  // Start caching
extern int  tuxtxt_stop(); // Stop caching
extern void tuxtxt_next_dec(int *i); /* skip to next decimal */
extern void tuxtxt_prev_dec(int *i); /* counting down */
extern int tuxtxt_is_dec(int i);
extern int tuxtxt_next_hex(int i);
extern void tuxtxt_decode_btt();
extern void tuxtxt_decode_adip(); /* additional information table */
extern void tuxtxt_compress_page(int p, int sp, unsigned char* buffer);
extern void tuxtxt_decompress_page(int p, int sp, unsigned char* buffer);
extern void tuxtxt_hex2str(char *s, unsigned int n);
extern tstPageinfo* tuxtxt_DecodePage(int showl25, unsigned char* page_char, tstPageAttr *page_atrb, int hintmode, int showflof);
extern void tuxtxt_FillRect(unsigned char *lfb, int xres, int x, int y, int w, int h, int color);
extern int tuxtxt_RenderChar(unsigned char *lfb, int xres,int Char, int *pPosX, int PosY, tstPageAttr *Attribute, int zoom, int curfontwidth, int curfontwidth2, int fontheight, int transpmode, unsigned char* axdrcs, int ascender);
extern void tuxtxt_RenderDRCS(int xres,unsigned char *s,unsigned char *d,unsigned char *ax, unsigned char fgcolor, unsigned char bgcolor);
#if TUXTXT_DEBUG
extern int tuxtxt_get_zipsize(int p, int sp);
#endif
extern void tuxtxt_RenderPage(tstRenderInfo* renderinfo);
extern void tuxtxt_SetPosX(tstRenderInfo* renderinfo, int column);
extern void tuxtxt_RenderCharFB(tstRenderInfo* renderinfo, int Char, tstPageAttr *Attribute);
extern void tuxtxt_ClearBB(tstRenderInfo* renderinfo,int color);
extern void tuxtxt_ClearFB(tstRenderInfo* renderinfo,int color);
extern void tuxtxt_setcolors(tstRenderInfo* renderinfo,unsigned short *pcolormap, int offset, int number);
extern int tuxtxt_SetRenderingDefaults(tstRenderInfo* renderinfo);
extern int tuxtxt_InitRendering(tstRenderInfo* renderinfo,int setTVFormat);
extern void tuxtxt_EndRendering(tstRenderInfo* renderinfo);
extern void tuxtxt_CopyBB2FB(tstRenderInfo* renderinfo);
extern void tuxtxt_SwitchScreenMode(tstRenderInfo* renderinfo,int newscreenmode);
#endif


#define TUXTXTCONF CONFIGDIR "/tuxtxt/tuxtxt2.conf"


#define fontwidth_small_lcd 8




/* rc codes */
#if HAVE_DVB_API_VERSION < 3
#define KEY_0       0x5C00
#define KEY_1       0x5C01
#define KEY_2       0x5C02
#define KEY_3       0x5C03
#define KEY_4       0x5C04
#define KEY_5       0x5C05
#define KEY_6       0x5C06
#define KEY_7       0x5C07
#define KEY_8       0x5C08
#define KEY_9       0x5C09
#define KEY_POWER   0x5C0C
#define KEY_UP      0x5C0E
#define KEY_DOWN    0x5C0F
#define KEY_VOLUMEUP    0x5C16
#define KEY_VOLUMEDOWN  0x5C17
#define KEY_HOME    0x5C20
#define KEY_SETUP   0x5C27
#define KEY_MUTE    0x5C28
#define KEY_RED     0x5C2D
#define KEY_RIGHT   0x5C2E
#define KEY_LEFT    0x5C2F
#define KEY_OK      0x5C30
#define KEY_BLUE    0x5C3B
#define KEY_YELLOW  0x5C52
#define KEY_GREEN   0x5C55
#define KEY_HELP    0x5C82
#endif
#define RC_0        0x00
#define RC_1        0x01
#define RC_2        0x02
#define RC_3        0x03
#define RC_4        0x04
#define RC_5        0x05
#define RC_6        0x06
#define RC_7        0x07
#define RC_8        0x08
#define RC_9        0x09
#define RC_RIGHT    0x0A
#define RC_LEFT     0x0B
#define RC_UP       0x0C
#define RC_DOWN     0x0D
#define RC_OK       0x0E
#define RC_MUTE     0x0F
#define RC_STANDBY  0x10
#define RC_GREEN    0x11
#define RC_YELLOW   0x12
#define RC_RED      0x13
#define RC_BLUE     0x14
#define RC_PLUS     0x15
#define RC_MINUS    0x16
#define RC_HELP     0x17
#define RC_DBOX     0x18
#define RC_HOME     0x1F






/* messages */
#define ShowInfoBar     0
//#define PageNotFound    1
#define ShowServiceName 2
#define NoServicesFound 3






/* national subsets */
const char countrystring[] =
"         Default          "   /*  0 no subset specified */
"       Czech/Slovak       "   /*  1 czech, slovak */
"         English          "   /*  2 english */
"         Estonian         "   /*  3 estonian */
"          French          "   /*  4 french */
"         Deutsch          "   /*  5 german */
"         Italian          "   /*  6 italian */
"    Latvian/Lithuanian    "   /*  7 latvian, lithuanian */
"          Polish          "   /*  8 polish */
"    Portuguese/Spanish    "   /*  9 portuguese, spanish */
"         Romanian         "   /* 10 romanian */
"Serbian/Croatian/Slovenian"   /* 11 serbian, croatian, slovenian */
"Swedish/Finnish/Hungarian "   /* 12 swedish, finnish, hungarian */
"          T~rk}e          "   /* 13 turkish */
"      Srpski/Hrvatski     "   /* 14 serbian, croatian */
"     Russkij/Bylgarski    "   /* 15 russian, bulgarian */
"        Ukra&nsxka        "   /* 16 ukrainian */
"         Ekkgmij\\         "   /* 17 greek */
"          zixar           "   /* 18 hebrew */
"           pHQY           "   /* 19 arabic */
;
#define COUNTRYSTRING_WIDTH 26
#define MAX_NATIONAL_SUBSET (sizeof(countrystring) / COUNTRYSTRING_WIDTH - 1)




/* some data */
char versioninfo[16];
int hotlist[10];
int maxhotlist;

int rc, lcd;
int lastpage;
int savedscreenmode;
char dumpl25;
int catch_row, catch_col, catched_page;
int swapupdown, menulanguage;
int pids_found, current_service, getpidsdone;
int SDT_ready;
int pc_old_row, pc_old_col;     /* for page catching */
int temp_page;	/* for page input */
char saveconfig, hotlistchanged;
tstRenderInfo renderinfo;
struct timeval tv_delay;
FILE *conf;


unsigned short RCCode;

struct _pid_table
{
	int  vtxt_pid;
	int  service_id;
	int  service_name_len;
	char service_name[24];
	int  national_subset;
}pid_table[128];

unsigned char restoreaudio = 0;
/* 0 Nokia, 1 Philips, 2 Sagem */
/* typ_vcr/dvb: 	v1 a1 v2 a2 v3 a3 (vcr_only: fblk) */
const int avstable_ioctl[7] =
{
	AVSIOSVSW1, AVSIOSASW1, AVSIOSVSW2, AVSIOSASW2, AVSIOSVSW3, AVSIOSASW3, AVSIOSFBLK
};
const int avstable_ioctl_get[7] =
{
	AVSIOGVSW1, AVSIOGASW1, AVSIOGVSW2, AVSIOGASW2, AVSIOGVSW3, AVSIOGASW3, AVSIOGFBLK
};
const unsigned char avstable_scart[3][7] =
{
	{ 3, 2, 1, 0, 1, 1, 2 },
	{ 3, 3, 2, 2, 3, 2, 2 },
	{ 2, 1, 0, 0, 0, 0, 0 },
};
unsigned char avstable_dvb[3][7] =
{
	{ 5, 1, 1, 0, 1, 1, 0 },
	{ 1, 1, 1, 1, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
};

/* language dependent texts */
#define MAXMENULANGUAGE 10 /* 0 deutsch, 1 englisch, 2 franzצsisch, 3 niederlהndisch, 4 griechisch, 5 italienisch, 6 polnisch, 7 schwedisch, 8 suomi, 9 portuguesa, 10 russian */
const int menusubset[] =   { NAT_DE   , NAT_UK    , NAT_FR       , NAT_UK          , NAT_GR      , NAT_IT       , NAT_PL    , NAT_SW      , NAT_SW ,   NAT_SP,      NAT_RB};


#define Menu_StartX (renderinfo.StartX + renderinfo.fontwidth*9/2)
#define Menu_StartY (renderinfo.StartY + renderinfo.fontheight)
#define Menu_Height 24
#define Menu_Width 31

const char MenuLine[] =
{
	3,8,11,12,15,17,19,20,21
};

enum
{
	M_HOT=0,
	M_PID,
	M_SC1,
	M_SC2,
	M_COL,
	M_TRA,
	M_AUN,
	M_NAT,
	M_LNG,
	M_Number
};

#define M_Start M_HOT
#define M_MaxDirect M_AUN

const char hotlistpagecolumn[] =	/* last(!) column of page to show in each language */
{
	22, 26, 28, 27, 28, 27, 28, 21, 20, 26, 26
};
const char hotlisttextcolumn[] =
{
	24, 14, 14, 15, 14, 15, 14, 23, 22, 14, 14
};
const char hotlisttext[][2*5] =
{
	{ "dazu entf." },
	{ " add rem. " },
	{ "ajoutenlev" },
	{ "toev.verw." },
	{ "pq|shava_q" },
	{ "agg. elim." },
	{ "dodajkasuj" },
	{ "ny   bort " },
	{ "lis{{pois " },
	{ " adi rem. " },
	{ "Dob. Udal." },	
};

const char configonoff[][2*3] =
{
	{ "ausein" },
	{ "offon " },
	{ "desact" },
	{ "uitaan" },
	{ "emeape" },
	{ "offon " },
	{ "wy}w} " },
	{ "p} av " },
	{ "EI ON " },
	{ "offon " },
	{ "w&kwkl" },
};
const char menuatr[Menu_Height*Menu_Width] =
{
	"0000000000000000000000000000002"
	"0111111111111111111111111111102"
	"0000000000000000000000000000002"
	"3144444444444444444444444444432"
	"3556655555555555555555555555532"
	"3555555555555555555555555555532"
	"3333333333333333333333333333332"
	"3144444444444444444444444444432"
	"3555555555555555555555555555532"
	"3333333333333333333333333333332"
	"3444444444444444444444444444432"
	"3155555555555555555555555555532"
	"3155555555555555555555555555532"
	"3333333333333333333333333333332"
	"3144444444444444444444444444432"
	"3555555555555555555555555555532"
	"3144444444444444444444444444432"
	"3555555555555555555555555555532"
	"3144444444444444444444444444432"
	"3555555555555555555555555555532"
	"3555555555555555555555555555532"
	"3555555555555555555555555555532"
	"3334444444444444444444444443332"
	"2222222222222222222222222222222"
};
const char configmenu[][Menu_Height*Menu_Width] =
{
	{
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד     Konfigurationsmen}     הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Favoriten: Seite 111 dazu הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2     Teletext-Auswahl      הי"
		"דם          suchen          מהי"
		"ד                            הי"
		"ד      Bildschirmformat      הי"
		"ד3  Standard-Modus 16:9      הי"
		"ד4  TextBild-Modus 16:9      הי"
		"ד                            הי"
		"ד5        Helligkeit         הי"
		"דם                          מהי"
		"ד6       Transparenz         הי"
		"דם                          מהי"
		"ד7  nationaler Zeichensatz   הי"
		"דautomatische Erkennung      הי"
		"דם                          מהי"
		"דם Sprache/Language deutsch מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד     Configuration menu     הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Favorites:  add page 111  הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2     Teletext selection    הי"
		"דם          search          מהי"
		"ד                            הי"
		"ד        Screen format       הי"
		"ד3 Standard mode 16:9        הי"
		"ד4 Text/TV mode  16:9        הי"
		"ד                            הי"
		"ד5        Brightness         הי"
		"דם                          מהי"
		"ד6       Transparency        הי"
		"דם                          מהי"
		"ד7   national characterset   הי"
		"ד automatic recognition      הי"
		"דם                          מהי"
		"דם Sprache/language english מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד    Menu de configuration   הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Favorites: ajout. page 111הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2  Selection de teletext    הי"
		"דם        recherche         מהי"
		"ד                            הי"
		"ד      Format de l'#cran     הי"
		"ד3 Mode standard 16:9        הי"
		"ד4 Texte/TV      16:9        הי"
		"ד                            הי"
		"ד5          Clarte           הי"
		"דם                          מהי"
		"ד6       Transparence        הי"
		"דם                          מהי"
		"ד7     police nationale      הי"
		"דreconn. automatique         הי"
		"דם                          מהי"
		"דם Sprache/language francaisמהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד      Configuratiemenu      הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Favorieten: toev. pag 111 הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2     Teletekst-selectie    הי"
		"דם          zoeken          מהי"
		"ד                            הי"
		"ד     Beeldschermformaat     הי"
		"ד3   Standaardmode 16:9      הי"
		"ד4   Tekst/TV mode 16:9      הי"
		"ד                            הי"
		"ד5        Helderheid         הי"
		"דם                          מהי"
		"ד6       Transparantie       הי"
		"דם                          מהי"
		"ד7    nationale tekenset     הי"
		"דautomatische herkenning     הי"
		"דם                          מהי"
		"דם Sprache/Language nederl. מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד      Lemo} quhl_seym       הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Vaboq_:    pqo_h. sek. 111הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2     Epikoc^ Teket]nt      הי"
		"דם        Amaf^tgsg         מהי"
		"ד                            הי"
		"ד       Loqv^ oh|mgr         הי"
		"ד3 Tq|por pq|tupor   16:9    הי"
		"ד4 Tq|por eij. jeil. 16:9    הי"
		"ד                            הי"
		"ד5       Kalpq|tgta          הי"
		"דם                          מהי"
		"ד6       Diav\\meia           הי"
		"דם                          מהי"
		"ד7    Ehmij^ tuposeiq\\       הי"
		"דaut|latg amacm~qisg         הי"
		"דם                          מהי"
		"דם Ck~ssa/Language ekkgmij\\ מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד   Menu di configurazione   הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1  Preferiti:  agg. pag.111 הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2   Selezione televideo     הי"
		"דם         ricerca          מהי"
		"ד                            הי"
		"ד      Formato schermo       הי"
		"ד3  Modo standard 16:9       הי"
		"ד4  Text/Mod.TV 16:9         הי"
		"ד                            הי"
		"ד5        Luminosit{         הי"
		"דם                          מהי"
		"ד6        Trasparenza        הי"
		"דם                          מהי"
		"ד7   nazionalita'caratteri   הי"
		"ד riconoscimento automatico  הי"
		"דם                          מהי"
		"דם Lingua/Language Italiana מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד        Konfiguracja        הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Ulubione : kasuj  str. 111הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2     Wyb_r telegazety      הי"
		"דם          szukaj          מהי"
		"ד                            הי"
		"ד       Format obrazu        הי"
		"ד3 Tryb standard 16:9        הי"
		"ד4 Telegazeta/TV 16:9        הי"
		"ד                            הי"
		"ד5          Jasno|^          הי"
		"דם                          מהי"
		"ד6      Prze~roczysto|^      הי"
		"דם                          מהי"
		"ד7 Znaki charakterystyczne   הי"
		"ד automatyczne rozpozn.      הי"
		"דם                          מהי"
		"דם  J`zyk/Language   polski מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד     Konfigurationsmeny     הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Favoriter: sida 111 ny    הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2      TextTV v{ljaren      הי"
		"דם            s|k           מהי"
		"ד                            הי"
		"ד        TV- format          הי"
		"ד3 Standard l{ge 16:9        הי"
		"ד4 Text/Bild l{ge  16:9      הי"
		"ד                            הי"
		"ד5        Ljusstyrka         הי"
		"דם                          מהי"
		"ד6     Genomskinlighet       הי"
		"דם                          מהי"
		"ד7nationell teckenupps{ttningהי"
		"ד automatisk igenk{nning     הי"
		"דם                          מהי"
		"דם Sprache/language svenska מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד        Asetusvalikko       הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Suosikit: sivu 111 lis{{  הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2   Tekstikanavan valinta   הי"
		"דם          search          מהי"
		"ד                            הי"
		"ד         N{ytt|tila         הי"
		"ד3 Vakiotila     16:9        הי"
		"ד4 Teksti/TV     16:9        הי"
		"ד                            הי"
		"ד5         Kirkkaus          הי"
		"דם                          מהי"
		"ד6       L{pin{kyvyys        הי"
		"דם                          מהי"
		"ד7   kansallinen merkist|    הי"
		"ד automaattinen tunnistus    הי"
		"דם                          מהי"
		"דם Kieli            suomi   מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד    Menu de Configuracao    הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Favoritos:  adi pag. 111  הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2     Seleccao Teletext     הי"
		"דם         Procurar         מהי"
		"ד                            הי"
		"ד       formato ecran        הי"
		"ד3 Standard mode 16:9        הי"
		"ד4 Text/TV mode  16:9        הי"
		"ד                            הי"
		"ד5          Brilho           הי"
		"דם                          מהי"
		"ד6      Transparencia        הי"
		"דם                          מהי"
		"ד7  Caracteres nacionaist    הי"
		"דreconhecimento utomatico    הי"
		"דם                          מהי"
		"דם Lingua      Portuguesa   מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
/*     0000000000111111111122222222223 */
/*     0123456789012345678901234567890 */
	{
		"אבבבבבבבבבבבבבבבבבבבבבבבבבבבבגט"
		"ד        Konfiguraciq        הי"
		"וזזזזזזזזזזזזזזזזזזזזזזזזזזזזחי"
		"ד1 Faworit&:   dob str. 111  הי"
		"דםמסע                        הי"
		"ד+-?                         הי"
		"ד                            הי"
		"ד2     W&bor teleteksta      הי"
		"דם           Poisk          מהי"
		"ד                            הי"
		"ד       Format kartinki      הי"
		"ד3 Stand. revim  16:9        הי"
		"ד4 Tekst/TW rev. 16:9        הי"
		"ד                            הי"
		"ד5          Qrkostx          הי"
		"דם                          מהי"
		"ד6       Prozra~nostx        הי"
		"דם                          מהי"
		"ד7  Ispolxzuem&j alfawit     הי"
		"ד      awtoopredelenie       הי"
		"דם                          מהי"
		"דם  Qz&k:         Russkij   מהי"
		"וז   www.tuxtxt.net  x.xx   זחי"
		"כלללללללללללללללללללללללללללללך"
	},
};

const char catchmenutext[][80] =
{
	{ "        םןנמ w{hlen   סע anzeigen       "
	  "0000000011110000000000110000000000000000" },
	{ "        םןנמ select   סע show           "
	  "0000000011110000000000110000000000000000" },
	{ "  םןנמ selectionner   סע montrer        "
	  "0011110000000000000000110000000000000000" },
	{ "        םןנמ kiezen   סע tonen          "
	  "0000000011110000000000110000000000000000" },
	{ "        םןנמ epikoc^  סע pqobok^        "
	  "0000000011110000000000110000000000000000" },
	{ "        םןנמseleziona סע mostra         "
	  "0000000011110000000000110000000000000000" },
	{ "        םןנמ wybiez   סע wyswietl       "
	  "0000000011110000000000110000000000000000" },
	{ "        םןנמ v{lj     סע visa           "
     "0000000011110000000000110000000000000000" },
	{ "        םןנמ valitse  סע n{yt{          "
	  "0000000011110000000000110000000000000000" },
	{ "        םןנמ seleccao סע mostrar        "
	  "0000000011110000000000110000000000000000" },
	{ "        םןנמ w&bratx  סע pokazatx       "
	  "0000000011110000000000110000000000000000" },
};

const char message_3[][38] =
{
	{ "ד   Suche nach Teletext-Anbietern   הי" },
	{ "ד  Searching for teletext services  הי" },
	{ "ד  Recherche des services teletext  הי" },
	{ "ד Zoeken naar teletekst aanbieders  הי" },
	{ "ד     amaf^tgsg voq]ym Teket]nt     הי" },
	{ "ד     attesa opzioni televideo      הי" },
	{ "ד  Poszukiwanie sygna}u telegazety  הי" },
	{ "ד    s|ker efter TextTV tj{nster    הי" },
	{ "ד   etsit{{n Teksti-TV -palvelua    הי" },
	{ "ד  Procurar servicos de teletexto   הי" },
	{ "ד   W&polnqetsq poisk teleteksta    הי" },
};
const char message_3_blank[] = "ד                                   הי";
const char message_7[][38] =
{
	{ "ד kein Teletext auf dem Transponder הי" },
	{ "ד   no teletext on the transponder  הי" },
	{ "ד pas de teletext sur le transponderהי" },
	{ "ד geen teletekst op de transponder  הי" },
	{ "ד jal]la Teket]nt ston amaletadot^  הי" },
	{ "ד nessun televideo sul trasponder   הי" },
	{ "ד   brak sygna}u na transponderze   הי" },
	{ "ד ingen TextTV p} denna transponder הי" },
	{ "ד    Ei Teksti-TV:t{ l{hettimell{   הי" },
	{ "ד  nao ha teletexto no transponder  הי" },
	{ "ד  Na transpondere net teleteksta   הי" },	
};
const char message_8[][38] =
{
/*    00000000001111111111222222222233333333334 */
/*    01234567890123456789012345678901234567890 */
	{ "ד  warte auf Empfang von Seite 100  הי" },
	{ "ד waiting for reception of page 100 הי" },
	{ "ד attentre la rיception de page 100 הי" },
	{ "דwachten op ontvangst van pagina 100הי" },
	{ "ד     amal]my k^xg sek_dar 100      הי" },
	{ "ד   attesa ricezione pagina 100     הי" },
	{ "ד     oczekiwanie na stron` 100     הי" },
	{ "ד  v{ntar p} mottagning av sida 100 הי" },
	{ "ד        Odotetaan sivua 100        הי" },
	{ "ד   esperando recepcao na pag 100   הי" },
	{ "ד   Ovidanie priema stranic& 100    הי" },	
};
const char message8pagecolumn[] = /* last(!) column of page to show in each language */
{
	33, 34, 34, 35, 29, 30, 30, 34, 34, 32, 34
};



/* buffers */
unsigned char  lcd_backbuffer[120*64 / 8];

//unsigned short page_atrb[40 * 25]; /*  ?????:h:cc:bbbb:ffff -> ?=reserved, h=double height, c=charset (0:G0 / 1:G1c / 2:G1s), b=background, f=foreground */



/* lcd layout */
const char lcd_layout[] =
{
#define ____ 0x0
#define ___X 0x1
#define __X_ 0x2
#define __XX 0x3
#define _X__ 0x4
#define _X_X 0x5
#define _XX_ 0x6
#define _XXX 0x7
#define X___ 0x8
#define X__X 0x9
#define X_X_ 0xA
#define X_XX 0xB
#define XX__ 0xC
#define XX_X 0xD
#define XXX_ 0xE
#define XXXX 0xF

#define i <<4|

	____ i _XXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i ____,
	___X i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i X___,
	__XX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XX__ i ____,XXX_ i _X__,_XXX i __X_,__XX i X___,___X i XX__,X___ i XXX_,____ i _XXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XX__,
	_XXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,X_XX i XXXX,_X_X i X_XX,X_X_ i XX_X,XX_X i _XXX,XXX_ i X_XX,_XXX i _X_X,XXXX i X_XX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXX_,
	_XXX i XXXX,X___ i ____,____ i ____,____ i __XX,X_XX i XXXX,_X_X i X_XX,X_X_ i XX_X,XX_X i _XXX,XXX_ i X_XX,_XXX i _X_X,XXXX i X_XX,X___ i ____,____ i ____,____ i ___X,XXXX i XXX_,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,X_XX i XXXX,_X_X i X_XX,X_X_ i XX_X,XX_X i _XXX,XXX_ i X_XX,_XXX i _X_X,XXXX i X_XX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XX__ i XX__,XX_X i X_XX,X_X_ i XXXX,XX_X i X__X,X__X i X_XX,XXXX i _XX_,_XX_ i _XXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XX__,____ i ____,____ i ____,____ i __XX,XXX_ i XX_X,XX_X i X_XX,X_XX i _XXX,__XX i XX_X,X_XX i XX_X,XX__ i XXXX,_XX_ i XXXX,X___ i ____,____ i ____,____ i ____,__XX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i XX_X,XX_X i X_XX,X_X_ i XXXX,XX_X i XX_X,X_XX i X_XX,XXXX i _XXX,_XX_ i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i XX_X,XX_X i X_XX,X_X_ i XX_X,XX_X i XX_X,X_XX i X_XX,_XXX i _XXX,_XX_ i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXX_ i ____,____ i ____,____ i ____,____ i __XX,XXX_ i XX_X,XX_X i XXXX,X_X_ i XX_X,XX_X i XX_X,X_XX i X_XX,_XXX i _XXX,_XX_ i XXXX,X___ i ____,____ i ____,____ i ____,____ i _XXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i XX_X,XX_X i XXXX,X_X_ i XX_X,XX_X i XX_X,X_XX i X_XX,_XXX i _XXX,_XX_ i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i __XX,XXX_ i ____,_XXX i __X_,__XX i XXX_,_XXX i XX__,X___ i XXXX,X__X i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i XX__,_XXX i XXX_,__XX i ___X,XXXX i X___,XX__ i _XXX,XXX_ i __XX,____ i ___X,X___ i XXXX,XX__ i _XX_,__XX i XXXX,___X i XXXX,X___ i XX_X,XX__ i _XXX,XXX_ i __XX,XXXX i ___X,
	X__X i __X_,X___ i ___X,_X__ i X_X_,____ i _X_X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ____,_X_X i __X_,__X_ i X___,___X i _X__,____ i X__X,
	X__X i __X_,X___ i ___X,_X__ i X_X_,____ i _X_X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ____,_X_X i __X_,__X_ i X___,___X i _X__,____ i X__X,
	X__X i __X_,X___ i ___X,_X__ i X_X_,____ i _X_X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ____,_X_X i __X_,__X_ i X___,___X i _X__,____ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X__X,X__X i X__X,__X_ i X__X,___X i _X__,X___ i __X_,_X_X i __XX,XX__ i X__X,_X__ i XXXX,__X_ i _X__,_X_X i __X_,__X_ i X__X,___X i _X__,XXXX i ___X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ___X,X__X i __X_,__X_ i X__X,___X i _X__,X___ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X___,___X i _X__,X___ i __X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i ____,_X_X i __X_,__X_ i X__X,___X i _X__,X___ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X__X,___X i _X__,XXXX i __X_,_X__ i XXX_,__X_ i X__X,_X__ i XXXX,__X_ i _X__,_X_X i __X_,__X_ i X__X,___X i _X__,X___ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X__X,___X i _X__,____ i X_X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i _X__,_X_X i ____,__X_ i X__X,___X i _X__,____ i X__X,
	X__X i __X_,X__X i ___X,_X__ i X___,X__X i ___X,__X_ i X__X,___X i _X__,____ i X_X_,_X_X i ____,__X_ i X__X,_X__ i ____,X_X_ i _X__,_X_X i ____,__X_ i X__X,___X i _X__,____ i X__X,
	X___ i XX__,XXX_ i XXXX,__XX i ____,_XX_ i ____,XX__ i _XX_,XXX_ i __XX,XXXX i ___X,X___ i XXXX,XX__ i _XX_,__XX i XXXX,___X i X_XX,X___ i XXXX,XX__ i _XX_,XXX_ i __XX,XXXX i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,XXXX i XXXX,X___ i XXXX,XX__ i _XXX,XXXX i XX__,_XXX i XXX_,__XX i XXXX,___X i XXXX,X___ i ____,__XX i XXXX,___X i X___,XXXX i XX__,_XXX i XXX_,__XX i XXXX,____ i ___X,
	X___ i ___X,____ i ____,_X_X i ____,__X_ i X___,____ i __X_,X___ i ___X,_X__ i ____,X_X_ i ____,_X__ i ____,_X__ i ____,X_X_ i _X_X,____ i __X_,X___ i ___X,_X__ i ____,X___ i ___X,
	X___ i ___X,____ i ____,_X_X i ____,__X_ i X___,____ i __X_,X___ i ___X,_X__ i ____,X_X_ i ____,_X__ i ____,_X__ i ____,X_X_ i _X_X,____ i __X_,X___ i ___X,_X__ i ____,X___ i ___X,
	X___ i ___X,____ i ____,_X_X i ____,__X_ i X___,____ i __X_,X___ i ___X,_X__ i ____,X_X_ i ____,_X__ i ____,_X__ i XX__,X_X_ i _X_X,____ i __X_,X___ i ___X,_X__ i ____,X___ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i __X_,__X_ i X__X,___X i __X_,X__X i XXX_,_X__ i X___,X__X i X__X,X___ i ____,_X__ i ____,X_X_ i _X__,XX__ i XX__,_XX_ i _XX_,_X__ i XXXX,____ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i __X_,__X_ i X__X,___X i __X_,X___ i ___X,_X__ i X___,X___ i X__X,____ i ____,_X__ i __XX,__X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i ____,X___ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i __X_,__X_ i X__X,___X i __X_,X___ i ___X,_X__ i X___,X___ i X__X,____ i ____,_X__ i ____,X_X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i ____,X___ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i __X_,__X_ i X__X,___X i __X_,X__X i XXX_,_X__ i X___,X___ i X__X,____ i ____,_X__ i XX__,X_X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i XXXX,____ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i ____,__X_ i X__X,___X i __X_,X___ i ___X,_X__ i X___,X___ i X__X,____ i ____,_X__ i ____,X_X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i ____,X___ i ___X,
	X___ i ___X,__X_ i __X_,_X_X i ____,__X_ i X__X,___X i __X_,X___ i ___X,_X__ i X___,X___ i X__X,____ i ____,_X__ i ____,X_X_ i _X__,_X__ i X___,__X_ i _X__,_X__ i ____,X___ i ___X,
	X___ i ____,XX_X i XX_X,X___ i XXXX,XX__ i _XX_,XXX_ i XX__,_XXX i XXX_,__XX i _XXX,____ i _XX_,____ i ____,__XX i XXXX,___X i X___,__XX i ____,___X i X___,__XX i XXXX,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	X___ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ___X,
	_X__ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i __X_,
	_X__ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i __X_,
	__X_ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i _X__,
	___X i X___,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,____ i ____,___X i X___,
	____ i _XXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXXX i XXXX,XXX_ i ____,

#undef i
};

/* lcd digits */
const char lcd_digits[] =
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

	/* 10: - */
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,1,1,1,1,1,1,0,0,
	0,1,0,0,0,0,0,0,1,0,
	0,1,0,0,0,0,0,0,1,0,
	0,1,0,0,0,0,0,0,1,0,
	0,0,1,1,1,1,1,1,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,

	/* 11: / */
	0,0,0,0,1,1,1,1,0,0,
	0,0,0,1,0,0,0,0,1,0,
	0,0,0,1,0,0,0,0,1,0,
	0,0,0,1,0,0,0,0,1,0,
	0,0,0,1,0,0,0,0,1,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,0,1,0,0,0,0,1,0,0,
	0,1,0,0,0,0,1,0,0,0,
	0,1,0,0,0,0,1,0,0,0,
	0,1,0,0,0,0,1,0,0,0,
	0,1,0,0,0,0,1,0,0,0,
	0,0,1,1,1,1,0,0,0,0,

	/* 12: ? */
	0,1,1,1,1,1,1,1,1,0,
	1,1,0,0,0,0,0,0,1,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	0,1,1,1,1,1,0,0,0,1,
	1,1,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,0,1,
	1,0,0,0,0,0,0,0,1,1,
	1,1,1,1,1,1,1,1,1,0,
	1,0,0,0,0,1,0,0,0,0,
	1,0,0,0,0,1,0,0,0,0,
	1,0,0,0,0,1,0,0,0,0,
	1,1,0,0,1,1,0,0,0,0,
	0,1,1,1,1,0,0,0,0,0,

	/* 13: " " */
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
};

/* functions */
void ConfigMenu(int Init);
void CleanUp();
void PageInput(int Number);
void ColorKey(int);
void PageCatching();
void CatchNextPage(int, int);
void GetNextPageOne(int up);
void GetNextSubPage(int offset);
void SwitchZoomMode();
void SwitchTranspMode();
void SwitchHintMode();
void RenderCatchedPage();
void RenderCharLCD(int Digit, int XPos, int YPos);
void RenderMessage(int Message);
void UpdateLCD();
int  Init();
int  GetNationalSubset(char *country_code);
int  GetTeletextPIDs();
int  GetRCCode();
/* Local Variables: */
/* indent-tabs-mode:t */
/* tab-width:3 */
/* c-basic-offset:3 */
/* comment-column:0 */
/* fill-column:120 */
/* End: */
