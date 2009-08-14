/*
 *   avia_gt_ir.c - AViA eNX ir driver (dbox-II-project)
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
 *
 *   $Log: avia_gt_ir.c,v $
 *   Revision 1.19  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.18  2002/06/07 18:06:03  Jolt
 *   GCC31 fixes 2nd shot (GTX version) - sponsored by Frankster (THX!)
 *
 *   Revision 1.17  2002/06/07 17:53:45  Jolt
 *   GCC31 fixes 2nd shot - sponsored by Frankster (THX!)
 *
 *   Revision 1.16  2002/05/20 21:09:11  Jolt
 *   IR RX support
 *
 *   Revision 1.15  2002/05/13 22:38:36  Jolt
 *   GTX fixes
 *
 *   Revision 1.14  2002/05/11 22:46:16  Jolt
 *   IR fixes
 *
 *   Revision 1.13  2002/05/11 21:25:04  obi 
 *   export symbols for avia_gt_lirc 
 *
 *   Revision 1.12  2002/05/11 21:09:34  Jolt
 *   GTX stuff
 *
 *   Revision 1.11  2002/05/11 20:32:08  Jolt
 *   Some pre-GTX stuff
 *
 *   Revision 1.10  2002/05/11 20:23:22  Jolt
 *   DMA IR mode added
 *
 *   Revision 1.9  2002/05/11 15:28:00  Jolt
 *   IR improvements
 *
 *   Revision 1.8  2002/05/11 01:02:21  Jolt
 *   IR stuff now working for eNX - THX TMB!
 *
 *   Revision 1.7  2002/05/09 19:18:05  Jolt
 *   IR stuff
 *
 *   Revision 1.6  2002/05/08 13:26:58  Jolt
 *   Disable ir-dos :)
 *
 *   Revision 1.5  2002/05/07 19:41:02  Jolt
 *   IR tests
 *
 *   Revision 1.4  2002/05/07 17:03:48  Jolt
 *   Small fix
 *
 *   Revision 1.3  2002/05/07 16:59:19  Jolt
 *   Misc stuff and cleanups
 *
 *   Revision 1.2  2002/05/07 16:40:32  Jolt
 *   IR stuff
 *
 *   Revision 1.1  2002/04/01 22:28:09  Jolt
 *   Basic IR support for eNX - more 2 come
 *
 *
 *
 *   $Revision: 1.19 $
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/init.h>

#include <dbox/avia_gt.h>
#include <dbox/avia_gt_ir.h>

DECLARE_WAIT_QUEUE_HEAD(rx_wait);
DECLARE_WAIT_QUEUE_HEAD(tx_wait);

static sAviaGtInfo *gt_info = (sAviaGtInfo *)NULL;
u32 duty_cycle = 33;
u16 first_period_low = (u16)0;
u16 first_period_high = (u16)0;
u32 frequency = 38000;
//sAviaGtRxIrPulse *rx_buffer;
#define RX_MAX 50000
sAviaGtIrPulse rx_buffer[RX_MAX];
u32 rx_period_low = (u32)0;
u32 rx_period_high = (u32)0;
u32 rx_buffer_read_position = 0;
u32 rx_buffer_write_position = 0;
u8 rx_unit_busy = 0;
sAviaGtTxIrPulse *tx_buffer = (sAviaGtTxIrPulse *)NULL;
u8 tx_buffer_pulse_count = 0;
u8 tx_unit_busy = 0;

void avia_gt_ir_enable_rx_dma(unsigned char enable, unsigned char offset);

static void avia_gt_ir_tx_irq(unsigned short irq)
{

	dprintk("avia_gt_ir: tx irq\n");

	tx_unit_busy = 0;
	
	wake_up_interruptible(&tx_wait);

}

#define IR_TICK_LENGTH (1000 * 1125 / 8)

#define TICK_COUNT_TO_USEC(tick_count) ((tick_count) * IR_TICK_LENGTH / 1000)

struct timeval last_timestamp;

static void avia_gt_ir_rx_irq(unsigned short irq)
{

	struct timeval timestamp;
	
	dprintk("avia_gt_ir: rx irq\n");

    do_gettimeofday(&timestamp);
	
	rx_buffer[rx_buffer_write_position].high = enx_reg_s(RPH)->RTCH * IR_TICK_LENGTH / 1000;
	rx_buffer[rx_buffer_write_position].low = ((timestamp.tv_sec - last_timestamp.tv_sec) * 1000 * 1000) + (timestamp.tv_usec - last_timestamp.tv_usec) - rx_buffer[rx_buffer_write_position].high;

	last_timestamp = timestamp;
	
	if (rx_buffer_write_position < (RX_MAX - 1))
		rx_buffer_write_position++;
	else
		rx_buffer_write_position = 0;

	rx_unit_busy = 0;
	
	wake_up_interruptible(&rx_wait);
	
}

void avia_gt_ir_enable_rx_dma(unsigned char enable, unsigned char offset)
{

	if (avia_gt_chip(ENX)) {
	
    	enx_reg_set(IRRO, Offset, 0);
		
    	enx_reg_set(IRRE, Offset, offset >> 1);
	    enx_reg_set(IRRE, E, enable);
		
	} else if (avia_gt_chip(GTX)) {

    	gtx_reg_set(IRRO, Offset, 0);
		
    	gtx_reg_set(IRRE, Offset, offset >> 1);
	    gtx_reg_set(IRRE, E, enable);
	
	}

}

void avia_gt_ir_enable_tx_dma(unsigned char enable, unsigned char length)
{

	if (avia_gt_chip(ENX)) {
	
		enx_reg_set(IRTO, Offset, 0);
	
		enx_reg_set(IRTE, Offset, length - 1);
		enx_reg_set(IRTE, C, 0);
		enx_reg_set(IRTE, E, enable);
		
	} else if (avia_gt_chip(GTX)) {

		gtx_reg_set(IRTO, Offset, 0);
	
		gtx_reg_set(IRTE, Offset, length - 1);
		gtx_reg_set(IRTE, C, 0);
		gtx_reg_set(IRTE, E, enable);
	
	}

}

u32 avia_gt_ir_get_rx_buffer_read_position(void)
{

	return rx_buffer_read_position;
	
}

u32 avia_gt_ir_get_rx_buffer_write_position(void)
{

	return rx_buffer_write_position;
	
}

int avia_gt_ir_queue_pulse(u32 period_high, u32 period_low, u8 block)
{

	WAIT_IR_UNIT_READY(tx);
	
	if (tx_buffer_pulse_count >= AVIA_GT_IR_MAX_PULSE_COUNT)
		return -EBUSY;
		
	if (tx_buffer_pulse_count == 0) {
	
		first_period_high = period_high;
		first_period_low = period_low;
	
	} else {
	
		tx_buffer[tx_buffer_pulse_count - 1].MSPR = USEC_TO_CWP(period_high + period_low) - 1;

		if (period_low != 0)
			tx_buffer[tx_buffer_pulse_count - 1].MSPL = USEC_TO_CWP(period_low) - 1;
		else
			tx_buffer[tx_buffer_pulse_count - 1].MSPL = 0;
			// Mhhh doesnt work :(
			//tx_buffer[tx_buffer_pulse_count - 1].MSPL = USEC_TO_CWP(period_high) - 1;

	}
	
	tx_buffer_pulse_count++;
	
	return 0;
	
}

wait_queue_head_t *avia_gt_ir_receive_data(void)
{

	return &rx_wait;

}

int avia_gt_ir_receive_pulse(u32 *period_low, u32 *period_high, u8 block)
{

	if (rx_buffer_write_position == rx_buffer_read_position) {
	
		rx_unit_busy = 1;
		WAIT_IR_UNIT_READY(rx);

	}
	
	if (period_low)
		*period_low = rx_buffer[rx_buffer_read_position].low;
		
	if (period_high)
		*period_high = rx_buffer[rx_buffer_read_position].high;

	if (rx_buffer_read_position < RX_MAX)
		rx_buffer_read_position++;
	else
		rx_buffer_read_position = 0;

	return 0;
	
}

void avia_gt_ir_reset(unsigned char reenable)
{

	if (avia_gt_chip(ENX))
        enx_reg_set(RSTR0, IR, 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(RR0, IR, 1);
						
    if (reenable) {

		if (avia_gt_chip(ENX))
	        enx_reg_set(RSTR0, IR, 0);
		else if (avia_gt_chip(GTX))
			gtx_reg_set(RR0, IR, 0);

	}

}

int avia_gt_ir_send_buffer(u8 block)
{

	WAIT_IR_UNIT_READY(tx);

	if (tx_buffer_pulse_count == 0)
		return 0;
	
	if (tx_buffer_pulse_count >= 2)
		avia_gt_ir_enable_tx_dma(1, tx_buffer_pulse_count);

	avia_gt_ir_send_pulse(first_period_high, first_period_low, block);
	
	tx_buffer_pulse_count = 0;
	
	return 0;
	
}

int avia_gt_ir_send_pulse(u32 period_high, u32 period_low, u8 block)
{

	WAIT_IR_UNIT_READY(tx);

	tx_unit_busy = 1;
					
	if (avia_gt_chip(ENX)) {

		// Verify this	
		if (period_low != 0)
			enx_reg_16(MSPL) = USEC_TO_CWP(period_low) - 1;
		else
			enx_reg_16(MSPL) = USEC_TO_CWP(period_high) - 1;
			
		enx_reg_16(MSPR) = (1 << 10) | (USEC_TO_CWP(period_high + period_low) - 1);

	} else if (avia_gt_chip(GTX)) {

		// Verify this	
		if (period_low != 0)
			gtx_reg_16(MSPL) = USEC_TO_CWP(period_low) - 1;
		else
			gtx_reg_16(MSPL) = USEC_TO_CWP(period_high) - 1;
			
		gtx_reg_16(MSPR) = (1 << 10) | (USEC_TO_CWP(period_high + period_low) - 1);
	
	}
	
	return 0;

}

void avia_gt_ir_set_duty_cycle(u32 new_duty_cycle)
{

	duty_cycle = new_duty_cycle;

	if (avia_gt_chip(ENX))
		enx_reg_set(CWPH, WavePulseHigh, ((AVIA_GT_ENX_IR_CLOCK * duty_cycle) / (frequency * 100)) - 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(CWPH, WavePulseHigh, ((AVIA_GT_GTX_IR_CLOCK * duty_cycle) / (frequency * 100)) - 1);

}

void avia_gt_ir_set_frequency(u32 new_frequency)
{

	frequency = new_frequency;

	if (avia_gt_chip(ENX))
		enx_reg_set(CWP, CarrierWavePeriod, (AVIA_GT_ENX_IR_CLOCK / frequency) - 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(CWP, CarrierWavePeriod, (AVIA_GT_GTX_IR_CLOCK / frequency) - 1);

	avia_gt_ir_set_duty_cycle(duty_cycle);

}

void avia_gt_ir_set_filter(u8 enable, u8 low, u8 high)
{

	if (avia_gt_chip(ENX)) {
	
	    enx_reg_set(RFR, Filt_H, high);
	    enx_reg_set(RFR, Filt_L, low);
    
	    enx_reg_set(RTC, S, enable);
	
	} else if (avia_gt_chip(GTX)) {

	    gtx_reg_set(RFR, Filt_H, high);
	    gtx_reg_set(RFR, Filt_L, low);
    
	    gtx_reg_set(RTC, S, enable);

	}

}

void avia_gt_ir_set_polarity(u8 polarity)
{

	if (avia_gt_chip(ENX))
	    enx_reg_set(RFR, P, polarity);
	else if (avia_gt_chip(GTX))
	    gtx_reg_set(RFR, P, polarity);

}

// Given in usec / 1000
void avia_gt_ir_set_tick_period(u32 tick_period)
{

	if (avia_gt_chip(ENX))
	    enx_reg_set(RTP, TickPeriod, (1000 * 1000 * 1000 / tick_period) - 1);
	else if (avia_gt_chip(GTX))
	    gtx_reg_set(RTP, TickPeriod, (1000 * 1000 * 1000 / tick_period) - 1);

}

void avia_gt_ir_set_queue(unsigned int addr)
{

	if (avia_gt_chip(ENX))
	    enx_reg_set(IRQA, Addr, addr >> 9);
	else if (avia_gt_chip(GTX))
	    gtx_reg_set(IRQA, Address, addr >> 9);

	//rx_buffer = (sAviaGtRxIrPulse *)(gt_info->mem_addr + addr);
	tx_buffer = (sAviaGtTxIrPulse *)(gt_info->mem_addr + addr + 256);
    
}

int __init avia_gt_ir_init(void)
{
	u16 rx_irq = 0;
	u16 tx_irq = 0;

    printk("avia_gt_ir: $Id: avia_gt_ir.c,v 1.19 2002/08/22 13:39:33 Jolt Exp $\n");

    do_gettimeofday(&last_timestamp);
	
	gt_info = avia_gt_get_info();
		
	if ((!gt_info) || ((!avia_gt_chip(ENX)) && (!avia_gt_chip(GTX)))) {
			
		printk("avia_gt_ir: Unsupported chip type\n");
					
		return -EIO;
							
	}
	
	if (avia_gt_chip(ENX)) {
	
		rx_irq = ENX_IRQ_IR_RX;
		tx_irq = ENX_IRQ_IR_TX;
	
	} else if (avia_gt_chip(GTX)) {

		rx_irq = GTX_IRQ_IR_RX;
		tx_irq = GTX_IRQ_IR_TX;
	
	}

	// For testing only
	avia_gt_free_irq(rx_irq);	
	avia_gt_free_irq(tx_irq);	
	
    if (avia_gt_alloc_irq(rx_irq, avia_gt_ir_rx_irq)) {

		printk("avia_gt_ir: unable to get rx interrupt\n");

		return -EIO;
	
    }
	
    if (avia_gt_alloc_irq(tx_irq, avia_gt_ir_tx_irq)) {

		printk("avia_gt_ir: unable to get tx interrupt\n");

		avia_gt_free_irq(rx_irq);
	
		return -EIO;
	
    }
		
	avia_gt_ir_reset(1);
	
    avia_gt_ir_set_tick_period(IR_TICK_LENGTH);
    avia_gt_ir_set_filter(0, 3, 5);
    avia_gt_ir_set_polarity(0);
    avia_gt_ir_set_queue(AVIA_GT_MEM_IR_OFFS);
	avia_gt_ir_set_frequency(frequency);
	
    return 0;
    
}

void __exit avia_gt_ir_exit(void)
{

	if (avia_gt_chip(ENX)) {
	
		avia_gt_free_irq(ENX_IRQ_IR_TX);
		avia_gt_free_irq(ENX_IRQ_IR_RX);
		
	} else if (avia_gt_chip(GTX)) {

		avia_gt_free_irq(GTX_IRQ_IR_TX);
		avia_gt_free_irq(GTX_IRQ_IR_RX);

	}
    
	avia_gt_ir_reset(0);

}

#ifdef MODULE
EXPORT_SYMBOL(avia_gt_ir_get_rx_buffer_read_position);
EXPORT_SYMBOL(avia_gt_ir_get_rx_buffer_write_position);
EXPORT_SYMBOL(avia_gt_ir_queue_pulse);
EXPORT_SYMBOL(avia_gt_ir_receive_data);
EXPORT_SYMBOL(avia_gt_ir_receive_pulse);
EXPORT_SYMBOL(avia_gt_ir_send_buffer);
EXPORT_SYMBOL(avia_gt_ir_send_pulse);
EXPORT_SYMBOL(avia_gt_ir_set_duty_cycle);
EXPORT_SYMBOL(avia_gt_ir_set_frequency);
#endif

#if defined(MODULE) && defined(STANDALONE)
module_init(avia_gt_ir_init);
module_exit(avia_gt_ir_exit);
#endif
