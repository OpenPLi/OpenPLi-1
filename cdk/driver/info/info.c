/*
 *   info.c - d-Box Hardware info
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
 *   $Log: info.c,v $
 *   Revision 1.20  2002/08/07 13:28:49  obi
 *   checked wrong byte for avia detection?
 *
 *   Revision 1.19  2002/07/18 19:19:35  wjoost
 *
 *   AVIA-Erkennung gefixt. Hinweis auf Konfigurationsfalle eingebaut.
 *
 *   Revision 1.18  2002/05/07 19:54:13  derget
 *   info.fe wieder eingebaut :)
 *
 *   Revision 1.17  2002/05/07 01:17:42  derget
 *   bla
 *
 *   Revision 1.16  2002/05/07 01:15:35  derget
 *   ves1993 detection für nokia
 *
 *   Revision 1.15  2002/05/06 02:18:19  obi
 *   cleanup for new kernel
 *
 *   Revision 1.14  2002/03/20 16:05:15  obi
 *   added missing variables to dbox.sh
 *
 *   Revision 1.13  2001/12/01 06:53:04  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.12  2001/10/18 23:22:15  Jolt
 *   Fix for gcc < 3
 *
 *   Revision 1.11  2001/09/29 23:49:04  TripleDES
 *   small fx in avia-detection
 *
 *   Revision 1.10  2001/09/29 23:44:00  TripleDES
 *   added avia-type
 *
 *   Revision 1.9  2001/09/29 03:16:18  TripleDES
 *   added dsID
 *
 *   Revision 1.8  2001/07/08 00:14:40  fnbrd
 *   parameter fe gibts nicht mehr
 *
 *   Revision 1.6  2001/06/09 23:51:44  tmbinc
 *   added fe.
 *
 *   Revision 1.5  2001/06/03 20:41:55  kwon
 *   indent
 *
 *   Revision 1.4  2001/04/23 00:24:45  fnbrd
 *   /proc/bus/dbox.sh an die sh der BusyBox angepasst.
 *
 *   Revision 1.3  2001/04/04 17:43:58  fnbrd
 *   /proc/bus/dbox.sh implementiert.
 *
 *   Revision 1.2  2001/04/03 17:48:24  tmbinc
 *   improved /proc/bus/info (philips support)
 *
 *   Revision 1.1  2001/03/28 23:33:31  tmbinc
 *   added /proc/bus/info.
 *
 *
 *   $Revision: 1.20 $
 *
 */

#include <linux/string.h>
#include <linux/fs.h>
#include <linux/unistd.h>
#include <linux/fcntl.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/i2c.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/8xx_immap.h>
#include <asm/pgtable.h>
#include <asm/mpc8xx.h>
#include <asm/bitops.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <dbox/info.h>

	/* ich weiss dass dieses programm suckt, aber was soll man machen ... */

//int fe=0;
static struct dbox_info_struct info;

#ifdef CONFIG_PROC_FS

static int info_proc_init(void);
static int info_proc_cleanup(void);

static int read_bus_info(char *buf, char **start, off_t offset, int len,
												int *eof , void *private);
volatile u8 *aviamem;

#else /* undef CONFIG_PROC_FS */

#define info_proc_init() 0
#define info_proc_cleanup() 0

#endif /* CONFIG_PROC_FS */

static int attach_dummy_adapter(struct i2c_adapter *);

static struct i2c_driver dummy_i2c_driver = {
	"FOR_PROBE_ONLY",
	I2C_DRIVERID_EXP2, // experimental use id
	I2C_DF_NOTIFY,
	attach_dummy_adapter,
	0,
	0,
	0,
	0
};

static struct i2c_client dummy_i2c_client = {
	"FOR_PROBE_ONLY_CLIENT",
	I2C_DRIVERID_EXP2, // experimental use id
	0,
	0,
	NULL,
	&dummy_i2c_driver,
	NULL
};

static u8 i2c_addr_of_device;
static u16 i2c_device_addr_to_read;
static u8 i2c_should_value;
static u8 i2c_should_mask;
static int i2c_found;
static int i2c_attach_called;
static int device_reg_addr_is_16bit;

// k.A. ob das noch wer anders so (wegen Protokoll) gebrauchen kann,
// deswegen lass ich das mal so hier drin
// Routine sieht etwas merkwuerdig aus, habe ich aber groesstenteils so
// aus ves1820 uebernommen und keine Lust das zu cleanen

static u8 readreg(struct i2c_client *client, u8 reg)
{
	struct i2c_adapter *adap=client->adapter;
	unsigned char mm1[2];
	unsigned char mm2[] = {0x00};
	struct i2c_msg msgs[2];

	msgs[0].flags=0;
	msgs[1].flags=I2C_M_RD;
	msgs[0].addr=msgs[1].addr=client->addr;
	if(device_reg_addr_is_16bit) {
	  mm1[0]=0;
	  mm1[1]=reg;
	  msgs[0].len=2;
	}
	else {
	  mm1[0]=reg;
	  msgs[0].len=1;
	}
	msgs[1].len=1;
	msgs[0].buf=mm1;
	msgs[1].buf=mm2;
	i2c_transfer(adap, msgs, 2);

	return mm2[0];
}

static int attach_dummy_adapter(struct i2c_adapter *adap)
{
  i2c_attach_called = 1;
  dummy_i2c_client.adapter=adap;
  dummy_i2c_client.addr=i2c_addr_of_device;
  if ( ( readreg(&dummy_i2c_client, i2c_device_addr_to_read) & i2c_should_mask ) != i2c_should_value ) {
//    printk(KERN_INFO "device not found\n");
    i2c_found=0;
  }
  else {
//    printk(KERN_INFO "device found\n");
    i2c_found=1;
  }
  return -1; // we don't need to attach, probing was done
}

static void probeDevice(void)
{
  i2c_attach_called = 0;
  i2c_add_driver(&dummy_i2c_driver); // fails allways
  if (!i2c_attach_called) {
    printk(KERN_ERR "info.o: cannot get enough information without i2c-adapter.\n");
  }
  i2c_del_driver(&dummy_i2c_driver);
}

static int checkForAT76C651(void)
{
  i2c_addr_of_device=0x0d;
  i2c_device_addr_to_read=0x0e;
  i2c_should_value=0x65;
  i2c_should_mask=0xff;
  device_reg_addr_is_16bit=0;
  probeDevice();
  return i2c_found;
}

static int checkForVES1820(void)
{
  i2c_addr_of_device=0x08; //0x10>>1
  i2c_device_addr_to_read=0x1a;
  i2c_should_value=0x70;
  i2c_should_mask=0xf0;
  device_reg_addr_is_16bit=1;
  probeDevice();
  return i2c_found;
}

static int checkForVES1993(void)
{               
  i2c_addr_of_device=0x08; //0x10>>1
  i2c_device_addr_to_read=0x1e;
  i2c_should_value=0xde;
  i2c_should_mask=0xde;
  device_reg_addr_is_16bit=1;
  probeDevice();
  return i2c_found;
}

volatile cpm8xx_t *cpm;

int ds_reset(void)
{
	int success;
	cpm->cp_pbdat&=~4;
	cpm->cp_pbdir|= 4;
	udelay(480);
	cpm->cp_pbdir&=~4;
	udelay(120);
	success = (cpm->cp_pbdat & 4);
	udelay(360);
	return success;
}

void write1(void)
{
	cpm->cp_pbdat&=~4;
	cpm->cp_pbdir|= 4;
	udelay(1);
	cpm->cp_pbdir&= ~4;
	udelay(59);
}

void write0(void)
{
	cpm->cp_pbdat&=~4;
	cpm->cp_pbdir|= 4;
	udelay(55);
	cpm->cp_pbdir &= ~4;
	udelay(5);
}

int readx(void)
{
	int result;
	cpm->cp_pbdat&=~4;
	cpm->cp_pbdir|= 4;
	udelay(1);
	cpm->cp_pbdir &= ~4;
	udelay(14);
	result = (cpm->cp_pbdat & 4)>>2;
	udelay(45);
	return result;
}

void writebyte(int data)
{
	int loop;
	for(loop=0;loop<8;loop++)
	{
		if(data & (0x01 << loop))
				write1();
			else
				write0();
	}
}

int readbyte(void)
{
	int loop;
	int result=0;

	for(loop=0;loop<8;loop++)
		result = result + (readx()<<loop);
	return result;
}

static unsigned char dsid[8];
static void get_dsid(void)
{
	int i;

	immap_t *immap=(immap_t *)IMAP_ADDR ;

	cpm = &immap->im_cpm;

	cpm->cp_pbpar&=~4;
	cpm->cp_pbodr|= 4;

	if(ds_reset()) printk("DS not responding!!! - please report\n");
	writebyte(0x33);
	for(i=0;i<8;i++)dsid[i]=readbyte();
	return;

}

static int aviatype(void)
{
	int aviarev=0;

	aviamem=(unsigned char*)ioremap(0xA000000,0x200);
	if(!aviamem)
	{
		printk("INFO: cannot remap avia-mem.\n");
		return -1;
	}
	(void)aviamem[0];

	aviamem[6] = 0x80;
	aviamem[5] = 0;
	aviamem[4] = 0;
	aviarev = aviamem[3] << 24;
	aviarev |= aviamem[2] << 16;
	aviarev |= aviamem[1] << 8;
	aviarev |= aviamem[0];

	return (aviarev & 0x030000) ? 500:600;
}

static int dbox_info_init(void)
{
	unsigned char *conf=(unsigned char*)ioremap(0x1001FFE0, 0x20);
	if (!conf)
	{
		printk(KERN_ERR "couldn't remap memory for getting device info.\n");
		return -1;
	}
	get_dsid();
	memcpy(info.dsID,dsid,8);
	info.mID=conf[0];
	info.avia=aviatype();
//	info.fe=fe;
	if (info.mID==DBOX_MID_SAGEM)								// das suckt hier, aber ich kenn keinen besseren weg.
	{
		info.feID=0;
		info.fpID=0x52;
		info.enxID=3;
		info.gtxID=-1;
		info.fe= checkForAT76C651() ? 0 : 1;
		info.hwREV=info.fe?0x21:0x41;
		info.fpREV=0x23;
		info.demod=info.fe?DBOX_DEMOD_VES1993 : DBOX_DEMOD_AT76C651;
	}	else if (info.mID==DBOX_MID_PHILIPS)		// never seen a cable-philips
	{
		info.fe=1;
		info.feID=0;
		info.fpID=0x52;
		info.enxID=3;
		info.gtxID=-1;
		info.hwREV=info.fe?0x01:-1;
		info.fpREV=0x30;
		info.demod=info.fe?DBOX_DEMOD_TDA8044H:-1;
	}	else if (info.mID==DBOX_MID_NOKIA)
	{
		if (checkForVES1820())
			{ info.fe=0;
			  info.feID=0x7a ;
			  info.demod=DBOX_DEMOD_VES1820;}
		else if (checkForVES1993())
			{info.fe=1;
			 info.feID=0x00  ;
			 info.demod=DBOX_DEMOD_VES1993;}
		else {info.fe=1;
		      info.feID=0xdd ;
		      info.demod=DBOX_DEMOD_VES1893;}

		info.fpID=0x5a;
		info.enxID=-1;
		info.gtxID=0xB;
		info.fpREV=0x81;
		info.hwREV=0x5;
	}
	iounmap(conf);
	printk(KERN_DEBUG "mID: %02x feID: %02x fpID: %02x enxID: %02x gtxID: %02x hwREV: %02x fpREV: %02x\n",
		info.mID, info.feID, info.fpID, info.enxID, info.gtxID, info.hwREV, info.fpREV);
	info_proc_init();
	return 0;
}

int dbox_get_info(struct dbox_info_struct *dinfo)
{
	memcpy(dinfo, &info, sizeof(info));
	return 0;
}

int dbox_get_info_ptr(struct dbox_info_struct **dinfo)
{
	if (!dinfo)
		return -EFAULT;
	*dinfo=&info;
	return 0;
}

#ifdef CONFIG_PROC_FS

static char *demod_table[5]={"VES1820", "VES1893", "AT76C651", "VES1993", "TDA8044H"};

static int read_bus_info(char *buf, char **start, off_t offset, int len,
												int *eof , void *private)
{
	return sprintf(buf, "mID=%02x\nfeID=%02x\nfpID=%02x\nenxID=%02x\ngtxID=%02x\nhwREV=%02x\nfpREV=%02x\nDEMOD=%s\nfe=%d\navia=%d\ndsID=%02x-%02x.%02x.%02x.%02x.%02x.%02x-%02x\n",
		info.mID, info.feID, info.fpID, info.enxID, info.gtxID, info.hwREV, info.fpREV, info.demod==-1 ? "UNKNOWN" : demod_table[info.demod],info.fe,info.avia,
		info.dsID[0],info.dsID[1],info.dsID[2],info.dsID[3],info.dsID[4],info.dsID[5],info.dsID[6],info.dsID[7]);

}

static int read_bus_info_sh(char *buf, char **start, off_t offset, int len,
												int *eof , void *private)
{
	return sprintf(buf, "#!/bin/sh\nexport mID=%02x\nexport feID=%02x\nexport fpID=%02x\nexport enxID=%02x\nexport gtxID=%02x\nexport hwREV=%02x\nexport fpREV=%02x\nexport DEMOD=%s\nexport fe=%d\nexport avia=%d\nexport dsID=%02x-%02x.%02x.%02x.%02x.%02x.%02x-%02x\n",
//	return sprintf(buf, "#!/bin/sh\nexport mID=%02x feID=%02x fpID=%02x enxID=%02x gtxID=%02x hwREV=%02x fpREV=%02x DEMOD=%s\n\n",
//	return sprintf(buf, "#!/bin/sh\nmID=%02x\nfeID=%02x\nfpID=%02x\nenxID=%02x\ngtxID=%02x\nhwREV=%02x\nfpREV=%02x\nDEMOD=%s\nexport mID feID fpID enxID gtxID hwREV fpREV DEMOD\n\n",
		info.mID, info.feID, info.fpID, info.enxID, info.gtxID, info.hwREV, info.fpREV, info.demod==-1 ? "UNKNOWN" : demod_table[info.demod], info.fe, info.avia,
		info.dsID[0], info.dsID[1], info.dsID[2], info.dsID[3], info.dsID[4], info.dsID[5], info.dsID[6], info.dsID[7]);
}

int info_proc_init(void)
{
	struct proc_dir_entry *proc_bus_info;
	struct proc_dir_entry *proc_bus_info_sh;

	if (! proc_bus) {
		printk("info.o: /proc/bus/ does not exist");
		return -ENOENT;
	}

	proc_bus_info = create_proc_entry("dbox", 0, proc_bus);

	if (!proc_bus_info)
	{
		printk("info.o: Could not create /proc/bus/dbox");
		return -ENOENT;
	}

	proc_bus_info_sh = create_proc_entry("dbox.sh", 0, proc_bus);
	if (!proc_bus_info_sh)
	{
		printk("info.o: Could not create /proc/bus/dbox.sh");
		return -ENOENT;
	}


	proc_bus_info->read_proc = &read_bus_info;
	proc_bus_info->write_proc = 0;
	proc_bus_info->owner = THIS_MODULE;
	proc_bus_info_sh->read_proc = &read_bus_info_sh;
	proc_bus_info_sh->write_proc = 0;
	proc_bus_info_sh->mode|=S_IXUGO;
	proc_bus_info_sh->owner = THIS_MODULE;
	return 0;
}

int info_proc_cleanup(void)
{
	remove_proc_entry("dbox", proc_bus);
	remove_proc_entry("dbox.sh", proc_bus);
	return 0;
}
#endif /* def CONFIG_PROC_FS */

#ifdef MODULE
MODULE_AUTHOR("Felix Domke <tmbinc@gmx.net>");
MODULE_DESCRIPTION("d-Box info");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
//MODULE_PARM(fe, "i");
EXPORT_SYMBOL(dbox_get_info);
EXPORT_SYMBOL(dbox_get_info_ptr);

int init_module(void)
{
	return dbox_info_init();
}

void cleanup_module(void)
{
	info_proc_cleanup();
	return;
}

EXPORT_SYMBOL(cleanup_module);

#endif
