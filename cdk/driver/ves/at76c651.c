/*
 * $Id: at76c651.c,v 1.31 2002/09/09 18:45:53 obi Exp $
 *
 * Sagem DVB-C Frontend Driver (at76c651/dat7021)
 *
 * Homepage: http://dbox2.elxsi.de
 *
 * Copyright (C) 2001 fnbrd <fnbrd@gmx.de>
 *             & 2002 Andreas Oberritter <obi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <asm/bitops.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/i2c.h>

#include <dbox/dvb_frontend.h>
#include <dbox/fp.h>


static int debug = 0;
#define dprintk	if (debug) printk



#define I2C_DRIVERID_DAT7021 0xF0C2


/*
 * DAT7021
 * -------
 * Input Frequency Range (RF): 48.25 MHz to 863.25 MHz
 * Band Width: 8 MHz
 * Level Input (Range for Digital Signals): -61 dBm to -41 dBm
 * Output Frequency (IF): 36 MHz
 *
 * (see http://www.atmel.com/atmel/acrobat/doc1320.pdf)
 */


static
FrontendInfo at76c651_info = {
	type:		FE_QAM,
	minFrequency:	48250000,
	maxFrequency:	863250000,
	minSymbolRate:	0,
	maxSymbolRate:	9360000,
	hwType:		0,
	hwVersion:	0
};


struct at76c651 {
	dvb_front_t frontend;
};


static struct i2c_client at76c651_i2c_client;
static struct i2c_driver at76c651_i2c_driver;
static struct i2c_client dat7021_i2c_client;
static struct i2c_driver dat7021_i2c_driver;
static struct i2c_client *dclient_tuner = NULL;
static struct i2c_client *dclient = NULL;



static
int at76c651_writereg (struct i2c_client *i2c, u8 reg, u8 data)
{
	int ret;
	u8 buf[] = { reg, data };
	struct i2c_msg msg = { addr: i2c->addr, flags: 0, buf: buf, len: 2};

	ret = i2c_transfer(i2c->adapter, &msg, 1);

	if (ret != 1)
		dprintk("%s: writereg error "
			"(reg == 0x%02x, val == 0x%02x, ret == %i)\n",
			__FUNCTION__, reg, data, ret);

	mdelay(10);
	return (ret != 1) ? -EREMOTEIO : 0;
}


static
u8 at76c651_readreg (struct i2c_client *i2c, u8 reg)
{
	int ret;
	u8 b0 [] = { reg };
	u8 b1 [] = { 0 };
	struct i2c_msg msg [] = { { addr: i2c->addr, flags: 0, buf: b0, len: 1 },
			   { addr: i2c->addr, flags: I2C_M_RD, buf: b1, len: 1 } };

	ret = i2c_transfer(i2c->adapter, msg, 2);

	if (ret != 2)
		dprintk("%s: readreg error (ret == %i)\n", __FUNCTION__, ret);

	return b1[0];
}


static
int at76c651_set_auto_config (struct i2c_client *i2c)
{
	at76c651_writereg (i2c, 0x06, 0x01);

	/* performance optimizations */
	at76c651_writereg (i2c, 0x10, 0x06);
	at76c651_writereg (i2c, 0x11, 0x10);
	at76c651_writereg (i2c, 0x15, 0x28); 
	at76c651_writereg (i2c, 0x20, 0x09); 
	at76c651_writereg (i2c, 0x24, 0x90); 

	return 0;
}


static
int at76c651_set_bbfreq (struct i2c_client *i2c)
{
	at76c651_writereg (i2c, 0x04, 0x3f);
	at76c651_writereg (i2c, 0x05, 0xee);
	return 0;
}


static
int at76c651_reset (struct i2c_client *i2c)
{
	return at76c651_writereg (i2c, 0x07, 0x01);
}


static
int at76c651_disable_interrupts (struct i2c_client *i2c)
{
	return at76c651_writereg (i2c, 0x0b, 0x00);
}


static
int at76c651_switch_tuner_i2c (struct i2c_client *i2c, u8 enable)
{
	if (enable)
		return at76c651_writereg(i2c, 0x0c, 0xc2 | 0x01);
	else 
		return at76c651_writereg(i2c, 0x0c, 0xc2);
}


static
int dat7021_set_dword (struct i2c_client *i2c, u32 tw)
{
	char msg[4];
	int ret;

	*(u32*)msg = tw;

	at76c651_switch_tuner_i2c (i2c, 1);

	ret = i2c_master_send (dclient_tuner, msg, 4);

	at76c651_switch_tuner_i2c (i2c, 0);

	if (ret != 4)
		return -1;

	at76c651_reset(i2c);
	return 0;
}


static
int dat7021_set_tv_freq (struct i2c_client *i2c, u32 freq)
{
	u32 dw;

	if ((freq < 48250) || (freq > 863250))
		return -1;

	/*
	 * formula: dw=0x17e28e06+(freq-346000UL)/8000UL*0x800000
	 *      or: dw=0x4E28E06+(freq-42000) / 125 * 0x20000
	 */

	dw = (freq - 42000) * 4096;
	dw = dw / 125;
	dw = dw * 32;

	if (freq > 394000)
		dw += 0x4E28E85;
	else
		dw += 0x4E28E06;

	return dat7021_set_dword(i2c, dw);
}


static
int at76c651_set_symbolrate (struct i2c_client *i2c, u32 symbolrate)
{
	u8 exponent;
	u32 mantissa;

	if (symbolrate > 9360000)
		return -1;

	/*
	 * FREF = 57800 kHz
	 * exponent = 10 + floor ( log2 ( symbolrate / FREF ) )
	 * mantissa = ( symbolrate / FREF) * ( 1 << ( 30 - exponent ) )
	 */

	exponent = __ilog2((symbolrate << 4) / 903125);
	mantissa = ((symbolrate / 3125) * (1 << (24 - exponent))) / 289;

	at76c651_writereg (i2c, 0x00, mantissa >> 13);
	at76c651_writereg (i2c, 0x01, mantissa >> 5);
	at76c651_writereg (i2c, 0x02, (mantissa << 3) | exponent);

	return 0;
}


static
int at76c651_set_qam (struct i2c_client *i2c, Modulation qam)
{
	u8 qamsel = 0;

	switch (qam)
	{
	case QPSK:    qamsel = 0x02; break;
	case QAM_16:  qamsel = 0x04; break;
	case QAM_32:  qamsel = 0x05; break;
	case QAM_64:  qamsel = 0x06; break;
	case QAM_128: qamsel = 0x07; break;
	case QAM_256: qamsel = 0x08; break;
	default:
		 return -EINVAL;
	}

	return at76c651_writereg (i2c, 0x03, qamsel);
}


static
int at76c651_set_inversion (struct i2c_client *i2c, SpectralInversion inversion)
{
	u8 feciqinv = at76c651_readreg (i2c, 0x60);

	switch (inversion)
	{
	case INVERSION_OFF:
		feciqinv |= 0x02;
		feciqinv &= 0xFE;
		break;

	case INVERSION_ON:
		feciqinv |= 0x03;
		break;

	case INVERSION_AUTO:
		feciqinv &= 0xFC;
		break;

	default:
		return -EINVAL;
	}

	return at76c651_writereg (i2c, 0x60, feciqinv);
}


static
int at76c651_set_parameters (struct i2c_client *i2c, FrontendParameters *p)
{
	at76c651_set_symbolrate (i2c, p->u.qam.SymbolRate);
	at76c651_set_qam (i2c, p->u.qam.QAM);
	at76c651_set_inversion (i2c, p->Inversion);
	at76c651_set_auto_config (i2c);
	return 0;
}


static
int at76c651_init (struct i2c_client *i2c)
{
	at76c651_set_symbolrate (i2c, 6900000);
	at76c651_set_qam (i2c, QAM_64);
	at76c651_set_bbfreq (i2c);
	at76c651_set_auto_config (i2c);
	at76c651_disable_interrupts (i2c);
	return 0;
}


static
int at76c651_ioctl (struct i2c_client *i2c, unsigned int cmd, void *arg)
{
	switch (cmd) {
	case FE_GET_INFO:
		memcpy (arg, &at76c651_info, sizeof(FrontendInfo));
		break;

	case FE_READ_STATUS:
	{
		FrontendStatus *status = (FrontendStatus *) arg;
		u8 sync;

		/* Bits: FEC, CAR, EQU, TIM, AGC2, AGC1, ADC, PLL (PLL=0) */
		sync = at76c651_readreg (i2c, 0x80);

		*status = 0;

		if (sync & 0x02) /* ADC */
			*status |= FE_HAS_POWER; // ?

		if (sync & 0x08) /* AGC2 */
			*status |= FE_HAS_SIGNAL;

		if (sync & 0x10) /* TIM */
			*status |= FE_HAS_VITERBI; // ?
		
		if (sync & 0x20) /* EQU */
			*status |= FE_HAS_SYNC;
		
		if (sync & 0x40) /* CAR */
			*status |= FE_HAS_CARRIER;

		if (sync & 0x80) /* FEC */
			*status |= FE_HAS_LOCK;

		if (at76c651_readreg (i2c, 0x60) & 0x04) /* IQINV */
			*status |= FE_SPECTRUM_INV;
		
		break;
	}

	case FE_READ_BER:
	{
                u32 *ber = (u32 *) arg;

		*ber = (at76c651_readreg (i2c, 0x81) & 0x0F ) << 16;
		*ber |= at76c651_readreg (i2c, 0x82) << 8;
		*ber |= at76c651_readreg (i2c, 0x83);
		*ber *= 10;
		break;
	}

	case FE_READ_SIGNAL_STRENGTH:
	{
		u8 gain = ~at76c651_readreg (i2c, 0x91);
		*(s32*)arg = (gain << 8) | gain;
		break;
	}

	case FE_READ_SNR:
		*(s32*)arg = 0xFFFF - ((at76c651_readreg (i2c, 0x8F) << 8) |
					at76c651_readreg (i2c, 0x90));
		break;

	case FE_READ_UNCORRECTED_BLOCKS:
		*(u32*)arg = at76c651_readreg (i2c, 0x82);
		break;

	case FE_SET_FRONTEND:
		return at76c651_set_parameters (i2c, arg);

	case FE_GET_FRONTEND:
		break;
#if 0
	case FE_SLEEP:
		break;
#endif
	case FE_INIT:
		return at76c651_init (i2c);

	case FE_RESET:
		return at76c651_reset (i2c);

	case FE_SETFREQ:
		return dat7021_set_tv_freq (i2c, *(u32*)arg);

	default:
		return -EINVAL;
	}
	
	return 0;
} 


static
int at76c651_attach(struct i2c_adapter *adap)
{
	struct at76c651 *at;
	struct i2c_client *i2c;

	at76c651_i2c_client.adapter = adap;

	if (at76c651_readreg(&at76c651_i2c_client, 0x0e) != 0x65) 
	{
		printk("no AT76C651(B) found\n");
		return -1;
	}
	
	if (at76c651_readreg(&at76c651_i2c_client, 0x0f) != 0x10)
	{
		if (at76c651_readreg(&at76c651_i2c_client, 0x0f) == 0x11) 
		{
			dprintk("AT76C651B found\n");
		}
		else 
		{
			printk("no AT76C651(B) found\n");
			return -1;
		}
	}
	else
	{
		dprintk("AT76C651B found\n");
	}

	i2c = kmalloc(sizeof(struct i2c_client), GFP_KERNEL);

	if (!i2c)
		return -ENOMEM;

	memcpy(i2c, &at76c651_i2c_client, sizeof(struct i2c_client));

	i2c->data = at = (struct at76c651 *) kmalloc(sizeof(struct at76c651), GFP_KERNEL);
	
	if (!at) 
	{
		kfree(i2c);
		return -ENOMEM;
	}
	
	dprintk("AT76C651: attaching AT76C651 at 0x%02x\n", i2c->addr << 1);
	i2c_attach_client(i2c);
	dprintk("AT76C651: attached to adapter %s\n", adap->name);

	at->frontend.type = DVB_C;
	at->frontend.capabilities = 0;
	at->frontend.i2cbus = adap;
	at->frontend.demod = i2c;
	at->frontend.demod_type = DVB_DEMOD_AT76C651;

	at76c651_init(i2c);
	register_frontend(&at->frontend);
	
	dclient = i2c;

	return 0;
}


static
int at76c651_detach (struct i2c_client *i2c)
{
	dprintk("AT76C651: detach_client\n");
	unregister_frontend(&((struct at76c651 *)i2c->data)->frontend);
	at76c651_disable_interrupts(i2c);
	i2c_detach_client(i2c);
	kfree(i2c->data);
	kfree(i2c);
	return 0;
}


static
void at76c651_inc_use (struct i2c_client *i2c)
{
#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif
}


static
void at76c651_dec_use (struct i2c_client *i2c)
{
#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
}


static
struct i2c_driver at76c651_i2c_driver =
{
	name:		"AT76C651 DVB-C Demodulator",
	id:		I2C_DRIVERID_AT76C651,
	flags:		I2C_DF_NOTIFY,
	attach_adapter:	at76c651_attach,
	detach_client:	at76c651_detach,
	command:	at76c651_ioctl,
	inc_use:	at76c651_inc_use,
	dec_use:	at76c651_dec_use
};


static
struct i2c_client at76c651_i2c_client =
{
	name:		"AT76C651",
	id:		I2C_DRIVERID_AT76C651,
	flags:		0,
	addr:		0x1a >> 1,
	adapter:	NULL,
	driver:		&at76c651_i2c_driver,
	usage_count:	0
};


static
int dat7021_init (struct i2c_client *i2c)
{
	int ret;

	at76c651_switch_tuner_i2c (i2c, 1);

	if ((ret = i2c_add_driver(&dat7021_i2c_driver)))
		printk("%s: DAT7021: registration failed\n", __FUNCTION__);

	else if (!dclient_tuner)
	{
		printk("%s: DAT7021: device not found\n", __FUNCTION__);
		i2c_del_driver(&dat7021_i2c_driver);
		ret = -EBUSY;
	}

	at76c651_switch_tuner_i2c (i2c, 0);

	return ret;
}


static
int dat7021_attach (struct i2c_adapter *adap)
{
	struct i2c_client *i2c;
	
	dat7021_i2c_client.adapter = adap;

	i2c = kmalloc(sizeof(struct i2c_client), GFP_KERNEL);

	if (!i2c)
		return -ENOMEM;

	memcpy(i2c, &dat7021_i2c_client, sizeof(struct i2c_client));

	dclient_tuner = i2c;

	i2c->data = 0;

	dprintk("%s: attaching at 0x%02x\n", __FUNCTION__, (i2c->addr) << 1);
	i2c_attach_client(i2c);
	dprintk("%s: attached to adapter %s\n", __FUNCTION__, adap->name);

	return 0;
}


static
int dat7021_detach (struct i2c_client *i2c)
{
	int ret;

	if ((ret = i2c_detach_client(i2c)))
	{
		dprintk("AT76C651: couldn't detach tuner client driver.\n");
		return ret;
	}

	kfree(i2c);
	return 0;
}


static
struct i2c_driver dat7021_i2c_driver =
{
	name:		"DAT7021 driver",
	id:		I2C_DRIVERID_DAT7021,
	flags:		I2C_DF_NOTIFY,
	attach_adapter:	&dat7021_attach,
	detach_client:	&dat7021_detach,
	command:	0,
	inc_use:	0,
	dec_use:	0,
};


static
struct i2c_client dat7021_i2c_client =
{
	name:		"DAT7021",
	id:		I2C_DRIVERID_DAT7021,
	flags:		0,
	addr:		0xc2 >> 1,
	adapter:	NULL,
	driver:		&dat7021_i2c_driver,
	usage_count:	0
};


static
int __init init_at76c651 (void)
{
	int res;

	printk("$Id: at76c651.c,v 1.31 2002/09/09 18:45:53 obi Exp $\n");
	
	if ((res = i2c_add_driver(&at76c651_i2c_driver)))
	{
		printk("%s: AT76C51: registration failed\n", __FUNCTION__);
		return res;
	}

	if ((res = dat7021_init(dclient)))
		return res;

	dprintk("%s: done\n", __FUNCTION__);
	return 0;
}


static
void __exit exit_at76c651 (void)
{
	if (i2c_del_driver(&at76c651_i2c_driver))
		printk("%s: AT76C51: deregistration failed\n", __FUNCTION__);

	if (i2c_del_driver(&dat7021_i2c_driver))
		printk("%s: DAT7021: deregistration failed\n", __FUNCTION__);

	dprintk("%s: done\n", __FUNCTION__);
}


#ifdef MODULE
module_init(init_at76c651);
module_exit(exit_at76c651);
MODULE_DESCRIPTION("at76c651/dat7021 dvb-c frontend driver");
MODULE_AUTHOR("Andreas Oberritter <obi@tuxbox.org>");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
MODULE_PARM(debug,"i");
#endif

