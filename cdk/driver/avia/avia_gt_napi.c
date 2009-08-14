/*
 *	 avia_gt_napi.c - AViA GTX demux driver (dbox-II-project)
 *
 *	 Homepage: http://dbox2.elxsi.de
 *
 *	 Copyright (C) 2000-2001 Felix "tmbinc" Domke (tmbinc@gmx.net)
 *
 *	 This program is free software; you can redistribute it and/or modify
 *	 it under the terms of the GNU General Public License as published by
 *	 the Free Software Foundation; either version 2 of the License, or
 *	 (at your option) any later version.
 *
 *	 This program is distributed in the hope that it will be useful,
 *	 but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 *	 GNU General Public License for more details.
 *
 *	 You should have received a copy of the GNU General Public License
 *	 along with this program; if not, write to the Free Software
 *	 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   $Revision: 1.141.2.1 $
 *   $Log: avia_gt_napi.c,v $
 *   Revision 1.141.2.1  2002/11/17 01:59:13  obi
 *   "backport" of latest dvb api version 1 drivers from HEAD branch
 *
 *   Revision 1.146  2002/11/11 21:35:32  wjoost
 *   SPTS: Software-Demultiplexing Audio-/Video-Queue
 *
 *   Revision 1.145  2002/11/07 20:07:32  wjoost
 *   streamen des aktuellen Programms im SPTS-Modus gefixed.
 *
 *   Revision 1.144  2002/11/06 19:34:41  wjoost
 *   oops--;
 *
 *   Revision 1.143  2002/11/04 20:43:36  wjoost
 *   IRQ-Handling fuers streamen geaendert
 *
 *   Revision 1.142  2002/10/26 16:46:18  wjoost
 *   bug--;
 *
 *   Revision 1.141  2002/10/09 20:20:36  Jolt
 *   DMX & Section fixes
 *
 *   Revision 1.140  2002/10/07 23:12:40  Jolt
 *   Bugfixes
 *
 *   Revision 1.139  2002/10/07 21:13:25  Jolt
 *   Cleanups / Fixes
 *
 *   Revision 1.138  2002/10/07 08:24:14  Jolt
 *   NAPI cleanups
 *
 *   Revision 1.137  2002/10/06 22:05:13  Jolt
 *   NAPI cleanups
 *
 *   Revision 1.136  2002/10/06 21:43:54  Jolt
 *   NAPI cleanups
 *
 *   Revision 1.135  2002/10/06 19:26:23  wjoost
 *   bug--;
 *
 *   Revision 1.134  2002/10/06 18:53:13  wjoost
 *   Debug-Code raus ;)
 *
 *   Revision 1.133  2002/10/06 18:49:02  wjoost
 *   Gleichzeitiger PES und TS-Empfang möglich, wenn TS zuerst gestartet wird.
 *
 *   Revision 1.132  2002/10/05 15:01:12  Jolt
 *   New NAPI compatible VBI interface
 *
 *   Revision 1.131  2002/09/30 19:46:10  Jolt
 *   SPTS support
 *
 *   Revision 1.130  2002/09/18 15:57:24  Jolt
 *   Queue handling changes #3
 *
 *   Revision 1.129  2002/09/18 12:13:20  Jolt
 *   Queue handling changes #2
 *
 *   Revision 1.128  2002/09/18 09:57:42  Jolt
 *   Queue handling changes
 *
 *   Revision 1.127  2002/09/16 21:41:37  wjoost
 *   noch was vergessen
 *
 *   Revision 1.126  2002/09/16 21:35:04  wjoost
 *   BUG hunting
 *
 *   Revision 1.125  2002/09/15 16:27:33  wjoost
 *   SW-Sections: no copy
 *   some fixes
 *
 *   Revision: 1.124
 *   Revision 1.124  2002/09/14 22:04:56  Jolt
 *   NAPI cleanup
 *
 *   Revision 1.123  2002/09/14 18:15:48  Jolt
 *   HW CRC for SW sections
 *
 *   Revision 1.122  2002/09/14 18:03:38  Jolt
 *   NAPI cleanup
 *
 *   Revision 1.121  2002/09/14 14:43:21  Jolt
 *   NAPI cleanup
 *
 *   Revision 1.120  2002/09/13 23:06:27  Jolt
 *   - Directly pass hw sections to napi
 *   - Enable hw crc for hw sections
 *
 *   Revision 1.119  2002/09/13 19:23:40  Jolt
 *   NAPI cleanup
 *
 *   Revision 1.118  2002/09/13 19:00:49  Jolt
 *   Changed queue handling
 *
 *   Revision 1.117  2002/09/12 14:58:52  Jolt
 *   HW sections bugfixes
 *
 *   Revision 1.116  2002/09/10 21:15:34  Jolt
 *   NAPI cleanup
 *
 *   Revision 1.115  2002/09/10 16:31:38  Jolt
 *   SW sections fix
 *
 *   Revision 1.114  2002/09/10 13:44:44  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.113  2002/09/09 21:59:01  Jolt
 *   HW sections fix
 *
 *   Revision 1.112  2002/09/09 18:30:36  Jolt
 *   Symbol fix
 *
 *   Revision 1.111  2002/09/09 17:46:30  Jolt
 *   Compile fix
 *
 *   Revision 1.110  2002/09/05 12:42:51  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.109  2002/09/05 12:30:53  Jolt
 *   NAPI cleanup
 *
 *   Revision 1.108  2002/09/05 11:57:44  Jolt
 *   NAPI bugfix
 *
 *   Revision 1.107  2002/09/05 09:40:32  Jolt
 *   - DMX/NAPI cleanup
 *   - Bugfixes (Thanks obi)
 *
 *   Revision 1.106  2002/09/04 22:40:47  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.105  2002/09/04 22:07:40  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.104  2002/09/04 21:12:52  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.103  2002/09/04 13:25:01  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.102  2002/09/04 07:46:29  Jolt
 *   - Removed GTX_SECTION
 *   - Removed auto pcr pid handling
 *
 *   Revision 1.101  2002/09/03 21:00:34  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.100  2002/09/03 15:37:50  wjoost
 *   Ein Bug weniger
 *
 *   Revision 1.99  2002/09/03 14:02:05  Jolt
 *   DMX/NAPI cleanup
 *
 *   Revision 1.98  2002/09/03 13:17:34  Jolt
 *   - DMX/NAPI cleanup
 *   - HW sections workaround
 *
 *   Revision 1.97  2002/09/03 05:17:38  Jolt
 *   HW sections workaround
 *
 *   Revision 1.96  2002/09/02 20:56:06  Jolt
 *   - HW section fix (GTX)
 *   - DMX/NAPI cleanups
 *
 *   Revision 1.95  2002/09/02 19:25:37  Jolt
 *   - DMX/NAPI cleanup
 *   - Compile fix
 *
 *   Revision 1.94  2002/09/01 17:50:51  wjoost
 *   I don't like #ifdef :-(
 *
 *   Revision 1.93  2002/08/25 09:38:26  wjoost
 *   Hardware Section Filtering
 *
 *   Revision 1.92  2002/08/24 09:36:07  Jolt
 *   Merge
 *
 *   Revision 1.91  2002/08/24 09:28:20  Jolt
 *   Compile fix
 *
 *   Revision 1.90  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.89  2002/06/11 20:35:43  Jolt
 *   Sections cleanup
 *
 *   Revision 1.88  2002/05/07 16:59:19  Jolt
 *   Misc stuff and cleanups
 *
 *   Revision 1.87  2002/05/06 12:58:37  Jolt
 *   obi[TM] fix 8-)
 *
 *   Revision 1.86  2002/05/06 02:18:18  obi
 *   cleanup for new kernel
 *
 *   Revision 1.85  2002/05/05 19:58:13  Jolt
 *   Doh 8-(
 *
 *   Revision 1.84  2002/05/04 17:05:53  Jolt
 *   PCR PID workaround
 *
 *   Revision 1.83  2002/05/03 17:06:44  obi
 *   replaced r*() by gtx_reg_()
 *
 *   Revision 1.82  2002/05/02 12:37:35  Jolt
 *   Merge
 *
 *   Revision 1.81  2002/05/02 04:56:47  Jolt
 *   Merge
 *
 *   Revision 1.80  2002/05/01 21:51:35  Jolt
 *   Merge
 *
 *   Revision 1.79  2002/04/22 17:40:01  Jolt
 *   Major cleanup
 *
 *   Revision 1.78  2002/04/19 11:31:53  Jolt
 *   Added missing module init stuff
 *
 *   Revision 1.77  2002/04/19 11:28:26  Jolt
 *   Final DMX merge
 *
 *   Revision 1.76  2002/04/19 11:02:43  obi
 *   build fix
 *
 *   Revision 1.75  2002/04/19 10:07:27  Jolt
 *   DMX merge
 *
 *   Revision 1.74  2002/04/18 18:17:37  happydude
 *   deactivate pcr pid failsafe
 *
 *   Revision 1.73  2002/04/14 18:06:19  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.72  2002/04/13 23:19:05  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.71  2002/04/12 23:20:25  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.70  2002/04/12 14:28:13  Jolt
 *   eNX/GTX merge
 *
 *   Revision 1.69  2002/03/19 18:32:25  happydude
 *   allow seperate setting of pcr pid
 *
 *   Revision 1.68  2002/02/24 15:29:23  woglinde
 *   test new tuner-api
 *
 *   Revision 1.66.2.1  2002/02/09 20:44:01  TripleDES
 *   fixes
 *
 *   CV: ----------------------------------------------------------------------
 *
 *   Revision 1.66  2002/01/18 14:48:52  tmbinc
 *   small fix for multiple pid streaming
 *
 *   Revision 1.65  2002/01/02 19:45:50  tmbinc
 *   added support for streaming a pid to multiple clients.
 *
 *   Revision 1.64  2002/01/02 04:40:08  McClean
 *   make OUT OF SYNC dprintf
 *
 *   Revision 1.63  2001/12/19 13:19:17  derget
 *   debugoutput entfernt
 *   CHCH [DEMUX] START
 *   CHCH [DEMUX] STOP
 *   wer braucht das schon ..
 *
 *   Revision 1.62  2001/12/17 19:29:51  gillem
 *   - sync with includes
 *
 *   Revision 1.61  2001/12/01 06:37:06  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.60  2001/11/14 17:59:22  wjoost
 *   Section-Empfang geaendert (Pruefung auf maximale Groesse, zusammenhaengende TS-Pakete)
 *
 *	 Revision 1.55	2001/09/02 01:28:34	TripleDES
 *	 -small fix (dac)
 *
 *	 Revision 1.54	2001/09/02 01:16:42	TripleDES
 *	 -more fixes (corrects my wrong commit)
 *
 *	 Revision 1.52	2001/08/18 18:59:40	tmbinc
 *	 fixed init
 *
 *	 Revision 1.51	2001/08/18 18:20:21	TripleDES
 *	 moved the ucode loading to dmx
 *
 *	 Revision 1.50	2001/08/15 14:57:44	tmbinc
 *	 fixed queue-reset
 *
 *	 Revision 1.49	2001/07/15 17:08:45	Toerli
 *	 Flimmern bei Sagem beseitigt
 *
 *	 Revision 1.48	2001/06/25 22:27:22	gillem
 *	 - start autodetect
 *
 *	 Revision 1.47	2001/06/22 00:03:42	tmbinc
 *	 fixed aligning of queues, system queues
 *
 *	 Revision 1.46	2001/06/18 20:30:59	tmbinc
 *	 decent buffersizes. change for your needs.
 *
 *	 Revision 1.45	2001/06/18 20:14:08	tmbinc
 *	 fixed state sets in sections, now it works as expected (multiple filter on one pid)
 *
 *	 Revision 1.44	2001/06/15 00:17:26	TripleDES
 *	 fixed queue-reset problem - solves zap problem with enx
 *
 *	 Revision 1.43	2001/04/28 22:43:13	fnbrd
 *	 Added fix from tmbinc. ;)
 *
 *	 Revision 1.42	2001/04/24 17:54:14	tmbinc
 *	 fixed 188bytes-on-callback bug
 *
 *	 Revision 1.41	2001/04/22 13:56:35	tmbinc
 *	 other philips- (and maybe sagem?) fixes
 *
 *	 Revision 1.40	2001/04/21 13:08:57	tmbinc
 *	 eNX now works on philips.
 *
 *	 Revision 1.39	2001/04/21 10:57:41	tmbinc
 *	 small fix.
 *
 *	 Revision 1.38	2001/04/21 10:40:13	tmbinc
 *	 fixes for eNX
 *
 *	 Revision 1.37	2001/04/19 02:14:43	tmbinc
 *	 renamed gtx-dmx.c to gen-dmx.c
 *
 *
 *	 old log: gtx-dmx.c,v
 *	 Revision 1.36	2001/04/10 03:07:29	Hunz
 *	 1st nokia/sat fix - supported by Wodka Gorbatschow *oerks* ;)
 *
 *	 Revision 1.35	2001/04/09 23:26:42	TripleDES
 *	 some changes
 *
 *	 Revision 1.34	2001/04/08 22:22:29	TripleDES
 *	 added eNX support
 *	 -every register/ucode access is temporarily duplicated for eNX testing - will be cleared soon ;)
 *	 -up to now there is no section support for eNX
 *	 -need to rewrite the register-defines gReg for gtx, eReg for enx (perhaps/tmb?)
 *	 -queue-interrupts are not correct for eNX
 *	 -uncomment the "#define enx_dmx" for testing eNX
 *
 *	 Revision 1.33	2001/04/08 02:05:40	tmbinc
 *	 made it more modular, this time the demux. dvb.c not anymore dependant on
 *	 the gtx.
 *
 *	 Revision 1.32	2001/03/30 01:19:55	tmbinc
 *	 Fixed multiple-section bug.
 *
 *	 Revision 1.31	2001/03/27 14:41:49	tmbinc
 *	 CRC check now optional.
 *
 *	 Revision 1.30	2001/03/21 15:30:25	tmbinc
 *	 Added SYNC-delay for avia, resulting in faster zap-time.
 *
 *	 Revision 1.29	2001/03/19 17:48:32	tmbinc
 *	 re-fixed a fixed fix by gillem.
 *
 *	 Revision 1.28	2001/03/19 16:24:32	tmbinc
 *	 fixed section parsing bugs.
 *
 *	 Revision 1.27	2001/03/16 19:50:28	gillem
 *	 - fix section parser
 *
 *	 Revision 1.26	2001/03/16 17:49:56	gillem
 *	 - fix section parser
 *
 *	 Revision 1.25	2001/03/15 15:56:26	gillem
 *	 - fix dprintk output
 *
 *	 Revision 1.24	2001/03/14 21:42:48	gillem
 *	 - fix bugs in section parsing
 *	 - add crc32 check in section parsing
 *
 *	 Revision 1.23	2001/03/12 22:32:01	gillem
 *	 - test only ... sections not work
 *
 *	 Revision 1.22	2001/03/11 22:58:09	gillem
 *	 - fix ts parser
 *
 *	 Revision 1.21	2001/03/11 21:34:29	gillem
 *	 - fix af parser
 *
 *	 Revision 1.20	2001/03/10 02:46:14	tmbinc
 *	 Fixed section support.
 *
 *	 Revision 1.19	2001/03/10 00:41:21	tmbinc
 *	 Fixed section handling.
 *
 *	 Revision 1.18	2001/03/09 22:10:20	tmbinc
 *	 Completed first table support (untested)
 *
 *	 Revision 1.17	2001/03/09 20:48:31	gillem
 *	 - add debug option
 *
 *	 Revision 1.16	2001/03/07 22:25:14	tmbinc
 *	 Tried to fix PCR.
 *
 *	 Revision 1.15	2001/03/04 14:15:42	tmbinc
 *	 fixed ucode-version autodetection.
 *
 *	 Revision 1.14	2001/03/04 13:03:17	tmbinc
 *	 Removed %188 bytes check (for PES)
 *
 *	 Revision 1.13	2001/02/27 14:15:22	tmbinc
 *	 added sections.
 *
 *	 Revision 1.12	2001/02/17 01:19:19	tmbinc
 *	 fixed DPCR
 *
 *	 Revision 1.11	2001/02/11 16:01:06	tmbinc
 *	 *** empty log message ***
 *
 *	 Revision 1.10	2001/02/11 15:53:25	tmbinc
 *	 section filtering (not yet working)
 *
 *	 Revision 1.9	2001/02/10 14:31:52	gillem
 *	 add GtxDmxCleanup function
 *
 *	 Revision 1.8	2001/01/31 17:17:46	tmbinc
 *	 Cleaned up avia drivers. - tmb
 *
 *	 last (old) Revision: 1.36
 *
 */

/*
		This driver implements the Nokia-DVB-Api (Kernel level Demux driver),
		but it isn't yet complete.

		It does not support descrambling (and some minor features as well).

		Writing isn't supported, either.
 */
#define __KERNEL_SYSCALLS__

#include <linux/string.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/tqueue.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/8xx_immap.h>
#include <asm/pgtable.h>
#include <asm/mpc8xx.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>

#include <ost/demux.h>

#include <dbox/avia_gt.h>
#include <dbox/avia_gt_dmx.h>
#include <dbox/avia_gt_napi.h>

static sAviaGtInfo *gt_info = (sAviaGtInfo *)NULL;

//#undef dprintk
//#define dprintk printk


#ifdef MODULE
MODULE_AUTHOR("Felix Domke <tmbinc@gmx.net>");
MODULE_DESCRIPTION("Avia eNX/GTX demux driver");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#endif

static gtx_demux_t gtx;

static int GtxDmxInit(gtx_demux_t *gtxdemux);
static int GtxDmxCleanup(gtx_demux_t *gtxdemux);
static void dmx_set_filter(gtx_demux_filter_t *filter);

#if 0
static unsigned short rb[4096], rrb[32];
#endif

#if 0
		/* hehe da kapiert ja eh niemand was das soll, ausser willid und TripleDES vielleicht, also kanns hier auch bleiben :) */
		/* man beachte: hier spionieren wir geheimste C-cube infos aus, indem wir auf undokumentierte register zugreifen!      */
void enx_tdp_trace(void)
{
	int i, oldpc=-1;
	unsigned short *r=(unsigned short*)enx_reg_o(TDP_INSTR_RAM);
	enx_reg_16(EC) = 0x03;			//start tdp
	memcpy(rb, r, 4096*2);
	memset(rrb, 0x76, 32*2);
	for (i=0; i<1000; i++)
	{
		int j;
		int pc=enx_reg_16(EPC);
		for (j=0; j<4096; j++)
			if (rb[j]!=r[j])
			{
				printk("(%04x: %04x -> %04x)\n", j, rb[j], r[j]);
				rb[j]=r[j];
			}
		if (pc!=oldpc)
		{
			int a;

			for (a=0; a<32; a++)
			{
				int tr=enx_reg_16(EPC-32+a*2);
				if (rrb[a]!=tr)
					printk("%04x ", enx_reg_16(EPC-32+a*2));
				else
					printk("     ");
				rrb[a]=tr;
			}
			printk("\n");
			printk("%03x (%04x) %x\n", pc, r[pc], (r[pc]>>3)&0xFF);
		}
		oldpc=pc;
		enx_reg_16(EC) = 0x03;			// und wieder nen stueck...
	}
	enx_reg_16(EC)=0;
}
#endif

extern int register_demux(struct dmx_demux_s *demux);
extern int unregister_demux(struct dmx_demux_s *demux);

void gtx_dmx_close(void)
{

	u8 queue_nr;

	for (queue_nr = 0; queue_nr < 32; queue_nr++)
		avia_gt_free_irq(avia_gt_dmx_get_queue_irq(queue_nr));

	if (avia_gt_chip(ENX))
		avia_gt_free_irq(ENX_IRQ_PCR);
	else if (avia_gt_chip(GTX))
		avia_gt_free_irq(GTX_IRQ_PCR);					

	unregister_demux(&gtx.dmx);
	GtxDmxCleanup(&gtx);

}

#if 0
static void dump(unsigned char *b1, unsigned b1l, unsigned char *b2, unsigned b2l)
{
	unsigned i = 0;

	while (b1l + b2l > 0)
	{
		if (b1l)
		{
			printk("%02X ",*b1 & 0xFF);
			b1l--;
			b1++;
		}
		else
		{
			printk("%02X ",*b2 & 0xFF);
			b2l--;
			b2++;
		}
		i++;
		if ( (i & 0x0F) == 0)
		{
			printk("\n");
		}
	}
	printk("\n");
}
#endif

								// nokia api

static void gtx_handle_section(gtx_demux_feed_t *gtxfeed)
{
	gtx_demux_secfilter_t *secfilter = (gtx_demux_secfilter_t *)NULL;
	int ok,i;
	unsigned max_check = DMX_MAX_FILTER_SIZE;

	if (gtxfeed->sec_recv != gtxfeed->sec_len)
	{
		dprintk("gtx_dmx: have: %d, want %d\n", gtxfeed->sec_recv, gtxfeed->sec_len);
	}

	if (!gtxfeed->sec_recv)
	{
		gtxfeed->sec_len=gtxfeed->sec_recv=0;
		return;
	}

	/*
	 * linux_dvb_api.pdf, allocate_filter():
	 * [..] Note that on most demux hardware it is not possible to filter on
	 * the section length field of the section header thus this field is
	 * ignored, even though it is included in filter value and filter mask
	 * fields.
	 */

	if (gtxfeed->sec_len < max_check)
	{
		max_check = gtxfeed->sec_len;
	}

	for (secfilter=gtxfeed->secfilter; secfilter; secfilter=secfilter->next)
	{
		ok = 1;
		i = 0;
		while ( (i < max_check) && ok)
		{
			if ( i == 1 )
			{
				i = 3;
			}
			if ( ((gtxfeed->sec_buffer[i]^secfilter->filter.filter_value[i])&secfilter->filter.filter_mask[i]) )
			{
				ok=0;
			}
			else
			{
				i++;
			}
		}

		if (ok)	{

			if (!gtxfeed->sec_crc)
				gtxfeed->cb.sec(gtxfeed->sec_buffer, gtxfeed->sec_len, 0, 0, &secfilter->filter, 0);
			else
				dprintk("gtx_dmx: CRC Problem !!!\n");
		}
	}

	gtxfeed->sec_len = 0;
	gtxfeed->sec_recv = 0;
	gtxfeed->sec_crc = 0;

}

static int avia_gt_napi_handle_section(gtx_demux_feed_t *gtxfeed, u8 queue_nr, u16 sec_len)
{

	gtx_demux_secfilter_t *secfilter = (gtx_demux_secfilter_t *)NULL;
	sAviaGtDmxQueue *queue = avia_gt_dmx_get_queue_info(queue_nr);
	int ok,i;
	int recover = 0;
	u32 chunk1;
	unsigned max_check = DMX_MAX_FILTER_SIZE;
	unsigned char copied = 0;


	if (sec_len < DMX_MAX_FILTER_SIZE)
	{
		max_check = sec_len;
		copied = 1;
		queue->info.get_data(queue_nr, gtxfeed->sec_buffer, sec_len, 0);
	}
	else if (gtxfeed->secfilter->next)      // mehr als ein client ?
	{
		queue->info.get_data(queue_nr, gtxfeed->sec_buffer, sec_len, 0);
		copied = 1;
	}
	else
	{
		queue->info.get_data(queue_nr, gtxfeed->sec_buffer, DMX_MAX_FILTER_SIZE, 1);
	}

	/*
	 * linux_dvb_api.pdf, allocate_filter():
	 * [..] Note that on most demux hardware it is not possible to filter on
	 * the section length field of the section header thus this field is
	 * ignored, even though it is included in filter value and filter mask
	 * fields.
	 */

	for (secfilter = gtxfeed->secfilter; secfilter; secfilter = secfilter->next) {

		ok = 1;
		i = 0;

		while ((i < max_check) && ok) {

			if (i == 1)
				i = 3;

			if (((gtxfeed->sec_buffer[i] ^ secfilter->filter.filter_value[i]) & secfilter->filter.filter_mask[i]))
				ok = 0;
			else
				i++;

		}

		if (ok)	{

			recover++;

			if (!copied)
			{
				if ((queue->read_pos + sec_len) > (queue->size))
				{
					chunk1 = queue->size - queue->read_pos;
					gtxfeed->cb.sec(gt_info->mem_addr + queue->mem_addr + queue->read_pos, chunk1, gt_info->mem_addr + queue->mem_addr, sec_len - chunk1, &secfilter->filter, 0);

				} else
				{
					gtxfeed->cb.sec(gt_info->mem_addr + queue->mem_addr + queue->read_pos, sec_len, NULL, 0, &secfilter->filter, 0);
				}
			}
			else
			{
				gtxfeed->cb.sec(gtxfeed->sec_buffer, sec_len, NULL, 0, &secfilter->filter, 0);
			}
		} else if (i >= 10) {
			recover++;
		}
	}

	if (!copied)
	{
		queue->info.get_data(queue_nr, NULL, sec_len, 0);
	}

	gtxfeed->sec_len = 0;
	gtxfeed->sec_recv = 0;

	return recover;

}

static s16 avia_gt_napi_strip_header_ts(gtx_demux_feed_t *gtxfeed, u8 queue_nr, sDVBTsHeader *ts_header)
{

	u8 adaptation_len = 0;
	sAviaGtDmxQueue *queue = avia_gt_dmx_get_queue_info(queue_nr);
	sAviaGtDmxQueueInfo *queue_info = &queue->info;
	sDVBTsHeader backup_ts_header;
	
	if (!ts_header)
		ts_header = &backup_ts_header;

	if (queue_info->bytes_avail(queue_nr) < 188)
		return -1;

	while (queue_info->get_data8(queue_nr, 1) != 0x47) {

		if (queue_info->bytes_avail(queue_nr) > 188) {

			queue_info->get_data(queue_nr, NULL, 1, 0);

		} else {
		
			printk("avia_gt_napi: ts stream out of sync\n");
			
			return -1;
			
		}

	}

	queue_info->get_data(queue_nr, ts_header, sizeof(ts_header), 0);

	if (!ts_header->payload) {		

		printk("avia_gt_napi: no payload\n");

		queue_info->get_data(queue_nr, NULL, 188 - sizeof(ts_header), 0);
		
		return 0;

	}

	if (ts_header->continuity_counter == gtxfeed->sec_ccn) {

		printk("avia_gt_napi: duplicate packet\n");

		queue_info->get_data(queue_nr, NULL, 188 - sizeof(ts_header), 0);

		return 0;
		
	}

	if ((((ts_header->continuity_counter > 0) && (ts_header->continuity_counter != (gtxfeed->sec_ccn + 1))) ||
		((ts_header->continuity_counter == 0) && (gtxfeed->sec_ccn != 15))) && (gtxfeed->sec_recv > 0)) {

		printk("avia_gt_napi: lost packet\n");

		gtxfeed->sec_recv = 0;
		gtxfeed->sec_len = 0;
		gtxfeed->sec_crc = 0;

	}

	gtxfeed->sec_ccn = ts_header->continuity_counter;

	if (ts_header->adaptation_field) {

		printk("avia_gt_napi: adaptation field (%d bytes)\n", adaptation_len);

		adaptation_len = queue_info->get_data8(queue_nr, 1) + 1;

		if (adaptation_len > 183)	{

			printk("avia_gt_napi: warning afle=%d (ignore)\n", adaptation_len);

			queue_info->get_data(queue_nr, NULL, 188 - sizeof(ts_header), 0);

			return 0;

		}

		queue_info->get_data(queue_nr, NULL, adaptation_len, 0);

	}
	
	return (188 - sizeof(ts_header) - adaptation_len);

}

void avia_gt_napi_message_callback(u8 queue_nr, void *data)
{

	gtx_demux_t *gtx=(gtx_demux_t*)data;
	sAviaGtDmxQueue *queue = avia_gt_dmx_get_queue_info(queue_nr);
	sAviaGtDmxQueueInfo *queue_info = &queue->info;
	static char sync_lost = 0;
	sCC_ERROR_MESSAGE msg;
	sSECTION_COMPLETED_MESSAGE comp_msg;
	sPRIVATE_ADAPTION_MESSAGE adaptation;
	unsigned i;
	__u32 blocked = 0;

	if (queue_nr != AVIA_GT_DMX_QUEUE_MESSAGE) {

		printk("avia_gt_napi: unexpected queue %d in dmx message handler\n", queue_nr);

		return;

	}

	while (queue_info->bytes_avail(queue_nr) > 0) {

		switch(queue_info->get_data8(queue_nr, 1)) {
		
			case DMX_MESSAGE_CC_ERROR:
								
				sync_lost = 0;

				if (queue_info->bytes_avail(queue_nr) < sizeof(msg)) {
									
					printk("avia_gt_napi: short CC-error-message received.\n");
										
					avia_gt_dmx_queue_reset(queue_nr);

					return;
										
				}
									
				queue_info->get_data(queue_nr, &msg, sizeof(msg), 0);

				for (i = USER_QUEUE_START; i < LAST_USER_QUEUE; i++) {

					if ((gtx->feed[i].state == DMX_STATE_GO) &&
						(gtx->feed[i].type == DMX_TYPE_HW_SEC) &&
						(gtx->feed[i].filter->pid == (msg.pid & 0x1FFF))) {

						dprintk("avia_gt_napi: cc discontinuity on feed %d\n",i);
						
						gtx->feed[i].filter->invalid = 1;
						dmx_set_filter(gtx->feed[i].filter);
						gtx->feed[i].filter->invalid = 0;
						avia_gt_dmx_queue_reset(i);
						gtx->feed[i].sec_len = 0;
						gtx->feed[i].sec_recv = 0;
						blocked |= 1 << i;

						dmx_set_filter(gtx->feed[i].filter);
						
						return;
						
					}

				}

			break;
			
			case DMX_MESSAGE_ADAPTION:

				sync_lost = 0;
									
				if (queue_info->bytes_avail(queue_nr) < sizeof(adaptation))	{
									
					printk("avia_gt_napi: short private adaptation field message.\n");

					avia_gt_dmx_queue_reset(queue_nr);
										
					return;
								
				}

				queue_info->get_data(queue_nr, &adaptation, sizeof(adaptation), 0);
									
				if (queue_info->bytes_avail(queue_nr) < adaptation.length) {
									
					printk("avia_gt_napi: short private adaptation field message.\n");

					avia_gt_dmx_queue_reset(queue_nr);

					return;
										
				}

				queue_info->get_data(queue_nr, NULL, adaptation.length, 0);

			break;

			case DMX_MESSAGE_SYNC_LOSS:
			
				queue_info->get_data8(queue_nr, 0);
#if 0
				if (!sync_lost) {

					sync_lost = 1;
					printk("avia_gt_napi: lost sync\n");

					for (i = USER_QUEUE_START; i < LAST_USER_QUEUE; i++) {
					
						if ((gtx->feed[i].state == DMX_STATE_GO) &&	(gtx->feed[i].type == DMX_TYPE_HW_SEC))	{

							gtx->feed[i].filter->invalid = 1;
							dmx_set_filter(gtx->feed[i].filter);
							gtx->feed[i].filter->invalid = 0;
							avia_gt_dmx_queue_reset(i);
							gtx->feed[i].sec_len = 0;
							gtx->feed[i].sec_recv = 0;
							blocked |= 1 << i;
							
							dmx_set_filter(gtx->feed[i].filter);

							return;
							
						}
						
					}
					
				}
#endif
			break;

			case DMX_MESSAGE_SECTION_COMPLETED:

				if (queue_info->bytes_avail(queue_nr) < sizeof(comp_msg))	{

					printk("avia_gt_napi: short section completed message.\n");

					avia_gt_dmx_queue_reset(queue_nr);

					return;

				}

				queue_info->get_data(queue_nr, &comp_msg, sizeof(comp_msg), 0);

				if (!(blocked & (1 << comp_msg.filter_index))) {

					sync_lost = 0;

					//avia_gt_dmx_fake_queue_irq(comp_msg.filter_index);

				}

			break;

			default:

				printk("avia_gt_napi: bad message, type-value %02X, len = %d\n", queue_info->get_data8(queue_nr, 1), queue_info->bytes_avail(queue_nr));

				avia_gt_dmx_queue_reset(queue_nr);

				return;

			break;

		}

	}

}

void avia_gt_handle_ts_data(gtx_demux_t *gtx, sAviaGtDmxQueue *queue,sAviaGtDmxQueueInfo *queue_info, u8 queue_nr, gtx_demux_feed_t *gtxfeed, u32 buf_len)
{
	u32 i;
	int chunk1;
	u32 need_payload = 0;
	sDVBTsHeader ts_header;
	s32 payload_len;

	for (i = USER_QUEUE_START; i < LAST_USER_QUEUE; i++) {

		if ((gtx->feed[i].state == DMX_STATE_GO) &&
			(gtx->feed[i].pid == gtxfeed->pid) &&
			(gtx->feed[i].output & TS_PACKET)) {

			if (!((gtxfeed->output^gtx->feed[i].output) & TS_PAYLOAD_ONLY)) {

				if ((queue->read_pos + buf_len) > queue->size) {

					chunk1 = queue->size - queue->read_pos;
					gtx->feed[i].cb.ts(gt_info->mem_addr + queue->mem_addr + queue->read_pos, chunk1, gt_info->mem_addr + queue->mem_addr, buf_len - chunk1, &gtx->feed[i].feed.ts, 0);

				} else {

					gtx->feed[i].cb.ts(gt_info->mem_addr + queue->mem_addr + queue->read_pos, buf_len, NULL, 0, &gtx->feed[i].feed.ts, 0);

				}

			} else if (gtx->feed[i].output & TS_PAYLOAD_ONLY) {

				need_payload++;

			}

		}

	}

	// Emuliere Payload only für feeds die mit TS-Header laufen

	if (need_payload > 0) {

		while (buf_len >= 188) {

			payload_len = avia_gt_napi_strip_header_ts(gtxfeed, queue_nr, &ts_header);

			if (payload_len < 0)
				return;

			if (!payload_len)
				continue;

			for (i = USER_QUEUE_START; i < LAST_USER_QUEUE; i++) {

				if ((gtx->feed[i].state == DMX_STATE_GO) &&
					(gtx->feed[i].pid == gtxfeed->pid) &&
					(gtx->feed[i].output & TS_PACKET) &&
					(gtx->feed[i].output & TS_PAYLOAD_ONLY)) {

					if ((queue->read_pos + payload_len) > queue->size) {

						chunk1 = queue->size - queue->read_pos;

						gtx->feed[i].cb.ts(gt_info->mem_addr + queue->mem_addr + queue->read_pos, chunk1, gt_info->mem_addr + queue->mem_addr, payload_len - chunk1, &gtx->feed[i].feed.ts, 0);

					} else {

						gtx->feed[i].cb.ts(gt_info->mem_addr + queue->mem_addr + queue->read_pos, payload_len, NULL, 0, &gtx->feed[i].feed.ts, 0);

					}

				}

			}

			queue_info->get_data(queue_nr, NULL, payload_len, 0);

			buf_len -= 188;

		}

	} else {

		gtxfeed->sec_ccn = 16;

		queue_info->get_data(queue_nr, NULL, buf_len, 0);

	}

}

void avia_gt_napi_queue_callback(u8 queue_nr, void *data)
{

	sAviaGtDmxQueue *queue = avia_gt_dmx_get_queue_info(queue_nr);
	sAviaGtDmxQueueInfo *queue_info = &queue->info;
	gtx_demux_t *gtx=(gtx_demux_t*)data;
	gtx_demux_feed_t *gtxfeed = gtx->feed + queue_nr;
	u8 section_header[3];
	u32 padding;
	u8 ts_header_old[5];
	u8 next_sec_offs;
	u32 buf_len;
	u32 chunk1;
	int i;
	u32 need_payload;
	s32 payload_len;
	sDVBTsHeader ts_header;
	unsigned char pid1_high, pid1_low;
#ifdef AVIA_SPTS
	u16 pid;
	unsigned char pid2_high, pid2_low;
#endif

	if (gtxfeed->state != DMX_STATE_GO) {

		dprintk("gtx_dmx: DEBUG: interrupt on non-GO feed, queue %d\n!", queue_nr);

	} else {

		switch (gtxfeed->type) {

			case DMX_TYPE_TS:

				if ( (buf_len = queue_info->bytes_avail(queue_nr)) < 188)
					return;

				need_payload = 0;

				/* Wir können bei den TS-Queues aus der Synchronisation kommen,
				   die zur Dekodierung an den AVIA weitergeleitet aber
				   sonst nicht verarbeitet werden (Videotext und Audio-/Video im
				   SPTS-Modus).
				 */

				pid1_high = gtxfeed->pid >> 8;
				pid1_low  = gtxfeed->pid & 0xFF;
#ifdef AVIA_SPTS
				if (queue_nr == 0)
				{
					pid2_high = gtx->feed[1].pid >> 8;
					pid2_low  = gtx->feed[1].pid & 0xFF;
				}
				else
				{
					pid2_high = pid1_high;
					pid2_low  = pid1_low;
				}
#endif

				queue_info->get_data(queue_nr, ts_header_old, sizeof(ts_header_old), 1);

				while (buf_len >= 188) {

					if ((ts_header_old[0] != 0x47) ||
						(
						  ( ((ts_header_old[1] & 0x1F) != pid1_high) ||
						    (ts_header_old[2] != pid1_low)
						  )
#ifdef AVIA_SPTS
						  &&
						  ( ((ts_header_old[1] & 0x1F) != pid2_high) ||
						    (ts_header_old[2] != pid2_low)
						  )
#endif
						)
					   ) {
						queue_info->get_data(queue_nr, NULL, 1, 0);
						queue_info->get_data(queue_nr, ts_header_old, sizeof(ts_header_old), 1);

						buf_len--;

					} else {

						break;

					}

				}

				if (buf_len < 188)
					return;

				buf_len -= buf_len % 188;

#ifdef AVIA_SPTS
				if (queue_nr != 0)
				{
#endif
					avia_gt_handle_ts_data(gtx,queue,queue_info,queue_nr,gtxfeed,buf_len);
#ifdef AVIA_SPTS
				}
				else
				{
					payload_len = 0;
					queue_info->get_data(queue_nr, ts_header_old, 3, 1);
					pid = ((ts_header_old[1] & 0x1F) << 8) | ts_header_old[2];
					while (buf_len >= 188)
					{
						queue_info->peek_data(queue_nr, payload_len, ts_header_old, 3);
						if ( pid != ( ((ts_header_old[1] & 0x1F) << 8) | ts_header_old[2]) )
						{
							i = (pid != gtxfeed->pid);
							avia_gt_handle_ts_data(gtx,queue,queue_info,0,&gtx->feed[i],payload_len);
							payload_len = 0;
						}
						pid = ((ts_header_old[1] & 0x1F) << 8) | ts_header_old[2];
						payload_len += 188;
						buf_len -= 188;
					}
					i = (pid != gtxfeed->pid);
					avia_gt_handle_ts_data(gtx,queue,queue_info,0,&gtx->feed[i],payload_len);
				}
#endif

				return;

			break;

			// handle prefiltered section
			case DMX_TYPE_HW_SEC:

				while (queue_info->bytes_avail(queue_nr) > 0) {

					while ((queue_info->bytes_avail(queue_nr)) && (queue_info->get_data8(queue_nr, 1) == 0xFF))
						queue_info->get_data(queue_nr, NULL, 1, 0);

					if (queue_info->bytes_avail(queue_nr) < 3)
						break;

					queue_info->get_data(queue_nr, section_header, sizeof(section_header), 1);
					gtxfeed->sec_len = (((section_header[1] & 0x0F) << 8) | section_header[2]) + 3;

					if (gtxfeed->sec_len > 4096) {

						dprintk(KERN_ERR "avia_gt_napi: section length %d > 4096!\n", gtxfeed->sec_len);

						gtxfeed->filter->invalid = 1;
						dmx_set_filter(gtxfeed->filter);
						gtxfeed->filter->invalid = 0;
						avia_gt_dmx_queue_reset(queue_nr);
						dmx_set_filter(gtxfeed->filter);

						return;

					}

					if (gtxfeed->sec_len > queue_info->bytes_avail(queue_nr)) {

						dprintk(KERN_ERR "avia_gt_napi: incomplete section: want %d have %d!\n", gtxfeed->sec_len, queue_info->bytes_avail(queue_nr));
						
						return;

					}

					if ((gtxfeed->check_crc) && (queue_info->crc32(queue_nr, gtxfeed->sec_len, 0))) {
					
						dprintk("avia_gt_napi: section CRC invalid\n");
						
						queue_info->get_data(queue_nr, NULL, gtxfeed->sec_len, 0);
						
						continue;
						
					}

					if (avia_gt_napi_handle_section(gtxfeed, queue_nr, gtxfeed->sec_len) == 0) {
					
						gtxfeed->filter->invalid = 1;
						dmx_set_filter(gtxfeed->filter);
						gtxfeed->filter->invalid = 0;
						avia_gt_dmx_queue_reset(queue_nr);
						dmx_set_filter(gtxfeed->filter);

						return;
						
					}

				}

			break;

			case DMX_TYPE_PES:

				payload_len = queue_info->bytes_avail(queue_nr);

				for (i = USER_QUEUE_START; i < LAST_USER_QUEUE; i++) {

					if ((gtx->feed[i].state == DMX_STATE_GO) &&
						(gtx->feed[i].pid == gtxfeed->pid) &&
						(gtx->feed[i].type == DMX_TYPE_PES)) {

						if ((queue->read_pos + payload_len) > queue->size) {
							chunk1 = queue->size - queue->read_pos;
							gtx->feed[i].cb.ts(gt_info->mem_addr + queue->mem_addr + queue->read_pos, chunk1, gt_info->mem_addr + queue->mem_addr, payload_len - chunk1, &gtx->feed[i].feed.ts, 0);
						} else {
							gtx->feed[i].cb.ts(gt_info->mem_addr + queue->mem_addr + queue->read_pos, payload_len, NULL, 0, &gtx->feed[i].feed.ts, 0);
						}
						
					}
					
				}
				
				queue->read_pos = queue->write_pos;
			
			break;

			case DMX_TYPE_SEC:

				padding = queue_info->bytes_avail(queue_nr) % 188;

				// let's rock
				while (queue_info->bytes_avail(queue_nr) > padding)	{
				
					int r = 0;
					
					payload_len = avia_gt_napi_strip_header_ts(gtxfeed, queue_nr, &ts_header);
					
					if (payload_len < 0)
						return;
							
					if (!payload_len)
						continue;

					if (ts_header.payload_unit_start_indicator) {

						next_sec_offs = queue_info->get_data8(queue_nr, 0);

						if (next_sec_offs) {								// neues Paket fängt mittendrin an

							if (gtxfeed->sec_recv) {				// haben wir den Anfang des vorherigen Paketes ?

								r = gtxfeed->sec_len - gtxfeed->sec_recv;

								if (r > next_sec_offs) {					// wenn eine neue Section kommt muß die vorherige abgeschlossen werden

									dprintk("gtx_dmx: dropping section because length-confusion.\n");

								} else {

									if (gtxfeed->check_crc)
										gtxfeed->sec_crc = queue_info->crc32(queue_nr, r, gtxfeed->sec_crc);

									queue_info->get_data(queue_nr, gtxfeed->sec_buffer + gtxfeed->sec_recv, r, 0);

									gtxfeed->sec_recv += r;

									gtx_handle_section(gtxfeed);

								}

							} else {
							
								queue_info->get_data(queue_nr, NULL, next_sec_offs, 0);

							}
							
							payload_len -= next_sec_offs + 1;
							
						} else {

							payload_len--;

						}

						gtxfeed->sec_recv = 0;
						gtxfeed->sec_len = 0;
						gtxfeed->sec_crc = 0;

					} else if (gtxfeed->sec_recv == 0) {	// kein Paketstart und keine bereits angefangene section im Puffer

						queue_info->get_data(queue_nr, NULL, payload_len, 0);

						continue;
						
					}

					while (payload_len) {

						if (gtxfeed->sec_recv) {				// haben bereits einen Anfang

							r = gtxfeed->sec_len - gtxfeed->sec_recv;

							if (r > payload_len)										// ein kleines Teilstück kommt hinzu
								r = payload_len;

							if (gtxfeed->check_crc)
								gtxfeed->sec_crc = queue_info->crc32(queue_nr, r, gtxfeed->sec_crc);

							queue_info->get_data(queue_nr, gtxfeed->sec_buffer + gtxfeed->sec_recv, r, 0);

							gtxfeed->sec_recv += r;

							if (gtxfeed->sec_len == gtxfeed->sec_recv)
								gtx_handle_section(gtxfeed);

						} else {													// neue Section

							if ((queue_info->get_data8(queue_nr, 1) == 0xFF) || (payload_len < 3)) {

								// Get rid of padding bytes
								if (payload_len)
									queue_info->get_data(queue_nr, NULL, payload_len, 0);

								break;

							}

							queue_info->get_data(queue_nr, section_header, sizeof(section_header), 1);

							r = (((section_header[1] & 0x0F) << 8) | section_header[2]) + 3;

							if (r <= 4096) {							// größer darf nicht

								gtxfeed->sec_len = r;

								if (r > payload_len)									// keine komplette Section
									r = payload_len;

								if (gtxfeed->check_crc)
									gtxfeed->sec_crc = queue_info->crc32(queue_nr, r, gtxfeed->sec_crc);

								if (gtxfeed->sec_len == r)
								{
									if (gtxfeed->sec_crc == 0) {
									
										avia_gt_napi_handle_section(gtxfeed,queue_nr,r);
										
									} else {

										queue_info->get_data(queue_nr, NULL, r, 0);
										
										dprintk("avia_gt_napi: section CRC invalid\n");
										
										gtxfeed->sec_len = 0;
										gtxfeed->sec_recv = 0;
										gtxfeed->sec_crc = 0;
										
									}
									
								} else {
								
									queue_info->get_data(queue_nr, gtxfeed->sec_buffer, r, 0);
									
									gtxfeed->sec_recv += r;
									
									if (gtxfeed->sec_len == gtxfeed->sec_recv)
										gtx_handle_section(gtxfeed);
										
								}

							}
							
						}
						
						payload_len -= r;
						
					}
					
				}

			break;
			
		} 
		
	}

}

static gtx_demux_filter_t *GtxDmxFilterAlloc(gtx_demux_feed_t *gtxfeed)
{

	gtx_demux_t *gtx = gtxfeed->demux;

	if (gtx->filter[gtxfeed->index].state != DMX_STATE_FREE)
		printk("ASSERTION FAILED: feed is not free but should be\n");

	gtx->filter[gtxfeed->index].state = DMX_STATE_ALLOCATED;

	return &gtx->filter[gtxfeed->index];

}

static gtx_demux_feed_t *GtxDmxFeedAlloc(gtx_demux_t *gtx, int type)
{

	s32 queue_nr = -EINVAL;

	switch (type) {

		case DMX_TS_PES_VIDEO:

			queue_nr = avia_gt_dmx_alloc_queue_video(NULL, avia_gt_napi_queue_callback, gtx);

		break;

		case DMX_TS_PES_AUDIO:

			queue_nr = avia_gt_dmx_alloc_queue_audio(NULL, avia_gt_napi_queue_callback, gtx);

		break;

		case DMX_TS_PES_TELETEXT:

			queue_nr = avia_gt_dmx_alloc_queue_teletext(NULL, avia_gt_napi_queue_callback, gtx);

		break;

		case DMX_TS_PES_PCR:
		case DMX_TS_PES_SUBTITLE:

			return NULL;

		break;

		case DMX_TS_PES_OTHER:

			queue_nr = avia_gt_dmx_alloc_queue_user(NULL, avia_gt_napi_queue_callback, gtx);

		break;

	}

	if (queue_nr < 0) {

		printk("avia_gt_napi: failed to allocate queue (error=%d)\n", queue_nr);

		return NULL;

	}

	if (gtx->feed[queue_nr].state != DMX_STATE_FREE)
		return NULL;

	gtx->feed[queue_nr].state = DMX_STATE_ALLOCATED;

	dprintk(KERN_DEBUG "gtx-dmx: using queue %d for %d\n", queue_nr, type);

	return &gtx->feed[queue_nr];

}

static int dmx_open(struct dmx_demux_s *demux)
{

	gtx_demux_t *gtx = (gtx_demux_t *)demux;

	gtx->users++;
	
	return 0;
	
}

static int dmx_close (struct dmx_demux_s* demux)
{
	gtx_demux_t *gtx=(gtx_demux_t*)demux;
	if (!gtx->users)
		return -ENODEV;
	gtx->users--;
	dprintk(KERN_DEBUG "gtx_dmx: close.\n");
	if (!gtx->users)
	{
		int i;
		// clear resources
		//gtx_tasklet.data=0;
		for (i=0; i<32; i++)
			if (gtx->feed[i].state!=DMX_STATE_FREE)
				dprintk(KERN_ERR "gtx-dmx.o: LEAK: queue %d used but it shouldn't.\n", i);
	}
	return 0;
}

static int dmx_write (struct dmx_demux_s* demux, const char* buf, size_t count)
{

	dprintk(KERN_ERR "gtx-dmx: dmx_write not yet implemented!\n");

	return 0;
	
}

static void dmx_set_filter(gtx_demux_filter_t *filter)
{

	if (filter->invalid)
		avia_gt_dmx_set_pid_table(filter->index, filter->wait_pusi, filter->invalid, filter->pid);

	if ( filter->output != GTX_OUTPUT_8BYTE)
		avia_gt_dmx_set_pid_control_table(filter->index, filter->output, filter->queue, filter->fork, filter->cw_offset, filter->cc, filter->start_up, filter->pec, 0, 0);
	else
		avia_gt_dmx_set_pid_control_table(filter->index, filter->output, filter->queue, filter->fork, filter->cw_offset, filter->cc, filter->start_up, filter->pec, filter->index, 1);

	if (!filter->invalid)
		avia_gt_dmx_set_pid_table(filter->index, filter->wait_pusi, filter->invalid, filter->pid);
		
}

static int dmx_ts_feed_set(struct dmx_ts_feed_s* feed, __u16 pid, size_t callback_length, size_t circular_buffer_size, int descramble, struct timespec timeout)
{

	gtx_demux_feed_t *gtxfeed=(gtx_demux_feed_t*)feed;
	gtx_demux_filter_t *filter=gtxfeed->filter;

	if (pid > 0x1FFF)
		return -EINVAL;

	gtxfeed->pid = pid;

	filter->pid = pid;
	filter->wait_pusi = 0;	// right?

#ifndef AVIA_SPTS
	if ((gtxfeed->output & TS_DECODER) && (gtxfeed->pes_type != DMX_TS_PES_TELETEXT)) {

		gtxfeed->output |= TS_PAYLOAD_ONLY;	 // weil: wir haben dual-pes
		gtxfeed->type = DMX_TYPE_PES;
		
	}
#endif

	if (gtxfeed->output & TS_PAYLOAD_ONLY)
		filter->output = GTX_OUTPUT_PESPAYLOAD;
	else
		filter->output = GTX_OUTPUT_TS;

	filter->queue = gtxfeed->index;

	filter->invalid = 1;
	filter->fork = 0;
	filter->cw_offset = 0;
	filter->cc = 0;
	filter->start_up = 0;
	filter->pec = 0;

	dmx_set_filter(gtxfeed->filter);

	gtxfeed->state=DMX_STATE_READY;

	return 0;
}

static void dmx_update_pid(gtx_demux_t *gtx, int pid)
{

	u8 i = 0;
	u8 used = 0;

	for (i = 0; i < LAST_USER_QUEUE; i++) {

		if ((gtx->feed[i].state == DMX_STATE_GO) && (gtx->feed[i].pid == pid) && (gtx->feed[i].output & TS_PACKET)) {

			used++;

			break;

		}

	}

	for (i = 0; i < LAST_USER_QUEUE; i++) {

		if ((gtx->feed[i].state == DMX_STATE_GO) && (gtx->feed[i].pid == pid)) {

			if ( used )
			{
				if (gtx->feed[i].irq_is_active == 0)
				{
					if ( (gtx->feed[i].type != DMX_TYPE_HW_SEC) &&
						 (gtx->feed[i].type != DMX_TYPE_SEC) )
					{
						printk(KERN_INFO "enabling irq mode 2 for pid 0x%04X\n",gtx->feed[i].pid);
						avia_gt_dmx_set_queue_irq(gtx->feed[i].index,1,-1);
					}
					else
					{
						avia_gt_dmx_set_queue_irq(gtx->feed[i].index,0,-1);
					}
#ifdef AVIA_SPTS
					if (i == 1)
					{
						avia_gt_dmx_queue_irq_enable(gtx->feed[0].index);
					}
					else
					{
#endif
						avia_gt_dmx_queue_irq_enable(gtx->feed[i].index);
#ifdef AVIA_SPTS
					}
#endif
					gtx->feed[i].irq_is_active = 1;
				}
			}
			else
			{
				avia_gt_dmx_queue_irq_disable(gtx->feed[i].index);
				gtx->feed[i].irq_is_active = 0;
			}
		}
	}
}

static int dmx_ts_feed_start_filtering(struct dmx_ts_feed_s* feed)
{
	gtx_demux_feed_t *gtxfeed=(gtx_demux_feed_t*)feed;
	gtx_demux_filter_t *filter=gtxfeed->filter;

	if (gtxfeed->state!=DMX_STATE_READY)
	{
		dprintk("gtx_dmx: feed not DMX_STATE_READY\n");
		return -EINVAL;
	}

	avia_gt_dmx_queue_reset(gtxfeed->index);

	filter->start_up=1;
	filter->invalid=0;
	dmx_set_filter(gtxfeed->filter);
	feed->is_filtering=1;

	gtxfeed->state=DMX_STATE_GO;

	dmx_update_pid(gtxfeed->demux, gtxfeed->pid);

//	udelay(100);
//	enx_tdp_trace();

	return 0;
}

static int dmx_ts_feed_set_type(struct dmx_ts_feed_s* feed, int type, dmx_ts_pes_t pes_type)
{
//	gtx_demux_feed_t *gtxfeed=(gtx_demux_feed_t*)feed;
	dprintk(KERN_DEBUG "gtx_dmx: dmx_ts_feed_set_type(%d, %d)\n", type, pes_type);
	return 0;
}

static int dmx_ts_feed_stop_filtering(struct dmx_ts_feed_s* feed)
{

	gtx_demux_feed_t *gtxfeed = (gtx_demux_feed_t *)feed;
	gtx_demux_filter_t *filter = gtxfeed->filter;

	filter->invalid = 1;

	dmx_set_filter(gtxfeed->filter);

	feed->is_filtering = 0;

	gtxfeed->state = DMX_STATE_READY;

	dmx_update_pid(gtxfeed->demux, gtxfeed->pid);

	avia_gt_dmx_queue_irq_disable(gtxfeed->index);
	gtxfeed->irq_is_active = 0;

	avia_gt_dmx_queue_reset(gtxfeed->index);

	return 0;

}

static int dmx_allocate_ts_feed (struct dmx_demux_s* demux, dmx_ts_feed_t** feed, dmx_ts_cb callback, int type, dmx_ts_pes_t pes_type)
{

	gtx_demux_t *gtx = (gtx_demux_t *)demux;
	gtx_demux_feed_t *gtxfeed = (gtx_demux_feed_t *)NULL;

	if (!(gtxfeed = GtxDmxFeedAlloc(gtx, pes_type))) {

		dprintk(KERN_ERR "gtx_dmx: couldn't get gtx feed\n");

		return -EBUSY;

	}

	if (type & TS_PAYLOAD_ONLY)
		gtxfeed->type = DMX_TYPE_PES;
	else
		gtxfeed->type = DMX_TYPE_TS;

	gtxfeed->cb.ts = callback;
	gtxfeed->demux = gtx;
	gtxfeed->pid = 0xFFFF;
	gtxfeed->sec_ccn = 16;
	gtxfeed->sec_len = 0;
	gtxfeed->pes_type = pes_type;
	gtxfeed->output = type;
	gtxfeed->irq_is_active = 0;

	*feed = &gtxfeed->feed.ts;

	(*feed)->is_filtering = 0;
	(*feed)->parent = demux;
	(*feed)->priv = 0;
	(*feed)->set = dmx_ts_feed_set;
	(*feed)->start_filtering = dmx_ts_feed_start_filtering;
	(*feed)->stop_filtering = dmx_ts_feed_stop_filtering;
	(*feed)->set_type = dmx_ts_feed_set_type;

	if (!(gtxfeed->filter = GtxDmxFilterAlloc(gtxfeed))) {

		dprintk(KERN_ERR "gtx_dmx: couldn't get gtx filter\n");

		gtxfeed->state = DMX_STATE_FREE;

		return -EBUSY;

	}

	gtxfeed->filter->feed = gtxfeed;
	gtxfeed->filter->state = DMX_STATE_READY;

	return 0;
}

static int dmx_release_ts_feed (struct dmx_demux_s* demux, dmx_ts_feed_t* feed)
{
	gtx_demux_feed_t *gtxfeed=(gtx_demux_feed_t*)feed;
	if (gtxfeed->state==DMX_STATE_FREE)
		return -EINVAL;
	// buffer.. ne, eher nicht.
	avia_gt_dmx_free_queue(gtxfeed->index);
	gtxfeed->state=DMX_STATE_FREE;
	gtxfeed->filter->state=DMX_STATE_FREE;
	// pid austragen
	gtxfeed->pid=0xFFFF;
	return 0;
}

static int dmx_allocate_pes_feed (struct dmx_demux_s* demux, dmx_pes_feed_t** feed, dmx_pes_cb callback)
{
	return -EINVAL;
}

static int dmx_release_pes_feed (struct dmx_demux_s* demux, dmx_pes_feed_t* feed)
{
	return -EINVAL;
}

static int dmx_section_feed_allocate_filter (struct dmx_section_feed_s* feed, dmx_section_filter_t** filter)
{
	gtx_demux_feed_t *gtxfeed=(gtx_demux_feed_t*)feed;
	gtx_demux_t *gtx=gtxfeed->demux;
//	gtx_demux_filter_t *gtxfilter=gtxfeed->filter;
	gtx_demux_secfilter_t *gtxsecfilter=0;
	int i = (int)0;

	dprintk("gtx_dmx: dmx_section_feed_allocate_filter.\n");

	if (gtxfeed->filter->no_of_filters >= 32)
		return -ENOSPC;

	for (i=0; i<32; i++)
		if (gtx->secfilter[i].state==DMX_STATE_FREE)
		{
			gtxsecfilter=gtx->secfilter+i;
			break;
		}

	if (!gtxsecfilter)
		return -ENOSPC;

	*filter=&gtxsecfilter->filter;
	(*filter)->parent=feed;
	(*filter)->priv=0;
	gtxsecfilter->feed=gtxfeed;
	gtxsecfilter->state=DMX_STATE_READY;

	gtxsecfilter->next=gtxfeed->secfilter;
	mb();
	gtxfeed->secfilter=gtxsecfilter;
	gtxfeed->filter->no_of_filters++;
	return 0;
}

static int dmx_section_feed_release_filter(dmx_section_feed_t *feed, dmx_section_filter_t* filter)
{
	gtx_demux_feed_t *gtxfeed=(gtx_demux_feed_t*)feed;
	gtx_demux_secfilter_t *f, *gtxfilter=(gtx_demux_secfilter_t*)filter;

	dprintk("gtx_dmx: dmx_section_feed_release_filter.\n");
	if (gtxfilter->feed!=gtxfeed)
	{
		dprintk("FAILED (gtxfilter->feed!=gtxfeed) (%p != %p)\n", gtxfilter->feed, gtxfeed);
		return -EINVAL;
	}
	if (feed->is_filtering)
	{
		dprintk("FAILED (feed->is_filtering)\n");
		return -EBUSY;
	}

	f=gtxfeed->secfilter;
	if (f==gtxfilter)
		gtxfeed->secfilter=gtxfilter->next;
	else
	{
		while (f->next!=gtxfilter)
			f=f->next;
		f->next=f->next->next;
	}
	gtxfilter->state=DMX_STATE_FREE;
	gtxfeed->filter->no_of_filters--;
	return 0;
}

static int dmx_section_feed_set(struct dmx_section_feed_s* feed,
										 __u16 pid, size_t circular_buffer_size,
										 int descramble, int check_crc)
{
	gtx_demux_feed_t *gtxfeed=(gtx_demux_feed_t*)feed;
	gtx_demux_filter_t *filter=gtxfeed->filter;

	if (pid>0x1FFF)
		return -EINVAL;

	gtxfeed->pid=pid;
	gtxfeed->check_crc=check_crc;

	filter->pid=pid;
	filter->queue=gtxfeed->index;

	filter->invalid=1;
	filter->fork=0;
	filter->cw_offset=0;
	filter->cc=0;
	filter->start_up=0;

	if (gtxfeed->demux->hw_sec_filt_enabled) {
	
		filter->output=GTX_OUTPUT_8BYTE;
		filter->pec=1;
		filter->wait_pusi=1;

	} else {

		filter->output=GTX_OUTPUT_TS;
		filter->pec=0;
		filter->wait_pusi=0;

	}

	dmx_set_filter(gtxfeed->filter);

	return 0;
	
}

static int dmx_section_feed_start_filtering(dmx_section_feed_t *feed)
{

	gtx_demux_feed_t *gtxfeed=(gtx_demux_feed_t*)feed;
	gtx_demux_filter_t *filter=gtxfeed->filter;
	int rc = (int)0;

	if (filter->output == GTX_OUTPUT_8BYTE)
	{
		rc = avia_gt_dmx_set_section_filter(gtxfeed->demux,gtxfeed->index,filter->no_of_filters,gtxfeed->secfilter);
		if (rc < 0)
		{
			return -ENOSPC;
		}
		avia_gt_dmx_queue_reset(gtxfeed->index);
		dprintk("gtx_dmx: section filtering start (%d filter)\n", filter->no_of_filters);
	}

	dmx_ts_feed_start_filtering((dmx_ts_feed_t*)feed);

	return 0;
	
}

static int dmx_section_feed_stop_filtering(struct dmx_section_feed_s* feed)
{
	dprintk("gtx_dmx: dmx_section_feed_stop_filtering.\n");
	dmx_ts_feed_stop_filtering((dmx_ts_feed_t*)feed);
	return 0;
}

static int dmx_allocate_section_feed (struct dmx_demux_s* demux, dmx_section_feed_t** feed, dmx_section_cb callback)
{
	gtx_demux_t *gtx=(gtx_demux_t*)demux;
	gtx_demux_feed_t *gtxfeed = (gtx_demux_feed_t *)NULL;

	dprintk("gtx_dmx: dmx_allocate_section_feed.\n");

	if (!(gtxfeed=GtxDmxFeedAlloc(gtx, DMX_TS_PES_OTHER)))
	{
		dprintk("gtx_dmx: couldn't get gtx feed (for section_feed)\n");
		return -EBUSY;
	}

	gtxfeed->cb.sec=callback;
	gtxfeed->demux=gtx;
	gtxfeed->pid=0xFFFF;
	gtxfeed->secfilter=0;

	*feed=&gtxfeed->feed.sec;
	(*feed)->is_filtering=0;
	(*feed)->parent=demux;
	(*feed)->priv=0;
	(*feed)->set=dmx_section_feed_set;
	(*feed)->allocate_filter=dmx_section_feed_allocate_filter;
	(*feed)->release_filter=dmx_section_feed_release_filter;
	(*feed)->start_filtering=dmx_section_feed_start_filtering;
	(*feed)->stop_filtering=dmx_section_feed_stop_filtering;

	gtxfeed->pes_type=DMX_TS_PES_OTHER;
	gtxfeed->sec_buffer=kmalloc(4096, GFP_KERNEL);
	gtxfeed->sec_recv=0;
	gtxfeed->sec_len=0;
	gtxfeed->sec_ccn=16;

	if (gtx->hw_sec_filt_enabled) {
	
		gtxfeed->type=DMX_TYPE_HW_SEC;
		gtxfeed->output = TS_PACKET | TS_PAYLOAD_ONLY;
		
	} else {
	
		gtxfeed->type=DMX_TYPE_SEC;
		gtxfeed->output = TS_PACKET;

	}

	gtxfeed->state=DMX_STATE_READY;

	if (!(gtxfeed->filter=GtxDmxFilterAlloc(gtxfeed)))
	{
		dprintk("gtx_dmx: couldn't get gtx filter\n");
		gtxfeed->state=DMX_STATE_FREE;
		return -EBUSY;
	}

	dprintk("gtx_dmx: allocating section feed, filter %d.\n", gtxfeed->filter->index);

	gtxfeed->filter->feed=gtxfeed;
	gtxfeed->filter->state=DMX_STATE_READY;
	gtxfeed->filter->no_of_filters = 0;
	return 0;
}

static int dmx_release_section_feed (struct dmx_demux_s* demux,	dmx_section_feed_t* feed)
{
	gtx_demux_feed_t *gtxfeed=(gtx_demux_feed_t*)feed;
	gtx_demux_t *gtx=(gtx_demux_t*)demux;

	dprintk("gtx_dmx: dmx_release_section_feed.\n");

	if (gtxfeed->secfilter)
	{
		dprintk("gtx_dmx: BUSY.\n");
		return -EBUSY;
	}
	kfree(gtxfeed->sec_buffer);
	dmx_release_ts_feed (demux, (dmx_ts_feed_t*)feed);						// free corresponding queue

	if (gtx->hw_sec_filt_enabled)
		avia_gt_dmx_release_section_filter(demux,gtxfeed->index);

	return 0;
	
}

static int dmx_add_frontend (struct dmx_demux_s* demux, dmx_frontend_t* frontend)
{
	gtx_demux_t *gtx=(gtx_demux_t*)demux;
	struct list_head *pos = (struct list_head *)NULL, *head=&gtx->frontend_list;
	if (!(frontend->id && frontend->vendor && frontend->model))
		return -EINVAL;
	list_for_each(pos, head)
	{
		if (!strcmp(DMX_FE_ENTRY(pos)->id, frontend->id))
			return -EEXIST;
	}
	list_add(&(frontend->connectivity_list), head);
	return 0;
}

static int dmx_remove_frontend (struct dmx_demux_s* demux,	dmx_frontend_t* frontend)
{
	gtx_demux_t *gtx=(gtx_demux_t*)demux;
	struct list_head *pos = (struct list_head *)NULL, *head=&gtx->frontend_list;
	list_for_each(pos, head)
	{
		if (DMX_FE_ENTRY(pos)==frontend)
		{
			list_del(pos);
			return 0;
		}
	}
	return -ENODEV;
}

static struct list_head* dmx_get_frontends (struct dmx_demux_s* demux)
{
	gtx_demux_t *gtx=(gtx_demux_t*)demux;
	if (list_empty(&gtx->frontend_list))
		return 0;
	return &gtx->frontend_list;
}

static int dmx_connect_frontend (struct dmx_demux_s* demux, dmx_frontend_t* frontend)
{
	if (demux->frontend)
		return -EINVAL;
	demux->frontend=frontend;
	return -EINVAL;			 // was soll das denn? :)
}

static int dmx_disconnect_frontend (struct dmx_demux_s* demux)
{
	demux->frontend=0;
	return -EINVAL;
}

static void gtx_dmx_set_pcr_pid(int pid)
{

	avia_gt_dmx_set_pcr_pid((u16)pid);

}

int GtxDmxInit(gtx_demux_t *gtxdemux)
{
	dmx_demux_t *dmx=&gtxdemux->dmx;
	int i =(int)0;
	u8 nullmask[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	gtxdemux->users=0;

	gtxdemux->frontend_list.next=
		gtxdemux->frontend_list.prev=
			&gtxdemux->frontend_list;

	for (i=0; i<NUM_PID_FILTER; i++)			// disable all pid filters
		avia_gt_dmx_set_pid_table(i, 0, 1, 0);

	for (i=0; i<NUM_QUEUES; i++)
	{
		gtxdemux->feed[i].index=i;
		gtxdemux->feed[i].state=DMX_STATE_FREE;
	}

	for (i=0; i<32; i++)
	{
		gtxdemux->filter[i].index=i;
		gtxdemux->filter[i].state=DMX_STATE_FREE;
	}

	for (i=0; i<32; i++)
	{
		gtxdemux->secfilter[i].index=i;
		gtxdemux->secfilter[i].state=DMX_STATE_FREE;
	}

	for (i=0; i<32; i++)
		gtxdemux->filter_definition_table_entry_user[i] = -1;

	if ((gtxdemux->hw_sec_filt_enabled = avia_gt_dmx_get_hw_sec_filt_avail())) {
	
		printk(KERN_INFO "avia_gt_napi: hardware section filtering enabled.\n");

		avia_gt_dmx_set_filter_parameter_table(31,nullmask,nullmask,0,0);
		
	} else {
	
		printk(KERN_INFO "avia_gt_napi: hardware section filtering disabled.\n");
		
	}

	dmx->id = "demux0";
	dmx->vendor = "C-Cube";
	dmx->model = "AViA eNX/GTX";
	dmx->frontend = 0;
	dmx->reg_list.next = dmx->reg_list.prev = &dmx->reg_list;
	dmx->priv = (void *)gtxdemux;
	dmx->open = dmx_open;
	dmx->close = dmx_close;
	dmx->write = dmx_write;
	dmx->allocate_ts_feed = dmx_allocate_ts_feed;
	dmx->release_ts_feed = dmx_release_ts_feed;
	dmx->allocate_pes_feed = dmx_allocate_pes_feed;
	dmx->release_pes_feed = dmx_release_pes_feed;
	dmx->allocate_section_feed = dmx_allocate_section_feed;
	dmx->release_section_feed = dmx_release_section_feed;

	dmx->descramble_mac_address = 0;
	dmx->descramble_section_payload = 0;

	dmx->add_frontend = dmx_add_frontend;
	dmx->remove_frontend = dmx_remove_frontend;
	dmx->get_frontends = dmx_get_frontends;
	dmx->connect_frontend = dmx_connect_frontend;
	dmx->disconnect_frontend = dmx_disconnect_frontend;
	dmx->flush_pcr = avia_gt_dmx_force_discontinuity;
	dmx->set_pcr_pid = gtx_dmx_set_pcr_pid;

	if (dmx_register_demux(dmx) < 0)
		return -1;

	if (dmx->open(dmx) < 0)
		return -1;

	if (gtxdemux->hw_sec_filt_enabled) {
	
		avia_gt_dmx_alloc_queue_message(NULL, avia_gt_napi_message_callback, gtxdemux);
		avia_gt_dmx_queue_reset(MESSAGE_QUEUE);

		gtxdemux->feed[MESSAGE_QUEUE].type = DMX_TYPE_MESSAGE;
		gtxdemux->feed[MESSAGE_QUEUE].pid = 0x2000;
		gtxdemux->feed[MESSAGE_QUEUE].output = TS_PAYLOAD_ONLY;
		gtxdemux->feed[MESSAGE_QUEUE].state = DMX_STATE_GO;

		avia_gt_dmx_queue_irq_enable(MESSAGE_QUEUE);
		
	}

	return 0;

}

int GtxDmxCleanup(gtx_demux_t *gtxdemux)
{

	dmx_demux_t *dmx = &gtxdemux->dmx;

	if ((gtxdemux->feed[MESSAGE_QUEUE].type == DMX_TYPE_MESSAGE) && (gtxdemux->feed[MESSAGE_QUEUE].state == DMX_STATE_GO)) {
	
		avia_gt_dmx_queue_irq_disable(MESSAGE_QUEUE);
		gtxdemux->feed[MESSAGE_QUEUE].state = DMX_STATE_FREE;
		avia_gt_dmx_free_queue(MESSAGE_QUEUE);
		
	}

	if (dmx_unregister_demux(dmx) < 0)
		return -1;

	return 0;
	
}

int __init avia_gt_napi_init(void)
{

	printk("avia_gt_napi: $Id: avia_gt_napi.c,v 1.141.2.1 2002/11/17 01:59:13 obi Exp $\n");

	gt_info = avia_gt_get_info();

	if ((!gt_info) || ((!avia_gt_chip(ENX)) && (!avia_gt_chip(GTX)))) {

		printk("avia_gt_napi: Unsupported chip type\n");

		return -EIO;

    }

	GtxDmxInit(&gtx);
	register_demux(&gtx.dmx);

	return 0;

}

void __exit avia_gt_napi_exit(void)
{

	gtx_dmx_close();

}

#ifdef MODULE
module_init(avia_gt_napi_init);
module_exit(avia_gt_napi_exit);
#endif
