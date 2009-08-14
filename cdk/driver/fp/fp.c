/*
 *   fp.c - FP driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2001 Felix "tmbinc" Domke (tmbinc@gmx.net)
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
 *   $Log: fp.c,v $
 *   Revision 1.72.2.5  2003/01/04 07:50:49  Zwen
 *   - improved timer detection/clear
 *
 *   Revision 1.72.2.4  2002/12/03 18:23:18  Zwen
 *   - modified wakeup detection
 *
 *   Revision 1.72.2.3  2002/11/27 19:24:26  Zwen
 *   - wakeup detection improved
 *
 *   Revision 1.72.2.2  2002/11/21 18:35:34  Zwen
 *   - IS_WAKEUP ioctl is now compatible with dbox_fp (u8 vs. int)
 *
 *   Revision 1.72.2.1  2002/10/22 20:20:43  Zwen
 *   Nokia needs an additional read from address 0x2A for the wakeup to be cleared
 *
 *   Revision 1.72  2002/10/09 16:51:00  Zwen
 *   - clear_wakeup fuer nokias implementiert
 *   - FP_IOCTL_CLEAR_WAKEUP entfernt, wird jetzt im module_init ausgefuehrt
 *   - neuer ioctl FP_IOCTL_IS_WAKEUP : Aufruf ioctl(fd, FP_IOCTL_IS_WAKEUP, &val)
 *     Fkt: Wie sind wir heute morgen geweckt worden ?
 *          val==0 : Sanft durch Benutzerhand
 *          val==1 : ruede durch so nen bloeden wakeup timer :-)
 *
 *   Revision 1.71  2002/10/08 19:42:06  Zwen
 *   - Wakeup fuer sagem/phillips implementiert
 *   - neuer IOCTL: FP_IOCTL_CLEAR_WAKEUP_TIMER loescht den wakeup-timer eines vorherigen wakeups
 *     sollte nach wakeup mind. 1 mal ausgefuehrt werden -> ioctl(fd,  FP_IOCTL_CLEAR_WAKEUP_TIMER, NULL)
 *
 *   Revision 1.70  2002/05/27 18:11:47  happydude
 *   make led on/off work on Nokia
 *
 *   Revision 1.69  2002/05/15 22:02:49  Hunz
 *   raw register read for debugging/testing
 *
 *   Revision 1.68  2002/05/12 11:18:38  Hunz
 *   LCD-DIMM fix (for all boxes)
 *
 *   Revision 1.67  2002/05/06 02:18:19  obi
 *   cleanup for new kernel
 *
 *   Revision 1.66  2002/03/02 19:09:09  tmbinc
 *   fixed status
 *
 *   Revision 1.65  2002/03/02 19:00:49  tmbinc
 *   ups, small typo
 *
 *   Revision 1.64  2002/03/02 18:33:14  tmbinc
 *   changed VCR_ON/OFF to _CHANGED, added IOCTL to get status
 *
 *   Revision 1.63  2002/03/02 17:10:16  waldi
 *   merge new_tuning_api
 *
 *   Revision 1.54.2.1  2002/03/02 17:03:43  tmbinc
 *   modified reset delay
 *
 *   Revision 1.62  2002/02/24 19:11:22  obi
 *   revert to current fp.c - is not related to tuning api
 *
 *   Revision 1.60  2002/02/11 16:11:19  Hunz
 *   keyboard-debug-msgs only when module loaded with debug=1 now
 *
 *   Revision 1.59  2002/02/08 20:52:30  Hunz
 *   keyboard/mouse now work with SAGEM (and Phillips??) too :-)
 *
 *   Revision 1.58  2002/02/08 00:28:59  Hunz
 *   mousepad-inputdev support / keyboard still raw (keyboard-inputdev support still sux :()
 *
 *   Revision 1.57  2002/02/06 23:54:57  Hunz
 *   IR-Keyboard Mousepad test - Inputdev not yet working
 *
 *   Revision 1.56  2002/01/23 19:54:11  Hunz
 *   experimental input support
 *
 *   Revision 1.55  2002/01/23 15:49:32  Hunz
 *   lala
 *
 *   Revision 1.54  2002/01/21 15:18:46  Hunz
 *   maybe keyboard fix
 *
 *   Revision 1.53  2002/01/20 06:21:49  Hunz
 *   keyboard change
 *
 *   Revision 1.52  2002/01/19 17:32:57  Hunz
 *   didn't help
 *
 *   Revision 1.51  2002/01/19 17:20:18  Hunz
 *   kbd-fix?
 *
 *   Revision 1.50  2002/01/19 08:57:38  Hunz
 *   last idea
 *
 *   Revision 1.49  2002/01/19 08:40:25  Hunz
 *   last commit so far...
 *
 *   Revision 1.48  2002/01/19 08:20:02  Hunz
 *   F(i)n(al) fix?
 *
 *   Revision 1.47  2002/01/19 07:58:21  Hunz
 *   SHOULD work...
 *
 *   Revision 1.46  2002/01/19 07:48:35  Hunz
 *   OOPS I did it again...
 *   (but that one was a really nasty one - hope it works now)
 *
 *   Revision 1.45  2002/01/19 05:59:28  Hunz
 *   I will first look for bugs and then commit
 *   I will first look for bugs and then commit
 *   I will first look for bugs and then commit
 *   I will first look for bugs and then commit
 *   I will first look for bugs and then commit
 *   I will first look for bugs and then commit
 *   I will first look for bugs and then commit
 *   I will first look for bugs and then commit
 *
 *   Revision 1.44  2002/01/19 05:54:46  Hunz
 *   keyboard might work now
 *
 *   Revision 1.43  2002/01/19 03:55:06  Hunz
 *   some stupid mistakes fixed
 *
 *   Revision 1.42  2002/01/19 03:38:06  Hunz
 *   initial keyboard kernel support
 *
 *   Revision 1.41  2002/01/17 22:38:09  tmbinc
 *   added keyboard support. keyboard events are ignored.
 *
 *   Revision 1.40  2002/01/10 16:47:06  Hunz
 *   possible SEC_VOLTAGE_LT fix maybe... (untested)
 *
 *   Revision 1.39  2002/01/04 00:59:13  Hunz
 *   added FP_IOCTL_REBOOT
 *
 *   Revision 1.38  2001/12/08 15:20:14  gillem
 *   - add global event handler now
 *
 *   Revision 1.37  2001/12/02 10:33:01  TripleDES
 *   added low-band support (never missed ;)
 *
 *   Revision 1.36  2001/12/01 10:51:22  gillem
 *   - add vcr handling
 *   - todo: add event (tmbinc???)
 *
 *   Revision 1.35  2001/12/01 06:52:28  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.34  2001/11/06 15:54:59  tmbinc
 *   added FP_WAKEUP and ioctls. (only for nokia)
 *
 *   Revision 1.33  2001/10/30 23:17:26  derget
 *
 *   FP_IOCTL_POWEROFF für sagem eingebaut
 *
 *   Revision 1.32  2001/10/30 13:40:55  derget
 *   sagem restart
 *
 *   Revision 1.31  2001/07/31 03:01:39  Hunz
 *   DiSEqC fix (sagem still untested)
 *
 *   Revision 1.30  2001/07/31 01:40:28  Hunz
 *   experimental sagem-diseqc support
 *
 *   Revision 1.29  2001/05/01 02:00:01  TripleDES
 *   added fp_sagem_set_secpower for LNB-voltage control (H/V)
 *   -not completed
 *
 *   Revision 1.28  2001/04/26 17:28:07  Hunz
 *   breakcode-fix
 *
 *   Revision 1.27  2001/04/26 16:56:58  Hunz
 *   added breakcodes support
 *
 *   Revision 1.26  2001/04/22 20:43:40  tmbinc
 *   fixed RC for new and old fp-code
 *
 *   Revision 1.25  2001/04/09 22:58:22  tmbinc
 *   added philips-support.
 *
 *   Revision 1.24  2001/04/09 22:33:57  TripleDES
 *   some unused commands cleared (sagem testing)
 *
 *   Revision 1.23  2001/04/09 19:49:40  TripleDES
 *   added fp_cam_reset for sagem/philips? support
 *
 *   Revision 1.22  2001/04/06 21:15:20  tmbinc
 *   Finally added new rc-support.
 *
 *   Revision 1.21  2001/04/01 01:54:25  tmbinc
 *   added "poll"-support, blocks on open if already opened
 *
 *   Revision 1.20  2001/03/18 21:28:24  tmbinc
 *   fixed again some bug.
 *
 *   Revision 1.19  2001/03/15 22:20:23  Hunz
 *   nothing important...
 *
 *   Revision 1.18  2001/03/14 14:35:58  Hunz
 *   fixed DiSEqC timing
 *
 *   Revision 1.17  2001/03/12 22:03:40  Hunz
 *   final? SEC-fix (always clrbit the VES)
 *
 *   Revision 1.16  2001/03/12 19:51:37  Hunz
 *   SEC changes
 *
 *   Revision 1.15  2001/03/11 18:28:50  gillem
 *   - add new option (test only)
 *
 *   Revision 1.14  2001/03/08 14:08:50  Hunz
 *   DiSEqC number of params changed to 0-3
 *
 *   Revision 1.13  2001/03/06 18:49:20  Hunz
 *   fix for sat-boxes (fp_set_pol... -> fp_set_sec)
 *
 *   Revision 1.12  2001/03/04 18:48:07  gillem
 *   - fix for sagem box
 *
 *   Revision 1.11  2001/03/03 18:20:39  waldi
 *   complete move to devfs; doesn't compile without devfs
 *
 *   Revision 1.10  2001/03/03 13:03:20  gillem
 *   - fix code
 *
 *   Revision 1.9  2001/02/25 21:11:36  gillem
 *   - fix fpid
 *
 *   Revision 1.8  2001/02/23 18:44:43  gillem
 *   - add ioctl
 *   - add debug option
 *   - some changes ...
 *
 *
 *   $Revision: 1.72.2.5 $
 *
 */

/* ---------------------------------------------------------------------- */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/tqueue.h>
#include <linux/i2c.h>
#include <linux/poll.h>
#include <asm/irq.h>
#include <asm/mpc8xx.h>
#include <asm/8xx_immap.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/signal.h>

#include "dbox/event.h"
#include "dbox/fp.h"
#include "dbox/info.h"

#include <linux/devfs_fs_kernel.h>

#include <ost/sec.h>

#ifdef CONFIG_INPUT_MODULE
#include <linux/input.h>
#endif

#include <linux/kbd_ll.h>
#include <linux/kbd_kern.h>
#include <asm/keyboard.h>

#ifndef CONFIG_DEVFS_FS
#error no devfs
#endif


static devfs_handle_t devfs_handle[2];
static int sec_bus_status=0;
static struct dbox_info_struct info;
static u8 is_wakeup=0;

/* ---------------------------------------------------------------------- */

#ifdef MODULE
static int debug=0;
static int useimap=1;
#endif

#define dprintk(fmt,args...) if(debug) printk( fmt,## args)

/* ---------------------------------------------------------------------- */

/*
      exported functions:

      int fp_set_tuner_dword(int type, u32 tw);
	T_QAM
	T_QPSK
      int fp_set_sec(int power,int tone);
*/

/*
fp:
 03  deep standby
 10 led on
 11 led off
 dez.
 20 reboot
 21 reboot
 42 lcd off / led off ( alloff ;-) )
 ADDR VAL
 18   X0  X = dimm 0=off F=on
 22 off
*/

#define FP_INTERRUPT	SIU_IRQ2
#define I2C_FP_DRIVERID     0xF060
#define RCBUFFERSIZE	16
#define FP_GETID	    0x1D
#define FP_WAKEUP		0x11
#define FP_WAKEUP_SAGEM		0x01
#define FP_STATUS		0x20
#define FP_CLEAR_WAKEUP		0x21
#define FP_CLEAR_WAKEUP_NOKIA	0x2B

/* ---------------------------------------------------------------------- */

/* Scan 0x60 */
static unsigned short normal_i2c[] = { 0x60>>1,I2C_CLIENT_END };
static unsigned short normal_i2c_range[] = { 0x60>>1, 0x60>>1, I2C_CLIENT_END };
I2C_CLIENT_INSMOD;

static int fp_id=0;
static int rc_bcodes=0;

struct fp_data
{
  int fpID;
  int fpVCR;
  struct i2c_client *client;
};

struct fp_data *defdata=0;

static u16 rcbuffer[RCBUFFERSIZE];
static u16 rcbeg, rcend;
static wait_queue_head_t rcwait;
static DECLARE_MUTEX_LOCKED(rc_open);

static void fp_task(void *);

struct tq_struct fp_tasklet=
{
  routine: fp_task,
  data: 0
};

static int fp_ioctl (struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int fp_open (struct inode *inode, struct file *file);
static ssize_t fp_write (struct file *file, const char *buf, size_t count, loff_t *offset);
static ssize_t fp_read (struct file *file, char *buf, size_t count, loff_t *offset);
static int fp_release(struct inode *inode, struct file *file);
static unsigned int fp_poll(struct file *file, poll_table *wait);

static int fp_detach_client(struct i2c_client *client);
static int fp_detect_client(struct i2c_adapter *adapter, int address, unsigned short flags, int kind);
static int fp_attach_adapter(struct i2c_adapter *adapter);
static int fp_getid(struct i2c_client *client);
static void fp_interrupt(int irq, void *dev, struct pt_regs * regs);
static int fp_cmd(struct i2c_client *client, u8 cmd, u8 *res, int size);
static int fp_sendcmd(struct i2c_client *client, u8 b0, u8 b1);
static void fp_check_queues(void);
static int fp_set_wakeup_timer(int minutes);
static int fp_get_wakeup_timer(void);
static int fp_clear_wakeup_timer(void);

static void fp_restart(char *cmd);
static void fp_power_off(void);
static void fp_halt(void);

static int irkbd_setkeycode(unsigned int scancode, unsigned int keycode);
static int irkbd_getkeycode(unsigned int scancode);
static int irkbd_translate(unsigned char scancode, unsigned char *keycode, char raw_mode);
static char irkbd_unexpected_up(unsigned char keycode);
static void irkbd_leds(unsigned char leds);
static void __init irkbd_init_hw(void);
#ifdef CONFIG_INPUT_MODULE
static int irkbd_event(struct input_dev *dev, unsigned int type, unsigned int code, int value);
struct input_dev input_irkbd;
static char irkbd_name[]="dBox2 IR-Keyboard";
#endif

unsigned char keymap[256]={
  0,   1,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,   0,   0,   0,   0,
  0,   0,   0,   0,  41,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,
 13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  43,
 58,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  28,  42,  44,  45,
 46,  47,  48,  49,  50,  51,  52,  53,  54,   0,  29, 125,  56,  57, 100,   0,
127,   0,  99,  70, 119, 110,   0,   0, 111,   0,   0,   0,   0,   0,   0, 103,
 86, 105, 108, 106,  69,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  /* Fn Keymap starts here */
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 104,
  0, 102, 109, 107,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

unsigned char fn_flags[128];

unsigned char irkbd_flags=0;

#define IRKBD_FN 0x80
#define IRKBD_MOUSER 2
#define IRKBD_MOUSEL 4

#ifdef CONFIG_INPUT_MODULE
unsigned char mouse_directions=0;

#define DIR_RIGHT_UP   0x01
#define DIR_UP_RIGHT   0x02
#define DIR_UP_LEFT    0x04
#define DIR_LEFT_UP    0x08
#define DIR_LEFT_DOWN  0x10
#define DIR_DOWN_LEFT  0x20
#define DIR_DOWN_RIGHT 0x40
#define DIR_RIGHT_DOWN 0x80
#endif

/* ------------------------------------------------------------------------- */

static struct i2c_driver fp_driver=
{
  name:		 "DBox2 Frontprocessor driver",
  id:		   I2C_FP_DRIVERID,
  flags:		I2C_DF_NOTIFY,
  attach_adapter:       &fp_attach_adapter,
  detach_client:	&fp_detach_client,
  command:	      0,
  inc_use:	      0,
  dec_use:	      0
};

static struct file_operations fp_fops = {
	owner:	  THIS_MODULE,
	read:	   fp_read,
	write:	  fp_write,
	ioctl:	  fp_ioctl,
	open:	   fp_open,
	release:	fp_release,
	poll:		fp_poll,
};

/* ------------------------------------------------------------------------- */

static int fp_ioctl (struct inode *inode, struct file *file, unsigned int cmd,
		  unsigned long arg)
{
	unsigned int minor = MINOR (file->f_dentry->d_inode->i_rdev);
	int val;

	switch (minor)
	{
		case FP_MINOR:
		{
			switch (cmd)
			{
				case FP_IOCTL_GETID:
					return fp_getid(defdata->client);
					break;

				case FP_IOCTL_POWEROFF:
					if (info.fpREV>=0x80)
						return fp_sendcmd(defdata->client, 0, 3);
					else
						return fp_sendcmd(defdata->client, 0, 0);
					break;
				case FP_IOCTL_REBOOT:
					fp_restart("LIFE SUX");
					return 0;
					break;
				case FP_IOCTL_LCD_DIMM:
					if (copy_from_user(&val, (void*)arg, sizeof(val)) )
					{
						return -EFAULT;
					}
					if (info.fpREV>=0x80)
						return fp_sendcmd(defdata->client, 0x18, val&0xff);
					else
						return fp_sendcmd(defdata->client, 0x06, val&0xff);
					break;

				case FP_IOCTL_LED:
					if (copy_from_user(&val, (void*)arg, sizeof(val)) )
					{
						return -EFAULT;
					}
					if (info.fpREV>=0x80)
						return fp_sendcmd(defdata->client, 0, 0x10|((~val)&1));
					else
						return fp_sendcmd(defdata->client, 0x10|(val&1), 0);
					break;
				case FP_IOCTL_GET_WAKEUP_TIMER:
					val=fp_get_wakeup_timer();
					if (val==-1)
						return -EIO;
					if (copy_to_user((void*)arg, &val, sizeof(val)))
						return -EFAULT;
					return 0;
					break;
				case FP_IOCTL_SET_WAKEUP_TIMER:
					if (copy_from_user(&val, (void*)arg, sizeof(val)) )
						return -EFAULT;
					return fp_set_wakeup_timer(val);
				case FP_IOCTL_IS_WAKEUP:
					if (copy_to_user((void*)arg, &is_wakeup, sizeof(is_wakeup)))
						return -EFAULT;
					return 0;
				case FP_IOCTL_GET_VCR:
					if (copy_to_user((void*)arg, &defdata->fpVCR, sizeof(defdata->fpVCR)))
						return -EFAULT;
					return 0;
				case FP_IOCTL_GET_REGISTER:
					{
						unsigned long foo;

						if (copy_from_user(&val, (void*)arg, sizeof(val)) )

							                return -EFAULT;
						
						fp_cmd(defdata->client, val&0xFF, (u8*)&foo, ((val>>8)&3)+1);
						if (copy_to_user((void*)arg, &foo, sizeof(foo)))
							                                                return -EFAULT;
						return 0;
					}
					
				default:
					return -EINVAL;
			}
		}

		case RC_MINOR:
		{
			switch (cmd)
			{
				case RC_IOCTL_BCODES:
					if (arg > 0)
					{
						rc_bcodes=1;
						return fp_sendcmd(defdata->client, 0x26, 0x80);
					}
					else
					{
						rc_bcodes=0;
						return fp_sendcmd(defdata->client, 0x26, 0);
					}
					break;
				default:
					return -EINVAL;
			}
		}

		default:
			return -EINVAL;
	}

	return -EINVAL;
}

/* ------------------------------------------------------------------------- */

static ssize_t fp_write (struct file *file, const char *buf, size_t count, loff_t *offset)
{
	return 0;
}

/* ------------------------------------------------------------------------- */

static ssize_t fp_read (struct file *file, char *buf, size_t count, loff_t *offset)
{
	unsigned int minor = MINOR (file->f_dentry->d_inode->i_rdev), read;

	switch (minor)
	{
		case FP_MINOR:
			return -EINVAL;

		case RC_MINOR:
		{
			int i;
			DECLARE_WAITQUEUE(wait, current);
			read=0;

			for(;;)
			{
				if (rcbeg==rcend)
				{
					if (file->f_flags & O_NONBLOCK)
					{
						return read;
					}

					add_wait_queue(&rcwait, &wait);
					set_current_state(TASK_INTERRUPTIBLE);
					schedule();
					current->state = TASK_RUNNING;
					remove_wait_queue(&rcwait, &wait);

					if (signal_pending(current))
					{
						return -ERESTARTSYS;
					}

					continue;
				}

				break;
			}

			count&=~1;

			for (i=0; i<count; i+=2)
			{
				if (rcbeg==rcend)
				{
					break;
				}

				*((u16*)(buf+i))=rcbuffer[rcbeg++];
				read+=2;

				if (rcbeg>=RCBUFFERSIZE)
				{
					rcbeg=0;
				}
			}

			return read;
		}

		default:
			return -EINVAL;
	}

	return -EINVAL;
}

/* ------------------------------------------------------------------------- */

static int fp_open (struct inode *inode, struct file *file)
{
	unsigned int minor = MINOR (file->f_dentry->d_inode->i_rdev);

	switch (minor)
	{
		case FP_MINOR:
			return 0;

		case RC_MINOR:
			if (file->f_flags & O_NONBLOCK)
			{
				if (down_trylock(&rc_open))
					return -EAGAIN;
			}	else
			{
				if (down_interruptible(&rc_open))
					return -ERESTARTSYS;
			}
			return 0;

		default:
			return -ENODEV;
	}

	return -EINVAL;
}

/* ------------------------------------------------------------------------- */

static int fp_release(struct inode *inode, struct file *file)
{
	unsigned int minor = MINOR (file->f_dentry->d_inode->i_rdev);

	switch (minor)
	{
	case FP_MINOR:
		return 0;
	case RC_MINOR:
		if (rc_bcodes != 0)
			fp_sendcmd(defdata->client, 0x26, 0);
		up(&rc_open);
		return 0;
	}
	return -EINVAL;
}

/* ------------------------------------------------------------------------- */

static unsigned int fp_poll(struct file *file, poll_table *wait)
{
	unsigned int minor = MINOR (file->f_dentry->d_inode->i_rdev);
	switch (minor)
	{
	case FP_MINOR:
		return -EINVAL;
	case RC_MINOR:
		poll_wait(file, &rcwait, wait);
		if (rcbeg!=rcend)
			return POLLIN|POLLRDNORM;
		return 0;
	}
	return -EINVAL;
}

/* ------------------------------------------------------------------------- */

static int fp_detach_client(struct i2c_client *client)
{
	int err;

	free_irq(FP_INTERRUPT, client->data);

	if ((err=i2c_detach_client(client)))
	{
		dprintk("fp.o: couldn't detach client driver.\n");
		return err;
	}

	kfree(client);
	return 0;
}

/* ------------------------------------------------------------------------- */

static int fp_detect_client(struct i2c_adapter *adapter, int address, unsigned short flags, int kind)
{
	int err = 0;
	struct i2c_client *new_client;
	struct fp_data *data;
	const char *client_name="DBox2 Frontprocessor client";

	if (!(new_client=kmalloc(sizeof(struct i2c_client)+sizeof(struct fp_data), GFP_KERNEL)))
	{
		return -ENOMEM;
	}

	new_client->data=new_client+1;
	defdata=data=(struct fp_data*)(new_client->data);
	/* init vcr value (off) */
	defdata->fpVCR=0;
	rcbeg=0;
	rcend=0;
	new_client->addr=address;
	data->client=new_client;
	new_client->data=data;
	new_client->adapter=adapter;
	new_client->driver=&fp_driver;
	new_client->flags=0;

	if (kind<0)
	{
		int fpid;
//		u8 buf[2];
		immap_t *immap=(immap_t*)IMAP_ADDR;

		/* FP ID
		 * NOKIA: 0x5A
		 * SAGEM: 0x52 ???
		 * PHILIPS: 0x52
		 */

		fpid=fp_getid(new_client);

		if ( (fpid!=0x52) && (fpid!=0x5a) )
		{
			dprintk("fp.o: bogus fpID %d\n", fpid);
			kfree(new_client);
			return -1;
		}

		if(useimap)
		{
			immap->im_ioport.iop_papar&=~2;
			immap->im_ioport.iop_paodr&=~2;
			immap->im_ioport.iop_padir|=2;
			immap->im_ioport.iop_padat&=~2;
		}

//    fp_sendcmd(new_client, 0x04, 0x51); //sagem needs this (71) LNB-Voltage 51-V 71-H
/*    fp_sendcmd(new_client, 0x22, 0xbf);
    fp_cmd(new_client, 0x25, buf, 2);
    fp_sendcmd(new_client, 0x19, 0x04);
    fp_sendcmd(new_client, 0x18, 0xb3);
    fp_cmd(new_client, 0x1e, buf, 2);
*/
		fp_sendcmd(new_client, 0x26, 0x00);		// disable (non-working) break code

	/*	fp_cmd(new_client, 0x23, buf, 1);
		fp_cmd(new_client, 0x20, buf, 1);
		fp_cmd(new_client, 0x01, buf, 2);*/
	}

	strcpy(new_client->name, client_name);
	new_client->id=fp_id++;

	if ((err=i2c_attach_client(new_client)))
	{
		kfree(new_client);
		return err;
	}

	if (request_8xxirq(FP_INTERRUPT, fp_interrupt, SA_ONESHOT, "fp", data) != 0)
	{
		panic("Could not allocate FP IRQ!");
	}

	up(&rc_open);
	return 0;
}

/* ------------------------------------------------------------------------- */

static int fp_attach_adapter(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, &fp_detect_client);
}

/* ------------------------------------------------------------------------- */

static int fp_cmd(struct i2c_client *client, u8 cmd, u8 *res, int size)
{
	struct i2c_msg msg[2];
	int i;

	msg[0].flags=0;
	msg[1].flags=I2C_M_RD;
	msg[0].addr=msg[1].addr=client->addr;

	msg[0].buf=&cmd;
	msg[0].len=1;

	msg[1].buf=res;
	msg[1].len=size;

	i2c_transfer(client->adapter, msg, 2);

	dprintk("fp.o: fp_cmd: %02x\n", cmd);
	dprintk("fp.o: fp_recv:");

	if(debug)
	{
		for (i=0; i<size; i++)
			dprintk(" %02x", res[i]);
		dprintk("\n");
	}

	return 0;
}

/* ------------------------------------------------------------------------- */

static int fp_sendcmd(struct i2c_client *client, u8 b0, u8 b1)
{
	u8 cmd[2]={b0, b1};

	dprintk("fp.o: fp_sendcmd: %02x %02x\n", b0, b1);

	if (i2c_master_send(client, cmd, 2)!=2)
		return -1;

	return 0;
}

/* ------------------------------------------------------------------------- */

static int fp_getid(struct i2c_client *client)
{
	u8 id[3]={0, 0, 0};

	if (fp_cmd(client, FP_GETID, id, 3))
		return 0;

	return id[0];
}

/* ------------------------------------------------------------------------- */

static void fp_add_event(int code)
{
	if (atomic_read(&rc_open.count)>=1)
		return;

	rcbuffer[rcend]=code;
	rcend++;

	if (rcend>=RCBUFFERSIZE)
	{
		rcend=0;
	}

	if (rcbeg==rcend)
	{
		printk("fp.o: RC overflow.\n");
	} else
	{
		wake_up(&rcwait);
	}
}

/* ------------------------------------------------------------------------- */

static void fp_handle_rc(struct fp_data *dev)
{
	u16 rc;

	fp_cmd(dev->client, 0x1, (u8*)&rc, 2);
	fp_add_event(rc);
}

/* ------------------------------------------------------------------------- */

static void fp_handle_new_rc(struct fp_data *dev)
{
	u16 rc;

	fp_cmd(dev->client, 0x26, (u8*)&rc, 2);
	fp_add_event(rc);
}

/* ------------------------------------------------------------------------- */

static void fp_handle_button(struct fp_data *dev)
{
	u8 rc;

	fp_cmd(dev->client, 0x25, (u8*)&rc, 1);
	fp_add_event(rc|0xFF00);
}

/* ------------------------------------------------------------------------- */

static void fp_handle_vcr(struct fp_data *dev, int fpVCR)
{
	struct event_t event;

	memset(&event,0,sizeof(event_t));

	if (dev->fpVCR!=fpVCR)
	{
		dev->fpVCR = fpVCR;

		event.event = EVENT_VCR_CHANGED;
		event_write_message( &event, 1 );
	}
}

/* ------------------------------------------------------------------------ */

int irkbd_setkeycode(unsigned int scancode, unsigned int keycode) {

  if(scancode>255||keycode>127)
    return -EINVAL;
  keymap[scancode&0xFF]=keycode;
  return 0;
}

int irkbd_getkeycode(unsigned int scancode) {

  return keymap[scancode&0xFF];
}

int irkbd_translate(unsigned char scancode, unsigned char *keycode, char raw_mode) {

  if((scancode&0x7f)==0x49) {  /* Fn toggled */
    if(scancode==0x49)
      irkbd_flags|=IRKBD_FN;
    else
      irkbd_flags&=~IRKBD_FN;
    return 0;
  }
  /* mouse button changed */
  else if(((scancode&0x7f)==0x7e)||((scancode&0x7f)==0x7f))
    return 0;

  if(irkbd_flags&IRKBD_FN) {
    *keycode=keymap[(scancode&0x7f)|IRKBD_FN];
    if(scancode&0x80)		   /* Fn pressed, other key released */
      if(!fn_flags[scancode&0x7f])       /* fn pressed, other key released which got pressed before fn */
	*keycode=keymap[scancode&0x7f];
      else			      /* fn pressed, other key released which got pressed during fn */
	fn_flags[scancode&0x7f]=0;
    else				/* Fn + other key pressed */
      fn_flags[scancode&0x7f]=1;
  }
  /* key got pressed during fn and gets now released after fn has been released*/
  else if((!(irkbd_flags&IRKBD_FN)) && (fn_flags[scancode&0x7f])) {
    *keycode=keymap[(scancode&0x7f)|IRKBD_FN];
    fn_flags[scancode&0x7f]=0;
  }
  /* no Fn - other key pressed/released */
  else
    *keycode=keymap[scancode&0x7f];

  if(*keycode==0) {
    if (!raw_mode)
      dprintk("fp.o: irkbd: unknown scancode 0x%02X (flags 0x%02X)\n",scancode,irkbd_flags);
    return 0;
  }
  return 1;
}

char irkbd_unexpected_up(unsigned char keycode) {
  dprintk("fp.o: irkbd_unexpected_up 0x%02X (flags 0x%02X)\n",keycode,irkbd_flags);
  return 0200;
}

void irkbd_leds(unsigned char leds) {
  dprintk("fp.o: irkbd_leds 0x%02X\n",leds);
}

void __init irkbd_init_hw(void) {
  dprintk("fp.o: IR-Keyboard initialized\n");
}

#ifdef CONFIG_INPUT_MODULE
int irkbd_event(struct input_dev *dev, unsigned int type, unsigned int code, int value) {

  /*
  if((type==EV_SND)&&(code==EV_BELL)) {
    printk("BEEP!\n");
    return 0;
  }
  else
  */
  if (type==EV_LED) {
    dprintk("IR-Keyboard LEDs: [%s|%s|%s]\n",(test_bit(LED_NUML,dev->led)?" NUM ":"     "),(test_bit(LED_CAPSL,dev->led)?"CAPS ":"     "),(test_bit(LED_SCROLLL,dev->led)?"SCROLL":"     "));
    return 0;
  }
  else
    return -1;
}
#endif

/* ------------------------------------------------------------------------- */

static void fp_handle_unknown(struct fp_data *dev)
{
	u8 rc;

	fp_cmd(dev->client, 0x24, (u8*)&rc, 1);
	dprintk("fp.o: misterious interrupt source 0x40: %x\n", rc);
}

/* ------------------------------------------------------------------------- */

static void fp_interrupt(int irq, void *vdev, struct pt_regs * regs)
{
	immap_t *immap=(immap_t*)IMAP_ADDR;

	if(useimap)
		immap->im_ioport.iop_padat|=2;
	schedule_task(&fp_tasklet);
	return;
}

/* ------------------------------------------------------------------------- */

static int fp_init(void)
{
	int res;
	//	int i;

	dbox_get_info(&info);
	init_waitqueue_head(&rcwait);

	if ((res=i2c_add_driver(&fp_driver)))
	{
		dprintk("fp.o: Driver registration failed, module not inserted.\n");
		return res;
	}

	if (!defdata)
	{
		i2c_del_driver(&fp_driver);
		dprintk("fp.o: Couldn't find FP.\n");
		return -EBUSY;
	}

//	if (register_chrdev(FP_MAJOR, "fp", &fp_fops))
//	{
//		i2c_del_driver(&fp_driver);
//		dprintk("fp.o: unable to get major %d\n", FP_MAJOR);
//		return -EIO;
//	}

  devfs_handle[FP_MINOR] =
    devfs_register ( NULL, "dbox/fp0", DEVFS_FL_DEFAULT, 0, FP_MINOR,
		     S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
		     &fp_fops, NULL );

  if ( ! devfs_handle[FP_MINOR] )
  {
    i2c_del_driver ( &fp_driver );
    return -EIO;
  }

  devfs_handle[RC_MINOR] =
    devfs_register ( NULL, "dbox/rc0", DEVFS_FL_DEFAULT, 0, RC_MINOR,
		     S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
		     &fp_fops, NULL );

  if ( ! devfs_handle[RC_MINOR] )
  {
    devfs_unregister ( devfs_handle[FP_MINOR] );
    i2c_del_driver ( &fp_driver );
    return -EIO;
  }

	ppc_md.restart=fp_restart;
	ppc_md.power_off=fp_power_off;
	ppc_md.halt=fp_halt;

	/* keyboard */
	memset(fn_flags,0,sizeof(fn_flags));
	ppc_md.kbd_setkeycode    = irkbd_setkeycode;
	ppc_md.kbd_getkeycode    = irkbd_getkeycode;
	ppc_md.kbd_translate     = irkbd_translate;
	ppc_md.kbd_unexpected_up = irkbd_unexpected_up;
	ppc_md.kbd_leds	  = irkbd_leds;
	ppc_md.kbd_init_hw       = irkbd_init_hw;
#ifdef CONFIG_MAGIC_SYSRQ
	ppc_md.kbd_sysrq_xlate   = keymap;
#endif
	//	irkbd_init_hw();
//	kbd_ledfunc = irkbd_leds;
#ifdef CONFIG_INPUT_MODULE
	memset(&input_irkbd,0,sizeof(input_irkbd));
	input_irkbd.evbit[0]=BIT(EV_KEY) | BIT(EV_REL); // BIT(EV_LED) | BIT(EV_REP)
	//input_irkbd.ledbit[0]=BIT(LED_NUML) | BIT(LED_CAPSL) | BIT(LED_SCROLLL);
	/*	for(i=0;i<=255;i++)
		set_bit(keymap[i],input_irkbd.keybit); */
	input_irkbd.keybit[LONG(BTN_MOUSE)] = BIT(BTN_LEFT) | BIT(BTN_RIGHT);
	input_irkbd.relbit[0] = BIT(REL_X) | BIT(REL_Y);
	input_irkbd.event=irkbd_event;
	input_irkbd.name=irkbd_name;
	input_irkbd.idbus=BUS_I2C;
	input_register_device(&input_irkbd);
#endif
   fp_clear_wakeup_timer();
	return 0;
}

/* ------------------------------------------------------------------------- */

static int fp_close(void)
{
	int res;

	if ((res=i2c_del_driver(&fp_driver)))
	{
		dprintk("fp.o: Driver unregistration failed, module not removed.\n");
		return res;
	}

//	if ((res=unregister_chrdev(FP_MAJOR, "fp")))
//	{
//		dprintk("fp.o: unable to release major %d\n", FP_MAJOR);
//		return res;
//	}

	devfs_unregister ( devfs_handle[FP_MINOR] );
	devfs_unregister ( devfs_handle[RC_MINOR] );

	if (ppc_md.restart==fp_restart)
	{
		ppc_md.restart=0;
	}

	if (ppc_md.power_off==fp_power_off)
	{
		ppc_md.power_off=0;
	}

	if (ppc_md.halt==fp_halt)
	{
		ppc_md.halt=0;
	}
	if(ppc_md.kbd_setkeycode==irkbd_setkeycode)
	  ppc_md.kbd_setkeycode=NULL;
	if(ppc_md.kbd_getkeycode==irkbd_getkeycode)
	  ppc_md.kbd_getkeycode=NULL;
	if(ppc_md.kbd_translate==irkbd_translate)
	  ppc_md.kbd_translate=NULL;
	if(ppc_md.kbd_unexpected_up==irkbd_unexpected_up)
	  ppc_md.kbd_unexpected_up=NULL;
	if(ppc_md.kbd_leds==irkbd_leds)
	  ppc_md.kbd_leds=NULL;
	if(ppc_md.kbd_init_hw==irkbd_init_hw)
	  ppc_md.kbd_init_hw=NULL;
#ifdef CONFIG_MAGIC_SYSRQ
	if(ppc_md.kbd_sysrq_xlate==irkbd_sysrq_xlate)
	  ppc_md.kbd_sysrq_xlate=NULL;
#endif
#ifdef CONFIG_INPUT_MODULE
	input_unregister_device(&input_irkbd);
#endif

	return 0;
}

/* ------------------------------------------------------------------------- */
int fp_cam_reset()    //needed for sagem / philips?
{
	char msg[2]={0x05, 0xEF};

	dprintk("fp: CAM-RESET\n");

	if (i2c_master_send(defdata->client, msg, 2)!=2)
	{
		return -1;
	}

	msg[1]=0xFF;

	if (i2c_master_send(defdata->client, msg, 2)!=2)
	{
		return -1;
	}

	return 0;
}

/* ------------------------------------------------------------------------- */

int fp_do_reset(int type)
{
	char msg[2]={0x22, type};

	if (i2c_master_send(defdata->client, msg, 2)!=2)
	{
		return -1;
	}

	/* TODO: make better */
	udelay(1000);

	msg[1]=0xBF;

	if (i2c_master_send(defdata->client, msg, 2)!=2)
	{
		return -1;
	}

	return 0;
}

/*
mouse codes:

1st halfbyte: acceleration

2nd halfbyte: direction:
	    4
	 5     3
      6	   2
   7		 1
8		       0
   9		 F
      A	   E
	 B     D
	    C

*/
static void fp_handle_mouse(struct fp_data *dev) {
  u16 mousecode=-1;

if (info.fpREV>=0x80)
  fp_cmd(dev->client, 5, (u8*)&mousecode, 2);
else
  fp_cmd(dev->client, 0x2A, (u8*)&mousecode, 2);

#ifdef CONFIG_INPUT_MODULE
  switch(mousecode&0x0F) {
  case 0: { /* right */
    input_report_rel(&input_irkbd, REL_X,  ((mousecode&0xFF)>>4)+1);
    mouse_directions=0;
    break;
  }
  case 1: { /* right up */
    input_report_rel(&input_irkbd, REL_X,  ((mousecode&0xFF)>>4)+1);
    if(mouse_directions==DIR_RIGHT_UP) {
      input_report_rel(&input_irkbd, REL_Y, -((mousecode&0xFF)>>4)-1);
      mouse_directions=0;
    }
    else
      mouse_directions=DIR_RIGHT_UP;
    break;
  }
  case 2: { /* right+up */
    input_report_rel(&input_irkbd, REL_X,  ((mousecode&0xFF)>>4)+1);
    input_report_rel(&input_irkbd, REL_Y, -((mousecode&0xFF)>>4)-1);
    mouse_directions=0;
    break;
  }
  case 3: { /* up right */
    input_report_rel(&input_irkbd, REL_Y, -((mousecode&0xFF)>>4)-1);
    if(mouse_directions==DIR_UP_RIGHT) {
      input_report_rel(&input_irkbd, REL_X,  ((mousecode&0xFF)>>4)+1);
      mouse_directions=0;
    }
    else
      mouse_directions=DIR_UP_RIGHT;
    break;
  }
  case 4: { /* up */
    input_report_rel(&input_irkbd, REL_Y, -((mousecode&0xFF)>>4)-1);
    mouse_directions=0;
    break;
  }
  case 5: { /* up left */
    input_report_rel(&input_irkbd, REL_Y, -((mousecode&0xFF)>>4)-1);
    if(mouse_directions==DIR_UP_LEFT) {
      input_report_rel(&input_irkbd, REL_X, -((mousecode&0xFF)>>4)-1);
      mouse_directions=0;
    }
    else
      mouse_directions=DIR_UP_LEFT;
    break;
  }
  case 6: { /* up+left */
    input_report_rel(&input_irkbd, REL_X, -((mousecode&0xFF)>>4)-1);
    input_report_rel(&input_irkbd, REL_Y, -((mousecode&0xFF)>>4)-1);
    mouse_directions=0;
    break;
  }
  case 7: { /* left up */
    input_report_rel(&input_irkbd, REL_X, -((mousecode&0xFF)>>4)-1);
    if(mouse_directions==DIR_LEFT_UP) {
      input_report_rel(&input_irkbd, REL_Y, -((mousecode&0xFF)>>4)-1);
      mouse_directions=0;
    }
    else
      mouse_directions=DIR_LEFT_UP;
    break;
  }
  case 8: { /* left */
    input_report_rel(&input_irkbd, REL_X, -((mousecode&0xFF)>>4)-1);
    mouse_directions=0;
    break;
  }
  case 9: { /* left down */
    input_report_rel(&input_irkbd, REL_X, -((mousecode&0xFF)>>4)-1);
    if(mouse_directions==DIR_LEFT_DOWN) {
      input_report_rel(&input_irkbd, REL_Y,  ((mousecode&0xFF)>>4)+1);
      mouse_directions=0;
    }
    else
      mouse_directions=DIR_LEFT_DOWN;
    break;
  }
  case 0x0A: { /* left+down */
    input_report_rel(&input_irkbd, REL_X, -((mousecode&0xFF)>>4)-1);
    input_report_rel(&input_irkbd, REL_Y,  ((mousecode&0xFF)>>4)+1);
    mouse_directions=0;
    break;
  }
  case 0x0B: { /* down left */
    input_report_rel(&input_irkbd, REL_Y,  ((mousecode&0xFF)>>4)+1);
    if(mouse_directions==DIR_DOWN_LEFT) {
      input_report_rel(&input_irkbd, REL_X, -((mousecode&0xFF)>>4)-1);
      mouse_directions=0;
    }
    else
      mouse_directions=DIR_DOWN_LEFT;
    break;
  }
  case 0x0C: { /* down */
    input_report_rel(&input_irkbd, REL_Y,  ((mousecode&0xFF)>>4)+1);
    mouse_directions=0;
    break;
  }
  case 0x0D: { /* down right */
    input_report_rel(&input_irkbd, REL_Y,  ((mousecode&0xFF)>>4)+1);
    if(mouse_directions==DIR_DOWN_RIGHT) {
      input_report_rel(&input_irkbd, REL_X,  ((mousecode&0xFF)>>4)+1);
      mouse_directions=0;
    }
    else
      mouse_directions=DIR_DOWN_RIGHT;
    break;
  }
  case 0x0E: { /* down+right */
    input_report_rel(&input_irkbd, REL_X,  ((mousecode&0xFF)>>4)+1);
    input_report_rel(&input_irkbd, REL_Y,  ((mousecode&0xFF)>>4)+1);
    mouse_directions=0;
    break;
  }
  case 0x0F: { /* right down */
    input_report_rel(&input_irkbd, REL_X,  ((mousecode&0xFF)>>4)+1);
    if(mouse_directions==DIR_RIGHT_DOWN) {
      input_report_rel(&input_irkbd, REL_Y,  ((mousecode&0xFF)>>4)+1);
      mouse_directions=0;
    }
    else
      mouse_directions=DIR_RIGHT_DOWN;
    break;
  }
  }
#endif
}

static void fp_handle_keyboard(struct fp_data *dev)
{
	u16 scancode=-1;
	unsigned char keycode=0;

	if (info.fpREV>=0x80)
	 fp_cmd(dev->client, 3, (u8*)&scancode, 2);
	else
	 fp_cmd(dev->client, 0x28, (u8*)&scancode, 2);
	//	printk("keyboard scancode: %02x\n", scancode);

#ifdef CONFIG_INPUT_MODULE
	/* mouse buttons */
	if(((scancode&0x7f)==0x7e)||((scancode&0x7f)==0x7f)) {
	  if((scancode&0x7f)==0x7e)
	    input_report_key(&input_irkbd,BTN_RIGHT,!(scancode&0x80));
	  else
	    input_report_key(&input_irkbd,BTN_LEFT,!(scancode&0x80));
	  return;
	}
#endif
	handle_scancode(scancode&0xFF, !((scancode&0xFF) & 0x80));
	/* no inputdev yet */
	return;

	if((scancode&0x7f)==0x49) {  /* Fn toggled */
	  if(scancode==0x49)
	    irkbd_flags|=IRKBD_FN;
	  else
	    irkbd_flags&=~IRKBD_FN;
	  return;
	}
	if(irkbd_flags&IRKBD_FN) {
	  keycode=keymap[(scancode&0x7f)|IRKBD_FN];
	  if(scancode&0x80)		   /* Fn pressed, other key released */
	    if(!fn_flags[scancode&0x7f])       /* fn pressed, other key released which got pressed before fn */
	      keycode=keymap[scancode&0x7f];
	    else			      /* fn pressed, other key released which got pressed during fn */
	      fn_flags[scancode&0x7f]=0;
	  else				/* Fn + other key pressed */
	    fn_flags[scancode&0x7f]=1;
	}
	/* key got pressed during fn and gets now released after fn has been released*/
	else if((!(irkbd_flags&IRKBD_FN)) && (fn_flags[scancode&0x7f])) {
	  keycode=keymap[(scancode&0x7f)|IRKBD_FN];
	  fn_flags[scancode&0x7f]=0;
	}
	/* no Fn - other key pressed/released */
	else
	  keycode=keymap[scancode&0x7f];

	if(keycode==0) {
	  dprintk("fp.o: irkbd: unknown scancode 0x%02X (flags 0x%02X)\n",scancode,irkbd_flags);
	}
#ifdef CONFIG_INPUT_MODULE
	else
	  input_report_key(&input_irkbd,keycode,!(scancode&0x80));
#endif
}


/* ------------------------------------------------------------------------- */

static void fp_check_queues(void)
{
	u8 status;
	int iwork=0;

	dprintk("fp.o: checking queues.\n");
	fp_cmd(defdata->client, 0x23, &status, 1);

	if(defdata->fpVCR!=status)
		fp_handle_vcr(defdata,status);

	iwork=0;
/*
fp status:

1       new rc
2       keyboard
4       mouse
8       RC
10      button
20      scart status
40      lnb alarm
80      timer underrun
*/
	do
	{
		fp_cmd(defdata->client, 0x20, &status, 1);
//		printk("status: %02x\n", status);

		/* remote control */
		if (status&9)
		{
			if (info.fpREV>=0x80)
				fp_handle_rc(defdata);
			else
				fp_handle_new_rc(defdata);
		}

		/* front button */
		if (status&0x10)
		{
			fp_handle_button(defdata);
		}

		if (status&0x2)
		{
			fp_handle_keyboard(defdata);
		}

		if(status&0x4)
		  fp_handle_mouse(defdata);

		/* ??? */
		if (status&0x40)		// LNB alarm
		{
			fp_handle_unknown(defdata);
		}

		/* if (status&0x20)  // scart status
		{
		} */

		if (iwork++ > 100)
		{
			dprintk("fp.o: Too much work at interrupt.\n");
			break;
		}

	} while (status & 0x5F);	    // only the ones we can handle

	if (status)
		dprintk("fp.o: unhandled interrupt source %x\n", status);

	return;
}

/* ------------------------------------------------------------------------- */

static void fp_task(void *arg)
{
	immap_t *immap=(immap_t*)IMAP_ADDR;

	fp_check_queues();

	if(useimap)
		immap->im_ioport.iop_padat&=~2;

	enable_irq(FP_INTERRUPT);
}

/* ------------------------------------------------------------------------- */

int fp_set_tuner_dword(int type, u32 tw)
{
	char msg[7]={0, 7, 0xC0};	/* default qam */
    int len=0;

	switch (type)
	{
		case T_QAM:
		{
			*((u32*)(msg+3))=tw;

			len = 7;

			dprintk("fp.o: fp_set_tuner_dword: QAM %08x\n", tw);

			break;
		}

		case T_QPSK:
		{
			*((u32*)(msg+2))=tw;
			msg[1] = 5;
			len = 6;

			dprintk("fp.o: fp_set_tuner_dword: QPSK %08x\n", tw);

			break;
		}

		default:
			break;
	}

	if(len)
	{
		if (i2c_master_send(defdata->client, msg, len)!=len)
		{
			return -1;
		}
	}

	return -1;
}

/* ------------------------------------------------------------------------- */

int fp_sec_status(void) {
  // < 0 means error: -1 for bus overload, -2 for busy
  return sec_bus_status;
}

int fp_send_diseqc(int style, u8 *cmd, unsigned int len)
{
	unsigned char msg[SEC_MAX_DISEQC_PARAMS+2+3]={0, 0};
	unsigned char status_cmd;
//	unsigned char sagem_send[1]={0x22};
	int c,sleep_perbyte,sleeptime;

	if (sec_bus_status == -1)
	  return -1;

	switch(style) {
	case 1: // NOKIA
		msg[1]=0x1B;
		sleeptime=2300;
		sleep_perbyte=300;
		status_cmd=0x2D;
	break;
	case 2: // SAGEM / PHILLIPS?
		msg[1]=0x25; //28

	/* this values are measured/calculated for nokia
	   dunno wether sagem needs longer or not */
		sleeptime=2300;
		sleep_perbyte=300;

		status_cmd=0x22;
	break;
	default:
		return -1;
	}

	memcpy(msg+2,cmd,len);

	dprintk("DiSEqC sent:");
	for(c=0;c<len;c++) {
	  dprintk(" %02X",msg[2+c]);
	}
	dprintk("\n");

	if(style==2 && len>1)
	{
		i2c_master_send(defdata->client, msg, 2+len);
		udelay(1000*100);																 // <- ;)
		return 0;
	}

	if(style==2) return 0;

	sec_bus_status=-2;
	i2c_master_send(defdata->client, msg, 2+len);

	current->state = TASK_INTERRUPTIBLE;
	schedule_timeout((sleeptime+(len * sleep_perbyte))/HZ);

	for (c=1;c<=5;c++) {
	  fp_cmd(defdata->client, status_cmd, msg, 1);
	  if ( !msg[0] )
	    break;
	  current->state = TASK_INTERRUPTIBLE;
	  schedule_timeout(sleep_perbyte/HZ);
	}

	if (c==5) {
	  dprintk("fp.o: DiSEqC TIMEOUT (could have worked anyway)\n");
	}
	else {
	  dprintk("fp.o: DiSEqC sent after %d poll(s)\n", c);
	}

	sec_bus_status=0;

	if (c==5)
	  return -1;
	else
	  return 0;
}

/* ------------------------------------------------------------------------- */
int fp_sagem_set_SECpower(int power,int tone)
{
   char msg[2]={0x4,0x71};

   if (power > 0) {
     if (power == 1)      // 13V
       msg[1]=0x50;
     else if (power == 2) // 14V
       msg[1]=0x50;
     else if (power == 3) // 18V
       msg[1]=0x60;
   }

	 if(tone) msg[1]|=0x1;


   dprintk("fp.o: fp_set_SECpower: %02X\n", msg[1]);
   sec_bus_status=-1;
   if (i2c_master_send(defdata->client, msg, 2)!=2)
     {
       return -1;
     }
   sec_bus_status=0;

   return 0;
}
/* ------------------------------------------------------------------------- */

int fp_set_sec(int power,int tone)
{
  char msg[2]={0x21, 0};

  if ((sec_bus_status == -1) && (power > 0))
    printk("restoring power to SEC bus\n");

  if (power > 0) { // bus power off/on
    msg[1]|=0x40;
    if (power == 1) // 13V
      msg[1]|=0x30;
    else if (power == 2) // 14V
      msg[1]|=0x20;
    else if (power == 3) // 18V
      msg[1]|=0x10;
    // otherwise 19V
    if(tone >0)
      msg[1]|=0x01;
  }
  else if (power < 0)
    msg[1]|=0x50; // activate loop-through - untested

  dprintk("fp.o: fp_set_sec: %02X\n", msg[1]);
  sec_bus_status=-1;
  if (i2c_master_send(defdata->client, msg, 2)!=2)
    {
      return -1;
    }
  sec_bus_status=0;
  return 0;
}


static int fp_set_wakeup_timer(int minutes)
{
	u8 cmd[3]={0x00, minutes&0xFF, minutes>>8};
	if (info.fpREV<0x80)
	{
		cmd[0]=FP_WAKEUP_SAGEM;
	} else
	{
		cmd[0]=FP_WAKEUP;
	}
		if (i2c_master_send(defdata->client, cmd, 3)!=3)
			return -1;

		return 0;
}


static int fp_get_wakeup_timer()
{
	u8 id[2]={0, 0};
	if (info.fpREV<0x80)
	{
		if (fp_cmd(defdata->client, FP_WAKEUP_SAGEM, id, 2))
			return -1;
	} else
	{

		if (fp_cmd(defdata->client, FP_WAKEUP, id, 2))
			return -1;
	}
	return id[0]+id[1]*256;
}

static int fp_clear_wakeup_timer()
{
	u8 id[1]={0};
	fp_set_wakeup_timer(0);
	if (fp_cmd(defdata->client, FP_STATUS, id, 1))
		return -1;
	dprintk("fp.o: clear_wakeup_timer [%x]\n",id[0]);
	if(id[0] & 0x80)
		is_wakeup=1;
	else
		is_wakeup=0;
	if (info.fpREV<0x80)
	{
		if (fp_cmd(defdata->client, FP_CLEAR_WAKEUP, id, 1))
			return -1;
	} else
	{
		if (fp_cmd(defdata->client, FP_CLEAR_WAKEUP_NOKIA, id, 1))
			return -1;
	}
	return 0;
}

/* ------------------------------------------------------------------------- */

EXPORT_SYMBOL(fp_set_tuner_dword);
EXPORT_SYMBOL(fp_set_sec);
EXPORT_SYMBOL(fp_do_reset);
EXPORT_SYMBOL(fp_cam_reset);
EXPORT_SYMBOL(fp_send_diseqc);
EXPORT_SYMBOL(fp_sec_status);
EXPORT_SYMBOL(fp_sagem_set_SECpower);

/* ------------------------------------------------------------------------- */

static void fp_restart(char *cmd)
{
	if (info.fpREV>=0x80)
		fp_sendcmd(defdata->client, 0, 20); // nokia
	else
		fp_sendcmd(defdata->client, 0, 9);  // sagem/philips
	for (;;);
}

/* ------------------------------------------------------------------------- */

static void fp_power_off(void)
{
	if (info.fpREV>=0x80)
		fp_sendcmd(defdata->client, 0, 3);
	else
		fp_sendcmd(defdata->client, 0, 0);
	for (;;);
}

/* ------------------------------------------------------------------------- */

static void fp_halt(void)
{
	fp_power_off();
}

/* ------------------------------------------------------------------------- */

#ifdef MODULE
MODULE_AUTHOR("Felix Domke <tmbinc@gmx.net>");
MODULE_DESCRIPTION("DBox2 Frontprocessor");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

MODULE_PARM(debug,"i");
MODULE_PARM(useimap,"i");

int init_module(void)
{
	return fp_init();
}

/* ------------------------------------------------------------------------- */

void cleanup_module(void)
{
	fp_close();
}
#endif

/* ------------------------------------------------------------------------- */


