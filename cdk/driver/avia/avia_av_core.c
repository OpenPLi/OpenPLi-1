/*
 *   avia_av.c - AViA x00 driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Felix "tmbinc" Domke (tmbinc@gmx.net)
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
 *   $Log: avia_av_core.c,v $
 *   Revision 1.36.2.1  2002/11/17 01:59:13  obi
 *   "backport" of latest dvb api version 1 drivers from HEAD branch
 *
 *   Revision 1.38  2002/10/28 14:34:25  wjoost
 *   SPTS und AC3 / cleanup
 *
 *   Revision 1.37  2002/10/21 11:38:58  obi
 *   fp driver cleanup
 *
 *   Revision 1.36  2002/10/03 12:47:57  Jolt
 *   AViA AV cleanups
 *
 *   Revision 1.35  2002/10/01 20:22:59  Jolt
 *   Cleanups
 *
 *   Revision 1.34  2002/09/30 19:46:10  Jolt
 *   SPTS support
 *
 *   Revision 1.33  2002/09/29 16:47:03  Jolt
 *   AViA command handling fixes
 *
 *   Revision 1.32  2002/09/24 17:50:19  Jolt
 *   PCM sample rate hack
 *
 *   Revision 1.31  2002/08/22 13:39:33  Jolt
 *   - GCC warning fixes
 *   - screen flicker fixes
 *   Thanks a lot to Massa
 *
 *   Revision 1.30  2002/05/06 02:18:18  obi
 *   cleanup for new kernel
 *
 *   Revision 1.29  2002/05/03 06:03:51  obi
 *   removed compile warnings
 *   use tabs instead of space
 *
 *   Revision 1.28  2002/03/17 16:25:38  happydude
 *   allow digital recording from SPDIF output
 *
 *   Revision 1.27  2002/03/06 09:01:55  gillem
 *   - fix output
 *
 *   Revision 1.26  2002/03/02 19:23:47  TripleDES
 *   fixes
 *
 *   Revision 1.25  2001/12/20 16:56:29  gillem
 *   - add host to decoder interrupt
 *
 *   Revision 1.24  2001/12/20 15:31:34  derget
 *   New sample freq output rausgeschmissen
 *
 *   Revision 1.23  2001/12/19 22:02:34  gillem
 *   - work on standby mode
 *
 *   Revision 1.22  2001/12/19 21:26:05  gillem
 *   - work on timer stuff
 *   - remove some logs
 *
 *   Revision 1.21  2001/12/18 19:39:21  TheDOC
 *   Changed event-delay to 30, which works well.
 *
 *   Revision 1.20  2001/12/18 18:01:51  gillem
 *   - add events
 *   - add timer
 *   - rewrite avia command handling
 *   - todo: optimize
 *   - i hope it works
 *
 *   Revision 1.19  2001/12/01 06:37:06  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.18  2001/07/08 02:24:59  fnbrd
 *   Parameter firmware is now only the path for the ucode.
 *   The filename itself is now according to the HW avia600.ux or avia500.ux.
 *
 *   Revision 1.17  2001/05/26 20:39:33  tmbinc
 *   fixed annoying audio bug (thought this was already fixed?!)
 *
 *   Revision 1.16  2001/05/15 22:19:11  kwon
 *   use __le32_to_cpu() instead of endian_swap()
 *
 *   Revision 1.15  2001/04/27 20:02:37  fnbrd
 *   Debugausgabe der Modulversion.
 *
 *   Revision 1.14  2001/04/27 19:46:59  fnbrd
 *   Unterscheidung von enx/gtx.
 *
 *   Revision 1.13  2001/04/21 00:32:01  TripleDES
 *   final "new resolution" fix
 *   -user fifo fix
 *
 *   Revision 1.12  2001/04/20 22:55:56  TripleDES
 *
 *   fixed "new resolution" bug
 *   - setting up the user-data fifo (in a free mem-area)
 *
 *   Revision 1.11  2001/03/21 15:30:25  tmbinc
 *   Added SYNC-delay for avia, resulting in faster zap-time.
 *
 *   Revision 1.10  2001/03/08 20:01:41  gillem
 *   - add display modes + ioctl
 *
 *   Revision 1.9  2001/03/07 20:58:07  gillem
 *   - add bitstream info @ procfs
 *
 *   Revision 1.8  2001/02/25 16:12:53  gillem
 *   - fix "volume" for AVIA600L
 *
 *   Revision 1.7  2001/02/25 15:27:02  gillem
 *   - fix sound for AVIA600L
 *
 *   Revision 1.6  2001/02/24 11:09:39  gillem
 *   - change osd stuff
 *
 *   Revision 1.5  2001/02/17 19:50:14  gillem
 *   - bugfix ...
 *
 *   Revision 1.4  2001/02/17 19:45:21  gillem
 *   - some changes
 *
 *   Revision 1.3  2001/02/17 11:12:42  gillem
 *   - fix init
 *
 *   Revision 1.2  2001/02/16 20:48:29  gillem
 *   - some avia600 tests
 *
 *   Revision 1.1  2001/02/15 21:55:56  gillem
 *   - change module name to avia.o
 *   - add interrupt for commands
 *   - some rewrites
 *
 *   Revision 1.14  2001/02/15 00:11:20  gillem
 *   - rewrite stuff ... not ready (read source to understand new params)
 *
 *   Revision 1.13  2001/02/13 23:46:30  gillem
 *   -fix the interrupt problem (ppc i like you)
 *
 *   Revision 1.12  2001/02/03 16:39:17  tmbinc
 *   sound fixes
 *
 *   Revision 1.11  2001/02/03 14:48:16  gillem
 *   - more audio fixes :-/
 *
 *   Revision 1.10  2001/02/03 11:29:54  gillem
 *   - fix audio
 *
 *   Revision 1.9  2001/02/02 18:17:18  gillem
 *   - add exports (avia_wait,avia_command)
 *
 *   Revision 1.8  2001/01/31 17:17:46  tmbinc
 *   Cleaned up avia drivers. - tmb
 *
 *   $Revision: 1.36.2.1 $
 *
 */

#define __KERNEL_SYSCALLS__

#include <linux/string.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/8xx_immap.h>
#include <asm/pgtable.h>
#include <asm/mpc8xx.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>

#include <dbox/fp.h>
#include <dbox/avia_av.h>
#include <dbox/avia_av_event.h>
#include <dbox/avia_av_proc.h>
#include <dbox/info.h>

/* ---------------------------------------------------------------------- */

#ifdef MODULE
static int   pal = 1;
static char *firmware = NULL;
#endif

static int debug = 0;
#define dprintk if (debug) printk

#ifdef MODULE

void avia_set_pcr   (u32 hi, u32 lo);
void avia_flush_pcr (void);

volatile u8 *aviamem;
int aviarev;
int silirev;

static int dev;

/* interrupt stuff */
#define AVIA_INTERRUPT		  SIU_IRQ4

static spinlock_t avia_lock;
static spinlock_t avia_register_lock;
static wait_queue_head_t avia_cmd_wait;
static wait_queue_head_t avia_cmd_state_wait;
static u16 sample_rate = 44100;

#if 0
static u16 pid_audio = 0xFFFF;
static u16 pid_video = 0xFFFF;
static u8 stream_type_audio = AVIA_AV_STREAM_TYPE_SPTS;
static u8 stream_type_video = AVIA_AV_STREAM_TYPE_SPTS;
#endif

/* finally i got them */
#define UX_MAGIC			0x00
#define UX_NUM_SECTIONS			0x01
#define UX_LENGTH_FILE			0x02
#define UX_FIRST_SECTION_START		0x03
#define UX_SECTION_LENGTH_OFFSET	0x01
#define UX_SECTION_WRITE_ADDR_OFFSET	0x02
#define UX_SECTION_CHECKSUM_OFFSET	0x03
#define UX_SECTION_DATA_OFFSET		0x04
#define UX_SECTION_DATA_GBUS_TABLE	0x02FF
#define UX_SECTION_DATA_IMEM		0x0200
#define UX_SECTION_HEADER_LENGTH	0x04

#define NTSC_16MB_WO_ROM_SRAM		7
#define NTSC_16MB_PL_ROM_SRAM		9
#define PAL_16MB_WO_ROM_SRAM		10
#define PAL_20MB_WO_ROM_SRAM		12

/* ---------------------------------------------------------------------- */

static int avia_delay_sync = -1;

static void avia_htd_interrupt(void);

static int avia_standby( int state );

/* ---------------------------------------------------------------------- */
u32
avia_rd (int mode, int address)
{

	u32 result;

	spin_lock_irq(&avia_register_lock);

	address   &= 0x3FFFFF;
	
	aviamem[6] = ((address >> 16) | mode) & 0xFF;
	aviamem[5] = (address >>  8) & 0xFF;
	aviamem[4] = address & 0xFF;
	
	mb();
	
	result  = aviamem[3] << 24;
	result |= aviamem[2] << 16;
	result |= aviamem[1] << 8;
	result |= aviamem[0];

	spin_unlock_irq(&avia_register_lock);

	return result;
	
}

/* ---------------------------------------------------------------------- */
void
avia_wr (int mode, u32 address, u32 data)
{

	spin_lock_irq(&avia_register_lock);

	address &= 0x3FFFFF;
	
	aviamem[6] = ((address >> 16) | mode) & 0xFF;
	aviamem[5] =  (address >>  8) & 0xFF;
	aviamem[4] = address & 0xFF;
	aviamem[3] = (data >> 24) & 0xFF;
	aviamem[2] = (data >> 16) & 0xFF;
	aviamem[1] = (data >>  8) & 0xFF;
	aviamem[0] = data & 0xFF;

	spin_unlock_irq(&avia_register_lock);
	
}

/* ---------------------------------------------------------------------- */
inline void
wIM (u32 addr, u32 data)
{

	wGB (0x36, addr);
	wGB (0x34, data);
	
}

/* ---------------------------------------------------------------------- */
inline u32
rIM (u32 addr)
{

	wGB(0x3A, 0x0B);
	wGB(0x3B, addr);
	wGB(0x3A, 0x0E);

	return rGB(0x3B);
	
}

/* ---------------------------------------------------------------------- */
static void
InitialGBus (u32 *microcode)
{
	unsigned long *ptr = ((unsigned long*) microcode) + 0x306;
	int words = *ptr--, data, addr;

	dprintk (KERN_DEBUG "%s: %s: Performing %d initial G-bus Writes. "
		 "(don't panic! ;)\n", __FILE__, __FUNCTION__, words);

	while (words--) {
		addr = *ptr--;
		data = *ptr--;
		wGB (addr, data);
	}
}

/* ---------------------------------------------------------------------- */
static void
FinalGBus (u32 *microcode)
{
	unsigned long *ptr = ((unsigned long*) microcode) + 0x306;
	int words = *ptr--, data, addr;

	*ptr -= words;
	 ptr -= words * 4;
	words = *ptr--;

	dprintk (KERN_DEBUG "%s: %s: Performing %d final G-bus Writes.\n",
		 __FILE__, __FUNCTION__, words);

	while (words--) {
		addr = *ptr--;
		data = *ptr--;
		wGB (addr, data);
	}
}

/* ---------------------------------------------------------------------- */
static void
dram_memcpyw (u32 dst, u32 *src, int words)
{
	while (words--) {
		wDR (dst, *src++);
		dst += 4;
	}
}

/* ---------------------------------------------------------------------- */
static void
load_dram_image (u32 *microcode, u32 section_start)
{
	u32 dst, *src, words, errors=0;

	words = __le32_to_cpu (microcode[section_start +
					 UX_SECTION_LENGTH_OFFSET]) / 4;
	dst   = __le32_to_cpu (microcode[section_start +
					 UX_SECTION_WRITE_ADDR_OFFSET]);
	src   = microcode + section_start + UX_SECTION_DATA_OFFSET;

	dprintk ("%s: %s: Microcode at: %.8x (%.8x)\n",
		 __FILE__, __FUNCTION__, (u32) dst, (u32) words * 4);

	dram_memcpyw (dst, src, words);

	while (words--) {
		if (rDR(dst) != *src++)
			errors++;
		dst += 4;
	}

	if (errors)
		printk (KERN_ERR "%s: %s: Microcode verify: %d errors.\n",
			__FILE__, __FUNCTION__, errors);
}

/* ---------------------------------------------------------------------- */
static void
load_imem_image (u32 *microcode, u32 data_start)
{
	u32 *src, i, words, errors = 0;

	src = microcode + data_start + UX_SECTION_DATA_IMEM;
	words = 256;

	for (i = 0; i < words; i++)
		wIM (i, src[i]);

	wGB (0x3A, 0xB);	       // CPU_RMADR2
	wGB (0x3B,   0);	       // set starting address
	wGB (0x3A, 0xE);	       // indirect regs

	for (i = 0; i < words; i++)
		if (rGB (0x3B) != src[i])
			errors++;

	if (errors)
		printk (KERN_ERR "%s: %s: Imem verify: %d errors\n",
			__FILE__, __FUNCTION__, errors);
}

/* ---------------------------------------------------------------------- */
static void
LoaduCode (u32 *microcode)
{
	int num_sections = __le32_to_cpu (microcode[1]);
	int data_offset  = 3;

	while (num_sections--) {
		load_dram_image (microcode, data_offset);
		data_offset += (__le32_to_cpu (
			 microcode[data_offset+UX_SECTION_LENGTH_OFFSET]) / 4)
			 + UX_SECTION_HEADER_LENGTH;
	}
}

/* ---------------------------------------------------------------------- */

static void
avia_htd_interrupt()
{
	wGB(0,rGB(0)|(1<<7));
	wGB(0,rGB(0)|(1<<6));
}

/* ---------------------------------------------------------------------- */
void
avia_interrupt (int irq, void *vdev, struct pt_regs *regs)
{
	u32 status	= (u32)0;
	u32 sem			= (u32)0;

	spin_lock(&avia_lock);

	status = rDR(0x2AC);

	/* usr data */
	if (status & (1 << 12)) {
//		printk (KERN_INFO "AVIA: User data :-)\n");
	}

	if (status & (1 << 5))
		if (avia_delay_sync && !--avia_delay_sync) {
			dprintk ("%s: %s: CHCH [DECODE] enabling SYNC.\n",
				 __FILE__, __FUNCTION__);
			wDR (AV_SYNC_MODE, 6);
		}

	/* avia cmd stuff */
	if (status & (1 << 15) || status & (1 << 9)) {
	
		dprintk (KERN_INFO "%s: %s: CMD INTR\n", __FILE__,
			 __FUNCTION__);

		wake_up_interruptible (&avia_cmd_wait);
		
	}

	/* INIT INTR */
	if (status & (1 << 23)) {
//		dprintk ("INIT RECV!\n");
	}

	/* AUD INTR */
	if (status & (1 << 22)) {
		sem = rDR (0x460);

		dprintk (KERN_DEBUG "%s: %s: AUD INTR %.8x\n",
			 __FILE__, __FUNCTION__, sem);

		// E0 AUDIO_CONFIG
		// E8 AUDIO_DAC_MODE
		// EC AUDIO_CLOCK_SELECTION
		// F0 I2C_958_DELAY

		/* new sample freq. */
		if (sem & 7) {
			switch (sem & 7) {
				// 44.1
				case 1: 
					wDR (0xEC, (rDR (0xEC) & ~(7 << 2)) | (1 << 2));
					sample_rate = 44100;
				break;
				// 48
				case 2: 
					wDR (0xEC, (rDR (0xEC) & ~(7 << 2)));
					sample_rate = 48000;
				break;
				// 32
				case 7:
					wDR (0xEC, (rDR (0xEC) & ~(7 << 2)) | (2 << 2));
					sample_rate = 32000;
				break;
			}

			//dprintk (KERN_INFO "%s: %s: New sample freq: %d.\n",
			//	__FILE__, __FUNCTION__, sem & 7);
		}

		/* reserved */
		if (sem & (7 << 3))
			printk (KERN_INFO "%s: %s: Reserved %02X.\n",
				__FILE__, __FUNCTION__, (sem >> 3) & 7);

		/* new audio emphasis */
		if (sem & (3 << 6))
			switch ((sem >> 6) & 3) {
			case 1: dprintk (KERN_INFO "%s: %s: "
					"New audio emphasis is off.\n",
					__FILE__, __FUNCTION__); break;
			case 2: dprintk (KERN_INFO "%s: %s: "
					"New audio emphasis is on.\n",
					__FILE__, __FUNCTION__); break;
			}
		if (sem&0xFF)
			wDR (0x468, 1);
	}

	/* buffer full */
	if ( status&(1<<16) ) {
//	      dprintk("BUF-F INTR\n");

		if ( rDR(0x2b4)&2 ) {
//		      dprintk("BUF-F VIDEO\n");
		}

		if ( rDR(0x2b4)&2 ) {
//		      dprintk("BUF-F AUDIO\n");
		}

	}

	/* buffer und. */
	if (status & (1 << 8)) {
//		dprintk ("UND INTR\n");

		if (rDR (0x2b8) & 1) {
//			dprintk ("UND VIDEO\n");
		}

		if (rDR (0x2b8) & 2) {
//			dprintk ("UND AUDIO\n");
		}
	}


	/* bitstream error */
	if (status & 1) {
//		dprintk ("ERR INTR\n");

		if (rDR (0x2c4) & (1 << 1)) {
//			dprintk ("ERR SYSTEM BITSTREAM CURR: %d\n",rDR(0x318));
		}

		if (rDR (0x2c4) & (1 << 2)) {
//			dprintk (KERN_DEBUG "AVIA: ERR AUDIO BITSTREAM CURR: %d\n",rDR(0x320));
		}

		if (rDR (0x2c4) & (1 << 3)) {
//			dprintk ("ERR VIDEO BITSTREAM CURR: %d\n",rDR(0x31C));
		}
	}

	/* intr. ack */
	wGB(0, ((rGB (0) & (~1)) | 2));

	/* clear flags */
	wDR (0x2B4, 0);
	wDR (0x2B8, 0);
	wDR (0x2C4, 0);
	wDR (0x2AC, 0);

	spin_unlock(&avia_lock);

	return;
	
}

u32 avia_cmd_status_get(u32 status_addr, u8 wait_for_completion)
{

	if (!status_addr)
		return 0;

	dprintk("SA: 0x%X -> run\n", status_addr);

	if (wait_for_completion) {
	
		if (wait_event_interruptible(avia_cmd_wait, (rDR(status_addr) >= 0x03))) {

			printk(KERN_ERR "avia_av: error while fetching command status\n");

			return 0;
		
		}
		
	}

	dprintk("SA: 0x%X -> end -> S: 0x%X\n", status_addr, rDR(status_addr));
	
	if (rDR(status_addr) == 0x05)
		printk("avia_av: warning - command @ 0x%X failed\n", status_addr);

	return rDR(status_addr);

}

static u32 avia_cmd_status_get_addr(void)
{

	while (!rDR(0x5C))
		schedule();

	return rDR(0x5C);
	
}

u32 avia_command(u32 command, ...)
{

	u32	i;
	va_list ap;
	u32 status_addr;
	
	if (!avia_cmd_status_get_addr()) {
	
		printk(KERN_ERR "avia_av: timeout.\n");
		
		return 0;
		
	}

	va_start(ap, command);

	spin_lock_irq(&avia_lock);

	wDR(0x40, command);

	for (i = 0; i < ((command & 0x7F00) >> 8); i++)
		wDR(0x44 + i * 4, va_arg(ap, int));

	for (; i < 8; i++)
		wDR(0x44 + i * 4, 0);

	// RUN
	wDR(0x5C, 0);

	// host-to-decoder interrupt
	avia_htd_interrupt();

	spin_unlock_irq(&avia_lock);

	va_end(ap);
	
	if (!(status_addr = avia_cmd_status_get_addr())) {
	
		printk(KERN_ERR "avia_av: timeout.\n");
		
		return 0;
		
	}

	dprintk("C: 0x%X -> SA: 0x%X\n", command, status_addr);

	if (command & 0x8000)
		avia_cmd_status_get(status_addr, 1);

	return status_addr;

}

void avia_set_pcr(u32 hi, u32 lo)
{
	u32 data1 = (hi>>16)&0xFFFF;
	u32 data2 = hi&0xFFFF;
	u32 timer_high = ((1<<21))|((data1 & 0xE000L) << 4)
				| (( 1 << 16)) | ((data1 & 0x1FFFL) << 3)
				| ((data2 & 0xC000L) >> 13) | ((1L));

	u32 timer_low = ((data2 & 0x03FFFL) << 2)
				| ((lo & 0x8000L) >> 14) | (( 1L ));

	dprintk(KERN_INFO "AVIA: Setting PCR: %08x:%08x\n", hi, lo);

	wGB(0x02, timer_high);
	wGB(0x03, timer_low);
	dprintk("CHCH [DECODE] delaying sync in 1s\n");
	avia_delay_sync=50;
}

void avia_flush_pcr(void)
{

	dprintk("CHCH [DECODE] disabling sync\n");
	
	wDR(AV_SYNC_MODE, 0);			   // no sync
	
}

/* ---------------------------------------------------------------------- */

/*
static int wait_audio_sequence(void)
{
	while(rDR(AUDIO_SEQUENCE_ID));
	return 0;
}

static int init_audio_sequence(void)
{
	wDR(AUDIO_SEQUENCE_ID, 0);
	wDR(NEW_AUDIO_SEQUENCE, 1);
	return wait_audio_sequence();
}

static int new_audio_sequence( u32 val )
{
	wDR(AUDIO_SEQUENCE_ID, val);
	wDR(NEW_AUDIO_SEQUENCE, 2);
	return wait_audio_sequence();
}
*/
/* ---------------------------------------------------------------------- */

static void avia_audio_init(void)
{

	u32 val;

	/* AUDIO_CONFIG
	 *
	 * 12,11,7,6,5,4 reserved or must be set to 0
	 */
	val  = 0;
	val |= (0<<10);	// 1: 64 0: 32/48
	val |= (0<<9);	// 1: I2S 0: other
	val |= (0<<8);	// 1: no clock on da-iec
	val |= (1<<3);	// 0: normal 1:I2S output
	val |= (1<<2);	// 0:off 1:on channels
	val |= (1<<1);	// 0:off 1:on IEC-958
	val |= (1);	// 0:encoded 1:decoded output
	wDR(AUDIO_CONFIG, val);

	/* AUDIO_DAC_MODE
	 * 0 reserved
	 */
	val  = 0;
	val |= (0<<8);	//
	val |= (3<<6);	//
	val |= (0<<4);	//
	val |= (0<<3);	// 0:high 1:low DA-LRCK polarity
	val |= (1<<2);	// 0:0 as MSB in 24 bit mode 1: sign ext. in 24bit
	val |= (0<<1);	// 0:msb 1:lsb first
	wDR(AUDIO_DAC_MODE, val);

	/* AUDIO_CLOCK_SELECTION */
	val = 0;
	val |= (1<<2);

	/* 500/600 test */
	if ( (aviarev == 0x00) && (silirev == 0x80) )
	{
		val |= (0<<1);	// 1:256 0:384 x sampling frequ.
	}
	else
	{
		val |= (1<<1);	// 1:256 0:384 x sampling frequ.
	}

	val |= (1);	// master,slave mode
	wDR(AUDIO_CLOCK_SELECTION, val);

	/* AUDIO_ATTENUATION */
	wDR(AUDIO_ATTENUATION, 0);

	/* SET SCMS */
	wDR(IEC_958_CHANNEL_STATUS_BITS, rDR(IEC_958_CHANNEL_STATUS_BITS)&~4);
	
	sample_rate = 44100;
	
}

/* ---------------------------------------------------------------------- */

void avia_set_default(void)
{
	u32 val = (u32)0;
	struct dbox_info_struct *dinfo = (struct dbox_info_struct *)NULL;

	val  = 0;
	val |= (0<<2);  // 1: tristate
	val |= (0<<0);  // 0: slave 1: master HSYNC/VSYNC
	val |= (0<<0);  // 0: BT.601 1: BT.656

	wDR(VIDEO_MODE, val);

	/* 0: 4:3 1: 16:9 downs. 2: 16:9 */
	wDR(DISPLAY_ASPECT_RATIO, 0);

	/* 0: disable 1: PAN&SCAN 2: Letterbox */
	wDR(ASPECT_RATIO_MODE, 2);

	/* 2: 4:3 3: 16:9 4: 20:9 */
	wDR(FORCE_CODED_ASPECT_RATIO, 0);

	wDR(PAN_SCAN_SOURCE, 0);

	wDR(PAN_SCAN_HORIZONTAL_OFFSET, 0);

	val = 0x108080;
	wDR(BORDER_COLOR, val);
	wDR(BACKGROUND_COLOR, val);

	/* 0: I_FRAME 1: I_SLICE */
	wDR(I_SLICE, 0);

	/* 0: frame 1: slice based error recovery */
	wDR(ERR_CONCEALMENT_LEVEL, 0);


	/* PLAY MODE PARAMETER */


	/* 0: Demux interface 2: Host Interface */
	wDR(BITSTREAM_SOURCE, 0);

	/* */
	dbox_get_info_ptr(&dinfo);
	if(dinfo->enxID==-1) {
		wDR(TM_MODE, 0x0a); //gtx
//	      dprintk("AVIA: GTX\n");
	}
	else {
		wDR(TM_MODE, 0x18); // eNX
//	      dprintk("AVIA: eNX\n");
	}

	wDR(AV_SYNC_MODE, 0x06);

	wDR(VIDEO_PTS_DELAY, 0);
	wDR(VIDEO_PTS_SKIP_THRESHOLD, 0xE10);
	wDR(VIDEO_PTS_REPEAT_THRESHOLD, 0xE10);

	wDR(AUDIO_PTS_DELAY, 0xe00);
	wDR(AUDIO_PTS_SKIP_THRESHOLD_1, 0xE10 );
	wDR(AUDIO_PTS_REPEAT_THRESHOLD_1, 0xE10 );
	wDR(AUDIO_PTS_SKIP_THRESHOLD_2, 0x2300 );
	wDR(AUDIO_PTS_REPEAT_THRESHOLD_2, 0x2300 );

	/* */
	wDR(INTERPRET_USER_DATA,0);
	wDR(INTERPRET_USER_DATA_MASK,0);

#if 0
	//3des:Fifo dont need setup
	//3des:is done by memory_map
	wDR(USER_DATA_BUFFER_START,0x1f0000);
	wDR(USER_DATA_BUFFER_END,0x1f0400);
#endif

	/* osd */
	wDR(DISABLE_OSD, 0);

	/* disable osd */
	wDR(OSD_EVEN_FIELD, 0);
	wDR(OSD_ODD_FIELD, 0);

	wDR(0x64, 0);
	wDR(DRAM_INFO, 0);
	wDR(UCODE_MEMORY, 0);

	/* set pal or ntsc */
	if (pal)
	{
		wDR(MEMORY_MAP, PAL_16MB_WO_ROM_SRAM);
	}
	else
	{
		wDR(MEMORY_MAP, NTSC_16MB_WO_ROM_SRAM);
	}
}

/* ---------------------------------------------------------------------- */

static int ppc_set_siumcr(void)
{
	immap_t *immap = (immap_t *)NULL;
	sysconf8xx_t * sys_conf = (sysconf8xx_t *)NULL;

	immap = (immap_t *)IMAP_ADDR;

	if (!immap)
	{
		dprintk(KERN_ERR "AVIA: Get immap failed.\n");
		return -1;
	}

	sys_conf = (sysconf8xx_t *)&immap->im_siu_conf;

	if (!sys_conf)
	{
		dprintk(KERN_ERR "AVIA: Get sys_conf failed.\n");
		return -1;
	}

	if ( sys_conf->sc_siumcr & (3<<10) )
	{
		cli();
		sys_conf->sc_siumcr &= ~(3<<10);
		sti();
	}

	return 0;
}

/* ---------------------------------------------------------------------- */
/* shamelessly stolen from sound_firmware.c */
static int errno;

static int
do_firmread (const char *fn, char **fp)
{
	int    fd								= (int)0;
	loff_t l								= (loff_t)0;
	char  *dp								= (char *)NULL;
	char firmwarePath[100]	= { "\0" };

	if (!fn)
		return 0;

	strncpy(firmwarePath, fn, sizeof(firmwarePath)-sizeof("/avia600.ux")-1);
	firmwarePath[sizeof(firmwarePath)-sizeof("/avia600.ux")-2]=0;

	if (!aviarev)
		strcat(firmwarePath, "/avia600.ux");
	else
		strcat(firmwarePath, "/avia500.ux");

	if ((fd = open (firmwarePath, 0, 0)) < 0) {
		printk (KERN_ERR "%s: %s: Unable to load '%s'.\n",
			__FILE__, __FUNCTION__, firmwarePath);
		return 0;
	}

	l = lseek (fd, 0L, 2);

	if (l <= 0 || l >= 128 * 1024) {
		printk (KERN_ERR "%s: %s: Firmware wrong size '%s'.\n",
			__FILE__, __FUNCTION__, firmwarePath);
		sys_close (fd);
		return 0;
	}

	lseek (fd, 0L, 0);
	dp = vmalloc (l);

	if (dp == NULL) {
		printk (KERN_ERR "%s: %s: Out of memory loading '%s'.\n",
			__FILE__, __FUNCTION__, firmwarePath);
		sys_close (fd);
		return 0;
	}

	if (read (fd, dp, l) != l) {
		printk (KERN_ERR "%s: %s: Failed to read '%s'.\n",
			__FILE__, __FUNCTION__, firmwarePath);
		vfree (dp);
		sys_close (fd);
		return 0;
	}

	close (fd);
	*fp = dp;
	return (int) l;
}

/* ---------------------------------------------------------------------- */

static int init_avia(void)
{
	u32						*microcode	= (u32 *)NULL;
	u32						 val				= (u32)0;
	int						 tries			= (int)0;
	mm_segment_t	 fs;
	
	/* remap avia memory */
	if(!aviamem)
		aviamem=(unsigned char*)ioremap(0xA000000, 0x200);

	if (!aviamem)
	{
		printk(KERN_ERR "AVIA: Failed to remap memory.\n");
		return -ENOMEM;
	}

	(void)aviamem[0];

	// read revision
	aviarev=(rGB(0)>>16)&3;

	fs = get_fs();

	set_fs(get_ds());


	/* read firmware */
	if (do_firmread(firmware, (char**) &microcode) == 0)
	{
		set_fs(fs);
		iounmap((void*)aviamem);
		return -EIO;
	}

	set_fs(fs);

	/* set siumcr for interrupt */
	if ( ppc_set_siumcr() < 0 )
	{
		vfree(microcode);
		iounmap((void*)aviamem);
		return -EIO;
	}

	/* request avia interrupt */
	if (request_8xxirq(AVIA_INTERRUPT, avia_interrupt, 0, "avia", &dev) != 0)
	{
		printk(KERN_ERR "AVIA: Failed to get interrupt.\n");
		vfree(microcode);
		iounmap((void*)aviamem);
		return -EIO;
	}

	/* init queue */
	init_waitqueue_head(&avia_cmd_wait);

	init_waitqueue_head(&avia_cmd_state_wait);

	/* enable host access */
	wGB(0, 0x1000);
	/* cpu reset */
	wGB(0x39, 0xF00000);
	/* cpu reset by fp */
	fp_do_reset(0xBF & ~ (2));
	/* enable host access */
	wGB(0, 0x1000);
	/* cpu reset */
	wGB(0x39, 0xF00000);

	wGB(0, 0x1000);

//	aviarev=(rGB(0)>>16)&3;
	silirev=((rGB(0x22)>>8)&0xFF);

	dprintk(KERN_INFO "AVIA: AVIA REV: %02X SILICON REV: %02X\n",aviarev,silirev);

	/* AR SR CHIP FILE
	 * 00 80 600L 600...
	 * 03 00 600L 500... ???
	 * 03 00 500  500
	*/

	switch (aviarev)
	{
		case 0:
			dprintk(KERN_INFO "AVIA: AVIA 600 found. (no support yet)\n");
			break;
		case 1:
			dprintk(KERN_INFO "AVIA: AVIA 500 LB3 found. (no microcode)\n");
			break;
#if 0
		case 3:
			dprintk("AVIA 600L found. (no support yet)\n");
			break;
#endif
		default:
			dprintk(KERN_INFO "AVIA: AVIA 500 LB4 found. (nice)\n");
			break;
	}

	/* TODO: AVIA 600 INIT !!! */

	/* D.4.3 - Initialize the SDRAM DMA Controller */
	switch (aviarev)
	{
		case 0:
//	      case 3:
			wGB(0x22, 0x10000);
			wGB(0x23, 0x5FBE);
			wGB(0x22, 0x12);
			wGB(0x23, 0x3a1800);
			break;
		default:
			wGB(0x22, 0xF);
			val = rGB(0x23) | 0x14EC;
			wGB(0x22, 0xF);
			wGB(0x23, val);
			rGB(0x23);

			wGB(0x22, 0x11);
			val = (rGB(0x23) & 0x00FFFFFF) | 1;
			wGB(0x22, 0x11);
			wGB(0x23, val);
			rGB(0x23);
			break;
	}

	InitialGBus(microcode);

	LoaduCode(microcode);

	load_imem_image(microcode,UX_FIRST_SECTION_START+UX_SECTION_DATA_OFFSET);

	avia_set_default();

	avia_audio_init();

//      init_audio_sequence();

	FinalGBus(microcode);

	vfree(microcode);

	/* set cpu running mode */
	wGB(0x39, 0x900000);

	/* enable decoder/host interrupt */
	wGB(0, rGB(0)|(1<<7));
	wGB(0, rGB(0)&~(1<<6));
	wGB(0, rGB(0)|(1<<1));
	wGB(0, rGB(0)&~1);

	/* clear int. flags */
	wDR(0x2B4, 0);
	wDR(0x2B8, 0);
	wDR(0x2C4, 0);
	wDR(0x2AC, 0);

	/* enable interrupts */
	wDR(0x200, (1<<23)|(1<<22)|(1<<16)|(1<<12)|(1<<8)|(1<<5)|(1) );

	tries=20;

	while ((rDR(0x2A0)!=0x2))
	{
		if (!--tries)
			break;
		udelay(10*1000);
		schedule();
	}

	if (!tries)
	{
		dprintk(KERN_ERR "AVIA: Timeout waiting for decoder initcomplete. (%08X)\n",rDR(0x2A0));
		iounmap((void*)aviamem);
		free_irq(AVIA_INTERRUPT, &dev);
		return -EIO;
	}

	/* new audio config */
	wDR(0x468, 0xFFFF);

	tries=20;

	while (rDR(0x468))
	{
		if (!--tries)
			break;
		udelay(10*1000);
		schedule();
	}

	if (!tries)
	{
		dprintk(KERN_ERR "AVIA: New_audio_config timeout\n");
		iounmap((void*)aviamem);
		free_irq(AVIA_INTERRUPT, &dev);
		return -EIO;
	}

	avia_av_event_init();

	avia_command(Abort, 0);

//      wDR(OSD_BUFFER_START, 0x1f0000);
//      wDR(OSD_BUFFER_END,   0x200000);

	avia_command(Reset);

	dprintk(KERN_INFO "AVIA: Using avia firmware revision %c%c%c%c\n", rDR(0x330)>>24, rDR(0x330)>>16, rDR(0x330)>>8, rDR(0x330));
	dprintk(KERN_INFO "AVIA: %x %x %x %x %x\n", rDR(0x2C8), rDR(0x2CC), rDR(0x2B4), rDR(0x2B8), rDR(0x2C4));


	return 0;
}

/* ---------------------------------------------------------------------- */

int avia_standby( int state )
{
	if (state == 0)
	{
		if ( init_avia() )
			printk("AVIA: wakeup ... error\n");
		else
			printk("AVIA: wakeup ... ok\n");
	}
	else
	{
		avia_av_event_exit();

		/* disable interrupts */
		wDR(0x200,0);

		free_irq(AVIA_INTERRUPT, &dev);

		/* enable host access */
		wGB(0, 0x1000);
		/* cpu reset, 3 state mode */
		wGB(0x39, 0xF00000);
		/* cpu reset by fp */
		fp_do_reset(0xBF & ~ (2));
	}

	return 0;
}

/* ---------------------------------------------------------------------- */

u16 avia_get_sample_rate(void)
{

	return sample_rate;

}

#if 0

int avia_av_pid_set_audio(u16 pid)
{

	pid_audio = pid;

	avia_command(SelectStream, 0x03, pid_audio);
	
	return 0;	

}


int avia_av_pid_set_video(u16 pid)
{

	pid_video = pid;

	avia_command(SelectStream, 0x03, pid_video);
	
	return 0;	

}

int avia_av_stream_type_set(u8 new_stream_type_video, u8 new_stream_type_audio)
{

	switch(new_stream_type_video) {
	
		case AVIA_AV_STREAM_TYPE_ES:

			switch(new_stream_type_audio) {
	
				case AVIA_AV_STREAM_TYPE_ES:
		
					avia_command(SetStreamType, 0x08, 0x0000);
					
				break;
			
				case AVIA_AV_STREAM_TYPE_PES:

					avia_command(SetStreamType, 0x0A, 0x0000);
					
				break;
			
				case AVIA_AV_STREAM_TYPE_SPTS:
				
					printk("avia_av: video ES with audio SPTS stream type is not supported\n");
				
					return -EINVAL;
				
				break;
			
				default:

					printk("avia_av: invalid audio stream type\n");
		
					return -EINVAL;
			
				break;
		
			}
			
		break;
			
		case AVIA_AV_STREAM_TYPE_PES:

			switch(new_stream_type_audio) {
	
				case AVIA_AV_STREAM_TYPE_ES:
		
					avia_command(SetStreamType, 0x09, 0x0000);
					
				break;
			
				case AVIA_AV_STREAM_TYPE_PES:

					avia_command(SetStreamType, 0x0B, 0x0000);
					
				break;
			
				case AVIA_AV_STREAM_TYPE_SPTS:
				
					printk("avia_av: video PES with audio SPTS stream type is not supported\n");
				
					return -EINVAL;
				
				break;
			
				default:

					printk("avia_av: invalid audio stream type\n");
		
					return -EINVAL;
			
				break;
		
			}
			
		break;
		
		case AVIA_AV_STREAM_TYPE_SPTS:

			switch(new_stream_type_audio) {
	
				case AVIA_AV_STREAM_TYPE_ES:
		
					printk("avia_av: video SPTS with audio ES stream type is not supported\n");
					
					return -EINVAL;
					
				break;
			
				case AVIA_AV_STREAM_TYPE_PES:

					printk("avia_av: video SPTS with audio PES stream type is not supported\n");
					
					return -EINVAL;
					
				break;
			
				case AVIA_AV_STREAM_TYPE_SPTS:

					// AViA 500 doesn't support SetStreamType 0x10/0x11
					// So we Reset the AViA 500 back to SPTS mode
					
//FIXME				if (avia 500) {
					
						avia_command(Reset);				

						avia_command(SelectStream, 0x00, pid_video);
						avia_command(SelectStream, 0x03, pid_audio);
						
//FIXME				} else {
					
						avia_command(SetStreamType, 0x10, pid_audio);
						avia_command(SetStreamType, 0x11, pid_video);
						
//FIXME				}
					
				break;
			
				default:

					printk("avia_av: invalid audio stream type\n");
		
					return -EINVAL;
			
				break;
		
			}
		break;
		
		default:
		
			printk("avia_av: invalid video stream type\n");
			
			return -EINVAL;
			
		break;
		
	}

	stream_type_audio = new_stream_type_audio;
	stream_type_video = new_stream_type_video;

	return 0;
	
}	

int avia_av_stream_type_set_audio(u8 stream_type)
{
	
	// SPTS is only supported if both video and audio are in SPTS mode
	if ((stream_type != AVIA_AV_STREAM_TYPE_SPTS) && (stream_type_video == AVIA_AV_STREAM_TYPE_SPTS))
		stream_type_video = stream_type;
		
	return avia_av_stream_type_set(stream_type_video, stream_type);

}

int avia_av_stream_type_set_video(u8 stream_type)
{
	
	// SPTS is only supported if both video and audio are in SPTS mode
	if ((stream_type != AVIA_AV_STREAM_TYPE_SPTS) && (stream_type_audio == AVIA_AV_STREAM_TYPE_SPTS))
		stream_type_audio = stream_type;
		
	return avia_av_stream_type_set(stream_type, stream_type_audio);

}

#endif

/* ---------------------------------------------------------------------- */

EXPORT_SYMBOL(aviarev);
EXPORT_SYMBOL(avia_wr);
EXPORT_SYMBOL(avia_rd);
EXPORT_SYMBOL(avia_command);
EXPORT_SYMBOL(avia_set_pcr);
EXPORT_SYMBOL(avia_flush_pcr);
EXPORT_SYMBOL(avia_standby);
EXPORT_SYMBOL(avia_get_sample_rate);

/* ---------------------------------------------------------------------- */

MODULE_AUTHOR("Felix Domke <tmbinc@gmx.net>");
MODULE_DESCRIPTION("Avia 500/600 driver");
MODULE_PARM(debug,"i");
MODULE_PARM(pal,"i");
MODULE_PARM(firmware,"s");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

int
init_module (void)
{

	int err;

	printk ("avia_av: $Id: avia_av_core.c,v 1.36.2.1 2002/11/17 01:59:13 obi Exp $\n");

	aviamem = 0;

	if (!(err = init_avia()))
		avia_av_proc_init();

	return err;
	
}

void cleanup_module(void)
{

	avia_av_proc_exit();

	avia_standby(1);

	if (aviamem)
		iounmap((void*)aviamem);

}
#endif
