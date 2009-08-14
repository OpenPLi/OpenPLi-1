/*
 *   satfind (signal monitoring tool) (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2002 Hunz and the dbox2-team
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
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

#include <dbox/lcd-ks0713.h>

#include "icons.h"
#include "font.h"

#include <ost/dmx.h> 
#include <ost/frontend.h> 
#include <ost/sec.h>

#define LCD "/dev/dbox/lcd0"
#define FP "/dev/dbox/fp0"
#define DMX "/dev/dvb/card0/demux0"
#define QPSK "/dev/dvb/card0/frontend0"
#define SEC "/dev/dvb/card0/sec0"

typedef unsigned char screen_t[LCD_BUFFER_SIZE];

#define FILLED 4

struct signal {
  uint32_t ber;
  uint16_t snr;
  uint16_t strength;
  FrontendStatus status;
};

int max_values[3]={0,0xFFFF,0xFFFF};

int draw_screen(screen_t screen,int lcd_fd) {
  return write(lcd_fd,screen,LCD_BUFFER_SIZE);
}

void put_pixel(screen_t screen,int x, int y,char col) {
  switch (col&3) {
  case LCD_PIXEL_ON:
    screen[((y/8)*LCD_COLS)+x]|=1<<(y%8);
    break;
  case LCD_PIXEL_OFF:
    screen[((y/8)*LCD_COLS)+x]&=~(1<<(y%8));
    break;
  case LCD_PIXEL_INV:
    screen[((y/8)*LCD_COLS)+x]^=1<<(y%8);
    break;
  }
}

void draw_horiz_line(screen_t screen,int x, int y,int x2, char col) {
  int count;
  
  for(count=x;count<=x2;count++)
    put_pixel(screen,count,y,col);
}

void draw_vert_line(screen_t screen,int x, int y,int y2, char col) {
  int count;
  
  for(count=y;count<=y2;count++)
    put_pixel(screen,x,count,col);
}

void draw_rectangle(screen_t screen,int x, int y,int x2, int y2, char col) {
  int x_count,y_count;
  
  if(col&FILLED) {
    for(y_count=y;y_count<=y2;y_count++)
      for(x_count=x;x_count<=x2;x_count++)
	put_pixel(screen,x_count,y_count,col);
  }
  else {
    draw_horiz_line(screen,x,y,x2,col);
    draw_horiz_line(screen,x,y2,x2,col);
    draw_vert_line(screen,x,y,y2,col);
    draw_vert_line(screen,x2,y,y2,col);
  }
}

void draw_progressbar(screen_t screen,int x, int y, int height, int len, int *max_val, int val, char col) {
  int screen_val;
  
  if(val>*max_val) *max_val=val;
  screen_val=x+((len*val)/ *max_val);
  draw_rectangle(screen,x,y,x+len,y+height,(!((col&3)>0))|4);
  draw_rectangle(screen,x,y,screen_val,y+height,((col&3)>0)|(col&4));
}

void draw_bmp(screen_t screen,char *icon,int x, int y) {
  int x_count,y_count;
  
  for(y_count=y;y_count<y+icon[1];y_count++)
    for(x_count=x;x_count<x+icon[0];x_count++)
      put_pixel(screen,x_count,y_count,icon[2+((y_count-y)*icon[0])+(x_count-x)]);
}

int get_signal(struct signal *signal_data, int fe_fd) {
  if(ioctl(fe_fd,FE_READ_BER,&signal_data->ber)<0) {
    fprintf(stderr,"frontend ioctl - Can't read BER: %d\n",errno);
    return -1;
  }
  if(ioctl(fe_fd,FE_READ_SNR,&signal_data->snr)<0) {
    fprintf(stderr,"frontend ioctl - Can't read SNR: %d\n",errno);
    return -1;
  }
  if(ioctl(fe_fd,FE_READ_SIGNAL_STRENGTH,&signal_data->strength)<0) {
    fprintf(stderr,"frontend ioctl - Can't read Signal Strength: %d\n",errno);
    return -1;
  }
  if(ioctl(fe_fd,FE_READ_STATUS,&signal_data->status)<0) {
    fprintf(stderr,"frontend ioctl - Can't read Status: %d\n",errno);
    return -1;
  }
  return 0;
}

void render_string(screen_t screen,int x, int y, char *string) {
  int x_count,y_count,pos;
  for(pos=0;string[pos]!=0;pos++)
    for(y_count=y;y_count<y+8;y_count++)
      for(x_count=x+(8*pos);x_count<x+(8*(pos+1));x_count++)
	put_pixel(screen,x_count,y_count,(font[(string[pos]*8)+y_count-y]>>(7-(x_count-(x+(8*pos)))))&1);
}

void draw_signal(struct signal *signal_data, struct signal *old_signal, screen_t screen) {
  char dec_buf[11];
  int val,count;
  
  if (signal_data->ber!=old_signal->ber) {
    draw_progressbar(screen,0,0,6,119,&max_values[0],signal_data->ber,4|1);//1656720 max?
    memset(dec_buf,0x20,10);
    dec_buf[10]=0;
    val=signal_data->ber;
    for(count=9;count>=0;count--) {
      dec_buf[count]=(val%10)+0x30;
      val/=10;
      if(val==0)
	break;
    }
    render_string(screen,32,8,dec_buf);
  }
  if (signal_data->snr!=old_signal->snr) {
    draw_progressbar(screen,0,18,6,119,&max_values[1],signal_data->snr,4|1);
    memset(dec_buf,0x20,10);
    dec_buf[10]=0;
    val=signal_data->snr;
    for(count=9;count>=0;count--) {
      dec_buf[count]=(val%10)+0x30;
      val/=10;
      if(val==0)
	break;
    }
    render_string(screen,32,26,dec_buf);
  }
  if (signal_data->strength!=old_signal->strength) {
    draw_progressbar(screen,0,37,6,119,&max_values[2],signal_data->strength,4|1);
    memset(dec_buf,0x20,10);
    dec_buf[10]=0;
    val=signal_data->strength;
    for(count=9;count>=0;count--) {
      dec_buf[count]=(val%10)+0x30;
      val/=10;
      if(val==0)
	break;
    }
    render_string(screen,32,45,dec_buf);
  }
  if ((signal_data->status&FE_HAS_SIGNAL) != (old_signal->status&FE_HAS_SIGNAL)) {
    if(signal_data->status&FE_HAS_SIGNAL) {
      /* we got signal */
      draw_bmp(screen,signal_icon,120-signal_icon[0],64-signal_icon[1]);
      render_string(screen,0,56,"???        ");
    }
    else {
      /* we lost signal */
      draw_rectangle(screen,120-signal_icon[0],64-signal_icon[1],120,64,4);
      render_string(screen,0,56,"NO SIGNAL  ");
    }
  }
  if ((signal_data->status&FE_HAS_LOCK) != (old_signal->status&FE_HAS_LOCK)) {
    if (signal_data->status&FE_HAS_LOCK)
      draw_bmp(screen,lock_icon,120-lock_icon[0]-8,64-lock_icon[1]);
    else
      draw_rectangle(screen,120-lock_icon[0]-8,64-lock_icon[1],120,64,4);
  }
  memcpy(old_signal,signal_data,sizeof(struct signal));
}

void get_network_name_from_nit(char *network_name, unsigned char *nit, int len) {
  unsigned char *ptr=nit+10;
  
  if (len<=24) {
    network_name[0]=0;
    return;
  }
  while((ptr-(nit+10)<(((nit[8]&0x0F)<<8)|nit[9]))&&(ptr<nit+len)) {
    if(ptr[0]==0x40) {
      if(ptr[1]>30)
	ptr[1]=30;
      memcpy(network_name,ptr+2,ptr[1]);
      network_name[ptr[1]]=0;
      return;
    }
    else
      ptr+=ptr[1]+2;
  }
  network_name[0]=0;
}

void prepare_main(screen_t screen) {
  render_string(screen,0,8,"ber          0");
  render_string(screen,0,26,"snr          0");
  render_string(screen,0,45,"sig          0");
}

int main(int argc, char **argv) {
  int lcd_fd;//,fp_fd;
  int qpsk_fd,dmx_fd;//,sec_fd
  int lcd_mode;
  fd_set rfds;
  int result;
  screen_t screen;
  struct timeval tv;
  struct signal signal_quality,old_signal;
  struct dmxSctFilterParams flt;
  unsigned char buf[1024];
  char network_name[31],old_name[31];

  /* open dbox2-specific devices (LCD and FP) */
  if((lcd_fd=open(LCD,O_RDWR))<0) {
    fprintf(stderr,"lcd open - Can't open LCD: %d\n",errno);
    return -1;
  }
  /*
  if((fp_fd=open(FP,O_RDWR))<0) {
    perror("Can't open FP");
    return -1;
  }
  */

  /* open nokia-api specific devices (demux,tuner and sat-control) */
  
  if((dmx_fd=open(DMX,O_RDWR))<0) {
    perror("Can't open Demux");
    return -1;
  }
  
  if((qpsk_fd=open(QPSK,O_RDONLY))<0) {
    fprintf(stderr,"frontend open - Can't open Tuner: %d\n",errno);
    return -1;
  }
  /*
  if((sec_fd=open(SEC,O_RDWR))<0) {
    perror("Can't open SEC");
    return -1;
  }
  */

  /* switch LCD to binary mode and clear it */
  lcd_mode=LCD_MODE_BIN;
  if ((ioctl(lcd_fd,LCD_IOCTL_ASC_MODE,&lcd_mode)<0) || (ioctl(lcd_fd,LCD_IOCTL_CLEAR)<0)) {
    fprintf(stderr,"lcd ioctl - error setting LCD-mode/clearing LCD: %d\n",errno);
    return -1;
  }
  memset(screen,0,sizeof(screen));
  memset(&old_signal,0,sizeof(old_signal));
  
  /* initialize demux to get the NIT */
  
  memset(&flt.filter.filter, 0, DMX_FILTER_SIZE);
  memset(&flt.filter.mask, 0, DMX_FILTER_SIZE);
  
  flt.pid=0x10;
  flt.filter.filter[0]=0x40;
  flt.filter.mask[0]=0xFF;
  flt.timeout=10000;
  flt.flags=DMX_IMMEDIATE_START;
  
  if (ioctl(dmx_fd, DMX_SET_FILTER, &flt)<0)  {
    perror("DMX_SET_FILTER");
    return -1;
  }
  
  /* main stuff here */
  prepare_main(screen);
  network_name[0]=0;
  old_name[0]=0;
  FD_ZERO(&rfds);
  FD_SET(dmx_fd,&rfds);
  tv.tv_sec=0;
  tv.tv_usec=10000;
  while(1) {
    if((result=select(dmx_fd+1,&rfds,NULL,NULL,&tv))>0) {
      if(FD_ISSET(dmx_fd,&rfds)) {
	/* got data */
	if((result=read(dmx_fd,buf,sizeof(buf)))>0)
	  get_network_name_from_nit(network_name,buf,result);
	/* zero or less read - we have to restart the DMX afaik*/
	else {
	  printf("result: %d\n",result);
	  ioctl(dmx_fd,DMX_STOP,0);
	  ioctl(dmx_fd,DMX_START,0);
	  network_name[0]=0;
	}
	/* new networkname != "" */
	if((memcmp(network_name,old_name,sizeof(network_name))!=0)&&(network_name[0]!=0)) {
	  int count;
	  for(count=strlen(network_name);count<=10;count++)
	    network_name[count]=0x20;
	  network_name[count]=0;
	  render_string(screen,0,56,network_name);
	  memcpy(old_name,network_name,sizeof(old_name));
	}
      }
      else
	printf("that should never happen...\n");
    }

    FD_ZERO(&rfds);
    FD_SET(dmx_fd,&rfds);
    tv.tv_sec=0;
    tv.tv_usec=10000;

    get_signal(&signal_quality,qpsk_fd);
    draw_signal(&signal_quality,&old_signal,screen);
    draw_screen(screen,lcd_fd);
    printf("%s %d %d %d [%c%c]\n",network_name,signal_quality.ber,signal_quality.snr,signal_quality.strength,signal_quality.status&FE_HAS_SIGNAL? 'S':' ',signal_quality.status&FE_HAS_LOCK? 'L':' ');
  }

  /* close devices */
  close(lcd_fd);
  //  close(fp_fd);
  
  close(dmx_fd);
  close(qpsk_fd);
  //close(sec_fd);
  
  return 0;
}
