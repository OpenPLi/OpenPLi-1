/* 
 * The contents of this file are subject to the NOKOS 
 * License Version 1.0 (the "License"); you may not use
 * this file except in compliance with the License. 
 * 
 * Software distributed under the License is distributed
 * on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND,
 * either express or implied. See the License for the
 * specific language governing rights and limitations
 * under the License. 
 * 
 * The Original Software is:
 * ostDevices.h
 * 
 * Copyright (c) 2001 Nokia and others. All Rights Reserved. 
 * 
 * Contributor(s):
 */

/**
 * \file ostDevices.h
 *
 * \brief Default major numbers and device names for OST-specific
 * device drivers.
 *
 * \version $Id: ostDevices.h,v 1.3 2002/02/24 15:32:06 woglinde Exp $
 *
 * \date <Add date>
 *
 * \author Rickard Westman, Nokia Home Communications
 *
 */

#ifndef _CC_DEVICES_H_
#define _CC_DEVICES_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ccDeviceNumbers.h
 *
 * Copyright (c) 1999, 2000 Nokia Home Communications,
 *
 * Project:
 *      OST
 */

/* $Id: ostDevices.h,v 1.3 2002/02/24 15:32:06 woglinde Exp $ */

/*
 * Defines and consts
 */

/**
 * Major device number registry for OST devices.  
 */
#define DVBCLK_MAJOR_NR     254
#define DVBFE_MAJOR_NR      253
#define DMX_MAJOR_NR        252
#define SEC_MAJOR_NR        251
#define CC_MAJOR_NR         250
#define SCART_MAJOR_NR      249
#define DVBTEST_MAJOR_NR    248
#define SC_MAJOR_NR         247
#define VIDEO_MAJOR_NR      246
#define AUDIO_MAJOR_NR      245
#define DSCR_MAJOR_NR       244
#define IRRC_MAJOR_NR       243
#define FPRTC_MAJOR_NR     242
#define DVB_FLASH_MAJOR_NR  241
#define TTXT_MAJOR_NR       240
/* N.B: Next number should be in 120-127 range. */
#define CI_MAJOR_NR         127
#define OSTKBD_MAJOR_NR     126
#define FPD_MAJOR_NR        125
#define DVBIO_MAJOR_NR      124

/**
 * Minor misc devices used by the OST.
 * The region 240-255 is reserved for local use, so we use it.
 */
#define OPM_MISC_MINOR_NR 240
#define IP_DVB_MISC_MINOR_NR 241

/**
 * Device name registry for OST devices.  
 */
#define DVBCLK_DEVICE_NAME    "dvbclk"
#define DVBFE_DEVICE_NAME     "dvbfe"
#define DMX_DEVICE_NAME       "demux"
#define SEC_DEVICE_NAME       "sec"
#define CC_DEVICE_NAME        "TBD"
#define SCART_DEVICE_NAME     "scart"
#define DVBTEST_DEVICE_NAME   "dvbtest"
#define OPM_DEVICE_NAME       "opm"
#define SC_DEVICE_NAME        "sc"
#define VIDEO_DEVICE_NAME     "video"
#define AUDIO_DEVICE_NAME     "audio"
#define DSCR_DEVICE_NAME      "dscr"
#define FPRTC_DEVICE_NAME     "fprtc"
#define DVB_FLASH_DEVICE_NAME "dvbflash"
#define TTXT_DEVICE_NAME      "ttxt"
#define IRRC_DEVICE_NAME      "irrc"
#define CI_DEVICE_NAME        "ci"
#define FPD_DEVICE_NAME       "fpd"
#define OSTKBD_DEVICE_NAME    "ostkbd"
#define DVBIO_DEVICE_NAME     "dvbio"

/**
 *  Base ioctl code for Nokia OST
 */      
#define OST_IOCTL 'N'                   /* sequences 0-7f reserved */

/** 
 * Sequence number registry for OST ioctl:s.  The idea is that
 * the real ioctl numbers are defined in the header file for
 * each device, using _IO/_IOR/_IORW/_IOW macros with
 * OST_IOCTL as the base code, and a sequence number which is
 * unique within the OST.  To reserve such a unique sequence
 * number, just add one #define using the lowest free sequence
 * number.  To make this easy, the list of sequence numbers
 * should be sorted in numerical order, not (necessarily)
 * grouped by device.  Note that this is just a mechanism to
 * get unique sequence numbers for the real ioctl definition,
 * and should *never* be be used in applications.  That is why
 * the names only indicate which driver has reserved the
 * number and not what it is used for.  
 */
#define DVBCLK_IOCTL0    0
#define DVBCLK_IOCTL1    1
#define DVBCLK_IOCTL2    2

#define DMX_IOCTL0       3
#define DMX_IOCTL1       4
#define DMX_IOCTL2       5
#define DMX_IOCTL3       6
#define DMX_IOCTL4       7
#define DMX_IOCTL5       8

#define DVBFE_IOCTL0     9
#define DVBFE_IOCTL1     10
#define DVBFE_IOCTL2     11
#define DVBFE_IOCTL3     12
#define DVBFE_IOCTL4     13
#define DVBFE_IOCTL5     14
#define DVBFE_IOCTL6     15
#define DVBFE_IOCTL7     16
#define DVBFE_IOCTL8     17
#define DVBFE_IOCTL9     18
#define DVBFE_IOCTL10    19
#define DVBFE_IOCTL11    20

#define SEC_IOCTL0       21
#define SEC_IOCTL1       22
#define SEC_IOCTL2       23

#define OPM_IOCTL0       24
#define OPM_IOCTL1       25
#define OPM_IOCTL2       26

#define AUDIO_IOCTL0     27
#define AUDIO_IOCTL1     28
#define AUDIO_IOCTL2     29
#define AUDIO_IOCTL3     30
#define AUDIO_IOCTL4     31
#define AUDIO_IOCTL5     32
#define AUDIO_IOCTL6     33
#define AUDIO_IOCTL7     34
#define AUDIO_IOCTL8     35
#define AUDIO_IOCTL9     36

#define VIDEO_IOCTL0     37
#define VIDEO_IOCTL1     38
#define VIDEO_IOCTL2     39
#define VIDEO_IOCTL3     40
#define VIDEO_IOCTL4     41
#define VIDEO_IOCTL5     42
#define VIDEO_IOCTL6     43
#define VIDEO_IOCTL7     44
#define VIDEO_IOCTL8     45
#define VIDEO_IOCTL9     46
#define VIDEO_IOCTL10    47
#define VIDEO_IOCTL11    48
#define VIDEO_IOCTL12    49
#define VIDEO_IOCTL13    50
#define VIDEO_IOCTL14    51
#define VIDEO_IOCTL15    52
#define VIDEO_IOCTL16    53

#define SC_IOCTL0        54
#define SC_IOCTL1        55
#define SC_IOCTL2        56
#define SC_IOCTL3        57
#define SC_IOCTL4        58
#define SC_IOCTL5        59
#define SC_IOCTL6        60
#define SC_IOCTL7        61
#define SC_IOCTL8        62
#define SC_IOCTL9        63

#define SCART_IOCTL0     63
#define SCART_IOCTL1     64
#define SCART_IOCTL2     65
#define SCART_IOCTL3     66
#define SCART_IOCTL4     67
#define SCART_IOCTL5     68
#define SCART_IOCTL6     69
#define SCART_IOCTL7     70
#define SCART_IOCTL8     71
#define SCART_IOCTL9     72
#define SCART_IOCTL10    73
#define SCART_IOCTL11    74
#define SCART_IOCTL12    75
#define SCART_IOCTL13    76
#define SCART_IOCTL14    77
#define SCART_IOCTL15    78

#define SC_IOCTL10       79
#define SC_IOCTL11       80

#define DSCR_IOCTL0      81
#define DSCR_IOCTL1      82
#define DSCR_IOCTL2      83
#define DSCR_IOCTL3      84

#define IP_DVB_IOCTL1    85

#define SCART_IOCTL16    86
#define SCART_IOCTL17    87

#define IP_DVB_IOCTL2    88

#define DF_IOCTL1        88
#define DF_IOCTL2        89
#define DF_IOCTL3        90
#define DF_IOCTL4        91

#define TTXT_IOCTL1      92
#define TTXT_IOCTL2      93
#define TTXT_IOCTL3      94

#define IRRC_IOCTL0      95
#define IRRC_IOCTL1      96
#define IRRC_IOCTL2      97

#define SEC_IOCTL3       98
#define SEC_IOCTL4       99

#define DMX_IOCTL6       100
#define DMX_IOCTL7       101
#define DMX_IOCTL8       102
#define DMX_IOCTL10      104
#define DMX_IOCTL11      105
#define DMX_IOCTL12      106
#define DMX_IOCTL13      107
#define DMX_IOCTL16      110
#define DMX_IOCTL17      111
#define DMX_IOCTL18      112
#define DMX_IOCTL19      113
#define DMX_IOCTL20      114
#define DMX_IOCTL21      115

#define OSTKBD_IOCTL0    116
#define OSTKBD_IOCTL1    117

#define IRKBD_IOCTL0    118
#define IRKBD_IOCTL1    119

#define CI_IOCTL0        120
#define CI_IOCTL1        121
#define CI_IOCTL2        122
#define CI_IOCTL3        123
#define CI_IOCTL4        124
#define CI_IOCTL5        125
#define CI_IOCTL6        126
#define CI_IOCTL7        127
#define CI_IOCTL8        128
#define CI_IOCTL9        129

#define FPD_IOCTL0       131
#define FPD_IOCTL1       132

#define DVBIO_IOCTL0     133

#define FPD_IOCTL2       134
#define FPD_IOCTL3       135
#define FPD_IOCTL4       136
#define FPD_IOCTL5       137

#define DMX_IOCTL22      138
#define DMX_IOCTL23      139 

#define CI_IOCTL10       140
#define CI_IOCTL11       141

#define FPRTC_IOCTL0     142
#define FPRTC_IOCTL1     143
#define FPRTC_IOCTL2     144
#define FPRTC_IOCTL3     145


/* A command that DVB side devices could implement (callSync) to report their version. */
#define DVB_DEV_VER      32767

#ifdef __cplusplus
}
#endif 

#endif /*#ifndef _CC_DEVICE_NUMBERS_H */
