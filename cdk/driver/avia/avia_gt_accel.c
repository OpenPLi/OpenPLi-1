/*
 *   avia_gt_accel.c - AViA accelerator driver (dbox-II-project)
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
 *   $Log: avia_gt_accel.c,v $
 *   Revision 1.8  2002/10/13 21:16:15  Jolt
 *   HW copy
 *
 *   Revision 1.7  2002/10/09 18:31:06  Jolt
 *   HW copy support
 *
 *   Revision 1.6  2002/09/19 21:09:20  Jolt
 *   Removed old sw crc dependency
 *
 *   Revision 1.5  2002/09/13 22:53:55  Jolt
 *   HW CRC support
 *
 *   Revision 1.4  2002/09/13 21:37:30  Jolt
 *   Fixed GTX HW-CRC
 *
 *   Revision 1.3  2002/08/27 20:18:45  Jolt
 *   Engine is working now for GTX and eNX (except for section data :-( )
 *
 *   Revision 1.2  2002/06/07 17:53:45  Jolt
 *   GCC31 fixes 2nd shot - sponsored by Frankster (THX!)
 *
 *   Revision 1.1  2002/04/01 22:29:11  Jolt
 *   HW accelerated functions for eNX
 *
 *
 *
 *   $Revision: 1.8 $
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <asm/errno.h>

#include <dbox/avia_gt.h>
#include <dbox/avia_gt_accel.h>

static sAviaGtInfo *gt_info = NULL;
static u8 max_transaction_size = 0;

void avia_gt_accel_copy(u32 buffer_src, u32 buffer_dst, u32 buffer_size, u8 decrement)
{

	u32 transaction_size;

	if (avia_gt_chip(ENX)) {
	
		enx_reg_set(CPCSRC1, Addr, buffer_src);
		enx_reg_set(CPCDST, Addr, buffer_dst);

	} else if (avia_gt_chip(GTX)) {
	
		gtx_reg_set(CCSA, Addr, buffer_src);
		gtx_reg_set(CDA, Addr, buffer_dst);
	
	}

    while (buffer_size) {
    
		if (buffer_size > max_transaction_size)
			transaction_size = max_transaction_size;
		else
			transaction_size = buffer_size;

		if (avia_gt_chip(ENX)) {
		
			enx_reg_16(CPCCMD) = ((!!decrement << 10)) | (3 << 8) | (transaction_size - 1);
			
		} else if (avia_gt_chip(GTX)) {
		
			if (((buffer_src & 1) || (buffer_dst & 1)) && (transaction_size == max_transaction_size))
				transaction_size -= 2;
	
			gtx_reg_16(CCOM) = ((!!decrement << 10)) | (1 << 9) | (1 << 8) | (transaction_size - 1);
			
		}
		
		buffer_size -= transaction_size;

	}
	
} 

u32 avia_gt_accel_crc32(u32 buffer, u32 buffer_size, u32 seed)
{

	u32 transaction_size;
	u8 odd_end_padding = 0;
	u8 odd_start_padding = 0;

	if (avia_gt_chip(ENX)) {
	
		//enx_reg_s(CPCCRCSRC2)->CRC.CRC = 0;
		if (seed)
			enx_reg_32(CPCCRCSRC2) = seed ^ 0xFFFFFFFF;
		else
			enx_reg_32(CPCCRCSRC2) = 0;
			
		enx_reg_set(CPCSRC1, Addr, buffer);
    
		/*enx_reg_set(CPCCMD, W, 0);
		enx_reg_set(CPCCMD, C, 1);
		enx_reg_set(CPCCMD, P, 0);
		enx_reg_set(CPCCMD, N, 0);
		enx_reg_set(CPCCMD, T, 0);
		enx_reg_set(CPCCMD, D, 0);*/
		
	} else if (avia_gt_chip(GTX)) {
	
		if (seed) {
		
			gtx_reg_set(RCRC, CRC, seed ^ 0xFFFFFFFF);
			
		} else {
		
			gtx_reg_set(RCRC, CRC, *((u32*)&gt_info->mem_addr[buffer]));
			
			buffer += 4;
			buffer_size -= 4;
			
		}

		if ((buffer & 1) || (!buffer_size))
			odd_start_padding = 1;

		if ((buffer + buffer_size) & 1)
			odd_end_padding = 1;
			
		buffer_size += odd_start_padding + odd_end_padding;
		
	}

    while (buffer_size) {
    
		if (buffer_size > max_transaction_size)
			transaction_size = max_transaction_size;
		else
			transaction_size = buffer_size;

		if (avia_gt_chip(ENX)) {
	
		    //enx_reg_set(CPCCMD, Len, transaction_size);
		    enx_reg_16(CPCCMD) = (1 << 14) | (transaction_size - 1);
	    
		} else if (avia_gt_chip(GTX)) {

			gtx_reg_32(CRCC) = (((transaction_size / 2) + (transaction_size % 2) - 1) << 25) | ((!(odd_end_padding && (buffer_size == transaction_size))) << 24) | buffer;

			if (odd_start_padding) {
		
				odd_start_padding = 0;
				buffer += transaction_size - 1;
		
			} else {

				buffer += transaction_size;
				
			}

		}

    	buffer_size -= transaction_size;
	
    }

	if (avia_gt_chip(ENX))
	    //return enx_reg_s(CPCCRCSRC2)->CRC.CRC;
    	return (enx_reg_32(CPCCRCSRC2) ^ 0xFFFFFFFF);
	else if (avia_gt_chip(GTX))
		return (gtx_reg_32(RCRC) ^ 0xFFFFFFFF);
		
	return 0;	

}

int __init avia_gt_accel_init(void)
{

    printk("avia_gt_accel: $Id: avia_gt_accel.c,v 1.8 2002/10/13 21:16:15 Jolt Exp $\n");

	gt_info = avia_gt_get_info();
	
	if ((!gt_info) || ((!avia_gt_chip(ENX)) && (!avia_gt_chip(GTX)))) {
		
		printk("avia_gt_accel: Unsupported chip type\n");
				
		return -EIO;
						
	}

	if (avia_gt_chip(ENX)) {
	
	    enx_reg_set(RSTR0, COPY, 0);
		
		max_transaction_size = 64;
		
	} else if (avia_gt_chip(GTX)) {
	
		gtx_reg_set(RR0, CRC, 0);
		
		max_transaction_size = 16;
		
	}
	
    return 0;
    
}

void __exit avia_gt_accel_exit(void)
{

	if (avia_gt_chip(ENX))
	    enx_reg_set(RSTR0, COPY, 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(RR0, CRC, 1);

}

#ifdef MODULE
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
EXPORT_SYMBOL(avia_gt_accel_copy);
EXPORT_SYMBOL(avia_gt_accel_crc32);
#endif

#if defined(MODULE) && defined(STANDALONE)
module_init(avia_gt_accel_init);
module_exit(avia_gt_accel_exit);
#endif
