/*
   $Id: ves1820.c,v 1.34 2002/10/08 23:18:42 obi Exp $

    VES1820  - Single Chip Cable Channel Receiver driver module
               used on the the Siemens DVB-C cards

    Copyright (C) 1999 Convergence Integrated Media GmbH <ralph@convergence.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/i2c.h>

#include <dbox/dvb_frontend.h>
#include <dbox/fp.h>


static int debug = 0;
#define dprintk	if (debug) printk


#define SET_PWM(i2c,x) do { ((struct ves1820 *)i2c->data)->pwm = x; } while (0)
#define GET_PWM(i2c) (((struct ves1820 *)i2c->data)->pwm)

#define SET_REG0(i2c,x) do { ((struct ves1820 *)i2c->data)->reg0 = x; } while (0)
#define GET_REG0(i2c) (((struct ves1820 *)i2c->data)->reg0)




static
FrontendInfo ves1820_info = {
        type: FE_QAM,
        minFrequency: 49000000,
        maxFrequency: 890000000,
        minSymbolRate: 543750,
        maxSymbolRate: 8700000,
        hwType: 0,
        hwVersion: 0
};



u8 ves1820_inittab [] =
{
  0x49, //0  noStdBy / internADC / noInversion / 64-QAM / outputs active / CLB-Softreset default
  0x6A, //1  AGC-Reference set to 64-QAM
  0x13, //2  Frontend Lock-Indicator /  internADC / AGC in PWM ?? (def=1) / AGC nonInverted / AGC Time constant is minimum (def=00)
  0x0A, //3  NDEC=0 / gain(3.decimation filter)=1 / gain(2.decimation filter)=1 / DYN=1 acquisition range for clock recovery is ± 240 ppm / CLK_C=def.
  0x15, //4  carrier acquisition range is ± 2.5% / loop bandwith (BW/RS)=0.03
  0x46, //5  carrier lock detection = 64-QAM
  0x26, //6  reference coefficient of the equalizer=def. / coefficients of the equalizer are continuously adjusted / linear transversal equalizer
  0x1A, //7  convergence steps during the acquisition phase / tracking phase
  0x43, //8  threshold value used to switch from the acquisition phase to the tracking phase=64-QAM
  0x6A, //9  reference parameter that optimizes the equalizer convergence during the acquisition phase=64-QAM
  0x1A, //A  LSB of the BAUD rate frequency to program
  0x61, //B  MID of the BAUD rate frequency to program
  0x19, //C  MSB of the BAUD rate frequency to program
  		//   according to int((2^24*SymbolRate*2^NDEC)/SYSCLK)
  0xA1, //D  BAUD rate inverse (If BDRI > 255, then set BDRI to 255)
  		//   according to int(16*SYSCLK /(SymbolRate*2^NDEC))
  0x63, //E  gain of the Nyquist filter=3 GNYQ=011 / SFIL=0 / gain of the antialiasing filter=1 GLPF=00 / number of samples=16384 SSAT=11
  0x00, //F  Test-Byte
  0xB8, //10 PVBER=10 (def=01) / CLB_UNC=1 / sync-unsync detection C=11 / correction capability of RS decoder validated RSI=0 / descrambler validated DESCI=0 / IEI=0
  0x00, //11 Status-Read-Register: 0 / (FEL but) NODVB / rough idea of the Bit Error Rate: BER[1] BER[0] / DVB lock: FEL=(FSYNC&&CARLOCK) / MPEG2 sync: if(FSYNC) / CarrierLock: if(CARLOCK) / Phase detection: acquisition=0 tracking=1 EQ_ALGO
  0xE1, //12 if some output Pins inverted or not
  0x00, //13 7-bit counter uncorractable Packets
  0x00, //14 8-bit LSB of the BER=OUTPUT SIGNAL QUALITY MEASUREMENT (BER)
  0x00, //15 8-bit MID of the BER
  0x00, //16 4-bit MSB of the BER
  0x00, //17 AGC information 0xFF is maximum
  0x00, //18 MSE Mean Square Error = representation of the signal quality
  0x00, //19 VAFC indicates the frequency offset when the carrier has been recovered
  		//	 sigmaF = (VAFC * SymbolRate) / 1024
  0x00, //1A IDENTITY
  0x01, //1B if(PDOWN) internal ADC in STDBY / PCLK polarity of SamplingClock SACLOCK if internal ADC is used
  0x00, //1C EQCONF2
  0x00, //1D CKOFF symbol clock frequency offset
  		//   If DYN=0 SRoffset = (CKOFF * 120 / 128) ppm
		//	 If DYN=1 SRoffset = (CKOFF * 240 / 128) ppm
  0x00, //1E PLL config
  0x00, //1F
  0x00, //20 INTSEL config
  0x00, //21 SATNYQ represents the number of saturations that occur at the output of the Nyquist filter
  0x00, //22 SATADC represents the number of saturations that occur at the output of the ADC
  0x00, //23 HALFADC HLFADC represents the number of times that the output of the ADC exceeds the mid-range
  0x00, //24 SATDEC1 SDEC1represents the number of saturations that occur at the output of first decimation filter
  0x00, //25 SATDEC2 SDEC2 represents the number of saturations that occur at the output of second decimation filter
  0x00, //26 SATDEC3 SDEC3 represents the number of saturations that occur at the output of third decimation filter
  0x00, //27 SATAAF SAAF represents the number of saturations that occur at the output of the antialiasing filter
  0x00, //28
  0x00, //29
  0x00, //2A
  0x00, //2B
  0x00, //2C
  0x00, //2D
  0x00, //2E
  0x00, //2F
  0x01, //30 SATTHR STHR is a threshold value, compared to the register SATADC. If SATADC > STHR then an interrupt can be generated on pin IT. (See register ITSEL)
  0x32, //31 HALFTHR HLFTHR is a threshold value, compared to the register HLFADC
		//	 If HLFADC < HLFTHR then an interrupt can be generated on pin IT (See register ITSEL)
  0xc8, //32 ITSEL ITEN=INTerrupt Enable / ITSEL[6]=interrupt if AGC is saturated (AGC = 0 or ACG = 255) / ITSEL[3]=interrupt if VBER is refreshed
  0x00, //33 ITSTAT interrupt status register corresponds to ITSEL(i)
  0x00  //34 PWM off
};


struct ves1820 {
        u8 pwm;
        u8 reg0;
	dvb_front_t frontend;
};



static
int ves1820_writereg (struct i2c_client *i2c, u8 reg, u8 data)
{
        int ret;
        u8 buf[] = { 0x00, reg, data };
        struct i2c_msg msg = { addr: i2c->addr, flags: 0, buf: buf, len: 3};

        ret = i2c_transfer(i2c->adapter, &msg, 1);

        if (ret != 1)
                dprintk("%s: writereg error "
                        "(reg == 0x%02x, val == 0x%02x, ret == %i)\n",
                        __FUNCTION__, reg, data, ret);

        mdelay(10);
        return (ret != 1) ? -EREMOTEIO : 0;
}


static
u8 ves1820_readreg (struct i2c_client *i2c, u8 reg)
{
        int ret;
        u8 b0 [] = { 0x00, reg };
        u8 b1 [] = { 0 };
        struct i2c_msg msg [] = { { addr: i2c->addr, flags: 0, buf: b0, len: 2 },
                           { addr: i2c->addr, flags: I2C_M_RD, buf: b1, len: 1 } };

        ret = i2c_transfer(i2c->adapter, msg, 2);

        if (ret != 2)
                dprintk("%s: readreg error (ret == %i)\n", __FUNCTION__, ret);

        return b1[0];
}


static
int tuner_set_tv_freq (u32 freq)
{
        u32 div = (freq + 36125) / 125;

        u8 buf [4] = { (div >> 8) & 0x7f, div & 0xff,
                        0x80 | (((div >> 15) & 0x03) << 6) | 0x04,
                        div > 4017 ? 0x04 : div < 2737 ? 0x02 : 0x01 };

        return fp_set_tuner_dword(T_QAM, *((u32*)buf));
}


static
int ves1820_init (struct i2c_client *i2c)
{
        u8 b0 [] = { 0xff };
        u8 pwm;
        int i;
        struct i2c_msg msg [] = { { addr: (0x28 << 1), flags: 0, buf: b0, len: 1 },
                           { addr: (0x28 << 1), flags: I2C_M_RD, buf: &pwm, len: 1 } };

        dprintk("VES1820: init chip\n");

        i2c_transfer(i2c->adapter, msg, 2);

        dprintk("VES1820: pwm=%02x\n", pwm);

        if (pwm == 0xff)
                pwm=0x48;

        ves1820_writereg (i2c, 0, 0);

        for (i=0; i<53; i++)
                ves1820_writereg (i2c, i, ves1820_inittab[i]);

        ves1820_writereg (i2c, 0x34, pwm);

        SET_PWM(i2c,pwm);
        SET_REG0(i2c,ves1820_inittab[0]);

	return 0;
}


static
int ves1820_setup_reg0 (struct i2c_client *i2c,
                        u8 real_qam, SpectralInversion inversion)
{
        u8 reg0 = (ves1820_inittab[0] & 0xe3) | (real_qam << 2);

        switch (inversion) {
        case INVERSION_ON:      /* XXX FIXME: reversed?? p. 25  */
                reg0 |= 0x20;
                break;

        case INVERSION_OFF:
                reg0 &= 0xdf;
                break;

        default:
                return -EINVAL;
        }

        SET_REG0(i2c, reg0);

        ves1820_writereg (i2c, 0x00, reg0 & 0xfe);
        ves1820_writereg (i2c, 0x00, reg0);

        return 0;
}


static
int ves1820_reset (struct i2c_client *i2c)
{
        u8 reg0 = GET_REG0(i2c);

        ves1820_writereg(i2c, 0x00, reg0 & 0xfe);
        ves1820_writereg(i2c, 0x00, reg0);

        return 0;
}


static
void ves1820_reset_uncorrected_block_counter (struct i2c_client *i2c)
{
        ves1820_writereg (i2c, 0x10, ves1820_inittab[0x10] & 0xdf);
        ves1820_writereg (i2c, 0x10, ves1820_inittab[0x10]);
}


static
int ves1820_set_symbolrate (struct i2c_client *i2c, u32 symbolrate)
{
        s32 BDR;
        s32 BDRI;
        s16 SFIL=0;
        u16 NDEC = 0;
        u32 tmp, ratio;

#define XIN 69600000UL
#define FIN (69600000UL>>4)

        if (symbolrate > XIN/2)
                symbolrate = XIN/2;

        if (symbolrate < 500000)
                symbolrate = 500000;

        if (symbolrate < XIN/16) NDEC = 1;
        if (symbolrate < XIN/32) NDEC = 2;
        if (symbolrate < XIN/64) NDEC = 3;

        if (symbolrate < (u32)(XIN/12.3)) SFIL = 1;
        if (symbolrate < (u32)(XIN/16))  SFIL = 0;
        if (symbolrate < (u32)(XIN/24.6)) SFIL = 1;
        if (symbolrate < (u32)(XIN/32))  SFIL = 0;
        if (symbolrate < (u32)(XIN/49.2)) SFIL = 1;
        if (symbolrate < (u32)(XIN/64))  SFIL = 0;
        if (symbolrate < (u32)(XIN/98.4)) SFIL = 1;

        symbolrate <<= NDEC;
        ratio = (symbolrate << 4) / FIN;
        tmp =  ((symbolrate << 4) % FIN) << 8;
        ratio = (ratio << 8) + tmp / FIN;
        tmp = (tmp % FIN) << 8;
        ratio = (ratio << 8) + (tmp + FIN/2) / FIN;

        BDR = ratio;
        BDRI = (((XIN << 5) / symbolrate) + 1) / 2;

        if (BDRI > 0xFF)
                BDRI = 0xFF;

        SFIL = (SFIL << 4) | ves1820_inittab[0x0E];
        NDEC = (NDEC << 6) | ves1820_inittab[0x03];

        ves1820_writereg (i2c, 0x03, NDEC);
        ves1820_writereg (i2c, 0x0a, BDR&0xff);
        ves1820_writereg (i2c, 0x0b, (BDR>> 8)&0xff);
        ves1820_writereg (i2c, 0x0c, (BDR>>16)&0x3f);

        ves1820_writereg (i2c, 0x0d, BDRI);
        ves1820_writereg (i2c, 0x0e, SFIL);

	return 0;
}


static
void ves1820_reset_pwm (struct i2c_client *i2c)
{
        u8 pwm = GET_PWM(i2c);

        ves1820_writereg (i2c, 0x34, pwm);
}


typedef struct {
        Modulation      QAM_Mode;
        int             NoOfSym;
        u8              Reg1;
        u8              Reg5;
        u8              Reg8;
        u8              Reg9;
} QAM_SETTING;


QAM_SETTING QAM_Values[] = {
        {  QAM_16,  16, 145, 164, 162, 145 },
        {  QAM_32,  32, 150, 120, 116, 150 },
        {  QAM_64,  64, 106,  70,  67, 106 },
        { QAM_128, 128, 126,  54,  52, 126 },
        { QAM_256, 256, 107,  38,  35, 107 }
};


static
int ves1820_set_parameters (struct i2c_client *i2c, FrontendParameters *p)
{
        int real_qam;

        switch (p->u.qam.QAM) {
        case QAM_16 : real_qam = 0; break;
        case QAM_32 : real_qam = 1; break;
        case QAM_64 : real_qam = 2; break;
        case QAM_128: real_qam = 3; break;
        case QAM_256: real_qam = 4; break;
        default:
                return -EINVAL;
        }

        ves1820_set_symbolrate (i2c, p->u.qam.SymbolRate);
        ves1820_reset_pwm (i2c);

        ves1820_writereg (i2c, 0x01, QAM_Values[real_qam].Reg1);
        ves1820_writereg (i2c, 0x05, QAM_Values[real_qam].Reg5);
        ves1820_writereg (i2c, 0x08, QAM_Values[real_qam].Reg8);
        ves1820_writereg (i2c, 0x09, QAM_Values[real_qam].Reg9);

        ves1820_setup_reg0 (i2c, real_qam, p->Inversion);

        return 0;
}


static
void ves1820_sleep (struct i2c_client *i2c)
{
        ves1820_writereg (i2c, 0x1b, 0x02);  /* pdown ADC */
        ves1820_writereg (i2c, 0x00, 0x80);  /* standby */
}


static int ves1820_ioctl (struct i2c_client *i2c, unsigned int cmd, void *arg)
{
        switch (cmd) {
        case FE_GET_INFO:
                memcpy (arg, &ves1820_info, sizeof(FrontendInfo));
                break;

        case FE_READ_STATUS:
        {
                FrontendStatus *status=(FrontendStatus *) arg;
                int sync;

                *status = 0;

                sync = ves1820_readreg (i2c, 0x11);

                if (sync & 0x01)
                        *status |= FE_HAS_POWER;

                if (sync & 0x02)
                        *status |= FE_HAS_SIGNAL;

                if (sync & 0x02)
                        *status |= FE_HAS_CARRIER;

                if (sync & 0x02)        /* XXX FIXME! */
                        *status |= FE_HAS_VITERBI;

                if (sync & 0x04)
                        *status |= FE_HAS_SYNC;

                if (sync & 0x08)
                        *status|=FE_HAS_LOCK;

                if (sync & 0x40)
                        *status|=FE_SPECTRUM_INV;

                dprintk ("ber: approx. 10E-%d, sync: %d, status; %d\n", (sync >> 4) + 2, sync, *status);
                break;
        }

        case FE_READ_BER:
        {
                u32 *ber = (u32 *) arg;

                *ber = ves1820_readreg (i2c, 0x14);
                *ber |= (ves1820_readreg (i2c, 0x15) << 8);
                *ber |= ((ves1820_readreg (i2c, 0x16) & 0x0F ) << 16);
                *ber *= 10;
                break;
        }

        case FE_READ_SIGNAL_STRENGTH:
        {
                u8 gain = ves1820_readreg (i2c, 0x17);
                *(s32*)arg = (gain << 8) | gain;
                break;
        }

        case FE_READ_SNR:
        {
                u8 quality = ~ves1820_readreg (i2c, 0x18);
                *(s32*)arg = (quality << 8) | quality;
                break;
        }

        case FE_READ_UNCORRECTED_BLOCKS:
                *(u32*)arg = ves1820_readreg(i2c, 0x13) & 0x7f;
                ves1820_reset_uncorrected_block_counter (i2c);
                break;

        case FE_SET_FRONTEND:
                return ves1820_set_parameters (i2c, arg);

        case FE_GET_FRONTEND:
                break;
#if 0
        case FE_SLEEP:
                ves1820_sleep (i2c);
                break;
#endif
        case FE_INIT:
                return ves1820_init (i2c);

        case FE_RESET:
                return ves1820_reset (i2c);

        case FE_SETFREQ:
                return tuner_set_tv_freq (*(u32*)arg);

	case FE_SET_INVERSION:
		return ves1820_setup_reg0 (i2c, (GET_REG0(i2c) >> 2) & 0x07, (SpectralInversion) arg);

        default:
                return -EINVAL;
        }

        return 0;
}

static struct i2c_client ves1820_i2c_client;

static
int ves1820_attach (struct i2c_adapter *adap)
{
        struct ves1820 *ves;
        struct i2c_client *i2c;

        ves1820_i2c_client.adapter = adap;

        i2c_master_send(&ves1820_i2c_client, NULL, 0);

        ves1820_i2c_client.adapter = adap;

        if ((ves1820_readreg(&ves1820_i2c_client, 0x1a) & 0xf0) != 0x70)
                return -1;

        i2c = kmalloc(sizeof(struct i2c_client), GFP_KERNEL);

        if (!i2c)
                return -ENOMEM;

        memcpy(i2c, &ves1820_i2c_client, sizeof(struct i2c_client));
        i2c->data = ves = kmalloc(sizeof(struct ves1820),GFP_KERNEL);

        if (!ves)
        {
                kfree(i2c);
                return -ENOMEM;
        }

        printk("VES1820: attaching VES1820 at 0x%02x\n", (i2c->addr)<<1);
        i2c_attach_client(i2c);
        printk("VES1820: attached to adapter %s\n\n", adap->name);

        ves->frontend.type = DVB_C;
        ves->frontend.capabilities = 0;
        ves->frontend.i2cbus = adap;
        ves->frontend.demod = i2c;
        ves->frontend.demod_type = DVB_DEMOD_VES1820;

        register_frontend(&ves->frontend);

        return 0;
}


static
int ves1820_detach (struct i2c_client *i2c)
{
        printk("VES1820: detach_client\n");
        unregister_frontend(&((struct ves1820*)i2c->data)->frontend);
        i2c_detach_client(i2c);
        kfree(i2c->data);
        kfree(i2c);
        return 0;
}


static
void ves1820_inc_use (struct i2c_client *client)
{
#ifdef MODULE
        MOD_INC_USE_COUNT;
#endif
}


static
void ves1820_dec_use (struct i2c_client *client)
{
#ifdef MODULE
        MOD_DEC_USE_COUNT;
#endif
}


static
struct i2c_driver ves1820_i2c_driver =
{
        name:		"VES1820 DVB-C Demodulator",
        id:		I2C_DRIVERID_VES1820,
        flags:		I2C_DF_NOTIFY,
        attach_adapter:	ves1820_attach,
        detach_client:	ves1820_detach,
        command:	ves1820_ioctl,
        inc_use:	ves1820_inc_use,
        dec_use:	ves1820_dec_use
};


static
struct i2c_client ves1820_i2c_client =
{
        name:		"VES1820",
        id:		I2C_DRIVERID_VES1820,
        flags:		0,
        addr:		0x10 >> 1,
        adapter:	NULL,
        driver:		&ves1820_i2c_driver,
	usage_count:	0
};


static
int __init init_ves1820(void)
{
        int res;

	printk("$Id: ves1820.c,v 1.34 2002/10/08 23:18:42 obi Exp $\n");

        if ((res = i2c_add_driver(&ves1820_i2c_driver)))
        {
		printk("%s: i2c driver registration failed\n", __FUNCTION__);
                return res;
        }

	dprintk("%s: done\n", __FUNCTION__);
        return 0;
}


static
void __exit exit_ves1820(void)
{
        int res;

	ves1820_sleep (&ves1820_i2c_client);

        if ((res = i2c_del_driver(&ves1820_i2c_driver)))
        {
		printk("%s: Driver deregistration failed, module not removed.\n", __FUNCTION__);
        }

	dprintk("%s: done\n", __FUNCTION__);
}


#ifdef MODULE
module_init(init_ves1820);
module_exit(exit_ves1820);

MODULE_DESCRIPTION("");
MODULE_AUTHOR("Ralph Metzler");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
MODULE_PARM(debug,"i");
#endif

