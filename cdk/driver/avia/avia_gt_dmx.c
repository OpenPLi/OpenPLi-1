/*
 *   avia_gt_dmx.c - AViA eNX/GTX dmx driver (dbox-II-project)
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
 *   $Log: avia_gt_dmx.c,v $
 *   Revision 1.139.2.1  2002/11/17 01:59:13  obi
 *   "backport" of latest dvb api version 1 drivers from HEAD branch
 *
 *   Revision 1.141  2002/11/11 21:35:32  wjoost
 *   SPTS: Software-Demultiplexing Audio-/Video-Queue
 *
 *   Revision 1.140  2002/11/04 20:43:36  wjoost
 *   IRQ-Handling fuers streamen geaendert
 *
 *   Revision 1.139  2002/10/09 20:58:31  Jolt
 *   Fix
 *
 *   Revision 1.138  2002/10/09 20:20:36  Jolt
 *   DMX & Section fixes
 *
 *   Revision 1.137  2002/10/09 09:42:59  Jolt
 *   Some small fixes
 *
 *   Revision 1.136  2002/10/08 13:47:52  Jolt
 *   Queue handling changes
 *
 *   Revision 1.135  2002/09/30 19:46:10  Jolt
 *   SPTS support
 *
 *   Revision 1.134  2002/09/21 00:00:08  Jolt
 *   Some queue changes
 *
 *   Revision 1.133  2002/09/19 11:37:01  Jolt
 *   Fixes
 *
 *   Revision 1.132  2002/09/18 15:57:24  Jolt
 *   Queue handling changes #3
 *
 *   Revision 1.131  2002/09/18 14:13:58  obi
 *   enable hw sections for ucode_0013
 *
 *   Revision 1.130  2002/09/18 13:17:28  Ghostrider
 *   fix Jolts fix
 *
 *   Revision 1.129  2002/09/18 12:13:20  Jolt
 *   Queue handling changes #2
 *
 *   Revision 1.128  2002/09/18 09:57:42  Jolt
 *   Queue handling changes
 *
 *   Revision 1.127  2002/09/18 09:26:58  Jolt
 *   Fixes
 *
 *   Revision 1.126  2002/09/17 18:06:09  Jolt
 *   DMX fixes and cleanups
 *
 *   Revision 1.125  2002/09/13 22:53:55  Jolt
 *   HW CRC support
 *
 *   Revision 1.124  2002/09/13 19:00:49  Jolt
 *   Changed queue handling
 *
 *   Revision 1.123  2002/09/13 17:07:44  Jolt
 *   Fixed section mode selection:
 *   0 - disabled
 *   1 - autodetect
 *   2 - force
 *
 *   Revision 1.122  2002/09/10 17:18:31  Jolt
 *   Grrrrr
 *
 *   Revision 1.121  2002/09/10 16:31:38  Jolt
 *   SW sections fix
 *
 *   Revision 1.120  2002/09/10 13:44:44  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.119  2002/09/09 18:30:36  Jolt
 *   Symbol fix
 *
 *   Revision 1.118  2002/09/08 16:15:22  Jolt
 *   DMX fixes
 *
 *   Revision 1.117  2002/09/08 13:02:49  Jolt
 *   DMX fixes
 *
 *   Revision 1.116  2002/09/05 18:16:13  Jolt
 *   Fixes
 *
 *   Revision 1.115  2002/09/05 17:08:08  Jolt
 *   DMX fix
 *
 *   Revision 1.114  2002/09/05 13:00:30  Jolt
 *   DMX sanity checks
 *
 *   Revision 1.113  2002/09/05 12:42:51  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.112  2002/09/05 10:35:13  Jolt
 *   Test
 *
 *   Revision 1.111  2002/09/05 10:00:47  Jolt
 *   DMX bugfix
 *
 *   Revision 1.110  2002/09/05 09:40:31  Jolt
 *   - DMX/NAPI cleanup
 *   - Bugfixes (Thanks obi)
 *
 *   Revision 1.109  2002/09/04 22:40:46  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.108  2002/09/04 22:07:40  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.107  2002/09/04 21:12:52  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.106  2002/09/04 14:26:49  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.105  2002/09/04 13:25:01  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.104  2002/09/03 21:00:34  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.103  2002/09/03 20:24:29  obi
 *   tp_pcr/stc/dir/diff: printk -> dprintk
 *
 *   Revision 1.102  2002/09/03 15:37:50  wjoost
 *   Ein Bug weniger
 *
 *   Revision 1.101  2002/09/03 14:02:05  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.100  2002/09/03 13:17:34  Jolt
 *   - DMX/NAPI cleanup
 *   - HW sections workaround
 *
 *   Revision 1.99  2002/09/02 19:25:37  Jolt
 *   - DMX/NAPI cleanup
 *   - Compile fix
 *
 *   Revision 1.98  2002/08/27 21:53:09  Jolt
 *   New sync logic
 *
 *   Revision 1.97  2002/08/25 22:14:54  Jolt
 *   Sync logic is broken :(
 *
 *   Revision 1.96  2002/08/25 20:33:19  Jolt
 *   Enabled basic sync mode
 *
 *   Revision 1.95  2002/08/25 10:19:33  Jolt
 *   HW sections can be disabled
 *
 *   Revision 1.94  2002/08/25 09:38:26  wjoost
 *   Hardware Section Filtering
 *
 *   Revision 1.93  2002/08/24 00:14:19  Jolt
 *   PCR stuff (currently no sync logic)
 *
 *   Revision 1.92  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.91  2002/07/08 15:12:47  wjoost
 *
 *   ein paar nicht benutzte Felder initialisiert (wie in avia_gt_napi.c 1.88)
 *
 *   Revision 1.90  2002/07/07 16:55:42  wjoost
 *
 *   Meine Sagem hustet mir sonst was :-(
 *
 *   Revision 1.89  2002/06/11 20:35:43  Jolt
 *   Sections cleanup
 *
 *   Revision 1.88  2002/06/07 18:06:03  Jolt
 *   GCC31 fixes 2nd shot (GTX version) - sponsored by Frankster (THX!)
 *
 *   Revision 1.87  2002/06/07 17:53:45  Jolt
 *   GCC31 fixes 2nd shot - sponsored by Frankster (THX!)
 *
 *   Revision 1.86  2002/05/09 07:29:21  waldi
 *   add correct license
 *
 *   Revision 1.85  2002/05/08 03:47:26  obi
 *   changed PRCPID to PCRPID
 *
 *   Revision 1.84  2002/05/07 16:59:19  Jolt
 *   Misc stuff and cleanups
 *
 *   Revision 1.83  2002/05/03 22:09:58  Jolt
 *   Do not require ucodes for framebuffer mode
 *
 *   Revision 1.82  2002/05/03 21:52:58  Jolt
 *   YEAH YEAH YEAH :(
 *
 *   Revision 1.81  2002/05/03 16:45:17  obi
 *   replaced r*() by gtx_reg_*()
 *   formatted source
 *
 *   Revision 1.80  2002/05/02 21:26:01  Jolt
 *   Test
 *
 *   Revision 1.79  2002/05/02 20:23:10  Jolt
 *   Fixes
 *
 *   Revision 1.78  2002/05/02 12:37:35  Jolt
 *   Merge
 *
 *   Revision 1.77  2002/05/02 04:56:47  Jolt
 *   Merge
 *
 *   Revision 1.76  2002/05/01 21:53:00  Jolt
 *   Merge
 *
 *
 *
 *
 *   $Revision: 1.139.2.1 $
 *
 */

#define __KERNEL_SYSCALLS__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/byteorder/swab.h>

#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/unistd.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#include <linux/tqueue.h>
#include <asm/8xx_immap.h>
#include <asm/pgtable.h>
#include <asm/mpc8xx.h>
#include <asm/div64.h>

//#define DEBUG
#include <ost/demux.h>
#include <dbox/avia_gt.h>
#include <dbox/avia_gt_dmx.h>
#include <dbox/avia_gt_accel.h>
#include <dbox/avia_gt_napi.h>

//#define dprintk printk

static void avia_gt_dmx_queue_task(void *tl_data);

struct tq_struct avia_gt_dmx_queue_tasklet = {

	routine: avia_gt_dmx_queue_task,
	data: 0

};

static int errno							= (int)0;
static sAviaGtInfo *gt_info						= (sAviaGtInfo *)NULL;
static sRISC_MEM_MAP *risc_mem_map				= (sRISC_MEM_MAP *)NULL;
static char *ucode							= (char *)NULL;
static s32 hw_sections = 1;
static u8 force_stc_reload = 0;
static sAviaGtDmxQueue queue_list[AVIA_GT_DMX_QUEUE_COUNT];
extern void avia_set_pcr(u32 hi, u32 lo);
static void gtx_pcr_interrupt(unsigned short irq);

static const u8 queue_size_table[AVIA_GT_DMX_QUEUE_COUNT] =	{	// sizes are 1<<x*64bytes. BEWARE OF THE ALIGNING!
																// DO NOT CHANGE UNLESS YOU KNOW WHAT YOU'RE DOING!
	10,						// video
	9,						// audio
	9,						// teletext
	10, 10, 10, 10, 10,		// user 3..7
	8, 8, 8, 8, 8, 8, 8, 8,	// user 8..15
	8, 8, 8, 8, 7, 7, 7, 7,	// user 16..23
	7, 7, 7, 7, 7, 7, 7,	// user 24..30
	7						// message

};

static const u8 queue_system_map[] = {2, 0, 1};

#if 0
static void avia_gt_dmx_dump(void) {
	int i;
	unsigned char *ptr;

	ptr = (unsigned char *) risc_mem_map;
	ptr += 1024;

	for (i = 0; i < 1024; i++) {
		if ( (i & 0x0F) == 0) {
			printk("\n" KERN_INFO "%04X ",i + 0x400);
		}
		printk("%02X ",*(ptr++));
	}
	printk("\n");
}
#endif

s32 avia_gt_dmx_alloc_queue(u8 queue_nr, AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *priv_data)
{

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: alloc_queue: queue %d out of bounce\n", queue_nr);

		return -EINVAL;

	}

	if (queue_list[queue_nr].busy) {

		printk("avia_gt_dmx: alloc_queue: queue %d busy\n", queue_nr);

		return -EBUSY;

	}

	queue_list[queue_nr].busy = 1;
	queue_list[queue_nr].cb_proc = cb_proc;
//	queue_list[queue_nr].hw_read_pos = 0;
//	queue_list[queue_nr].hw_write_pos = 0;
//	queue_list[queue_nr].irq_count = 0;
	queue_list[queue_nr].irq_proc = irq_proc;
	queue_list[queue_nr].priv_data = priv_data;
	queue_list[queue_nr].read_pos = 0;
	queue_list[queue_nr].write_pos = 0;
	queue_list[queue_nr].is_qim = 0;

	return queue_nr;

}

s32 avia_gt_dmx_alloc_queue_audio(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *priv_data)
{

    return avia_gt_dmx_alloc_queue(AVIA_GT_DMX_QUEUE_AUDIO, irq_proc, cb_proc, priv_data);

}

s32 avia_gt_dmx_alloc_queue_message(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *priv_data)
{

    return avia_gt_dmx_alloc_queue(AVIA_GT_DMX_QUEUE_MESSAGE, irq_proc, cb_proc, priv_data);

}

s32 avia_gt_dmx_alloc_queue_teletext(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *priv_data)
{

    return avia_gt_dmx_alloc_queue(AVIA_GT_DMX_QUEUE_TELETEXT, irq_proc, cb_proc, priv_data);

}

s32 avia_gt_dmx_alloc_queue_user(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *priv_data)
{

    u8 queue_nr;

    for (queue_nr = AVIA_GT_DMX_QUEUE_USER_START; queue_nr <= AVIA_GT_DMX_QUEUE_USER_END; queue_nr++) {

		if (!queue_list[queue_nr].busy)
			return avia_gt_dmx_alloc_queue(queue_nr, irq_proc, cb_proc, priv_data);

	}

	return -EBUSY;

}

s32 avia_gt_dmx_alloc_queue_video(AviaGtDmxQueueProc *irq_proc, AviaGtDmxQueueProc *cb_proc, void *priv_data)
{

	return avia_gt_dmx_alloc_queue(AVIA_GT_DMX_QUEUE_VIDEO, irq_proc, cb_proc, priv_data);

}

s32 avia_gt_dmx_free_queue(u8 queue_nr)
{

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: free_queue: queue %d out of bounce\n", queue_nr);

		return -EINVAL;

	}

	if (!queue_list[queue_nr].busy) {

		printk("avia_gt_dmx: free_queue: queue %d not busy\n", queue_nr);

		return -EFAULT;

	}

	avia_gt_dmx_queue_irq_disable(queue_nr);

	queue_list[queue_nr].busy = 0;
	queue_list[queue_nr].cb_proc = NULL;
	queue_list[queue_nr].irq_count = 0;
	queue_list[queue_nr].irq_proc = NULL;
	queue_list[queue_nr].priv_data = NULL;
    
	return 0;

}

u8 avia_gt_dmx_get_hw_sec_filt_avail(void)
{

	if ((hw_sections == 1) && (risc_mem_map->Version_no[0] == 0x00)) {
	
		switch (risc_mem_map->Version_no[1]) {
		
			case 0x13:
			case 0x14:

				return 1;

			break;
			
		}
		
	} else if (hw_sections == 2) {
	
		return 1;
		
	}

	return 0;

}

void avia_gt_dmx_release_section_filter(void *v_gtx, unsigned entry)
{
	gtx_demux_t *gtx = v_gtx;
	sPID_Parsing_Control_Entry e;
	unsigned i;

	dprintk("avia_gt_dmx_release_section_filter, entry %d\n",entry);

	*((u32 *) &e) = *((u32 *) &risc_mem_map->PID_Parsing_Control_Table[entry]);
	e.no_of_filter = 0;
	e.filt_tab_idx = entry;
	e._PSH = 1;
	*((u32 *) &risc_mem_map->PID_Parsing_Control_Table[entry]) = *((u32 *) &e);
	avia_gt_dmx_set_filter_definition_table(entry,1,31);

	for (i = 0; i < 31; i++)
	{
		if (gtx->filter_definition_table_entry_user[i] == entry)
		{
			gtx->filter_definition_table_entry_user[i] = -1;
		}
	}
}

int avia_gt_dmx_start_stop_feed(unsigned entry, unsigned what)
{

	sPID_Entry e;
	int rc;

	*((u16 *) &e) = *((u16 *) &risc_mem_map->PID_Search_Table[entry]);
	rc = e.VALID;

	dprintk("avia_gt_dmx_start_stop_feed, entry %d, what %d, old %d\n",entry,what,rc);

	if (what != e.VALID) {

		e.VALID = what;
		e.wait_pusi = 1;
		*((u16 *) &risc_mem_map->PID_Search_Table[entry]) = *((u16 *) &e);
		
	}

	return rc;

}


int avia_gt_dmx_compress_filter_parameter_table(gtx_demux_t *gtx)
{

	unsigned char valid[32];
	unsigned i;
	unsigned j;
	unsigned start;
	unsigned stop;
	sPID_Parsing_Control_Entry e;
	sPID_Entry f;

	/*
	 * stop all feeds with type GTX_OUTPUT_8BYTE.
	 */


	dprintk("avia_gt_dmx_compress_filter_parameter_table\n");

	i = 0;

	while (i < 32)
	{
		*((u32 *) &e) = *((u32 *) &risc_mem_map->PID_Parsing_Control_Table[i]);
		*((u16 *) &f) = *((u16 *) &risc_mem_map->PID_Search_Table[i]);
		if ( (e.type == 4) && (f.VALID == 0) ) {
			valid[i] = 0;
			avia_gt_dmx_start_stop_feed(i,1);
			queue_list[i].read_pos = avia_gt_dmx_queue_get_write_pos(gtx->feed[i].index);
			avia_gt_dmx_queue_set_write_pos(gtx->feed[i].index, queue_list[i].read_pos);
			gtx->feed[i].sec_len = 0;
			gtx->feed[i].sec_recv = 0;
		}
		else
		{
			valid[i] = 1;
		}
		i++;
	}

	/*
	 * compress
	 */

	for (i = 0; (i < 31) && (gtx->filter_definition_table_entry_user[i] != -1); i++);

	start = i;

	j = start + 1;
	while (j <31)
	{
		if (gtx->filter_definition_table_entry_user[j] != -1)
		{
			gtx->filter_definition_table_entry_user[i] = gtx->filter_definition_table_entry_user[j];
			gtx->filter_definition_table_entry_user[j] = -1;
			*(((u16 *) &risc_mem_map->Filter_Parameter_Table1[i]) + 0) = *(((u16 *) &risc_mem_map->Filter_Parameter_Table1[j]) + 0);
			*(((u16 *) &risc_mem_map->Filter_Parameter_Table1[i]) + 1) = *(((u16 *) &risc_mem_map->Filter_Parameter_Table1[j]) + 1);
			*(((u16 *) &risc_mem_map->Filter_Parameter_Table1[i]) + 2) = *(((u16 *) &risc_mem_map->Filter_Parameter_Table1[j]) + 2);
			*(((u16 *) &risc_mem_map->Filter_Parameter_Table2[i]) + 0) = *(((u16 *) &risc_mem_map->Filter_Parameter_Table2[j]) + 0);
			*(((u16 *) &risc_mem_map->Filter_Parameter_Table2[i]) + 1) = *(((u16 *) &risc_mem_map->Filter_Parameter_Table2[j]) + 1);
			*(((u16 *) &risc_mem_map->Filter_Parameter_Table2[i]) + 2) = *(((u16 *) &risc_mem_map->Filter_Parameter_Table2[j]) + 2);
			*(((u16 *) &risc_mem_map->Filter_Parameter_Table3[i]) + 0) = *(((u16 *) &risc_mem_map->Filter_Parameter_Table3[j]) + 0);
			*(((u16 *) &risc_mem_map->Filter_Parameter_Table3[i]) + 1) = *(((u16 *) &risc_mem_map->Filter_Parameter_Table3[j]) + 1);
			*(((u16 *) &risc_mem_map->Filter_Parameter_Table3[i]) + 2) = *(((u16 *) &risc_mem_map->Filter_Parameter_Table3[j]) + 2);
			i++;
		}
		j++;
	}
	stop = i;

	dprintk("start = %d, stop =%d\n",start,stop);

	/*
	 * update filter_definition_table
	 */

	j = -1;
	for (i = start; i < stop; i++)
	{
		if (gtx->filter_definition_table_entry_user[i] != j) {
			avia_gt_dmx_set_filter_definition_table(gtx->filter_definition_table_entry_user[i],1,i);
			j = i;
		}
	}

	/*
	 * start stopped feeds
	 */

	for (i = 0; i < 32; i++)
	{
		if (valid[i] == 0)
		{
			*((u32 *) &e) = *((u32 *) &risc_mem_map->PID_Parsing_Control_Table[i]);
			e.start_up = 1;
			*((u32 *) &risc_mem_map->PID_Parsing_Control_Table[i]) = *((u32 *) &e);
			avia_gt_dmx_start_stop_feed(i,0);
		}
	}

	return stop;
}

int avia_gt_dmx_set_filter_definition_table(u8 entry, u8 and_or_flag, u8 filter_param_id)
{
	sFilter_Definition_Entry e;
	u16 w,d;

	if (entry > 31) {

		printk("avia_gt_napi: pid control table entry out of bounce (entry=%d)!\n", entry);

		return -EINVAL;

	}

	e.and_or_flag = !!and_or_flag;
	e.filter_param_id = filter_param_id;
	e.Reserved = 0;

	/* my sagem doesn't like 8bit write access */

	w = *((u16 *) &risc_mem_map->Filter_Definition_Table[entry & 0xFE]);
	d = w;
	if (entry & 0x01)
	{
		w = (w & 0xFF00) | *((u8 *) &e);
	}
	else
	{
		w = (w & 0x00FF) | (*((u8 *) &e) << 8);
	}

	dprintk("avia_gt_dmx_set_filter_definition_table, entry %d, and_or_flag %d, filter_param_id %d, old=%04X, new=%04X\n",
		entry,and_or_flag,filter_param_id,d,w);


	*((u16 *) &risc_mem_map->Filter_Definition_Table[entry & 0xFE]) = w;

	return 0;

}

int avia_gt_dmx_set_filter_parameter_table(u8 entry, u8 mask[], u8 param[], u8 not_flag, u8 not_flag_ver_id_byte)
{
	sFilter_Parameter_Entry1 e1;
	sFilter_Parameter_Entry2 e2;
	sFilter_Parameter_Entry3 e3;

	if (entry > 31) {

		printk("avia_gt_napi: pid control table entry out of bounce (entry=%d)!\n", entry);

		return -EINVAL;

	}

	dprintk("avia_gt_dmx_set_filter_parameter_table, entry %d\n",entry);

	e1.mask_0 = mask[0];
	e1.param_0 = param[0];
	e1.mask_1 = mask[3];	// mask and param contain the section-length bytes,
	e1.param_1 = param[3];	// but the dmx-ucode skips these bytes
	e1.mask_2 = mask[4];
	e1.param_2 = param[4];

	e2.mask_3 = mask[5];
	e2.param_3 = param[5];
	e2.mask_4 = mask[6];
	e2.param_4 = param[6];
	e2.mask_5 = mask[7];
	e2.param_5 = param[7];

	e3.mask_6 = mask[8];
	e3.param_6 = param[8];
	e3.mask_7 = mask[9];
	e3.param_7 = param[9];
	e3.not_flag = !!not_flag;
	e3.not_flag_ver_id_byte = !!not_flag_ver_id_byte;
	e3.Reserved1 = 0;
	e3.Reserved2 = 0;
	e3.Reserved3 = 0;
	e3.Reserved4 = 0;

	*((u16 *) &risc_mem_map->Filter_Parameter_Table1[entry]) = *((u16 *) &e1);
	*(((u16 *) &risc_mem_map->Filter_Parameter_Table1[entry]) + 1) = *(((u16 *) &e1) + 1);
	*(((u16 *) &risc_mem_map->Filter_Parameter_Table1[entry]) + 2) = *(((u16 *) &e1) + 2);
	*((u16 *) &risc_mem_map->Filter_Parameter_Table2[entry]) = *((u16 *) &e2);
	*(((u16 *) &risc_mem_map->Filter_Parameter_Table2[entry]) + 1) = *(((u16 *) &e2) + 1);
	*(((u16 *) &risc_mem_map->Filter_Parameter_Table2[entry]) + 2) = *(((u16 *) &e2) + 2);
	*((u16 *) &risc_mem_map->Filter_Parameter_Table3[entry]) = *((u16 *) &e3);
	*(((u16 *) &risc_mem_map->Filter_Parameter_Table3[entry]) + 1) = *(((u16 *) &e3) + 1);
	*(((u16 *) &risc_mem_map->Filter_Parameter_Table3[entry]) + 2) = *(((u16 *) &e3) + 2);

	return 0;

}

int avia_gt_dmx_set_section_filter(void *v_gtx, unsigned entry, unsigned no_of_filters, void *v_secfilter)
{
	gtx_demux_t *gtx = v_gtx;
	gtx_demux_secfilter_t *secfilter = v_secfilter;
	gtx_demux_secfilter_t filter[32];
	gtx_demux_secfilter_t *ptr,*prev;
	int beststart;
	int bestlen;
	int i;
	int j;
	int k;
	int is_active;
	unsigned in_use_count;
	int base, len;
	unsigned total_free;
	sPID_Parsing_Control_Entry e;

	dprintk("avia_gt_dmx_set_section_filter, entry %d, no_of_filters %d\n",entry,no_of_filters);

	if (no_of_filters == 0)
	{
		avia_gt_dmx_release_section_filter(gtx,entry);
		return 0;
	}

	/*
	 * copy section filters and "normalize" filter_value
	 */
	for (i = 0; (i < no_of_filters) && (secfilter != NULL); i++)
	{
		filter[i].next = filter + i + 1;
		k = 0;
		for (j = 0; j < 10; j++)
		{
			filter[i].filter.filter_value[j] = secfilter->filter.filter_value[j] & secfilter->filter.filter_mask[j];
			if (secfilter->filter.filter_mask[j] != 0)
			{
				k = j + 1;
			}
			dprintk("%02X/%02X ",secfilter->filter.filter_value[j],secfilter->filter.filter_mask[j]);
		}

		dprintk("Length %d\n",k);
		memcpy(filter[i].filter.filter_mask,secfilter->filter.filter_mask,10);
		filter[i].filter.filter_value[1] = 0;	// section length cannot be filtered
		filter[i].filter.filter_mask[1] = 0;
		filter[i].filter.filter_value[2] = 0;
		filter[i].filter.filter_mask[2] = 0;
		filter[i].index = k;
		secfilter = secfilter->next;

		if (k == 0)
		{
			avia_gt_dmx_release_section_filter(gtx,entry);
			return 0;
		}
	}
	if (i != no_of_filters)
	{
		printk(KERN_ERR "avia_gt_dmx: no_of_filters != count of secfilters\n");
		return -EINVAL;
	}
	filter[i-1].next = NULL;

	/*
	 * search duplicate entries
	 */
	secfilter = filter;
	while (secfilter)
	{
		ptr = secfilter->next;
		prev = secfilter;
		while (ptr)
		{
			if (secfilter->index < ptr->index)
			{
				k = secfilter->index;
			}
			else
			{
				k = ptr->index;
			}
			if (memcmp(secfilter->filter.filter_value,ptr->filter.filter_value,k) ||
			    memcmp(secfilter->filter.filter_mask,ptr->filter.filter_mask,k))
			{
				prev = ptr;
			}
			else
			{
				while (k < 10)
				{
					secfilter->filter.filter_value[k] = 0;
					secfilter->filter.filter_mask[k++] = 0;
				}
				prev->next = ptr->next;
				no_of_filters--;
			}
			ptr = ptr->next;
		}
		secfilter = secfilter->next;
	}

	dprintk("after dupcheck %d filter entries\n",no_of_filters);

	/*
	 * only 1 filter with no mask?
	 */

	if (no_of_filters == 1)
	{
		k = 1;
		for (i = 0; (i < 10) && k; i++)
		{
			if (filter[0].filter.filter_mask[i] != 0)
			{
				k = 0;
			}
		}
		if (k)
		{
			avia_gt_dmx_release_section_filter(gtx,entry);
			return 0;
		}
	}

	if (no_of_filters > 12)
	{
		printk(KERN_ERR "avia_gt_dmx: Too many different section filters!\n");
		no_of_filters = 12;
	}
	/*
	 * test whether this feed is already active
	 */
	is_active = -1;
	in_use_count = 0;
	for (i = 0; i < 31; i++)
	{
		if (gtx->filter_definition_table_entry_user[i] == entry)
		{
			if (is_active == -1)
			{
				is_active = i;
			}
			in_use_count++;
			gtx->filter_definition_table_entry_user[i] = -1;
		}
	}

	dprintk("feed active: %d, len %d\n",is_active,in_use_count);

	/*
	 * look for space in the filter_definition_table
	 */

	total_free = 0;
	base = -1;
	bestlen = 256;
	beststart = -1;
	len = 0;
	for (i = 0; i < 31; i++)
	{
		if (gtx->filter_definition_table_entry_user[i] != -1)
		{
			if ( (len >= no_of_filters) && (len < bestlen) )
			{
				bestlen = len;
				beststart = base;
			}
			base = -1;
			len = 0;
		}
		else
		{
			if (len == 0)
			{
				base = i;
			}
			len++;
			total_free++;
		}
	}

	if ( (len >= no_of_filters) && (len < bestlen) )
	{
		bestlen = len;
		beststart = base;
	}

	dprintk("search gives base = %d, len = %d, beststart %d, bestlen %d, totalfree %d\n",
		base,len,beststart,bestlen,total_free);

	/*
	 * not enough space ?
	 */
	if (total_free < no_of_filters)
	{
		dprintk("not enough space!\n");
		while (in_use_count--)
		{
			gtx->filter_definition_table_entry_user[is_active++] = entry;
		}
		return -1;
	}

	/*
	 * stop feed
	 */

	avia_gt_dmx_start_stop_feed(entry,1);

	/*
	 * do we have to defragment ?
	 */

	if (beststart == -1)
	{
		beststart = avia_gt_dmx_compress_filter_parameter_table(gtx);
	}

	/*
	 * let's rock
	 */

	avia_gt_dmx_set_filter_definition_table(entry,1,beststart);
	ptr = filter;
	for (i = beststart; i < beststart + no_of_filters; i++) {
		avia_gt_dmx_set_filter_parameter_table(i,ptr->filter.filter_mask,ptr->filter.filter_value,0,0);
		gtx->filter_definition_table_entry_user[i] = entry;
		ptr = ptr->next;
	}

	*((u32 *) &e) = *((u32 *) &risc_mem_map->PID_Parsing_Control_Table[entry]);
	e.no_of_filter = no_of_filters-1;
	e._PSH = 1;
	*((u32 *) &risc_mem_map->PID_Parsing_Control_Table[entry]) = *((u32 *) &e);

	return 0;
}

unsigned char avia_gt_dmx_map_queue(unsigned char queue_nr)
{

	if (avia_gt_chip(ENX)) {

		if (queue_nr >= 16)
			enx_reg_set(CFGR0, UPQ, 1);
		else
			enx_reg_set(CFGR0, UPQ, 0);

	} else if (avia_gt_chip(GTX)) {

		if (queue_nr >= 16)
			gtx_reg_set(CR1, UPQ, 1);
		else
			gtx_reg_set(CR1, UPQ, 0);

	}

	mb();

	return (queue_nr & 0x0F);

}

void avia_gt_dmx_force_discontinuity(void)
{

	force_stc_reload = 1;

	if (avia_gt_chip(ENX))
		enx_reg_set(FC, FD, 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_16(FCR) |= 0x100;

}

sAviaGtDmxQueue *avia_gt_dmx_get_queue_info(u8 queue_nr)
{

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: get_queue_info: queue %d out of bounce\n", queue_nr);

		return NULL;

	}

	return &queue_list[queue_nr];

}

u32 avia_gt_dmx_get_queue_bytes_avail(u8 queue_nr)
{

	if (queue_list[queue_nr].write_pos >= queue_list[queue_nr].read_pos)
		return (queue_list[queue_nr].write_pos - queue_list[queue_nr].read_pos);
	else
		return (queue_list[queue_nr].size - queue_list[queue_nr].read_pos + queue_list[queue_nr].write_pos);

}

u16 avia_gt_dmx_get_queue_irq(u8 queue_nr)
{

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: alloc_queue: queue %d out of bounce\n", queue_nr);

		return 0;

	}

	if (avia_gt_chip(ENX)) {

		if (queue_nr >= 17)
			return AVIA_GT_IRQ(3, queue_nr - 16);
		else if (queue_nr >= 2)
			return AVIA_GT_IRQ(4, queue_nr - 1);
		else
			return AVIA_GT_IRQ(5, queue_nr + 6);

	} else if (avia_gt_chip(GTX)) {

		return AVIA_GT_IRQ(2 + !!(queue_nr & 16), queue_nr & 15);

	}

	return 0;

}

int avia_gt_dmx_load_ucode(void)
{

	int fd = (int)0;
	loff_t file_size;
	mm_segment_t fs;

	fs = get_fs();
	set_fs(get_ds());

	if ((fd = open(ucode, 0, 0)) < 0) {

		printk (KERN_ERR "avia_gt_dmx: Unable to load firmware file '%s'\n", ucode);

		set_fs(fs);

		return -EFAULT;

	}

	file_size = lseek(fd, 0L, 2);

	if (file_size <= 0) {

		printk (KERN_ERR "avia_gt_dmx: Firmware wrong size '%s'\n", ucode);

		sys_close(fd);
		set_fs(fs);

		return -EFAULT;

	}

	lseek(fd, 0L, 0);

	if (read(fd, (void *)risc_mem_map, file_size) != file_size) {

		printk (KERN_ERR "avia_gt_dmx: Failed to read firmware file '%s'\n", ucode);

		sys_close(fd);
		set_fs(fs);

		return -EIO;

	}

	close(fd);
	set_fs(fs);

	printk("avia_gt_dmx: Successfully loaded ucode V%X.%X\n", risc_mem_map->Version_no[0], risc_mem_map->Version_no[1]);

	return 0;

}

void avia_gt_dmx_fake_queue_irq(u8 queue_nr)
{

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: fake_queue_irq: queue %d out of bounce\n", queue_nr);

		return;

	}

	queue_list[queue_nr].irq_count++;
	
	schedule_task(&avia_gt_dmx_queue_tasklet);
	
}

u32 avia_gt_dmx_queue_crc32(u8 queue_nr, u32 count, u32 seed)
{

	u32 chunk1_size;
	u32 crc;

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: crc32: queue %d out of bounce\n", queue_nr);

		return 0;

	}

	if ((queue_list[queue_nr].read_pos + count) > queue_list[queue_nr].size) {
	
		chunk1_size = queue_list[queue_nr].size - queue_list[queue_nr].read_pos;

		// FIXME
		if ((avia_gt_chip(GTX)) && (chunk1_size <= 4))
			return 0; 
		
		crc = avia_gt_accel_crc32(queue_list[queue_nr].mem_addr + queue_list[queue_nr].read_pos, chunk1_size, seed);
		
		return avia_gt_accel_crc32(queue_list[queue_nr].mem_addr, count - chunk1_size, crc);

	} else {

		return avia_gt_accel_crc32(queue_list[queue_nr].mem_addr + queue_list[queue_nr].read_pos, count, seed);

	}

}

u32 avia_gt_dmx_queue_data_peek(u8 queue_nr, u32 index, void *dest, u32 count)
{
	u32 bytes_avail = avia_gt_dmx_get_queue_bytes_avail(queue_nr);
	u32 done = 0;
	u32 read_pos;

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {
		printk("avia_gt_dmx: queue_data_peek: queue %d out of bounce\n", queue_nr);
		return 0;
	}

	if (count + index > bytes_avail) {
		printk("avia_gt_dmx: queue_data_peek: %d bytes requested, %d available\n", count, bytes_avail);
		return 0;
	}

	read_pos = queue_list[queue_nr].read_pos + index;
	if (read_pos >queue_list[queue_nr].size)
	{
		read_pos -= queue_list[queue_nr].size;
	}

	if ((read_pos > queue_list[queue_nr].write_pos) &&
		(count >= (queue_list[queue_nr].size - read_pos))) {

		done = queue_list[queue_nr].size - read_pos;

		if (dest)
			memcpy(dest, gt_info->mem_addr + queue_list[queue_nr].mem_addr + read_pos, done);

		read_pos = 0;

	}

	if ((dest) && (count - done))
		memcpy(((u8 *)dest) + done, gt_info->mem_addr + queue_list[queue_nr].mem_addr + read_pos, count - done);

	return count;
}

u32 avia_gt_dmx_queue_data_get(u8 queue_nr, void *dest, u32 count, u8 peek)
{

	u32 bytes_avail = avia_gt_dmx_get_queue_bytes_avail(queue_nr);
	u32	done = 0;
	u32 read_pos;

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: queue_data_move: queue %d out of bounce\n", queue_nr);

		return 0;

	}

	if (count > bytes_avail) {

		printk("avia_gt_dmx: queue_data_move: %d bytes requested, %d availible\n", count, bytes_avail);

		count = bytes_avail;

	}

	read_pos = queue_list[queue_nr].read_pos;

	if ((read_pos > queue_list[queue_nr].write_pos) &&
		(count >= (queue_list[queue_nr].size - read_pos))) {

		done = queue_list[queue_nr].size - read_pos;

		if (dest)
			memcpy(dest, gt_info->mem_addr + queue_list[queue_nr].mem_addr + read_pos, done);

		read_pos = 0;

	}

	if ((dest) && (count - done))
		memcpy(((u8 *)dest) + done, gt_info->mem_addr + queue_list[queue_nr].mem_addr + read_pos, count - done);

	if (!peek)
		queue_list[queue_nr].read_pos = read_pos + (count - done);

	return count;

}

u8 avia_gt_dmx_queue_data_get8(u8 queue_nr, u8 peek)
{

	u8 data;

	avia_gt_dmx_queue_data_get(queue_nr, &data, sizeof(u8), peek);

	return data;

}

u16 avia_gt_dmx_queue_data_get16(u8 queue_nr, u8 peek)
{

	u16 data;

	avia_gt_dmx_queue_data_get(queue_nr, &data, sizeof(u16), peek);

	return data;

}

u32 avia_gt_dmx_queue_data_get32(u8 queue_nr, u8 peek)
{

	u32 data;

	avia_gt_dmx_queue_data_get(queue_nr, &data, sizeof(u32), peek);

	return data;

}

u32 avia_gt_dmx_queue_data_put(u8 queue_nr, void *src, u32 count, u8 src_is_user_space)
{

	u32 bytes_free = avia_gt_dmx_queue_get_bytes_free(queue_nr);
	u32 done = 0;

	if ((queue_nr != AVIA_GT_DMX_QUEUE_VIDEO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_AUDIO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_TELETEXT)) {

		printk("avia_gt_dmx: queue_data_put: queue %d out of bounce\n", queue_nr);

		return 0;

	}

	if (count > bytes_free) {
	
		printk("avia_gt_dmx: queue_data_put: %d bytes requested, %d availible\n", count, bytes_free);
		
		count = bytes_free;
		
	}
	
	if ((queue_list[queue_nr].write_pos + count) >= queue_list[queue_nr].size) {

		done = queue_list[queue_nr].size - queue_list[queue_nr].write_pos;

		if (src_is_user_space)
			copy_from_user(gt_info->mem_addr + queue_list[queue_nr].mem_addr + queue_list[queue_nr].write_pos, src, done);
		else
			memcpy(gt_info->mem_addr + queue_list[queue_nr].mem_addr + queue_list[queue_nr].write_pos, src, done);

		queue_list[queue_nr].write_pos = 0;

	}

	if (count - done) {

		if (src_is_user_space)
			copy_from_user(gt_info->mem_addr + queue_list[queue_nr].mem_addr + queue_list[queue_nr].write_pos, ((u8 *)src) + done, count - done);
		else
			memcpy(gt_info->mem_addr + queue_list[queue_nr].mem_addr + queue_list[queue_nr].write_pos, ((u8 *)src) + done, count - done);

		queue_list[queue_nr].write_pos += (count - done);
		
	}

	return count;

}

u32 avia_gt_dmx_queue_get_bytes_free(u8 queue_nr)
{

	if ((queue_nr != AVIA_GT_DMX_QUEUE_VIDEO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_AUDIO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_TELETEXT)) {

		printk("avia_gt_dmx: queue_get_bytes_free: queue %d out of bounce\n", queue_nr);

		return 0;

	}
	
	queue_list[queue_nr].hw_read_pos = avia_gt_dmx_system_queue_get_read_pos(queue_nr);
	
	if (queue_list[queue_nr].write_pos >= queue_list[queue_nr].hw_read_pos)
		/*         free chunk1        busy chunk1        free chunk2   */
		/* queue [ FFFFFFFFFFF (RPOS) BBBBBBBBBBB (WPOS) FFFFFFFFFFF ] */
		return (queue_list[queue_nr].size - queue_list[queue_nr].write_pos) + queue_list[queue_nr].hw_read_pos - 1;
	else
		/*         busy chunk1        free chunk1        busy chunk2   */
		/* queue [ BBBBBBBBBBB (WPOS) FFFFFFFFFFF (RPOS) BBBBBBBBBBB ] */
		return (queue_list[queue_nr].hw_read_pos - queue_list[queue_nr].write_pos) - 1;

}

u32 avia_gt_dmx_queue_get_write_pos(u8 queue_nr)
{

	u8 mapped_queue_nr;
	u32 previous_write_pos;
	u32 write_pos = 0xFFFFFFFF;

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: queue_get_write_pos: queue %d out of bounce\n", queue_nr);

		return 0;

	}

	mapped_queue_nr = avia_gt_dmx_map_queue(queue_nr);

    /*
     *
     * CAUTION: The correct sequence for accessing Queue
     * Pointer registers is as follows:
     * For reads,
     * Read low word
     * Read high word
     * CAUTION: Not following these sequences will yield
     * invalid data.
     *
     */

	do {

		previous_write_pos = write_pos;

		if (avia_gt_chip(ENX))
			write_pos = ((enx_reg_so(QWPnL, 4 * mapped_queue_nr)->Queue_n_Write_Pointer) | (enx_reg_so(QWPnH, 4 * mapped_queue_nr)->Queue_n_Write_Pointer << 16));
		else if (avia_gt_chip(GTX))
			write_pos = ((gtx_reg_so(QWPnL, 4 * mapped_queue_nr)->Queue_n_Write_Pointer) | (gtx_reg_so(QWPnH, 4 * mapped_queue_nr)->Upper_WD_n << 16));

	} while (previous_write_pos != write_pos);

	if ((write_pos >= (queue_list[queue_nr].mem_addr + queue_list[queue_nr].size)) ||
		(write_pos < queue_list[queue_nr].mem_addr)) {

		printk(KERN_CRIT "avia_gt_napi: queue %d hw_write_pos out of bounds! (B:0x%X/P:0x%X/E:0x%X)\n", queue_nr, queue_list[queue_nr].mem_addr, write_pos, queue_list[queue_nr].mem_addr + queue_list[queue_nr].size);

#ifdef DEBUG
		BUG();
#endif

		queue_list[queue_nr].hw_read_pos = 0;
		queue_list[queue_nr].hw_write_pos = 0;
		queue_list[queue_nr].read_pos = 0;
		queue_list[queue_nr].write_pos = 0;

		avia_gt_dmx_queue_set_write_pos(queue_nr, 0);

		return 0;

	}

	return (write_pos - queue_list[queue_nr].mem_addr);

}

static void avia_gt_dmx_queue_interrupt(unsigned short irq)
{

	u8 bit = AVIA_GT_IRQ_BIT(irq);
	u8 nr = AVIA_GT_IRQ_REG(irq);
	u32 old_hw_write_pos;

	s32 queue_nr = -EINVAL;

    if (avia_gt_chip(ENX)) {

		if (nr == 3)
			queue_nr = bit + 16;
		else if (nr == 4)
			queue_nr = bit + 1;
		else if (nr == 5)
			queue_nr = bit - 6;

    } else if (avia_gt_chip(GTX)) {

		queue_nr = (nr - 2) * 16 + bit;

    }

	if ((queue_nr < 0) || (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT)) {

		printk("avia_gt_dmx: unexpected queue irq (nr=%d, bit=%d)\n", nr, bit);

		return;

	}

	if (!queue_list[queue_nr].busy) {

		printk("avia_gt_dmx: irq on idle queue (queue_nr=%d)\n", queue_nr);

		return;

	}

	queue_list[queue_nr].irq_count++;

	old_hw_write_pos = queue_list[queue_nr].hw_write_pos;
	queue_list[queue_nr].hw_write_pos = avia_gt_dmx_queue_get_write_pos(queue_nr);

	if (queue_list[queue_nr].is_qim)
	{
		avia_gt_dmx_set_queue_irq(queue_nr,1,queue_list[queue_nr].hw_write_pos);
	}

	// Cases:
	//
	//    [    R     OW    ] (R < OW)
	//
	// 1. [    R     OW  W ] OK
	// 2. [ W  R     OW    ] OK
	// 3. [    R  W  OW    ] OVERFLOW (incl. R == W)
	//
	//    [    OW     R    ] (OW < R)
	//
	// 4. [    OW  W  R    ] OK
	// 5. [    OW     R  W ] OVERFLOW (incl. R == W)
	// 6. [ W  OW  R       ] OVERFLOW

#if 1

	if (((queue_list[queue_nr].read_pos < old_hw_write_pos) && ((queue_list[queue_nr].read_pos <= queue_list[queue_nr].hw_write_pos) && (queue_list[queue_nr].hw_write_pos < old_hw_write_pos))) ||
		((old_hw_write_pos < queue_list[queue_nr].read_pos) && ((queue_list[queue_nr].read_pos <= queue_list[queue_nr].hw_write_pos) || (queue_list[queue_nr].hw_write_pos < old_hw_write_pos))))
		queue_list[queue_nr].overflow_count++;	// We can't recovery here or we will break queue handling (get_data*, bytes_avail, ...)

#endif

	if (queue_list[queue_nr].irq_proc)
		queue_list[queue_nr].irq_proc(queue_nr, queue_list[queue_nr].priv_data);
	else
		schedule_task(&avia_gt_dmx_queue_tasklet);

}

void avia_gt_dmx_queue_irq_disable(u8 queue_nr)
{

	avia_gt_free_irq(avia_gt_dmx_get_queue_irq(queue_nr));

}

s32 avia_gt_dmx_queue_irq_enable(u8 queue_nr)
{

	return avia_gt_alloc_irq(avia_gt_dmx_get_queue_irq(queue_nr), avia_gt_dmx_queue_interrupt);

}

s32 avia_gt_dmx_queue_reset(u8 queue_nr)
{

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: queue_reset: queue %d out of bounce\n", queue_nr);

		return -EINVAL;

	}

	queue_list[queue_nr].write_pos = avia_gt_dmx_queue_get_write_pos(queue_nr);
	queue_list[queue_nr].read_pos = queue_list[queue_nr].write_pos;

	avia_gt_dmx_queue_set_write_pos(queue_nr, queue_list[queue_nr].read_pos);

	return 0;

}

void avia_gt_dmx_queue_set_write_pos(u8 queue_nr, u32 write_pos)
{

	u8 mapped_queue_nr;

	if (queue_nr >= AVIA_GT_DMX_QUEUE_COUNT) {

		printk("avia_gt_dmx: set_queue queue (%d) out of bounce\n", queue_nr);

		return;

	}

	mapped_queue_nr = avia_gt_dmx_map_queue(queue_nr);

	if (avia_gt_chip(ENX)) {

		enx_reg_16(QWPnL + 4 * mapped_queue_nr) = (queue_list[queue_nr].mem_addr + write_pos) & 0xFFFF;
		enx_reg_16(QWPnH + 4 * mapped_queue_nr) = (((queue_list[queue_nr].mem_addr + write_pos) >> 16) & 0x3F) | (queue_size_table[queue_nr] << 6);

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_16(QWPnL + 4 * mapped_queue_nr) = (queue_list[queue_nr].mem_addr + write_pos) & 0xFFFF;
		gtx_reg_16(QWPnH + 4 * mapped_queue_nr) = (((queue_list[queue_nr].mem_addr + write_pos) >> 16) & 0x3F) | (queue_size_table[queue_nr] << 6);

	}

}

static void avia_gt_dmx_queue_task(void *tl_data)
{

	s8 queue_nr;

	for (queue_nr = (AVIA_GT_DMX_QUEUE_COUNT - 1); queue_nr >= 0; queue_nr--) {	// msg queue must have priority
	
		queue_list[queue_nr].write_pos = queue_list[queue_nr].hw_write_pos;

		if (queue_list[queue_nr].overflow_count) {

			printk("avia_gt_dmx: queue %d overflow (count: %d)\n", queue_nr, queue_list[queue_nr].overflow_count);
		
			queue_list[queue_nr].overflow_count = 0;
			queue_list[queue_nr].read_pos = queue_list[queue_nr].hw_write_pos;
			
			continue;
		
		}

		if (queue_list[queue_nr].write_pos == queue_list[queue_nr].read_pos)
			continue;

		if (queue_list[queue_nr].cb_proc)
			queue_list[queue_nr].cb_proc(queue_nr, queue_list[queue_nr].priv_data);

		//queue_list[queue_nr].read_pos = queue_list[queue_nr].write_pos;
		queue_list[queue_nr].irq_count = 0;

	}

}

void avia_gt_dmx_set_pcr_pid(u16 pid)
{

	if (avia_gt_chip(ENX)) {

		//enx_reg_set(PCR_PID, E, 0);
		//enx_reg_set(PCR_PID, PID, pid);
		//enx_reg_set(PCR_PID, E, 1);

		enx_reg_16(PCR_PID) = (1 << 13) | pid;

		avia_gt_free_irq(ENX_IRQ_PCR);
		avia_gt_alloc_irq(ENX_IRQ_PCR, gtx_pcr_interrupt);			 // pcr reception

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_16(PCRPID) = (1 << 13) | pid;
		avia_gt_free_irq(GTX_IRQ_PCR);
		avia_gt_alloc_irq(GTX_IRQ_PCR, gtx_pcr_interrupt);			 // pcr reception

	}

	avia_gt_dmx_force_discontinuity();

}

int avia_gt_dmx_set_pid_control_table(u8 entry, u8 type, u8 queue, u8 fork, u8 cw_offset, u8 cc, u8 start_up, u8 pec, u8 filt_tab_idx, u8 _psh)
{
	sPID_Parsing_Control_Entry e;

	if (entry > 31) {

		printk("avia_gt_dmx: pid control table entry out of bounce (entry=%d)!\n", entry);

		return -EINVAL;

	}

	dprintk("avia_gt_dmx_set_pid_control_table, entry %d, type %d, queue %d, fork %d, cw_offset %d, cc %d, start_up %d, pec %d, filt_tab_idx %d, _psh %d\n",
		entry,type,queue,fork,cw_offset,cc,start_up,pec,filt_tab_idx,_psh);

#ifdef AVIA_SPTS
	// Route audio stream into video queue
	if (queue == 1)
		queue = 0;
#endif

	if (risc_mem_map->Version_no[0] < 0xA0)
		queue++;

	*((u32 *) &e ) = *((u32 *) &risc_mem_map->PID_Parsing_Control_Table[entry]);

	e.type = type;
	e.QID = queue;
	e.fork = !!fork;
	e.CW_offset = cw_offset;
	e.CC = cc;
	e._PSH = _psh;
	e.start_up = !!start_up;
	e.PEC = !!pec;
	e.filt_tab_idx = filt_tab_idx;
//	e.State = 0;
	e.State = 7;

	*((u32 *) &risc_mem_map->PID_Parsing_Control_Table[entry]) = *((u32 *) &e);

	return 0;

}

int avia_gt_dmx_set_pid_table(u8 entry, u8 wait_pusi, u8 valid, u16 pid)
{
	sPID_Entry e;

	if (entry > 31) {

		printk("avia_gt_dmx: pid search table entry out of bounce (entry=%d)!\n", entry);

		return -EINVAL;

	}

	dprintk("avia_gt_dmx_set_pid_table, entry %d, wait_pusi %d, valid %d, pid 0x%04X\n",
		entry,wait_pusi,valid,pid);

	e.wait_pusi = wait_pusi;
	e.VALID = !!valid;			// 0 = VALID, 1 = INVALID
	e.Reserved1 = 0;
	e.PID = pid;
	*((u16 *) &risc_mem_map->PID_Search_Table[entry]) = *((u16 *) &e);

//	if (valid == 0)
//		avia_gt_dmx_dump();

	return 0;

}

/*
 * Für den "Block-IRQ-Modus" wird folgender Algorithmus verwendet:
 * Der gesamte Puffer wird geachtelt. Es wird herausgefunden, in
 * welchem Achtel sich der Schreibpointer gerade befindet:
 * Analog der Prozentrechnung
 *  write_pointer * 100 / queue_size mit den Grenzen 12,5, 25 ... 100 wird
 *  write_pointer * 8 / queue_size mit den Grenzen 1, 2 .. 8
 * verwendet.
 * Der Interrupt wird dann auf das erreichen des _übernächsten_ Achtel gesetzt.
 * Das Setzen auf das nächste Achtel könnte zu Problemen führen, wenn der
 * write_pointer kurz vor der Grenze ist.
 */

void avia_gt_dmx_set_queue_irq(unsigned char queue_nr, unsigned char qim, s32 write_pos)
{
	u16 value;
	u8 block_pos;

	if (write_pos == -1)	// not in irq
	{
		queue_list[queue_nr].is_qim = qim;
	}

	if (qim)
	{
		if (write_pos == -1)
		{
			write_pos = avia_gt_dmx_queue_get_write_pos(queue_nr);
		}

		block_pos = (write_pos << 4) / queue_list[queue_nr].size;

		if (block_pos == 15)
		{
			value = 0x8000;
		}
		else
		{
			value = 0x8000 | ((block_pos + 2) << 11);
		}
	}
	else
	{
		value = 0;
	}

//	printk(KERN_INFO "avia_gt_dmx_set_queue_irq: write_pos: %04X, new irq: %04X\n",write_pos,value);

	queue_nr = avia_gt_dmx_map_queue(queue_nr);

	if (avia_gt_chip(ENX))
		enx_reg_16n(0x08C0 + queue_nr * 2) = value;
	else if (avia_gt_chip(GTX))
		gtx_reg_16(QIn + queue_nr * 2) = value;

}

u32 avia_gt_dmx_system_queue_get_read_pos(u8 queue_nr)
{

	u16	base = 0;
	u32 read_pos = 0xFFFFFFFF;
	u32 previous_read_pos;

	if ((queue_nr != AVIA_GT_DMX_QUEUE_VIDEO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_AUDIO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_TELETEXT)) {

		printk("avia_gt_dmx: system_queue_get_read_pos: queue %d out of bounce\n", queue_nr);

		return 0;

	}

	if (avia_gt_chip(ENX))
		base = queue_system_map[queue_nr] * 8 + 0x8E0;
	else if (avia_gt_chip(GTX))
		base = queue_system_map[queue_nr] * 8 + 0x1E0;

    /*
     *
     * CAUTION: The correct sequence for accessing Queue
     * Pointer registers is as follows:
     * For reads,
     * Read low word
     * Read high word
     * CAUTION: Not following these sequences will yield
     * invalid data.
     *
     */

	do {

		previous_read_pos = read_pos;

		if (avia_gt_chip(ENX))
			read_pos = enx_reg_16n(base) | ((enx_reg_16n(base + 2) & 0x3F) << 16);
		else if (avia_gt_chip(GTX))
			read_pos = gtx_reg_16n(base) | ((gtx_reg_16n(base + 2) & 0x3F) << 16);

	} while (previous_read_pos != read_pos);
	
	if (read_pos > (queue_list[queue_nr].mem_addr + queue_list[queue_nr].size)) {
	
		printk("avia_gt_dmx: system_queue_get_read_pos: queue %d read_pos 0x%X > queue_end 0x%X\n", queue_nr, read_pos, queue_list[queue_nr].mem_addr + queue_list[queue_nr].size);
		
		read_pos = queue_list[queue_nr].mem_addr;
	
	}

	if (read_pos < queue_list[queue_nr].mem_addr) {
	
		printk("avia_gt_dmx: system_queue_get_read_pos: queue %d read_pos 0x%X < queue_base 0x%X\n", queue_nr, read_pos, queue_list[queue_nr].mem_addr);

		read_pos = queue_list[queue_nr].mem_addr;
	
	}

	return (read_pos - queue_list[queue_nr].mem_addr);

}


void avia_gt_dmx_system_queue_set_pos(u8 queue_nr, u32 read_pos, u32 write_pos)
{

	int	base;

	if ((queue_nr != AVIA_GT_DMX_QUEUE_VIDEO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_AUDIO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_TELETEXT)) {

		printk("avia_gt_dmx: queue_system_set_pos: queue %d out of bounce\n", queue_nr);

		return;

	}

	if (avia_gt_chip(ENX)) {

		base = queue_system_map[queue_nr] * 8 + 0x8E0;

		enx_reg_16n(base + 0) = (queue_list[queue_nr].mem_addr + read_pos) & 0xFFFF;
		avia_gt_dmx_system_queue_set_write_pos(queue_nr, write_pos);
		enx_reg_16n(base + 2) = ((queue_list[queue_nr].mem_addr + read_pos) >> 16) & 63;

	} else if (avia_gt_chip(GTX)) {

		base = queue_system_map[queue_nr] * 8 + 0x1E0;

		gtx_reg_16n(base + 0) = (queue_list[queue_nr].mem_addr + read_pos) & 0xFFFF;
		avia_gt_dmx_system_queue_set_write_pos(queue_nr, write_pos);
		gtx_reg_16n(base + 2) = ((queue_list[queue_nr].mem_addr + read_pos) >> 16) & 63;

	}

}

void avia_gt_dmx_system_queue_set_read_pos(u8 queue_nr, u32 read_pos)
{

	int	base;

	if ((queue_nr != AVIA_GT_DMX_QUEUE_VIDEO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_AUDIO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_TELETEXT)) {

		printk("avia_gt_dmx: system_queue_set_read_pos: queue %d out of bounce\n", queue_nr);

		return;

	}

	if (avia_gt_chip(ENX)) {

		base = queue_system_map[queue_nr] * 8 + 0x8E0;

		enx_reg_16n(base) = (queue_list[queue_nr].mem_addr + read_pos) & 0xFFFF;
		enx_reg_16n(base + 2) = ((queue_list[queue_nr].mem_addr + read_pos) >> 16) & 63;

	} else if (avia_gt_chip(GTX)) {

		base = queue_system_map[queue_nr] * 8 + 0x1E0;

		gtx_reg_16n(base) = (queue_list[queue_nr].mem_addr + read_pos) & 0xFFFF;
		gtx_reg_16n(base + 2) = ((queue_list[queue_nr].mem_addr + read_pos) >> 16) & 63;

	}

}

void avia_gt_dmx_system_queue_set_write_pos(u8 queue_nr, u32 write_pos)
{

	int	base;

	if ((queue_nr != AVIA_GT_DMX_QUEUE_VIDEO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_AUDIO) &&
		(queue_nr != AVIA_GT_DMX_QUEUE_TELETEXT)) {

		printk("avia_gt_dmx: system_queue_set_write_pos: queue %d out of bounce\n", queue_nr);

		return;

	}
	
	if (avia_gt_chip(ENX)) {

		base = queue_system_map[queue_nr] * 8 + 0x8E0;

		enx_reg_16n(base + 4) = (queue_list[queue_nr].mem_addr + write_pos) & 0xFFFF;
		enx_reg_16n(base + 6) = (((queue_list[queue_nr].mem_addr + write_pos) >> 16) & 63) | (queue_size_table[queue_nr] << 6);

	} else if (avia_gt_chip(GTX)) {

		base = queue_system_map[queue_nr] * 8 + 0x1E0;

		gtx_reg_16n(base + 4) = (queue_list[queue_nr].mem_addr + write_pos) & 0xFFFF;
		gtx_reg_16n(base + 6) = (((queue_list[queue_nr].mem_addr + write_pos) >> 16) & 63) | (queue_size_table[queue_nr] << 6);

	}

}

void avia_gt_dmx_reset(unsigned char reenable)
{

	if (avia_gt_chip(ENX))
		enx_reg_set(RSTR0, TDMP, 1);
	else if (avia_gt_chip(GTX))
		gtx_reg_set(RR1, RISC, 1);

	if (reenable) {
	
		if (avia_gt_chip(ENX))
			enx_reg_set(RSTR0, TDMP, 0);
		else if (avia_gt_chip(GTX))
			gtx_reg_set(RR1, RISC, 0);

	}

}

int avia_gt_dmx_risc_init(void)
{

	avia_gt_dmx_reset(0);

	if (avia_gt_dmx_load_ucode()) {

		printk("avia_gt_dmx: No valid firmware found! TV mode disabled.\n");

		return 0;

	}

	if (avia_gt_chip(ENX)) {

		avia_gt_dmx_reset(1);
		enx_reg_32(RSTR0) &= ~(1 << 22); //clear tdp-reset bit
		//enx_tdp_trace();
		enx_reg_16(EC) = 0;

	} else if (avia_gt_chip(GTX)) {

		avia_gt_dmx_reset(1);

	}

	return 0;

}

void enx_tdp_stop(void)
{

	enx_reg_32(EC) = 2;			//stop tdp

}

// Ugly as hell - but who cares? :-)
#define MAKE_PCR(base2, base1, base0, extension) ((((u64)(extension)) << 50) | (((u64)(base2)) << 17) | (((u64)(base1)) << 1) | ((u64)(base0)))
#define PCR_BASE(pcr) ((pcr) & 0x1FFFFFFFF)
#define PCR_EXTENSION(pcr) ((pcr) >> 50)
#define PCR_VALUE(pcr) (PCR_BASE(pcr) * 300 + PCR_EXTENSION(pcr))

u64 avia_gt_dmx_get_transport_pcr(void)
{

	if (avia_gt_chip(ENX))
		return MAKE_PCR(enx_reg_s(TP_PCR_2)->PCR_Base, enx_reg_s(TP_PCR_1)->PCR_Base, enx_reg_s(TP_PCR_0)->PCR_Base, enx_reg_s(TP_PCR_0)->PCR_Extension);
	else if (avia_gt_chip(GTX))
		return MAKE_PCR(gtx_reg_s(PCR2)->PCR_Base, gtx_reg_s(PCR1)->PCR_Base, gtx_reg_s(PCR0)->PCR_Base, gtx_reg_s(PCR0)->PCR_Extension);

	return 0;
	
}

u64 avia_gt_dmx_get_latched_stc(void)
{

	if (avia_gt_chip(ENX))
		return MAKE_PCR(enx_reg_s(LC_STC_2)->Latched_STC_Base, enx_reg_s(LC_STC_1)->Latched_STC_Base, enx_reg_s(LC_STC_0)->Latched_STC_Base, enx_reg_s(LC_STC_0)->Latched_STC_Extension);
	else if (avia_gt_chip(GTX))
		return MAKE_PCR(gtx_reg_s(LSTC2)->Latched_STC_Base, gtx_reg_s(LSTC1)->Latched_STC_Base, gtx_reg_s(LSTC0)->Latched_STC_Base, gtx_reg_s(LSTC0)->Latched_STC_Extension);

	return 0;
	
}

u64 avia_gt_dmx_get_stc(void)
{

	if (avia_gt_chip(ENX))
		return MAKE_PCR(enx_reg_s(STC_COUNTER_2)->STC_Count, enx_reg_s(STC_COUNTER_1)->STC_Count, enx_reg_s(STC_COUNTER_0)->STC_Count, enx_reg_s(STC_COUNTER_0)->STC_Extension);
	else if (avia_gt_chip(GTX))
		return MAKE_PCR(gtx_reg_s(STCC2)->STC_Count, gtx_reg_s(STCC1)->STC_Count, gtx_reg_s(STCC0)->STC_Count, gtx_reg_s(STCC0)->STC_Extension);

	return 0;

}

static void avia_gt_dmx_set_dac(s16 pulse_count)
{

	if (avia_gt_chip(ENX)) {

		enx_reg_16(DAC_PC) = pulse_count;
		enx_reg_16(DAC_CP) = 9;

	} else if (avia_gt_chip(GTX)) {

		gtx_reg_32(DPCR) = (pulse_count << 16) | 9;
		
	}

}

static s16 gain = 0;
static s64 last_remote_diff = 0;

static void gtx_pcr_interrupt(unsigned short irq)
{

	u64 tp_pcr;
	u64 l_stc;
	u64 stc;

	s64 local_diff;
	s64 remote_diff;

	tp_pcr = avia_gt_dmx_get_transport_pcr();
	l_stc = avia_gt_dmx_get_latched_stc();
	stc = avia_gt_dmx_get_stc();

	if (force_stc_reload) {

		printk("avia_gt_dmx: reloading stc\n");

		avia_set_pcr(PCR_BASE(tp_pcr) >> 1, (PCR_BASE(tp_pcr) & 0x01) << 15);
		force_stc_reload = 0;

	}

	local_diff = (s64)PCR_VALUE(stc) - (s64)PCR_VALUE(l_stc);
	remote_diff = (s64)PCR_VALUE(tp_pcr) - (s64)PCR_VALUE(stc);

#define GAIN 25

	if (remote_diff > 0) {

		if (remote_diff > last_remote_diff)
			gain -= 2*GAIN;
		else
			gain += GAIN;

	} else if (remote_diff < 0) {

		if (remote_diff < last_remote_diff) 
			gain += 2*GAIN;
		else
			gain -= GAIN;


	}

	avia_gt_dmx_set_dac(gain);
	
	last_remote_diff = remote_diff;

	dprintk(KERN_DEBUG "tp_pcr/stc/dir/diff: 0x%08x%08x/0x%08x%08x//%d\n", (u32)(PCR_VALUE(tp_pcr) >> 32), (u32)(PCR_VALUE(tp_pcr) & 0x0FFFFFFFF), (u32)(PCR_VALUE(stc) >> 32), (u32)(PCR_VALUE(stc) & 0x0FFFFFFFF), (s32)(remote_diff));

#if 0

	if ((remote_diff > TIME_THRESHOLD) || (remote_diff < -TIME_THRESHOLD)) {

		printk("avia_gt_dmx: stc out of sync!\n");
		avia_gt_dmx_force_discontinuity();

	}

#endif

}


int __init avia_gt_dmx_init(void)
{

	int result;
	u32 queue_addr;
	u8 queue_nr;

	printk("avia_gt_dmx: $Id: avia_gt_dmx.c,v 1.139.2.1 2002/11/17 01:59:13 obi Exp $\n");;

	gt_info = avia_gt_get_info();

	if ((!gt_info) || ((!avia_gt_chip(ENX)) && (!avia_gt_chip(GTX)))) {

		printk("avia_gt_dmx: Unsupported chip type\n");

		return -EIO;

	}

//	avia_gt_dmx_reset(1);


	if (avia_gt_chip(ENX)) {

		enx_reg_32(RSTR0)|=(1<<31)|(1<<23)|(1<<22);

		risc_mem_map = (sRISC_MEM_MAP *)enx_reg_o(TDP_INSTR_RAM);

	} else if (avia_gt_chip(GTX)) {

		risc_mem_map = (sRISC_MEM_MAP *)gtx_reg_o(GTX_REG_RISC);

	}

	if ((result = avia_gt_dmx_risc_init()))
		return result;

	if (avia_gt_chip(ENX)) {

		enx_reg_32(RSTR0) &= ~(1 << 27);
		enx_reg_32(RSTR0) &= ~(1 << 13);
		enx_reg_32(RSTR0) &= ~(1 << 11);
		enx_reg_32(RSTR0) &= ~(1 << 9);
		enx_reg_32(RSTR0) &= ~(1 << 23);
		enx_reg_32(RSTR0) &= ~(1 << 31);

		enx_reg_32(CFGR0) &= ~(1 << 3);
		enx_reg_32(CFGR0) &= ~(1 << 1);
		enx_reg_32(CFGR0) &= ~(1 << 0);

		enx_reg_16(FC) = 0x9147;
		enx_reg_16(SYNC_HYST) = 0x21;
		enx_reg_16(BQ) = 0x00BC;

		enx_reg_32(CFGR0) |= 1 << 24;		// enable dac output

		enx_reg_16(AVI_0) = 0xF;					// 0x6CF geht nicht (ordentlich)
		enx_reg_16(AVI_1) = 0xA;

		enx_reg_32(CFGR0) &= ~3;				// disable clip mode

		printk("ENX-INITed -> %x\n", enx_reg_16(FIFO_PDCT));

		if (!enx_reg_16(FIFO_PDCT))
			printk("there MIGHT be no TS :(\n");

	} else if (avia_gt_chip(GTX)) {

	//	rh(RR1)&=~0x1C;							 // take framer, ci, avi module out of reset
		gtx_reg_set(RR1, DAC, 1);
		gtx_reg_set(RR1, DAC, 0);

		gtx_reg_16(RR0) = 0;						// autsch, das muss so. kann das mal wer überprüfen?
		gtx_reg_16(RR1) = 0;
		gtx_reg_16(RISCCON) = 0;

		gtx_reg_16(FCR) = 0x9147;							 // byte wide input
		gtx_reg_16(SYNCH) = 0x21;

		gtx_reg_16(AVI) = 0x71F;
		gtx_reg_16(AVI+2) = 0xF;

	}

	memset(queue_list, 0, sizeof(queue_list));

	queue_addr = AVIA_GT_MEM_DMX_OFFS;

	for (queue_nr = 0; queue_nr < AVIA_GT_DMX_QUEUE_COUNT; queue_nr++) {

		queue_list[queue_nr].size = (1 << queue_size_table[queue_nr]) * 64;

		if (queue_addr & (queue_list[queue_nr].size - 1)) {

			printk("avia_gt_dmx: warning, misaligned queue %d (is 0x%X, size %d), aligning...\n", queue_nr, queue_addr, queue_list[queue_nr].size);

			queue_addr += queue_list[queue_nr].size;
			queue_addr &= ~(queue_list[queue_nr].size - 1);

		}

		queue_list[queue_nr].mem_addr = queue_addr;
		queue_addr += queue_list[queue_nr].size;

		queue_list[queue_nr].info.bytes_avail = avia_gt_dmx_get_queue_bytes_avail;
		queue_list[queue_nr].info.crc32 = avia_gt_dmx_queue_crc32;
		queue_list[queue_nr].info.get_data = avia_gt_dmx_queue_data_get;
		queue_list[queue_nr].info.get_data8 = avia_gt_dmx_queue_data_get8;
		queue_list[queue_nr].info.get_data16 = avia_gt_dmx_queue_data_get16;
		queue_list[queue_nr].info.get_data32 = avia_gt_dmx_queue_data_get32;
		queue_list[queue_nr].info.peek_data = avia_gt_dmx_queue_data_peek;
		queue_list[queue_nr].info.nr = queue_nr;
		queue_list[queue_nr].info.put_data = avia_gt_dmx_queue_data_put;

		if ((queue_nr == AVIA_GT_DMX_QUEUE_VIDEO) || (queue_nr == AVIA_GT_DMX_QUEUE_AUDIO) || (queue_nr == AVIA_GT_DMX_QUEUE_TELETEXT))
			avia_gt_dmx_system_queue_set_pos(queue_nr, 0, 0);

		avia_gt_dmx_queue_set_write_pos(queue_nr, 0);
		avia_gt_dmx_set_queue_irq(queue_nr, 0, -1);
		avia_gt_dmx_queue_irq_disable(queue_nr);

	}

	return 0;

}

void __exit avia_gt_dmx_exit(void)
{

	avia_gt_dmx_reset(0);

}

#ifdef MODULE
MODULE_PARM(ucode, "s");
MODULE_PARM(hw_sections, "i");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

EXPORT_SYMBOL(avia_gt_dmx_force_discontinuity);
EXPORT_SYMBOL(avia_gt_dmx_alloc_queue_audio);
EXPORT_SYMBOL(avia_gt_dmx_alloc_queue_message);
EXPORT_SYMBOL(avia_gt_dmx_alloc_queue_teletext);
EXPORT_SYMBOL(avia_gt_dmx_alloc_queue_user);
EXPORT_SYMBOL(avia_gt_dmx_alloc_queue_video);
EXPORT_SYMBOL(avia_gt_dmx_fake_queue_irq);
EXPORT_SYMBOL(avia_gt_dmx_free_queue);
EXPORT_SYMBOL(avia_gt_dmx_get_queue_info);
EXPORT_SYMBOL(avia_gt_dmx_get_queue_irq);
EXPORT_SYMBOL(avia_gt_dmx_set_queue_irq);

EXPORT_SYMBOL(avia_gt_dmx_queue_get_bytes_free);
EXPORT_SYMBOL(avia_gt_dmx_queue_get_write_pos);
EXPORT_SYMBOL(avia_gt_dmx_queue_irq_disable);
EXPORT_SYMBOL(avia_gt_dmx_queue_irq_enable);
EXPORT_SYMBOL(avia_gt_dmx_queue_reset);
EXPORT_SYMBOL(avia_gt_dmx_queue_set_write_pos);
EXPORT_SYMBOL(avia_gt_dmx_system_queue_set_pos);
EXPORT_SYMBOL(avia_gt_dmx_system_queue_set_read_pos);
EXPORT_SYMBOL(avia_gt_dmx_system_queue_set_write_pos);

EXPORT_SYMBOL(avia_gt_dmx_set_pcr_pid);
EXPORT_SYMBOL(avia_gt_dmx_set_pid_control_table);
EXPORT_SYMBOL(avia_gt_dmx_set_pid_table);
EXPORT_SYMBOL(avia_gt_dmx_get_hw_sec_filt_avail);
EXPORT_SYMBOL(avia_gt_dmx_set_section_filter);
EXPORT_SYMBOL(avia_gt_dmx_release_section_filter);
EXPORT_SYMBOL(avia_gt_dmx_set_filter_parameter_table);
#endif

#if defined(MODULE) && defined(STANDALONE)
module_init(avia_gt_dmx_init);
module_exit(avia_gt_dmx_exit);
#endif
