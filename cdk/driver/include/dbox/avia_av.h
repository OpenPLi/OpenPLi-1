/*
 *   avia_av.h - AViA driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2001 Gillem (htoa@gmx.net)
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
 *   $Log: avia_av.h,v $
 *   Revision 1.9.2.1  2002/11/17 01:59:13  obi
 *   "backport" of latest dvb api version 1 drivers from HEAD branch
 *
 *   Revision 1.10  2002/10/28 14:34:25  wjoost
 *   SPTS und AC3 / cleanup
 *
 *   Revision 1.9  2002/10/03 12:47:58  Jolt
 *   AViA AV cleanups
 *
 *
 *
 *
 *   $Revision: 1.9.2.1 $
 *
 */


#ifndef AVIA_AV_H
#define AVIA_AV_H

#define AVIA_AV_STREAM_TYPE_SPTS	0x01
#define AVIA_AV_STREAM_TYPE_PES		0x02
#define AVIA_AV_STREAM_TYPE_ES		0x03

#define TM_DRAM  0x00
#define TM_GBUS  0x80
#define TM_SRAM  0xC0

extern int aviarev;

extern u32 avia_rd(int mode, int address);
extern void avia_wr(int mode, u32 address, u32 data);

extern u32 avia_command(u32 cmd, ...);
extern void avia_flush_pcr(void);

#define wGB(a, d) avia_wr(TM_GBUS, a, d)
#define rGB(a) avia_rd(TM_GBUS, a)
#define wSR(a, d) avia_wr(TM_SRAM, a, d)
#define rSR(a) avia_rd(TM_SRAM, a)
#define wDR(a, d) avia_wr(TM_DRAM, a, d)
#define rDR(a) avia_rd(TM_DRAM, a)

#define Abort				0x8120
#define Digest				0x0621
#define Fade				0x0223
#define Freeze				0x0125
#define NewPlayMode			0x0028
#define Pause				0x022A
#define Reset				0x802D
#define Resume				0x002E

#define SelectStream			0x0231
#define SetFill				0x0532

#define CancelStill			0x0040
#define DigitalStill			0x0241
#define NewChannel			0x8342
#define Play				0x0343
#define SetWindow			0x0244
#define WindowClear			0x0045
#define SetStreamType			0x8146
#define SwitchOSDBuffer			0x8254
#define CancelTimeCode			0x0147
#define SetTimeCode1			0x0148
#define SetTimeCode			0x0248
#define GenerateTone			0x0449

#define OSDCopyData			0x0350
#define OSDCopyRegion			0x0651
#define OSDFillData			0x0352
#define OSDFillRegion			0x0553
#define OSDXORData			0x0357
#define OSDXORegion			0x0658
#define PCM_Mix				0x0659
#define PCM_MakeWaves			0x065A

/* DRAM Register */
#define AC3_ACMOD_CMIXLEV		0x0418
#define AC3_BSI_FRAME			0x040c
#define AC3_BSI_IS_BEING_READ		0x0404
#define AC3_BSI_VALID			0x0408
#define AC3_BSID_BSMOD			0x0414
#define AC3_C_LEVEL			0x0130
#define AC3_COMPR_LANGCOD		0x0424
#define AC3_DIALNORM2_COMPR2		0x042c
#define AC3_FRAME_NUMBER		0x0400
#define AC3_FSCOD_FRMSIZECOD		0x0410
#define AC3_HIGH_CUT			0x011c
#define AC3_L_LEVEL			0x012c
#define AC3_LANGCOD2_MIXLEV2		0x0430
#define AC3_LFE_LEVEL			0x0140
#define AC3_LFE_OUTPUT_ENABLE		0x0124
#define AC3_LFEON_DIALNORM		0x0420
#define AC3_LOW_BOOST			0x0118
#define AC3_MIXLEV_ROOMTYP		0x0428
#define AC3_OPERATION_MODE		0x0114
#define AC3_ORIGBS_TIMECOD1		0x0438
#define AC3_OUTPUT_MODE			0x0110
#define AC3_PCM_SCALE_FACTOR		0x0120
#define AC3_R_LEVEL			0x0134
#define AC3_ROOMTYP2_COPYRIGHTB		0x0434
#define AC3_SURMIXLEV_DSURMOD		0x041c
#define AC3_SR_LEVEL			0x013c
#define AC3_SL_LEVEL			0x0138
#define AC3_TIMECOD2_EBITS		0x043c

#define ARGUMENT_1			0x0044
#define ARGUMENT_2			0x0048
#define ARGUMENT_3			0x004c
#define ARGUMENT_4			0x0050
#define ARGUMENT_5			0x0054
#define ARGUMENT_6			0x0058

#define ASPECT_RATIO			0x03b8
#define ASPECT_RATIO_MODE		0x0084

#define AUDIO_ATTENUATION		0x00f4
#define AUDIO_CLOCK_SELECTION		0x00ec
#define AUDIO_CONFIG			0x00e0
#define AUDIO_DAC_MODE			0x00e8
#define AUDIO_EMPTINESS			0x02cc
#define AUDIO_PTS_DELAY			0x01c0
#define AUDIO_PTS_REPEAT_THRESHOLD_1	0x01c8
#define AUDIO_PTS_REPEAT_THRESHOLD_2	0x01d0
#define AUDIO_PTS_SKIP_THRESHOLD_1	0x01c4
#define AUDIO_PTS_SKIP_THRESHOLD_2	0x01cc
#define AUDIO_TYPE			0x03f0

#define AV_SYNC_MODE			0x01b0

#define BACKGROUND_COLOR		0x009c
#define BIT_RATE			0x03c0
#define BITSTREAM_SOURCE		0x01a4
#define BORDER_COLOR			0x0098

#define BUFF_INT_SRC			0x02b4

#define COMMAND				0x0040
#define CUR_PIC_DISPLAYED		0x02d0
#define DATE_TIME			0x0324
#define DISABLE_OSD			0x0250
#define DISP_SIZE_H_V			0x03cc
#define DISPLAY_ASPECT_RATIO		0x0080
#define DRAM_INFO			0x0068

#define ERR_ASPECT_RATIO_INFORMATION	0x00c0
#define ERR_CONCEALMENT_LEVEL		0x00b4
#define ERR_FRAME_RATE_CODE		0x00c4
#define ERR_HORIZONTAL_SIZE		0x00b8
#define ERR_INT_SRC			0x02c4
#define ERR_STILL_DEF_HSIZE		0x00ac
#define ERR_STILL_DEF_VSIZE		0x00b0
#define ERR_VERTICAL_SIZE		0x00bc
#define EXTENDED_VERSION		0x0334
#define FORCE_CODED_ASPECT_RATIO	0x00c8
#define FRAME_RATE			0x03bc
#define GOP_FLAGS			0x03d4
#define H_SIZE				0x03b0
#define I_SLICE				0x00d8
#define IEC_958_CHANNEL_STATUS_BITS	0x00fc
#define ERR_958_DELAY			0x00f0
#define INT_MASK			0x0200
#define INT_STATUS			0x02ac
#define INTERPRET_USER_DATA		0x01d8
#define INTERPRET_USER_DATA_MASK	0x01dc
#define MEMORY_MAP			0x021c

#define MIXPCM_BUFSTART			0x0580
#define MIXPCM_BUFEND			0x0584
#define MIXPCM_BUFPTR			0x0588
#define MIXPCM_LOOPCNTR			0x058c
#define MIXPCM_CMDWAITING		0x0594

#define ML_HEARTBEAT			0x0470
#define MPEG_AUDIO_HEADER1		0x0400
#define MPEG_AUDIO_HEADER2		0x0404

#define MR_AUD_PTS			0x030c
#define MR_AUD_STC			0x0310
#define MR_PIC_PTS			0x02f0
#define MR_PIC_STC			0x02f4

#define MRC_ID				0x02A0
#define MRC_STATUS			0x02A8

#define N_AUD_DECODED			0x02f8
#define N_AUD_ERRORS			0x0320
#define N_AUD_SLOWDOWN1			0x0300
#define N_AUD_SLOWDOWN2			0x0308
#define N_AUD_SPEEDUP1			0x02fc
#define N_AUD_SPEEDUP2			0x0304

#define N_DECODED			0x02e4
#define N_REPEATED			0x02ec
#define N_SKIPPED			0x02e8
#define N_SYS_ERRORS			0x0318
#define N_VID_ERRORS			0x031c

#define NEW_AUDIO_CONFIG		0x0468
#define NEW_AUDIO_MODE			0x0460

#define NEXT_PIC_DISPLAYED		0x02d4

#define OSD_BUFFER_END			0x0244
#define OSD_BUFFER_IDLE_START		0x0248
#define OSD_BUFFER_START		0x0240
#define OSD_EVEN_FIELD			0x00a0
#define OSD_ODD_FIELD			0x00a4
#define OSD_VALID			0x02e0

#define PAN_SCAN_HORIZONTAL_OFFSET	0x008c
#define PAN_SCAN_SOURCE			0x0088

#define PIC_HEADER			0x03e4
#define PIC_TYPE			0x03dc

#define PIC1_CLOSED_CAPTION		0x0350
#define PIC1_EXTENDED_DATA		0x0354
#define PIC1_PAN_SCAN			0x0348
#define PIC1_PICTURE_START		0x0340
#define PIC1_PTS			0x0344
#define PIC1_TREF_PTYP_FLGS		0x0358
#define PIC1_USER_DATA			0x034c

#define PIC2_CLOSED_CAPTION		0x0370
#define PIC2_EXTENDED_DATA		0x0374
#define PIC2_PAN_SCAN			0x0368
#define PIC2_PICTURE_START		0x0360
#define PIC2_PTS			0x0364
#define PIC2_TREF_PTYP_FLGS		0x0378
#define PIC2_USER_DATA			0x036c

#define PIC3_CLOSED_CAPTION		0x0380
#define PIC3_EXTENDED_DATA		0x0390
#define PIC3_PAN_SCAN			0x0394
#define PIC3_PICTURE_START		0x0388
#define PIC3_PTS			0x0384
#define PIC3_TREF_PTYP_FLGS		0x0398
#define PIC3_USER_DATA			0x038c

#define PROC_STATE			0x02a0
#define SEQ_FLAGS			0x03c8
#define STATUS_ADDRESS			0x005c
#define TEMP_REF			0x03d8
#define TIME_CODE			0x03d0
#define TM_MODE				0x01a8
#define UCODE_MEMORY			0x006c
#define UND_INT_SRC			0x02b8

#define USER_DATA_BUFFER_END		0x0274
#define USER_DATA_BUFFER_START		0x0270
#define USER_DATA_READ			0x0278
#define USER_DATA_WRITE			0x027c

#define V_SIZE				0x03b4

#define VBV_DELAY			0x03e0
#define VBV_SIZE			0x03c4

#define AVIA_AV_VERSION			0x0330

#define VIDEO_EMPTINESS			0x02c8
#define VIDEO_FIELD			0x02d8
#define VIDEO_MODE			0x007c
#define VIDEO_PTS_DELAY			0x01b4
#define VIDEO_PTS_REPEAT_THRESHOLD	0x01bc
#define VIDEO_PTS_SKIP_THRESHOLD	0x01b8

#define VSYNC_HEARTBEAT			0x046c

#define AUDIO_SEQUENCE_ID		0x0540
#define NEW_AUDIO_SEQUENCE		0x0544

#define PROC_STATE_INITIALIZATION	0x0001
#define PROC_STATE_IDLE			0x0002
#define PROC_STATE_PLAY			0x0004
#define PROC_STATE_FREEZE		0x0020
#define PROC_STATE_NEWCHANNEL		0x0080

#endif
