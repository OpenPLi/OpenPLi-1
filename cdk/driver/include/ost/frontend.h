/*
 * frontend.h
 *
 * Copyright (C) 2000 Marcus Metzler <marcus@convergence.de>
 *                  & Ralph  Metzler <ralph@convergence.de>
 *                    for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef _FRONTEND_H_
#define _FRONTEND_H_

#include <asm/types.h>


#define ENOSIGNAL 768
#ifndef EBUFFEROVERFLOW
#define EBUFFEROVERFLOW 769
#endif


typedef __u32 FrontendStatus;

/* bit definitions for FrontendStatus */
#define FE_HAS_POWER         1
#define FE_HAS_SIGNAL        2
#define FE_SPECTRUM_INV      4
#define FE_HAS_LOCK          8
#define FE_HAS_CARRIER      16
#define FE_HAS_VITERBI      32
#define FE_HAS_SYNC         64
#define FE_TUNER_HAS_LOCK  128


/* possible values for spectral inversion */
typedef enum {
        INVERSION_OFF,
        INVERSION_ON,
        INVERSION_AUTO
} SpectralInversion;

/* possible values for FEC_inner/FEC_outer */
typedef enum {
        FEC_AUTO,
        FEC_1_2,
        FEC_2_3,
        FEC_3_4,
        FEC_5_6,
        FEC_7_8,
        FEC_NONE
} CodeRate;


typedef enum {
        QPSK,
        QAM_16,
        QAM_32,
        QAM_64,
        QAM_128,
        QAM_256
} Modulation;


typedef enum {
        TRANSMISSION_MODE_2K,
        TRANSMISSION_MODE_8K
} TransmitMode;

typedef enum {
        BANDWIDTH_8_MHZ,
        BANDWIDTH_7_MHZ,
        BANDWIDTH_6_MHZ
} BandWidth;


typedef enum {
        GUARD_INTERVAL_1_32,
        GUARD_INTERVAL_1_16,
        GUARD_INTERVAL_1_8,
        GUARD_INTERVAL_1_4
} GuardInterval;


typedef enum {
        HIERARCHY_NONE,
        HIERARCHY_1,
        HIERARCHY_2,
        HIERARCHY_4
} Hierarchy;


typedef struct {
        __u32     SymbolRate; /* symbol rate in Symbols per second */
        CodeRate  FEC_inner;  /* forward error correction (see above) */
} QPSKParameters;


typedef struct {
        __u32      SymbolRate; /* symbol rate in Symbols per second */
        CodeRate   FEC_inner;  /* forward error correction (see above) */
        Modulation QAM;        /* modulation type (see above) */
} QAMParameters;


typedef struct {
        BandWidth     bandWidth;
        CodeRate      HP_CodeRate;          /* high priority stream code rate */
        CodeRate      LP_CodeRate;          /* low priority stream code rate */
        Modulation    Constellation;        /* modulation type (see above) */
        TransmitMode  TransmissionMode;
        GuardInterval guardInterval;
        Hierarchy     HierarchyInformation;
} OFDMParameters;


typedef enum {
        FE_QPSK,
        FE_QAM,
        FE_OFDM
} FrontendType;


typedef struct {
        __u32 Frequency;                /* (absolute) frequency in Hz for QAM/OFDM */
                                        /* intermediate frequency in kHz for QPSK */
	SpectralInversion Inversion;    /* spectral inversion */
        union {
                QPSKParameters qpsk;
                QAMParameters  qam;
                OFDMParameters ofdm;
        } u;
} FrontendParameters;


typedef enum {
        FE_UNEXPECTED_EV, /* unexpected event (e.g. loss of lock) */
        FE_COMPLETION_EV, /* completion event, tuning succeeded */
        FE_FAILURE_EV     /* failure event, we couldn't tune */
} EventType;


typedef struct {
        EventType type; /* type of event, FE_UNEXPECTED_EV, ... */

        long timestamp; /* time in seconds since 1970-01-01 */

        union {
                struct {
                        FrontendStatus previousStatus; /* status before event */
                        FrontendStatus currentStatus;  /* status during event */
                } unexpectedEvent;
                FrontendParameters completionEvent;    /* parameters for which the
                                                          tuning succeeded */
                FrontendStatus failureEvent;           /* status at failure (e.g. no lock) */
        } u;
} FrontendEvent;

typedef struct {
        FrontendType type;
        __u32        minFrequency;
        __u32        maxFrequency;
        __u32        maxSymbolRate;
        __u32        minSymbolRate;
        __u32        hwType;
        __u32        hwVersion;
} FrontendInfo;


typedef enum {
        FE_POWER_ON,
        FE_POWER_STANDBY,
        FE_POWER_SUSPEND,
        FE_POWER_OFF
} FrontendPowerState;


#define FE_SELFTEST                   _IO('o', 61)
#define FE_SET_POWER_STATE            _IOW('o', 62, FrontendPowerState)
#define FE_GET_POWER_STATE            _IOR('o', 63, FrontendPowerState*)
#define FE_READ_STATUS                _IOR('o', 64, FrontendStatus*)
#define FE_READ_BER                   _IOW('o', 65, __u32*)
#define FE_READ_SIGNAL_STRENGTH       _IOR('o', 66, __s32*)
#define FE_READ_SNR                   _IOR('o', 67, __s32*)
#define FE_READ_UNCORRECTED_BLOCKS    _IOW('o', 68, __u32*)
#define FE_GET_NEXT_FREQUENCY         _IOW('o', 69, __u32*)
#define FE_GET_NEXT_SYMBOL_RATE       _IOW('o', 70, __u32*)

#define FE_SET_FRONTEND               _IOW('o', 71, FrontendParameters*)
#define FE_GET_FRONTEND               _IOR('o', 72, FrontendParameters*)
#define FE_GET_INFO                   _IOR('o', 73, FrontendInfo*)
#define FE_GET_EVENT                  _IOR('o', 74, FrontendEvent*)

#define FE_SEC_SET_TONE               _IOW('o', 75, secToneMode)

#define FE_SEC_SET_VOLTAGE            _IOW('o', 76, secVoltage)
#define FE_SEC_MINI_COMMAND           _IOW('o', 77, secMiniCmd)
#define FE_SEC_COMMAND                _IOW('o', 78, struct secCommand*)
#define FE_SEC_GET_STATUS             _IOR('o', 79, struct secStatus*)

#define FE_SETFREQ                    _IOW('o', 80, u32*)

#endif /*_FRONTEND_H_*/

