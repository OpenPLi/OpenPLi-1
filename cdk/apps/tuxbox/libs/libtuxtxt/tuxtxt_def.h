/******************************************************************************
 * definitions for plugin and lib                                             *
 ******************************************************************************/
#ifndef TUXTXT_DEF_H

#define TUXTXT_DEF_H
#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
 #define TUXTXT_COMPRESS 1 // compress page data: 0 no compression, 1 with zlib, 2 with own algorithm
#else
 #define TUXTXT_COMPRESS 2
#endif

#include <config.h>
#include <sys/time.h>
#include <pthread.h>

#if HAVE_DVB_API_VERSION < 3
 #define dmx_pes_filter_params dmxPesFilterParams
 #define pes_type pesType
 #define dmx_sct_filter_params dmxSctFilterParams
 #include <ost/dmx.h>
 #define DMX "/dev/dvb/card0/demux0"
#else
 #include <linux/dvb/dmx.h>
 #define DMX "/dev/dvb/adapter0/demux0"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H
#include <linux/fb.h>

#include <dbox/avs_core.h>
#include <dbox/saa7126_core.h>

#define FLOFSIZE 4

#define PAGESIZE (40*25)

/* spacing attributes */
#define alpha_black         0x00
#define alpha_red           0x01
#define alpha_green         0x02
#define alpha_yellow        0x03
#define alpha_blue          0x04
#define alpha_magenta       0x05
#define alpha_cyan          0x06
#define alpha_white         0x07
#define flash               0x08
#define steady              0x09
#define end_box             0x0A
#define start_box           0x0B
#define normal_size         0x0C
#define double_height       0x0D
#define double_width        0x0E
#define double_size         0x0F
#define mosaic_black        0x10
#define mosaic_red          0x11
#define mosaic_green        0x12
#define mosaic_yellow       0x13
#define mosaic_blue         0x14
#define mosaic_magenta      0x15
#define mosaic_cyan         0x16
#define mosaic_white        0x17
#define conceal             0x18
#define contiguous_mosaic   0x19
#define separated_mosaic    0x1A
#define esc                 0x1B
#define black_background    0x1C
#define new_background      0x1D
#define hold_mosaic         0x1E
#define release_mosaic      0x1F

const int fncmodes[] = {AVS_FNCOUT_EXT43, AVS_FNCOUT_EXT169};
const int saamodes[] = {SAA_WSS_43F, SAA_WSS_169F};

typedef enum /* object type */
{
	OBJ_PASSIVE,
	OBJ_ACTIVE,
	OBJ_ADAPTIVE
} tObjType;


enum
{
	NAT_DEFAULT = 0,
	NAT_CZ = 1,
	NAT_UK = 2,
	NAT_ET = 3,
	NAT_FR = 4,
	NAT_DE = 5,
	NAT_IT = 6,
	NAT_LV = 7,
	NAT_PL = 8,
	NAT_SP = 9,
	NAT_RO = 10,
	NAT_SR = 11,
	NAT_SW = 12,
	NAT_TR = 13,
	NAT_MAX_FROM_HEADER = 13,
	NAT_SC = 14,
	NAT_RB = 15,
	NAT_UA = 16,		
	NAT_GR = 17,	
	NAT_HB = 18,
	NAT_AR = 19
};
const unsigned char countryconversiontable[] = { NAT_UK, NAT_DE, NAT_SW, NAT_IT, NAT_FR, NAT_SP, NAT_CZ, NAT_RO};
/* tables for color table remapping, first entry (no remapping) skipped, offsets for color index */
const unsigned char MapTblFG[] = {  0,  0,  8,  8, 16, 16, 16 };
const unsigned char MapTblBG[] = {  8, 16,  8, 16,  8, 16, 24 };
const unsigned short tuxtxt_defaultcolors[] =	/* 0x0bgr */
{
	0x000, 0x00f, 0x0f0, 0x0ff, 0xf00, 0xf0f, 0xff0, 0xfff,
	0x000, 0x007, 0x070, 0x077, 0x700, 0x707, 0x770, 0x777,
	0x50f, 0x07f, 0x7f0, 0xbff, 0xac0, 0x005, 0x256, 0x77c,
	0x333, 0x77f, 0x7f7, 0x7ff, 0xf77, 0xf7f, 0xff7, 0xddd,
	0x420, 0x210, 0x420, 0x000, 0x000
};
/* colortable */
enum
{
	tuxtxt_color_black = 0,
	tuxtxt_color_red, /* 1 */
	tuxtxt_color_green, /* 2 */
	tuxtxt_color_yellow, /* 3 */
	tuxtxt_color_blue,	/* 4 */
	tuxtxt_color_magenta,	/* 5 */
	tuxtxt_color_cyan,	/* 6 */
	tuxtxt_color_white, /* 7 */
	tuxtxt_color_menu1 = (4*8),
	tuxtxt_color_menu2,
	tuxtxt_color_menu3,
	tuxtxt_color_transp,
	tuxtxt_color_transp2,
	tuxtxt_color_SIZECOLTABLE
};

enum /* options for charset */
{
	C_G0P = 0, /* primary G0 */
	C_G0S, /* secondary G0 */
	C_G1C, /* G1 contiguous */
	C_G1S, /* G1 separate */
	C_G2,
	C_G3,
	C_OFFSET_DRCS = 32
	/* 32..47: 32+subpage# GDRCS (offset/20 in page_char) */
	/* 48..63: 48+subpage#  DRCS (offset/20 in page_char) */
};

enum /* page function */
{
	FUNC_LOP = 0, /* Basic Level 1 Teletext page (LOP) */
	FUNC_DATA, /* Data broadcasting page coded according to EN 300 708 [2] clause 4 */
	FUNC_GPOP, /* Global Object definition page (GPOP) - (see clause 10.5.1) */
	FUNC_POP, /* Normal Object definition page (POP) - (see clause 10.5.1) */
	FUNC_GDRCS, /* Global DRCS downloading page (GDRCS) - (see clause 10.5.2) */
	FUNC_DRCS, /* Normal DRCS downloading page (DRCS) - (see clause 10.5.2) */
	FUNC_MOT, /* Magazine Organization table (MOT) - (see clause 10.6) */
	FUNC_MIP, /* Magazine Inventory page (MIP) - (see clause 11.3) */
	FUNC_BTT, /* Basic TOP table (BTT) } */
	FUNC_AIT, /* Additional Information Table (AIT) } (see clause 11.2) */
	FUNC_MPT, /* Multi-page table (MPT) } */
	FUNC_MPTEX, /* Multi-page extension table (MPT-EX) } */
	FUNC_TRIGGER /* Page contain trigger messages defined according to [8] */
};
/* struct for page attributes */
typedef struct
{
	unsigned char fg      :6; /* foreground color */
	unsigned char bg      :6; /* background color */
	unsigned char charset :6; /* see enum above */
	unsigned char doubleh :1; /* double height */
	unsigned char doublew :1; /* double width */
	/* ignore at Black Background Color Substitution */
	/* black background set by New Background ($1d) instead of start-of-row default or Black Backgr. ($1c) */
	/* or black background set by level 2.5 extensions */
	unsigned char IgnoreAtBlackBgSubst:1;
	unsigned char concealed:1; /* concealed information */
	unsigned char inverted :1; /* colors inverted */
	unsigned char flashing :5; /* flash mode */
	unsigned char diacrit  :4; /* diacritical mark */
	unsigned char underline:1; /* Text underlined */
	unsigned char boxwin   :1; /* Text boxed/windowed */
	unsigned char setX26   :1; /* Char is set by packet X/26 (no national subset used) */
	unsigned char setG0G2  :7; /* G0+G2 set designation  */
} tstPageAttr;


/* struct for (G)POP/(G)DRCS links for level 2.5, allocated at reception of p27/4 or /5, initialized with 0 after allocation */
typedef struct
{
	short page; /* linked page number */
	unsigned short subpage; /* 1 bit for each needed (1) subpage */
	unsigned char l25:1; /* 1: page required at level 2.5 */
	unsigned char l35:1; /* 1: page required at level 3.5 */
	unsigned char drcs:1; /* 1: link to (G)DRCS, 0: (G)POP */
	unsigned char local:1; /* 1: global (G*), 0: local */
} tstp27;

/* struct for extension data for level 2.5, allocated at reception, initialized with 0 after allocation */
typedef struct
{
	unsigned char *p26[16]; /* array of pointers to max. 16 designation codes of packet 26 */
	tstp27 *p27; /* array of 4 structs for (G)POP/(G)DRCS links for level 2.5 */
	unsigned short bgr[16]; /* CLUT 2+3, 2*8 colors, 0x0bgr */
	unsigned char DefaultCharset:7; /* default G0/G2 charset + national option */
	unsigned char LSP:1; /* 1: left side panel to be displayed */
	unsigned char SecondCharset:7; /* second G0 charset */
	unsigned char RSP:1; /* 1: right side panel to be displayed */
	unsigned char DefScreenColor:5; /* default screen color (above and below lines 0..24) */
	unsigned char ColorTableRemapping:3; /* 1: index in table of CLUTs to use */
	unsigned char DefRowColor:5; /* default row color (left and right to lines 0..24) */
	unsigned char BlackBgSubst:1; /* 1: substitute black background (as result of start-of-line or 1c, not 00/10+1d) */
	unsigned char SPL25:1; /* 1: side panel required at level 2.5 */
	unsigned char p28Received:1; /* 1: extension data valid (p28/0 received) */
	unsigned char LSPColumns:4; /* number of columns in left side panel, 0->16, rsp=16-lsp */
} tstExtData;


/* struct for pageinfo, max. 16 Bytes, at beginning of each cached page buffer, initialized with 0 after allocation */
typedef struct
{
	unsigned char *p24; /* pointer to lines 25+26 (packets 24+25) (2*40 bytes) for FLOF or level 2.5 data */
	tstExtData *ext; /* pointer to array[16] of data for level 2.5 */
	unsigned char boxed         :1; /* p0: boxed (newsflash or subtitle) */
	unsigned char nationalvalid :1; /* p0: national option character subset is valid (no biterror detected) */
	unsigned char national      :3; /* p0: national option character subset */
	unsigned char function      :3; /* p28/0: page function */
} tstPageinfo;

/* one cached page: struct for pageinfo, 24 lines page data */
typedef struct
{
	tstPageinfo pageinfo;
	unsigned char p0[24]; /* packet 0: center of headline */
#if TUXTXT_COMPRESS == 1
	unsigned char * pData;/* packet 1-23 */
	unsigned short ziplen;
#elif TUXTXT_COMPRESS == 2
	unsigned char * pData;/* packet 1-23 */
	unsigned char bitmask[23*5];
#else
	unsigned char data[23*40];	/* packet 1-23 */
#endif
} tstCachedPage;

typedef struct
{
	short page; 
	short language;
} tstSubtitles;

#define SUBTITLE_CACHESIZE 50 
typedef struct 
{
	unsigned char valid;
	struct timeval tv_timestamp;
	unsigned char  page_char[40 * 25];
	tstPageAttr page_atrb[40 * 25];
} subtitle_cache;

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
#define FONTTYPE FTC_ImageTypeRec
#else
#define FONTTYPE FTC_Image_Desc
#endif
/* struct for all Information needed for Page Rendering */
#define aydrcs (renderinfo->axdrcs + 12+1)
typedef struct
{
	unsigned char *lfb;
	unsigned char page_char[25*40];
	tstPageAttr page_atrb[25*40];
	int auto_national;
	tstPageinfo *pageinfo;
	int zoommode;
	int hintmode;
	int PosX;
	int PosY;
	int StartX;
	int StartY;
	int sx,sy,ex,ey;
	int nofirst;
	int transpmode;
	int pig, avs, saa,fb;
	unsigned char axdrcs[12+1+10+1];
	struct fb_var_screeninfo var_screeninfo;
	struct fb_fix_screeninfo fix_screeninfo;
	int TTFWidthFactor16, TTFHeightFactor16, TTFShiftX, TTFShiftY; /* parameters for adapting to various TTF fonts */
	int fontheight, fontwidth, fontwidth_normal, fontwidth_small, fontwidth_topmenumain, fontwidth_topmenusmall, ascender;
	int displaywidth;
	int usettf;
	int boxed;
	int screenmode,prevscreenmode;
	int pagecatching;
	int inputcounter;
	int subtitledelay, delaystarted;
	int prev_100, prev_10, next_10, next_100;
	int showhex, showflof,show39, showl25;
	int fnc_old, saa_old, screen_mode1, screen_mode2,color_mode, trans_mode;
	signed char clearbbcolor;
	FT_Library      library;
	FTC_Manager     manager;
	FTC_SBitCache   cache;
	FTC_SBit        sbit;
	FT_Face			face;
	FONTTYPE typettf;
	subtitle_cache *subtitlecache[SUBTITLE_CACHESIZE];
	unsigned short rd0[tuxtxt_color_SIZECOLTABLE];
	unsigned short gn0[tuxtxt_color_SIZECOLTABLE];
	unsigned short bl0[tuxtxt_color_SIZECOLTABLE];
	unsigned short tr0[tuxtxt_color_SIZECOLTABLE];
	int previousbackcolor;
} tstRenderInfo;

/* main data structure */
typedef struct
{
	short flofpages[0x900][FLOFSIZE];
	unsigned char adip[0x900][13];
	unsigned char subpagetable[0x900];
	int dmx;
	int vtxtpid;
	int cached_pages, page, subpage, pageupdate,page_receiving, current_page[9], current_subpage[9];
	int receiving, thread_starting, zap_subpage_manual;
	char bttok;
	int adippg[10];
	int maxadippg;
	unsigned char basictop[0x900];

	unsigned char  timestring[8];
	/* cachetable for packets 29 (one for each magazine) */
	tstExtData *astP29[9];
	/* cachetable */
	tstCachedPage *astCachetable[0x900][0x80];
	pthread_t thread_id;
	void *thread_result;
	unsigned char FullRowColor[25];
	unsigned char FullScrColor;
	unsigned char tAPx, tAPy;	/* temporary offset to Active Position for objects */
	short pop, gpop, drcs, gdrcs;
	int national_subset, national_subset_secondary;
	unsigned short *colortable;
	tstSubtitles subtitlepages[8];
} tuxtxt_cache_struct;

typedef struct
{
	unsigned char* page_char; // Character array (25*40) of decoded page
	tstPageAttr* page_atrb;   // Attributes Array (25*40) of decoded page
	int col;                  // current column (0..39 )
	int row;                  // current row (0..23)
	tstPageinfo* pageinfo;    // pageinfo of decoded page
	unsigned short cstyles_n[1024];
	unsigned short cstyles_d[1024];
	unsigned short cstyles_g[32];
	unsigned short cstyles_b[32];
	unsigned short stylecount_n;
	unsigned short stylecount_d;
	unsigned short stylecount_g;
	unsigned short stylecount_b;	
} tstHTML;

enum /* indices in atrtable */
{
	ATR_WB, /* white on black */
	ATR_PassiveDefault, /* Default for passive objects: white on black, ignore at Black Background Color Substitution */
	ATR_L250, /* line25 */
	ATR_L251, /* line25 */
	ATR_L252, /* line25 */
	ATR_L253, /* line25 */
	ATR_TOPMENU0, /* topmenu */
	ATR_TOPMENU1, /* topmenu */
	ATR_TOPMENU2, /* topmenu */
	ATR_TOPMENU3, /* topmenu */
	ATR_MSG0, /* message */
	ATR_MSG1, /* message */
	ATR_MSG2, /* message */
	ATR_MSG3, /* message */
	ATR_MSGDRM0, /* message */
	ATR_MSGDRM1, /* message */
	ATR_MSGDRM2, /* message */
	ATR_MSGDRM3, /* message */
	ATR_MENUHIL0, /* hilit menu line */
	ATR_MENUHIL1, /* hilit menu line */
	ATR_MENUHIL2, /* hilit menu line */
	ATR_MENU0, /* menu line */
	ATR_MENU1, /* menu line */
	ATR_MENU2, /* menu line */
	ATR_MENU3, /* menu line */
	ATR_MENU4, /* menu line */
	ATR_MENU5, /* menu line */
	ATR_MENU6, /* menu line */
	ATR_CATCHMENU0, /* catch menu line */
	ATR_CATCHMENU1 /* catch menu line */
};
#endif
