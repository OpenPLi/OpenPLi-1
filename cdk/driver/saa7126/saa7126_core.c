/*
 *   saa7126_core.c - pal driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Gillem (htoa@gmx.net)
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
 *   $Log: saa7126_core.c,v $
 *   Revision 1.26  2002/09/15 09:15:06  LazyT
 *   SAAIOGWSS works now and doesn't kill the box
 *
 *   Revision 1.25  2002/08/12 17:19:06  obi
 *   removed compiler warnings
 *
 *   Revision 1.24  2002/08/12 17:08:44  wjoost
 *   SAA_WSS_OFF hinzugefügt
 *
 *   Revision 1.23  2002/08/04 12:14:21  wjoost
 *   wide screen signaling
 *
 *   Revision 1.22  2002/06/12 16:28:51  LazyT
 *   fixed VBI insertion
 *
 *   Revision 1.21  2002/06/11 15:31:49  Jolt
 *   VBI stuff
 *
 *   Revision 1.20  2002/05/06 02:18:19  obi
 *   cleanup for new kernel
 *
 *   Revision 1.19  2002/03/06 12:54:52  gillem
 *   - fix include path
 *
 *   Revision 1.18  2002/03/06 09:36:33  gillem
 *   - clean module unload (set into standby)
 *
 *   Revision 1.17  2001/12/01 06:53:46  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.16  2001/11/22 20:19:32  gillem
 *   - simple bugfix
 *
 *   Revision 1.15  2001/11/22 17:26:12  gillem
 *   - add power save mode (experimental)
 *   - start vps
 *
 *   Revision 1.14  2001/07/03 20:23:11  gillem
 *   - some changes
 *
 *   Revision 1.13  2001/07/03 19:55:05  gillem
 *   - add ioctl to set rgb/fbas/svideo
 *   - remove module option svideo
 *   - add module option mode
 *
 *   Revision 1.12  2001/07/03 19:23:56  gillem
 *   - change flags
 *
 *   Revision 1.11  2001/06/11 15:58:45  gillem
 *   - some change in svideo stuff
 *
 *   Revision 1.10  2001/05/16 22:12:48  gillem
 *   - add encoder setting
 *
 *   Revision 1.9  2001/05/07 02:17:52  DerSchrauber
 *   Parameter svideo für saa7126 hinzugefügt. S-Video Ausgang jetzt möglich.
 *
 *   Revision 1.8  2001/04/07 01:45:34  tmbinc
 *   added philips-support.
 *
 *   Revision 1.7  2001/03/03 18:00:45  waldi
 *   complete change to devfs; doesn't compile without devfs
 *
 *   Revision 1.6  2001/02/11 21:29:12  gillem
 *   - add ioctl SAAIOSOUT (output control)
 *
 *   Revision 1.4  2001/02/08 18:53:53  gillem
 *   *** empty log message ***
 *
 *   Revision 1.3  2001/02/01 19:56:38  gillem
 *   - 2.4.1 support
 *
 *   Revision 1.2  2001/01/06 10:06:55  gillem
 *   cvs check
 *
 *   $Revision: 1.26 $
 *
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/pci.h>
#include <linux/signal.h>
#include <asm/io.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <linux/sched.h>
#include <asm/segment.h>
#include <linux/types.h>
#include <linux/wrapper.h>

#include <linux/proc_fs.h>

#include <linux/videodev.h>
#include <linux/version.h>
#include <asm/uaccess.h>

#include <linux/i2c.h>
#include <linux/video_encoder.h>
#include <dbox/info.h>
#include <dbox/saa7126_core.h>

#ifndef CONFIG_DEVFS_FS
#error no devfs
#endif

static devfs_handle_t devfs_handle;

#define DEBUG(x)   x		/* Debug driver */

/* ------------------------------------------------------------------------- */

#if 0
static unsigned char PAL_SAA_NOKIA[] =
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x21, 0x1D, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x1A, 0x1A,
/* COLORBAR=0x80 */
0x03,
0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
0x00, 0x00, 0x68, 0x7D, 0xAF, 0x33, 0x35, 0x35,
0x00, 0x06, 0x2F, 0xCB, 0x8A, 0x09, 0x2A, 0x00,
0x00, 0x00, 0x00,
/* 0x6B */
0x52,
0x28, 0x01, 0x20, 0x31,
0x7D, 0xBC, 0x60, 0x41, 0x05, 0x00, 0x06, 0x16,
0x06, 0x16, 0x16, 0x36, 0x60, 0x00, 0x00, 0x00
};
#endif

static unsigned char PAL_SAA_NOKIA_CONFIG[] =
{
0x00, 0x1b, 0x00, 0x22, 0x2b, 0x08, 0x74, 0x55,
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x22, 0x15, 0x60, 0x00, 0x07, 0x7e, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x3b, 0x72, 0x00, 0x00, 0x02, 0x54, 0x00, 0x00,
0x21, 0x1d, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
0x2c, 0x04, 0x00, 0xfe, 0x00, 0x00, 0x7e, 0x00,
0x1a, 0x1a, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
0x00, 0x00, 0x68, 0x7d, 0xaf, 0x33, 0x35, 0x35,
0x00, 0x06, 0x2f, 0xcb, 0x8a, 0x09, 0x2a, 0x00,
0x00, 0x00, 0x00, 0x52, 0x28, 0x01, 0x20, 0x31,
0x7d, 0xbc, 0x60, 0x41, 0x05, 0x00, 0x06, 0x16,
0x06, 0x16, 0x16, 0x36, 0x60, 0x00, 0x00, 0x00
};

static unsigned char PAL_SAA_PHILIPS[] =
{
0x00, 0x1B, 0x00, 0x22, 0x2B, 0x08, 0x74, 0x55,
0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x22, 0x15, 0x60, 0x00, 0x07, 0x7E, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x3B, 0x72, 0x00, 0x00, 0x02, 0x54, 0x00, 0x00,
0x22, 0x1E, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
0x2C, 0x04, 0x00, 0xFE, 0x00, 0x00, 0x7E, 0x00,
0x1C, 0x1C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x70, 0x78, 0xAB, 0x1B, 0x26, 0x26,
0x00, 0x16, 0x2E, 0xCB, 0x8A, 0x09, 0x2A, 0x00,
0x00, 0x00, 0x00, 0x72, 0x00, 0x00, 0xA0, 0x31,
0x7D, 0xBB, 0x60, 0x40, 0x07, 0x00, 0x06, 0x16,
0x06, 0x16, 0x16, 0x36, 0x60, 0x00, 0x00, 0x00
};

static unsigned char PAL_SAA_SAGEM[] =
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x21, 0x1D, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x1E, 0x1E, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0xF2, 0x90,
0x00, 0x00, 0x70, 0x75, 0xA5, 0x37, 0x39, 0x39,
0x00, 0x06, 0x2C, 0xCB, 0x8A, 0x09, 0x2A, 0x00,
0x00, 0x00, 0x00, 0x52, 0x6F, 0x00, 0xA0, 0x31,
0x7D, 0xBF, 0x60, 0x40, 0x07, 0x00, 0x06, 0x16,
0x06, 0x16, 0x16, 0x36, 0x60, 0x00, 0x00, 0x00
};

static const unsigned char wss_data[8] =
{
0x08, 0x01, 0x02, 0x0B, 0x04, 0x0D, 0x0E, 0x07
};

/* ------------------------------------------------------------------------- */

//static struct s_saa_data saa_data;	/* current settings */

/* ------------------------------------------------------------------------- */

/* Addresses to scan */
static unsigned short normal_i2c[]			= {I2C_CLIENT_END};
static unsigned short normal_i2c_range[]	= { 0x88>>1,0x88>>1,I2C_CLIENT_END};
static unsigned short probe[2]			= { I2C_CLIENT_END, I2C_CLIENT_END };
static unsigned short probe_range[2]		= { I2C_CLIENT_END, I2C_CLIENT_END };
static unsigned short ignore[2]		= { I2C_CLIENT_END, I2C_CLIENT_END };
static unsigned short ignore_range[2]		= { I2C_CLIENT_END, I2C_CLIENT_END };
static unsigned short force[2]			= { I2C_CLIENT_END, I2C_CLIENT_END };

static struct i2c_client_address_data addr_data = {
	normal_i2c, normal_i2c_range,
	probe, probe_range,
	ignore, ignore_range,
	force
};

/* ------------------------------------------------------------------------- */

static int saa7126_ioctl (struct inode *inode, struct file *file,
			 unsigned int cmd, unsigned long arg);
static int saa7126_open (struct inode *inode, struct file *file);

static int saa7126_cmd(struct i2c_client *client, u8 cmd, u8 *res, int size);
static int saa7126_sendcmd(struct i2c_client *client, u8 b0, u8 b1);

static int saa7126_mode( int inp );

static struct file_operations saa7126_fops = {
	owner:		THIS_MODULE,
	ioctl:		saa7126_ioctl,
	open:		saa7126_open,
};

static int saa7126_encoder( int inp );

/* ------------------------------------------------------------------------- */

static int debug =  0; /* insmod parameter */
static int addr  =  0;
static int board =	0;
static int mode  =	0;

static int saa_power_mode;
static u8 saa_power_reg_2d;
static u8 saa_power_reg_61;

#if LINUX_VERSION_CODE > 0x020100
#ifdef MODULE
MODULE_PARM(debug,"i");
MODULE_PARM(addr,"i");
MODULE_PARM(board,"i");
MODULE_PARM(mode,"i");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#endif
#endif

/* ------------------------------------------------------------------------- */

static int this_adap;

#define dprintk     if (debug) printk

#define I2C_DRIVERID_SAA7126	1
#define SAA7126_MAJOR			240
#define SAA_I2C_BLOCK_SIZE		0x40

#define SAA_DEVICE	"dbox/saa0"

/* ------------------------------------------------------------------------- */

struct saa7126
{
	int addr;
	int norm;
	int enable;
	int bright;
	int contrast;
	int hue;
	int sat;
};

static struct i2c_driver driver;
static struct i2c_client client_template;

struct saa7126type
{
	char *name;
	u8 Vendor;
	u8 Type;
};

#if 0
static struct saa7126type saa7126[] = {
	{"SAA7126", 0, 0 }
};
#endif

/* -------------------------------------------------------------------------
 * the functional interface to the i2c busses
 * -------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------- */

static int saa7126_attach(struct i2c_adapter *adap, int addr,
			unsigned short flags, int kind)
{
	int i;
	char buf[0x41];
	char *config;
	struct saa7126 *t;
	struct i2c_client *client;

	dprintk("[saa7126]: attach\n");

	if (this_adap > 0) {
		return -1;
	}

	this_adap++;

	client_template.adapter = adap;
	client_template.addr    = addr;

	dprintk("[saa7126]: chip found @ 0x%x\n",addr);

	if (NULL == (client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL))) {
		return -ENOMEM;
	}

	memcpy(client,&client_template,sizeof(struct i2c_client));

	client->data = t = kmalloc(sizeof(struct saa7126),GFP_KERNEL);

	if (NULL == t) {
		kfree(client);
		return -ENOMEM;
	}

	switch(board) {
		case DBOX_MID_NOKIA:
								config = PAL_SAA_NOKIA_CONFIG;
								break;
		case DBOX_MID_PHILIPS:
								config = PAL_SAA_PHILIPS;
								break;
		case DBOX_MID_SAGEM:
								config = PAL_SAA_SAGEM;
								break;
		default:
								config = 0;
								break;
	}

	i2c_attach_client(client);

	if (config) {

		/* svideo stuff */
		i  = 0x07; // RGB normal mode (0=b/w)

		i |= 0x08; // CVBS normal mode

		i |= 0x80; // lumi -> G
		i |= 0x20; // CVBS -> B

		/* upload data */
		for(i=0;i<0x80;i+=SAA_I2C_BLOCK_SIZE) {
			buf[0] = i;
			memcpy( buf+1, config+i,SAA_I2C_BLOCK_SIZE );
			i2c_master_send( client, buf, SAA_I2C_BLOCK_SIZE );
		}

		/* set video mode */
		saa7126_mode( mode );
	}

	//MOD_INC_USE_COUNT;

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_probe(struct i2c_adapter *adap)
{
	int ret;

	ret = 0;

	dprintk("[saa7126]: probe\n");

	if (0 != addr) {
		normal_i2c_range[0] = addr;
		normal_i2c_range[1] = addr;
	}

	this_adap = 0;

	if (1) {
		ret = i2c_probe(adap, &addr_data, saa7126_attach );
	}

	dprintk("[saa7126]: probe end %d\n",ret);

	return ret;
}

/* ------------------------------------------------------------------------- */

static int saa7126_detach(struct i2c_client *client)
{
	struct saa7126 *t = (struct saa7126*)client->data;

	dprintk("[Ssaa7126]: detach\n");

	i2c_detach_client(client);

	kfree(t);
	kfree(client);

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_command(struct i2c_client *client, unsigned int cmd, void *arg )
{
	struct saa7126 *encoder = client->data;

	dprintk("[saa7126]: command\n");

	switch (cmd)
	{
		case ENCODER_GET_CAPABILITIES:
		{
			struct video_encoder_capability *cap = arg;

			cap->flags
			    = VIDEO_ENCODER_PAL
			    | VIDEO_ENCODER_NTSC;

			cap->inputs  = 1;
			cap->outputs = 1;
		}
		break;

		case ENCODER_SET_NORM:
		{
			int *iarg = arg;

			switch (*iarg)
			{
				case VIDEO_MODE_NTSC:
					saa7126_encoder(0);
					break;

				case VIDEO_MODE_PAL:
					saa7126_encoder(1);
					break;

				default:
					return -EINVAL;
			}

			encoder->norm = *iarg;
		}
		break;

		case ENCODER_SET_INPUT:
		{
			int *iarg = arg;

			/* not much choice of outputs */
			if (*iarg != 0) {
				return -EINVAL;
			}

			/* RJ: *iarg = 0: input is from SA7111
			   *iarg = 1: input is from ZR36060 */

			/* Switch RTCE to 0/1 ... */
		}
		break;

		case ENCODER_SET_OUTPUT:
		{
			int *iarg = arg;

			/* not much choice of outputs */
			if (*iarg != 0) {
				return -EINVAL;
			}
		}
		break;

		case ENCODER_ENABLE_OUTPUT:
		{
			int *iarg = arg;

			encoder->enable = !!*iarg;

//			saa7185_write(encoder, 0x61, (encoder->reg[0x61] & 0xbf) | (encoder->enable ? 0x00 : 0x40));
		}
		break;

		default:
			return -EINVAL;
	}

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_cmd(struct i2c_client *client, u8 cmd, u8 *res, int size)
{
  struct i2c_msg msg[2];

  msg[0].flags=0;
  msg[1].flags=I2C_M_RD;
  msg[0].addr=msg[1].addr=client->addr;

  msg[0].buf=&cmd;
  msg[0].len=1;

  msg[1].buf=res;
  msg[1].len=size;

  i2c_transfer(client->adapter, msg, 2);

  return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_sendcmd(struct i2c_client *client, u8 b0, u8 b1)
{
	u8 cmd[2] = {b0,b1};

	if ( i2c_master_send(client,cmd,2) != 2 ) {
		return -1;
	}

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_read_register( char * buf )
{
	/* read status */
	saa7126_cmd(&client_template,0,buf,1);

	/* read register */
	saa7126_cmd(&client_template,1,buf+1,0x7f);

	return 0;
}

/* ------------------------------------------------------------------------- */

int saa7126_write_register(u8 reg, u8 val)
{

	return saa7126_sendcmd(&client_template, reg, val);
	
}

/* ------------------------------------------------------------------------- */

static int saa7126_input_control( int inp )
{
	saa7126_sendcmd( &client_template, 0x3a, (inp&0x9f) );

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_output_control( int inp )
{
	saa7126_sendcmd( &client_template, 0x2d, inp&0xff );

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_encoder( int inp )
{
	u8 b;

	/* read status */
	saa7126_cmd(&client_template,0x61,&b,1);

	/* set pal or ntsc */
	if (inp) {
		b |=  2; // pal
	} else {
		b &= ~2; // ntsc
	}

	saa7126_sendcmd( &client_template, 0x61, b );

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_mode( int inp )
{
	u8 b;

	/* read status */
	saa7126_cmd(&client_template,0x2d,&b,1);

	switch(inp)
	{
		case SAA_MODE_RGB:
		case SAA_MODE_FBAS:
				b &= ~0x10;
				b &= ~0x40;
				break;
		case SAA_MODE_SVIDEO:
				b |= 0x10; // croma -> R
				b |= 0x40; // lumi -> CVBS
				break;
		default:
				return -EINVAL;
	}

	saa7126_output_control( b );

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_power_save( int inp )
{
	if ( (inp) && (!saa_power_mode) ) // go power save
	{
		/* read 2dh */
		saa7126_cmd( &client_template, 0x2d, &saa_power_reg_2d, 1 );
		/* read 61h */
		saa7126_cmd( &client_template, 0x61, &saa_power_reg_61, 1 );

		/* write 2dh */
		saa7126_sendcmd( &client_template, 0x2d, (saa_power_reg_2d&0xf0) );
		/* write 61h */
		saa7126_sendcmd( &client_template, 0x61, (saa_power_reg_61|0xc0) );

		saa_power_mode = 1;
	}
	else if ( (!inp) && (saa_power_mode) ) // go out power save
	{
		/* write 2dh */
		saa7126_sendcmd( &client_template, 0x2d, saa_power_reg_2d );
		/* write 61h */
		saa7126_sendcmd( &client_template, 0x61, saa_power_reg_61 );

		saa_power_mode = 0;
	}
	else // wrong
	{
		return -EINVAL;
	}

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_vps_set_data( char * buf )
{
	u8 b;

	/* read vps status */
	saa7126_cmd(&client_template,0x54,&b,1);

	if (buf[0]==1)
	{
		b |= 0x80;

		i2c_master_send( &client_template, buf, 5 );
	}
	else if (buf[1]==0)
	{
		b &= ~0x80;
	}
	else
	{
		return -EINVAL;
	}

	/* write vps status */
	saa7126_sendcmd( &client_template, 0x54, b );

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_vps_get_data( char * buf )
{
	u8 b;

	/* read vps status */
	saa7126_cmd(&client_template,0x54,&b,1);

	if (b&0x80)
	{
		buf[0] = 1;
	}
	else
	{
		buf[0] = 0;
	}

	/* read vps date */
	saa7126_cmd(&client_template,0x54,buf+1,5);

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_wss_get(void)
{
	u8 b[2];
	unsigned i;

	saa7126_cmd(&client_template,0x26,b,2);
	if (b[1] & 0x80)
	{
		b[0] &= 0x0F;
		i = 0;
		while (i < 8)
		{
			if (b[0] == wss_data[i])
			{
				return i;
			}
			i++;
		}
	}

	return SAA_WSS_OFF;
}

/* ------------------------------------------------------------------------- */

static int saa7126_wss_set(int i)
{
	u8 b[3];

	if (i == SAA_WSS_OFF) {
		saa7126_sendcmd(&client_template,0x27,0x00);
		return 0;
	}

	if (i > 7)
	{
		return -EINVAL;
	}
	b[0] = 0x26;
	b[1] = wss_data[i];
	b[2] = 0x80;
	i2c_master_send(&client_template,b,3);

	return 0;
}


/* ------------------------------------------------------------------------- */

static int saa7126_ioctl (struct inode *inode, struct file *file, unsigned int cmd,
		  unsigned long arg)
{
	char saa_data[SAA_DATA_SIZE];
    int val;

	dprintk("[SAA7126]: IOCTL\n");

	/* no ioctl in power save mode */
	if ( (saa_power_mode) && (cmd!=SAAIOSPOWERSAVE) && (cmd!=SAAIOGPOWERSAVE) )
	{
		return -EINVAL;
	}

	switch (cmd)
	{
		case SAAIOGREG:
				if ( saa7126_read_register(saa_data) ) {
					return -EINVAL;
				}

				return copy_to_user( (void*)arg, saa_data, 0x80 );

		case SAAIOSINP:
				if ( copy_from_user( &val, (void*)arg, sizeof(val) ) ) {
					return -EFAULT;
				}

				saa7126_input_control(val);

				break;

		case SAAIOSOUT:
				if ( copy_from_user( &val, (void*)arg, sizeof(val) ) ) {
					return -EFAULT;
				}

				saa7126_output_control(val);

				break;

		case SAAIOSMODE:
				if ( copy_from_user( &val, (void*)arg, sizeof(val) ) ) {
					return -EFAULT;
				}

				saa7126_mode(val);

				break;

		case SAAIOSENC:
				if ( copy_from_user( &val, (void*)arg, sizeof(val) ) ) {
					return -EFAULT;
				}

				saa7126_encoder(val);

				break;

		case SAAIOSPOWERSAVE:
				if ( copy_from_user( &val, (void*)arg, sizeof(val) ) ) {
					return -EFAULT;
				}

				saa7126_power_save(val);

				break;

		case SAAIOGPOWERSAVE:
				val = saa_power_mode;

				return copy_to_user( (void*)arg, &val, sizeof(val) );

				break;

		case SAAIOSVPSDATA:
				if ( copy_from_user( saa_data, (void*)arg, 6 ) ) {
					return -EFAULT;
				}

				saa7126_vps_set_data(saa_data);

				break;

		case SAAIOGVPSDATA:
				saa7126_vps_get_data(saa_data);

				return copy_to_user( (void*)arg, saa_data, 6 );

				break;

		case SAAIOSWSS:
				if (copy_from_user( &val, (void*)arg,sizeof(val))) {
					return -EFAULT;
				}
				return saa7126_wss_set(val);

				break;

		case SAAIOGWSS:
				val = saa7126_wss_get();
				if (val < 0) {
					return val;
				}
				else {
					return copy_to_user( (void*)arg, &val, sizeof(val));
				}

				break;

		default:
				return -EINVAL;
	}

	return 0;
}

/* ------------------------------------------------------------------------- */

static int saa7126_open (struct inode *inode, struct file *file)
{
	return 0;
}

/* ------------------------------------------------------------------------- */

void inc_use (struct i2c_client *client)
{
#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif
}

void dec_use (struct i2c_client *client)
{
#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
}

/* ------------------------------------------------------------------------- */

static struct i2c_driver driver = {
	"i2c saa7126 driver",
	I2C_DRIVERID_SAA7126,
	I2C_DF_NOTIFY,
	saa7126_probe,
	saa7126_detach,
	saa7126_command,
	inc_use,
	dec_use,
};

static struct i2c_client client_template =
{
	"i2c saa7126 chip",		/* name				*/
	I2C_DRIVERID_SAA7126,	/* ID					*/
	0,
	0,										/* interpret as 7Bit-Adr	*/
	NULL,
	&driver
};

/* ------------------------------------------------------------------------- */

EXPORT_SYMBOL(saa7126_write_register);

#ifdef MODULE
int init_module(void)
#else
int saa7126_init(void)
#endif
{
	/* power mode vars */
	saa_power_mode   = 0;
	saa_power_reg_2d = 0;
	saa_power_reg_61 = 0;

	if (!board)
	{
		struct dbox_info_struct dbox;
		dbox_get_info(&dbox);
		board=dbox.mID;
	}

	if ( (board<1) || (board>3) )
	{
		printk("saa7126.o: wrong board %d\n", board);
		return -EIO;
	}

	devfs_handle = devfs_register ( NULL, SAA_DEVICE, DEVFS_FL_DEFAULT, 0, 0,
		     S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
		     &saa7126_fops, NULL );

	if ( ! devfs_handle )
	{
		return -EIO;
	}

	i2c_add_driver(&driver);

	/* setup teletext timings for pal*/
	saa7126_write_register(0x73, 0x42);
	saa7126_write_register(0x76, 0x05);
	saa7126_write_register(0x77, 0x16);
	saa7126_write_register(0x78, 0x04);
	saa7126_write_register(0x79, 0x16);

	return 0;
}

#ifdef MODULE
void cleanup_module(void)
{
	saa7126_power_save(1);

	i2c_del_driver(&driver);

	devfs_unregister ( devfs_handle );

	return;
}
#endif
