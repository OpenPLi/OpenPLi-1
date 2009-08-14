/* 
  $Id: ves1993.c,v 1.25.2.2 2003/02/20 00:41:28 ghostrider Exp $

		VES1993	- Single Chip Satellite Channel Receiver driver module
							 
		Copyright (C) 2001 Ronny Strutz	<3DES@tuxbox.org>

		This program is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.

		This program is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with this program; if not, write to the Free Software
		Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  $Log: ves1993.c,v $
  Revision 1.25.2.2  2003/02/20 00:41:28  ghostrider
  merge FE_READ_SNR, FE_READ_SIGNAL_STRENGTH, FE_READ_BER, FE_READ_UNCORRECED_BLOCKS with head.... i hope in enigma now the signals bars are correct...

  Revision 1.25.2.1  2002/10/23 22:56:18  obi
  bugfixes

  Revision 1.25  2002/06/01 11:11:10  happydude
  add module license

  Revision 1.24  2002/05/12 21:22:59  derget
  diseq solte nun gehen (untested)

  Revision 1.23  2002/05/08 00:54:39  derget
  minidiseq eingebaut (untestet)

  Revision 1.22  2002/05/08 00:10:32  derget
  support für nokia mit ves1993 und tsa5059 eingebaut
  diseqc tut da noch nicht
  sonst gehts *hoff*

  Revision 1.21  2002/05/05 12:07:37  happydude
  reception improvements

  Revision 1.20  2002/04/30 14:54:01  happydude
  fix ASTRA MTV-Transponder, please test thoroughly, it works on my 2 dishes

  Revision 1.19  2002/04/24 12:08:38  obi
  made framing byte hack nicer

  Revision 1.18  2002/04/20 18:23:16  obi
  added raw diseqc command

  Revision 1.17  2002/04/04 06:15:41  obi
  partially implemented FE_SEC_GET_STATUS

  Revision 1.16  2002/02/24 15:32:07  woglinde
  new tuner-api now in HEAD, not only in branch,
  to check out the old tuner-api should be easy using
  -r and date

  Revision 1.13.2.4  2002/02/14 20:16:47  TripleDES
  DiSEqC(tm) fix

  Revision 1.13.2.3  2002/01/22 23:44:56  fnbrd
  Id und Log reingemacht.

  
*/		


#include <linux/module.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <asm/io.h>

#include <linux/i2c.h>

#include <dbox/dvb_frontend.h>
#include <dbox/fp.h>
#include <ost/sec.h>

#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#ifdef MODULE
MODULE_PARM(debug,"i");
#endif

static struct i2c_driver dvbt_driver;
static struct i2c_client client_template, *dclient;

static int writereg(struct i2c_client *client, int reg, int data);

static int debug = 0;
#define dprintk	if (debug) printk
#define TUNER_I2C_DRIVERID	0xF0C2
//Tuner ----------------------------------------------------------------------
static int tuner_detach_client(struct i2c_client *tuner_client);
static int tuner_detect_client(struct i2c_adapter *adapter, int address, unsigned short flags,int kind);
static int tuner_attach_adapter(struct i2c_adapter *adapter);
int set_tuner_dword(u32 tw);

static unsigned short normal_i2c[] = { 0xc2>>1,I2C_CLIENT_END };
static unsigned short normal_i2c_range[] = { 0xc2>>1, 0xc2>>1, I2C_CLIENT_END };
I2C_CLIENT_INSMOD;

int mid = 3; // 3 Sagem , 1 Nokia 

struct tuner_data
{
	struct i2c_client *tuner_client;
};	
struct tuner_data *defdata=0;

static struct i2c_driver tuner_driver = {
	"sp5659/tsa5059 tuner driver",
	TUNER_I2C_DRIVERID,
	I2C_DF_NOTIFY,
	&tuner_attach_adapter,
	&tuner_detach_client,
	0,
	0,
	0,
};

static int tuner_detach_client(struct i2c_client *tuner_client)
{
	int err;
	
	if ((err=i2c_detach_client(tuner_client)))
	{
		dprintk("VES1993: couldn't detach tuner client driver.\n");
		return err;
	}
	
	kfree(tuner_client);
	return 0;
}

static int tuner_detect_client(struct i2c_adapter *adapter, int address, unsigned short flags,int kind)
{
	int err = 0;
	struct i2c_client *new_client;
	struct tuner_data *data;
	const char *client_name="DBox2 Tuner Driver";
	
	if (!(new_client=kmalloc(sizeof(struct i2c_client)+sizeof(struct tuner_data), GFP_KERNEL)))
	{
		return -ENOMEM;
	}
	
	new_client->data=new_client+1;
	defdata=data=(struct tuner_data*)(new_client->data);
	new_client->addr=address;
	data->tuner_client=new_client;
	new_client->data=data;
	new_client->adapter=adapter;
	new_client->driver=&tuner_driver;
	new_client->flags=0;
	
	strcpy(new_client->name, client_name);
	
	if ((err=i2c_attach_client(new_client)))
	{
		kfree(new_client);
		return err;
	}
	
	dprintk("VES1993: tuner attached @%02x\n", address>>1);
	
	return 0;
}

static int tuner_attach_adapter(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, &tuner_detect_client);
}

static int tuner_init(void)
{
	int res;
        char tsa5059init[4]= {0x06,0x5c,0x83,0x60}; // byte 4 oder 0xa0
	char sp5659init[4]= {0x25,0x70,0x92,0x40};  
	
	writereg(dclient, 0x00,0x11);	//enable tuner access on ves1993
	if ((res=i2c_add_driver(&tuner_driver)))
	{
		printk("VES1993: tuner driver registration failed!!!\n");
		return res;
	}
		
	if (!defdata)
	{
		i2c_del_driver(&tuner_driver);
		printk("VES1993: Couldn't find tuner.\n");
		return -EBUSY;
	}

	if (mid==1) {i2c_master_send(defdata->tuner_client, tsa5059init, 4); printk("VES1993: tsa5059 tuner found\n");}
	   else {i2c_master_send(defdata->tuner_client, sp5659init, 4); printk("VES1993: sp5659 tuner found\n");}

	writereg(dclient, 0x00,0x01);	//disable tuner access on ves1993
	
	return 0;
}

static int tuner_close(void)
{
	int res;
	
	if ((res=i2c_del_driver(&tuner_driver)))
	{
		printk("VES1993: tuner driver unregistration failed.\n");
		return res;
	}
	
	return 0;
}
		
int set_tuner_dword(u32 tw)
{
	char msg[4];
	char tmp[2];
	*((u32*)(msg))=tw;
	tmp[0]=msg[0];
	tmp[1]=msg[1];

	writereg(dclient, 0x00,0x11);
	if (i2c_master_send(defdata->tuner_client, tmp, 2)!=2)
	{
		return -1;
	}
	writereg(dclient, 0x00,0x01);
	return -1;

}

	/* KILOHERTZ!!! */

static int tuner_set_freq(int freq)
{
        u8 buffer[4];
        
	freq/=1000; 
        
	buffer[0]=(freq>>8) & 0x7F;
        buffer[1]=freq & 0xFF;

        set_tuner_dword(*((u32*)buffer));

        return 0;
}       



//----------------------------------------------------------------------------


static u8 Init1993_sagemTab[] =
{
				0x00, 0x9c, 0x35, 0x80, 0x6a, 0x29, 0x72, 0x8c,		// 0x00
				0x09, 0x6b, 0x00, 0x00, 0x4c, 0x08, 0x00, 0x00,		// 0x08
				0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 0x10
				0x80, 0x40, 0x21, 0xb0, 0x00, 0x00, 0x00, 0x10,		// 0x18
				0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 0x20
				0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00,		// 0x28
				0x00, 0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x03,		// 0x30
				0x00, 0x00, 0x0e, 0x80, 0x00				// 0x38
};


static u8 Init1993_nokiaTab[] =
{
				0x00, 0x94, 0x00, 0x80, 0x6a, 0x0b, 0xab, 0x2a,		// 0x00 
				0x09, 0x70, 0x00, 0x00, 0x4c, 0x02, 0x00, 0x00,     	// 0x08
				0x00, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 0x10
				0x80, 0x40, 0x21, 0xb0, 0x00, 0x00, 0x00, 0x00,		// 0x18
				0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// 0x20
				0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00,		// 0x28
				0x00, 0x55, 0x03, 0x00, 0x00, 0x00, 0x01, 0x03,  	// 0x30
				0x00, 0x00, 0x0e, 0xfd, 0x56   				// 0x38
};


static u8 Init1993WTab[] =
{	//			0 1 2 3 4 5 6 7	 8 9 a b c d e f
				0,1,1,1,1,1,1,1, 1,1,0,0,1,1,0,0,			// 0x00
				0,1,0,0,0,0,0,0, 1,1,1,1,0,0,0,1,			// 0x10
				1,1,1,0,0,0,0,0, 0,0,1,1,0,0,0,0,			// 0x20
				1,1,1,0,1,1,1,1, 1,1,1,1,1				// 0x30
};

struct ves1993 {
	u32 srate;
	u8 ctr;
	u8 fec;
	u8 inv;
	
	int power, tone;
	
	dvb_front_t frontend;
};

static int writereg(struct i2c_client *client, int reg, int data)
{
	int ret;
	unsigned char msg[] = {0x00, 0x1f, 0x00};
				
	msg[1]=reg; msg[2]=data;
	ret=i2c_master_send(client, msg, 3);
	if (ret!=3) 
		dprintk("writereg error\n");
	return ret;
}

static u8 readreg(struct i2c_client *client, u8 reg)
{
	struct i2c_adapter *adap=client->adapter;
	unsigned char mm1[] = {0x00, 0x1e};
	unsigned char mm2[] = {0x00};
	struct i2c_msg msgs[2];
				
	msgs[0].flags=0;
	msgs[1].flags=I2C_M_RD;
	msgs[0].addr=msgs[1].addr=client->addr;
	mm1[1]=reg;
	msgs[0].len=2; msgs[1].len=1;
	msgs[0].buf=mm1; msgs[1].buf=mm2;
	i2c_transfer(adap, msgs, 2);
				
	return mm2[0];
}

static int init(struct i2c_client *client)
{
	struct ves1993 *ves=(struct ves1993 *) client->data;
	int i;
					
	if (writereg(client, 0, 0)<0)
		dprintk("ves1993: send error\n");
		
	//Init fuer VES1993

	if(mid==1) {
	dprintk("nokia\n");
	writereg(client,0x3a, 0x0e); 	

	for (i=0; i<0x3d; i++)
		if (Init1993WTab[i])
		writereg(client, i, Init1993_nokiaTab[i]);
				
	writereg(client,0x00, 0x01); 
			
	ves->ctr=Init1993_nokiaTab[0x1f];
	ves->srate=0;
	ves->fec=9;
	ves->inv=0;
	}
	else {
        dprintk("sagem\n");
	writereg(client,0x3a, 0x0c); 

        for (i=0; i<0x3d; i++)
                if (Init1993WTab[i])
                writereg(client, i, Init1993_sagemTab[i]);
        
        writereg(client,0x00, 0x01);
 
        ves->ctr=Init1993_sagemTab[0x1f];
        ves->srate=0;
        ves->fec=9;  
        ves->inv=0;
        }

	return 0;
}

static inline void ddelay(int i) 
{
	current->state=TASK_INTERRUPTIBLE;
	schedule_timeout((HZ*i)/100);
}

static void ClrBit1893(struct i2c_client *client)
{
	writereg(client, 0, 0);
	writereg(client, 0, 1);
}

static int SetFEC(struct i2c_client *client, u8 fec)
{
	struct ves1993 *ves=(struct ves1993 *) client->data;
				
	if (fec>=8) 
		fec=8;
	if (ves->fec==fec)
		return 0;
	ves->fec=fec;
	return writereg(client, 0x0d, 8);
}

static int SetSymbolrate(struct i2c_client *client, u32 srate, int doclr)
{
	struct ves1993 *ves=(struct ves1993 *) client->data;
	u32 BDR;
	u32 ratio;
	u8	ADCONF, FCONF, FNR;
	u32 BDRI;
	u32 tmp;
	unsigned long XIN;
	unsigned long FIN;
			
	if (ves->srate==srate) 
	{
		if (doclr)
			ClrBit1893(client);
		return 0;
	}

	if (mid==1) {
		XIN = 96000000UL; // ves1993 Nokia Crystal ist 96 Mhz (6 * 16 Mhz)
	} else {
		XIN = 92160000UL; // ves1993 sagem Crystal ist 92,16 MHz 
	}
	FIN = XIN >> 4;

	if (srate>XIN/2)
								srate=XIN/2;
				if (srate<500000)
								srate=500000;
				ves->srate=srate;
				
#define MUL (1UL<<26)
				tmp=srate<<6;
	ratio=tmp/FIN;

	tmp=(tmp%FIN)<<8;
	ratio=(ratio<<8)+tmp/FIN;
	 	
	tmp=(tmp%FIN)<<8;
	ratio=(ratio<<8)+tmp/FIN;		 
	
	FNR = 0xFF;
	
	if (ratio < MUL/3)					FNR = 0;
	if (ratio < (MUL*11)/50)	 			FNR = 1;
	if (ratio < MUL/6)					FNR = 2;
	if (ratio < MUL/9)					FNR = 3;
	if (ratio < MUL/12)					FNR = 4;
	if (ratio < (MUL*11)/200)				FNR = 5;
	if (ratio < MUL/24)					FNR = 6;
	if (ratio < (MUL*27)/1000)				FNR = 7;
	if (ratio < MUL/48)					FNR = 8;
	if (ratio < (MUL*137)/10000)				FNR = 9;

	if (FNR == 0xFF)
	{
		ADCONF = 0x89;		//bypass Filter
		FCONF	= 0x80;		//default
		FNR	= 0;
	} else	{
		ADCONF = 0x81;
		FCONF	= 0x80 | ((FNR > 1) << 3) | (FNR >> 1) | ((FNR & 0x01) << 5); //default | DFN | AFS
	}


	BDR = ((	(ratio<<(FNR>>1))	>>4)+1)>>1;
	BDRI = (	((FIN<<8) / ((srate << (FNR>>1))>>2)	) +1 ) >> 1;

	if (BDRI > 0xFF)
					BDRI = 0xFF;

	writereg(client, 0x20, ADCONF);
	writereg(client, 0x21, FCONF);

#if 0	// This makes reception worse in my experience
	if (srate<6000000) 
		writereg(client, 5, Init1993Tab[0x05] | 0x80);
	else
		writereg(client, 5, Init1993Tab[0x05] & 0x7f);
#endif

	writereg(client, 0x00,0x00);
	writereg(client, 6, 0xff&BDR);
	writereg(client, 7, 0xff&(BDR>>8));
	writereg(client, 8, 0x0f&(BDR>>16));
	writereg(client, 9, BDRI);
	writereg(client, 0x00,0x01);

	return 0;
}

static int attach_adapter(struct i2c_adapter *adap)
{
	struct ves1993 *ves;
	struct i2c_client *client;

	client_template.adapter=adap;
	client_template.adapter=adap;
				
	if ((readreg(&client_template, 0x1e)&0xf0)!=0xd0)
	{
		if ((readreg(&client_template, 0x1a)&0xF0)==0x70)
			printk("warning, no ves1993 found but a VES1820\n");
		return -1;
	}
	dprintk("feID: 1893 %x\n", readreg(&client_template, 0x1e));
				
	if (NULL == (client = kmalloc(sizeof(struct i2c_client), GFP_KERNEL)))
		return -ENOMEM;
	memcpy(client, &client_template, sizeof(struct i2c_client));
	dclient=client;
				
	client->data=ves=kmalloc(sizeof(struct ves1993),GFP_KERNEL);
	if (ves==NULL) {
		kfree(client);
		return -ENOMEM;
	}
			 
	i2c_attach_client(client);
	init(client);
	
	ves->frontend.type=DVB_S;
	ves->frontend.capabilities=0;
	ves->frontend.i2cbus=adap;
	
	ves->frontend.demod=client;
	ves->frontend.demod_type=DVB_DEMOD_VES1993;
	
	register_frontend(&ves->frontend);

	return 0;
}

static int detach_client(struct i2c_client *client)
{
	unregister_frontend(&((struct ves1993*)client->data)->frontend);
	i2c_detach_client(client);
	kfree(client->data);
	kfree(client);
	return 0;
}

static const uint8_t fectab[8]={8,0,1,2,4,6,0,8};
static const uint8_t fectab2[9]={1,2,3,-1,4,-1,5,-1,0};

static int dvb_command(struct i2c_client *client, unsigned int cmd, void *arg)
{
	struct ves1993 *ves=(struct ves1993 *)client->data;
	switch (cmd)
	{
	case FE_READ_STATUS:
	{
		FrontendStatus *status=(FrontendStatus*)arg;
		int sync=readreg(dclient, 0x0E);
		*status=0;
		if (sync&1)
			*status|=FE_HAS_SIGNAL;
		if (sync&2)
			*status|=FE_HAS_CARRIER;
		if (sync&4)
			*status|=FE_HAS_VITERBI;
		if (sync&8)
			*status|=FE_HAS_SYNC;
		if ((sync&0x1F)==0x1F)
			*status|=FE_HAS_LOCK;
		if (readreg(client, 0x0F)&0x2)
			*status|=FE_SPECTRUM_INV;
		break;
	}
	case FE_READ_BER:
	{
		u32 *ber=(u32 *) arg;

		*ber = readreg(client,0x15);
		*ber|=(readreg(client,0x16)<<8);
		*ber|=((readreg(client,0x17)&0x0F)<<16);
		*ber*=10;
		break;
	}
	case FE_READ_SIGNAL_STRENGTH:
	{
		u8 signal = ~readreg(client,0x0b);
		*((u16*) arg) = (signal << 8) | signal;
		break;
	}
	case FE_READ_SNR:
	{
		u8 snr = ~readreg(client,0x1c);
		*(u16*) arg = (snr << 8) | snr;
		break;
	}
	case FE_READ_UNCORRECTED_BLOCKS:
	{
		*(u32*) arg = readreg(client, 0x18) & 0x7f;

		if (*(u32*) arg == 0x7f)
			*(u32*) arg = 0xffffffff;   /* counter overflow... */
		writereg (client, 0x18, 0x00);  /* reset the counter */
		writereg (client, 0x18, 0x80);  /* dto. */
		break;
	}
	case FE_READ_AFC:
	{
		s32 *afc=(s32 *) arg;
		
		*afc=((int)((char)(readreg(client,0x0a)<<1)))/2;
		*afc=(*afc*(int)(ves->srate/8))/16;
		break;
	}
	case FE_GET_INFO:
	{
		FrontendInfo *feinfo=(FrontendInfo *) arg;

		feinfo->type=FE_QPSK;
		feinfo->minFrequency=500; 
		feinfo->maxFrequency=2700000;
		feinfo->minSymbolRate=500000;
		feinfo->maxSymbolRate=30000000;
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
		init(client);
		break;
	}
	case FE_SET_FRONTEND:
	{
		FrontendParameters *param = (FrontendParameters *) arg;
		
		if (ves->inv!=param->Inversion)
		{
			ves->inv=param->Inversion;
			if (mid==1) {writereg(dclient, 0x0c, Init1993_nokiaTab[0x0c] ^ (ves->inv ? 0x40 : 0x00));}
			    else {writereg(dclient, 0x0c, Init1993_sagemTab[0x0c] ^ (ves->inv ? 0x40 : 0x00));}
					
			ClrBit1893(dclient);
		}
		SetFEC(dclient, fectab[param->u.qpsk.FEC_inner]);							
		SetSymbolrate(dclient, param->u.qpsk.SymbolRate, 1);
		break;
	}
	case FE_RESET:
	{
		ClrBit1893(client);
		break;
	}
	case FE_SEC_SET_TONE:
	{
		secToneMode mode=(secToneMode)arg;
		ves->tone=(mode==SEC_TONE_ON)?1:0;
		if (mid==1) return fp_set_sec(ves->power, ves->tone);
		else return fp_sagem_set_SECpower(ves->power, ves->tone);
	}

	case FE_SEC_SET_VOLTAGE:
	{
		secVoltage volt=(secVoltage)arg;
		switch (volt)
		{
		case SEC_VOLTAGE_OFF:
			ves->power=0;
			break;
		case SEC_VOLTAGE_LT:
			ves->power=-2;
			break;
		case SEC_VOLTAGE_13:
			ves->power=1;
			break;
		case SEC_VOLTAGE_13_5:
			ves->power=2;
			break;
		case SEC_VOLTAGE_18:
			ves->power=3;
			break;
		case SEC_VOLTAGE_18_5:
			ves->power=4;
			break;
		default:
			dprintk("invalid voltage\n");
		}
		if (mid==1) return fp_set_sec(ves->power, ves->tone);
		else return fp_sagem_set_SECpower(ves->power, ves->tone);

	}
	case FE_SEC_MINI_COMMAND:
	{
	if (mid==1) {
                secMiniCmd minicmd = (secMiniCmd) arg;
                 
                switch (minicmd) {
                case SEC_MINI_A:
                        dprintk ("minidiseqc: A\n");   
                        return fp_send_diseqc (1, "\x00\x00\x00\x00", 4);
                        
                case SEC_MINI_B:
                        dprintk ("minidiseqc: B\n");
                        return fp_send_diseqc (1, "\xff", 1);
                 
                default:
                        break;
                }
		}
	 else {		    
		//secMiniCmd minicmd=(secMiniCmd)arg;
		//return fp_send_diseqc(1, (minicmd==SEC_MINI_A)?"\xFF\xFF\xFF\xFF":"\x00\x00\x00\x00", 4); // das ist evtl. falschrum
		return 0;
              }
		break;
	}
	case FE_SEC_COMMAND:
	{
		struct secCommand *command=(struct secCommand*)arg;
		switch (command->type) {
		case SEC_CMDTYPE_DISEQC:
		{
			unsigned char msg[SEC_MAX_DISEQC_PARAMS+3];
			dprintk("[VES1993] SEND DiSEqC\n");
			msg[0]=0xE0;
			msg[1]=command->u.diseqc.addr;
			msg[2]=command->u.diseqc.cmd;
			memcpy(msg+3, command->u.diseqc.params, command->u.diseqc.numParams);
			if (mid==1) {
				    	return fp_send_diseqc(1, msg, command->u.diseqc.numParams+3);
				    } 
				else { 
					return fp_send_diseqc(2, msg, command->u.diseqc.numParams+3);
			     	     }
		}
                case SEC_CMDTYPE_DISEQC_RAW:
		{
			unsigned char msg[SEC_MAX_DISEQC_PARAMS+3];
			dprintk("[VES1993] SEND DiSEqC_RAW\n");
			msg[0]=command->u.diseqc.cmdtype;
			msg[1]=command->u.diseqc.addr;
			msg[2]=command->u.diseqc.cmd;
			memcpy(msg+3, command->u.diseqc.params, command->u.diseqc.numParams);
                        if (mid==1) {
                                        return fp_send_diseqc(1, msg, command->u.diseqc.numParams+3);
                                    }
                                else {
					return fp_send_diseqc(2, msg, command->u.diseqc.numParams+3);
				     }
		}
		default:
			return -EINVAL;
		}
		break;
	}
	case FE_SEC_GET_STATUS:
	{
                struct secStatus status;

		/* todo: implement */
		status.busMode=SEC_BUS_IDLE;

		switch(ves->power)
		{
		case -2:
			status.selVolt=SEC_VOLTAGE_LT;
			break;
		case 0:
			status.selVolt=SEC_VOLTAGE_OFF;
			break;
		case 1:
			status.selVolt=SEC_VOLTAGE_13;
			break;
		case 2:
			status.selVolt=SEC_VOLTAGE_13_5;
			break;
		case 3:
			status.selVolt=SEC_VOLTAGE_18;
			break;
		case 4:
			status.selVolt=SEC_VOLTAGE_18_5;
			break;
		default:
			return -EINVAL;
		}

		status.contTone=(ves->tone ? SEC_TONE_ON : SEC_TONE_OFF);

		if (copy_to_user(arg, &status, sizeof(status)))
			return -EFAULT;

		break;
	}
	case FE_SETFREQ:
		tuner_set_freq(*(u32*)arg);
		break;
	default:
		return -1;
	}
	return 0;
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

static struct i2c_driver dvbt_driver = {
	"VES1993 DVB DECODER",
	I2C_DRIVERID_VES1993,
	I2C_DF_NOTIFY,
	attach_adapter,
	detach_client,
	dvb_command,
	inc_use,
	dec_use,
};

static struct i2c_client client_template = {
	"VES1993",
	I2C_DRIVERID_VES1993,
	0,
	(0x10 >> 1),
	NULL,
	&dvbt_driver,
	NULL
};

/* ---------------------------------------------------------------------- */

#ifdef MODULE
int init_module(void) {
	int res;
	unsigned char *conf=(unsigned char*)ioremap(0x1001FFE0, 0x20);
	if (conf[0]==1) mid=1; // 1 Nokia , 3 Sagem

	if ((res = i2c_add_driver(&dvbt_driver))) 
	{
		printk("ves1993: Driver registration failed, module not inserted.\n");
		return res;
	}
	if (!dclient)
	{
		printk("ves1993: not found.\n");
		i2c_del_driver(&dvbt_driver);
		return -EBUSY;
	}
				
	tuner_init();
	
	return 0;
}

void cleanup_module(void)
{
	int res;
				
	if ((res = i2c_del_driver(&dvbt_driver))) 
	{
		printk("dvb-tuner: Driver deregistration failed, module not removed.\n");
	}
	dprintk("ves1993: cleanup\n");
	tuner_close();
}
#endif
