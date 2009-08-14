/*
 *   lcd-ks0713.c - lcd driver for KS0713 and compatible (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2002 Gillem (gillem@berlios.de)
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
 *   $Log: lcd-ks0713.c,v $
 *   Revision 1.22  2002/09/23 17:07:02  obi
 *   moved lcd-ks0713.h to include/dbox
 *
 *   Revision 1.21  2002/05/06 02:18:19  obi
 *   cleanup for new kernel
 *
 *   Revision 1.20  2002/03/21 19:26:31  obi
 *   compilable with kernels >= 2.4.10
 *
 *   Revision 1.19  2002/03/03 09:32:33  gillem
 *   - add lcd defines
 *   - boxtype output
 *
 *   Revision 1.18  2002/03/03 07:11:48  gillem
 *   - add lcd interface detect
 *
 *   Revision 1.17  2002/02/24 19:19:25  obi
 *   reverted to previous revision - is not related to tuning api
 *
 *   Revision 1.15  2002/01/23 19:03:25  gillem
 *   - fix lcd init
 *
 *   Revision 1.14  2001/12/01 06:53:17  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.13  2001/11/25 21:37:46  gillem
 *   - fix adc
 *   - add new ioctl (LCD_IOCTL_INIT)
 *   - remove init from reset
 *
 *   Revision 1.12  2001/11/25 21:11:39  gillem
 *   - update reset function (test only!)
 *   - add sirc
 *
 *   Revision 1.11  2001/06/03 20:45:50  kwon
 *   indent
 *
 *   Revision 1.10  2001/03/03 18:00:28  waldi
 *   complete change to devfs; doesn't compile without devfs
 *
 *   Revision 1.9  2001/01/28 19:47:12  gillem
 *   - fix setpos ...
 *
 *   Revision 1.8  2001/01/28 18:49:08  gillem
 *   add ioctl
 *   LCD_IOCTL_CLEAR
 *   LCD_IOCTL_SET_POS
 *   LCD_IOCTL_GET_POS
 *
 *   Revision 1.7  2001/01/26 23:51:33  gillem
 *   some kernel styles change
 *
 *   Revision 1.6  2001/01/20 19:01:21  gillem
 *   - add pixel function
 *
 *   Revision 1.5  2001/01/06 10:06:35  gillem
 *   cvs check
 *
 *   $Revision: 1.22 $
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/smp_lock.h>
#include <linux/delay.h>

#include <linux/init.h>

/* ppc stuff */
#include <asm/8xx_immap.h>
#include <asm/pgtable.h>
#include <asm/mpc8xx.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

/* HACK HACK HACK */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,10)
#include <asm/commproc.h>
#else
#include <commproc.h>
#endif

#include <dbox/info.h>
#include <dbox/lcd-ks0713.h>
#include "lcd-console.h"

#include <linux/devfs_fs_kernel.h>

#ifndef CONFIG_DEVFS_FS
#error no devfs
#endif

static devfs_handle_t devfs_handle;

/* lcd interface id */
#define KS0713		0x00
#define SED153X		0x00
#define SSD181X		0x0a

///////////////////////////////////////////////////////////////////////////////

#define CFG_IMMR IMAP_ADDR

volatile iop8xx_t * iop;

#ifdef MODULE
extern int init_module(void);
extern int cleanup_module(void);
#endif /* def MODULE */

static ssize_t lcd_read (struct file *file, char *buf, size_t count,
			    loff_t *offset);
static ssize_t lcd_write (struct file *file, const char *buf, size_t count,
			     loff_t *offset);

static loff_t lcd_seek (struct file *file, loff_t, int );

static int lcd_ioctl (struct inode *inode, struct file *file,
			 unsigned int cmd, unsigned long arg);
static int lcd_open (struct inode *inode, struct file *file);

#ifdef MODULE
static
#else
extern
#endif
int __init lcd_init(void);

static int lcd_cleanup(void);
void lcd_reset_init(void);

///////////////////////////////////////////////////////////////////////////////

static struct file_operations lcd_fops = {
	owner:		THIS_MODULE,
	read:		lcd_read,
	write:		lcd_write,
	ioctl:		lcd_ioctl,
	open:		lcd_open,
	llseek:		lcd_seek,
};

static struct file_vars f_vars;

//static int lcd_initialized;

static int LCD_MODE;

///////////////////////////////////////////////////////////////////////////////

/* Front panel LCD display major */
#define LCD_MAJOR				156
#define LCD_DELAY				1

///////////////////////////////////////////////////////////////////////////////

/* internal functions ... first read the manual ! (not included) */

/* Read display data 11XXXXXXXX
 */
#define LCD_READ_DATA			0x0600

/* Write display data 10XXXXXXXX
 */
#define LCD_WRITE_DATA		0x0400

/* Read Status 01BAOR0000
 * B BUSY
 * A ADC
 * O ON/OFF
 * R RESETB
 */
#define LCD_READ_STATUS		0x0200

/* Display ON/OFF	001010111O
 * O 0=OFF 1=ON
 */
#define LCD_CMD_ON				0x00AE

/* Initial display line	0001FEDCBA
 * F-A ST5-0
 * Specify DDRAM line for COM1
 */
#define LCD_CMD_IDL				0x0040

/* Set reference voltage mode 0010000001
 */
#define LCD_CMD_SRV				0x0081

/* Set page address	001011DCBA
 * D-A P3-P1
 */
#define LCD_CMD_SPAGE			0x00B0

/* Set column address 000001DCBA
 * D-A  Y7-4
 * next Y3-0
 */
#define LCD_CMD_COL				0x0010

/* ADC select 001010011A
 * A 0=normal direction 1=reverse
 */
#define LCD_CMD_ADC				0x00A0

/* Reverse display ON/OFF 001010011R
 * R 0=normal 1=reverse
 */
#define	LCD_CMD_REVERSE		0x00A6

/* Entire display ON/OFF 001010010E
 * E 0=normal 1=entire
 */
#define LCD_CMD_EON				0x00A4

/* LCD bias select 001010001B
 * B Bias
 */
#define LCD_CMD_BIAS			0x00A2

/* Set modify-read 0011100000
 */
#define LCD_CMD_SMR				0x00E0

/* Reset modify-read 0011101110
 */
#define LCD_CMD_RMR				0x00EE

/* Reset 0011100010
 * Initialize the internal functions
 */
#define LCD_CMD_RESET			0x00E2

/* SHL select 001100SXXX
 * S 0=normal 1=reverse
 */
#define LCD_CMD_SHL				0x00C0

/* Power control 0000101CRF
 * control power circuite operation
 */
#define LCD_CMD_POWERC		0x0028

/* Regulator resistor select 0000100CBA
 * select internal resistor ration
 */
#define LCD_CMD_RES				0x0020 // Regulator resistor select

/* Set static indicator 001010110S
 * S 0=off 1=on
 * next 00XXXXXXBC
 */
#define LCD_CMD_SIR				0x00AC

/* clocks */
#define LCD_CLK_LO			0x0000
#define LCD_CLK_HI			0x0800
#define LCD_CMD_LO			0x0800
#define LCD_CMD_HI			0x0B00

/* direction mode */
#define LCD_DIR_READ		0x0F00
#define LCD_DIR_WRITE		0x0FFF

///////////////////////////////////////////////////////////////////////////////
/* set direction to read */

static void lcd_set_port_read(void)
{
	iop->iop_pddir = LCD_DIR_READ;
}

///////////////////////////////////////////////////////////////////////////////
/* set direction to write */

static void lcd_set_port_write(void)
{
	iop->iop_pddat = 0x0B00;
	iop->iop_pddir = LCD_DIR_WRITE;
	udelay(LCD_DELAY);
}

///////////////////////////////////////////////////////////////////////////////
/* send cmd */

static void lcd_send_cmd( int cmd, int flag )
{
		lcd_set_port_write();

		iop->iop_pddat = cmd | flag | LCD_CMD_LO;
		udelay(LCD_DELAY);
		iop->iop_pddat = cmd | flag | LCD_CMD_HI;
		udelay(LCD_DELAY);
}

///////////////////////////////////////////////////////////////////////////////
/* read status of display */

static int lcd_read_status(void)
{
	int status;

	lcd_set_port_read();

	iop->iop_pddat = LCD_READ_STATUS | LCD_CLK_LO;
	udelay(LCD_DELAY);
	iop->iop_pddat = LCD_READ_STATUS | LCD_CLK_HI;
	udelay(LCD_DELAY);

	status = (iop->iop_pddat & 0xF0);

	return status;
}

///////////////////////////////////////////////////////////////////////////////
/* read function */

static void lcd_read_dummy(void)
{
	lcd_set_port_read();

	// write data
	iop->iop_pddat = LCD_READ_DATA | LCD_CLK_LO;
	udelay(LCD_DELAY);
	iop->iop_pddat = LCD_READ_DATA | LCD_CLK_HI;
	udelay(LCD_DELAY);
}

///////////////////////////////////////////////////////////////////////////////
/* read byte */

static int lcd_read_byte(void)
{
	int data;

	lcd_read_dummy();

	data = (iop->iop_pddat & 0xFF);

	return data;
}

///////////////////////////////////////////////////////////////////////////////
/* write byte */

void lcd_write_byte( int data )
{
	lcd_set_port_write();

	// write data
	iop->iop_pddat = LCD_WRITE_DATA | (data&0xFF) | LCD_CLK_LO;
	udelay(LCD_DELAY);
	iop->iop_pddat = LCD_WRITE_DATA | (data&0xFF) | LCD_CLK_HI;
	udelay(LCD_DELAY);
	iop->iop_pddat = LCD_WRITE_DATA | (data&0xFF) | LCD_CLK_LO;
}

///////////////////////////////////////////////////////////////////////////////

int lcd_set_pixel( struct lcd_pixel * pix )
{
    int y=0,val,bit=0,v;

    if ( pix->y >= (LCD_ROWS*8) ) {
	return -EINVAL;
    }

    if ( pix->x >= LCD_COLS ) {
	return -EINVAL;
    }

    if (pix->y) {
	y = (pix->y/8);
    }

    bit = pix->y - (y*8);

	// set dram pointer
	lcd_set_pos( y, pix->x );
	lcd_read_dummy();

    val = lcd_read_byte();

    v = pix->v;

    // invertieren
    if ( v == 2 ) {
	v = (((~val)>>bit)&1);
    }

    if ( v == 1 ) {
	val |= (1<<bit);
    } else {
	val &= ~(1<<bit);
    }

	// set dram pointer
	lcd_set_pos( y, pix->x );
    lcd_write_byte(val);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void lcd_read_dram( unsigned char * dest )
{
	int pa,col;

	for(pa=0;pa<LCD_ROWS;pa++) {
		// set dram pointer
		lcd_set_pos( pa, 0 );

		lcd_read_dummy();

		for(col=0;col<LCD_COLS;col++) {
			dest[(pa*LCD_COLS)+col] = lcd_read_byte();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

void lcd_write_dram( unsigned char * source )
{
	int pa,col;

	for(pa=0;pa<LCD_ROWS;pa++) {

		lcd_set_pos( pa, 0 );

		for(col=0;col<LCD_COLS;col++) {
			lcd_write_byte( source[(pa*LCD_COLS)+col] );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////

static ssize_t lcd_read (struct file *file, char *buf, size_t count,
			    loff_t *offset)
{
	char *obp, *bp;
	int pa,col,i;
    int ret;

	if (count>LCD_BUFFER_SIZE) {
		return -EFAULT;
    }

	if ( lcd_read_status() & LCD_STAT_BUSY ) {
		return -EBUSY;
    }

	if ( (obp = kmalloc( count, GFP_KERNEL)) == NULL ) {
		return -ENOMEM;
    }

	bp = obp;

	i = count;

	/* calculate row,col */
	if (f_vars.pos>LCD_COLS) {
		pa =f_vars.pos/LCD_COLS;
		col=f_vars.pos-(f_vars.pos*LCD_COLS);
	} else {
		pa  = 0;
		col = f_vars.pos;
	}

	for(/*pa=0*/;(pa<LCD_ROWS) && i;pa++) {

		// set dram pointer
		lcd_set_pos( pa, col );

		lcd_read_dummy();

		for(/*col=0*/;(col<LCD_COLS) && i;col++,bp++,i--) {

			*bp = lcd_read_byte();
		}

		col = 0;
	}

	ret = copy_to_user( buf, obp,count) ?-EFAULT:(bp-obp);

	kfree(obp);

	return ret;
}

///////////////////////////////////////////////////////////////////////////////

static ssize_t lcd_write (struct file *file, const char *buf, size_t count,
			     loff_t *offset)
{
	char *obp, *bp;
	int pa,col;

	if (count<=0) {
		return -EFAULT;
    }

	if ( lcd_read_status() & LCD_STAT_BUSY ) {
		return -EBUSY;
    }

	if ( (obp = kmalloc( count, GFP_KERNEL)) == NULL ) {
		return -ENOMEM;
    }

	if ( copy_from_user( obp, buf, count ) ) {
		kfree(obp);
		return -EFAULT;
	}

	bp = obp;

	if ( LCD_MODE == LCD_MODE_BIN ) {
		if ( (count!=LCD_BUFFER_SIZE) ) {
			kfree(obp);
			return -EFAULT;
		}

		for(pa=0;pa<LCD_ROWS;pa++) {
			// set dram pointer
			lcd_set_pos( pa, 0 );

			for(col=0;col<LCD_COLS;col++,bp++) {
				lcd_write_byte( *bp );
			}
		}
	} else if ( LCD_MODE == LCD_MODE_ASC ) {
		lcd_console_put_data( bp, count );
	} else {
		kfree(obp);
		return -EFAULT;
	}

	kfree(obp);

	return count;
}

///////////////////////////////////////////////////////////////////////////////
/* today not supported */

static loff_t lcd_seek (struct file *file, loff_t offset, int origin)
{
//	printk("[LCD]: OFFSET = %d\n",offset);

	switch ( origin ) {

		case 0:
					/* nothing to do */
					printk("[LCD]: ORIGIN = 0\n");
					break;
		case 1:
					offset += f_vars.pos;
					printk("[LCD]: ORIGIN = 1 : %d\n", f_vars.pos);
					break;
		case 2:
					offset = LCD_BUFFER_SIZE - offset;
					printk("[LCD]: ORIGIN = 2 : %d\n", f_vars.pos);
					break;
		default:
					printk("[LCD]: seek unknown : %d\n", origin);
					break;
	}

//	printk("[LCD]: OFFSET = %d\n",offset);

	return ( (offset >=0) ? (f_vars.pos=offset) : -EINVAL );
}

///////////////////////////////////////////////////////////////////////////////

void lcd_set_pos( int row, int col )
{
//todo: save io
	f_vars.pos = row*col;

//	if ( (f_vars.row != row) && (row>=0) ) {
//    }

    f_vars.row = row;
	lcd_send_cmd( LCD_CMD_SPAGE, row );

//	if ( (f_vars.col != col) && (col>=0) ) {
//    }

    f_vars.col = col;
    lcd_send_cmd( LCD_CMD_COL, (col>>4)&0x0F );
	lcd_send_cmd( 0x00, col&0x0F );
}

///////////////////////////////////////////////////////////////////////////////

int lcd_ioctl (struct inode *inode, struct file *file, unsigned int cmd,
		  unsigned long arg)
{
	int val;
    struct lcd_pixel pix;
    struct lcd_pos pos;

	switch (cmd) {

	case LCD_IOCTL_STATUS:

				val = lcd_read_status();

			if ( copy_to_user((int*)arg,&val,sizeof(int)) )
				return -EFAULT;

				return 0;

		case LCD_IOCTL_RESET:

			lcd_send_cmd( LCD_CMD_RESET, 0 );
				return 0;

		case LCD_IOCTL_INIT:

				lcd_reset_init();
				return 0;
	}

	if ( lcd_read_status() & LCD_STAT_BUSY )
		return -EBUSY;

    if ( cmd & LCDSET ) {

	if ( copy_from_user( &val, (int*)arg, sizeof(int) ) )
		return -EFAULT;

	    switch (cmd) {

		case LCD_IOCTL_ON:

				lcd_send_cmd( LCD_CMD_ON, val&0x01 );
				break;

		case LCD_IOCTL_EON:
			lcd_send_cmd( LCD_CMD_EON, val&0x01 );
					break;

		case LCD_IOCTL_REVERSE:

				lcd_send_cmd( LCD_CMD_REVERSE, val&0x01 );
				break;

		case LCD_IOCTL_BIAS:

				lcd_send_cmd( LCD_CMD_BIAS, val&0x01 );
				break;

		case LCD_IOCTL_ADC:

				lcd_send_cmd( LCD_CMD_ADC, val&0x01 );
				break;

		case LCD_IOCTL_SHL:

				lcd_send_cmd( LCD_CMD_SHL, (val&0x01)<<3 );
				break;

		case LCD_IOCTL_IDL:

				if ( (val > 0x1F) || (val < 0) )
				return -EINVAL;

				lcd_send_cmd( LCD_CMD_IDL, val&0x1F );
				break;

		case LCD_IOCTL_SRV:

				if ( (val > 0x3F) || (val < 0) )
					return -EINVAL;

			lcd_send_cmd( LCD_CMD_SRV, 0 );
				lcd_send_cmd( 0x00, val&0x3F );
				break;

		case LCD_IOCTL_POWERC:

				if ( (val > 0x07) || (val < 0) )
					return -EINVAL;

			lcd_send_cmd( LCD_CMD_POWERC, val&0x07 );
				break;

		case LCD_IOCTL_SEL_RES:

			if ( (val > 0x07) || (val < 0) )
					return -EINVAL;

			lcd_send_cmd( LCD_CMD_RES, val&0x07 );
				break;

		case LCD_IOCTL_SIR:

				if ( (val > 0x03) || (val < 0) )
					return -EINVAL;

				lcd_send_cmd( LCD_CMD_SIR, 0x01 );
				lcd_send_cmd( 0x00, val&0x03 );
				break;

		case LCD_IOCTL_SIRC:

				if ( (val > 0x03) || (val < 0) )
					return -EINVAL;

				lcd_send_cmd( LCD_CMD_SIR, 0x00 );
				lcd_send_cmd( 0x00, val&0x03 );
				break;

		case LCD_IOCTL_SROW:
//		case LCD_IOCTL_SPAGE:

				if ( (val >= LCD_ROWS ) || (val < 0) )
					return -EINVAL;

		lcd_set_pos( val, -1 );

				break;

		case LCD_IOCTL_SCOLUMN:

				if ( (val >= LCD_COLS) || (val < 0) )
					return -EINVAL;

		lcd_set_pos( -1, val );

				break;

		case LCD_IOCTL_WRITE_BYTE:

				lcd_write_byte( val&0xFF );
				break;

		// set ASC (0)(default) or BIN (1) mode for read/write
		case LCD_IOCTL_ASC_MODE:

				LCD_MODE = val&0x01;
				break;
	    }
    } else if (cmd & LCDGET ) {

	switch (cmd) {

		case LCD_IOCTL_READ_BYTE:

			val = lcd_read_byte();

		if ( copy_to_user( &val, (int*)arg, sizeof(int) ) )
			return -EFAULT;

			break;
	}
    } else {

	switch (cmd) {

		case LCD_IOCTL_SMR:

				lcd_send_cmd( LCD_CMD_SMR, 0 );
				break;

		case LCD_IOCTL_RMR:

				lcd_send_cmd( LCD_CMD_RMR, 0 );
				break;

		case LCD_IOCTL_SET_ADDR:

			lcd_read_dummy();
				break;

		case LCD_IOCTL_SET_PIXEL:

			if ( copy_from_user(&pix,(struct lcd_pixel*)arg,sizeof(struct lcd_pixel)) ) {
			return -EFAULT;
		}

		return lcd_set_pixel(&pix);

		case  LCD_IOCTL_GET_POS:

		pos.row = f_vars.row;
		pos.col = f_vars.col;

		if ( copy_to_user( &pos, (lcd_pos*)arg, sizeof(struct lcd_pos) ) ) {
			return -EFAULT;
		}

		break;

		case  LCD_IOCTL_SET_POS:

			if ( copy_from_user(&pos,(struct lcd_pos*)arg,sizeof(struct lcd_pos)) ) {
			return -EFAULT;
		}

		if ( pos.row > LCD_ROWS ) {
		    return -EINVAL;
		}

		if ( pos.col > LCD_COLS ) {
		    return -EINVAL;
		}

		lcd_set_pos( pos.row, pos.col );
		break;

	    case LCD_IOCTL_CLEAR:
		lcd_clear();
		break;
	}
    }

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int lcd_open (struct inode *inode, struct file *file)
{
	/* init lcd vars */
	f_vars.pos = 0;
	f_vars.row = 0;
	f_vars.col = 0;

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void lcd_clear(void)
{
	static unsigned char d[LCD_BUFFER_SIZE];

	f_vars.row = 0;
	f_vars.col = 0;

	memset(d,0x00,LCD_BUFFER_SIZE);
	lcd_write_dram(d);
}

///////////////////////////////////////////////////////////////////////////////

void lcd_reset_init(void)
{
	// i hope it works now
	lcd_send_cmd( LCD_CMD_RESET, 0 );

	udelay(1000*100);

	lcd_send_cmd( LCD_CMD_ON, 1 );
	lcd_send_cmd( LCD_CMD_EON, 0 );
	lcd_send_cmd( LCD_CMD_REVERSE, 0 );
	lcd_send_cmd( LCD_CMD_BIAS, 1 );
	lcd_send_cmd( LCD_CMD_ADC, 0 );
	lcd_send_cmd( LCD_CMD_SHL, 0 );
	lcd_send_cmd( LCD_CMD_POWERC, 7 );
	lcd_send_cmd( LCD_CMD_RES, 7 );
	lcd_send_cmd( LCD_CMD_SIR, 3 );
	lcd_send_cmd( LCD_CMD_IDL, 0 );
	lcd_send_cmd( LCD_CMD_SRV, 1 );
	lcd_send_cmd( 0x00, 15 );
}

///////////////////////////////////////////////////////////////////////////////

int __init lcd_init(void)
{
	int status;
	struct dbox_info_struct dinfo;

	immap_t	*immap;

	printk("lcd.o: init lcd driver module\n");

//	lcd_initialized = 0;
//	if (register_chrdev(LCD_MAJOR,"lcd",&lcd_fops)) {
//		printk("lcd.o: unable to get major %d\n", LCD_MAJOR);
//		return -EIO;
//	}

	devfs_handle =
		devfs_register ( NULL, "dbox/lcd0", DEVFS_FL_DEFAULT, 0, 0,
		     S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
		     &lcd_fops, NULL );

	if ( ! devfs_handle )
	{
		return -EIO;
	}

//  lcd_initialized ++;

	if ( ( immap = ( immap_t * ) CFG_IMMR ) == NULL )
	{
		devfs_unregister ( devfs_handle );
		return -EIO;
	}

	iop = (iop8xx_t *)&immap->im_ioport;

	/* init lcd vars */
	f_vars.pos = 0;
	f_vars.row = 0;
	f_vars.col = 0;

	/* reset lcd todo ;-) */
	//lcd_reset();

	// set defaults
	LCD_MODE = LCD_MODE_ASC;

	status = lcd_read_status();

	dbox_get_info(&dinfo);

	switch(status&0x0f)
	{
		case KS0713:
//		case SED153X:
			printk("lcd.o: found KS0713/SED153X lcd interface on %x\n",dinfo.mID);
			break;
		case SSD181X:
			printk("lcd.o: found SSD181X lcd interface on %x\n",dinfo.mID);
			break;
		default:
			printk("lcd.o: found unknown (%02X) lcd interface on %x\n",status&0x0f,dinfo.mID);
			break;
	}

	lcd_init_console();

	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int lcd_cleanup(void)
{
//	if (lcd_initialized >= 1) {
//		if ((res = unregister_chrdev(LCD_MAJOR,"lcd"))) {
//			printk("lcd.o: unable to release major %d\n", LCD_MAJOR );
//			return res;
//		}
//		lcd_initialized --;
//	}

  devfs_unregister ( devfs_handle );

  return 0;
}

///////////////////////////////////////////////////////////////////////////////

EXPORT_NO_SYMBOLS;

#ifdef MODULE

MODULE_AUTHOR("Gillem <gillem@berlios.de>");
MODULE_DESCRIPTION("LCD driver (KS0713)");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

int init_module(void)
{
	return lcd_init();
}

int cleanup_module(void)
{
	return lcd_cleanup();
}

#endif /* def MODULE */
