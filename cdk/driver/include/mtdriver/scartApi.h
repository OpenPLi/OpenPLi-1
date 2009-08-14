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
 * scartApi.h
 * 
 * Copyright (c) 2001 Nokia and others. All Rights Reserved. 
 * 
 * Contributor(s):
 */

/**
 * \file scartApi.h
 *
 * \brief API definitions for the Scart Device Driver.
 *
 * \version $Id: scartApi.h,v 1.1 2001/05/25 22:59:34 gillem Exp $
 *
 * \date <Add date>
 *
 * \author Mika Leppanen, Nokia Home Communications
 *
 * The Scart Device Driver is provides functions
 * for communciation with peripheral devices Scart
 * connectors.
 * The driver API is entierly implemented in terms of IOCTL commands.
 * This file defines the IOCTL-codes, datatypes and constants used
 * in the driver's API.
 */


#ifndef _SCART_H_
#define _SCART_H_

#ifdef __cplusplus
extern "C" {
#if 0
# }  Brace balancing for EMACS
#endif
#endif

/* scartApi.h
 *
 * Copyright (c) 2000, Nokia Home Communiations
 *
 * Project:
 *     OST
 */

/* $Id: scartApi.h,v 1.1 2001/05/25 22:59:34 gillem Exp $ */

/*
 * Include files
 */
#include <linux/ioctl.h>
#include "ostDevices.h"
#include "ostErrors.h"

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

/*
 * Structs and typedefs
 */
typedef struct _scartVolume {
    int32_t minVol;
    int32_t maxVol;
    int32_t incVol;
    int32_t curVol;
} scartVolume;

typedef struct _scartRGBLevel {
    int32_t minRGB;
    int32_t maxRGB;
    int32_t incRGB;
    int32_t curRGB;
} scartRGBLevel;

typedef struct _scartBypass {
    int32_t ctrlCmd;
    int32_t TVBypass;
    int32_t VCRBypass;
} scartBypass;

/*
 * Defines and consts
 */

/*
 * Scart mute control (bitcoded)
 */
#define AUDIO_ON                0x00
#define AUDIO_OFF               0x01

/*
 * Audio stereo/mono (bitcoded)
 */
#define AUDIO_STEREO_NORMAL     0x00
#define AUDIO_MONO_NORMAL       0x01
#define AUDIO_STEREO_SWAP       0x02
#define AUDIO_MONO_RIGHT        0x03
#define AUDIO_MONO_LEFT         0x04


/*
 * Scart Pin8 level control (bitcoded)
 */
#define VIDEO_TV_OFF            0x00 /* Only used for input pin8 */
#define VIDEO_TV_4X3            0x03
#define VIDEO_TV_16X9           0x01

/*
 * ON/OFF switch for the Scart Pin8 (4x3 / 16x9)
 */
#define VIDEO_SLOW_SWITCH_OFF   0x00
#define VIDEO_SLOW_SWITCH_ON    0x01


/*
 * Scart Pin16 control (bitcoded)
 */
#define VIDEO_RGB_SWITCH_OFF    0x00
#define VIDEO_RGB_SWITCH_ACT1   0x01
#define VIDEO_RGB_SWITCH_ACT2   0x02
#define VIDEO_RGB_SWITCH_ON     0x03


/* 
 * Scart A/V routing, used with SCART_BYPASS_SET / SCART_BYPASS_GET
 */

/*
 * Scart control defines (TV or VCR), used when setting the bypass mode
 */
#define SET_TV_SCART            0x01
#define SET_VCR_SCART           0x02

/*
 * Scart A/V bypass options (bitcoded)
 */
#define SCART_TV_OUT_DIG_NORMAL          0x0    /* TV:  DIGITAL -> CVBS + RGB  */
#define SCART_TV_OUT_DIG_YC              0x4    /* TV:  DIGITAL -> Y/C         */
#define SCART_TV_OUT_DIG_SECAM           0x5    /* TV   DIGITAL -> SECAM       */
#define SCART_TV_OUT_VCR                 0x2    /* TV:  VCR     -> CVBS (Y/C)  */
#define SCART_TV_OUT_MUTE                0x7    /* TV:  MUTED                  */

#define SCART_VCR_OUT_DIG_NORMAL_OSD     0x0    /* VCR: DIGITAL -> CVBS with OSD       */
#define SCART_VCR_OUT_DIG_NORMAL_NO_OSD  0x3    /* VCR: DIGITAL -> CVBS no OSD         */
#define SCART_VCR_OUT_DIG_YC_OSD         0x4    /* VCR: DIGITAL -> Y/C with OSD        */
#define SCART_VCR_OUT_DIG_YC_NO_OSD      0x1    /* VCR: DIGITAL -> Y/C no OSD          */
#define SCART_VCR_OUT_DIG_SECAM          0x5    /* VCR: DIGITAL -> SECAM with OSD      */
#define SCART_VCR_OUT_MUTE               0x7    /* VCR: MUTED                          */


/*
 *  General ioctl operations for scart
 */
#define SCART_VOLUME_SET        _IOW(OST_IOCTL, SCART_IOCTL0, scartVolume*)
#define SCART_VOLUME_GET        _IOR(OST_IOCTL, SCART_IOCTL1, scartVolume*)

#define SCART_MUTE_SET          _IOW(OST_IOCTL, SCART_IOCTL2, int32_t*)
#define SCART_MUTE_GET          _IOR(OST_IOCTL, SCART_IOCTL3, int32_t*)

#define SCART_AUD_FORMAT_SET    _IOW(OST_IOCTL, SCART_IOCTL4, int32_t*)
#define SCART_AUD_FORMAT_GET    _IOR(OST_IOCTL, SCART_IOCTL5, int32_t*)

#define SCART_VID_FORMAT_SET    _IOW(OST_IOCTL, SCART_IOCTL6, int32_t*)
#define SCART_VID_FORMAT_GET    _IOR(OST_IOCTL, SCART_IOCTL7, int32_t*)
#define SCART_VID_FORMAT_INPUT_GET _IOR(OST_IOCTL, SCART_IOCTL16, int32_t*)

#define SCART_SLOW_SWITCH_SET   _IOW(OST_IOCTL, SCART_IOCTL8, int32_t*)
#define SCART_SLOW_SWITCH_GET   _IOR(OST_IOCTL, SCART_IOCTL9, int32_t*)

#define SCART_RGB_LEVEL_SET     _IOW(OST_IOCTL, SCART_IOCTL10, scartRGBLevel*)
#define SCART_RGB_LEVEL_GET     _IOR(OST_IOCTL, SCART_IOCTL11, scartRGBLevel*)

#define SCART_RGB_SWITCH_SET    _IOW(OST_IOCTL, SCART_IOCTL12, int32_t*)
#define SCART_RGB_SWITCH_GET    _IOR(OST_IOCTL, SCART_IOCTL13, int32_t*)

#define SCART_BYPASS_SET        _IOW(OST_IOCTL, SCART_IOCTL14, scartBypass*)
#define SCART_BYPASS_GET        _IOR(OST_IOCTL, SCART_IOCTL15, scartBypass*)



#ifdef __cplusplus
}
#endif 

#endif /*#ifndef _SCART_H_ */
