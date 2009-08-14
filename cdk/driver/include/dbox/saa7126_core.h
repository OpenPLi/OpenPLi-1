/*
 *   saa7126_core.h - pal driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Gillem (htoa@gmx.net)
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
 *   $Log: saa7126_core.h,v $
 *   Revision 1.11  2002/08/12 17:08:44  wjoost
 *   SAA_WSS_OFF hinzugefügt
 *
 *   Revision 1.10  2002/08/04 12:14:21  wjoost
 *   wide screen signaling
 *
 *   Revision 1.9  2001/11/22 17:26:12  gillem
 *   - add power save mode (experimental)
 *   - start vps
 *
 *   Revision 1.8  2001/07/03 20:23:22  gillem
 *   - add defines
 *
 *   Revision 1.7  2001/07/03 19:55:05  gillem
 *   - add ioctl to set rgb/fbas/svideo
 *   - remove module option svideo
 *   - add module option mode
 *
 *   Revision 1.6  2001/05/16 22:12:48  gillem
 *   - add encoder setting
 *
 *   Revision 1.5  2001/04/07 01:45:34  tmbinc
 *   added philips-support.
 *
 *   Revision 1.4  2001/02/11 21:29:43  gillem
 *   - some stuff
 *
 *   Revision 1.3  2001/02/11 12:31:29  gillem
 *   - add ioctl SAAIOGREG,SAAIOSINP,SAAIOSENC
 *   - change i2c stuff
 *   - change device to pal
 *   - change major to 240
 *   - add load option board (0=nokia,1=philips,2=sagem,3=no config)
 *
 *   Revision 1.2  2001/01/06 10:06:55  gillem
 *   cvs check
 *
 *   $Revision: 1.11 $
 *
 */

#define SAAIOGREG		1	/* read registers				*/
#define SAAIOSINP		2	/* input control				*/
#define SAAIOSOUT		3	/* output control 			*/
#define SAAIOSENC		4	/* set encoder (pal/ntsc)		*/
#define SAAIOSMODE		5	/* set mode (rgb/fbas/svideo)	*/
#define SAAIOSPOWERSAVE	6	/* set power save */
#define SAAIOGPOWERSAVE	7	/* get power save */

#define SAAIOSVPSDATA	8	/* set vps data */
#define SAAIOGVPSDATA	9	/* get vps data */

#define SAAIOSWSS		10	/* set wide screen signaling data */
#define SAAIOGWSS		11	/* get wide screen signaling data */

#define SAA_MODE_RGB	0
#define SAA_MODE_FBAS	1
#define SAA_MODE_SVIDEO	2

#define SAA_NTSC		0
#define SAA_PAL			1

#define SAA_INP_MP1		1
#define SAA_INP_MP2		2
#define SAA_INP_CSYNC	4
#define SAA_INP_DEMOFF	6
#define SAA_INP_SYMP	8
#define SAA_INP_CBENB	128

#define SAA_WSS_43F		0	/* full format 4:3 */
#define SAA_WSS_149C	1	/* box 14:9 center */
#define SAA_WSS_149T	2	/* box 14:9 top */
#define SAA_WSS_169C	3	/* box 16:9 center */
#define SAA_WSS_169T	4	/* box 16:9 top */
#define SAA_WSS_GT169C	5	/* box > 16:9 center */
#define SAA_WSS_43_149C	6	/* full format 4:3 with 14:9 center letterbox content */
#define SAA_WSS_169F	7	/* full format 16:9 (anamorphic) */
#define SAA_WSS_OFF		8	/* no wide screen signaling */

// TODO: fix this table

typedef struct s_saa_data {
	unsigned char version   : 3  __attribute__((packed));
	unsigned char ccrdo     : 1  __attribute__((packed));
	unsigned char ccrde     : 1  __attribute__((packed));
	unsigned char res01     : 1  __attribute__((packed));
	unsigned char fseq      : 1  __attribute__((packed));
	unsigned char o_2       : 1  __attribute__((packed));

	unsigned char res02[0x26-0x01]   __attribute__((packed));	// NULL

	unsigned char wss_7_0   : 8  __attribute__((packed));

	unsigned char wsson     : 1  __attribute__((packed));
	unsigned char res03     : 1  __attribute__((packed));
	unsigned char wss_13_8  : 6  __attribute__((packed));

	unsigned char deccol    : 1  __attribute__((packed));
	unsigned char decfis    : 1  __attribute__((packed));
	unsigned char bs        : 6  __attribute__((packed));

	unsigned char sres      : 1  __attribute__((packed));
	unsigned char res04     : 1  __attribute__((packed));
	unsigned char be        : 6  __attribute__((packed));

	unsigned char cg_7_0    : 8  __attribute__((packed));

	unsigned char cg_15_8   : 8  __attribute__((packed));

	unsigned char cgen      : 1  __attribute__((packed));
	unsigned char res05     : 3  __attribute__((packed));
	unsigned char cg_19_16  : 4  __attribute__((packed));

	unsigned char vbsen     : 2  __attribute__((packed));
	unsigned char cvbsen    : 1  __attribute__((packed));
	unsigned char cen       : 1  __attribute__((packed));
	unsigned char cvbstri   : 1  __attribute__((packed));
	unsigned char rtri      : 1  __attribute__((packed));
	unsigned char gtri      : 1  __attribute__((packed));
	unsigned char btri      : 1  __attribute__((packed));

	unsigned char res06[0x38-0x2e]  __attribute__((packed));	// NULL

	unsigned char res07     : 3  __attribute__((packed));
	unsigned char gy        : 5  __attribute__((packed));

	unsigned char res08     : 3  __attribute__((packed));
	unsigned char gcd       : 5  __attribute__((packed));

	unsigned char vbenb     : 1  __attribute__((packed));
	unsigned char res09     : 2  __attribute__((packed));
	unsigned char symp      : 1  __attribute__((packed));
	unsigned char demoff    : 1  __attribute__((packed));
	unsigned char csync     : 1  __attribute__((packed));
	unsigned char mp2c      : 2  __attribute__((packed));

	unsigned char res10[0x54-0x3b]  __attribute__((packed));	// ???

	unsigned char vpsen     : 1  __attribute__((packed));
	unsigned char ccirs     : 1  __attribute__((packed));
	unsigned char res11     : 4  __attribute__((packed));
	unsigned char edge      : 2  __attribute__((packed));

	unsigned char vps5      : 8  __attribute__((packed));
	unsigned char vps11     : 8  __attribute__((packed));
	unsigned char vps12     : 8  __attribute__((packed));
	unsigned char vps13     : 8  __attribute__((packed));
	unsigned char vps14     : 8  __attribute__((packed));

	unsigned char chps      : 8  __attribute__((packed));
	unsigned char gainu_7_0 : 8  __attribute__((packed));
	unsigned char gainv_7_0 : 8  __attribute__((packed));

	unsigned char gainu_8   : 1  __attribute__((packed));
	unsigned char decoe     : 1  __attribute__((packed));
	unsigned char blckl     : 6  __attribute__((packed));

	unsigned char gainv_8   : 1  __attribute__((packed));
	unsigned char decph     : 1  __attribute__((packed));
	unsigned char blnnl     : 6  __attribute__((packed));

	unsigned char ccrs      : 2  __attribute__((packed));
	unsigned char blnvb     : 6  __attribute__((packed));

	unsigned char res12     : 8  __attribute__((packed)); // NULL

	unsigned char downb     : 1  __attribute__((packed));
	unsigned char downa     : 1  __attribute__((packed));
	unsigned char inpi      : 1  __attribute__((packed));
	unsigned char ygs       : 1  __attribute__((packed));
	unsigned char res13     : 1  __attribute__((packed));
	unsigned char scbw      : 1  __attribute__((packed));
	unsigned char pal       : 1  __attribute__((packed));
	unsigned char fise      : 1  __attribute__((packed));

	// 62h
	unsigned char rtce      : 1  __attribute__((packed));
	unsigned char bsta      : 7  __attribute__((packed));

	unsigned char fsc0      : 8  __attribute__((packed));
	unsigned char fsc1      : 8  __attribute__((packed));
	unsigned char fsc2      : 8  __attribute__((packed));
	unsigned char fsc3      : 8  __attribute__((packed));

	unsigned char l21o0     : 8  __attribute__((packed));
	unsigned char l21o1     : 8  __attribute__((packed));
	unsigned char l21e0     : 8  __attribute__((packed));
	unsigned char l21e1     : 8  __attribute__((packed));

	unsigned char srcv0     : 1  __attribute__((packed));
	unsigned char srcv1     : 1  __attribute__((packed));
	unsigned char trcv2     : 1  __attribute__((packed));
	unsigned char orcv1     : 1  __attribute__((packed));
	unsigned char prcv1     : 1  __attribute__((packed));
	unsigned char cblf      : 1  __attribute__((packed));
	unsigned char orcv2     : 1  __attribute__((packed));
	unsigned char prcv2     : 1  __attribute__((packed));

	// 6ch
	unsigned char htrig0    : 8  __attribute__((packed));
	unsigned char htrig1    : 8  __attribute__((packed));

	unsigned char sblbn     : 1  __attribute__((packed));
	unsigned char blckon    : 1  __attribute__((packed));
	unsigned char phres     : 2  __attribute__((packed));
	unsigned char ldel      : 2  __attribute__((packed));
	unsigned char flc       : 2  __attribute__((packed));

	unsigned char ccen      : 2  __attribute__((packed));
	unsigned char ttxen     : 1  __attribute__((packed));
	unsigned char sccln     : 5  __attribute__((packed));

	unsigned char rcv2s_lsb : 8  __attribute__((packed));
	unsigned char rcv2e_lsb : 8  __attribute__((packed));

	unsigned char res14     : 1  __attribute__((packed));
	unsigned char rvce_mbs  : 3  __attribute__((packed));
	unsigned char res15     : 1  __attribute__((packed));
	unsigned char rvcs_mbs  : 3  __attribute__((packed));

	unsigned char ttxhs     : 8  __attribute__((packed));
	unsigned char ttxhl     : 4  __attribute__((packed));
	unsigned char ttxhd     : 4  __attribute__((packed));

	unsigned char csynca    : 5  __attribute__((packed));
	unsigned char vss       : 3  __attribute__((packed));

	unsigned char ttxovs    : 8  __attribute__((packed));
	unsigned char ttxove    : 8  __attribute__((packed));
	unsigned char ttxevs    : 8  __attribute__((packed));
	unsigned char ttxeve    : 8  __attribute__((packed));

	// 7ah
	unsigned char fal       : 8  __attribute__((packed));
	unsigned char lal       : 8  __attribute__((packed));

	unsigned char ttx60     : 1  __attribute__((packed));
	unsigned char lal8      : 1  __attribute__((packed));
	unsigned char ttx0      : 1  __attribute__((packed));
	unsigned char fal8      : 1  __attribute__((packed));
	unsigned char ttxeve8   : 1  __attribute__((packed));
	unsigned char ttxove8   : 1  __attribute__((packed));
	unsigned char ttxevs8   : 1  __attribute__((packed));
	unsigned char ttxovs8   : 1  __attribute__((packed));

	unsigned char res16     : 8  __attribute__((packed));

	unsigned char ttxl12    : 1  __attribute__((packed));
	unsigned char ttxl11    : 1  __attribute__((packed));
	unsigned char ttxl10    : 1  __attribute__((packed));
	unsigned char ttxl9     : 1  __attribute__((packed));
	unsigned char ttxl8     : 1  __attribute__((packed));
	unsigned char ttxl7     : 1  __attribute__((packed));
	unsigned char ttxl6     : 1  __attribute__((packed));
	unsigned char ttxl5     : 1  __attribute__((packed));

	unsigned char ttxl20    : 1  __attribute__((packed));
	unsigned char ttxl19    : 1  __attribute__((packed));
	unsigned char ttxl18    : 1  __attribute__((packed));
	unsigned char ttxl17    : 1  __attribute__((packed));
	unsigned char ttxl16    : 1  __attribute__((packed));
	unsigned char ttxl15    : 1  __attribute__((packed));
	unsigned char ttxl14    : 1  __attribute__((packed));
	unsigned char ttxl13    : 1  __attribute__((packed));
} s_saa_data;

#define SAA_DATA_SIZE		sizeof(s_saa_data)

#ifdef __KERNEL__

#endif
