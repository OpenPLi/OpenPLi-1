/* 
 * test_dmx.c - Test program for new API
 *
 * Copyright (C) 2000 Ralph  Metzler <ralph@convergence.de>
 *                  & Marcus Metzler <marcus@convergence.de>
                      for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/poll.h>

#include <ost/dmx.h>
#include <ost/frontend.h>
#include <ost/sec.h>
#include <ost/video.h>

#define PID_MASK_HI    0x1F
#define MAX_SECTION_SIZE 4096

static inline unsigned short get_pid(uint8_t *pid)
{
	uint16_t *pp;
	uint8_t ppid[2];

	ppid[0] = pid[0] & PID_MASK_HI;
	ppid[1] = pid[1];
	pp = (short *) ppid;
	return ntohs(*pp);
}



void set_pat_filt(int fd)
{
	size_t bytesRead;
	struct dmxSctFilterParams sctFilterParams;


	memset(&sctFilterParams.filter, 0, sizeof(struct dmxFilter));
	sctFilterParams.pid                       = 0;
	sctFilterParams.filter.filter[0]          = 0x00;
	sctFilterParams.filter.mask[0]            = 0x00;
	sctFilterParams.timeout                   = 1000;
	sctFilterParams.flags                     = DMX_IMMEDIATE_START;

	if (ioctl(fd, DMX_SET_FILTER, &sctFilterParams) < 0)  
		perror("DMX SET FILTER:");

}

void set_pmt_filt(int fd,uint16_t ppid)
{
	size_t bytesRead;
	struct dmxSctFilterParams sctFilterParams;


	memset(&sctFilterParams.filter, 0, sizeof(struct dmxFilter));
	sctFilterParams.pid                       = ppid;
	sctFilterParams.filter.filter[0]          = 0x02;
	sctFilterParams.filter.mask[0]            = 0xFF;
	sctFilterParams.timeout                   = 1000;
	sctFilterParams.flags                     = DMX_IMMEDIATE_START;

	if (ioctl(fd, DMX_SET_FILTER, &sctFilterParams) < 0)  
		perror("DMX SET FILTER:");

}

void set_av_filts(int vfd,int afd,uint16_t vpid,uint16_t apid)
{
	size_t bytesRead;
	struct dmxPesFilterParams pesFilterParams; 


	if (ioctl(vfd, DMX_SET_BUFFER_SIZE, 64*1024) < 0)  
		perror("DMX SET BUFFER:");
	pesFilterParams.pid = vpid; 
	pesFilterParams.input = DMX_IN_DVR; 
	pesFilterParams.output = DMX_OUT_DECODER; 
	pesFilterParams.pesType = DMX_PES_VIDEO; 
	pesFilterParams.flags = DMX_IMMEDIATE_START;
	
	if (ioctl(vfd, DMX_SET_PES_FILTER, &pesFilterParams) < 0) 
		perror("DMX SET FILTER:");
  
	if (ioctl(afd, DMX_SET_BUFFER_SIZE, 64*1024) < 0) 
		perror("DMX SET BUFFER:");
	pesFilterParams.pid = apid;
	pesFilterParams.input = DMX_IN_DVR; 
	pesFilterParams.output = DMX_OUT_DECODER; 
	pesFilterParams.pesType = DMX_PES_AUDIO; 
	pesFilterParams.flags = DMX_IMMEDIATE_START;
  
	if (ioctl(afd, DMX_SET_PES_FILTER, &pesFilterParams) < 0)
		perror("DMX SET FILTER:");
	
}


uint16_t get_pmt_pid(int fd)
{
	u_char sec[MAX_SECTION_SIZE];
	int len, i;
	uint16_t cpid = 0;
	uint16_t length;
	len=read(fd, sec, 4096);
	
	if (len <= 0) return 0;
	
	length  = (sec[1]& 0x0F)<<8;
	length |= (sec[2]& 0xFF);
	
	for (i = 8; i< length-1 && cpid == 0; i+=4){
		if (sec[i] != 0 || sec[i+1] !=0){
			cpid = get_pid(sec+i+2);
			printf("TS: PMT PID: %04x\n",cpid);
		}
	}
	return cpid;
}


get_av_pids(int fd, uint16_t *vpid, uint16_t *apid)
{		
	u_char sec[MAX_SECTION_SIZE];
	int len, i, ilength;
	uint16_t cpid = 0;
	uint16_t length;
	len=read(fd, sec, 4096);
	
	if (len <= 0) return 0;

	length  = (sec[1]& 0x0F)<<8;
	length |= (sec[2]& 0xFF);
	
        
	ilength = (unsigned short)
		((sec[10]&3)<<8);
	ilength |= (sec[11]&0xFF);
	for (i = 12+ilength; i< length-1; i+=5){
		if (sec[i] == 0x02){
			*vpid = get_pid(sec+i+1);
			printf("TS: VIDEO PID: %d\n",*vpid);
		}
		if (sec[i] == 0x03 || sec[i] == 0x04){
			*apid = get_pid(sec+i+1);
			printf("TS: AUDIO PID: %d\n",*apid);
		}
		if (*vpid && *apid) break;
		i+=((sec[i+3]&15)<<8)|sec[i+4];

	}
}

#define BUFFY 32768
#define NFD   2

void play_file_dvr(int filefd, int fd_dvr, int fd)
{
	char buf[BUFFY];
	int count;
	int written;
	int stopped = 0;
	int ch;
	uint16_t ppid=0;
	uint16_t apid=0;
	uint16_t vpid=0;
	int fd_vdemux=open("/dev/ost/demux", O_RDONLY|O_NONBLOCK);
	int fd_ademux=open("/dev/ost/demux", O_RDONLY|O_NONBLOCK);

	//count = read(filefd,buf,BUFFY);
	//write(fd,buf,count); ?????
	
	set_pat_filt(fd);
	while ( (count = read(filefd,buf,BUFFY)) >= 0  ){
		written = 0;
		if(!ppid){
			ppid = get_pmt_pid(fd);
			if (ppid) set_pmt_filt(fd,ppid);
		} else if (!vpid && !apid){
			get_av_pids(fd,&vpid,&apid);
			if (vpid && apid) set_av_filts(fd_vdemux,fd_ademux
						      ,vpid,apid);
		}
		while(written < count){
			written += write(fd_dvr,buf+written,
					 count-written);
		}
	}
}


main(int argc, char **argv)
{
	int fd,fd_dvr,filefd;

	if (argc < 2) return -1;

	if((fd = open("/dev/ost/demux",O_RDONLY|O_NONBLOCK)) < 0){
		perror("DEMUX DEVICE: ");
		return -1;
	}


	if((fd_dvr = open("/dev/ost/dvr",O_WRONLY)) < 0){
		perror("DVR DEVICE: ");
		return -1;
	}


	if ( (filefd = open(argv[1],O_RDONLY)) < 0){
		perror("File open:");
		return -1;
	}


	play_file_dvr(filefd,fd_dvr,fd);


	close(fd);
	close(fd_dvr);
}

