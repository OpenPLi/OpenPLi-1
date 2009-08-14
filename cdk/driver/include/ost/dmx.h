/* 
 * dmx.h
 *
 * Copyright (C) 2000 Marcus Metzler <marcus@convergence.de>
 *                  & Ralph  Metzler <ralph@convergence.de>
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

#ifndef _OST_DMX_H_
#define _OST_DMX_H_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#include <time.h>
#endif

#ifndef EBUFFEROVERFLOW
#define EBUFFEROVERFLOW 769
#endif

/* pid_t conflicts with linux/include/linux/types.h !!!*/

typedef uint16_t dvb_pid_t;  

#define DMX_FILTER_SIZE 16

typedef enum
{
	DMX_OUT_DECODER, /* Streaming directly to decoder. */
	DMX_OUT_TAP,     /* Output going to a memory buffer */
	                 /* (to be retrieved via the read command).*/
	DMX_OUT_TS_TAP,  /* Output multiplexed into a new TS  */
	                 /* (to be retrieved by reading from the */
	                 /* logical DVR device).                 */
	DMX_OUT_ES_NET,  /* send elementary stream over network */
	DMX_OUT_TS_NET   /* send transport stream over network */
} dmxOutput_t;


typedef enum
{
	DMX_IN_FRONTEND, /* Input from a front-end device.  */
	DMX_IN_DVR       /* Input from the logical DVR device.  */
} dmxInput_t;


typedef enum
{
        DMX_PES_AUDIO,
	DMX_PES_VIDEO,
	DMX_PES_TELETEXT,
	DMX_PES_SUBTITLE,
	DMX_PES_PCR,
	DMX_PES_OTHER
} dmxPesType_t;


typedef enum
{
        DMX_SCRAMBLING_EV,
        DMX_FRONTEND_EV
} dmxEvent_t;


typedef enum
{
	DMX_SCRAMBLING_OFF,
	DMX_SCRAMBLING_ON
} dmxScramblingStatus_t;


typedef struct dmxFilter
{
	uint8_t         filter[DMX_FILTER_SIZE];
	uint8_t         mask[DMX_FILTER_SIZE];
} dmxFilter_t;


struct dmxFrontEnd
{
  //TBD             tbd;
};


struct dmxSctFilterParams
{
	dvb_pid_t                    pid;
	dmxFilter_t                  filter;
	uint32_t                     timeout;
	uint32_t                     flags;
#define DMX_CHECK_CRC       1
#define DMX_ONESHOT         2
#define DMX_IMMEDIATE_START 4
#define DMX_KERNEL_CLIENT   0x8000
};


struct dmxPesFilterParams
{
	dvb_pid_t                    pid;
	dmxInput_t                   input;
	dmxOutput_t                  output;
	dmxPesType_t                 pesType;
	uint32_t                     flags;
	uint32_t                     ip;
	uint16_t                     port;
};


struct dmxEvent
{
	dmxEvent_t                  event;
	time_t                      timeStamp;
	union
	{
		dmxScramblingStatus_t scrambling;
	} u;
};


#define DMX_START                _IOW('o',41,int) 
#define DMX_STOP                 _IOW('o',42,int)
#define DMX_SET_FILTER           _IOW('o',43,struct dmxSctFilterParams *)
#define DMX_SET_PES_FILTER       _IOW('o',44,struct dmxPesFilterParams *)
#define DMX_SET_BUFFER_SIZE      _IOW('o',45,unsigned long)
#define DMX_GET_EVENT            _IOR('o',46,struct dmxEvent *)
#define DMX_GET_PES_PIDS         _IOR('o',47,dvb_pid_t *)

#endif /*_OST_DMX_H_*/
