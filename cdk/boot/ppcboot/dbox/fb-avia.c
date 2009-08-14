/*
 *   fb-avia.c - fb driver for AVIA-GTX/ENX (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 
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
 *  $Log: fb-avia.c,v $
 *  Revision 1.10  2001/10/02 20:27:53  TripleDES
 *  added Philips Bootlogo support
 *
 *  Revision 1.9  2001/09/17 20:47:21  TripleDES
 *  some fixes
 *
 *  Revision 1.8  2001/08/31 02:34:03  derget
 *
 *  fixxed logo over tftp detection of no tftp logo
 *
 *  Revision 1.7  2001/08/28 10:52:38  derget
 *
 *  implemented logo over tftp if logo flash failes ..
 *  not good , but working , will fixx bad code in some time
 *
 *  Revision 1.6  2001/07/09 22:51:39  derget
 *
 *  finaly fixxed sagem s/w problem
 *  was wrong dac timning
 *  fixxed fnc switching , so that tv kan switch pack tp programm
 *
 *  Revision 1.5  2001/06/29 22:54:53  TripleDES
 *  added colours to the bootlogo for enx (FNC/FBLK)
 *
 *  Revision 1.4  2001/06/24 12:54:46  TripleDES
 *  added eNX support
 *
 *  Revision 1.3  2001/06/05 11:08:10  derget
 *
 *  completed AVIA-GTX support
 *  implemented IDXFS_OFFSET
 *
 *  Revision 1.2  2001/06/05 11:02:11  derget
 *
 *  test import
 *
 *  Revision 1.1  2001/04/25 Jolt 
 *  
 *  Reimplementet gtxfb.c
 *
 *  $Revision: 1.10 $
 *
 */
#include <stdio.h>
#include <malloc.h>
#include "i2c.h"
#include "fb-avia.h"
#include <idxfs.h>
#include "enx.h"

#define fboffset 1024*1024

static int pal=1;
unsigned char *gtxmem, *gtxreg;
unsigned char *enx_mem_addr = (unsigned char*)ENX_MEM_BASE;
unsigned char *enx_reg_addr = (unsigned char*)ENX_REG_BASE;

static unsigned char PAL_SAA7126_NOKIA_INIT[] = {

  0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0f, 0xf7, 0xef, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   
  0x3f, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x1d, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00,
  0x3f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0xff, 0x00, 0x1a, 0x1a,
  0x03, // normal mode
//0x80, // colorbar mode
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x68, 0x7d, 0xaf, 0x33, 0x35, 0x35,
  0x00, 0x06, 0x2f, 0xcb, 0x8a, 0x09, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x52, 0x28, 0x01, 0x20, 0x31,
  0x7d, 0xbc, 0x60, 0x41, 0x05, 0x00, 0x06, 0x16, 0x06, 0x16, 0x16, 0x36, 0x60, 0x00, 0x00, 0x00

};

static unsigned char PAL_SAA7126_SAGEM_INIT[] =
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x1D, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x1E, 
  0x03, // normal mode
//0x80, // clolorbar mode 
  0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0xF2, 0x90, 0x00, 0x00, 0x70, 0x75, 0xA5, 0x37, 0x39, 0x39,
  0x00, 0x06, 0x2C, 0xCB, 0x8A, 0x09, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x52, 0x6F, 0x00, 0xA0, 0x31,
  0x7D, 0xBF, 0x60, 0x40, 0x07, 0x00, 0x06, 0x16, 0x06, 0x16, 0x16, 0x36, 0x60, 0x00, 0x00, 0x00
};

void i2c_bus_init(void)
{
  i2c_init();
  i2c_setspeed(50000);
}

void saa7126_init(char mId)
{
  switch(mId) {
    case 1:
    i2c_send(0x88, 0x00, 0x01, 0x40, PAL_SAA7126_NOKIA_INIT);
    i2c_send(0x88, 0x40, 0x01, 0x40, PAL_SAA7126_NOKIA_INIT + 0x40);
    break;
    case 2:
    case 3:
    i2c_send(0x88, 0x00, 0x01, 0x40, PAL_SAA7126_SAGEM_INIT);
    i2c_send(0x88, 0x40, 0x01, 0x40, PAL_SAA7126_SAGEM_INIT + 0x40);
    break;	
  }    

}

void avs_init(char mId)
{
  switch(mId) {
    case 1:
      i2c_send(0x90, 0, 0, 5, "\x0d\x29\xc9\xa9\x00");
    break;
    case 2:
      i2c_send(0x94, 0, 0, 8, "\x00\x04\x19\x11\xa5\x00\x30\x88");
    break;	
    case 3:
      i2c_send(0x90, 0, 0, 7, "\x00\x00\x00\x00\x04\x3F\x00");
      break;	
  }    
}

void gtxcore_init(void)
{
        int cr;
        gtxmem=(unsigned char*)GTX_PHYSBASE;
        gtxreg=gtxmem+0x400000;

        rh(RR0)=0xFFFF;
        rh(RR1)=0x00FF;
        rh(RR0)&=~((1<<13)|(1<<12)|(1<<10)|(1<<9)|(1<<6)|1);
        rh(RR0)=0;   
        rh(RR1)=0; 
        cr=rh(CR0);
        cr|=1<<11;
        cr&=~(1<<10);
        cr|=1<<9;
        cr&=~(1<<5);
        cr|=1<<3; 
            
        cr&=~(3<<6);
        cr|=1<<6; 
        cr&=~(1<<2);
        rh(CR0)=cr;
}

unsigned char *enx_get_mem_addr(void) {
    return enx_mem_addr;
}

unsigned char *enx_get_reg_addr(void) {
    return enx_reg_addr;
}

void enxcore_init(void)
{
	enx_reg_w(RSTR0) = 0xFCF6BEFF;
	enx_reg_w(SCSC) = 0x00000000;
	enx_reg_w(RSTR0) &= ~(1 << 12);
	enx_reg_w(MC) = 0x00001015;
	enx_reg_w(RSTR0) &= ~(1 << 11);
	enx_reg_w(RSTR0) &= ~(1 << 9);

	enx_reg_w(CFGR0) |= 1 << 24;    // dac
        enx_reg_w(RSTR0) &= ~(1 << 20); // Get dac out of reset state
        enx_reg_h(DAC_PC) = 0x0000;     // dac auf 0
        enx_reg_h(DAC_CP) = 0x0009;     // dac
	
}	
void gtxvideo_init(void)
{
        int val;
        rh(VCR)=0x340;
        rh(VHT)=pal?858:852;
        rh(VLT)=pal?(623|(21<<11)):(523|(18<<11));
    
        val=3<<30;   //16bit rgb
        val|=3<<24;  //chroma filter	
        val|=0<<20;  //BLEV1
        val|=0<<16;  //BLEV0
        val|=720*2;  //Stride
  
        rw(GMR)=val;

        rh(CCR)=0x7FFF;
        rw(GVSA)=fboffset;
        rh(GVP)=0;
        VCR_SET_HP(2);
        VCR_SET_FP(0);
    
        val=pal?127:117;
        val*=8;
        if (rw(GMR)&(1<<28))
                val/=9;
        else
                val/=8;
    
        val-=64;
        GVP_SET_COORD(val, pal?42:36);
        rh(GFUNC)=0x10;
        rh(TCR)=0xFC0F;
        GVS_SET_XSZ(720);
        GVS_SET_YSZ(576);
        rw(VBR)=(1<<24)|0x123456;               // hier nochmal schwarz einbauen...
}

void enxvideo_init(void)
{
	int val;
	enx_reg_w(VBR) = 0;
	enx_reg_h(VCR) = 0x40;
	enx_reg_h(VHT) = 857|0x5000;
	enx_reg_h(VLT) = 623|(21<<11);

	val=0;
	val|=1<<26;  
	val|=3<<20;  
	val|=(720*2);		
	
        enx_reg_w(GMR1)=val;
        enx_reg_w(GMR2)=0;
	enx_reg_h(GBLEV1) = 0;
	enx_reg_h(GBLEV2) = 0;
	enx_reg_w(GVSA1) = fboffset;
	enx_reg_w(GVP1) = 0;
	enx_reg_h(G1CFR)=0;
	enx_reg_h(G2CFR)=0;

#define ENX_GVP_SET_X(X)     enx_reg_w(GVP1) = ((enx_reg_w(GVP1)&(~(0x3FF<<16))) | ((X&0x3FF)<<16))
#define ENX_GVP_SET_Y(X)     enx_reg_w(GVP1) = ((enx_reg_w(GVP1)&(~0x3FF))|(X&0x3FF))
#define ENX_GVP_SET_COORD(X,Y) ENX_GVP_SET_X(X); ENX_GVP_SET_Y(Y)
#define ENX_GVS_SET_XSZ(X)   enx_reg_w(GVSZ1) = ((enx_reg_w(GVSZ1)&(~(0x3FF<<16))) | ((X&0x3FF)<<16))
#define ENX_GVS_SET_YSZ(X)   enx_reg_w(GVSZ1) = ((enx_reg_w(GVSZ1)&(~0x3FF))|(X&0x3FF))

	ENX_GVP_SET_COORD(113,42);
	ENX_GVS_SET_XSZ(720);
	ENX_GVS_SET_YSZ(576);
}
#define XRES 720
#define YRES 576
extern int decodestillmpg(void *pic, const void *src, int X_RESOLUTION, int Y_RESOLUTION);
    
int fb_init(void)
{
	unsigned char mID = *(char*)(0x1001ffe0);
	unsigned int size, offset = 0;
	unsigned char *iframe_logo;
	int have_logo = 0;
	
	idxfs_file_info((unsigned char*)IDXFS_OFFSET, 0, "logo-fb", &offset, &size);
  
	if (!offset) 
	{
		printf("  No FB Logo in Flash , trying tftp\n");
        	if (1==NetLoop(33161152, "TFTP","%(bootpath)/tftpboot/logo-fb", 0x120000))
		{
			have_logo = 1;
			iframe_logo = (unsigned char*)(0x120000);
		} 
		else printf("  FB logo not found\n");
	}    	
    	else 
	{ 
		have_logo = 1;
		iframe_logo = (unsigned char*)(IDXFS_OFFSET + offset); 
	} 
		
	i2c_bus_init();
	saa7126_init(mID);

	if (have_logo)
	{
		printf("  FB logo at: 0x%X (0x%X bytes)\n", offset, size);
		switch (mID)
		{
			case 1: //Nokia
				gtxcore_init();
				decodestillmpg(gtxmem + fboffset, iframe_logo, XRES, YRES);
				gtxvideo_init();
				break;
			case 2:	// Philips
			case 3:	// Sagem
				enxcore_init();
				decodestillmpg(enx_mem_addr + fboffset, iframe_logo, XRES, YRES);
				enxvideo_init();
				break;
		}
		printf("  AVIA Frambuffer\n");		
	}  
	avs_init(mID);
}
