/* 
 *   $Id: tda8044h.c,v 1.14.2.1 2003/02/20 00:41:28 ghostrider Exp $
 *   
 *   tda8044h.c - Philips TDA8044H (d-box 2 project) 
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
 *   $Log: tda8044h.c,v $
 *   Revision 1.14.2.1  2003/02/20 00:41:28  ghostrider
 *   merge FE_READ_SNR, FE_READ_SIGNAL_STRENGTH, FE_READ_BER, FE_READ_UNCORRECED_BLOCKS with head.... i hope in enigma now the signals bars are correct...
 *
 *   Revision 1.14  2002/08/26 16:29:10  obi
 *   - added symbol rate calculation and anti-alias filter selection
 *   - divided set_sec into set_tone and set_voltage
 *   - some cleanup
 *
 *   Revision 1.13  2002/08/25 14:15:46  obi
 *   - BER should work (the higher fbcn (min 0, max F), the higher the bitrange)
 *   - SIGNAL_STRENGTH should work (min 0, max 65535)
 *   - SNR should work (min 0, max 65535)
 *   - STATUS does not reread to avoid randomness
 *   - fixed some &'s
 *
 *   Revision 1.12  2002/08/12 16:56:42  obi
 *   removed compiler warnings
 *
 *   Revision 1.11  2002/08/11 19:48:15  happydude
 *   disable debug messages
 *
 *   Revision 1.10  2002/05/18 21:01:48  derget
 *   fixx wegem lock problem von zapit (untested)
 *   sync wurde nie richtig gelesen !
 *   danke an TheTobe
 *
 *   Revision 1.9  2002/04/24 12:08:38  obi
 *   made framing byte hack nicer
 *
 *   Revision 1.8  2002/04/20 18:23:16  obi
 *   added raw diseqc command
 *
 *   Revision 1.7  2002/02/24 15:32:06  woglinde
 *   new tuner-api now in HEAD, not only in branch,
 *   to check out the old tuner-api should be easy using
 *   -r and date
 *
 *   Revision 1.4.2.3  2002/01/22 23:44:56  fnbrd
 *   Id und Log reingemacht.
 *
 *   Revision 1.4.2.2  2002/01/22 23:41:41  fnbrd
 *   changed to new tuning_api
 *
 *   Revision 1.5  2002/01/22 22:35:26  tmbinc
 *   added new tuning api (branch: new_tuning_api)
 *

 *   Revision 1.4  2001/11/02 02:33:57  TripleDES
 *   fixed diseqc(tm) problem
 *
 *   Revision 1.3  2001/11/01 02:17:00  TripleDES
 *   added DiSEqC(tm) support - no minidiseqc up to now
 *
 *   Revision 1.2  2001/07/07 23:39:38  tmbinc
 *   fixes
 *
 *   Revision 1.1  2001/04/19 22:48:10  tmbinc
 *   philips support (sat, tda8044h), ost/dvb.c fix to call demod->init() now.
 *
 *
 *   $Revision: 1.14.2.1 $
 *
 */

/* ---------------------------------------------------------------------- */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <linux/i2c.h>

#include <dbox/dvb_frontend.h>
#include <dbox/fp.h>
#include <ost/sec.h>

#ifdef MODULE
MODULE_DESCRIPTION("");
MODULE_AUTHOR("Felix Domke");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
MODULE_PARM(debug,"i");
#endif

static DECLARE_MUTEX(i2c_mutex);

static int tda_set_tone (struct i2c_client * client);
static int tda_set_voltage (struct i2c_client * client);

static int tda_set_freq (struct i2c_client * client, u32 freq);
static int tda_send_diseqc (struct i2c_client * client, u8 * cmd, u32 len);


static int debug = 0;
#define dprintk	if (debug) printk

static int instances = 0;

static struct i2c_driver dvbt_driver;

static int attach_adapter(struct i2c_adapter *adap);
static int detach_client(struct i2c_client *client);
static int dvb_command(struct i2c_client *client, unsigned int cmd, void *arg);
static void inc_use (struct i2c_client *client);
static void dec_use (struct i2c_client *client);

static struct i2c_driver dvbt_driver = {
	"TDA8044H DVB DECODER",
	DVB_DEMOD_TDA8044,
	I2C_DF_NOTIFY,
	attach_adapter,
	detach_client,
	dvb_command,
	inc_use,
	dec_use,
};


static struct i2c_client client_template = {
	"TDA8044H",
	I2C_DRIVERID_TDA8044,
	0,
	(0xD0 >> 1),
	NULL,
	&dvbt_driver,
	NULL
};


static struct i2c_client tuner = {
	"TUNER",
	I2C_DRIVERID_TDA8044+1,
	0,
	(0xC6 >> 1),
	NULL,
	&dvbt_driver,
	NULL
};




#define TDA_INTERRUPT 14
static void tda_interrupt(int irq, void *vdev, struct pt_regs * regs);

static void tda_task(void*);

struct tq_struct tda_tasklet=
{
	routine: tda_task,
	data: 0
};

static u8 Init8044Tab[] =
{
	0x06,			// General settings
	0x00,			// Viterbi decoder
	0x6f, 0xb5, 0x86,	// CLK: ratio
	0x22,			// CLK: Anti Alias Filter, Roll off
	0x00,			// Sigma Delta converter
	0xea,			// FEC: Possible puncturing rates

	0x30,			// carrier lock detector threshold value
	0x42,			// AFC1: proportional path settings
	0x98,			// AFC1: Integral path setting
	0x68,			// PD: Leaky integrator SCPC mode
	0x70,			// AFC2, AFC1 controls
	0x42,			// PD: proportional part settings
	0x99,			// PD: integral part settings

	0x58,			// AGC
	0x95,			// AGC: gain
	0x10,			// AGC: Threshold
	0xf5,			// AGC: Integration time
	0xe7,			// FEC
	0x93,			// FEC: RS lock
	0x0b,			// FEC: RS and Viterbi lock
	0x15,			// FEC: Miscellaneous
	0x68,			// CLK: proportional path
	0x9a,			// CLK: integral path
	0x90,			// CLK: lock threshold
	0x61,			// CLK: loop control
	0x80,			// Offset value I
	0x00,			// Offset value Q
	0xe0, 0x40,		// Fine AGC gain
	0x00,			// interrupt selection

	0x0f,			// EXPWR.
	0x15,			// Evaluation modes
	0x00,			// SCPC: Frequency offset
	0x00,			// CTL: Frame byte
	0x00,			// CTL: address byte
	0x00,			// CTL: command byte
	0x00,			// CTL: data byte 0
	0x00,			// CTL: data byte 1
	0x00,			// CTL: data byte 2
	0x00			// CTL: Send
};

struct tda8044
{
	u32 srate;
	u8 sync;

	CodeRate fec;
	SpectralInversion inversion;

	secVoltage power;
	secToneMode tone;

	dvb_front_t frontend;
};

/* ---------------------------------------------------------------------- */

int writereg(struct i2c_client *client, u8 reg, u8 data)
{
	int ret;
	u8 msg[2];

	if (down_interruptible(&i2c_mutex))
		return -EAGAIN;

	msg[0]=reg;
	msg[1]=data;
	
	ret=i2c_master_send(client, msg, 2);
	
	if (ret!=2)
		printk("%s: writereg error\n", __FILE__);
	
	mdelay(10);
	up(&i2c_mutex);
	return ret;
}

/* ---------------------------------------------------------------------- */

u8 readreg(struct i2c_client *client, u8 reg)
{
	struct i2c_adapter *adap=client->adapter;
	unsigned char mm1[1];
	unsigned char mm2[1];
	struct i2c_msg msgs[2];

	if (down_interruptible(&i2c_mutex))
		return -EAGAIN;

	msgs[0].flags=0;
	msgs[1].flags=I2C_M_RD;
	msgs[0].addr=msgs[1].addr=client->addr;
	mm1[0]=reg;
	mm2[0]=0;
	msgs[0].len=1;
	msgs[1].len=1;
	msgs[0].buf=mm1;
	msgs[1].buf=mm2;
	
	i2c_transfer(adap, msgs, 2);

	up(&i2c_mutex);

	return mm2[0];
}

/* ---------------------------------------------------------------------- */

static u8 fbcn = 0xF;

int init(struct i2c_client *client)
{
	struct tda8044 *tda=(struct tda8044*)client->data;
	int i;

	for (i=0; i<sizeof(Init8044Tab); i++)
		if (writereg(client, i, Init8044Tab[i])<0)
			return -1;
	
	udelay(10000);

	writereg(client, 0x0F, 0x50);
	writereg(client, 0x20, 0x8F);
	writereg(client, 0x20, 0xBF);
	writereg(client, 0x00, 0x04);
	writereg(client, 0x00, 0x0C);
	
	udelay(10000);
	
	Init8044Tab[0x00] = 0x04;
	Init8044Tab[0x0F] = 0x50;
	Init8044Tab[0x16] |= fbcn << 4;
	Init8044Tab[0x1F] = 0x7F;

	for (i=0; i<sizeof(Init8044Tab); i++)
		if (writereg(client, i, Init8044Tab[i])<0)
			return -1;
	
	tda->inversion = INVERSION_AUTO;
	tda->power = SEC_VOLTAGE_OFF;
	tda->srate = 27500000;

	return 0;
}

static void SetSymbolrate (struct i2c_client * client)
{
	struct tda8044 * tda = (struct tda8044 *) client->data;

	u8 buffer[16];
	u32 ratio;

	dprintk("%s: setting symbolrate: %d\n", __FILE__, tda->srate);

	/* 1st target */
	buffer[0x00] = 0x00;
	/* Interrupt request */
	buffer[0x01] = 0x04;
	
	if (i2c_master_send(client, buffer, 2) != 2)
		printk("%s: i2c_master_send 1 failed!\n", __FILE__);

	/* 2nd target */
	buffer[0x00] = 0x01;

	/*
	 * Viterbi decoder:
	 * Differential decoding off
	 * Spectral inversion unknown
	 * QPSK modulation
	 */
	buffer[0x01] = 0x00;

	if (tda->inversion == INVERSION_ON)
		buffer[0x01] |= 0x60;
	else if (tda->inversion == INVERSION_OFF)
		buffer[0x01] |= 0x20;

	/*
	 * CLK ratio:
	 * 
	 * system clock frequency is 96000000 Hz
	 * TODO: calculate better ;)
	 * formula: 2^21 * freq / symrate
	 * 
	 * clock and symbol rate are divided by 100000
	 * to avoid an integer overflow
	 */
	ratio = (2 << 20) * 960 / (tda->srate / 100000);
	buffer[0x02] = ratio >> 16;
	buffer[0x03] = ratio >> 8;
	buffer[0x04] = ratio;

	/* nyquist filter roll-off factor 35% */
	buffer[0x05] = 0x20;

	/* Anti Alias Filter */
	if (tda->srate < 4500000)
		printk("%s: unsupported symbol rate: %d\n", __FILE__, tda->srate);
	else if (tda->srate <= 6000000)
		buffer[0x05] |= 0x07;
	else if (tda->srate <= 9000000)
		buffer[0x05] |= 0x06;
	else if (tda->srate <= 12000000)
		buffer[0x05] |= 0x05;
	else if (tda->srate <= 18000000)
		buffer[0x05] |= 0x04;
	else if (tda->srate <= 24000000)
		buffer[0x05] |= 0x03;
	else if (tda->srate <= 36000000)
		buffer[0x05] |= 0x02;
	else if (tda->srate <= 45000000)
		buffer[0x05] |= 0x01;
	else
		printk("%s: unsupported symbol rate: %d\n", __FILE__, tda->srate);

	/* Sigma Delta converter */
	buffer[0x06] = 0x00;

	/* FEC: Possible puncturing rates */
	switch (tda->fec)
	{
	case FEC_1_2:
		buffer[0x07] = 0x80;
		break;
		
	case FEC_2_3:
		buffer[0x07] = 0x40;
		break;
		
	case FEC_3_4:
		buffer[0x07] = 0x20;
		break;
		
	case FEC_5_6:
		buffer[0x07] = 0x08;
		break;
		
	case FEC_7_8:
		buffer[0x07] = 0x02;
		break;

	case FEC_AUTO:
	case FEC_NONE:
	default:
		buffer[0x07] = 0xFF;
		break;
	}

	/* carrier lock detector threshold value */
	buffer[0x08] = 0x30;
	
	/* AFC1: proportional part settings */
	buffer[0x09] = 0x42;
	
	/* AFC1: integral part settings */
	buffer[0x0A] = 0x98;
	
	/* PD: Leaky integrator SCPC mode */
	buffer[0x0B] = 0x28;

	/* AFC2, AFC1 controls */
	buffer[0x0C] = 0x30;
	
	/* PD: proportional part settings */
	buffer[0x0D] = 0x42;
	
	/* PD: integral part settings */
	buffer[0x0E] = 0x99;
	
	/* AGC */
	buffer[0x0F] = 0x50;

	if (i2c_master_send(client, buffer, 16) != 16)
		printk("%s: i2c_master_send 2 failed!\n", __FILE__);

	if (debug)
	{
		u8 i;

		printk("%s: SetSymbolrate:", __FILE__);

		for (i = 0; i < 16; i++)
			printk(" %02x", buffer[i]);

		printk("\n");
	}

	/* 3rd target */
	buffer[0x00] = 0x17;
	/* CLK proportional part */
	buffer[0x01] = 0x68;
	/* CLK integral part */
	buffer[0x02] = 0x9A;
	
	if (i2c_master_send(client, buffer, 3) != 3)
		printk("%s: i2c_master_send 3 failed!\n", __FILE__);

	/* 4th target */
	buffer[0x00] = 0x22;
	/* SCPC Frequency offset */
	buffer[0x01] = 0xF9;
	
	if (i2c_master_send(client, buffer, 2) != 2)
		printk("%s: i2c_master_send 4 failed!\n", __FILE__);
}

int attach_adapter(struct i2c_adapter *adap)
{
	struct tda8044 *tda;
	struct i2c_client *client;
	
	client_template.adapter=adap;
	
	if (writereg(&client_template, 0x89, 00)<0)
		return -EINVAL;

	if (readreg(&client_template, 0)!=4)
	{
		printk("%s: no TDA8044H found.\n", __FILE__);
		return -EINVAL;
	}

	if (!(client=kmalloc(sizeof(struct i2c_client), GFP_KERNEL)))
		return -ENOMEM;
	
	memcpy(client, &client_template, sizeof(struct i2c_client));
	
	client->data=tda=kmalloc(sizeof(struct tda8044), GFP_KERNEL);
	if (!tda)
	{
		kfree(client);
		return -ENOMEM;
	}
	
	tda->frontend.type=DVB_S;
	tda->frontend.capabilities=-1; // kann alles
	tda->frontend.i2cbus=adap;
	
	tda->frontend.demod=client;
	tda->frontend.demod_type=DVB_DEMOD_TDA8044;
	
	printk("%s: attaching TDA8044H at 0x%02x\n", __FILE__, (client->addr)<<1);
	i2c_attach_client(client);
	tuner.adapter=adap;
	if (register_frontend(&tda->frontend))
		printk("%s: can't register demod.\n", __FILE__);

	tda_tasklet.data = (void*)client;

	if (request_8xxirq(TDA_INTERRUPT, tda_interrupt, SA_ONESHOT, "tda8044h", NULL) != 0)
	{
		i2c_del_driver(&dvbt_driver);
		printk("%s: can't request interrupt\n", __FILE__);
		return -EBUSY;
	}

	instances++; 
	return 0;
}

int detach_client(struct i2c_client *client)
{
	instances--;
	printk("%s: detach_client\n", __FILE__);
	unregister_frontend(&((struct tda8044*)client->data)->frontend);
	i2c_detach_client(client);
	kfree(client->data);
	kfree(client);
	free_irq(TDA_INTERRUPT, NULL);
	return 0;
}

static int dvb_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	struct tda8044 *tda=(struct tda8044*)client->data;
	
	switch (cmd)
	{
	case FE_READ_STATUS:
	{
		FrontendStatus *status=(FrontendStatus*)arg;

		/* rereading status introduces some randomness */
		/*tda->sync=readreg(client, 2);*/

		*status = 0x00;

		if (tda->sync & 0x01) /* demodulator lock */
			*status |= FE_HAS_SIGNAL;
		if (tda->sync & 0x02) /* clock recovery lock */
			*status |= FE_HAS_CARRIER;
		if (tda->sync & 0x04) /* viterbi lock */
			*status |= FE_HAS_VITERBI;
		if (tda->sync & 0x08) /* deinterleaver lock */
			*status |= FE_HAS_SYNC;
		if (tda->sync & 0x10) /* derandomizer lock */
			*status |= FE_HAS_LOCK;
		if (tda->inversion)
			*status |= FE_SPECTRUM_INV;
		break;
	}
	case FE_READ_BER:
	{
		*((u32*) arg) = readreg(client, 0x0d) |
				(readreg(client, 0x0c) << 8) |
				((readreg(client, 0x0b) & 0x1f) << 16);
		break;
	}
	case FE_READ_SIGNAL_STRENGTH:
	{
		u8 gain = readreg(client, 0x01);
		*((u16*) arg) = (gain << 8) | gain;
		break;
	}
	case FE_READ_SNR:
	{
		u8 quality = readreg(client, 0x08);
		*((u16*) arg) = (quality << 8) | quality;
		break;
	}
	case FE_READ_UNCORRECTED_BLOCKS:
	{
		*((u32*) arg) = readreg(client, 0x0f);
		if (*((u32*) arg) == 0xff)
			*((u32*) arg) = 0xffffffff;
		break;
	}
	case  FE_READ_AFC:
	{
		s32 *afc=(s32 *) arg;
		*afc=readreg(client, 5)<<16;
		*afc|=readreg(client, 6)<<8;
		*afc|=readreg(client, 7);
		break;
	}
	case FE_GET_INFO:
	{
		FrontendInfo *feinfo=(FrontendInfo *) arg;

		feinfo->type=FE_QPSK;
		feinfo->minFrequency=500; 
		feinfo->maxFrequency=2700000;
		feinfo->minSymbolRate=4500000;
		feinfo->maxSymbolRate=45000000;
		feinfo->hwType=0;    
		feinfo->hwVersion=0;
		break;
	}
	case FE_WRITEREG:
	{
		u8 *msg = (u8 *) arg;
		writereg(client, msg[0], msg[1]);
		break;
	}
	case FE_READREG:
	{
		u8 *msg = (u8 *) arg;
		msg[1]=readreg(client, msg[0]);
		break;
	}
	case FE_INIT:
	{
		return init(client);
	}
	case FE_SET_FRONTEND:
	{
		FrontendParameters * param = (FrontendParameters *) arg;

		tda->srate = param->u.qpsk.SymbolRate;
		tda->fec = param->u.qpsk.FEC_inner;
		tda->inversion = param->Inversion;
		SetSymbolrate(client);

		/* stop sweep, close loop */
		if (i2c_master_send(client, "\x0b\x68\x70", 3) != 3)
			printk("%s: i2c_master_send failed\n", __FILE__);
		break;
	}
	case FE_RESET:
	{
		return -EINVAL;
	}
	case FE_SEC_SET_TONE:
	{
		tda->tone = (secToneMode) arg;
		return tda_set_tone(client);
	}
	case FE_SEC_SET_VOLTAGE:
	{
		tda->power = (secVoltage) arg;
		return tda_set_voltage(client);
	}
	case FE_SEC_MINI_COMMAND:
	{
		printk("%s: FIXME: mini diseqc not implemented!\n", __FILE__);
		return -EINVAL;
	}
	case FE_SEC_COMMAND:
	{
		struct secCommand * command = (struct secCommand *) arg;
		u8 msg[SEC_MAX_DISEQC_PARAMS + 3];
		
		if (command->type == SEC_CMDTYPE_DISEQC)
			msg[0] = 0xE0;
		else if (command->type == SEC_CMDTYPE_DISEQC_RAW)
			msg[0] = command->u.diseqc.cmdtype;
		else
			return -EINVAL;

		msg[1] = command->u.diseqc.addr;
		msg[2] = command->u.diseqc.cmd;
		memcpy(msg + 3, command->u.diseqc.params, command->u.diseqc.numParams);
		return tda_send_diseqc(client, msg, command->u.diseqc.numParams + 3);
	}
	case FE_SEC_GET_STATUS:
	{
		return -EINVAL;
	}
	case FE_SETFREQ:
	{
		u32 freq = *(u32 *) arg;
		return tda_set_freq(client, freq);
	}
	default:
		return -EINVAL;
	}
	
	return 0;
}

static int tda_set_freq(struct i2c_client *client, u32 freq)
{
	int tries=10;
	u8 msg[4];
	
	dprintk("%s: setting freq %d\n", __FILE__, freq);

	// semaphore, irgendwie
	writereg(client, 0x1C, 0x80);
	
	msg[0]=(freq/1000)>>8;
	msg[1]=(freq/1000)&0xFF;
	msg[2]=0x81;
	msg[3]=0x60;

	if (i2c_master_send(&tuner, msg, 4)!=4)
		printk("%s: writereg error\n", __FILE__);

	writereg(client, 0x1C, 0x00);
	
	while (tries--)
	{
		udelay(10*1000);
		writereg(client, 0x1C, 0x80);
	
		if (i2c_master_recv(&tuner, msg, 1)!=1)
			printk("%s: read tuner error\n", __FILE__);
	
		writereg(client, 0x1C, 0x00);
		
		if (msg[0]&0x40)		// in-lock flag
		{
			dprintk("%s: tuner locked\n", __FILE__);
			return 0;
		}
	}
	
	dprintk("%s: tuner couldn't acquire lock.\n", __FILE__);
	return -1;
}

static int tda_set_voltage (struct i2c_client * client)
{
	u8 power;
	struct tda8044 * tda = (struct tda8044 *) client->data;

	switch (tda->power)
	{
	case SEC_VOLTAGE_OFF:
		power = 0x0F;
		break;

	case SEC_VOLTAGE_13:
	case SEC_VOLTAGE_13_5:
		power = 0x3F;
		break;

	case SEC_VOLTAGE_18:
	case SEC_VOLTAGE_18_5:
		power = 0xBF;
		break;

	case SEC_VOLTAGE_LT:
	default:
		power = 0x00;
		break;
	}

	writereg(client, 0x20, power);

	return 0;
}

static int tda_set_tone (struct i2c_client * client)
{
	struct tda8044 * tda = (struct tda8044 *) client->data;

	if (tda->tone == SEC_TONE_ON)
		writereg(client, 0x29, 0x80);
	else
		writereg(client, 0x29, 0x00);

	return 0;
}

static int tda_send_diseqc(struct i2c_client *client, u8 *cmd,unsigned int len)
{
	int i,a;

	if(len>6 || len<3)
	{
		dprintk("%s: wrong message size.\n", __FILE__);
		return 0;
	}
	for(a=0;a<2;a++)
	{
		writereg(client,0x29,8+(len-3));
		for(i=0;i<len;i++) writereg(client,0x23+i,cmd[i]);
		writereg(client,0x29,0x0C+(len-3));
	}
	return 0;
}

/*
static int tda_sec_status(void)
{
	return 0;
}
*/

static void tda_interrupt(int irq, void *vdev, struct pt_regs * regs)
{
	schedule_task(&tda_tasklet);
}

static void tda_task(void *data)
{
	struct i2c_client *client=(struct i2c_client *)data;
	struct tda8044 *tda=(struct tda8044*)client->data;
	u8 sync;

	writereg(client, 0, 4);
	enable_irq(TDA_INTERRUPT);

	sync = readreg(client, 2);

	if (((sync & 0x3F) == 0x1F) && ((tda->sync & 0x3F) != 0x1F))
	{
		u8 val = readreg(client, 0x0E);

		dprintk("%s: got full lock\n", __FILE__);

		/* store detected inversion */
		if (val & 0x80)
			tda->inversion = INVERSION_ON;
		else
			tda->inversion = INVERSION_OFF;

		/* store detected fec */
		switch (val & 0x07)
		{
		case 1:
			tda->fec = FEC_1_2;
			break;
		case 2:
			tda->fec = FEC_2_3;
			break;
		case 3:
			tda->fec = FEC_3_4;
			break;
		case 5:
			tda->fec = FEC_5_6;
			break;
		case 7:
			tda->fec = FEC_7_8;
			break;
		default:
			tda->fec = FEC_AUTO;
			break;
		}
	}

	if (sync & 0x20)
		tda->sync = 0x00;
	else
		tda->sync = sync & 0x1F;
}

static void inc_use (struct i2c_client *client)
{
#ifdef MODULE
	MOD_INC_USE_COUNT;
#endif
}

static void dec_use (struct i2c_client *client)
{
#ifdef MODULE
	MOD_DEC_USE_COUNT;
#endif
}


#ifdef MODULE
int init_module(void) {
	int res;
	
	if ((res = i2c_add_driver(&dvbt_driver))) 
	{
		printk("%s: Driver registration failed, module not inserted.\n", __FILE__);
		return res;
	}
	if (!instances)
	{
		printk("%s: demod not found.\n", __FILE__);
		i2c_del_driver(&dvbt_driver);
		return -EBUSY;
	}
	return 0;
}

void cleanup_module(void)
{
	int res;
	
	if ((res = i2c_del_driver(&dvbt_driver))) 
	{
		printk("%s: Driver deregistration failed, module not removed.\n", __FILE__);
	}
	dprintk("%s: cleanup\n", __FILE__);
}
#endif

