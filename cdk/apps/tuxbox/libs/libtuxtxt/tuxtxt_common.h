#ifndef TUXTXT_COMMON_H
#define TUXTXT_COMMON_H

#include <config.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include "tuxtxt_def.h"
#ifndef HAVE_DREAMBOX_HARDWARE
#include <tuxbox.h>
#endif
#if TUXTXT_COMPRESS == 1
#include <zlib.h>
#endif

#if HAVE_DVB_API_VERSION < 3
#include <dbox/avia_gt_pig.h>
#else
#include <linux/input.h>
#include <linux/videodev.h>
#endif

const char *ObjectSource[] =
{
	"(illegal)",
	"Local",
	"POP",
	"GPOP"
};
const char *ObjectType[] =
{
	"Passive",
	"Active",
	"Adaptive",
	"Passive"
};
//const (avoid warnings :<)
tstPageAttr tuxtxt_atrtable[] =
{
	{ tuxtxt_color_white  , tuxtxt_color_black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_WB */
	{ tuxtxt_color_white  , tuxtxt_color_black , C_G0P, 0, 0, 1 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_PassiveDefault */
	{ tuxtxt_color_white  , tuxtxt_color_red   , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_L250 */
	{ tuxtxt_color_black  , tuxtxt_color_green , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_L251 */
	{ tuxtxt_color_black  , tuxtxt_color_yellow, C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_L252 */
	{ tuxtxt_color_white  , tuxtxt_color_blue  , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_L253 */
	{ tuxtxt_color_magenta, tuxtxt_color_black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_TOPMENU0 */
	{ tuxtxt_color_green  , tuxtxt_color_black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_TOPMENU1 */
	{ tuxtxt_color_yellow , tuxtxt_color_black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_TOPMENU2 */
	{ tuxtxt_color_cyan   , tuxtxt_color_black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_TOPMENU3 */
	{ tuxtxt_color_menu2  , tuxtxt_color_menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSG0 */
	{ tuxtxt_color_yellow , tuxtxt_color_menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSG1 */
	{ tuxtxt_color_menu2  , tuxtxt_color_transp, C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSG2 */
	{ tuxtxt_color_white  , tuxtxt_color_menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSG3 */
	{ tuxtxt_color_menu2  , tuxtxt_color_menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSGDRM0 */
	{ tuxtxt_color_yellow , tuxtxt_color_menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSGDRM1 */
	{ tuxtxt_color_menu2  , tuxtxt_color_black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSGDRM2 */
	{ tuxtxt_color_white  , tuxtxt_color_menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MSGDRM3 */
	{ tuxtxt_color_menu1  , tuxtxt_color_blue  , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENUHIL0 5a Z */
	{ tuxtxt_color_white  , tuxtxt_color_blue  , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENUHIL1 58 X */
	{ tuxtxt_color_menu2  , tuxtxt_color_transp, C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENUHIL2 9b › */
	{ tuxtxt_color_menu2  , tuxtxt_color_menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU0 ab « */
	{ tuxtxt_color_yellow , tuxtxt_color_menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU1 a4 ¤ */
	{ tuxtxt_color_menu2  , tuxtxt_color_transp, C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU2 9b › */
	{ tuxtxt_color_menu2  , tuxtxt_color_menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU3 cb Ë */
	{ tuxtxt_color_cyan   , tuxtxt_color_menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU4 c7 Ç */
	{ tuxtxt_color_white  , tuxtxt_color_menu3 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU5 c8 È */
	{ tuxtxt_color_white  , tuxtxt_color_menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_MENU6 a8 ¨ */
	{ tuxtxt_color_yellow , tuxtxt_color_menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}, /* ATR_CATCHMENU0 a4 ¤ */
	{ tuxtxt_color_white  , tuxtxt_color_menu1 , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f}  /* ATR_CATCHMENU1 a8 ¨ */
};

// G2 Set as defined in ETS 300 706
const unsigned short int G2table[4][6*16] =
{
	// Latin G2 Supplementary Set  
	{ 0x0020, 0x00A1, 0x00A2, 0x00A3, 0x0024, 0x00A5, 0x0023, 0x00A7, 0x00A4, 0x2018, 0x201C, 0x00AB, 0x2190, 0x2191, 0x2192, 0x2193,
	  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00D7, 0x00B5, 0x00B6, 0x00B7, 0x00F7, 0x2019, 0x201D, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	  0x0020, 0x0300, 0x0301, 0x02C6, 0x0303, 0x02C9, 0x02D8, 0x02D9, 0x00A8, 0x002E, 0x02DA, 0x00B8, 0x005F, 0x02DD, 0x02DB, 0x02C7,
	  0x2014, 0x00B9, 0x00AE, 0x00A9, 0x2122, 0x266A, 0x20AC, 0x2030, 0x03B1, 0x0020, 0x0020, 0x0020, 0x215B, 0x215C, 0x215D, 0x215E,
	  0x2126, 0x00C6, 0x00D0, 0x00AA, 0x0126, 0x0020, 0x0132, 0x013F, 0x0141, 0x00D8, 0x0152, 0x00BA, 0x00DE, 0x0166, 0x014A, 0x0149,
	  0x0138, 0x00E6, 0x0111, 0x00F0, 0x0127, 0x0131, 0x0133, 0x0140, 0x0142, 0x00F8, 0x0153, 0x00DF, 0x00FE, 0x0167, 0x014B, 0x25A0},
	// Cyrillic G2 Supplementary Set  
	{ 0x0020, 0x00A1, 0x00A2, 0x00A3, 0x0024, 0x00A5, 0x0020, 0x00A7, 0x0020, 0x2018, 0x201C, 0x00AB, 0x2190, 0x2191, 0x2192, 0x2193,
	  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00D7, 0x00B5, 0x00B6, 0x00B7, 0x00F7, 0x2019, 0x201D, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF,
	  0x0020, 0x0300, 0x0301, 0x02C6, 0x02DC, 0x02C9, 0x02D8, 0x02D9, 0x00A8, 0x002E, 0x02DA, 0x00B8, 0x005F, 0x02DD, 0x02DB, 0x02C7,
	  0x2014, 0x00B9, 0x00AE, 0x00A9, 0x2122, 0x266A, 0x20AC, 0x2030, 0x03B1, 0x0141, 0x0142, 0x00DF, 0x215B, 0x215C, 0x215D, 0x215E,
	  0x0044, 0x0045, 0x0046, 0x0047, 0x0049, 0x004A, 0x004B, 0x004C, 0x004E, 0x0051, 0x0052, 0x0053, 0x0055, 0x0056, 0x0057, 0x005A,
	  0x0064, 0x0065, 0x0066, 0x0067, 0x0069, 0x006A, 0x006B, 0x006C, 0x006E, 0x0071, 0x0072, 0x0073, 0x0075, 0x0076, 0x0077, 0x007A},
	// Greek G2 Supplementary Set  
	{ 0x0020, 0x0061, 0x0062, 0x00A3, 0x0065, 0x0068, 0x0069, 0x00A7, 0x003A, 0x2018, 0x201C, 0x006B, 0x2190, 0x2191, 0x2192, 0x2193,
	  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00D7, 0x006D, 0x006E, 0x0070, 0x00F7, 0x2019, 0x201D, 0x0074, 0x00BC, 0x00BD, 0x00BE, 0x0078,
	  0x0020, 0x0300, 0x0301, 0x02C6, 0x02DC, 0x02C9, 0x02D8, 0x02D9, 0x00A8, 0x002E, 0x02DA, 0x00B8, 0x005F, 0x02DD, 0x02DB, 0x02C7,
	  0x003F, 0x00B9, 0x00AE, 0x00A9, 0x2122, 0x266A, 0x20AC, 0x2030, 0x03B1, 0x038A, 0x038E, 0x038F, 0x215B, 0x215C, 0x215D, 0x215E,
	  0x0043, 0x0044, 0x0046, 0x0047, 0x004A, 0x004C, 0x0051, 0x0052, 0x0053, 0x0055, 0x0056, 0x0057, 0x0059, 0x005A, 0x0386, 0x0389,
	  0x0063, 0x0064, 0x0066, 0x0067, 0x006A, 0x006C, 0x0071, 0x0072, 0x0073, 0x0075, 0x0076, 0x0077, 0x0079, 0x007A, 0x0388, 0x25A0},
	// Arabic G2 Set
	{ 0x0020, 0x0639, 0xFEC9, 0xFE83, 0xFE85, 0xFE87, 0xFE8B, 0xFE89, 0xFB7C, 0xFB7D, 0xFB7A, 0xFB58, 0xFB59, 0xFB56, 0xFB6D, 0xFB8E,
	  0x0660, 0x0661, 0x0662, 0x0663, 0x0664, 0x0665, 0x0666, 0x0667, 0x0668, 0x0669, 0xFECE, 0xFECD, 0xFEFC, 0xFEEC, 0xFEEA, 0xFEE9,
	  0x00E0, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00EB, 0x00EA, 0x00F9, 0x00EE, 0xFECA,
	  0x00E9, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x00E2, 0x00F4, 0x00FB, 0x00E7, 0x25A0}
};
// G0 Table as defined in ETS 300 706
// cyrillic G0 Charset (0 = Serbian/Croatian, 1 = Russian/Bulgarian, 2 = Ukrainian)
const unsigned short int G0table[6][6*16] =
{
	// Cyrillic G0 Set - Option 1 - Serbian/Croatian
	{ ' ', '!', '\"', '#', '$', '%', '&', '\'', '(' , ')' , '*', '+', ',', '-', '.', '/', 
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	  0x0427, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413, 0x0425, 0x0418, 0x0408, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
	  0x041F, 0x040C, 0x0420, 0x0421, 0x0422, 0x0423, 0x0412, 0x0403, 0x0409, 0x040A, 0x0417, 0x040B, 0x0416, 0x0402, 0x0428, 0x040F,
	  0x0447, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433, 0x0445, 0x0438, 0x0458, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
	  0x043F, 0x045C, 0x0440, 0x0441, 0x0442, 0x0443, 0x0432, 0x0453, 0x0459, 0x045A, 0x0437, 0x045B, 0x0436, 0x0452, 0x0448, 0x25A0},
	// Cyrillic G0 Set - Option 2 - Russian/Bulgarian
	{ ' ', '!', '\"', '#', '$', '%', 0x044B, '\'', '(' , ')' , '*', '+', ',', '-', '.', '/', 
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	  0x042E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413, 0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
	  0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412, 0x042C, 0x042A, 0x0417, 0x0428, 0x042D, 0x0429, 0x0427, 0x042B,
	  0x044E, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433, 0x0445, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
	  0x043F, 0x044F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432, 0x044C, 0x044A, 0x0437, 0x0448, 0x044D, 0x0449, 0x0447, 0x25A0},
	// Cyrillic G0 Set - Option 3 - Ukrainian
	{ ' ', '!', '\"', '#', '$', '%', 0x0457, '\'', '(' , ')' , '*', '+', ',', '-', '.', '/', 
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	  0x042E, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413, 0x0425, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 0x041D, 0x041E,
	  0x041F, 0x042F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412, 0x042C, 0x0406, 0x0417, 0x0428, 0x0404, 0x0429, 0x0427, 0x0407,
	  0x044E, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433, 0x0445, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
	  0x043F, 0x044F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432, 0x044C, 0x0456, 0x0437, 0x0448, 0x0454, 0x0449, 0x0447, 0x25A0},
	// Greek G0 Set
	{ ' ', '!', '\"', '#', '$', '%', '&', '\'', '(' , ')' , '*', '+', ',', '-', '.', '/', 
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', 0x00AB, '=', 0x00BB, '?',
	  0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 0x039E, 0x039F,
	  0x03A0, 0x03A1, 0x0384, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x03AC, 0x03AD, 0x03AE, 0x03AF,
	  0x03B0, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 0x03B5, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 0x03BE, 0x03BF,
	  0x03C0, 0x03C1, 0x03C2, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x25A0},
	// Hebrew G0 Set
	{ ' ', '!', 0x05F2, 0x00A3, '$', '%', '&', '\'', '(' , ')' , '*', '+', ',', '-', '.', '/', 
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	  '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0x2190, 0x00BD, 0x2192, 0x2191, '#',
	  0x05D0, 0x05D1, 0x05D2, 0x05D3, 0x05D4, 0x05D5, 0x05D6, 0x05D7, 0x05D8, 0x05D9, 0x05DA, 0x05DB, 0x05DC, 0x05DD, 0x05DE, 0x05DF,
	  0x05E0, 0x05E1, 0x05E2, 0x05E3, 0x05E4, 0x05E5, 0x05E6, 0x05E7, 0x05E8, 0x05E9, 0x05EA, 0x20AA, 0x2551, 0x00BE, 0x00F7, 0x25A0},
	// Arabic G0 Set - Thanks to Habib2006(fannansat)
	{ ' ', '!', 0x05F2, 0x00A3, '$', 0x066A, 0xFEF0, 0xFEF2, 0xFD3F, 0xFD3E, '*', '+', ',', '-', '.', '/', 
	  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', 0x061B, '>', '=', '<', 0x061F,
	  0xFE94, 0x0621, 0xFE92, 0x0628, 0xFE98, 0x062A, 0xFE8E, 0xFE8D, 0xFE91, 0xFE93, 0xFE97, 0xFE9B, 0xFE9F, 0xFEA3, 0xFEA7, 0xFEA9,
	  0x0630, 0xFEAD, 0xFEAF, 0xFEB3, 0xFEB7, 0xFEBB, 0xFEBF, 0xFEC1, 0xFEC5, 0xFECB, 0xFECF, 0xFE9C, 0xFEA0, 0xFEA4, 0xFEA8, 0x0023,
	  0x0640, 0xFED3, 0xFED7, 0xFEDB, 0xFEDF, 0xFEE3, 0xFEE7, 0xFEEB, 0xFEED, 0xFEEF, 0xFEF3, 0xFE99, 0xFE9D, 0xFEA1, 0xFEA5, 0xFEF4,
	  0xFEF0, 0xFECC, 0xFED0, 0xFED4, 0xFED1, 0xFED8, 0xFED5, 0xFED9, 0xFEE0, 0xFEDD, 0xFEE4, 0xFEE1, 0xFEE8, 0xFEE5, 0xFEFB, 0x25A0}
};

const unsigned short int nationaltable23[14][2] =
{
	{ '#',    0x00A4 }, /* 0          */
	{ '#',    0x016F }, /* 1  CS/SK   */
	{ 0x00A3,    '$' }, /* 2    EN    */
	{ '#',    0x00F5 }, /* 3    ET    */
	{ 0x00E9, 0x0457 }, /* 4    FR    */
	{ '#',       '$' }, /* 5    DE    */
	{ 0x00A3,    '$' }, /* 6    IT    */
	{ '#',       '$' }, /* 7  LV/LT   */
	{ '#',    0x0144 }, /* 8    PL    */
	{ 0x00E7,    '$' }, /* 9  PT/ES   */
	{ '#',    0x00A4 }, /* A    RO    */
	{ '#',    0x00CB }, /* B SR/HR/SL */
	{ '#',    0x00A4 }, /* C SV/FI/HU */
	{ 0x20A4, 0x011F }, /* D    TR    */
};
const unsigned short int nationaltable40[14] =
{
	'@',    /* 0          */
	0x010D, /* 1  CS/SK   */
	'@',    /* 2    EN    */
	0x0161, /* 3    ET    */
	0x00E0, /* 4    FR    */
	0x00A7, /* 5    DE    */
	0x00E9, /* 6    IT    */
	0x0161, /* 7  LV/LT   */
	0x0105, /* 8    PL    */
	0x00A1, /* 9  PT/ES   */
	0x0162, /* A    RO    */
	0x010C, /* B SR/HR/SL */
	0x00C9, /* C SV/FI/HU */
	0x0130, /* D    TR    */
};
const unsigned short int nationaltable5b[14][6] =
{
	{    '[',   '\\',    ']',    '^',    '_',    '`' }, /* 0          */
	{ 0x0165, 0x017E, 0x00FD, 0x00ED, 0x0159, 0x00E9 }, /* 1  CS/SK   */
	{ 0x2190, 0x00BD, 0x2192, 0x2191,    '#', 0x00AD }, /* 2    EN    */
	{ 0x00C4, 0x00D6, 0x017D, 0x00DC, 0x00D5, 0x0161 }, /* 3    ET    */
	{ 0x0451, 0x00EA, 0x00F9, 0x00EE,    '#', 0x00E8 }, /* 4    FR    */
	{ 0x00C4, 0x00D6, 0x00DC,    '^',    '_', 0x00B0 }, /* 5    DE    */
	{ 0x00B0, 0x00E7, 0x2192, 0x2191,    '#', 0x00F9 }, /* 6    IT    */
	{ 0x0117, 0x0119, 0x017D, 0x010D, 0x016B, 0x0161 }, /* 7  LV/LT   */
	{ 0x017B, 0x015A, 0x0141, 0x0107, 0x00F3, 0x0119 }, /* 8    PL    */
	{ 0x00E1, 0x00E9, 0x00ED, 0x00F3, 0x00FA, 0x00BF }, /* 9  PT/ES   */
	{ 0x00C2, 0x015E, 0x01CD, 0x01CF, 0x0131, 0x0163 }, /* A    RO    */
	{ 0x0106, 0x017D, 0x00D0, 0x0160, 0x0451, 0x010D }, /* B SR/HR/SL */
	{ 0x00C4, 0x00D6, 0x00C5, 0x00DC,    '_', 0x00E9 }, /* C SV/FI/HU */
	{ 0x015E, 0x00D6, 0x00C7, 0x00DC, 0x011E, 0x0131 }, /* D    TR    */
};
const unsigned short int nationaltable7b[14][4] =
{
	{ '{',       '|',    '}',    '~' }, /* 0          */
	{ 0x00E1, 0x011B, 0x00FA, 0x0161 }, /* 1  CS/SK   */
	{ 0x00BC, 0x2551, 0x00BE, 0x00F7 }, /* 2    EN    */
	{ 0x00E4, 0x00F6, 0x017E, 0x00FC }, /* 3    ET    */
	{ 0x00E2, 0x00F4, 0x00FB, 0x00E7 }, /* 4    FR    */
	{ 0x00E4, 0x00F6, 0x00FC, 0x00DF }, /* 5    DE    */
	{ 0x00E0, 0x00F3, 0x00E8, 0x00EC }, /* 6    IT    */
	{ 0x0105, 0x0173, 0x017E, 0x012F }, /* 7  LV/LT   */
	{ 0x017C, 0x015B, 0x0142, 0x017A }, /* 8    PL    */
	{ 0x00FC, 0x00F1, 0x00E8, 0x00E0 }, /* 9  PT/ES   */
	{ 0x00E2, 0x015F, 0x01CE, 0x00EE }, /* A    RO    */
	{ 0x0107, 0x017E, 0x0111, 0x0161 }, /* B SR/HR/SL */
	{ 0x00E4, 0x00F6, 0x00E5, 0x00FC }, /* C SV/FI/HU */
	{ 0x015F, 0x00F6, 0x00E7, 0x00FC }, /* D    TR    */
};
const unsigned short int arrowtable[] =
{
	8592, 8594, 8593, 8595, 'O', 'K', 8592, 8592
};

/* hamming table */
const unsigned char dehamming[] =
{
	0x01, 0xFF, 0x01, 0x01, 0xFF, 0x00, 0x01, 0xFF, 0xFF, 0x02, 0x01, 0xFF, 0x0A, 0xFF, 0xFF, 0x07,
	0xFF, 0x00, 0x01, 0xFF, 0x00, 0x00, 0xFF, 0x00, 0x06, 0xFF, 0xFF, 0x0B, 0xFF, 0x00, 0x03, 0xFF,
	0xFF, 0x0C, 0x01, 0xFF, 0x04, 0xFF, 0xFF, 0x07, 0x06, 0xFF, 0xFF, 0x07, 0xFF, 0x07, 0x07, 0x07,
	0x06, 0xFF, 0xFF, 0x05, 0xFF, 0x00, 0x0D, 0xFF, 0x06, 0x06, 0x06, 0xFF, 0x06, 0xFF, 0xFF, 0x07,
	0xFF, 0x02, 0x01, 0xFF, 0x04, 0xFF, 0xFF, 0x09, 0x02, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0x03, 0xFF,
	0x08, 0xFF, 0xFF, 0x05, 0xFF, 0x00, 0x03, 0xFF, 0xFF, 0x02, 0x03, 0xFF, 0x03, 0xFF, 0x03, 0x03,
	0x04, 0xFF, 0xFF, 0x05, 0x04, 0x04, 0x04, 0xFF, 0xFF, 0x02, 0x0F, 0xFF, 0x04, 0xFF, 0xFF, 0x07,
	0xFF, 0x05, 0x05, 0x05, 0x04, 0xFF, 0xFF, 0x05, 0x06, 0xFF, 0xFF, 0x05, 0xFF, 0x0E, 0x03, 0xFF,
	0xFF, 0x0C, 0x01, 0xFF, 0x0A, 0xFF, 0xFF, 0x09, 0x0A, 0xFF, 0xFF, 0x0B, 0x0A, 0x0A, 0x0A, 0xFF,
	0x08, 0xFF, 0xFF, 0x0B, 0xFF, 0x00, 0x0D, 0xFF, 0xFF, 0x0B, 0x0B, 0x0B, 0x0A, 0xFF, 0xFF, 0x0B,
	0x0C, 0x0C, 0xFF, 0x0C, 0xFF, 0x0C, 0x0D, 0xFF, 0xFF, 0x0C, 0x0F, 0xFF, 0x0A, 0xFF, 0xFF, 0x07,
	0xFF, 0x0C, 0x0D, 0xFF, 0x0D, 0xFF, 0x0D, 0x0D, 0x06, 0xFF, 0xFF, 0x0B, 0xFF, 0x0E, 0x0D, 0xFF,
	0x08, 0xFF, 0xFF, 0x09, 0xFF, 0x09, 0x09, 0x09, 0xFF, 0x02, 0x0F, 0xFF, 0x0A, 0xFF, 0xFF, 0x09,
	0x08, 0x08, 0x08, 0xFF, 0x08, 0xFF, 0xFF, 0x09, 0x08, 0xFF, 0xFF, 0x0B, 0xFF, 0x0E, 0x03, 0xFF,
	0xFF, 0x0C, 0x0F, 0xFF, 0x04, 0xFF, 0xFF, 0x09, 0x0F, 0xFF, 0x0F, 0x0F, 0xFF, 0x0E, 0x0F, 0xFF,
	0x08, 0xFF, 0xFF, 0x05, 0xFF, 0x0E, 0x0D, 0xFF, 0xFF, 0x0E, 0x0F, 0xFF, 0x0E, 0x0E, 0xFF, 0x0E
};

/* odd parity table, error=0x20 (space) */
const unsigned char deparity[] =
{
	' ' , 0x01, 0x02, ' ' , 0x04, ' ' , ' ' , 0x07, 0x08, ' ' , ' ' , 0x0b, ' ' , 0x0d, 0x0e, ' ' ,
	0x10, ' ' , ' ' , 0x13, ' ' , 0x15, 0x16, ' ' , ' ' , 0x19, 0x1a, ' ' , 0x1c, ' ' , ' ' , 0x1f,
	0x20, ' ' , ' ' , 0x23, ' ' , 0x25, 0x26, ' ' , ' ' , 0x29, 0x2a, ' ' , 0x2c, ' ' , ' ' , 0x2f,
	' ' , 0x31, 0x32, ' ' , 0x34, ' ' , ' ' , 0x37, 0x38, ' ' , ' ' , 0x3b, ' ' , 0x3d, 0x3e, ' ' ,
	0x40, ' ' , ' ' , 0x43, ' ' , 0x45, 0x46, ' ' , ' ' , 0x49, 0x4a, ' ' , 0x4c, ' ' , ' ' , 0x4f,
	' ' , 0x51, 0x52, ' ' , 0x54, ' ' , ' ' , 0x57, 0x58, ' ' , ' ' , 0x5b, ' ' , 0x5d, 0x5e, ' ' ,
	' ' , 0x61, 0x62, ' ' , 0x64, ' ' , ' ' , 0x67, 0x68, ' ' , ' ' , 0x6b, ' ' , 0x6d, 0x6e, ' ' ,
	0x70, ' ' , ' ' , 0x73, ' ' , 0x75, 0x76, ' ' , ' ' , 0x79, 0x7a, ' ' , 0x7c, ' ' , ' ' , 0x7f,
	0x00, ' ' , ' ' , 0x03, ' ' , 0x05, 0x06, ' ' , ' ' , 0x09, 0x0a, ' ' , 0x0c, ' ' , ' ' , 0x0f,
	' ' , 0x11, 0x12, ' ' , 0x14, ' ' , ' ' , 0x17, 0x18, ' ' , ' ' , 0x1b, ' ' , 0x1d, 0x1e, ' ' ,
	' ' , 0x21, 0x22, ' ' , 0x24, ' ' , ' ' , 0x27, 0x28, ' ' , ' ' , 0x2b, ' ' , 0x2d, 0x2e, ' ' ,
	0x30, ' ' , ' ' , 0x33, ' ' , 0x35, 0x36, ' ' , ' ' , 0x39, 0x3a, ' ' , 0x3c, ' ' , ' ' , 0x3f,
	' ' , 0x41, 0x42, ' ' , 0x44, ' ' , ' ' , 0x47, 0x48, ' ' , ' ' , 0x4b, ' ' , 0x4d, 0x4e, ' ' ,
	0x50, ' ' , ' ' , 0x53, ' ' , 0x55, 0x56, ' ' , ' ' , 0x59, 0x5a, ' ' , 0x5c, ' ' , ' ' , 0x5f,
	0x60, ' ' , ' ' , 0x63, ' ' , 0x65, 0x66, ' ' , ' ' , 0x69, 0x6a, ' ' , 0x6c, ' ' , ' ' , 0x6f,
	' ' , 0x71, 0x72, ' ' , 0x74, ' ' , ' ' , 0x77, 0x78, ' ' , ' ' , 0x7b, ' ' , 0x7d, 0x7e, ' ' ,
};

#if 1	/* lookup-table algorithm for decoding Hamming 24/18, credits to: */
/*
 *  libzvbi - Error correction functions
 *
 *  Copyright (C) 2001 Michael H. Schimek
 *
 *  Based on code from AleVT 1.5.1
 *  Copyright (C) 1998, 1999 Edgar Toernig <froese@gmx.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 *  [AleVT]
 *
 *  This table generates the parity checks for hamm24/18 decoding.
 *  Bit 0 is for test A, 1 for B, ...
 *
 *  Thanks to R. Gancarz for this fine table *g*
 */
const unsigned char hamm24par[3][256] = {
    {
        /* Parities of first byte */
	 0, 33, 34,  3, 35,  2,  1, 32, 36,  5,  6, 39,  7, 38, 37,  4,
	37,  4,  7, 38,  6, 39, 36,  5,  1, 32, 35,  2, 34,  3,  0, 33,
	38,  7,  4, 37,  5, 36, 39,  6,  2, 35, 32,  1, 33,  0,  3, 34,
	 3, 34, 33,  0, 32,  1,  2, 35, 39,  6,  5, 36,  4, 37, 38,  7,
	39,  6,  5, 36,  4, 37, 38,  7,  3, 34, 33,  0, 32,  1,  2, 35,
	 2, 35, 32,  1, 33,  0,  3, 34, 38,  7,  4, 37,  5, 36, 39,  6,
	 1, 32, 35,  2, 34,  3,  0, 33, 37,  4,  7, 38,  6, 39, 36,  5,
	36,  5,  6, 39,  7, 38, 37,  4,  0, 33, 34,  3, 35,  2,  1, 32,
	40,  9, 10, 43, 11, 42, 41,  8, 12, 45, 46, 15, 47, 14, 13, 44,
	13, 44, 47, 14, 46, 15, 12, 45, 41,  8, 11, 42, 10, 43, 40,  9,
	14, 47, 44, 13, 45, 12, 15, 46, 42, 11,  8, 41,  9, 40, 43, 10,
	43, 10,  9, 40,  8, 41, 42, 11, 15, 46, 45, 12, 44, 13, 14, 47,
	15, 46, 45, 12, 44, 13, 14, 47, 43, 10,  9, 40,  8, 41, 42, 11,
	42, 11,  8, 41,  9, 40, 43, 10, 14, 47, 44, 13, 45, 12, 15, 46,
	41,  8, 11, 42, 10, 43, 40,  9, 13, 44, 47, 14, 46, 15, 12, 45,
	12, 45, 46, 15, 47, 14, 13, 44, 40,  9, 10, 43, 11, 42, 41,  8
    }, {
        /* Parities of second byte */
	 0, 41, 42,  3, 43,  2,  1, 40, 44,  5,  6, 47,  7, 46, 45,  4,
	45,  4,  7, 46,  6, 47, 44,  5,  1, 40, 43,  2, 42,  3,  0, 41,
	46,  7,  4, 45,  5, 44, 47,  6,  2, 43, 40,  1, 41,  0,  3, 42,
	 3, 42, 41,  0, 40,  1,  2, 43, 47,  6,  5, 44,  4, 45, 46,  7,
	47,  6,  5, 44,  4, 45, 46,  7,  3, 42, 41,  0, 40,  1,  2, 43,
	 2, 43, 40,  1, 41,  0,  3, 42, 46,  7,  4, 45,  5, 44, 47,  6,
	 1, 40, 43,  2, 42,  3,  0, 41, 45,  4,  7, 46,  6, 47, 44,  5,
	44,  5,  6, 47,  7, 46, 45,  4,  0, 41, 42,  3, 43,  2,  1, 40,
	48, 25, 26, 51, 27, 50, 49, 24, 28, 53, 54, 31, 55, 30, 29, 52,
	29, 52, 55, 30, 54, 31, 28, 53, 49, 24, 27, 50, 26, 51, 48, 25,
	30, 55, 52, 29, 53, 28, 31, 54, 50, 27, 24, 49, 25, 48, 51, 26,
	51, 26, 25, 48, 24, 49, 50, 27, 31, 54, 53, 28, 52, 29, 30, 55,
	31, 54, 53, 28, 52, 29, 30, 55, 51, 26, 25, 48, 24, 49, 50, 27,
	50, 27, 24, 49, 25, 48, 51, 26, 30, 55, 52, 29, 53, 28, 31, 54,
	49, 24, 27, 50, 26, 51, 48, 25, 29, 52, 55, 30, 54, 31, 28, 53,
	28, 53, 54, 31, 55, 30, 29, 52, 48, 25, 26, 51, 27, 50, 49, 24
    }, {
        /* Parities of third byte */
	63, 14, 13, 60, 12, 61, 62, 15, 11, 58, 57,  8, 56,  9, 10, 59,
	10, 59, 56,  9, 57,  8, 11, 58, 62, 15, 12, 61, 13, 60, 63, 14,
	 9, 56, 59, 10, 58, 11,  8, 57, 61, 12, 15, 62, 14, 63, 60, 13,
	60, 13, 14, 63, 15, 62, 61, 12,  8, 57, 58, 11, 59, 10,  9, 56,
	 8, 57, 58, 11, 59, 10,  9, 56, 60, 13, 14, 63, 15, 62, 61, 12,
	61, 12, 15, 62, 14, 63, 60, 13,  9, 56, 59, 10, 58, 11,  8, 57,
	62, 15, 12, 61, 13, 60, 63, 14, 10, 59, 56,  9, 57,  8, 11, 58,
	11, 58, 57,  8, 56,  9, 10, 59, 63, 14, 13, 60, 12, 61, 62, 15,
	31, 46, 45, 28, 44, 29, 30, 47, 43, 26, 25, 40, 24, 41, 42, 27,
	42, 27, 24, 41, 25, 40, 43, 26, 30, 47, 44, 29, 45, 28, 31, 46,
	41, 24, 27, 42, 26, 43, 40, 25, 29, 44, 47, 30, 46, 31, 28, 45,
	28, 45, 46, 31, 47, 30, 29, 44, 40, 25, 26, 43, 27, 42, 41, 24,
	40, 25, 26, 43, 27, 42, 41, 24, 28, 45, 46, 31, 47, 30, 29, 44,
	29, 44, 47, 30, 46, 31, 28, 45, 41, 24, 27, 42, 26, 43, 40, 25,
	30, 47, 44, 29, 45, 28, 31, 46, 42, 27, 24, 41, 25, 40, 43, 26,
	43, 26, 25, 40, 24, 41, 42, 27, 31, 46, 45, 28, 44, 29, 30, 47
    }
};

/*
 *  [AleVT]
 *
 *  Table to extract the lower 4 bit from hamm24/18 encoded bytes
 */
const unsigned char hamm24val[256] = {
      0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,
      2,  2,  2,  2,  3,  3,  3,  3,  2,  2,  2,  2,  3,  3,  3,  3,
      4,  4,  4,  4,  5,  5,  5,  5,  4,  4,  4,  4,  5,  5,  5,  5,
      6,  6,  6,  6,  7,  7,  7,  7,  6,  6,  6,  6,  7,  7,  7,  7,
      8,  8,  8,  8,  9,  9,  9,  9,  8,  8,  8,  8,  9,  9,  9,  9,
     10, 10, 10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 11,
     12, 12, 12, 12, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13, 13, 13,
     14, 14, 14, 14, 15, 15, 15, 15, 14, 14, 14, 14, 15, 15, 15, 15,
      0,  0,  0,  0,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,
      2,  2,  2,  2,  3,  3,  3,  3,  2,  2,  2,  2,  3,  3,  3,  3,
      4,  4,  4,  4,  5,  5,  5,  5,  4,  4,  4,  4,  5,  5,  5,  5,
      6,  6,  6,  6,  7,  7,  7,  7,  6,  6,  6,  6,  7,  7,  7,  7,
      8,  8,  8,  8,  9,  9,  9,  9,  8,  8,  8,  8,  9,  9,  9,  9,
     10, 10, 10, 10, 11, 11, 11, 11, 10, 10, 10, 10, 11, 11, 11, 11,
     12, 12, 12, 12, 13, 13, 13, 13, 12, 12, 12, 12, 13, 13, 13, 13,
     14, 14, 14, 14, 15, 15, 15, 15, 14, 14, 14, 14, 15, 15, 15, 15
};

const signed char hamm24err[64] = {
     0, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,
    -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,  -1, -1, -1, -1,
     0,  0,  0,  0,   0,  0,  0,  0,   0,  0,  0,  0,   0,  0,  0,  0,
     0,  0,  0,  0,   0,  0,  0,  0,  -1, -1, -1, -1,  -1, -1, -1, -1,
};

/*
 *  [AleVT]
 *
 *  Mapping from parity checks made by table hamm24par to faulty bit
 *  in the decoded 18 bit word.
 */
const unsigned int hamm24cor[64] = {
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
    0x00000, 0x00000, 0x00000, 0x00001, 0x00000, 0x00002, 0x00004, 0x00008,
    0x00000, 0x00010, 0x00020, 0x00040, 0x00080, 0x00100, 0x00200, 0x00400,
    0x00000, 0x00800, 0x01000, 0x02000, 0x04000, 0x08000, 0x10000, 0x20000,
    0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000, 0x00000,
};

/**
 * @internal
 * @param p Pointer to a byte triplet, bytes in transmission order,
 *   lsb first transmitted.
 *
 * This function decodes a Hamming 24/18 protected byte triplet
 * as specified in ETS 300 706 8.3.
 *
 * @return
 * Triplet data bits D18 [msb] ... D1 [lsb] or a negative
 * value if the triplet contained incorrectable errors.
 */
signed int deh24(unsigned char *p)
{
	int e = hamm24par[0][p[0]]
		^ hamm24par[1][p[1]]
		^ hamm24par[2][p[2]];

	int x = hamm24val[p[0]]
		+ (p[1] & 127) * 16
		+ (p[2] & 127) * 2048;

	return (x ^ hamm24cor[e]) | hamm24err[e];
}

#else	 /* my (rm) slower but smaller solution without lookup tables */

/* calc parity */
int parity(int c)
{
	int n = 0;
	for (; c; c &= (c-1)) /* reset least significant set bit */
		n ^= 1;
	return n;
}

#if 0	/* just for testing */
/* encode hamming 24/18 */
unsigned int ham24(unsigned int val)
{
	val = ((val & 0x000001) << 2) |
		((val & 0x00000e) << 3) |
		((val & 0x0007f0) << 4) |
		((val & 0x03f800) << 5);
	val |= parity(val & 0x555554);
	val |= parity(val & 0x666664) << 1;
	val |= parity(val & 0x787870) << 3;
	val |= parity(val & 0x007f00) << 7;
	val |= parity(val & 0x7f0000) << 15;
	val |= parity(val) << 23;
	return val;
}
#endif

/* decode hamming 24/18, error=-1 */
signed int deh24(unsigned char *ph24)
{
	int h24 = *ph24 | (*(ph24+1)<<8) | (*(ph24+2)<<16);
	int a = parity(h24 & 0x555555);
	int f = parity(h24 & 0xaaaaaa) ^ a;
	a |= (parity(h24 & 0x666666) << 1) |
		(parity(h24 & 0x787878) << 2) |
		(parity(h24 & 0x007f80) << 3) |
		(parity(h24 & 0x7f8000) << 4);
	if (a != 0x1f)
	{
		if (f) /* 2 biterrors */
			return -1;
		else /* correct 1 biterror */
			h24 ^= (1 << ((a ^ 0x1f)-1));
	}
	return
		((h24 & 0x000004) >> 2) |
		((h24 & 0x000070) >> 3) |
		((h24 & 0x007f00) >> 4) |
		((h24 & 0x7f0000) >> 5);
}
#endif /* table or serial */

/* shapes */
enum
{
	S_END = 0,
	S_FHL, /* full horizontal line: y-offset */
	S_FVL, /* full vertical line: x-offset */
	S_BOX, /* rectangle: x-offset, y-offset, width, height */
	S_TRA, /* trapez: x0, y0, l0, x1, y1, l1 */
	S_BTR, /* trapez in bgcolor: x0, y0, l0, x1, y1, l1 */
	S_INV, /* invert */
	S_LNK, /* call other shape: shapenumber */
	S_CHR, /* Character from freetype hibyte, lowbyte */
	S_ADT, /* Character 2F alternating raster */
	S_FLH, /* flip horizontal */
	S_FLV  /* flip vertical */
};

/* shape coordinates */
enum
{
	S_W13 = 5, /* width*1/3 */
	S_W12, /* width*1/2 */
	S_W23, /* width*2/3 */
	S_W11, /* width */
	S_WM3, /* width-3 */
	S_H13, /* height*1/3 */
	S_H12, /* height*1/2 */
	S_H23, /* height*2/3 */
	S_H11, /* height */
	S_NrShCoord
};

/* G3 characters */
unsigned char aG3_20[] = { S_TRA, 0, S_H23, 1, 0, S_H11, S_W12, S_END };
unsigned char aG3_21[] = { S_TRA, 0, S_H23, 1, 0, S_H11, S_W11, S_END };
unsigned char aG3_22[] = { S_TRA, 0, S_H12, 1, 0, S_H11, S_W12, S_END };
unsigned char aG3_23[] = { S_TRA, 0, S_H12, 1, 0, S_H11, S_W11, S_END };
unsigned char aG3_24[] = { S_TRA, 0, 0, 1, 0, S_H11, S_W12, S_END };
unsigned char aG3_25[] = { S_TRA, 0, 0, 1, 0, S_H11, S_W11, S_END };
unsigned char aG3_26[] = { S_INV, S_LNK, 0x66, S_END };
unsigned char aG3_27[] = { S_INV, S_LNK, 0x67, S_END };
unsigned char aG3_28[] = { S_INV, S_LNK, 0x68, S_END };
unsigned char aG3_29[] = { S_INV, S_LNK, 0x69, S_END };
unsigned char aG3_2a[] = { S_INV, S_LNK, 0x6a, S_END };
unsigned char aG3_2b[] = { S_INV, S_LNK, 0x6b, S_END };
unsigned char aG3_2c[] = { S_INV, S_LNK, 0x6c, S_END };
unsigned char aG3_2d[] = { S_INV, S_LNK, 0x6d, S_END };
unsigned char aG3_2e[] = { S_BOX, 2, 0, 3, S_H11, S_END };
unsigned char aG3_2f[] = { S_ADT };
unsigned char aG3_30[] = { S_LNK, 0x20, S_FLH, S_END };
unsigned char aG3_31[] = { S_LNK, 0x21, S_FLH, S_END };
unsigned char aG3_32[] = { S_LNK, 0x22, S_FLH, S_END };
unsigned char aG3_33[] = { S_LNK, 0x23, S_FLH, S_END };
unsigned char aG3_34[] = { S_LNK, 0x24, S_FLH, S_END };
unsigned char aG3_35[] = { S_LNK, 0x25, S_FLH, S_END };
unsigned char aG3_36[] = { S_INV, S_LNK, 0x76, S_END };
unsigned char aG3_37[] = { S_INV, S_LNK, 0x77, S_END };
unsigned char aG3_38[] = { S_INV, S_LNK, 0x78, S_END };
unsigned char aG3_39[] = { S_INV, S_LNK, 0x79, S_END };
unsigned char aG3_3a[] = { S_INV, S_LNK, 0x7a, S_END };
unsigned char aG3_3b[] = { S_INV, S_LNK, 0x7b, S_END };
unsigned char aG3_3c[] = { S_INV, S_LNK, 0x7c, S_END };
unsigned char aG3_3d[] = { S_INV, S_LNK, 0x7d, S_END };
unsigned char aG3_3e[] = { S_LNK, 0x2e, S_FLH, S_END };
unsigned char aG3_3f[] = { S_BOX, 0, 0, S_W11, S_H11, S_END };
unsigned char aG3_40[] = { S_BOX, 0, S_H13, S_W11, S_H13, S_LNK, 0x7e, S_END };
unsigned char aG3_41[] = { S_BOX, 0, S_H13, S_W11, S_H13, S_LNK, 0x7e, S_FLV, S_END };
unsigned char aG3_42[] = { S_LNK, 0x50, S_BOX, S_W12, S_H13, S_W12, S_H13, S_END };
unsigned char aG3_43[] = { S_LNK, 0x50, S_BOX, 0, S_H13, S_W12, S_H13, S_END };
unsigned char aG3_44[] = { S_LNK, 0x48, S_FLV, S_LNK, 0x48, S_END };
unsigned char aG3_45[] = { S_LNK, 0x44, S_FLH, S_END };
unsigned char aG3_46[] = { S_LNK, 0x47, S_FLV, S_END };
unsigned char aG3_47[] = { S_LNK, 0x48, S_FLH, S_LNK, 0x48, S_END };
unsigned char aG3_48[] = { S_TRA, 0, 0, S_W23, 0, S_H23, 0, S_BTR, 0, 0, S_W13, 0, S_H13, 0, S_END };
unsigned char aG3_49[] = { S_LNK, 0x48, S_FLH, S_END };
unsigned char aG3_4a[] = { S_LNK, 0x48, S_FLV, S_END };
unsigned char aG3_4b[] = { S_LNK, 0x48, S_FLH, S_FLV, S_END };
unsigned char aG3_4c[] = { S_LNK, 0x50, S_BOX, 0, S_H13, S_W11, S_H13, S_END };
unsigned char aG3_4d[] = { S_CHR, 0x25, 0xE6 };
unsigned char aG3_4e[] = { S_CHR, 0x25, 0xCF };
unsigned char aG3_4f[] = { S_CHR, 0x25, 0xCB };
unsigned char aG3_50[] = { S_BOX, S_W12, 0, 2, S_H11, S_FLH, S_BOX, S_W12, 0, 2, S_H11,S_END };
unsigned char aG3_51[] = { S_BOX, 0, S_H12, S_W11, 2, S_FLV, S_BOX, 0, S_H12, S_W11, 2,S_END };
unsigned char aG3_52[] = { S_LNK, 0x55, S_FLH, S_FLV, S_END };
unsigned char aG3_53[] = { S_LNK, 0x55, S_FLV, S_END };
unsigned char aG3_54[] = { S_LNK, 0x55, S_FLH, S_END };
unsigned char aG3_55[] = { S_LNK, 0x7e, S_FLV, S_BOX, 0, S_H12, S_W12, 2, S_FLV, S_BOX, 0, S_H12, S_W12, 2, S_END };
unsigned char aG3_56[] = { S_LNK, 0x57, S_FLH, S_END};
unsigned char aG3_57[] = { S_LNK, 0x55, S_LNK, 0x50 , S_END};
unsigned char aG3_58[] = { S_LNK, 0x59, S_FLV, S_END};
unsigned char aG3_59[] = { S_LNK, 0x7e, S_LNK, 0x51 , S_END};
unsigned char aG3_5a[] = { S_LNK, 0x50, S_LNK, 0x51 , S_END};
unsigned char aG3_5b[] = { S_CHR, 0x21, 0x92};
unsigned char aG3_5c[] = { S_CHR, 0x21, 0x90};
unsigned char aG3_5d[] = { S_CHR, 0x21, 0x91};
unsigned char aG3_5e[] = { S_CHR, 0x21, 0x93};
unsigned char aG3_5f[] = { S_CHR, 0x00, 0x20};
unsigned char aG3_60[] = { S_INV, S_LNK, 0x20, S_END };
unsigned char aG3_61[] = { S_INV, S_LNK, 0x21, S_END };
unsigned char aG3_62[] = { S_INV, S_LNK, 0x22, S_END };
unsigned char aG3_63[] = { S_INV, S_LNK, 0x23, S_END };
unsigned char aG3_64[] = { S_INV, S_LNK, 0x24, S_END };
unsigned char aG3_65[] = { S_INV, S_LNK, 0x25, S_END };
unsigned char aG3_66[] = { S_LNK, 0x20, S_FLV, S_END };
unsigned char aG3_67[] = { S_LNK, 0x21, S_FLV, S_END };
unsigned char aG3_68[] = { S_LNK, 0x22, S_FLV, S_END };
unsigned char aG3_69[] = { S_LNK, 0x23, S_FLV, S_END };
unsigned char aG3_6a[] = { S_LNK, 0x24, S_FLV, S_END };
unsigned char aG3_6b[] = { S_BOX, 0, 0, S_W11, S_H13, S_TRA, 0, S_H13, S_W11, 0, S_H23, 1, S_END };
unsigned char aG3_6c[] = { S_TRA, 0, 0, 1, 0, S_H12, S_W12, S_FLV, S_TRA, 0, 0, 1, 0, S_H12, S_W12, S_BOX, 0, S_H12, S_W12,1, S_END };
unsigned char aG3_6d[] = { S_TRA, 0, 0, S_W12, S_W12, S_H12, 0, S_FLH, S_TRA, 0, 0, S_W12, S_W12, S_H12, 0, S_END };
unsigned char aG3_6e[] = { S_CHR, 0x00, 0x20};
unsigned char aG3_6f[] = { S_CHR, 0x00, 0x20};
unsigned char aG3_70[] = { S_INV, S_LNK, 0x30, S_END };
unsigned char aG3_71[] = { S_INV, S_LNK, 0x31, S_END };
unsigned char aG3_72[] = { S_INV, S_LNK, 0x32, S_END };
unsigned char aG3_73[] = { S_INV, S_LNK, 0x33, S_END };
unsigned char aG3_74[] = { S_INV, S_LNK, 0x34, S_END };
unsigned char aG3_75[] = { S_INV, S_LNK, 0x35, S_END };
unsigned char aG3_76[] = { S_LNK, 0x66, S_FLH, S_END };
unsigned char aG3_77[] = { S_LNK, 0x67, S_FLH, S_END };
unsigned char aG3_78[] = { S_LNK, 0x68, S_FLH, S_END };
unsigned char aG3_79[] = { S_LNK, 0x69, S_FLH, S_END };
unsigned char aG3_7a[] = { S_LNK, 0x6a, S_FLH, S_END };
unsigned char aG3_7b[] = { S_LNK, 0x6b, S_FLH, S_END };
unsigned char aG3_7c[] = { S_LNK, 0x6c, S_FLH, S_END };
unsigned char aG3_7d[] = { S_LNK, 0x6d, S_FLV, S_END };
unsigned char aG3_7e[] = { S_BOX, S_W12, 0, 2, S_H12, S_FLH, S_BOX, S_W12, 0, 2, S_H12, S_END };// help char, not printed directly (only by S_LNK)

unsigned char *aShapes[] =
{
	aG3_20, aG3_21, aG3_22, aG3_23, aG3_24, aG3_25, aG3_26, aG3_27, aG3_28, aG3_29, aG3_2a, aG3_2b, aG3_2c, aG3_2d, aG3_2e, aG3_2f,
	aG3_30, aG3_31, aG3_32, aG3_33, aG3_34, aG3_35, aG3_36, aG3_37, aG3_38, aG3_39, aG3_3a, aG3_3b, aG3_3c, aG3_3d, aG3_3e, aG3_3f,
	aG3_40, aG3_41, aG3_42, aG3_43, aG3_44, aG3_45, aG3_46, aG3_47, aG3_48, aG3_49, aG3_4a, aG3_4b, aG3_4c, aG3_4d, aG3_4e, aG3_4f,
	aG3_50, aG3_51, aG3_52, aG3_53, aG3_54, aG3_55, aG3_56, aG3_57, aG3_58, aG3_59, aG3_5a, aG3_5b, aG3_5c, aG3_5d, aG3_5e, aG3_5f,
	aG3_60, aG3_61, aG3_62, aG3_63, aG3_64, aG3_65, aG3_66, aG3_67, aG3_68, aG3_69, aG3_6a, aG3_6b, aG3_6c, aG3_6d, aG3_6e, aG3_6f,
	aG3_70, aG3_71, aG3_72, aG3_73, aG3_74, aG3_75, aG3_76, aG3_77, aG3_78, aG3_79, aG3_7a, aG3_7b, aG3_7c, aG3_7d, aG3_7e
};

tuxtxt_cache_struct tuxtxt_cache;
static pthread_mutex_t tuxtxt_cache_lock = PTHREAD_MUTEX_INITIALIZER;
int tuxtxt_get_zipsize(int p,int sp)
{
    tstCachedPage* pg = tuxtxt_cache.astCachetable[p][sp];
    if (!pg) return 0;
#if TUXTXT_COMPRESS == 1
	return pg->ziplen;
#elif TUXTXT_COMPRESS == 2
	pthread_mutex_lock(&tuxtxt_cache_lock);
	int zipsize = 0,i,j;
	for (i = 0; i < 23*5; i++)
		for (j = 0; j < 8; j++)
		zipsize += pg->bitmask[i]>>j & 0x01;

	zipsize+=23*5;//bitmask
	pthread_mutex_unlock(&tuxtxt_cache_lock);
	return zipsize;
#else
	return 23*40;
#endif
}
void tuxtxt_compress_page(int p, int sp, unsigned char* buffer)
{
	pthread_mutex_lock(&tuxtxt_cache_lock);
	tstCachedPage* pg = tuxtxt_cache.astCachetable[p][sp];
	if (!pg)
	{
		printf("tuxtxt: trying to compress a not allocated page!!\n");
		pthread_mutex_unlock(&tuxtxt_cache_lock);
		return;
	}

#if TUXTXT_COMPRESS == 1
	unsigned char pagecompressed[23*40];
	uLongf comprlen = 23*40;
	if (compress2(pagecompressed,&comprlen,buffer,23*40,Z_BEST_SPEED) == Z_OK)
	{
		if (pg->pData)
			pg->pData = realloc(pg->pData,comprlen); 
		else
			pg->pData = malloc(comprlen);
		pg->ziplen = 0;
		if (pg->pData)
		{
			pg->ziplen = comprlen;
			memcpy(pg->pData,pagecompressed,comprlen);
		}
	}
#elif TUXTXT_COMPRESS == 2
	int i,j=0;
	unsigned char cbuf[23*40];
	memset(pg->bitmask,0,sizeof(pg->bitmask));
	for (i = 0; i < 23*40; i++)
	{
		if (i && buffer[i] == buffer[i-1])
		    continue;
		pg->bitmask[i>>3] |= 0x80>>(i&0x07);
		cbuf[j++]=buffer[i];
	}
	if (pg->pData)
		pg->pData = realloc(pg->pData,j); 
	else
		pg->pData = malloc(j);
	if (pg->pData)
	{
		memcpy(pg->pData,cbuf,j);
	}
	else
		memset(pg->bitmask,0,sizeof(pg->bitmask));

#else
	memcpy(pg->data,buffer,23*40);
#endif
	pthread_mutex_unlock(&tuxtxt_cache_lock);

}
void tuxtxt_decompress_page(int p, int sp, unsigned char* buffer)
{
	pthread_mutex_lock(&tuxtxt_cache_lock);
    tstCachedPage* pg = tuxtxt_cache.astCachetable[p][sp];
	memset(buffer,' ',23*40);
    if (!pg)
    {
		printf("tuxtxt: trying to decompress a not allocated page!!\n");
		pthread_mutex_unlock(&tuxtxt_cache_lock);
		return;
    }
	if (pg->pData)
	{
#if TUXTXT_COMPRESS == 1
		if (pg->ziplen)
		{
			uLongf comprlen = 23*40;
			uncompress(buffer,&comprlen,pg->pData,pg->ziplen);
		}

#elif TUXTXT_COMPRESS == 2
		int i,j=0;
		char c=0x20;
		for (i = 0; i < 23*40; i++)
		{
		    if (pg->bitmask[i>>3] & 0x80>>(i&0x07))
				c = pg->pData[j++];
		    buffer[i] = c;
		}
#else
		memcpy(buffer,pg->data,23*40);
#endif
	}
	pthread_mutex_unlock(&tuxtxt_cache_lock);
}
void tuxtxt_next_dec(int *i) /* skip to next decimal */
{
	(*i)++;

	if ((*i & 0x0F) > 0x09)
		*i += 0x06;

	if ((*i & 0xF0) > 0x90)
		*i += 0x60;

	if (*i > 0x899)
		*i = 0x100;
}

void tuxtxt_prev_dec(int *i)           /* counting down */
{
	(*i)--;

	if ((*i & 0x0F) > 0x09)
		*i -= 0x06;

	if ((*i & 0xF0) > 0x90)
		*i -= 0x60;

	if (*i < 0x100)
		*i = 0x899;
}

int tuxtxt_is_dec(int i)
{
	return ((i & 0x00F) <= 9) && ((i & 0x0F0) <= 0x90);
}

int tuxtxt_next_hex(int i) /* return next existing non-decimal page number */
{
	int startpage = i;
	if (startpage < 0x100)
		startpage = 0x100;

	do
	{
		i++;
		if (i > 0x8FF)
			i = 0x100;
		if (i == startpage)
			break;
	}  while ((tuxtxt_cache.subpagetable[i] == 0xFF) || tuxtxt_is_dec(i));
	return i;
}
#define number2char(c) ((c) + (((c) <= 9) ? '0' : ('A' - 10)))
/* print hex-number into string, s points to last digit, caller has to provide enough space, no termination */
void tuxtxt_hex2str(char *s, unsigned int n)
{
	do {
		char c = (n & 0xF);
		*s-- = number2char(c);
		n >>= 4;
	} while (n);
}
/*
 * TOP-Text
 * Info entnommen aus videotext-0.6.19991029,
 * Copyright (c) 1994-96 Martin Buck  <martin-2.buck@student.uni-ulm.de>
 */
void tuxtxt_decode_btt()
{
	/* basic top table */
	int i, current, b1, b2, b3, b4;
	unsigned char btt[23*40];

	if (tuxtxt_cache.subpagetable[0x1f0] == 0xff || 0 == tuxtxt_cache.astCachetable[0x1f0][tuxtxt_cache.subpagetable[0x1f0]]) /* not yet received */
		return;
	tuxtxt_decompress_page(0x1f0,tuxtxt_cache.subpagetable[0x1f0],btt);
	if (btt[799] == ' ') /* not completely received or error */
		return;

	current = 0x100;
	for (i = 0; i < 800; i++)
	{
		b1 = btt[i];
		if (b1 == ' ')
			b1 = 0;
		else
		{
			b1 = dehamming[b1];
			if (b1 == 0xFF) /* hamming error in btt */
			{
				btt[799] = ' '; /* mark btt as not received */
				return;
			}
		}
		tuxtxt_cache.basictop[current] = b1;
		tuxtxt_next_dec(&current);
	}
	/* page linking table */
	tuxtxt_cache.maxadippg = -1; /* rebuild table of adip pages */
	for (i = 0; i < 10; i++)
	{
		b1 = dehamming[btt[800 + 8*i +0]];

		if (b1 == 0xE)
			continue; /* unused */
		else if (b1 == 0xF)
			break; /* end */

		b4 = dehamming[btt[800 + 8*i +7]];

		if (b4 != 2) /* only adip, ignore multipage (1) */
			continue;

		b2 = dehamming[btt[800 + 8*i +1]];
		b3 = dehamming[btt[800 + 8*i +2]];

		if (b1 == 0xFF || b2 == 0xFF || b3 == 0xFF)
		{
			printf("TuxTxt <Biterror in btt/plt index %d>\n", i);
			btt[799] = ' '; /* mark btt as not received */
			return;
		}

		b1 = b1<<8 | b2<<4 | b3; /* page number */
		tuxtxt_cache.adippg[++tuxtxt_cache.maxadippg] = b1;
	}
#if DEBUG
	printf("TuxTxt <BTT decoded>\n");
#endif
	tuxtxt_cache.bttok = 1;
}

void tuxtxt_decode_adip() /* additional information table */
{
	int i, p, j, b1, b2, b3, charfound;
	unsigned char padip[23*40];

	for (i = 0; i <= tuxtxt_cache.maxadippg; i++)
	{
		p = tuxtxt_cache.adippg[i];
		if (!p || tuxtxt_cache.subpagetable[p] == 0xff || 0 == tuxtxt_cache.astCachetable[p][tuxtxt_cache.subpagetable[p]]) /* not cached (avoid segfault) */
			continue;

		tuxtxt_decompress_page(p,tuxtxt_cache.subpagetable[p],padip);
		for (j = 0; j < 44; j++)
		{
			b1 = dehamming[padip[20*j+0]];
			if (b1 == 0xE)
				continue; /* unused */

			if (b1 == 0xF)
				break; /* end */

			b2 = dehamming[padip[20*j+1]];
			b3 = dehamming[padip[20*j+2]];

			if (b1 == 0xFF || b2 == 0xFF || b3 == 0xFF)
			{
				printf("TuxTxt <Biterror in ait %03x %d %02x %02x %02x %02x %02x %02x>\n", p, j,
						 padip[20*j+0],
						 padip[20*j+1],
						 padip[20*j+2],
						 b1, b2, b3
						 );
				return;
			}

			if (b1>8 || b2>9 || b3>9) /* ignore extries with invalid or hex page numbers */
			{
				continue;
			}

			b1 = b1<<8 | b2<<4 | b3; /* page number */
			charfound = 0; /* flag: no printable char found */

			for (b2 = 11; b2 >= 0; b2--)
			{
				b3 = deparity[padip[20*j + 8 + b2]];
				if (b3 < ' ')
					b3 = ' ';

				if (b3 == ' ' && !charfound)
					tuxtxt_cache.adip[b1][b2] = '\0';
				else
				{
					tuxtxt_cache.adip[b1][b2] = b3;
					charfound = 1;
				}
			}
		} /* next link j */
		tuxtxt_cache.adippg[i] = 0; /* completely decoded: clear entry */
#if DEBUG
		printf("TuxTxt <ADIP %03x decoded>\n", p);
#endif
	} /* next adip page i */

	while (!tuxtxt_cache.adippg[tuxtxt_cache.maxadippg] && (tuxtxt_cache.maxadippg >= 0)) /* and shrink table */
		tuxtxt_cache.maxadippg--;
}
/******************************************************************************
 * GetSubPage                                                                 *
 ******************************************************************************/
int tuxtxt_GetSubPage(int page, int subpage, int offset)
{
	int loop;


	for (loop = subpage + offset; loop != subpage; loop += offset)
	{
		if (loop < 0)
			loop = 0x79;
		else if (loop > 0x79)
			loop = 0;
		if (loop == subpage)
			break;

		if (tuxtxt_cache.astCachetable[page][loop])
		{
#if DEBUG
			printf("TuxTxt <NextSubPage: %.3X-%.2X>\n", page, subpage);
#endif
			return loop;
		}
	}

#if DEBUG
	printf("TuxTxt <NextSubPage: no other SubPage>\n");
#endif
	return subpage;
}

/******************************************************************************
 * clear_cache                                                                *
 ******************************************************************************/

void tuxtxt_clear_cache()
{
	pthread_mutex_lock(&tuxtxt_cache_lock);
	int clear_page, clear_subpage, d26;
	tuxtxt_cache.maxadippg  = -1;
	tuxtxt_cache.bttok      = 0;
	tuxtxt_cache.cached_pages  = 0;
	tuxtxt_cache.page_receiving = -1;
	tuxtxt_cache.vtxtpid = -1;
	memset(&tuxtxt_cache.subpagetable, 0xFF, sizeof(tuxtxt_cache.subpagetable));
	memset(&tuxtxt_cache.basictop, 0, sizeof(tuxtxt_cache.basictop));
	memset(&tuxtxt_cache.adip, 0, sizeof(tuxtxt_cache.adip));
	memset(&tuxtxt_cache.flofpages, 0 , sizeof(tuxtxt_cache.flofpages));
	memset(&tuxtxt_cache.timestring, 0x20, 8);
	memset(&tuxtxt_cache.subtitlepages, 0, sizeof(tuxtxt_cache.subtitlepages));
 	unsigned char magazine;
	for (magazine = 1; magazine < 9; magazine++)
	{
		tuxtxt_cache.current_page  [magazine] = -1;
		tuxtxt_cache.current_subpage [magazine] = -1;
	}

	for (clear_page = 0; clear_page < 0x900; clear_page++)
		for (clear_subpage = 0; clear_subpage < 0x80; clear_subpage++)
			if (tuxtxt_cache.astCachetable[clear_page][clear_subpage])
			{
				tstPageinfo *p = &(tuxtxt_cache.astCachetable[clear_page][clear_subpage]->pageinfo);
				if (p->p24)
					free(p->p24);
				if (p->ext)
				{
					if (p->ext->p27)
						free(p->ext->p27);
					for (d26=0; d26 < 16; d26++)
						if (p->ext->p26[d26])
							free(p->ext->p26[d26]);
					free(p->ext);
				}
#if TUXTXT_COMPRESS >0
				if (tuxtxt_cache.astCachetable[clear_page][clear_subpage]->pData)
					free(tuxtxt_cache.astCachetable[clear_page][clear_subpage]->pData);
#endif
				free(tuxtxt_cache.astCachetable[clear_page][clear_subpage]);
				tuxtxt_cache.astCachetable[clear_page][clear_subpage] = 0;
			}
	for (clear_page = 0; clear_page < 9; clear_page++)
	{
		if (tuxtxt_cache.astP29[clear_page])
		{
		    if (tuxtxt_cache.astP29[clear_page]->p27)
			free(tuxtxt_cache.astP29[clear_page]->p27);
		    for (d26=0; d26 < 16; d26++)
			if (tuxtxt_cache.astP29[clear_page]->p26[d26])
			    free(tuxtxt_cache.astP29[clear_page]->p26[d26]);
		    free(tuxtxt_cache.astP29[clear_page]);
		    tuxtxt_cache.astP29[clear_page] = 0;
		}
		tuxtxt_cache.current_page  [clear_page] = -1;
		tuxtxt_cache.current_subpage [clear_page] = -1;
	}
	memset(&tuxtxt_cache.astCachetable, 0, sizeof(tuxtxt_cache.astCachetable));
	memset(&tuxtxt_cache.astP29, 0, sizeof(tuxtxt_cache.astP29));
#if DEBUG
	printf("TuxTxt cache cleared\n");
#endif
	pthread_mutex_unlock(&tuxtxt_cache_lock);
}
/******************************************************************************
 * init_demuxer                                                               *
 ******************************************************************************/

int tuxtxt_init_demuxer()
{
	/* open demuxer */
	if ((tuxtxt_cache.dmx = open(DMX, O_RDWR)) == -1)
	{
		perror("TuxTxt <open DMX>");
		return 0;
	}


	if (ioctl(tuxtxt_cache.dmx, DMX_SET_BUFFER_SIZE, 64*1024) < 0)
	{
		perror("TuxTxt <DMX_SET_BUFFERSIZE>");
		return 0;
	}
#if DEBUG
	printf("TuxTxt: initialized\n");
#endif
	/* init successfull */

	return 1;
}
/******************************************************************************
 * CacheThread support functions                                              *
 ******************************************************************************/

void tuxtxt_decode_p2829(unsigned char *vtxt_row, tstExtData **ptExtData)
{
	int bitsleft, colorindex;
	unsigned char *p;
	int t1 = deh24(&vtxt_row[7-4]);
	int t2 = deh24(&vtxt_row[10-4]);

	if (t1 < 0 || t2 < 0)
	{
#if DEBUG
		printf("TuxTxt <Biterror in p28>\n");
#endif
		return;
	}

	if (!(*ptExtData))
		(*ptExtData) = calloc(1, sizeof(tstExtData));
	if (!(*ptExtData))
		return;

	(*ptExtData)->p28Received = 1;
	(*ptExtData)->DefaultCharset = (t1>>7) & 0x7f;
	(*ptExtData)->SecondCharset = ((t1>>14) & 0x0f) | ((t2<<4) & 0x70);
	(*ptExtData)->LSP = !!(t2 & 0x08);
	(*ptExtData)->RSP = !!(t2 & 0x10);
	(*ptExtData)->SPL25 = !!(t2 & 0x20);
	(*ptExtData)->LSPColumns = (t2>>6) & 0x0f;

	bitsleft = 8; /* # of bits not evaluated in val */
	t2 >>= 10; /* current data */
	p = &vtxt_row[13-4];	/* pointer to next data triplet */
	for (colorindex = 0; colorindex < 16; colorindex++)
	{
		if (bitsleft < 12)
		{
			t2 |= deh24(p) << bitsleft;
			if (t2 < 0)	/* hamming error */
				break;
			p += 3;
			bitsleft += 18;
		}
		(*ptExtData)->bgr[colorindex] = t2 & 0x0fff;
		bitsleft -= 12;
		t2 >>= 12;
	}
	if (t2 < 0 || bitsleft != 14)
	{
#if DEBUG
		printf("TuxTxt <Biterror in p28/29 t2=%d b=%d>\n", t2, bitsleft);
#endif
		(*ptExtData)->p28Received = 0;
		return;
	}
	(*ptExtData)->DefScreenColor = t2 & 0x1f;
	t2 >>= 5;
	(*ptExtData)->DefRowColor = t2 & 0x1f;
	(*ptExtData)->BlackBgSubst = !!(t2 & 0x20);
	t2 >>= 6;
	(*ptExtData)->ColorTableRemapping = t2 & 0x07;
}

void tuxtxt_erase_page(int magazine)
{
	pthread_mutex_lock(&tuxtxt_cache_lock);
    tstCachedPage* pg = tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]];
	if (pg)
	{
		memset(&(pg->pageinfo), 0, sizeof(tstPageinfo));	/* struct pageinfo */
		memset(pg->p0, ' ', 24);
#if TUXTXT_COMPRESS == 1
    	if (pg->pData) {free(pg->pData); pg->pData = NULL;}
#elif TUXTXT_COMPRESS == 2
		memset(pg->bitmask, 0, 23*5);
#else
		memset(pg->data, ' ', 23*40);
#endif
	}
	pthread_mutex_unlock(&tuxtxt_cache_lock);
}

void tuxtxt_allocate_cache(int magazine)
{
	/* check cachetable and allocate memory if needed */
	if (tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]] == 0)
	{

		tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]] = malloc(sizeof(tstCachedPage));
		if (tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]] )
		{
#if TUXTXT_COMPRESS >0
			tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]]->pData = 0;
#endif
			tuxtxt_erase_page(magazine);
			tuxtxt_cache.cached_pages++;
		}
	}
}
/******************************************************************************
 * CacheThread                                                                *
 ******************************************************************************/

void *tuxtxt_CacheThread(void *arg)
{
	const unsigned char rev_lut[32] = {
		0x00,0x08,0x04,0x0c, /*  upper nibble */
		0x02,0x0a,0x06,0x0e,
		0x01,0x09,0x05,0x0d,
		0x03,0x0b,0x07,0x0f,
		0x00,0x80,0x40,0xc0, /*  lower nibble */
		0x20,0xa0,0x60,0xe0,
		0x10,0x90,0x50,0xd0,
		0x30,0xb0,0x70,0xf0 };
	unsigned char pes_packet[184];
	unsigned char vtxt_row[42];
	int line, byte/*, bit*/;
	int b1, b2, b3, b4;
	int packet_number;
	int doupdate=0;
	unsigned char magazine = 0xff;
	unsigned char pagedata[9][23*40];
	tstPageinfo *pageinfo_thread;

	printf("TuxTxt running thread...(%03x)\n",tuxtxt_cache.vtxtpid);
	tuxtxt_cache.receiving = 1;
	nice(3);
	while (1)
	{
		/* check stopsignal */
		pthread_testcancel();

		if (!tuxtxt_cache.receiving) continue;

		/* read packet */
		ssize_t readcnt;
		readcnt = read(tuxtxt_cache.dmx, &pes_packet, sizeof(pes_packet));

		if (readcnt != sizeof(pes_packet))
		{
#if DEBUG
			printf ("TuxTxt: readerror\n");
#endif
			continue;
		}

		/* analyze it */
		for (line = 0; line < 4; line++)
		{
			unsigned char *vtx_rowbyte = &pes_packet[line*0x2e];
			if ((vtx_rowbyte[0] == 0x02 || vtx_rowbyte[0] == 0x03) && (vtx_rowbyte[1] == 0x2C))
			{
				/* clear rowbuffer */
				/* convert row from lsb to msb (begin with magazin number) */
				for (byte = 4; byte < 46; byte++)
				{
					unsigned char upper,lower;
					upper = (vtx_rowbyte[byte] >> 4) & 0xf;
					lower = vtx_rowbyte[byte] & 0xf;
					vtxt_row[byte-4] = (rev_lut[upper]) | (rev_lut[lower+16]);
				}

				/* get packet number */
				b1 = dehamming[vtxt_row[0]];
				b2 = dehamming[vtxt_row[1]];

				if (b1 == 0xFF || b2 == 0xFF)
				{
#if DEBUG
					printf("TuxTxt <Biterror in Packet>\n");
#endif
					continue;
				}

				b1 &= 8;

				packet_number = b1>>3 | b2<<1;

				/* get magazine number */
				magazine = dehamming[vtxt_row[0]] & 7;
				if (!magazine) magazine = 8;

				if (packet_number == 0 && tuxtxt_cache.current_page[magazine] != -1 && tuxtxt_cache.current_subpage[magazine] != -1)
 				    tuxtxt_compress_page(tuxtxt_cache.current_page[magazine],tuxtxt_cache.current_subpage[magazine],pagedata[magazine]);

//printf("receiving packet %d %03x/%02x\n",packet_number, tuxtxt_cache.current_page[magazine],tuxtxt_cache.current_subpage[magazine]);

				/* analyze row */
				if (packet_number == 0)
				{
    					/* get pagenumber */
					b2 = dehamming[vtxt_row[3]];
					b3 = dehamming[vtxt_row[2]];

					if (b2 == 0xFF || b3 == 0xFF)
					{
						tuxtxt_cache.current_page[magazine] = tuxtxt_cache.page_receiving = -1;
#if DEBUG
						printf("TuxTxt <Biterror in Page>\n");
#endif
						continue;
					}

					tuxtxt_cache.current_page[magazine] = tuxtxt_cache.page_receiving = magazine<<8 | b2<<4 | b3;

					if (b2 == 0x0f && b3 == 0x0f)
					{
						tuxtxt_cache.current_subpage[magazine] = -1; /* ?ff: ignore data transmissions */
						continue;
					}

					/* get subpagenumber */
					b1 = dehamming[vtxt_row[7]];
					b2 = dehamming[vtxt_row[6]];
					b3 = dehamming[vtxt_row[5]];
					b4 = dehamming[vtxt_row[4]];

					if (b1 == 0xFF || b2 == 0xFF || b3 == 0xFF || b4 == 0xFF)
					{
#if DEBUG
						printf("TuxTxt <Biterror in SubPage>\n");
#endif
						tuxtxt_cache.current_subpage[magazine] = -1;
						continue;
					}

					b1 &= 3;
					b3 &= 7;

					if (tuxtxt_is_dec(tuxtxt_cache.page_receiving)) /* ignore other subpage bits for hex pages */
					{
#if 0	/* ? */
						if (b1 != 0 || b2 != 0)
						{
#if DEBUG
							printf("TuxTxt <invalid subpage data p%03x %02x %02x %02x %02x>\n", tuxtxt_cache.page_receiving, b1, b2, b3, b4);
#endif
							tuxtxt_cache.current_subpage[magazine] = -1;
							continue;
						}
						else
#endif
							tuxtxt_cache.current_subpage[magazine] = b3<<4 | b4;
					}
					else
						tuxtxt_cache.current_subpage[magazine] = b4; /* max 16 subpages for hex pages */

					/* store current subpage for this page */
					tuxtxt_cache.subpagetable[tuxtxt_cache.current_page[magazine]] = tuxtxt_cache.current_subpage[magazine];

					tuxtxt_allocate_cache(magazine);
					tuxtxt_decompress_page(tuxtxt_cache.current_page[magazine],tuxtxt_cache.current_subpage[magazine],pagedata[magazine]);
					pageinfo_thread = &(tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]]->pageinfo);

					if ((tuxtxt_cache.page_receiving & 0xff) == 0xfe) /* ?fe: magazine organization table (MOT) */
						pageinfo_thread->function = FUNC_MOT;

					/* check controlbits */
					if (dehamming[vtxt_row[5]] & 8)   /* C4 -> erase page */
					{
#if TUXTXT_COMPRESS == 1
						tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]]->ziplen = 0;
#elif TUXTXT_COMPRESS == 2
						memset(tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]]->bitmask, 0, 23*5);
#else
						memset(tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]]->data, ' ', 23*40);
#endif
						memset(pagedata[magazine],' ', 23*40);
					}
					if (dehamming[vtxt_row[9]] & 8)   /* C8 -> update page */
						doupdate = tuxtxt_cache.page_receiving;

					pageinfo_thread->boxed = !!(dehamming[vtxt_row[7]] & 0x0c);

					/* get country control bits */
					b1 = dehamming[vtxt_row[9]];
					if (b1 == 0xFF)
					{
#if DEBUG
						printf("TuxTxt <Biterror in CountryFlags>\n");
#endif
					}
					else
					{
						pageinfo_thread->nationalvalid = 1;
						pageinfo_thread->national = rev_lut[b1] & 0x07;
					}

					if (dehamming[vtxt_row[7]] & 0x08)// subtitle page
					{
						int i = 0, found = -1, use = -1;
						for (; i < 8; i++)
						{
							if (use == -1 && !tuxtxt_cache.subtitlepages[i].page)
								use = i;
							else if (tuxtxt_cache.subtitlepages[i].page == tuxtxt_cache.page_receiving)
							{
								found = i;
								use = i;
								break;
							}
						}
						if (found == -1 && use != -1)
							tuxtxt_cache.subtitlepages[use].page = tuxtxt_cache.page_receiving;
						if (use != -1)
							tuxtxt_cache.subtitlepages[use].language = countryconversiontable[pageinfo_thread->national];
					}
					/* check parity, copy line 0 to cache (start and end 8 bytes are not needed and used otherwise) */
					unsigned char *p = tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]]->p0;
					for (byte = 10; byte < 42-8; byte++)
						*p++ = deparity[vtxt_row[byte]];

					if (!tuxtxt_is_dec(tuxtxt_cache.page_receiving))
						continue; /* valid hex page number: just copy headline, ignore timestring */

					/* copy timestring */
					p = tuxtxt_cache.timestring;
					for (; byte < 42; byte++)
						*p++ = deparity[vtxt_row[byte]];
				} /* (packet_number == 0) */
				else if (packet_number == 29 && dehamming[vtxt_row[2]]== 0) /* packet 29/0 replaces 28/0 for a whole magazine */
				{
					tuxtxt_decode_p2829(vtxt_row, &(tuxtxt_cache.astP29[magazine]));
				}
				else if (tuxtxt_cache.current_page[magazine] != -1 && tuxtxt_cache.current_subpage[magazine] != -1)
					/* packet>0, 0 has been correctly received, buffer allocated */
				{
					pageinfo_thread = &(tuxtxt_cache.astCachetable[tuxtxt_cache.current_page[magazine]][tuxtxt_cache.current_subpage[magazine]]->pageinfo);
					/* pointer to current info struct */

					if (packet_number <= 25)
					{
						unsigned char *p = NULL;
						if (packet_number < 24)
							p = pagedata[magazine] + 40*(packet_number-1);
						else
						{
							if (!(pageinfo_thread->p24))
								pageinfo_thread->p24 = calloc(2, 40);
							if (pageinfo_thread->p24)
								p = pageinfo_thread->p24 + (packet_number - 24) * 40;
						}
						if (p)
						{
							if (tuxtxt_is_dec(tuxtxt_cache.current_page[magazine]))
								for (byte = 2; byte < 42; byte++)
									*p++ = vtxt_row[byte] & 0x7f; /* allow values with parity errors as some channels don't care :( */
							else if ((tuxtxt_cache.current_page[magazine] & 0xff) == 0xfe)
								for (byte = 2; byte < 42; byte++)
									*p++ = dehamming[vtxt_row[byte]]; /* decode hamming 8/4 */
							else /* other hex page: no parity check, just copy */
								memcpy(p, &vtxt_row[2], 40);
						}
					}
					else if (packet_number == 27)
					{
						int descode = dehamming[vtxt_row[2]]; /* designation code (0..15) */

						if (descode == 0xff)
						{
#if DEBUG
							printf("TuxTxt <Biterror in p27>\n");
#endif
							continue;
						}
						if (descode == 0) // reading FLOF-Pagelinks
						{
							b1 = dehamming[vtxt_row[0]];
							if (b1 != 0xff)
							{
								b1 &= 7;

								for (byte = 0; byte < FLOFSIZE; byte++)
								{
									b2 = dehamming[vtxt_row[4+byte*6]];
									b3 = dehamming[vtxt_row[3+byte*6]];

									if (b2 != 0xff && b3 != 0xff)
									{
										b4 = ((b1 ^ (dehamming[vtxt_row[8+byte*6]]>>1)) & 6) |
											((b1 ^ (dehamming[vtxt_row[6+byte*6]]>>3)) & 1);
										if (b4 == 0)
											b4 = 8;
										if (b2 <= 9 && b3 <= 9)
											tuxtxt_cache.flofpages[tuxtxt_cache.current_page[magazine] ][byte] = b4<<8 | b2<<4 | b3;
									}
								}

								/* copy last 2 links to adip for TOP-Index */
								if (pageinfo_thread->p24) /* packet 24 received */
								{
									int a, a1, e=39, l=3;
									char *p = pageinfo_thread->p24;
									do
									{
										for (;
											  l >= 2 && 0 == tuxtxt_cache.flofpages[tuxtxt_cache.current_page[magazine]][l];
											  l--)
											; /* find used linkindex */
										for (;
											  e >= 1 && !isalnum(p[e]);
											  e--)
											; /* find end */
										for (a = a1 = e - 1;
											  a >= 0 && p[a] >= ' ';
											  a--) /* find start */
											if (p[a] > ' ')
											a1 = a; /* first non-space */
										if (a >= 0 && l >= 2)
										{
											strncpy(tuxtxt_cache.adip[tuxtxt_cache.flofpages[tuxtxt_cache.current_page[magazine]][l]],
													  &p[a1],
													  12);
											if (e-a1 < 11)
												tuxtxt_cache.adip[tuxtxt_cache.flofpages[tuxtxt_cache.current_page[magazine]][l]][e-a1+1] = '\0';
#if 0 //DEBUG
											printf(" %03x/%02x %d %d %d %d %03x %s\n",
													 tuxtxt_cache.current_page[magazine], tuxtxt_cache.current_subpage[magazine],
													 l, a, a1, e,
													 tuxtxt_cache.flofpages[tuxtxt_cache.current_page[magazine]][l],
													 tuxtxt_cache.adip[tuxtxt_cache.flofpages[tuxtxt_cache.current_page[magazine]][l]]
													 );
#endif
										}
										e = a - 1;
										l--;
									} while (l >= 2);
								}
							}
						}
						else if (descode == 4)	/* level 2.5 links (ignore level 3.5 links of /4 and /5) */
						{
							int i;
							tstp27 *p;

							if (!pageinfo_thread->ext)
								pageinfo_thread->ext = calloc(1, sizeof(tstExtData));
							if (!pageinfo_thread->ext)
								continue;
							if (!(pageinfo_thread->ext->p27))
								pageinfo_thread->ext->p27 = calloc(4, sizeof(tstp27));
							if (!(pageinfo_thread->ext->p27))
								continue;
							p = pageinfo_thread->ext->p27;
							for (i = 0; i < 4; i++)
							{
								int d1 = deh24(&vtxt_row[6*i + 3]);
								int d2 = deh24(&vtxt_row[6*i + 6]);
								if (d1 < 0 || d2 < 0)
								{
#if DEBUG
									printf("TuxTxt <Biterror in p27/4-5>\n");
#endif
									continue;
								}
								p->local = i & 0x01;
								p->drcs = !!(i & 0x02);
								p->l25 = !!(d1 & 0x04);
								p->l35 = !!(d1 & 0x08);
								p->page =
									(((d1 & 0x000003c0) >> 6) |
									 ((d1 & 0x0003c000) >> (14-4)) |
									 ((d1 & 0x00003800) >> (11-8))) ^
									(dehamming[vtxt_row[0]] << 8);
								if (p->page < 0x100)
									p->page += 0x800;
								p->subpage = d2 >> 2;
								if ((p->page & 0xff) == 0xff)
									p->page = 0;
								else if (p->page > 0x899)
								{
									// workaround for crash on RTL Shop ...
									// sorry.. i dont understand whats going wrong here :)
									printf("[TuxTxt] page > 0x899 ... ignore!!!!!!\n");
									continue;
								}
								else if (tuxtxt_cache.astCachetable[p->page][0])	/* link valid && linked page cached */
								{
									tstPageinfo *pageinfo_link = &(tuxtxt_cache.astCachetable[p->page][0]->pageinfo);
									if (p->local)
										pageinfo_link->function = p->drcs ? FUNC_DRCS : FUNC_POP;
									else
										pageinfo_link->function = p->drcs ? FUNC_GDRCS : FUNC_GPOP;
								}
								p++; /*  */
							}
						}
					}

					else if (packet_number == 26)
					{
						int descode = dehamming[vtxt_row[2]]; /* designation code (0..15) */

						if (descode == 0xff)
						{
#if DEBUG
							printf("TuxTxt <Biterror in p26>\n");
#endif
							continue;
						}
						if (!pageinfo_thread->ext)
							pageinfo_thread->ext = calloc(1, sizeof(tstExtData));
						if (!pageinfo_thread->ext)
							continue;
						if (!(pageinfo_thread->ext->p26[descode]))
							pageinfo_thread->ext->p26[descode] = malloc(13 * 3);
						if (pageinfo_thread->ext->p26[descode])
							memcpy(pageinfo_thread->ext->p26[descode], &vtxt_row[3], 13 * 3);
#if 0//DEBUG
						int i, t, m;

						printf("P%03x/%02x %02d/%x",
								 tuxtxt_cache.current_page[magazine], tuxtxt_cache.current_subpage[magazine],
								 packet_number, dehamming[vtxt_row[2]]);
						for (i=7-4; i <= 45-4; i+=3) /* dump all triplets */
						{
							t = deh24(&vtxt_row[i]); /* mode/adr/data */
							m = (t>>6) & 0x1f;
							printf(" M%02xA%02xD%03x", m, t & 0x3f, (t>>11) & 0x7f);
							if (m == 0x1f)	/* terminator */
								break;
						}
						putchar('\n');
#endif
					}
					else if (packet_number == 28)
					{
						int descode = dehamming[vtxt_row[2]]; /* designation code (0..15) */

						if (descode == 0xff)
						{
#if DEBUG
							printf("TuxTxt <Biterror in p28>\n");
#endif
							continue;
						}
						if (descode != 2)
						{
							int t1 = deh24(&vtxt_row[7-4]);
							pageinfo_thread->function = t1 & 0x0f;
							if (!pageinfo_thread->nationalvalid)
							{
								pageinfo_thread->nationalvalid = 1;
								pageinfo_thread->national = (t1>>4) & 0x07;
							}
						}

						switch (descode) /* designation code */
						{
						case 0: /* basic level 1 page */
						{
							tuxtxt_decode_p2829(vtxt_row, &(pageinfo_thread->ext));
							break;
						}
						case 1: /* G0/G1 designation for older decoders, level 3.5: DCLUT4/16, colors for multicolored bitmaps */
						{
							break; /* ignore */
						}
						case 2: /* page key */
						{
							break; /* ignore */
						}
						case 3: /* types of PTUs in DRCS */
						{
							break; /* TODO */
						}
						case 4: /* CLUTs 0/1, only level 3.5 */
						{
							break; /* ignore */
						}
						default:
						{
							break; /* invalid, ignore */
						}
						} /* switch designation code */
					}
					else if (packet_number == 30)
					{
#if 0//DEBUG
						int i;

						printf("p%03x/%02x %02d/%x ",
								 tuxtxt_cache.current_page[magazine], tuxtxt_cache.current_subpage[magazine],
								 packet_number, dehamming[vtxt_row[2]]);
						for (i=26-4; i <= 45-4; i++) /* station ID */
							putchar(deparity[vtxt_row[i]]);
						putchar('\n');
#endif
					}
				}
				/* set update flag */
				if (tuxtxt_cache.current_page[magazine] == tuxtxt_cache.page && tuxtxt_cache.current_subpage[magazine] != -1)
				{
 				    tuxtxt_compress_page(tuxtxt_cache.current_page[magazine],tuxtxt_cache.current_subpage[magazine],pagedata[magazine]);
					tuxtxt_cache.pageupdate = 1+(doupdate == tuxtxt_cache.page ? 1: 0);
					doupdate=0;
					if (!tuxtxt_cache.zap_subpage_manual)
						tuxtxt_cache.subpage = tuxtxt_cache.current_subpage[magazine];
				}
			}
		}
	}
	return 0;
}
/******************************************************************************
 * start_thread                                                               *
 ******************************************************************************/
int tuxtxt_start_thread()
{
	if (tuxtxt_cache.vtxtpid == -1) return 0;


	tuxtxt_cache.thread_starting = 1;
	struct dmx_pes_filter_params dmx_flt;

	/* set filter & start demuxer */
	dmx_flt.pid      = tuxtxt_cache.vtxtpid;
	dmx_flt.input    = DMX_IN_FRONTEND;
	dmx_flt.output   = DMX_OUT_TAP;
	dmx_flt.pes_type = DMX_PES_OTHER;
	dmx_flt.flags    = DMX_IMMEDIATE_START;

	if (tuxtxt_cache.dmx == -1) tuxtxt_init_demuxer();

	if (ioctl(tuxtxt_cache.dmx, DMX_SET_PES_FILTER, &dmx_flt) == -1)
	{
		perror("TuxTxt <DMX_SET_PES_FILTER>");
		tuxtxt_cache.thread_starting = 0;
		return 0;
	}

	/* create decode-thread */
	if (pthread_create(&tuxtxt_cache.thread_id, NULL, tuxtxt_CacheThread, NULL) != 0)
	{
		perror("TuxTxt <pthread_create>");
		tuxtxt_cache.thread_starting = 0;
		tuxtxt_cache.thread_id = 0;
		return 0;
	}
#if 1//DEBUG
	printf("TuxTxt service started %x\n", tuxtxt_cache.vtxtpid);
#endif
	tuxtxt_cache.receiving = 1;
	tuxtxt_cache.thread_starting = 0;
	return 1;
}
/******************************************************************************
 * stop_thread                                                                *
 ******************************************************************************/

int tuxtxt_stop_thread()
{

	/* stop decode-thread */
	if (tuxtxt_cache.thread_id != 0)
	{
		if (pthread_cancel(tuxtxt_cache.thread_id) != 0)
		{
			perror("TuxTxt <pthread_cancel>");
			return 0;
		}

		if (pthread_join(tuxtxt_cache.thread_id, &tuxtxt_cache.thread_result) != 0)
		{
			perror("TuxTxt <pthread_join>");
			return 0;
		}
		tuxtxt_cache.thread_id = 0;
	}
	if (tuxtxt_cache.dmx != -1)
	{
		ioctl(tuxtxt_cache.dmx, DMX_STOP);
//        close(tuxtxt_cache.dmx);
  	}
//	tuxtxt_cache.dmx = -1;
#if 1//DEBUG
	printf("TuxTxt stopped service %x\n", tuxtxt_cache.vtxtpid);
#endif
	return 1;
}

/******************************************************************************
 * decode Level2.5                                                            *
 ******************************************************************************/
int tuxtxt_eval_triplet(int iOData, tstCachedPage *pstCachedPage,
					  unsigned char *pAPx, unsigned char *pAPy,
					  unsigned char *pAPx0, unsigned char *pAPy0,
					  unsigned char *drcssubp, unsigned char *gdrcssubp,
					  signed char *endcol, tstPageAttr *attrPassive, unsigned char* pagedata, unsigned char* page_char, tstPageAttr* page_atrb);

/* get object data */
/* in: absolute triplet number (0..506, start at packet 3 byte 1) */
/* in: pointer to cache struct of page data */
/* out: 18 bit triplet data, <0 if invalid number, not cached, or hamming error */
int tuxtxt_iTripletNumber2Data(int iONr, tstCachedPage *pstCachedPage, unsigned char* pagedata)
{
	if (iONr > 506 || 0 == pstCachedPage)
		return -1;

	unsigned char *p;
	int packet = (iONr / 13) + 3;
	int packetoffset = 3 * (iONr % 13);

	if (packet <= 23)
		p = pagedata + 40*(packet-1) + packetoffset + 1;
	else if (packet <= 25)
	{
		if (0 == pstCachedPage->pageinfo.p24)
			return -1;
		p = pstCachedPage->pageinfo.p24 + 40*(packet-24) + packetoffset + 1;
	}
	else
	{
		int descode = packet - 26;
		if (0 == pstCachedPage->pageinfo.ext)
			return -1;
		if (0 == pstCachedPage->pageinfo.ext->p26[descode])
			return -1;
		p = pstCachedPage->pageinfo.ext->p26[descode] + packetoffset;	/* first byte (=designation code) is not cached */
	}
	return deh24(p);
}

#define RowAddress2Row(row) ((row == 40) ? 24 : (row - 40))

/* dump interpreted object data to stdout */
/* in: 18 bit object data */
/* out: termination info, >0 if end of object */
void tuxtxt_eval_object(int iONr, tstCachedPage *pstCachedPage,
					  unsigned char *pAPx, unsigned char *pAPy,
					  unsigned char *pAPx0, unsigned char *pAPy0,
					  tObjType ObjType, unsigned char* pagedata, unsigned char* page_char, tstPageAttr* page_atrb)
{
	int iOData;
	int iONr1 = iONr + 1; /* don't terminate after first triplet */
	unsigned char drcssubp=0, gdrcssubp=0;
	signed char endcol = -1; /* last column to which to extend attribute changes */
	tstPageAttr attrPassive = { tuxtxt_color_white  , tuxtxt_color_black , C_G0P, 0, 0, 1 ,0, 0, 0, 0, 0, 0, 0, 0x3f}; /* current attribute for passive objects */

	do
	{
		iOData = tuxtxt_iTripletNumber2Data(iONr, pstCachedPage,pagedata);	/* get triplet data, next triplet */
		if (iOData < 0) /* invalid number, not cached, or hamming error: terminate */
			break;
#if TUXTXT_DEBUG
		if (dumpl25)
			printf("  t%03d ", iONr);
#endif
		if (endcol < 0)
		{
			if (ObjType == OBJ_ACTIVE)
			{
				endcol = 40;
			}
			else if (ObjType == OBJ_ADAPTIVE) /* search end of line */
			{
				int i;
				for (i = iONr; i <= 506; i++)
				{
					int iTempOData = tuxtxt_iTripletNumber2Data(i, pstCachedPage,pagedata); /* get triplet data, next triplet */
					int iAddress = (iTempOData      ) & 0x3f;
					int iMode    = (iTempOData >>  6) & 0x1f;
					//int iData    = (iTempOData >> 11) & 0x7f;
					if (iTempOData < 0 || /* invalid number, not cached, or hamming error: terminate */
						 (iAddress >= 40	/* new row: row address and */
						 && (iMode == 0x01 || /* Full Row Color or */
							  iMode == 0x04 || /* Set Active Position */
							  (iMode >= 0x15 && iMode <= 0x17) || /* Object Definition */
							  iMode == 0x17))) /* Object Termination */
						break;
					if (iAddress < 40 && iMode != 0x06)
						endcol = iAddress;
				}
#if TUXTXT_DEBUG
				if (dumpl25)
					printf("  endcol %02d", endcol);
#endif
			}
		}
		iONr++;
	}
	while (0 == tuxtxt_eval_triplet(iOData, pstCachedPage, pAPx, pAPy, pAPx0, pAPy0, &drcssubp, &gdrcssubp, &endcol, &attrPassive, pagedata, page_char, page_atrb)
			 || iONr1 == iONr); /* repeat until termination reached */
}

void tuxtxt_eval_NumberedObject(int p, int s, int packet, int triplet, int high,
								 unsigned char *pAPx, unsigned char *pAPy,
								 unsigned char *pAPx0, unsigned char *pAPy0, unsigned char* page_char, tstPageAttr* page_atrb)
{
	if (!packet || 0 == tuxtxt_cache.astCachetable[p][s])
		return;
	unsigned char pagedata[23*40];
	tuxtxt_decompress_page(p, s,pagedata);


	int idata = deh24(pagedata + 40*(packet-1) + 1 + 3*triplet);
	int iONr;

	if (idata < 0)	/* hamming error: ignore triplet */
		return;
	if (high)
		iONr = idata >> 9; /* triplet number of odd object data */
	else
		iONr = idata & 0x1ff; /* triplet number of even object data */
	if (iONr <= 506)
	{
#if TUXTXT_DEBUG
		if (dumpl25)
			printf("P%xT%x%c %8s %c#%03d@%03d\n", packet, triplet, "LH"[!!high],	/* pointer pos, type, number, data pos */
					 ObjectType[triplet % 3], "PCD"[triplet % 3], 8*packet + 2*(triplet-1)/3, iONr);

#endif
		tuxtxt_eval_object(iONr, tuxtxt_cache.astCachetable[p][s], pAPx, pAPy, pAPx0, pAPy0, (tObjType)(triplet % 3),pagedata, page_char, page_atrb);
	}
}

int tuxtxt_eval_triplet(int iOData, tstCachedPage *pstCachedPage,
					  unsigned char *pAPx, unsigned char *pAPy,
					  unsigned char *pAPx0, unsigned char *pAPy0,
					  unsigned char *drcssubp, unsigned char *gdrcssubp,
					  signed char *endcol, tstPageAttr *attrPassive, unsigned char* pagedata, unsigned char* page_char, tstPageAttr* page_atrb)
{
	int iAddress = (iOData      ) & 0x3f;
	int iMode    = (iOData >>  6) & 0x1f;
	int iData    = (iOData >> 11) & 0x7f;

	if (iAddress < 40) /* column addresses */
	{
		int offset;	/* offset to page_char and page_atrb */

		if (iMode != 0x06)
			*pAPx = iAddress;	/* new Active Column */
		offset = (*pAPy0 + *pAPy) * 40 + *pAPx0 + *pAPx;	/* offset to page_char and page_atrb */
#if TUXTXT_DEBUG
		if (dumpl25)
			printf("  M%02xC%02xD%02x %d r:%d ch:%02x", iMode, iAddress, iData, *endcol,*pAPy0 + *pAPy,page_char[offset]);
#endif

		switch (iMode)
		{
		case 0x00:
			if (0 == (iData>>5))
			{
				int newcolor = iData & 0x1f;
				if (*endcol < 0) /* passive object */
					attrPassive->fg = newcolor;
				else if (*endcol == 40) /* active object */
				{
					tstPageAttr *p = &page_atrb[offset];
					int oldcolor = (p)->fg; /* current color (set-after) */
					int c = *pAPx0 + *pAPx;	/* current column absolute */
					do
					{
						p->fg = newcolor;
						p++;
						c++;
					} while (c < 40 && p->fg == oldcolor);	/* stop at change by level 1 page */
				}
				else /* adaptive object */
				{
					tstPageAttr *p = &page_atrb[offset];
					int c = *pAPx;	/* current column relative to object origin */
					do
					{
						p->fg = newcolor;
						p++;
						c++;
					} while (c <= *endcol);
				}
#if TUXTXT_DEBUG
				if (dumpl25)
					printf("  ,%02d FGCol T%x#%x", iAddress, (iData>>3)&0x03, iData&0x07);
#endif
			}
			break;
		case 0x01:
			if (iData >= 0x20)
			{
#if TUXTXT_DEBUG
				if (dumpl25)
					printf("  ,%02d BlockMosaic G1 #%02x", iAddress, iData);
#endif
				page_char[offset] = iData;
				if (*endcol < 0) /* passive object */
				{
					attrPassive->charset = C_G1C; /* FIXME: separated? */
					page_atrb[offset] = *attrPassive;
				}
				else if (page_atrb[offset].charset != C_G1S)
					page_atrb[offset].charset = C_G1C; /* FIXME: separated? */
			}
			break;
		case 0x02:
		case 0x0b:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  ,%02d G3 #%02x f%db%d", iAddress, iData,attrPassive->fg, attrPassive->bg);
#endif
			page_char[offset] = iData;
			if (*endcol < 0) /* passive object */
			{
				attrPassive->charset = C_G3;
				page_atrb[offset] = *attrPassive;
			}
			else
				page_atrb[offset].charset = C_G3;
			break;
		case 0x03:
			if (0 == (iData>>5))
			{
				int newcolor = iData & 0x1f;
				if (*endcol < 0) /* passive object */
					attrPassive->bg = newcolor;
				else if (*endcol == 40) /* active object */
				{
					tstPageAttr *p = &page_atrb[offset];
					int oldcolor = (p)->bg; /* current color (set-after) */
					int c = *pAPx0 + *pAPx;	/* current column absolute */
					do
					{
						p->bg = newcolor;
						if (newcolor == tuxtxt_color_black)
							p->IgnoreAtBlackBgSubst = 1;
						p++;
						c++;
					} while (c < 40 && p->bg == oldcolor);	/* stop at change by level 1 page */
				}
				else /* adaptive object */
				{
					tstPageAttr *p = &page_atrb[offset];
					int c = *pAPx;	/* current column relative to object origin */
					do
					{
						p->bg = newcolor;
						if (newcolor == tuxtxt_color_black)
							p->IgnoreAtBlackBgSubst = 1;
						p++;
						c++;
					} while (c <= *endcol);
				}
#if TUXTXT_DEBUG
				if (dumpl25)
					printf("  ,%02d BGCol T%x#%x", iAddress, (iData>>3)&0x03, iData&0x07);
#endif
			}
			break;
		case 0x06:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  PDC");
#endif
			/* ignore */
			break;
		case 0x07:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  ,%02d Flash M%xP%x", iAddress, iData & 0x03, (iData >> 2) & 0x07);
#endif
			if ((iData & 0x60) != 0) break; // reserved data field
			if (*endcol < 0) /* passive object */
			{
				attrPassive->flashing=iData & 0x1f;
				page_atrb[offset] = *attrPassive;
			}
			else
				page_atrb[offset].flashing=iData & 0x1f;
			break;
		case 0x08:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  ,%02d G0+G2 set #%02x (p105)", iAddress, iData);
#endif
			if (*endcol < 0) /* passive object */
			{
				attrPassive->setG0G2=iData & 0x3f;
				page_atrb[offset] = *attrPassive;
			}
			else
				page_atrb[offset].setG0G2=iData & 0x3f;
			break;
		case 0x09:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  ,%02d G0 #%02x '%c'", iAddress, iData, iData);
#endif
			page_char[offset] = iData;
			if (*endcol < 0) /* passive object */
			{
				attrPassive->charset = C_G0P; /* FIXME: secondary? */
				attrPassive->setX26  = 1;
				page_atrb[offset] = *attrPassive;
			}
			else
			{
				page_atrb[offset].charset = C_G0P; /* FIXME: secondary? */
				page_atrb[offset].setX26  = 1;
			}
			break;
//		case 0x0b: (see 0x02)
		case 0x0c:
		{
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  ,%02d Attribute%s%s%s%s%s%s", iAddress,
						 (iData & 0x40) ? " DoubleWidth" : "",
						 (iData & 0x20) ? " UnderlineSep" : "",
						 (iData & 0x10) ? " InvColour" : "",
						 (iData & 0x04) ? " Conceal" : "",
						 (iData & 0x02) ? " Boxing" : "",
						 (iData & 0x01) ? " DoubleHeight" : "");
#endif
			int conc = (iData & 0x04);
			int inv  = (iData & 0x10);
			int dw   = (iData & 0x40 ?1:0);
			int dh   = (iData & 0x01 ?1:0);
			int sep  = (iData & 0x20);
			int bw   = (iData & 0x02 ?1:0);
			if (*endcol < 0) /* passive object */
			{
				if (conc)
				{
					attrPassive->concealed = 1;
					attrPassive->fg = attrPassive->bg;
				}
				attrPassive->inverted = (inv ? 1- attrPassive->inverted : 0);
				attrPassive->doubleh = dh;
				attrPassive->doublew = dw;
				attrPassive->boxwin = bw;
				if (bw) attrPassive->IgnoreAtBlackBgSubst = 0;
				if (sep)
				{
					if (attrPassive->charset == C_G1C)
						attrPassive->charset = C_G1S;
					else
						attrPassive->underline = 1;
				}
				else
				{
					if (attrPassive->charset == C_G1S)
						attrPassive->charset = C_G1C;
					else
						attrPassive->underline = 0;
				}
			}
			else
			{

				int c = *pAPx0 + (*endcol == 40 ? *pAPx : 0);	/* current column */
				int c1 = offset;
				tstPageAttr *p = &page_atrb[offset];
				do
				{
					p->inverted = (inv ? 1- p->inverted : 0);
					if (conc)
					{
						p->concealed = 1;
						p->fg = p->bg;
					}
					if (sep)
					{
						if (p->charset == C_G1C)
							p->charset = C_G1S;
						else
							p->underline = 1;
					}
					else
					{
						if (p->charset == C_G1S)
							p->charset = C_G1C;
						else
							p->underline = 0;
					}
					p->doublew = dw;
					p->doubleh = dh;
					p->boxwin = bw;
					if (bw) p->IgnoreAtBlackBgSubst = 0;
					p++;
					c++;
					c1++;
				} while (c < *endcol);
			}
			break;
		}
		case 0x0d:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  ,%02d %cDRCS #%02x", iAddress, (iData & 0x40) ? ' ' : 'G', iData & 0x3f);
#endif
			page_char[offset] = iData & 0x3f;
			if (*endcol < 0) /* passive object */
			{
				attrPassive->charset = C_OFFSET_DRCS + ((iData & 0x40) ? (0x10 + *drcssubp) : *gdrcssubp);
				page_atrb[offset] = *attrPassive;
			}
			else
				page_atrb[offset].charset = C_OFFSET_DRCS + ((iData & 0x40) ? (0x10 + *drcssubp) : *gdrcssubp);
			break;
		case 0x0f:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  ,%02d G2 #%02x", iAddress, iData);
#endif
			page_char[offset] = iData;
			if (*endcol < 0) /* passive object */
			{
				attrPassive->charset = C_G2;
				page_atrb[offset] = *attrPassive;
			}
			else
				page_atrb[offset].charset = C_G2;
			break;
		default:
			if (iMode == 0x10 && iData == 0x2a)
				iData = '@';
			if (iMode >= 0x10)
			{
#if TUXTXT_DEBUG
				if (dumpl25)
					printf("  ,%02d G0 #%02x %c +diacr. %x", iAddress, iData, iData, iMode & 0x0f);
#endif
				page_char[offset] = iData;
				if (*endcol < 0) /* passive object */
				{
					attrPassive->charset = C_G0P;
					attrPassive->diacrit = iMode & 0x0f;
					attrPassive->setX26  = 1;
					page_atrb[offset] = *attrPassive;
				}
				else
				{
					page_atrb[offset].charset = C_G0P;
					page_atrb[offset].diacrit = iMode & 0x0f;
					page_atrb[offset].setX26  = 1;
				}
			}
			break; /* unsupported or not yet implemented mode: ignore */
		} /* switch (iMode) */
	}
	else /* ================= (iAddress >= 40): row addresses ====================== */
	{
#if TUXTXT_DEBUG
		if (dumpl25)
			printf("  M%02xR%02xD%02x", iMode, iAddress, iData);
#endif
		switch (iMode)
		{
		case 0x00:
			if (0 == (iData>>5))
			{
#if TUXTXT_DEBUG
				if (dumpl25)
					printf("  FScrCol T%x#%x", (iData>>3)&0x03, iData&0x07);
#endif
				tuxtxt_cache.FullScrColor = iData & 0x1f;
			}
			break;
		case 0x01:
			if (*endcol == 40) /* active object */
			{
				*pAPy = RowAddress2Row(iAddress);	/* new Active Row */

				int color = iData & 0x1f;
				int row = *pAPy0 + *pAPy;
				int maxrow;
#if TUXTXT_DEBUG
				if (dumpl25)
				{
					printf("  AP=%d,0", RowAddress2Row(iAddress));
					if (0 == (iData>>5))
						printf("  FRowCol T%x#%x", (iData>>3)&0x03, iData&0x07);
					else if (3 == (iData>>5))
						printf("  FRowCol++ T%x#%x", (iData>>3)&0x03, iData&0x07);
				}
#endif
				if (row <= 24 && 0 == (iData>>5))
					maxrow = row;
				else if (3 == (iData>>5))
					maxrow = 24;
				else
					maxrow = -1;
				for (; row <= maxrow; row++)
					tuxtxt_cache.FullRowColor[row] = color;
				*endcol = -1;
			}
			break;
		case 0x04:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf(" AP=%d,%d", RowAddress2Row(iAddress), iData);
#endif
			*pAPy = RowAddress2Row(iAddress); /* new Active Row */
			if (iData < 40)
				*pAPx = iData;	/* new Active Column */
			*endcol = -1; /* FIXME: check if row changed? */
			break;
		case 0x07:
#if TUXTXT_DEBUG
			if (dumpl25)
			{
				if (iAddress == 0x3f)
					printf("  AP=0,0");
				if (0 == (iData>>5))
					printf("  Address Display R0 FRowCol T%x#%x", (iData>>3)&0x03, iData&0x07);
				else if (3 == (iData>>5))
					printf("  Address Display R0->24 FRowCol T%x#%x", (iData>>3)&0x03, iData&0x07);
			}
#endif
			if (iAddress == 0x3f)
			{
				*pAPx = *pAPy = 0; /* new Active Position 0,0 */
				if (*endcol == 40) /* active object */
				{
					int color = iData & 0x1f;
					int row = *pAPy0; // + *pAPy;
					int maxrow;

					if (row <= 24 && 0 == (iData>>5))
						maxrow = row;
					else if (3 == (iData>>5))
						maxrow = 24;
					else
						maxrow = -1;
					for (; row <= maxrow; row++)
						tuxtxt_cache.FullRowColor[row] = color;
				}
				*endcol = -1;
			}
			break;
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0c:
		case 0x0d:
		case 0x0e:
		case 0x0f:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  PDC");
#endif
			/* ignore */
			break;
		case 0x10:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  AP=%d,%d  temp. Origin Modifier", iAddress - 40, iData);
#endif
			tuxtxt_cache.tAPy = iAddress - 40;
			tuxtxt_cache.tAPx = iData;
			break;
		case 0x11:
		case 0x12:
		case 0x13:
			if (iAddress & 0x10)	/* POP or GPOP */
			{
				unsigned char APx = 0, APy = 0;
				unsigned char APx0 = *pAPx0 + *pAPx + tuxtxt_cache.tAPx, APy0 = *pAPy0 + *pAPy + tuxtxt_cache.tAPy;
				int triplet = 3 * ((iData >> 5) & 0x03) + (iMode & 0x03);
				int packet = (iAddress & 0x03) + 1;
				int subp = iData & 0x0f;
				int high = (iData >> 4) & 0x01;


				if (APx0 < 40) /* not in side panel */
				{
#if TUXTXT_DEBUG
					if (dumpl25)
						printf("  Object Invocation %5s %8s S%xP%xT%x%c %c#%03d\n---",
								 ObjectSource[(iAddress >> 3) & 0x03], ObjectType[iMode & 0x03],
								 subp,
							 packet,
								 triplet,
								 "LH"[high], /* low/high */
								 "PCD"[triplet % 3],
								 8*packet + 2*(triplet-1)/3 + 1);
#endif
					tuxtxt_eval_NumberedObject((iAddress & 0x08) ? tuxtxt_cache.gpop : tuxtxt_cache.pop, subp, packet, triplet, high, &APx, &APy, &APx0, &APy0, page_char,page_atrb);
#if TUXTXT_DEBUG
					if (dumpl25)
						printf("---");
#endif
				}
#if TUXTXT_DEBUG
				else if (dumpl25)
					printf("Object Invocation for Side Panel - ignored");
#endif
			}
			else if (iAddress & 0x08)	/* local: eval invoked object */
			{
				unsigned char APx = 0, APy = 0;
				unsigned char APx0 = *pAPx0 + *pAPx + tuxtxt_cache.tAPx, APy0 = *pAPy0 + *pAPy + tuxtxt_cache.tAPy;
				int descode = ((iAddress & 0x01) << 3) | (iData >> 4);
				int triplet = iData & 0x0f;

				if (APx0 < 40) /* not in side panel */
				{
#if TUXTXT_DEBUG
					if (dumpl25)
						printf("  local Object Invocation %5s %8s D%xT%x:\n---",
								 ObjectSource[(iAddress >> 3) & 0x03], ObjectType[iMode & 0x03], descode, triplet);
#endif
					tuxtxt_eval_object(13 * 23 + 13 * descode + triplet, pstCachedPage, &APx, &APy, &APx0, &APy0, (tObjType)(triplet % 3), pagedata, page_char, page_atrb);
#if TUXTXT_DEBUG
					if (dumpl25)
						printf("---");
#endif
				}
#if TUXTXT_DEBUG
				else if (dumpl25)
					printf("local Object Invocation for Side Panel - ignored");
#endif
			}
			break;
		case 0x15:
		case 0x16:
		case 0x17:
			if (0 == (iAddress & 0x08))	/* Object Definition illegal or only level 3.5 */
				break; /* ignore */
#if TUXTXT_DEBUG
			if (dumpl25)
			{
				printf("  Object Definition %8s", ObjectType[iMode & 0x03]);
				{ /* *POP */
					int triplet = 3 * ((iData >> 5) & 0x03) + (iMode & 0x03);
					int packet = (iAddress & 0x03) + 1;
					printf("  S%xP%xT%x%c %c#%03d",
							 iData & 0x0f,	/* subpage */
							 packet,
							 triplet,
							 "LH"[(iData >> 4) & 0x01], /* low/high */
							 "PCD"[triplet % 3],
							 8*packet + 2*(triplet-1)/3 + 1);
				}
				{ /* local */
					int descode = ((iAddress & 0x03) << 3) | (iData >> 4);
					int triplet = iData & 0x0f;
					printf("  D%xT%x", descode, triplet);
				}
				putchar('\n');
			}
#endif
			tuxtxt_cache.tAPx = tuxtxt_cache.tAPy = 0;
			*endcol = -1;
			return 0xFF; /* termination by object definition */
			break;
		case 0x18:
			if (0 == (iData & 0x10)) /* DRCS Mode reserved or only level 3.5 */
				break; /* ignore */
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  %cDRCS S%x", (iData & 0x40) ? ' ' : 'G', iData & 0x0f);	/* subpage */
#endif
			if (iData & 0x40)
				*drcssubp = iData & 0x0f;
			else
				*gdrcssubp = iData & 0x0f;
			break;
		case 0x1f:
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("  Termination Marker %x\n", iData);	/* subpage */
#endif
			tuxtxt_cache.tAPx = tuxtxt_cache.tAPy = 0;
			*endcol = -1;
			return 0x80 | iData; /* explicit termination */
			break;
		default:
			break; /* unsupported or not yet implemented mode: ignore */
		} /* switch (iMode) */
	} /* (iAddress >= 40): row addresses */
#if TUXTXT_DEBUG
	if (dumpl25 && iAddress < 40)
		putchar('\n');
#endif
	if (iAddress < 40 || iMode != 0x10) /* leave temp. AP-Offset unchanged only immediately after definition */
		tuxtxt_cache.tAPx = tuxtxt_cache.tAPy = 0;


	return 0; /* normal exit, no termination */
}
int tuxtxt_setnational(unsigned char sec)
{
	switch (sec)
	{
		case 0x08:
			return NAT_PL; //polish
		case 0x16:
		case 0x36:
			return NAT_TR; //turkish
		case 0x1d:
			return NAT_SR; //serbian, croatian, slovenian
		case 0x20:
			return NAT_SC; // serbian, croatian
		case 0x24:
			return NAT_RB; // russian, bulgarian
		case 0x25:
			return NAT_UA; // ukrainian
		case 0x22:
			return NAT_ET; // estonian
		case 0x23:
			return NAT_LV; // latvian, lithuanian
		case 0x37:
			return NAT_GR; // greek
		case 0x55:
			return NAT_HB; // hebrew			
		case 0x47:
		case 0x57:
			return NAT_AR; // arabic
	}
	return countryconversiontable[sec & 0x07];
}
/* evaluate level 2.5 information */
void tuxtxt_eval_l25(unsigned char* page_char, tstPageAttr *page_atrb, int hintmode)
{
	memset(tuxtxt_cache.FullRowColor, 0, sizeof(tuxtxt_cache.FullRowColor));
	tuxtxt_cache.FullScrColor = tuxtxt_color_black;
	tuxtxt_cache.colortable = NULL;

	if (!tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage])
		return;

#if TUXTXT_DEBUG
	if (dumpl25)
		printf("=== %03x/%02x %d/%d===\n", tuxtxt_cache.page, tuxtxt_cache.subpage,tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage]->pageinfo.nationalvalid,tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage]->pageinfo.national);
#endif


	/* normal page */
	if (tuxtxt_is_dec(tuxtxt_cache.page))
	{
		unsigned char APx0, APy0, APx, APy;
		tstPageinfo *pi = &(tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage]->pageinfo);
		tstCachedPage *pmot = tuxtxt_cache.astCachetable[(tuxtxt_cache.page & 0xf00) | 0xfe][0];
		int p26Received = 0;
		int BlackBgSubst = 0;
		int ColorTableRemapping = 0;


		tuxtxt_cache.pop = tuxtxt_cache.gpop = tuxtxt_cache.drcs = tuxtxt_cache.gdrcs = 0;


		if (pi->ext)
		{
			tstExtData *e = pi->ext;

			if (e->p26[0])
				p26Received = 1;

			if (e->p27)
			{
				tstp27 *p27 = e->p27;
				if (p27[0].l25)
					tuxtxt_cache.gpop = p27[0].page;
				if (p27[1].l25)
					tuxtxt_cache.pop = p27[1].page;
				if (p27[2].l25)
					tuxtxt_cache.gdrcs = p27[2].page;
				if (p27[3].l25)
					tuxtxt_cache.drcs = p27[3].page;
			}

			if (e->p28Received)
			{
				tuxtxt_cache.colortable = e->bgr;
				BlackBgSubst = e->BlackBgSubst;
				ColorTableRemapping = e->ColorTableRemapping;
				memset(tuxtxt_cache.FullRowColor, e->DefRowColor, sizeof(tuxtxt_cache.FullRowColor));
				tuxtxt_cache.FullScrColor = e->DefScreenColor;
				tuxtxt_cache.national_subset = tuxtxt_setnational(e->DefaultCharset);
				tuxtxt_cache.national_subset_secondary = tuxtxt_setnational(e->SecondCharset);
#if TUXTXT_DEBUG
				if (dumpl25)
				{
					int c; /* color */
					printf("p28/0: DefCharset %02x Sec %02x SidePanel %c%c%x DefScrCol %02x DefRowCol %02x BlBgSubst %x Map %x\n CBGR",
							 e->DefaultCharset,
							 e->SecondCharset,
							 e->LSP ? (e->SPL25 ? 'L' : 'l') : '-',	/* left panel (small: only in level 3.5) */
							 e->RSP ? (e->SPL25 ? 'R' : 'r') : '-',	/* right panel (small: only in level 3.5) */
							 e->LSPColumns,
							 e->DefScreenColor,
							 e->DefRowColor,
							 e->BlackBgSubst,
							 e->ColorTableRemapping);
					for (c = 0; c < 16; c++)
						printf(" %x%03x", c, e->bgr[c]);
					putchar('\n');
				}
#endif
			} /* e->p28Received */
		}

		if (!tuxtxt_cache.colortable && tuxtxt_cache.astP29[tuxtxt_cache.page >> 8])
		{
			tstExtData *e = tuxtxt_cache.astP29[tuxtxt_cache.page >> 8];
			tuxtxt_cache.colortable = e->bgr;
			BlackBgSubst = e->BlackBgSubst;
			ColorTableRemapping = e->ColorTableRemapping;
			memset(tuxtxt_cache.FullRowColor, e->DefRowColor, sizeof(tuxtxt_cache.FullRowColor));
			tuxtxt_cache.FullScrColor = e->DefScreenColor;
			tuxtxt_cache.national_subset = tuxtxt_setnational(e->DefaultCharset);
			tuxtxt_cache.national_subset_secondary = tuxtxt_setnational(e->SecondCharset);
#if TUXTXT_DEBUG
			if (dumpl25)
			{
				int c; /* color */
				printf("p29/0: DefCharset %02x Sec %02x SidePanel %c%c%x DefScrCol %02x DefRowCol %02x BlBgSubst %x Map %x\n CBGR",
						 e->DefaultCharset,
						 e->SecondCharset,
						 e->LSP ? (e->SPL25 ? 'L' : 'l') : '-',	/* left panel (small: only in level 3.5) */
						 e->RSP ? (e->SPL25 ? 'R' : 'r') : '-',	/* right panel (small: only in level 3.5) */
						 e->LSPColumns,
						 e->DefScreenColor,
						 e->DefRowColor,
						 e->BlackBgSubst,
						 e->ColorTableRemapping);
				for (c = 0; c < 16; c++)
					printf(" %x%03x", c, e->bgr[c]);
				putchar('\n');
			}
#endif

		}

		if (ColorTableRemapping)
		{
			int i;
			for (i = 0; i < 25*40; i++)
			{
				page_atrb[i].fg += MapTblFG[ColorTableRemapping - 1];
				if (!BlackBgSubst || page_atrb[i].bg != tuxtxt_color_black || page_atrb[i].IgnoreAtBlackBgSubst)
					page_atrb[i].bg += MapTblBG[ColorTableRemapping - 1];
			}
		}

		/* determine ?pop/?drcs from MOT */
		if (pmot)
		{
			unsigned char pmot_data[23*40];
			tuxtxt_decompress_page((tuxtxt_cache.page & 0xf00) | 0xfe,0,pmot_data);

			unsigned char *p = pmot_data; /* start of link data */
			int o = 2 * (((tuxtxt_cache.page & 0xf0) >> 4) * 10 + (tuxtxt_cache.page & 0x0f));	/* offset of links for current page */
			int opop = p[o] & 0x07;	/* index of POP link */
			int odrcs = p[o+1] & 0x07;	/* index of DRCS link */
			unsigned char obj[3*4*4]; // types* objects * (triplet,packet,subp,high)
			unsigned char type,ct, tstart = 4*4;
			memset(obj,0,sizeof(obj));


			if (p[o] & 0x08) /* GPOP data used */
			{
				if (!tuxtxt_cache.gpop || !(p[18*40] & 0x08)) /* no p27 data or higher prio of MOT link */
				{
					tuxtxt_cache.gpop = ((p[18*40] << 8) | (p[18*40+1] << 4) | p[18*40+2]) & 0x7ff;
					if ((tuxtxt_cache.gpop & 0xff) == 0xff)
						tuxtxt_cache.gpop = 0;
					else
					{
						if (tuxtxt_cache.gpop < 0x100)
							tuxtxt_cache.gpop += 0x800;
						if (!p26Received)
						{
							ct=2;
							while (ct)
							{
								ct--;
								type = (p[18*40+5] >> 2*ct) & 0x03;

								if (type == 0) continue;
							    obj[(type-1)*(tstart)+ct*4  ] = 3 * ((p[18*40+7+ct*2] >> 1) & 0x03) + type; //triplet
							    obj[(type-1)*(tstart)+ct*4+1] = ((p[18*40+7+ct*2] & 0x08) >> 3) + 1       ; //packet
							    obj[(type-1)*(tstart)+ct*4+2] = p[18*40+6+ct*2] & 0x0f                    ; //subp
							    obj[(type-1)*(tstart)+ct*4+3] = p[18*40+7+ct*2] & 0x01                    ; //high

#if TUXTXT_DEBUG
								if (dumpl25)
									printf("GPOP  DefObj%d S%xP%xT%x%c %c#%03d\n"
										,2-ct
										, obj[(type-1)*(tstart)+ct*4+2]
										, obj[(type-1)*(tstart)+ct*4+1]
										, obj[(type-1)*(tstart)+ct*4]
										, "LH"[obj[(type-1)*(tstart)+ct*4+3]]
										, "-CDP"[type]
										, 8*(obj[(type-1)*(tstart)+ct*4+1]-1) + 2*(obj[(type-1)*(tstart)+ct*4]-1)/3 + 1);
#endif
							}
						}
					}
				}
			}
			if (opop) /* POP data used */
			{
				opop = 18*40 + 10*opop;	/* offset to POP link */
				if (!tuxtxt_cache.pop || !(p[opop] & 0x08)) /* no p27 data or higher prio of MOT link */
				{
					tuxtxt_cache.pop = ((p[opop] << 8) | (p[opop+1] << 4) | p[opop+2]) & 0x7ff;
					if ((tuxtxt_cache.pop & 0xff) == 0xff)
						tuxtxt_cache.pop = 0;
					else
					{
						if (tuxtxt_cache.pop < 0x100)
							tuxtxt_cache.pop += 0x800;
						if (!p26Received)
						{
							ct=2;
							while (ct)
							{
								ct--;
								type = (p[opop+5] >> 2*ct) & 0x03;

								if (type == 0) continue;
							    obj[(type-1)*(tstart)+(ct+2)*4  ] = 3 * ((p[opop+7+ct*2] >> 1) & 0x03) + type; //triplet
							    obj[(type-1)*(tstart)+(ct+2)*4+1] = ((p[opop+7+ct*2] & 0x08) >> 3) + 1       ; //packet
							    obj[(type-1)*(tstart)+(ct+2)*4+2] = p[opop+6+ct*2]                           ; //subp
							    obj[(type-1)*(tstart)+(ct+2)*4+3] = p[opop+7+ct*2] & 0x01                    ; //high
#if TUXTXT_DEBUG
								if (dumpl25)
									printf("POP  DefObj%d S%xP%xT%x%c %c#%03d\n"
										, 2-ct
										, obj[(type-1)*(tstart)+(ct+2)*4+2]
										, obj[(type-1)*(tstart)+(ct+2)*4+1]
										, obj[(type-1)*(tstart)+(ct+2)*4]
										, "LH"[obj[(type-1)*(tstart)+(ct+2)*4+3]]
										, "-CDP"[type], 8*(obj[(type-1)*(tstart)+(ct+2)*4+1]-1) + 2*(obj[(type-1)*(tstart)+(ct+2)*4]-1)/3 + 1);
#endif
							}
						}
					}
				}
			}
			// eval default objects in correct order
			for (ct = 0; ct < 12; ct++)
			{
#if TUXTXT_DEBUG
								if (dumpl25)
									printf("eval  DefObjs : %d S%xP%xT%x%c %c#%03d\n"
										, ct
										, obj[ct*4+2]
										, obj[ct*4+1]
										, obj[ct*4]
										, "LH"[obj[ct*4+3]]
										, "-CDP"[obj[ct*4 % 3]]
										, 8*(obj[ct*4+1]-1) + 2*(obj[ct*4]-1)/3 + 1);
#endif
				if (obj[ct*4] != 0)
				{
					APx0 = APy0 = APx = APy = tuxtxt_cache.tAPx = tuxtxt_cache.tAPy = 0;
					tuxtxt_eval_NumberedObject(ct % 4 > 1 ? tuxtxt_cache.pop : tuxtxt_cache.gpop, obj[ct*4+2], obj[ct*4+1], obj[ct*4], obj[ct*4+3], &APx, &APy, &APx0, &APy0, page_char, page_atrb);
				}
			}

			if (p[o+1] & 0x08) /* GDRCS data used */
			{
				if (!tuxtxt_cache.gdrcs || !(p[20*40] & 0x08)) /* no p27 data or higher prio of MOT link */
				{
					tuxtxt_cache.gdrcs = ((p[20*40] << 8) | (p[20*40+1] << 4) | p[20*40+2]) & 0x7ff;
					if ((tuxtxt_cache.gdrcs & 0xff) == 0xff)
						tuxtxt_cache.gdrcs = 0;
					else if (tuxtxt_cache.gdrcs < 0x100)
						tuxtxt_cache.gdrcs += 0x800;
				}
			}
			if (odrcs) /* DRCS data used */
			{
				odrcs = 20*40 + 4*odrcs;	/* offset to DRCS link */
				if (!tuxtxt_cache.drcs || !(p[odrcs] & 0x08)) /* no p27 data or higher prio of MOT link */
				{
					tuxtxt_cache.drcs = ((p[odrcs] << 8) | (p[odrcs+1] << 4) | p[odrcs+2]) & 0x7ff;
					if ((tuxtxt_cache.drcs & 0xff) == 0xff)
						tuxtxt_cache.drcs = 0;
					else if (tuxtxt_cache.drcs < 0x100)
						tuxtxt_cache.drcs += 0x800;
				}
			}
			if (tuxtxt_cache.astCachetable[tuxtxt_cache.gpop][0])
				tuxtxt_cache.astCachetable[tuxtxt_cache.gpop][0]->pageinfo.function = FUNC_GPOP;
			if (tuxtxt_cache.astCachetable[tuxtxt_cache.pop][0])
				tuxtxt_cache.astCachetable[tuxtxt_cache.pop][0]->pageinfo.function = FUNC_POP;
			if (tuxtxt_cache.astCachetable[tuxtxt_cache.gdrcs][0])
				tuxtxt_cache.astCachetable[tuxtxt_cache.gdrcs][0]->pageinfo.function = FUNC_GDRCS;
			if (tuxtxt_cache.astCachetable[tuxtxt_cache.drcs][0])
				tuxtxt_cache.astCachetable[tuxtxt_cache.drcs][0]->pageinfo.function = FUNC_DRCS;
		} /* if mot */

#if TUXTXT_DEBUG
		if (dumpl25)
			printf("gpop %03x pop %03x gdrcs %03x drcs %03x p28/0: Func %x Natvalid %x Nat %x Box %x\n",
					 tuxtxt_cache.gpop, tuxtxt_cache.pop, tuxtxt_cache.gdrcs, tuxtxt_cache.drcs,
					 pi->function, pi->nationalvalid, pi->national, pi->boxed);
#endif

		/* evaluate local extension data from p26 */
		if (p26Received)
		{
#if TUXTXT_DEBUG
			if (dumpl25)
				printf("p26/x:\n");
#endif
			APx0 = APy0 = APx = APy = tuxtxt_cache.tAPx = tuxtxt_cache.tAPy = 0;
			tuxtxt_eval_object(13 * (23-2 + 2), tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage], &APx, &APy, &APx0, &APy0, OBJ_ACTIVE, &page_char[40], page_char, page_atrb); /* 1st triplet p26/0 */
		}

		{
			int r, c;
			int o = 0;


			for (r = 0; r < 25; r++)
				for (c = 0; c < 40; c++)
				{
					if (BlackBgSubst && page_atrb[o].bg == tuxtxt_color_black && !(page_atrb[o].IgnoreAtBlackBgSubst))
					{
						if (tuxtxt_cache.FullRowColor[r] == 0x08)
							page_atrb[o].bg = tuxtxt_cache.FullScrColor;
						else
							page_atrb[o].bg = tuxtxt_cache.FullRowColor[r];
					}
					o++;
				}
		}

		if (!hintmode)
		{
			int i;
			for (i = 0; i < 25*40; i++)
			{
				if (page_atrb[i].concealed) page_atrb[i].fg = page_atrb[i].bg;
			}
		}

	} /* is_dec(page) */

}

/******************************************************************************
 * DecodePage                                                                 *
 ******************************************************************************/

tstPageinfo* tuxtxt_DecodePage(int showl25, // 1=decode Level2.5-graphics
				 unsigned char* page_char, // page buffer, min. 25*40 
				 tstPageAttr *page_atrb, // attribut buffer, min 25*40
				 int hintmode,// 1=show hidden information
				 int showflof // 1=decode FLOF-line
				 )
{
	int row, col;
	int hold, dhset;
	int foreground, background, doubleheight, doublewidth, charset, previous_charset, mosaictype, IgnoreAtBlackBgSubst, concealed, flashmode, boxwin;
	unsigned char held_mosaic, *p;
	tstCachedPage *pCachedPage;

	/* copy page to decode buffer */
	if (tuxtxt_cache.subpagetable[tuxtxt_cache.page] == 0xff) /* not cached: do nothing */
		return NULL;
	if (tuxtxt_cache.zap_subpage_manual)
		pCachedPage = tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpage];
	else
		pCachedPage = tuxtxt_cache.astCachetable[tuxtxt_cache.page][tuxtxt_cache.subpagetable[tuxtxt_cache.page]];
	if (!pCachedPage)	/* not cached: do nothing */
		return NULL;

	tuxtxt_decompress_page(tuxtxt_cache.page,tuxtxt_cache.subpage,&page_char[40]);

	memcpy(&page_char[8], pCachedPage->p0, 24); /* header line without timestring */

	tstPageinfo* pageinfo = &(pCachedPage->pageinfo);
	if (pageinfo->p24)
		memcpy(&page_char[24*40], pageinfo->p24, 40); /* line 25 for FLOF */

	/* copy timestring */
	memcpy(&page_char[32], &tuxtxt_cache.timestring, 8);

	int boxed;
	/* check for newsflash & subtitle */
	if (pageinfo->boxed && tuxtxt_is_dec(tuxtxt_cache.page))
		boxed = 1;
	else
		boxed = 0;


	/* modify header */
	if (boxed)
		memset(page_char, ' ', 40);
	else
	{
		memset(page_char, ' ', 8);
		tuxtxt_hex2str(page_char+3, tuxtxt_cache.page);
		if (tuxtxt_cache.subpage)
		{
			*(page_char+4) ='/';
			*(page_char+5) ='0';
			tuxtxt_hex2str(page_char+6, tuxtxt_cache.subpage);
		}

	}

	if (!tuxtxt_is_dec(tuxtxt_cache.page))
	{
		tstPageAttr atr = { tuxtxt_color_white  , tuxtxt_color_black , C_G0P, 0, 0, 0 ,0, 0, 0, 0, 0, 0, 0, 0x3f};
		if (pageinfo->function == FUNC_MOT) /* magazine organization table */
		{
#if TUXTXT_DEBUG
			printf("TuxTxt <decoding MOT %03x/%02x %d>\n", tuxtxt_cache.page, tuxtxt_cache.subpage, pageinfo->function);
#endif
			for (col = 0; col < 24*40; col++)
				page_atrb[col] = atr;
			for (col = 40; col < 24*40; col++)
				page_char[col] = number2char(page_char[col]);
			boxed = 0;
			return pageinfo; /* don't interpret irregular pages */
		}
		else if (pageinfo->function == FUNC_GPOP || pageinfo->function == FUNC_POP) /* object definitions */
		{
#if TUXTXT_DEBUG
			printf("TuxTxt <decoding *POP %03x/%02x %d>\n", tuxtxt_cache.page, tuxtxt_cache.subpage, pageinfo->function);
#endif
			for (col = 0; col < 24*40; col++)
				page_atrb[col] = atr;
			p = page_char + 40;
			for (row = 1; row < 12; row++)
			{
				*p++ = number2char(row); /* first column: number (0-9, A-..) */
				for (col = 1; col < 40; col += 3)
				{
					int d = deh24(p);
					if (d < 0)
					{
						memcpy(p, "???", 3);
					p += 3;
					}
					else
					{
						*p++ = number2char((d >> 6) & 0x1f); /* mode */
						*p++ = number2char(d & 0x3f); /* address */
						*p++ = number2char((d >> 11) & 0x7f); /* data */
					}
				}
			}
			boxed = 0;
			return pageinfo; /* don't interpret irregular pages */
		}
		else if (pageinfo->function == FUNC_GDRCS || pageinfo->function == FUNC_DRCS) /* character definitions */
		{
			boxed = 0;
			return pageinfo; /* don't interpret irregular pages */
		}
		else
		{
			int i;
			int h, parityerror = 0;

			for (i = 0; i < 8; i++)
				page_atrb[i] = atr;

			/* decode parity/hamming */
			for (i = 40; i < sizeof(page_char); i++)
			{
				page_atrb[i] = atr;
				p = page_char + i;
				h = dehamming[*p];
				if (parityerror && h != 0xFF)	/* if no regular page (after any parity error) */
					tuxtxt_hex2str(p, h);	/* first try dehamming */
				else
				{
					if (*p == ' ' || deparity[*p] != ' ') /* correct parity */
						*p &= 127;
					else
					{
						parityerror = 1;
						if (h != 0xFF)	/* first parity error: try dehamming */
							tuxtxt_hex2str(p, h);
						else
							*p = ' ';
					}
				}
			}
			if (parityerror)
			{
				boxed = 0;
				return pageinfo; /* don't interpret irregular pages */
			}
		}
	}
	int mosaic_pending,esc_pending;
	/* decode */
	for (row = 0;
		  row < ((showflof && pageinfo->p24) ? 25 : 24);
		  row++)
	{
		/* start-of-row default conditions */
		foreground   = tuxtxt_color_white;
		background   = tuxtxt_color_black;
		doubleheight = 0;
		doublewidth  = 0;
		charset      = previous_charset = C_G0P; // remember charset for switching back after mosaic charset was used
		mosaictype   = 0;
		concealed    = 0;
		flashmode    = 0;
		hold         = 0;
		boxwin		 = 0;
		held_mosaic  = ' ';
		dhset        = 0;
		IgnoreAtBlackBgSubst = 0;
		mosaic_pending = esc_pending = 0; // we need to render at least one mosaic char if 'esc' is received immediatly after mosac charset switch on

		if (boxed && memchr(&page_char[row*40], start_box, 40) == 0)
		{
			foreground = tuxtxt_color_transp;
			background = tuxtxt_color_transp;
		}

		for (col = 0; col < 40; col++)
		{
			int index = row*40 + col;

			page_atrb[index].fg = foreground;
			page_atrb[index].bg = background;
			page_atrb[index].charset = charset;
			page_atrb[index].doubleh = doubleheight;
			page_atrb[index].doublew = (col < 39 ? doublewidth : 0);
			page_atrb[index].IgnoreAtBlackBgSubst = IgnoreAtBlackBgSubst;
			page_atrb[index].concealed = concealed;
			page_atrb[index].flashing  = flashmode;
			page_atrb[index].boxwin    = boxwin;
			page_atrb[index].inverted  = 0; // only relevant for Level 2.5
			page_atrb[index].underline = 0; // only relevant for Level 2.5
			page_atrb[index].diacrit   = 0; // only relevant for Level 2.5
			page_atrb[index].setX26    = 0; // only relevant for Level 2.5
			page_atrb[index].setG0G2   = 0x3f; // only relevant for Level 2.5

			if (page_char[index] < ' ')
			{
				if (esc_pending) { // mosaic char has been rendered and we can switch charsets
					charset = previous_charset;
					if (charset == C_G0P)
						charset = previous_charset = C_G0S;
					else if (charset == C_G0S)
						charset = previous_charset = C_G0P;
					esc_pending = 0;
				}
				switch (page_char[index])
				{
				case alpha_black:
				case alpha_red:
				case alpha_green:
				case alpha_yellow:
				case alpha_blue:
				case alpha_magenta:
				case alpha_cyan:
				case alpha_white:
					concealed = 0;
					foreground = page_char[index] - alpha_black + tuxtxt_color_black;
					if (col == 0 && page_char[index] == alpha_white)
						page_atrb[index].fg = tuxtxt_color_black; // indicate level 1 color change on column 0; (hack)
					if ((charset!=C_G0P) && (charset!=C_G0S)) // we need to change charset to state it was before mosaic
						charset = previous_charset;
					break;

				case flash:
					flashmode = 1;
					break;
				case steady:
					flashmode = 0;
					page_atrb[index].flashing = 0;
					break;
				case end_box:
					boxwin = 0;
					IgnoreAtBlackBgSubst = 0;
/*					if (boxed)
					{
						foreground = tuxtxt_color_transp;
						background = tuxtxt_color_transp;
						IgnoreAtBlackBgSubst = 0;
					}
*/
					break;

				case start_box:
					if (!boxwin)
					{
						boxwin = 1;
						//background = 0x08;
					}
/*					if (boxed)
					{
						int rowstart = row * 40;
						if (col > 0)
							memset(&page_char[rowstart], ' ', col);
						for (clear = 0; clear < col; clear++)
						{
							page_atrb[rowstart + clear].fg = page_atrb[rowstart + clear].bg = tuxtxt_color_transp;
							page_atrb[rowstart + clear].IgnoreAtBlackBgSubst = 0;
						}
					}
*/
					break;

				case normal_size:
					doubleheight = 0;
					doublewidth = 0;
					page_atrb[index].doubleh = doubleheight;
					page_atrb[index].doublew = doublewidth;
					break;

				case double_height:
					if (row < 23)
					{
						doubleheight = 1;
						dhset = 1;
					}
					doublewidth = 0;

					break;

				case double_width:
					if (col < 39)
						doublewidth = 1;
					doubleheight = 0;
					break;

				case double_size:
					if (row < 23)
					{
						doubleheight = 1;
						dhset = 1;
					}
					if (col < 39)
						doublewidth = 1;
					break;

				case mosaic_black:
				case mosaic_red:
				case mosaic_green:
				case mosaic_yellow:
				case mosaic_blue:
				case mosaic_magenta:
				case mosaic_cyan:
				case mosaic_white:
					concealed = 0;
					foreground = page_char[index] - mosaic_black + tuxtxt_color_black;
					if ((charset==C_G0P) || (charset==C_G0S))
						previous_charset=charset;
					charset = mosaictype ? C_G1S : C_G1C;
					mosaic_pending = 1;					
					break;

				case conceal:
					page_atrb[index].concealed = 1;
					concealed = 1;
					if (!hintmode)
					{
						foreground = background;
						page_atrb[index].fg = foreground;
					}
					break;

				case contiguous_mosaic:
					mosaictype = 0;
					if (charset == C_G1S)
					{
						charset = C_G1C;
						page_atrb[index].charset = charset;
					}
					break;

				case separated_mosaic:
					mosaictype = 1;
					if (charset == C_G1C)
					{
						charset = C_G1S;
						page_atrb[index].charset = charset;
					}
					break;

				case esc:
					if (!mosaic_pending) { // if mosaic is pending we need to wait before mosaic arrives
						if ((charset != C_G0P) && (charset != C_G0S)) // we need to switch to charset which was active before mosaic
							charset = previous_charset;
						if (charset == C_G0P)
							charset = previous_charset = C_G0S;
						else if (charset == C_G0S)
							charset = previous_charset = C_G0P;
					} else esc_pending = 1;
					break;

				case black_background:
					background = tuxtxt_color_black;
					IgnoreAtBlackBgSubst = 0;
					page_atrb[index].bg = background;
					page_atrb[index].IgnoreAtBlackBgSubst = IgnoreAtBlackBgSubst;
					break;

				case new_background:
					background = foreground;
					if (background == tuxtxt_color_black)
						IgnoreAtBlackBgSubst = 1;
					else
						IgnoreAtBlackBgSubst = 0;
					page_atrb[index].bg = background;
					page_atrb[index].IgnoreAtBlackBgSubst = IgnoreAtBlackBgSubst;
					break;

				case hold_mosaic:
					hold = 1;
					break;

				case release_mosaic:
					hold = 2;
					break;
				}

				/* handle spacing attributes */
				if (hold && (page_atrb[index].charset == C_G1C || page_atrb[index].charset == C_G1S))
					page_char[index] = held_mosaic;
				else
					page_char[index] = ' ';

				if (hold == 2)
					hold = 0;
			}
			else /* char >= ' ' */
			{
				mosaic_pending = 0; // charset will be switched next if esc_pending
				/* set new held-mosaic char */
				if ((charset == C_G1C || charset == C_G1S) &&
					 ((page_char[index]&0xA0) == 0x20))
					held_mosaic = page_char[index];
				if (page_atrb[index].doubleh)
					page_char[index + 40] = 0xFF;

			}
			if (!(charset == C_G1C || charset == C_G1S))
				held_mosaic = ' '; /* forget if outside mosaic */

		} /* for col */

		/* skip row if doubleheight */
		if (row < 23 && dhset)
		{
			for (col = 0; col < 40; col++)
			{
				int index = row*40 + col;
				page_atrb[index+40].bg = page_atrb[index].bg;
				page_atrb[index+40].fg = tuxtxt_color_white;
				if (!page_atrb[index].doubleh)
					page_char[index+40] = ' ';
				page_atrb[index+40].flashing = 0;
				page_atrb[index+40].charset = C_G0P;
				page_atrb[index+40].doubleh = 0;
				page_atrb[index+40].doublew = 0;
				page_atrb[index+40].IgnoreAtBlackBgSubst = 0;
				page_atrb[index+40].concealed = 0;
				page_atrb[index+40].flashing  = 0;
				page_atrb[index+40].boxwin    = page_atrb[index].boxwin;
			}
			row++;
		}
	} /* for row */
	tuxtxt_cache.FullScrColor = tuxtxt_color_black;

	if (showl25)
		tuxtxt_eval_l25(page_char,page_atrb, hintmode);


	/* handle Black Background Color Substitution and transparency (CLUT1#0) */
	{
		int r, c;
		int o = 0;
		char bitmask ;



		for (r = 0; r < 25; r++)
		{
			for (c = 0; c < 40; c++)
			{
				bitmask = (page_atrb[o].bg == 0x08 ? 0x08 : 0x00) | (tuxtxt_cache.FullRowColor[r] == 0x08 ? 0x04 : 0x00) | (page_atrb[o].boxwin <<1) | boxed;
				switch (bitmask)
				{
					case 0x08:
					case 0x0b:
						if (tuxtxt_cache.FullRowColor[r] == 0x08)
							page_atrb[o].bg = tuxtxt_cache.FullScrColor;
						else
							page_atrb[o].bg = tuxtxt_cache.FullRowColor[r];
						break;
					case 0x01:
					case 0x05:
					case 0x09:
					case 0x0a:
					case 0x0c:
					case 0x0d:
					case 0x0e:
					case 0x0f:
						page_atrb[o].bg = tuxtxt_color_transp;
						break;
				}
				bitmask = (page_atrb[o].fg  == 0x08 ? 0x08 : 0x00) | (tuxtxt_cache.FullRowColor[r] == 0x08 ? 0x04 : 0x00) | (page_atrb[o].boxwin <<1) | boxed;
				switch (bitmask)
				{
					case 0x08:
					case 0x0b:
						if (tuxtxt_cache.FullRowColor[r] == 0x08)
							page_atrb[o].fg = tuxtxt_cache.FullScrColor;
						else
							page_atrb[o].fg = tuxtxt_cache.FullRowColor[r];
						break;
					case 0x01:
					case 0x05:
					case 0x09:
					case 0x0a:
					case 0x0c:
					case 0x0d:
					case 0x0e:
					case 0x0f:
						page_atrb[o].fg = tuxtxt_color_transp;
						break;
				}
				o++;
			}
		}
	}
	return pageinfo;
}
void tuxtxt_FillRect(unsigned char *lfb, int xres, int x, int y, int w, int h, int color)
{
	if (!lfb) return;
	unsigned char *p = lfb + x + y * xres;

	if (w > 0)
		for ( ; h > 0 ; h--)
		{
			memset(p, color, w);
			p += xres;
		}
}

void tuxtxt_RenderDRCS(int xres,
	unsigned char *s,	/* pointer to char data, parity undecoded */
	unsigned char *d,	/* pointer to frame buffer of top left pixel */
	unsigned char *ax, /* array[0..12] of x-offsets, array[0..10] of y-offsets for each pixel */
	unsigned char fgcolor, unsigned char bgcolor)
{
	if (d == NULL) return;
	int bit, x, y;
	unsigned char *ay = ax + 13; /* array[0..10] of y-offsets for each pixel */

	for (y = 0; y < 10; y++) /* 10*2 bytes a 6 pixels per char definition */
	{
		unsigned char c1 = deparity[*s++];
		unsigned char c2 = deparity[*s++];
		int h = ay[y+1] - ay[y];

		if (!h)
			continue;
		if (((c1 == ' ') && (*(s-2) != ' ')) || ((c2 == ' ') && (*(s-1) != ' '))) /* parity error: stop decoding FIXME */
			return;
		for (bit = 0x20, x = 0;
			  bit;
			  bit >>= 1, x++)	/* bit mask (MSB left), column counter */
		{
			int i, f1, f2;

			f1 = (c1 & bit) ? fgcolor : bgcolor;
			f2 = (c2 & bit) ? fgcolor : bgcolor;
			for (i = 0; i < h; i++)
			{
				if (ax[x+1] > ax[x])
					memset(d + ax[x], f1, ax[x+1] - ax[x]);
				if (ax[x+7] > ax[x+6])
					memset(d + ax[x+6], f2, ax[x+7] - ax[x+6]); /* 2nd byte 6 pixels to the right */
				d += xres;
			}
			d -= h * xres;
		}
		d += h * xres;
	}
}


void tuxtxt_DrawVLine(unsigned char *lfb, int xres, int x, int y, int l, int color)
{
	if (!lfb) return;
	unsigned char *p = lfb + x + y * xres;

	for ( ; l > 0 ; l--)
	{
		*p = color;
		p += xres;
	}
}

void tuxtxt_DrawHLine(unsigned char* lfb,int xres,int x, int y, int l, int color)
{
	if (!lfb) return;
	if (l > 0)
		memset(lfb + x + y * xres, color, l);
}

void tuxtxt_FillRectMosaicSeparated(unsigned char *lfb, int xres,int x, int y, int w, int h, int fgcolor, int bgcolor, int set)
{
	if (!lfb) return;
	tuxtxt_FillRect(lfb,xres,x, y, w, h, bgcolor);
	if (set)
	{
		tuxtxt_FillRect(lfb,xres,x+1, y+1, w-2, h-2, fgcolor);
	}
}

void tuxtxt_FillTrapez(unsigned char *lfb, int xres,int x0, int y0, int l0, int xoffset1, int h, int l1, int color)
{
	unsigned char *p = lfb + x0 + y0 * xres;
	int xoffset, l;
	int yoffset;

	for (yoffset = 0; yoffset < h; yoffset++)
	{
		l = l0 + ((l1-l0) * yoffset + h/2) / h;
		xoffset = (xoffset1 * yoffset + h/2) / h;
		if (l > 0)
			memset(p + xoffset, color, l);
		p += xres;
	}
}
void tuxtxt_FlipHorz(unsigned char *lfb, int xres,int x, int y, int w, int h)
{
	unsigned char buf[w];
	unsigned char *p = lfb + x + y * xres;
	int w1,h1;

	for (h1 = 0 ; h1 < h ; h1++)
	{
		memcpy(buf,p,w);
		for (w1 = 0 ; w1 < w ; w1++)
		{
			*(p+w1) = buf[w-(w1+1)];
		}
		p += xres;
	}
}
void tuxtxt_FlipVert(unsigned char *lfb, int xres,int x, int y, int w, int h)
{
	unsigned char buf[w];
	unsigned char *p = lfb + x + y * xres, *p1, *p2;
	int h1;

	for (h1 = 0 ; h1 < h/2 ; h1++)
	{
		p1 = (p+(h1*xres));
		p2 = (p+(h-(h1+1))*xres);
		memcpy(buf,p1,w);
		memcpy(p1,p2,w);
		memcpy(p2,buf,w);
	}
}

int tuxtxt_ShapeCoord(int param, int curfontwidth, int curfontheight)
{
	switch (param)
	{
	case S_W13:
		return curfontwidth/3;
	case S_W12:
		return curfontwidth/2;
	case S_W23:
		return curfontwidth*2/3;
	case S_W11:
		return curfontwidth;
	case S_WM3:
		return curfontwidth-3;
	case S_H13:
		return curfontheight/3;
	case S_H12:
		return curfontheight/2;
	case S_H23:
		return curfontheight*2/3;
	case S_H11:
		return curfontheight;
	default:
		return param;
	}
}

void tuxtxt_DrawShape(unsigned char *lfb, int xres,int x, int y, int shapenumber, int curfontwidth, int fontheight, int curfontheight, int fgcolor, int bgcolor, int clear)
{
	if (!lfb || shapenumber < 0x20 || shapenumber > 0x7e || (shapenumber == 0x7e && clear))
		return;

	unsigned char *p = aShapes[shapenumber - 0x20];

	if (*p == S_INV)
	{
		int t = fgcolor;
		fgcolor = bgcolor;
		bgcolor = t;
		p++;
	}

	if (clear)
		tuxtxt_FillRect(lfb,xres,x, y, curfontwidth, fontheight, bgcolor);
	while (*p != S_END)
		switch (*p++)
		{
		case S_FHL:
		{
			int offset = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			tuxtxt_DrawHLine(lfb,xres,x, y + offset, curfontwidth, fgcolor);
			break;
		}
		case S_FVL:
		{
			int offset = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			tuxtxt_DrawVLine(lfb,xres,x + offset, y, fontheight, fgcolor);
			break;
		}
		case S_FLH:
			tuxtxt_FlipHorz(lfb,xres,x,y,curfontwidth, fontheight);
			break;
		case S_FLV:
			tuxtxt_FlipVert(lfb,xres,x,y,curfontwidth, fontheight);
			break;
		case S_BOX:
		{
			int xo = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int yo = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int w = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int h = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			tuxtxt_FillRect(lfb,xres,x + xo, y + yo, w, h, fgcolor);
			break;
		}
		case S_TRA:
		{
			int x0 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int y0 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int l0 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int x1 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int y1 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int l1 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			tuxtxt_FillTrapez(lfb,xres,x + x0, y + y0, l0, x1-x0, y1-y0, l1, fgcolor);
			break;
		}
		case S_BTR:
		{
			int x0 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int y0 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int l0 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int x1 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int y1 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			int l1 = tuxtxt_ShapeCoord(*p++, curfontwidth, curfontheight);
			tuxtxt_FillTrapez(lfb,xres,x + x0, y + y0, l0, x1-x0, y1-y0, l1, bgcolor);
			break;
		}
		case S_LNK:
		{
			tuxtxt_DrawShape(lfb,xres,x, y, tuxtxt_ShapeCoord(*p, curfontwidth, curfontheight), curfontwidth, fontheight, curfontheight, fgcolor, bgcolor, 0);
			//p = aShapes[ShapeCoord(*p, curfontwidth, curfontheight) - 0x20];
			break;
		}
		default:
			break;
		}
		
}


/******************************************************************************
 * RenderChar                                                                 *
 ******************************************************************************/

int tuxtxt_RenderChar(unsigned char *lfb, // pointer to render buffer, min. fontheight*2*xres
		      int xres,// length of 1 line in render buffer
		      int Char,// character to render
		      int *pPosX,// left border for rendering relative to *lfb, will be set to right border after rendering
		      int PosY,// vertical position of char in *lfb
		      tstPageAttr *Attribute,// Attributes of Char
		      int zoom,// 1= character will be rendered in double height
		      int curfontwidth,// rendering width of character
		      int curfontwidth2,// rendering width of next character (needed for doublewidth)
		      int fontheight,// height of character
		      int transpmode,// 1= transparent display
		      unsigned char *axdrcs,// width and height of DRCS-chars
		      int ascender // ascender of font
		      )
{
	int Row;
	int bgcolor, fgcolor;
	int factor, xfactor;
	int national_subset_local = tuxtxt_cache.national_subset;
	int ymosaic[4];
	ymosaic[0] = 0; /* y-offsets for 2*3 mosaic */
	ymosaic[1] = (fontheight + 1) / 3;
	ymosaic[2] = (fontheight * 2 + 1) / 3;
	ymosaic[3] = fontheight;


	if (Attribute->setX26)
	{
		national_subset_local = 0; // no national subset
	}

	// G0+G2 set designation
	if (Attribute->setG0G2 != 0x3f)
	{
		switch (Attribute->setG0G2)
		{
			case 0x20 :
				national_subset_local = NAT_SC;
				break;				
			case 0x24 :
				national_subset_local = NAT_RB;
				break;				
			case 0x25 :
				national_subset_local = NAT_UA;
				break;
			case 0x37:
				national_subset_local = NAT_GR;
				break;
			case 0x55:
				national_subset_local = NAT_HB;
				break;
			case 0x47:
			case 0x57:
				national_subset_local = NAT_AR; 
				break;
			default:
				national_subset_local = countryconversiontable[Attribute->setG0G2 & 0x07];
				break;
		}
	}
	if (Attribute->charset == C_G0S) // use secondary charset
		national_subset_local = tuxtxt_cache.national_subset_secondary;
	if (zoom && Attribute->doubleh)
		factor = 4;
	else if (zoom || Attribute->doubleh)
		factor = 2;
	else
		factor = 1;

	if (Attribute->doublew)
	{
		curfontwidth += curfontwidth2;
		xfactor = 2;
	}
	else
		xfactor = 1;

	if (Char == 0xFF)	/* skip doubleheight chars in lower line */
	{
		*pPosX += curfontwidth;
		return -1;
	}

	/* get colors */
	if (Attribute->inverted)
	{
		int t = Attribute->fg;
		Attribute->fg = Attribute->bg;
		Attribute->bg = t;
	}
	fgcolor = Attribute->fg;
	if (transpmode == 1 && PosY < 24*fontheight)
	{
		if (fgcolor == tuxtxt_color_transp) /* outside boxed elements (subtitles, news) completely transparent */
			bgcolor = tuxtxt_color_transp;
		else
			bgcolor = tuxtxt_color_transp2;
	}
	else
		bgcolor = Attribute->bg;

	/* handle mosaic */
	if ((Attribute->charset == C_G1C || Attribute->charset == C_G1S) &&
		 ((Char&0xA0) == 0x20))
	{
		int w1 = (curfontwidth / 2 ) *xfactor;
		int w2 = (curfontwidth - w1) *xfactor;
		int y;

		Char = (Char & 0x1f) | ((Char & 0x40) >> 1);
		if (Attribute->charset == C_G1S) /* separated mosaic */
			for (y = 0; y < 3; y++)
			{
				tuxtxt_FillRectMosaicSeparated(lfb,xres,*pPosX,      PosY +  ymosaic[y]*factor, w1, (ymosaic[y+1] - ymosaic[y])*factor, fgcolor, bgcolor, Char & 0x01);
				tuxtxt_FillRectMosaicSeparated(lfb,xres,*pPosX + w1, PosY +  ymosaic[y]*factor, w2, (ymosaic[y+1] - ymosaic[y])*factor, fgcolor, bgcolor, Char & 0x02);
				Char >>= 2;
			}
		else
			for (y = 0; y < 3; y++)
			{
				tuxtxt_FillRect(lfb,xres,*pPosX,      PosY + ymosaic[y]*factor, w1, (ymosaic[y+1] - ymosaic[y])*factor, (Char & 0x01) ? fgcolor : bgcolor);
				tuxtxt_FillRect(lfb,xres,*pPosX + w1, PosY + ymosaic[y]*factor, w2, (ymosaic[y+1] - ymosaic[y])*factor, (Char & 0x02) ? fgcolor : bgcolor);
				Char >>= 2;
			}

		*pPosX += curfontwidth;
		return 0;;
	}

	if (Attribute->charset == C_G3)
	{
		if (Char < 0x20 || Char > 0x7d)
		{
			Char = 0x20;
		}
		else
		{
			if (*aShapes[Char - 0x20] == S_CHR)
			{
				unsigned char *p = aShapes[Char - 0x20];
				Char = (*(p+1) <<8) + (*(p+2));
			}
			else if (*aShapes[Char - 0x20] == S_ADT)
			{
				if (lfb) 
				{
					int x,y,f,c;
					unsigned char* p = lfb + *pPosX + PosY* xres;
					for (y=0; y<fontheight;y++)
					{
						for (f=0; f<factor; f++)
						{
							for (x=0; x<curfontwidth*xfactor;x++)
							{
								c = (y&4 ? (x/3)&1 :((x+3)/3)&1);
								*(p+x) = (c ? fgcolor : bgcolor);
							}
							p += xres;
						}
					}
				}
				*pPosX += curfontwidth;
				return 0;
			}
			else
			{
				tuxtxt_DrawShape(lfb,xres,*pPosX, PosY, Char, curfontwidth, fontheight, factor*fontheight, fgcolor, bgcolor,1);
				*pPosX += curfontwidth;
				return 0;
			}
		}
	}
	else if (Attribute->charset >= C_OFFSET_DRCS)
	{

		tstCachedPage *pcache = tuxtxt_cache.astCachetable[(Attribute->charset & 0x10) ? tuxtxt_cache.drcs : tuxtxt_cache.gdrcs][Attribute->charset & 0x0f];
		if (pcache)
		{
			unsigned char drcs_data[23*40];
			tuxtxt_decompress_page((Attribute->charset & 0x10) ? tuxtxt_cache.drcs : tuxtxt_cache.gdrcs,Attribute->charset & 0x0f,drcs_data);
			unsigned char *p;
			if (Char < 23*2)
				p = drcs_data + 20*Char;
			else if (pcache->pageinfo.p24)
				p = pcache->pageinfo.p24 + 20*(Char - 23*2);
			else
			{
				tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth, factor*fontheight, bgcolor);
				*pPosX += curfontwidth;
				return 0;
			}
			axdrcs[12] = curfontwidth; /* adjust last x-offset according to position, FIXME: double width */
			tuxtxt_RenderDRCS(xres,p,
						  lfb + *pPosX + PosY * xres,
						  axdrcs, fgcolor, bgcolor);
		}
		else
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth, factor*fontheight, bgcolor);
		*pPosX += curfontwidth;
		return 0;
	}
	else if (Attribute->charset == C_G2 && Char >= 0x20 && Char <= 0x7F)
	{
		if ((national_subset_local == NAT_SC) || (national_subset_local == NAT_RB) || (national_subset_local == NAT_UA))
			Char = G2table[1][Char-0x20];
		else if (national_subset_local == NAT_GR)
			Char = G2table[2][Char-0x20];
		else if (national_subset_local == NAT_AR)
			Char = G2table[3][Char-0x20];
		else
			Char = G2table[0][Char-0x20];

		//if (Char == 0x7F)
		//{
		//	tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth, factor*ascender, fgcolor);
		//	tuxtxt_FillRect(lfb,xres,*pPosX, PosY + factor*ascender, curfontwidth, factor*(fontheight-ascender), bgcolor);
		//	*pPosX += curfontwidth;
		//	return 0;
		//}
	}
	else if (national_subset_local == NAT_SC && Char >= 0x20 && Char <= 0x7F) /* remap complete areas for serbian/croatian */
		Char = G0table[0][Char-0x20];
	else if (national_subset_local == NAT_RB && Char >= 0x20 && Char <= 0x7F) /* remap complete areas for russian/bulgarian */
		Char = G0table[1][Char-0x20];
	else if (national_subset_local == NAT_UA && Char >= 0x20 && Char <= 0x7F) /* remap complete areas for ukrainian */
		Char = G0table[2][Char-0x20];
	else if (national_subset_local == NAT_GR && Char >= 0x20 && Char <= 0x7F) /* remap complete areas for greek */
		Char = G0table[3][Char-0x20];
	else if (national_subset_local == NAT_HB && Char >= 0x20 && Char <= 0x7F) /* remap complete areas for hebrew */
		Char = G0table[4][Char-0x20];
	else if (national_subset_local == NAT_AR && Char >= 0x20 && Char <= 0x7F) /* remap complete areas for arabic */
		Char = G0table[5][Char-0x20];
	else
	{
		/* load char */
		switch (Char)
		{
		case 0x00:
		case 0x20:
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth, factor*fontheight, bgcolor);
			*pPosX += curfontwidth;
			return -3;
		case 0x23:
		case 0x24:
			Char = nationaltable23[national_subset_local][Char-0x23];
			break;
		case 0x40:
			Char = nationaltable40[national_subset_local];
			break;
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x5E:
		case 0x5F:
		case 0x60:
			Char = nationaltable5b[national_subset_local][Char-0x5B];
			break;
		case 0x7B:
		case 0x7C:
		case 0x7D:
		case 0x7E:
			Char = nationaltable7b[national_subset_local][Char-0x7B];
			break;
		case 0x7F:
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY , curfontwidth, factor*ascender, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY + factor*ascender, curfontwidth, factor*(fontheight-ascender), bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE0: /* |- */
			tuxtxt_DrawHLine(lfb,xres,*pPosX, PosY, curfontwidth, fgcolor);
			tuxtxt_DrawVLine(lfb,xres,*pPosX, PosY +1, fontheight -1, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX +1, PosY +1, curfontwidth-1, fontheight-1, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE1: /* - */
			tuxtxt_DrawHLine(lfb,xres,*pPosX, PosY, curfontwidth, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY +1, curfontwidth, fontheight-1, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE2: /* -| */
			tuxtxt_DrawHLine(lfb,xres,*pPosX, PosY, curfontwidth, fgcolor);
			tuxtxt_DrawVLine(lfb,xres,*pPosX + curfontwidth -1, PosY +1, fontheight -1, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY +1, curfontwidth-1, fontheight-1, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE3: /* |  */
			tuxtxt_DrawVLine(lfb,xres,*pPosX, PosY, fontheight, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX +1, PosY, curfontwidth -1, fontheight, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE4: /*  | */
			tuxtxt_DrawVLine(lfb,xres,*pPosX + curfontwidth -1, PosY, fontheight, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth -1, fontheight, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE5: /* |_ */
			tuxtxt_DrawHLine(lfb,xres,*pPosX, PosY + fontheight -1, curfontwidth, fgcolor);
			tuxtxt_DrawVLine(lfb,xres,*pPosX, PosY, fontheight -1, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX +1, PosY, curfontwidth-1, fontheight-1, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE6: /* _ */
			tuxtxt_DrawHLine(lfb,xres,*pPosX, PosY + fontheight -1, curfontwidth, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth, fontheight-1, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE7: /* _| */
			tuxtxt_DrawHLine(lfb,xres,*pPosX, PosY + fontheight -1, curfontwidth, fgcolor);
			tuxtxt_DrawVLine(lfb,xres,*pPosX + curfontwidth -1, PosY, fontheight -1, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth-1, fontheight-1, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE8: /* Ii */
			tuxtxt_FillRect(lfb,xres,*pPosX +1, PosY, curfontwidth -1, fontheight, bgcolor);
			for (Row=0; Row < curfontwidth/2; Row++)
				tuxtxt_DrawVLine(lfb,xres,*pPosX + Row, PosY + Row, fontheight - Row, fgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xE9: /* II */
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth/2, fontheight, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX + curfontwidth/2, PosY, (curfontwidth+1)/2, fontheight, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xEA: /* °  */
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth, fontheight, bgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth/2, curfontwidth/2, fgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xEB: /* ¬ */
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY +1, curfontwidth, fontheight -1, bgcolor);
			for (Row=0; Row < curfontwidth/2; Row++)
				tuxtxt_DrawHLine(lfb,xres,*pPosX + Row, PosY + Row, curfontwidth - Row, fgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xEC: /* -- */
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth, curfontwidth/2, fgcolor);
			tuxtxt_FillRect(lfb,xres,*pPosX, PosY + curfontwidth/2, curfontwidth, fontheight - curfontwidth/2, bgcolor);
			*pPosX += curfontwidth;
			return 0;
		case 0xED:
		case 0xEE:
		case 0xEF:
		case 0xF0:
		case 0xF1:
		case 0xF2:
		case 0xF3:
		case 0xF4:
		case 0xF5:
		case 0xF6:
			Char = arrowtable[Char - 0xED];
			break;
		default:
			break;
		}
	}
	if (Char <= 0x20)
	{
#if TUXTXT_DEBUG
		printf("TuxTxt found control char: %x \"%c\" \n", Char, Char);
#endif
		tuxtxt_FillRect(lfb,xres,*pPosX, PosY, curfontwidth, factor*fontheight, bgcolor);
		*pPosX += curfontwidth;
		return -2;
	}
	return Char; // Char is an alphanumeric unicode character
}
/************************************************************************
	Everything needed to render a Teletext page to the Framebuffer
	Usage:
	- Create tstRenderInfo-Object o
	- Set Default settings by calling tuxtxt_SetRenderingDefaults
	- Fill o with your Settings (especially fill fb member and screen positions sx,ex,sy,ey)
	- Call tuxtxt_InitRendering with o
	- for every page you wish to display
		- Call tuxtxt_DecodePage to receive o.pageinfo
		- Call tuxtxt_RenderPage with o
	- Call tuxtxt_EndRendering with o
*************************************************************************/

/* devices */
#define AVS "/dev/dbox/avs0"
#define SAA "/dev/dbox/saa0"
#if HAVE_DVB_API_VERSION < 3
#define PIG "/dev/dbox/pig0"
#else
#define PIG "/dev/v4l/video0"
#endif

#define TOPMENUSTARTX TV43STARTX+2
#define TOPMENUENDX TVENDX
#define TOPMENUSTARTY renderinfo->StartY
#define TOPMENUENDY TV43STARTY

#define TOPMENULINEWIDTH ((TOPMENUENDX-TOPMENU43STARTX+fontwidth_topmenusmall-1)/fontwidth_topmenusmall)
#define TOPMENUINDENTBLK 0
#define TOPMENUINDENTGRP 1
#define TOPMENUINDENTDEF 2
#define TOPMENUSPC 0
#define TOPMENUCHARS (TOPMENUINDENTDEF+12+TOPMENUSPC+4)

#define TV43STARTX (renderinfo->ex - 146) //(renderinfo->StartX + 2 + (40-renderinfo->nofirst)*renderinfo->fontwidth_topmenumain + (40*renderinfo->fontwidth_topmenumain/abx))
#define TV169FULLSTARTX (renderinfo->sx+ 8*40) //(renderinfo->sx +(renderinfo->ex +1 - renderinfo->sx)/2)
#define TVENDX renderinfo->ex
#define TVENDY (renderinfo->StartY + 25*renderinfo->fontheight)
#define TV43WIDTH 144 /* 120 */
#define TV43HEIGHT 116 /* 96 */
#define TV43STARTY (TVENDY - TV43HEIGHT)
#define TV169FULLSTARTY renderinfo->sy
#define TV169FULLWIDTH  (renderinfo->ex - renderinfo->sx)/2
#define TV169FULLHEIGHT (renderinfo->ey - renderinfo->sy)

/* fonts */
#define TUXTXTTTF FONTDIR "/tuxtxt.ttf"
#define TUXTXTOTB FONTDIR "/tuxtxt.otb"
/* alternative fontdir */
#define TUXTXTTTFVAR "/var/tuxtxt/tuxtxt.ttf"
#define TUXTXTOTBVAR "/var/tuxtxt/tuxtxt.otb"

int tuxtxt_toptext_getnext(int startpage, int up, int findgroup)
{
	int current, nextgrp, nextblk;

	int stoppage =  (tuxtxt_is_dec(startpage) ? startpage : startpage & 0xF00); // avoid endless loop in hexmode
	nextgrp = nextblk = 0;
	current = startpage;

	do {
		if (up)
			tuxtxt_next_dec(&current);
		else
			tuxtxt_prev_dec(&current);

		if (!tuxtxt_cache.bttok || tuxtxt_cache.basictop[current]) /* only if existent */
		{
			if (findgroup)
			{
				if (tuxtxt_cache.basictop[current] >= 6 && tuxtxt_cache.basictop[current] <= 7)
					return current;
				if (!nextgrp && (current&0x00F) == 0)
					nextgrp = current;
			}
			if (tuxtxt_cache.basictop[current] >= 2 && tuxtxt_cache.basictop[current] <= 5) /* always find block */
				return current;

			if (!nextblk && (current&0x0FF) == 0)
				nextblk = current;
		}
	} while (current != stoppage);

	if (nextgrp)
		return nextgrp;
	else if (nextblk)
		return nextblk;
	else
		return current;
}

void tuxtxt_FillBorder(tstRenderInfo* renderinfo, int color)
{
	int ys =  renderinfo->var_screeninfo.yres-renderinfo->var_screeninfo.yoffset;
	tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,0     , ys                     ,renderinfo->StartX      ,renderinfo->var_screeninfo.yres                       ,color);
	tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->StartX, ys                     ,renderinfo->displaywidth,renderinfo->StartY                                    ,color);
	tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->StartX, ys+renderinfo->StartY+25*renderinfo->fontheight,renderinfo->displaywidth,renderinfo->var_screeninfo.yres-(renderinfo->StartY+25*renderinfo->fontheight),color);

	if (renderinfo->screenmode == 0 )
		tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->StartX+renderinfo->displaywidth, ys,renderinfo->var_screeninfo.xres-(renderinfo->StartX+renderinfo->displaywidth),renderinfo->var_screeninfo.yres   ,color);
}


void tuxtxt_setfontwidth(tstRenderInfo* renderinfo,int newwidth)
{
	if (renderinfo->fontwidth != newwidth)
	{
		int i;
		renderinfo->fontwidth = newwidth;
		if (renderinfo->usettf)
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
			renderinfo->typettf.width  = (FT_UShort) renderinfo->fontwidth * renderinfo->TTFWidthFactor16 / 16;
#else
			renderinfo->typettf.font.pix_width  = (FT_UShort) renderinfo->fontwidth * renderinfo->TTFWidthFactor16 / 16;
#endif
		else
		{
			if (newwidth < 11)
				newwidth = 21;
			else if (newwidth < 14)
				newwidth = 22;
			else
				newwidth = 23;
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
			renderinfo->typettf.width  = renderinfo->typettf.height = (FT_UShort) newwidth;
#else
			renderinfo->typettf.font.pix_width  = renderinfo->typettf.font.pix_height = (FT_UShort) newwidth;
#endif
		}
		for (i = 0; i <= 12; i++)
			renderinfo->axdrcs[i] = (renderinfo->fontwidth * i + 6) / 12;
	}
}
void tuxtxt_ClearBB(tstRenderInfo* renderinfo,int color)
{
	memset(renderinfo->lfb + (renderinfo->var_screeninfo.yres-renderinfo->var_screeninfo.yoffset )*renderinfo->var_screeninfo.xres, color, renderinfo->var_screeninfo.xres*renderinfo->var_screeninfo.yres);
}

void tuxtxt_ClearFB(tstRenderInfo* renderinfo,int color)
{
	memset(renderinfo->lfb + renderinfo->var_screeninfo.xres*renderinfo->var_screeninfo.yoffset, color, renderinfo->var_screeninfo.xres*renderinfo->var_screeninfo.yres);
}

int  tuxtxt_GetCurFontWidth(tstRenderInfo* renderinfo)
{
	int mx = (renderinfo->displaywidth)%(40-renderinfo->nofirst); // # of unused pixels
	int abx = (mx == 0 ? renderinfo->displaywidth+1 : (renderinfo->displaywidth)/(mx+1));// distance between 'inserted' pixels
	int nx= abx+1-((renderinfo->PosX-renderinfo->sx) % (abx+1)); // # of pixels to next insert
	return renderinfo->fontwidth+(((renderinfo->PosX+renderinfo->fontwidth+1-renderinfo->sx) <= renderinfo->displaywidth && nx <= renderinfo->fontwidth+1) ? 1 : 0);
}

void tuxtxt_SetPosX(tstRenderInfo* renderinfo, int column)
{
		renderinfo->PosX = renderinfo->StartX;
		int i;
		for (i = 0; i < column-renderinfo->nofirst; i++)
			renderinfo->PosX += tuxtxt_GetCurFontWidth(renderinfo);
}

/******************************************************************************
 * RenderChar                                                                 *
 ******************************************************************************/

void tuxtxt_RenderCharIntern(tstRenderInfo* renderinfo,int Char, tstPageAttr *Attribute, int zoom, int yoffset)
{
	int Row, Pitch, Bit;
	int error, glyph;
	int bgcolor, fgcolor;
	int factor, xfactor;
	int national_subset_local = tuxtxt_cache.national_subset;
	unsigned char *sbitbuffer;

	int curfontwidth = tuxtxt_GetCurFontWidth(renderinfo);
	int t = curfontwidth;
	renderinfo->PosX += t;
	int curfontwidth2 = tuxtxt_GetCurFontWidth(renderinfo);
	renderinfo->PosX -= t;
	int alphachar = tuxtxt_RenderChar(renderinfo->lfb+(yoffset+renderinfo->StartY)*renderinfo->var_screeninfo.xres,  renderinfo->var_screeninfo.xres,Char, &renderinfo->PosX, renderinfo->PosY-renderinfo->StartY, Attribute, zoom, curfontwidth, curfontwidth2, renderinfo->fontheight, renderinfo->transpmode,renderinfo->axdrcs, renderinfo->ascender);
	if (alphachar <= 0) return;

	if (zoom && Attribute->doubleh)
		factor = 4;
	else if (zoom || Attribute->doubleh)
		factor = 2;
	else
		factor = 1;

	fgcolor = Attribute->fg;
	if (renderinfo->transpmode == 1 && renderinfo->PosY < renderinfo->StartY + 24*renderinfo->fontheight)
	{
		if (fgcolor == tuxtxt_color_transp) /* outside boxed elements (subtitles, news) completely transparent */
			bgcolor = tuxtxt_color_transp;
		else
			bgcolor = tuxtxt_color_transp2;
	}
	else
		bgcolor = Attribute->bg;
	if (Attribute->doublew)
	{
		curfontwidth += curfontwidth2;
		xfactor = 2;
	}
	else
		xfactor = 1;

	if (!(glyph = FT_Get_Char_Index(renderinfo->face, alphachar)))
	{
#if TUXTXT_DEBUG
		printf("TuxTxt <FT_Get_Char_Index for Char %x \"%c\" failed\n", alphachar, alphachar);
#endif
		tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->PosX, renderinfo->PosY + yoffset, curfontwidth, factor*renderinfo->fontheight, bgcolor);
		renderinfo->PosX += curfontwidth;
		return;
	}

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	if ((error = FTC_SBitCache_Lookup(renderinfo->cache, &renderinfo->typettf, glyph, &renderinfo->sbit, NULL)) != 0)
#else
	if ((error = FTC_SBit_Cache_Lookup(renderinfo->cache, &renderinfo->typettf, glyph, &renderinfo->sbit)) != 0)
#endif
	{
#if TUXTXT_DEBUG
		printf("TuxTxt <FTC_SBitCache_Lookup: 0x%x> c%x a%x g%x w%d h%d x%d y%d\n",
				 error, alphachar, Attribute, glyph, curfontwidth, renderinfo->fontheight, renderinfo->PosX, renderinfo->PosY);
#endif
		tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->PosX, renderinfo->PosY + yoffset, curfontwidth, renderinfo->fontheight, bgcolor);
		renderinfo->PosX += curfontwidth;
		return;
	}

	/* render char */
	sbitbuffer = renderinfo->sbit->buffer;
	char localbuffer[1000]; // should be enough to store one character-bitmap...
	// add diacritical marks
	if (Attribute->diacrit)
	{
		FTC_SBit        sbit_diacrit;

		if ((national_subset_local == NAT_SC) || (national_subset_local == NAT_RB) || (national_subset_local == NAT_UA))
			Char = G2table[1][0x20+ Attribute->diacrit];
		else if (national_subset_local == NAT_GR)
			Char = G2table[2][0x20+ Attribute->diacrit];
		else if (national_subset_local == NAT_HB)
			Char = G2table[3][0x20+ Attribute->diacrit];
		else if (national_subset_local == NAT_AR)
			Char = G2table[4][0x20+ Attribute->diacrit];
		else
			Char = G2table[0][0x20+ Attribute->diacrit];
		if ((glyph = FT_Get_Char_Index(renderinfo->face, Char)))
		{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
			if ((error = FTC_SBitCache_Lookup(renderinfo->cache, &renderinfo->typettf, glyph, &sbit_diacrit, NULL)) == 0)
#else
			if ((error = FTC_SBit_Cache_Lookup(renderinfo->cache, &renderinfo->typettf, glyph, &sbit_diacrit)) == 0)
#endif
			{
					sbitbuffer = localbuffer;
					memcpy(sbitbuffer,renderinfo->sbit->buffer,renderinfo->sbit->pitch*renderinfo->sbit->height);

					for (Row = 0; Row < renderinfo->sbit->height; Row++)
					{
						for (Pitch = 0; Pitch < renderinfo->sbit->pitch; Pitch++)
						{
							if (sbit_diacrit->pitch > Pitch && sbit_diacrit->height > Row)
								sbitbuffer[Row*renderinfo->sbit->pitch+Pitch] |= sbit_diacrit->buffer[Row*renderinfo->sbit->pitch+Pitch];
						}
					}
				}
			}
		}

    int backupTTFshiftY = renderinfo->TTFShiftY;
    if (national_subset_local == NAT_AR) 
        renderinfo->TTFShiftY = backupTTFshiftY - 2; // for arabic TTF font should be shifted up slightly

		unsigned char *p;
		int f; /* running counter for zoom factor */
		int he = renderinfo->sbit->height; // sbit->height should not be altered, I guess
		Row = factor * (renderinfo->ascender - renderinfo->sbit->top + renderinfo->TTFShiftY);
		if (Row < 0)
		{
		    sbitbuffer  -= renderinfo->sbit->pitch*Row;
		    he += Row;
		    Row = 0;
		}
		else		
		    tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->PosX, renderinfo->PosY + yoffset, curfontwidth, Row, bgcolor); /* fill upper margin */

		if (renderinfo->ascender - renderinfo->sbit->top + renderinfo->TTFShiftY + he > renderinfo->fontheight)
			he = renderinfo->fontheight - renderinfo->ascender + renderinfo->sbit->top - renderinfo->TTFShiftY; /* limit char height to defined/calculated fontheight */
		if (he < 0) he = renderinfo->fontheight;

		p = renderinfo->lfb + renderinfo->PosX + (yoffset + renderinfo->PosY + Row) * renderinfo->var_screeninfo.xres; /* running pointer into framebuffer */
		for (Row = he; Row; Row--) /* row counts up, but down may be a little faster :) */
		{
			int pixtodo = (renderinfo->usettf ? renderinfo->sbit->width : curfontwidth);
			char *pstart = p;

			for (Bit = xfactor * (renderinfo->sbit->left + renderinfo->TTFShiftX); Bit > 0; Bit--) /* fill left margin */
			{
				for (f = factor-1; f >= 0; f--)
					*(p + f*renderinfo->var_screeninfo.xres) = bgcolor;
				p++;
				if (!renderinfo->usettf)
					pixtodo--;
			}

			for (Pitch = renderinfo->sbit->pitch; Pitch; Pitch--)
			{
				for (Bit = 0x80; Bit; Bit >>= 1)
				{
					int color;

					if (--pixtodo < 0)
						break;

					if (*sbitbuffer & Bit) /* bit set -> foreground */
						color = fgcolor;
					else /* bit not set -> background */
						color = bgcolor;

					for (f = factor-1; f >= 0; f--)
						*(p + f*renderinfo->var_screeninfo.xres) = color;
					p++;

					if (xfactor > 1) /* double width */
					{
						for (f = factor-1; f >= 0; f--)
							*(p + f*renderinfo->var_screeninfo.xres) = color;
						p++;
						if (!renderinfo->usettf)
							pixtodo--;
					}
				}
				sbitbuffer++;
			}
			for (Bit = (renderinfo->usettf ? (curfontwidth - xfactor*(renderinfo->sbit->width + renderinfo->sbit->left + renderinfo->TTFShiftX)) : pixtodo);
				  Bit > 0; Bit--) /* fill rest of char width */
			{
				for (f = factor-1; f >= 0; f--)
					*(p + f*renderinfo->var_screeninfo.xres) = bgcolor;
				p++;
			}

			p = pstart + factor*renderinfo->var_screeninfo.xres;
		}

		Row = renderinfo->ascender - renderinfo->sbit->top + he + renderinfo->TTFShiftY;
		tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->PosX, renderinfo->PosY + yoffset + Row*factor, curfontwidth, (renderinfo->fontheight - Row) * factor, bgcolor); /* fill lower margin */
		if (Attribute->underline)
			tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->PosX, renderinfo->PosY + yoffset + (renderinfo->fontheight-2)* factor, curfontwidth,2*factor, fgcolor); /* underline char */

		renderinfo->PosX += curfontwidth;
		renderinfo->TTFShiftY = backupTTFshiftY; // restore TTFShiftY
}

/******************************************************************************
 * RenderCharFB                                                               *
 ******************************************************************************/

void tuxtxt_RenderCharFB(tstRenderInfo* renderinfo, int Char, tstPageAttr *Attribute)
{
	tuxtxt_RenderCharIntern(renderinfo,Char, Attribute, renderinfo->zoommode, renderinfo->var_screeninfo.yoffset);
}

/******************************************************************************
 * RenderCharBB                                                               *
 ******************************************************************************/

void tuxtxt_RenderCharBB(tstRenderInfo* renderinfo, int Char, tstPageAttr *Attribute)
{
	tuxtxt_RenderCharIntern(renderinfo, Char, Attribute, 0, renderinfo->var_screeninfo.yres-renderinfo->var_screeninfo.yoffset);
}

void tuxtxt_RenderClearMenuLineBB(tstRenderInfo* renderinfo,char *p, tstPageAttr *attrcol, tstPageAttr *attr)
{
	int col;

	renderinfo->PosX = TOPMENUSTARTX;
	tuxtxt_RenderCharBB(renderinfo,' ', attrcol); /* indicator for navigation keys */
#if 0
	tuxtxt_RenderCharBB(renderinfo,' ', attr); /* separator */
#endif
	for(col = 0; col < TOPMENUCHARS; col++)
	{
		tuxtxt_RenderCharBB(renderinfo,*p++, attr);
	}
	renderinfo->PosY += renderinfo->fontheight;
	memset(p-TOPMENUCHARS, ' ', TOPMENUCHARS); /* init with spaces */
}


/******************************************************************************
 * SwitchScreenMode                                                           *
 ******************************************************************************/

void tuxtxt_SwitchScreenMode(tstRenderInfo* renderinfo,int newscreenmode)
{
#if HAVE_DVB_API_VERSION >= 3
	struct v4l2_format format;
#endif
	/* reset transparency mode */
	if (renderinfo->transpmode)
		renderinfo->transpmode = 0;

	if (newscreenmode < 0) /* toggle mode */
		renderinfo->screenmode++;
	else /* set directly */
		renderinfo->screenmode = newscreenmode;
//	if ((screenmode > (screen_mode2 ? 2 : 1)) || (screenmode < 0))
	if ((renderinfo->screenmode > 2) || (renderinfo->screenmode < 0))
		renderinfo->screenmode = 0;

#if TUXTXT_DEBUG
	printf("TuxTxt <tuxtxt_SwitchScreenMode: %d>\n", renderinfo->screenmode);
#endif

	/* update page */
	tuxtxt_cache.pageupdate = 1;

	/* clear back buffer */
#ifndef HAVE_DREAMBOX_HARDWARE
	renderinfo->clearbbcolor = tuxtxt_color_black;
#else
	renderinfo->clearbbcolor = renderinfo->screenmode?tuxtxt_color_transp:tuxtxt_cache.FullScrColor;
#endif
	tuxtxt_ClearBB(renderinfo,renderinfo->clearbbcolor);


	/* set mode */
	if (renderinfo->screenmode)								 /* split */
	{
		tuxtxt_ClearFB(renderinfo,renderinfo->clearbbcolor);

		int fw, fh, tx, ty, tw, th;

		if (renderinfo->screenmode==1) /* split with topmenu */
		{
			fw = renderinfo->fontwidth_topmenumain;
			fh = renderinfo->fontheight;
			tw = TV43WIDTH;
			renderinfo->displaywidth= (TV43STARTX     -renderinfo->sx);
			renderinfo->StartX = renderinfo->sx; //+ (((ex-sx) - (40*fw+2+tw)) / 2); /* center screen */
			tx = TV43STARTX;
			ty = TV43STARTY;
			th = TV43HEIGHT;
#if defined BOXMODEL_DM500 || defined BOXMODEL_DM500PLUS || defined BOXMODEL_DM600PVR
			tw = renderinfo->var_screeninfo.xres/4; // DM500 seems to only like PIG sizes with same ratio
			th = renderinfo->var_screeninfo.yres/4;
#endif
		}
		else /* 2: split with full height tv picture */
		{
			fw = renderinfo->fontwidth_small;
			fh = renderinfo->fontheight;
			tx = TV169FULLSTARTX;
			ty = TV169FULLSTARTY;
			tw = TV169FULLWIDTH;
			th = TV169FULLHEIGHT;
			renderinfo->displaywidth= (TV169FULLSTARTX-renderinfo->sx);
#if defined BOXMODEL_DM500 || defined BOXMODEL_DM500PLUS || defined BOXMODEL_DM600PVR
			tw = renderinfo->var_screeninfo.xres/2; // DM500 seems to only like PIG sizes with same ratio
			th = renderinfo->var_screeninfo.yres/2;
#endif
		}
		tuxtxt_setfontwidth(renderinfo,fw);

#if HAVE_DVB_API_VERSION < 3
		avia_pig_hide(renderinfo->pig);
		avia_pig_set_pos(renderinfo->pig, tx, ty);
		avia_pig_set_size(renderinfo->pig, tw, th);
		avia_pig_set_stack(renderinfo->pig, 2);
		avia_pig_show(renderinfo->pig);
#else
		int sm = 0;
		ioctl(renderinfo->pig, VIDIOC_OVERLAY, &sm);
		sm = 1;
		ioctl(renderinfo->pig, VIDIOC_G_FMT, &format);
		format.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
		format.fmt.win.w.left   = tx;
		format.fmt.win.w.top    = ty;
		format.fmt.win.w.width  = tw;
		format.fmt.win.w.height = th;
		ioctl(renderinfo->pig, VIDIOC_S_FMT, &format);
		ioctl(renderinfo->pig, VIDIOC_OVERLAY, &sm);
#endif
		ioctl(renderinfo->avs, AVSIOSSCARTPIN8, &fncmodes[renderinfo->screen_mode2]);
		ioctl(renderinfo->saa, SAAIOSWSS, &saamodes[renderinfo->screen_mode2]);
	}
	else /* not split */
	{
#if HAVE_DVB_API_VERSION < 3
		avia_pig_hide(renderinfo->pig);
#else
		ioctl(renderinfo->pig, VIDIOC_OVERLAY, &renderinfo->screenmode);
#endif

		tuxtxt_setfontwidth(renderinfo,renderinfo->fontwidth_normal);
		renderinfo->displaywidth= (renderinfo->ex-renderinfo->sx);
		renderinfo->StartX = renderinfo->sx; //+ (ex-sx - 40*fontwidth) / 2; /* center screen */

		ioctl(renderinfo->avs, AVSIOSSCARTPIN8, &fncmodes[renderinfo->screen_mode1]);
		ioctl(renderinfo->saa, SAAIOSWSS, &saamodes[renderinfo->screen_mode1]);
	}
}

/******************************************************************************
 * CreateLine25                                                               *
 ******************************************************************************/

void tuxtxt_showlink(tstRenderInfo* renderinfo, int column, int linkpage)
{
	unsigned char *p, line[] = "   >???   ";
	int oldfontwidth = renderinfo->fontwidth;
	int yoffset;

	if (renderinfo->var_screeninfo.yoffset)
		yoffset = 0;
	else
		yoffset = renderinfo->var_screeninfo.yres;
	int abx = ((renderinfo->displaywidth)%(40-renderinfo->nofirst) == 0 ? renderinfo->displaywidth+1 : (renderinfo->displaywidth)/(((renderinfo->displaywidth)%(40-renderinfo->nofirst)))+1);// distance between 'inserted' pixels
	int width = renderinfo->displaywidth /4;

	renderinfo->PosY = renderinfo->StartY + 24*renderinfo->fontheight;

	if (renderinfo->boxed)
	{
		renderinfo->PosX = renderinfo->StartX + column*width;
		tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->PosX, renderinfo->PosY+yoffset, renderinfo->displaywidth, renderinfo->fontheight, tuxtxt_color_transp);
		return;
	}

	if (tuxtxt_cache.adip[linkpage][0])
	{
		renderinfo->PosX = renderinfo->StartX + column*width;
		int l = strlen(tuxtxt_cache.adip[linkpage]);

		if (l > 9) /* smaller font, if no space for one half space at front and end */
			tuxtxt_setfontwidth(renderinfo,oldfontwidth * 10 / (l+1));
		tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->PosX, renderinfo->PosY+yoffset, width+(renderinfo->displaywidth%4), renderinfo->fontheight, tuxtxt_atrtable[ATR_L250 + column].bg);
		renderinfo->PosX += ((width) - (l*renderinfo->fontwidth+l*renderinfo->fontwidth/abx))/2; /* center */
		for (p = tuxtxt_cache.adip[linkpage]; *p; p++)
			tuxtxt_RenderCharBB(renderinfo,*p, &tuxtxt_atrtable[ATR_L250 + column]);
		tuxtxt_setfontwidth(renderinfo,oldfontwidth);
	}
	else /* display number */
	{
		renderinfo->PosX = renderinfo->StartX + column*width;
		tuxtxt_FillRect(renderinfo->lfb,renderinfo->var_screeninfo.xres,renderinfo->PosX, renderinfo->PosY+yoffset, renderinfo->displaywidth+renderinfo->sx-renderinfo->PosX, renderinfo->fontheight, tuxtxt_atrtable[ATR_L250 + column].bg);
		if (linkpage < tuxtxt_cache.page)
		{
			line[6] = '<';
			tuxtxt_hex2str(line + 5, linkpage);
		}
		else
			tuxtxt_hex2str(line + 6, linkpage);
		for (p = line; p < line+9; p++)
			tuxtxt_RenderCharBB(renderinfo,*p, &tuxtxt_atrtable[ATR_L250 + column]);
	}
}

void tuxtxt_CreateLine25(tstRenderInfo* renderinfo)
{

	if (!tuxtxt_cache.bttok)
		/* btt completely received and not yet decoded */
		tuxtxt_decode_btt();
	if (tuxtxt_cache.maxadippg >= 0)
		tuxtxt_decode_adip();

	if (!renderinfo->showhex && renderinfo->showflof &&
		 (tuxtxt_cache.flofpages[tuxtxt_cache.page][0] || tuxtxt_cache.flofpages[tuxtxt_cache.page][1] || tuxtxt_cache.flofpages[tuxtxt_cache.page][2] || tuxtxt_cache.flofpages[tuxtxt_cache.page][3])) // FLOF-Navigation present
	{
		int i;

		renderinfo->prev_100 = tuxtxt_cache.flofpages[tuxtxt_cache.page][0];
		renderinfo->prev_10  = tuxtxt_cache.flofpages[tuxtxt_cache.page][1];
		renderinfo->next_10  = tuxtxt_cache.flofpages[tuxtxt_cache.page][2];
		renderinfo->next_100 = tuxtxt_cache.flofpages[tuxtxt_cache.page][3];

		renderinfo->PosY = renderinfo->StartY + 24*renderinfo->fontheight;
		renderinfo->PosX = renderinfo->StartX;
		for (i=renderinfo->nofirst; i<40; i++)
			tuxtxt_RenderCharBB(renderinfo,renderinfo->page_char[24*40 + i], &renderinfo->page_atrb[24*40 + i]);
	}
	else
	{
		/*  normal: blk-1, grp+1, grp+2, blk+1 */
		/*  hex:    hex+1, blk-1, grp+1, blk+1 */
		if (renderinfo->showhex)
		{
			/* arguments: startpage, up, findgroup */
			renderinfo->prev_100 = tuxtxt_next_hex(tuxtxt_cache.page);
			renderinfo->prev_10  = tuxtxt_toptext_getnext(tuxtxt_cache.page, 0, 0);
			renderinfo->next_10  = tuxtxt_toptext_getnext(tuxtxt_cache.page, 1, 1);
		}
		else
		{
			renderinfo->prev_100 = tuxtxt_toptext_getnext(tuxtxt_cache.page, 0, 0);
			renderinfo->prev_10  = tuxtxt_toptext_getnext(tuxtxt_cache.page, 1, 1);
			renderinfo->next_10  = tuxtxt_toptext_getnext(renderinfo->prev_10, 1, 1);
		}
		renderinfo->next_100 = tuxtxt_toptext_getnext(renderinfo->next_10, 1, 0);
		tuxtxt_showlink(renderinfo,0, renderinfo->prev_100);
		tuxtxt_showlink(renderinfo,1, renderinfo->prev_10);
		tuxtxt_showlink(renderinfo,2, renderinfo->next_10);
		tuxtxt_showlink(renderinfo,3, renderinfo->next_100);
	}

	if (//tuxtxt_cache.bttok &&
		 renderinfo->screenmode == 1) /* TOP-Info present, divided screen -> create TOP overview */
	{
		char line[TOPMENUCHARS];
		int current;
		int prev10done, next10done, next100done, indent;
		tstPageAttr *attrcol, *attr; /* color attribute for navigation keys and text */

		int olddisplaywidth = renderinfo->displaywidth;
		renderinfo->displaywidth = 1000*(40-renderinfo->nofirst); // disable pixelinsert;
		tuxtxt_setfontwidth(renderinfo,renderinfo->fontwidth_topmenusmall);

		renderinfo->PosY = TOPMENUSTARTY;
		memset(line, ' ', TOPMENUCHARS); /* init with spaces */

		memcpy(line+TOPMENUINDENTBLK, tuxtxt_cache.adip[renderinfo->prev_100], 12);
		tuxtxt_hex2str(&line[TOPMENUINDENTDEF+12+TOPMENUSPC+2], renderinfo->prev_100);
		tuxtxt_RenderClearMenuLineBB(renderinfo,line, &tuxtxt_atrtable[ATR_L250], &tuxtxt_atrtable[ATR_TOPMENU2]);

/*  1: blk-1, grp-1, grp+1, blk+1 */
/*  2: blk-1, grp+1, grp+2, blk+1 */
#if (LINE25MODE == 1)
		current = renderinfo->prev_10 - 1;
#else
		current = tuxtxt_cache.page - 1;
#endif

		prev10done = next10done = next100done = 0;
		while (renderinfo->PosY <= (TOPMENUENDY-renderinfo->fontheight))
		{
			attr = 0;
			attrcol = &tuxtxt_atrtable[ATR_WB];
			if (!next100done && (renderinfo->PosY > (TOPMENUENDY - 2*renderinfo->fontheight))) /* last line */
			{
				attrcol = &tuxtxt_atrtable[ATR_L253];
				current = renderinfo->next_100;
			}
			else if (!next10done && (renderinfo->PosY > (TOPMENUENDY - 3*renderinfo->fontheight))) /* line before */
			{
				attrcol = &tuxtxt_atrtable[ATR_L252];
				current = renderinfo->next_10;
			}
			else if (!prev10done && (renderinfo->PosY > (TOPMENUENDY - 4*renderinfo->fontheight))) /* line before */
			{
				attrcol = &tuxtxt_atrtable[ATR_L251];
				current = renderinfo->prev_10;
			}
			else do
			{
				tuxtxt_next_dec(&current);
				if (current == renderinfo->prev_10)
				{
					attrcol = &tuxtxt_atrtable[ATR_L251];
					prev10done = 1;
					break;
				}
				else if (current == renderinfo->next_10)
				{
					attrcol = &tuxtxt_atrtable[ATR_L252];
					next10done = 1;
					break;
				}
				else if (current == renderinfo->next_100)
				{
					attrcol = &tuxtxt_atrtable[ATR_L253];
					next100done = 1;
					break;
				}
				else if (current == tuxtxt_cache.page)
				{
					attr = &tuxtxt_atrtable[ATR_TOPMENU0];
					break;
				}
			} while (tuxtxt_cache.adip[current][0] == 0 && (tuxtxt_cache.basictop[current] < 2 || tuxtxt_cache.basictop[current] > 7));

			if (!tuxtxt_cache.bttok || (tuxtxt_cache.basictop[current] >= 2 && tuxtxt_cache.basictop[current] <= 5)) /* block (also for FLOF) */
			{
				indent = TOPMENUINDENTBLK;
				if (!attr)
					attr = &tuxtxt_atrtable[tuxtxt_cache.basictop[current] <=3 ? ATR_TOPMENU1 : ATR_TOPMENU2]; /* green for program block */
			}
			else if (tuxtxt_cache.basictop[current] >= 6 && tuxtxt_cache.basictop[current] <= 7) /* group */
			{
				indent = TOPMENUINDENTGRP;
				if (!attr)
					attr = &tuxtxt_atrtable[ATR_TOPMENU3];
			}
			else
			{
				indent = TOPMENUINDENTDEF;
				if (!attr)
					attr = &tuxtxt_atrtable[ATR_WB];
			}
			memcpy(line+indent, tuxtxt_cache.adip[current], 12);
			tuxtxt_hex2str(&line[TOPMENUINDENTDEF+12+TOPMENUSPC+2], current);
			tuxtxt_RenderClearMenuLineBB(renderinfo,line, attrcol, attr);
		}
		renderinfo->displaywidth = olddisplaywidth;
		tuxtxt_setfontwidth(renderinfo,renderinfo->fontwidth_topmenumain);
	}
}
/******************************************************************************
 * CopyBB2FB                                                                  *
 ******************************************************************************/

void tuxtxt_CopyBB2FB(tstRenderInfo* renderinfo)
{
	unsigned char *src, *dst, *topsrc;
	int fillcolor, i, screenwidth;

	/* line 25 */
	if (!renderinfo->pagecatching)
		tuxtxt_CreateLine25(renderinfo);
	/* copy backbuffer to framebuffer */
	if (!renderinfo->zoommode)
	{

		if (renderinfo->var_screeninfo.yoffset)
			renderinfo->var_screeninfo.yoffset = 0;
		else
			renderinfo->var_screeninfo.yoffset = renderinfo->var_screeninfo.yres;
		if (ioctl(renderinfo->fb, FBIOPAN_DISPLAY, &renderinfo->var_screeninfo) == -1)
			perror("TuxTxt <FBIOPAN_DISPLAY>");

		if (renderinfo->StartX > 0 && *renderinfo->lfb != *(renderinfo->lfb + renderinfo->var_screeninfo.xres * renderinfo->var_screeninfo.yres)) /* adapt background of backbuffer if changed */
			tuxtxt_FillBorder(renderinfo,*(renderinfo->lfb + renderinfo->var_screeninfo.xres * renderinfo->var_screeninfo.yoffset));
//			 ClearBB(*(lfb + renderinfo->var_screeninfo.xres * renderinfo->var_screeninfo.yoffset));

		if (renderinfo->clearbbcolor >= 0)
		{
//			ClearBB(clearbbcolor);
			renderinfo->clearbbcolor = -1;
		}
		return;
	}

	src = dst = topsrc = renderinfo->lfb + renderinfo->StartY*renderinfo->var_screeninfo.xres;


	if (renderinfo->var_screeninfo.yoffset)
		dst += renderinfo->var_screeninfo.xres * renderinfo->var_screeninfo.yres;
	else
	{
		src += renderinfo->var_screeninfo.xres * renderinfo->var_screeninfo.yres;
		topsrc += renderinfo->var_screeninfo.xres * renderinfo->var_screeninfo.yres;
	}
	if (!renderinfo->pagecatching )
		memcpy(dst+(24*renderinfo->fontheight)*renderinfo->var_screeninfo.xres, src + (24*renderinfo->fontheight)*renderinfo->var_screeninfo.xres, renderinfo->var_screeninfo.xres*renderinfo->fontheight); /* copy line25 in normal height */

	if (renderinfo->transpmode)
		fillcolor = tuxtxt_color_transp;
	else
		fillcolor = tuxtxt_cache.FullScrColor;

	if (renderinfo->zoommode == 2)
		src += 12*renderinfo->fontheight*renderinfo->var_screeninfo.xres;

	if (renderinfo->screenmode == 1) /* copy topmenu in normal height (since PIG also keeps dimensions) */
	{
		unsigned char *topdst = dst;

		screenwidth = TV43STARTX;

		topsrc += screenwidth;
		topdst += screenwidth;
		for (i=0; i < 24*renderinfo->fontheight; i++)
		{
			memcpy(topdst, topsrc,renderinfo->ex-screenwidth);
			topdst += renderinfo->var_screeninfo.xres;
			topsrc += renderinfo->var_screeninfo.xres;
		}
	}
	else if (renderinfo->screenmode == 2)
		screenwidth = TV169FULLSTARTX;
	else
		screenwidth = renderinfo->var_screeninfo.xres;

	for (i = renderinfo->StartY; i>0;i--)
	{
		memset(dst - i*renderinfo->var_screeninfo.xres, fillcolor, screenwidth);
	}

	for (i = 12*renderinfo->fontheight; i; i--)
	{
		memcpy(dst, src, screenwidth);
		dst += renderinfo->var_screeninfo.xres;
		memcpy(dst, src, screenwidth);
		dst += renderinfo->var_screeninfo.xres;
		src += renderinfo->var_screeninfo.xres;
	}

//	if (!pagecatching )
//		memcpy(dst, lfb + (StartY+24*fontheight)*renderinfo->var_screeninfo.xres, renderinfo->var_screeninfo.xres*fontheight); /* copy line25 in normal height */
	for (i = renderinfo->var_screeninfo.yres - renderinfo->StartY - 25*renderinfo->fontheight; i >= 0;i--)
	{
		memset(dst + renderinfo->var_screeninfo.xres*(renderinfo->fontheight+i), fillcolor, screenwidth);
	}
}

void tuxtxt_setcolors(tstRenderInfo* renderinfo,unsigned short *pcolormap, int offset, int number)
{
	struct fb_cmap colormap_0 = {0, tuxtxt_color_SIZECOLTABLE, renderinfo->rd0, renderinfo->gn0, renderinfo->bl0, renderinfo->tr0};
	int i, changed=0;
	int j = offset; /* index in global color table */

	unsigned short t = renderinfo->tr0[tuxtxt_color_transp2];
	renderinfo->tr0[tuxtxt_color_transp2] = (renderinfo->trans_mode+7)<<11 | 0x7FF;
#ifndef HAVE_DREAMBOX_HARDWARE
	/* "correct" semi-transparent for Nokia (GTX only allows 2(?) levels of transparency) */
	if (tuxbox_get_vendor() == TUXBOX_VENDOR_NOKIA)
		renderinfo->tr0[tuxtxt_color_transp2] = 0xFFFF;
#endif
	if (t != renderinfo->tr0[tuxtxt_color_transp2]) changed = 1;
	for (i = 0; i < number; i++)
	{
		int r = (pcolormap[i] << 12) & 0xf000;
		int g = (pcolormap[i] <<  8) & 0xf000;
		int b = (pcolormap[i] <<  4) & 0xf000;


		r = (r * (0x3f+(renderinfo->color_mode<<3))) >> 8;
		g = (g * (0x3f+(renderinfo->color_mode<<3))) >> 8;
		b = (b * (0x3f+(renderinfo->color_mode<<3))) >> 8;
		if (renderinfo->rd0[j] != r)
		{
			renderinfo->rd0[j] = r;
			changed = 1;
		}
		if (renderinfo->gn0[j] != g)
		{
			renderinfo->gn0[j] = g;
			changed = 1;
		}
		if (renderinfo->bl0[j] != b)
		{
			renderinfo->bl0[j] = b;
			changed = 1;
		}
		j++;
	}
	if (changed)
		if (ioctl(renderinfo->fb, FBIOPUTCMAP, &colormap_0) == -1)
			perror("TuxTxt <FBIOPUTCMAP>");
}

/******************************************************************************
 * RenderPage                                                                 *
 ******************************************************************************/

void tuxtxt_DoFlashing(tstRenderInfo* renderinfo,int startrow)
{
	int row, col;
	/* get national subset */
	if (renderinfo->auto_national &&
		 tuxtxt_cache.national_subset <= NAT_MAX_FROM_HEADER && /* not for GR/RU as long as line28 is not evaluated */
		 renderinfo->pageinfo && renderinfo->pageinfo->nationalvalid) /* individual subset according to page header */
	{
		tuxtxt_cache.national_subset = countryconversiontable[renderinfo->pageinfo->national];
	}
	/* Flashing */
	tstPageAttr flashattr;
	char flashchar;
	struct timeval tv;
	gettimeofday(&tv,NULL);
	long flashphase = (tv.tv_usec / 1000) % 1000;
	int srow = startrow;
	int erow = 24;
	int factor=1;
	switch (renderinfo->zoommode)
	{
		case 1: erow = 12; factor=2;break;
		case 2: srow = 12; factor=2;break;
	}
	renderinfo->PosY = renderinfo->StartY + startrow*renderinfo->fontheight*factor;
	for (row = srow; row < erow; row++)
	{
		int index = row * 40;
		int dhset = 0;
		int incflash = 3;
		int decflash = 2;
		renderinfo->PosX = renderinfo->StartX;
		for (col = renderinfo->nofirst; col < 40; col++)
		{
			if (renderinfo->page_atrb[index + col].flashing && renderinfo->page_char[index + col] > 0x20 && renderinfo->page_char[index + col]!= 0xff )
			{
				tuxtxt_SetPosX(renderinfo,col);
				flashchar = renderinfo->page_char[index + col];
				int doflash = 0;
				memcpy(&flashattr,&renderinfo->page_atrb[index + col],sizeof(tstPageAttr));
				switch (flashattr.flashing &0x1c) // Flash Rate
				{
					case 0x00 :	// 1 Hz
						if (flashphase>500) doflash = 1;
						break;
					case 0x04 :	// 2 Hz  Phase 1
						if (flashphase<250) doflash = 1;
						break;
					case 0x08 :	// 2 Hz  Phase 2
						if (flashphase>=250 && flashphase<500) doflash = 1;
						break;
					case 0x0c :	// 2 Hz  Phase 3
						if (flashphase>=500 && flashphase<750) doflash = 1;
						break;
					case 0x10 :	// incremental flash
						incflash++;
						if (incflash>3) incflash = 1;
						switch (incflash)
						{
							case 1: if (flashphase<250) doflash = 1; break;
							case 2: if (flashphase>=250 && flashphase<500) doflash = 1;break;
							case 3: if (flashphase>=500 && flashphase<750) doflash = 1;
						}
						break;
					case 0x11 :	// decremental flash
						decflash--;
						if (decflash<1) decflash = 3;
						switch (decflash)
						{
							case 1: if (flashphase<250) doflash = 1; break;
							case 2: if (flashphase>=250 && flashphase<500) doflash = 1;break;
							case 3: if (flashphase>=500 && flashphase<750) doflash = 1;
						}
						break;

				}

				switch (flashattr.flashing &0x03) // Flash Mode
				{
					case 0x01 :	// normal Flashing
						if (doflash) flashattr.fg = flashattr.bg;
						break;
					case 0x02 :	// inverted Flashing
						doflash = 1-doflash;
						if (doflash) flashattr.fg = flashattr.bg;
						break;
					case 0x03 :	// color Flashing
						if (doflash) flashattr.fg = flashattr.fg + (flashattr.fg > 7 ? (-8) : 8);
						break;

				}
				tuxtxt_RenderCharFB(renderinfo,flashchar,&flashattr);
				if (flashattr.doublew) col++;
				if (flashattr.doubleh) dhset = 1;
			}
		}
		if (dhset)
		{
			row++;
			renderinfo->PosY += renderinfo->fontheight*factor;
		}
		renderinfo->PosY += renderinfo->fontheight*factor;
	}

}
void tuxtxt_DoRender(tstRenderInfo* renderinfo, int startrow, int national_subset_bak)
{
	int row, col, byte;
		if (renderinfo->boxed)
		{ 
			if (renderinfo->screenmode != 0) 
				tuxtxt_SwitchScreenMode(renderinfo,0); /* turn off divided screen */
		}
		else 
		{ 
			if (renderinfo->screenmode != renderinfo->prevscreenmode && !renderinfo->transpmode) 
				tuxtxt_SwitchScreenMode(renderinfo,renderinfo->prevscreenmode);
		}

		/* display first column?  */
		renderinfo->nofirst = renderinfo->show39;
		for (row = 1; row < 24; row++)
		{
			byte = renderinfo->page_char[row*40];
			if (byte != ' '  && byte != 0x00 && byte != 0xFF &&
				renderinfo->page_atrb[row*40].fg != renderinfo->page_atrb[row*40].bg)
			{
				renderinfo->nofirst = 0;
				break;
			}
		}
		renderinfo->fontwidth_normal = (renderinfo->ex-renderinfo->sx) / (40-renderinfo->nofirst);
		tuxtxt_setfontwidth(renderinfo,renderinfo->fontwidth_normal);
		renderinfo->fontwidth_topmenumain = (TV43STARTX-renderinfo->sx) / (40-renderinfo->nofirst);
		renderinfo->fontwidth_topmenusmall = (renderinfo->ex- TOPMENUSTARTX) / TOPMENUCHARS;
		renderinfo->fontwidth_small = (TV169FULLSTARTX-renderinfo->sx)  / (40-renderinfo->nofirst);
		switch(renderinfo->screenmode)
		{
			case 0:	tuxtxt_setfontwidth(renderinfo,renderinfo->fontwidth_normal)     ; renderinfo->displaywidth= (renderinfo->ex             -renderinfo->sx);break;
			case 1:  tuxtxt_setfontwidth(renderinfo,renderinfo->fontwidth_topmenumain); renderinfo->displaywidth= (TV43STARTX     -renderinfo->sx);break;
			case 2:  tuxtxt_setfontwidth(renderinfo,renderinfo->fontwidth_small)      ; renderinfo->displaywidth= (TV169FULLSTARTX-renderinfo->sx);break;
		}
		if (renderinfo->transpmode || (renderinfo->boxed && !renderinfo->screenmode))
		{
			tuxtxt_FillBorder(renderinfo,tuxtxt_color_transp);//ClearBB(transp);
#ifndef HAVE_DREAMBOX_HARDWARE
			renderinfo->clearbbcolor = tuxtxt_color_black;
#else
			renderinfo->clearbbcolor = tuxtxt_color_transp;
#endif
		}

		/* get national subset */
		if (renderinfo->auto_national &&
			tuxtxt_cache.national_subset <= NAT_MAX_FROM_HEADER && /* not for GR/RU as long as line28 is not evaluated */
			renderinfo->pageinfo && renderinfo->pageinfo->nationalvalid) /* individual subset according to page header */
		{
			tuxtxt_cache.national_subset = countryconversiontable[renderinfo->pageinfo->national];
#if TUXTXT_DEBUG
			printf("p%03x b%d n%d v%d i%d\n", tuxtxt_cache.page,national_subset_bak, tuxtxt_cache.national_subset, renderinfo->pageinfo->nationalvalid, renderinfo->pageinfo->national);
#endif
		}
		/* render page */
		if (renderinfo->pageinfo && (renderinfo->pageinfo->function == FUNC_GDRCS || renderinfo->pageinfo->function == FUNC_DRCS)) /* character definitions */
		{
			#define DRCSROWS 8
			#define DRCSCOLS (48/DRCSROWS)
			#define DRCSZOOMX 3
			#define DRCSZOOMY 5
			#define DRCSXSPC (12*DRCSZOOMX + 2)
			#define DRCSYSPC (10*DRCSZOOMY + 2)

			unsigned char ax[] = { /* array[0..12] of x-offsets, array[0..10] of y-offsets for each pixel */
				DRCSZOOMX * 0,
				DRCSZOOMX * 1,
				DRCSZOOMX * 2,
				DRCSZOOMX * 3,
				DRCSZOOMX * 4,
				DRCSZOOMX * 5,
				DRCSZOOMX * 6,
				DRCSZOOMX * 7,
				DRCSZOOMX * 8,
				DRCSZOOMX * 9,
				DRCSZOOMX * 10,
				DRCSZOOMX * 11,
				DRCSZOOMX * 12,
				DRCSZOOMY * 0,
				DRCSZOOMY * 1,
				DRCSZOOMY * 2,
				DRCSZOOMY * 3,
				DRCSZOOMY * 4,
				DRCSZOOMY * 5,
				DRCSZOOMY * 6,
				DRCSZOOMY * 7,
				DRCSZOOMY * 8,
				DRCSZOOMY * 9,
				DRCSZOOMY * 10
			};
#if TUXTXT_DEBUG
			printf("TuxTxt <decoding *DRCS %03x/%02x %d>\n", tuxtxt_cache.page, tuxtxt_cache.subpage, renderinfo->pageinfo->function);
#endif
			tuxtxt_ClearBB(renderinfo,tuxtxt_color_black);
			for (col = 0; col < 24*40; col++)
				renderinfo->page_atrb[col] = tuxtxt_atrtable[ATR_WB];

			for (row = 0; row < DRCSROWS; row++)
				for (col = 0; col < DRCSCOLS; col++)
					tuxtxt_RenderDRCS(renderinfo->var_screeninfo.xres,
						renderinfo->page_char + 20 * (DRCSCOLS * row + col + 2),
						renderinfo->lfb
						+ (renderinfo->StartY + renderinfo->fontheight + DRCSYSPC * row + renderinfo->var_screeninfo.yres - renderinfo->var_screeninfo.yoffset) * renderinfo->var_screeninfo.xres
						+ renderinfo->StartX + DRCSXSPC * col,
						ax, tuxtxt_color_white, tuxtxt_color_black);

			memset(renderinfo->page_char + 40, 0xff, 24*40); /* don't render any char below row 0 */
		}
		renderinfo->PosY = renderinfo->StartY + startrow*renderinfo->fontheight;
		for (row = startrow; row < 24; row++)
		{
			int index = row * 40;

			renderinfo->PosX = renderinfo->StartX;
			for (col = renderinfo->nofirst; col < 40; col++)
			{
				tuxtxt_RenderCharBB(renderinfo,renderinfo->page_char[index + col], &renderinfo->page_atrb[index + col]);

				if (renderinfo->page_atrb[index + col].doubleh && renderinfo->page_char[index + col] != 0xff)	/* disable lower char in case of doubleh setting in l25 objects */
					renderinfo->page_char[index + col + 40] = 0xff;
				if (renderinfo->page_atrb[index + col].doublew)	/* skip next column if double width */
				{
					col++;
					if (renderinfo->page_atrb[index + col-1].doubleh && renderinfo->page_char[index + col] != 0xff)	/* disable lower char in case of doubleh setting in l25 objects */
						renderinfo->page_char[index + col + 40] = 0xff;
				}
			}
			renderinfo->PosY += renderinfo->fontheight;
		}
		tuxtxt_DoFlashing(renderinfo,startrow);

		/* update framebuffer */
		tuxtxt_CopyBB2FB(renderinfo);
		tuxtxt_cache.national_subset = national_subset_bak;
}
void tuxtxt_RenderPage(tstRenderInfo* renderinfo)
{
	int i, col, byte, startrow = 0;
	int national_subset_bak = tuxtxt_cache.national_subset;


	/* update page or timestring */
	if (renderinfo->transpmode != 2 && tuxtxt_cache.pageupdate && tuxtxt_cache.page_receiving != tuxtxt_cache.page && renderinfo->inputcounter == 2)
	{
		/* reset update flag */
		tuxtxt_cache.pageupdate = 0;
		if (renderinfo->boxed && renderinfo->subtitledelay) 
		{
			subtitle_cache* c = NULL;
			int j = -1;
			for (i = 0; i < SUBTITLE_CACHESIZE; i++)
			{
				if (j == -1 && !renderinfo->subtitlecache[i])
					j = i;
				if (renderinfo->subtitlecache[i] && !renderinfo->subtitlecache[i]->valid)
				{
					c = renderinfo->subtitlecache[i];
					break;
				}
			}
			if (c == NULL)
			{
				if (j == -1) // no more space in subtitlecache
					return;
				c= malloc(sizeof(subtitle_cache));
				if (c == NULL)
					return;
				memset(c, 0x00, sizeof(subtitle_cache));
				renderinfo->subtitlecache[j] = c;
			}
			c->valid = 0x01;
			gettimeofday(&c->tv_timestamp,NULL);
			if (tuxtxt_cache.subpagetable[tuxtxt_cache.page] != 0xFF)
			{
				tstPageinfo * p = tuxtxt_DecodePage(renderinfo->showl25,c->page_char,c->page_atrb,renderinfo->hintmode, renderinfo->showflof);
				if (p) 
				{
					renderinfo->boxed = p->boxed;
				}
			}
			renderinfo->delaystarted = 1;
			return;
		}
		renderinfo->delaystarted=0;
		/* decode page */
		if (tuxtxt_cache.subpagetable[tuxtxt_cache.page] != 0xFF)
		{
			tstPageinfo * p = tuxtxt_DecodePage(renderinfo->showl25,renderinfo->page_char,renderinfo->page_atrb,renderinfo->hintmode, renderinfo->showflof);
			if (p) 
			{
				renderinfo->pageinfo = p;
				renderinfo->boxed = p->boxed;
			}
			if (renderinfo->boxed || renderinfo->transpmode)
//				tuxtxt_cache.FullScrColor = tuxtxt_color_transp;
				tuxtxt_FillBorder(renderinfo,tuxtxt_color_transp);
			else
				tuxtxt_FillBorder(renderinfo,tuxtxt_cache.FullScrColor);
			if (tuxtxt_cache.colortable) /* as late as possible to shorten the time the old page is displayed with the new colors */
				tuxtxt_setcolors(renderinfo,tuxtxt_cache.colortable, 16, 16); /* set colors for CLUTs 2+3 */
		}
		else
			startrow = 1;
		tuxtxt_DoRender(renderinfo,startrow,national_subset_bak);
	}
	else if (renderinfo->transpmode != 2)
	{
		if (renderinfo->delaystarted)
		{
			struct timeval tv;
			gettimeofday(&tv,NULL);
			for (i = 0; i < SUBTITLE_CACHESIZE ; i++)
			{
				if (renderinfo->subtitlecache[i] && renderinfo->subtitlecache[i]->valid && tv.tv_sec - renderinfo->subtitlecache[i]->tv_timestamp.tv_sec >= renderinfo->subtitledelay)
				{
					memcpy(renderinfo->page_char, renderinfo->subtitlecache[i]->page_char,40 * 25);
					memcpy(renderinfo->page_atrb, renderinfo->subtitlecache[i]->page_atrb,40 * 25 * sizeof(tstPageAttr));
					tuxtxt_DoRender(renderinfo,startrow,national_subset_bak);
					renderinfo->subtitlecache[i]->valid = 0;
					//memset(subtitlecache[i], 0x00, sizeof(subtitle_cache));
					return;
				}
			}
		}	
		if (renderinfo->zoommode != 2)
		{
			renderinfo->PosY = renderinfo->StartY;
			if (tuxtxt_cache.subpagetable[tuxtxt_cache.page] == 0xff)
			{
				renderinfo->page_atrb[32].fg = tuxtxt_color_yellow;
				renderinfo->page_atrb[32].bg = tuxtxt_color_menu1;
				int showpage = tuxtxt_cache.page_receiving;
				int showsubpage = tuxtxt_cache.subpagetable[showpage];
				if (showsubpage!=0xff)
				{

					tstCachedPage *pCachedPage;
					pCachedPage = tuxtxt_cache.astCachetable[showpage][showsubpage];
					if (pCachedPage && tuxtxt_is_dec(showpage))
					{
						renderinfo->PosX = renderinfo->StartX;
						if (renderinfo->inputcounter == 2)
						{
							if (tuxtxt_cache.bttok && !tuxtxt_cache.basictop[tuxtxt_cache.page]) /* page non-existent according to TOP (continue search anyway) */
							{
								renderinfo->page_atrb[0].fg = tuxtxt_color_white;
								renderinfo->page_atrb[0].bg = tuxtxt_color_red;
							}
							else
							{
								renderinfo->page_atrb[0].fg = tuxtxt_color_yellow;
								renderinfo->page_atrb[0].bg = tuxtxt_color_menu1;
							}
							tuxtxt_hex2str(renderinfo->page_char+3, tuxtxt_cache.page);
							for (col = renderinfo->nofirst; col < 7; col++) // selected page
							{
								tuxtxt_RenderCharFB(renderinfo,renderinfo->page_char[col], &renderinfo->page_atrb[0]);
							}
							tuxtxt_RenderCharFB(renderinfo,renderinfo->page_char[col], &renderinfo->page_atrb[32]);
						}
						else
							tuxtxt_SetPosX(renderinfo,8);

						memcpy(&renderinfo->page_char[8], pCachedPage->p0, 24); /* header line without timestring */
						for (col = 0; col < 24; col++)
						{
							tuxtxt_RenderCharFB(renderinfo,pCachedPage->p0[col], &renderinfo->page_atrb[32]);
						}
					}
				}
			}
			/* update timestring */
			tuxtxt_SetPosX(renderinfo,32);
			for (byte = 0; byte < 8; byte++)
			{
				if (!renderinfo->page_atrb[32+byte].flashing)
					tuxtxt_RenderCharFB(renderinfo,tuxtxt_cache.timestring[byte], &renderinfo->page_atrb[32]);
				else
				{
					tuxtxt_SetPosX(renderinfo,33+byte);
					renderinfo->page_char[32+byte] = renderinfo->page_char[32+byte];
				}


			}
		}
		tuxtxt_DoFlashing(renderinfo,startrow);
		tuxtxt_cache.national_subset = national_subset_bak;
	}
	else if (renderinfo->transpmode == 2 && tuxtxt_cache.pageupdate == 2)
	{
#if TUXTXT_DEBUG
		printf("received Update flag for page %03x\n",tuxtxt_cache.page);
#endif
		// display pagenr. when page has been updated while in transparency mode
		renderinfo->PosY = renderinfo->StartY;

		char ns[3];
		tuxtxt_SetPosX(renderinfo,1);
		tuxtxt_hex2str(ns+2,tuxtxt_cache.page);

		tuxtxt_RenderCharFB(renderinfo,ns[0],&tuxtxt_atrtable[ATR_WB]);
		tuxtxt_RenderCharFB(renderinfo,ns[1],&tuxtxt_atrtable[ATR_WB]);
		tuxtxt_RenderCharFB(renderinfo,ns[2],&tuxtxt_atrtable[ATR_WB]);

		tuxtxt_cache.pageupdate=0;
	}
}
/******************************************************************************
 * MyFaceRequester
 ******************************************************************************/

FT_Error tuxtxt_MyFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	FT_Error result;

	result = FT_New_Face(library, face_id, 0, aface);

#if TUXTXT_DEBUG
	if (!result)
		printf("TuxTxt <font %s loaded>\n", (char*)face_id);
	else
		printf("TuxTxt <open font %s failed>\n", (char*)face_id);
#endif

	return result;
}

void tuxtxt_SetRenderingDefaults(tstRenderInfo* renderinfo)
{

	memset(renderinfo,0,sizeof(tstRenderInfo));
	renderinfo->auto_national   = 1;
	renderinfo->showflof        = 1;
	renderinfo->show39          = 1;
	renderinfo->showl25         = 1;
	renderinfo->TTFWidthFactor16  = 28;
	renderinfo->TTFHeightFactor16 = 15;
	renderinfo->color_mode   = 10;
	renderinfo->trans_mode   = 10;
	renderinfo->prev_100   = 0x100;
	renderinfo->prev_10    = 0x100;
	renderinfo->next_100   = 0x100;
	renderinfo->next_10    = 0x100;
	renderinfo->showflof        = 1;
	renderinfo->show39          = 1;
	renderinfo->showl25         = 1;
	renderinfo->TTFWidthFactor16  = 28;
	renderinfo->TTFHeightFactor16 = 16;
	renderinfo->inputcounter  = 2;
	renderinfo->pig = -1;
	renderinfo->avs = -1;
	renderinfo->saa = -1;
	unsigned short rd0[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0x00<<8, 0x00<<8, 0x00<<8, 0,      0      };
	unsigned short gn0[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0x20<<8, 0x10<<8, 0x20<<8, 0,      0      };
	unsigned short bl0[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0x40<<8, 0x20<<8, 0x40<<8, 0,      0      };
	unsigned short tr0[] = {0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0x0000 , 0x0000 , 0x0A00 , 0xFFFF, 0x3000 };

	memcpy(renderinfo->rd0,rd0,tuxtxt_color_SIZECOLTABLE*sizeof(unsigned short));
	memcpy(renderinfo->gn0,gn0,tuxtxt_color_SIZECOLTABLE*sizeof(unsigned short));
	memcpy(renderinfo->bl0,bl0,tuxtxt_color_SIZECOLTABLE*sizeof(unsigned short));
	memcpy(renderinfo->tr0,tr0,tuxtxt_color_SIZECOLTABLE*sizeof(unsigned short));
}
/******************************************************************************
 * InitRendering
 ******************************************************************************/
int tuxtxt_InitRendering(tstRenderInfo* renderinfo,int setTVFormat)
{
	int error,i;

	/* init fontlibrary */
	if ((error = FT_Init_FreeType(&renderinfo->library)))
	{
		printf("TuxTxt <FT_Init_FreeType: 0x%.2X>", error);
		return 0;
	}

	if ((error = FTC_Manager_New(renderinfo->library, 7, 2, 0, &tuxtxt_MyFaceRequester, NULL, &renderinfo->manager)))
	{
		FT_Done_FreeType(renderinfo->library);
		printf("TuxTxt <FTC_Manager_New: 0x%.2X>\n", error);
		return 0;
	}

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	if ((error = FTC_SBitCache_New(renderinfo->manager, &renderinfo->cache)))
#else
	if ((error = FTC_SBit_Cache_New(renderinfo->manager, &renderinfo->cache)))
#endif
	{
		FTC_Manager_Done(renderinfo->manager);
		FT_Done_FreeType(renderinfo->library);
		printf("TuxTxt <FTC_SBit_Cache_New: 0x%.2X>\n", error);
		return 0;
	}


	/* calculate font dimensions */
	renderinfo->displaywidth = (renderinfo->ex-renderinfo->sx);
	renderinfo->fontheight = (renderinfo->ey-renderinfo->sy) / 25; //21;
	renderinfo->fontwidth_normal = (renderinfo->ex-renderinfo->sx) / 40;
	tuxtxt_setfontwidth(renderinfo,renderinfo->fontwidth_normal);
	renderinfo->fontwidth_topmenumain = (TV43STARTX-renderinfo->sx) / 40;
	renderinfo->fontwidth_topmenusmall = (renderinfo->ex- TOPMENUSTARTX) / TOPMENUCHARS;
	renderinfo->fontwidth_small = (TV169FULLSTARTX-renderinfo->sx)  / 40;
	{
		int i;
		for (i = 0; i <= 10; i++)
			aydrcs[i] = (renderinfo->fontheight * i + 5) / 10;
	}

	/* center screen */
	renderinfo->StartX = renderinfo->sx; //+ (((ex-sx) - 40*fontwidth) / 2);
	renderinfo->StartY = renderinfo->sy + (((renderinfo->ey-renderinfo->sy) - 25*renderinfo->fontheight) / 2);


	if (renderinfo->usettf)
	{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
		renderinfo->typettf.face_id = (FTC_FaceID) TUXTXTTTFVAR;
		renderinfo->typettf.height = (FT_UShort) renderinfo->fontheight * renderinfo->TTFHeightFactor16 / 16;
#else
		renderinfo->typettf.font.face_id = (FTC_FaceID) TUXTXTTTFVAR;
		renderinfo->typettf.font.pix_height = (FT_UShort) renderinfo->fontheight * renderinfo->TTFHeightFactor16 / 16;
#endif
	}
	else
	{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
		renderinfo->typettf.face_id = (FTC_FaceID) TUXTXTOTBVAR;
		renderinfo->typettf.width  = (FT_UShort) 23;
		renderinfo->typettf.height = (FT_UShort) 23;
#else
		renderinfo->typettf.font.face_id = (FTC_FaceID) TUXTXTOTBVAR;
		renderinfo->typettf.font.pix_width  = (FT_UShort) 23;
		renderinfo->typettf.font.pix_height = (FT_UShort) 23;
#endif
	}

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	renderinfo->typettf.flags = FT_LOAD_MONOCHROME;
	if ((error = FTC_Manager_LookupFace(renderinfo->manager, renderinfo->typettf.face_id, &renderinfo->face)))
	{
		renderinfo->typettf.face_id = (renderinfo->usettf ? (FTC_FaceID) TUXTXTTTF : TUXTXTOTB);
		if ((error = FTC_Manager_Lookup_Face(renderinfo->manager, renderinfo->typettf.face_id, &renderinfo->face)))
		{
#else
	renderinfo->typettf.image_type = ftc_image_mono;
	if ((error = FTC_Manager_Lookup_Face(renderinfo->manager, renderinfo->typettf.font.face_id, &renderinfo->face)))
	{
		renderinfo->typettf.font.face_id = (renderinfo->usettf ? (FTC_FaceID) TUXTXTTTF : TUXTXTOTB);
		if ((error = FTC_Manager_Lookup_Face(renderinfo->manager, renderinfo->typettf.font.face_id, &renderinfo->face)))
		{
#endif
			printf("TuxTxt <FTC_Manager_Lookup_Face failed with Errorcode 0x%.2X>\n", error);
			FTC_Manager_Done(renderinfo->manager);
			FT_Done_FreeType(renderinfo->library);
			return 0;
		}
	}
	renderinfo->ascender = (renderinfo->usettf ? renderinfo->fontheight * renderinfo->face->ascender / renderinfo->face->units_per_EM : 16);
#if TUXTXT_DEBUG
	printf("TuxTxt <fh%d fw%d fs%d tm%d ts%d ym%d %d %d sx%d sy%d a%d>\n",
			 renderinfo->fontheight, renderinfo->fontwidth, renderinfo->fontwidth_small, renderinfo->fontwidth_topmenumain, renderinfo->fontwidth_topmenusmall,
			 ymosaic[0], ymosaic[1], ymosaic[2], renderinfo->StartX, renderinfo->StartY, renderinfo->ascender);
#endif

	/* get fixed screeninfo */
	if (ioctl(renderinfo->fb, FBIOGET_FSCREENINFO, &renderinfo->fix_screeninfo) == -1)
	{
		perror("TuxTxt <FBIOGET_FSCREENINFO>");
		FTC_Manager_Done(renderinfo->manager);
		FT_Done_FreeType(renderinfo->library);
		return 0;
	}

	/* get variable screeninfo */
	if (ioctl(renderinfo->fb, FBIOGET_VSCREENINFO, &renderinfo->var_screeninfo) == -1)
	{
		perror("TuxTxt <FBIOGET_VSCREENINFO>");
		FTC_Manager_Done(renderinfo->manager);
		FT_Done_FreeType(renderinfo->library);
		return 0;
	}


	/* set variable screeninfo for double buffering */
	renderinfo->var_screeninfo.yres_virtual = 2*renderinfo->var_screeninfo.yres;
	renderinfo->var_screeninfo.xres_virtual = renderinfo->var_screeninfo.xres;
	renderinfo->var_screeninfo.yoffset      = 0;

	if (ioctl(renderinfo->fb, FBIOPUT_VSCREENINFO, &renderinfo->var_screeninfo) == -1)
	{
		perror("TuxTxt <FBIOPUT_VSCREENINFO>");
		FTC_Manager_Done(renderinfo->manager);
		FT_Done_FreeType(renderinfo->library);
		return 0;
	}

#if TUXTXT_DEBUG
	if (ioctl(fb, FBIOGET_VSCREENINFO, &renderinfo->var_screeninfo) == -1)
	{
		perror("TuxTxt <FBIOGET_VSCREENINFO>");
		FTC_Manager_Done(renderinfo->manager);
		FT_Done_FreeType(renderinfo->library);
		return 0;
	}

	printf("TuxTxt <screen real %d*%d, virtual %d*%d, offset %d>\n",
	renderinfo->var_screeninfo.xres, renderinfo->var_screeninfo.yres,
	renderinfo->var_screeninfo.xres_virtual, renderinfo->var_screeninfo.yres_virtual,
	renderinfo->var_screeninfo.yoffset);
#endif
		/* map framebuffer into memory */
	renderinfo->lfb = (unsigned char*)mmap(0, renderinfo->fix_screeninfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, renderinfo->fb, 0);

	/* set new colormap */
	tuxtxt_setcolors(renderinfo,(unsigned short *)tuxtxt_defaultcolors, 0, tuxtxt_color_SIZECOLTABLE);


	if (!renderinfo->lfb)
	{
		perror("TuxTxt <mmap>");
		FTC_Manager_Done(renderinfo->manager);
		FT_Done_FreeType(renderinfo->library);
		return 0;
	}
	tuxtxt_ClearFB(renderinfo,tuxtxt_color_transp);
	tuxtxt_ClearBB(renderinfo,tuxtxt_color_transp); /* initialize backbuffer */
	for (i = 0; i < 40 * 25; i++)
	{
		renderinfo->page_char[i] = ' ';
		renderinfo->page_atrb[i].fg = tuxtxt_color_transp;
		renderinfo->page_atrb[i].bg = tuxtxt_color_transp;
		renderinfo->page_atrb[i].charset = C_G0P;
		renderinfo->page_atrb[i].doubleh = 0;
		renderinfo->page_atrb[i].doublew = 0;
		renderinfo->page_atrb[i].IgnoreAtBlackBgSubst = 0;
	}
	if (setTVFormat)
	{
		/* open avs */
		if ((renderinfo->avs = open(AVS, O_RDWR)) == -1)
		{
			perror("TuxTxt <open AVS>");
			FTC_Manager_Done(renderinfo->manager);
			FT_Done_FreeType(renderinfo->library);
			return 0;
		}

		ioctl(renderinfo->avs, AVSIOGSCARTPIN8, &renderinfo->fnc_old);
		ioctl(renderinfo->avs, AVSIOSSCARTPIN8, &fncmodes[renderinfo->screen_mode1]);
		/* open saa */
		if ((renderinfo->saa = open(SAA, O_RDWR)) == -1)
		{
			perror("TuxTxt <open SAA>");
			FTC_Manager_Done(renderinfo->manager);
			FT_Done_FreeType(renderinfo->library);
			return 0;
		}
	
		ioctl(renderinfo->saa, SAAIOGWSS, &renderinfo->saa_old);
		ioctl(renderinfo->saa, SAAIOSWSS, &saamodes[renderinfo->screen_mode1]);
	}
	/* open pig */
	if ((renderinfo->pig = open(PIG, O_RDWR)) == -1)
	{
		perror("TuxTxt <open PIG>");
		FTC_Manager_Done(renderinfo->manager);
		FT_Done_FreeType(renderinfo->library);
		return 0;
	}
	return 1;	
}
/******************************************************************************
 * EndRendering
 ******************************************************************************/
void tuxtxt_EndRendering(tstRenderInfo* renderinfo)
{
	int i;
	if (renderinfo->pig >= 0)
		close(renderinfo->pig);
	renderinfo->pig = -1;
	/* restore videoformat */
	if (renderinfo->avs >= 0)
		ioctl(renderinfo->avs, AVSIOSSCARTPIN8, &renderinfo->fnc_old);
	if (renderinfo->saa >= 0)
		ioctl(renderinfo->saa, SAAIOSWSS, &renderinfo->saa_old);
	/* clear subtitlecache */
	for (i = 0; i < SUBTITLE_CACHESIZE; i++)
	{
		if (renderinfo->subtitlecache[i])
			free(renderinfo->subtitlecache[i]);
	}

	if (renderinfo->var_screeninfo.yoffset)
	{
		renderinfo->var_screeninfo.yoffset = 0;

		if (ioctl(renderinfo->fb, FBIOPAN_DISPLAY, &renderinfo->var_screeninfo) == -1)
			perror("TuxTxt <FBIOPAN_DISPLAY>");
	}
	 /* close avs */
	if (renderinfo->avs >= 0)
		close(renderinfo->avs);
	renderinfo->avs = -1;

	/* close saa */
	if (renderinfo->saa >= 0)
		close(renderinfo->saa);
	renderinfo->saa = -1;


	/* close freetype */
	if (renderinfo->manager)
		FTC_Manager_Done(renderinfo->manager);
	if (renderinfo->library)
		FT_Done_FreeType(renderinfo->library);
	renderinfo->manager = 0;
	renderinfo->library = 0;
	tuxtxt_ClearFB(renderinfo,renderinfo->previousbackcolor);
	/* unmap framebuffer */
	munmap(renderinfo->lfb, renderinfo->fix_screeninfo.smem_len);
	printf("[TTX] Rendering ended\n");
}
#endif
