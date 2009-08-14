/*
 *   avia_gt_ir.h - ir driver for AViA (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2002 Florian Schirmer (jolt@tuxbox.org)
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
 */

#ifndef AVIA_GT_IR_H
#define AVIA_GT_IR_H

#define AVIA_GT_IR_MAX_PULSE_COUNT	(128 + 1)

#define USEC_TO_CWP(period)			((((period) * frequency) + 500000) / 1000000)

#define WAIT_IR_UNIT_READY(unit)	if (unit##_unit_busy) { 												\
																											\
										if (block) {														\
																											\
											if (wait_event_interruptible(unit##_wait, !unit##_unit_busy))	\
									               return -ERESTARTSYS;										\
																											\
										} else {															\
																											\
											return -EWOULDBLOCK;											\
																											\
										}																	\
																											\
									}

typedef struct {

	u8 RTH;
	u8 RTC;

} sAviaGtRxIrPulse;

typedef struct {

	u32 low;
	u32 high;

} sAviaGtIrPulse;

typedef struct {

	u8 MSPR;
	u8 MSPL;

} sAviaGtTxIrPulse;

extern u32 avia_gt_ir_get_rx_buffer_read_position(void);
extern u32 avia_gt_ir_get_rx_buffer_write_position(void);
extern int avia_gt_ir_queue_pulse(u32 period_high, u32 period_low, u8 block);
extern wait_queue_head_t *avia_gt_ir_receive_data(void);
extern int avia_gt_ir_receive_pulse(u32 *period_low, u32 *period_high, u8 block);
extern int avia_gt_ir_send_buffer(u8 block);
extern int avia_gt_ir_send_pulse(u32 period_high, u32 period_low, u8 block);
extern void avia_gt_ir_set_duty_cycle(u32 new_duty_cycle);
extern void avia_gt_ir_set_frequency(u32 new_frequency);
extern int avia_gt_ir_init(void);
extern void avia_gt_ir_exit(void);
	    
#endif
