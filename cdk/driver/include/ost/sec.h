/* 
 * sec.h
 *
 * Copyright (C) 2000 Ralph  Metzler <ralph@convergence.de>
 *                  & Marcus Metzler <marcus@convergence.de>
                      for convergence integrated media GmbH
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

#ifndef _OST_SEC_H_
#define _OST_SEC_H_

#ifndef __KERNEL__
#include <stdint.h>
#endif

#define SEC_MAX_DISEQC_PARAMS 3

/*
 * cmdtype is not part of the official api.
 * it is ignored if SEC_CMDTYPE_DISEQC_RAW
 * is not set.
 */
struct secDiseqcCmd {
	uint8_t cmdtype; 
	uint8_t addr;
	uint8_t cmd;
	uint8_t numParams;
	uint8_t params[SEC_MAX_DISEQC_PARAMS];
};

typedef uint32_t secVoltage;

enum {
	SEC_VOLTAGE_OFF,
	SEC_VOLTAGE_LT, 
	SEC_VOLTAGE_13, 
	SEC_VOLTAGE_13_5,
	SEC_VOLTAGE_18,  
	SEC_VOLTAGE_18_5
};

#define SEC_VOLTAGE_HORIZONTAL SEC_VOLTAGE_18
#define SEC_VOLTAGE_VERTICAL   SEC_VOLTAGE_13

typedef uint32_t secToneMode;

typedef enum {
        SEC_TONE_ON,
	SEC_TONE_OFF
} secToneMode_t;


typedef uint32_t secMiniCmd;

typedef enum {
	SEC_MINI_NONE,
	SEC_MINI_A,    
	SEC_MINI_B
} secMiniCmd_t;

struct secStatus {
	int32_t     busMode;
	secVoltage  selVolt;
	secToneMode contTone;
};

enum {
	SEC_BUS_IDLE,
	SEC_BUS_BUSY,
	SEC_BUS_OFF,
	SEC_BUS_OVERLOAD
};

struct secCommand {
	int32_t type;
	union {
	        struct secDiseqcCmd diseqc;
		uint8_t vsec;
		uint32_t pause;
	} u;
};

struct secCmdSequence {
	secVoltage voltage;
	secMiniCmd miniCommand;
	secToneMode continuousTone;
	
	uint32_t numCommands;
	struct secCommand* commands;
};

enum {
	SEC_CMDTYPE_DISEQC,
	SEC_CMDTYPE_DISEQC_RAW,
	SEC_CMDTYPE_VSEC,
	SEC_CMDTYPE_PAUSE
};


#define SEC_GET_STATUS         _IOR('o',91,struct secStatus *)
#define SEC_RESET_OVERLOAD     _IOW('o',92,void)
#define SEC_SEND_SEQUENCE      _IOW('o',93,struct secCmdSequence *)
#define SEC_SET_TONE           _IOW('o',94,secToneMode)
#define SEC_SET_VOLTAGE        _IOW('o',95,secVoltage)

typedef enum {
	SEC_DISEQC_SENT,
	SEC_VSEC_SENT,
	SEC_PAUSE_COMPLETE,
	SEC_CALLBACK_ERROR
} secCallback_t;		


#endif /*_OST_SEC_H_*/ 
