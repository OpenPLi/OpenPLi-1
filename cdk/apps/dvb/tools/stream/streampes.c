/*
 * $Id: streampes.c,v 1.10 2005/07/31 23:32:44 tmbinc Exp $
 *
 * Copyright (C) 2001,2005 by tmbinc
 * Copyright (C) 2001 by kwon
 * Copyright (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
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
 * Calling:
 * GET /<pid>  \r\n -> for tcp opration
 * GET /<pid>,<udpport> \r\n -> for udp operation, tcp connection ist maintained as control connection
 *                              to end udp streaming
 */

#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <config.h>

#if HAVE_DVB_API_VERSION < 3
	#include <ost/dmx.h>
	#define dmx_pes_filter_params dmxPesFilterParams
	#define pes_type pesType
	#ifdef HAVE_DREAMBOX_HARDWARE
		#define DEMUX_DEV "/dev/dvb/card0/demux1"
	#else
		#define DEMUX_DEV "/dev/dvb/card0/demux0"
	#endif
#else
	#include <linux/dvb/dmx.h>
	#define DEMUX_DEV "/dev/dvb/adapter0/demux0"
#endif

#define BSIZE					 1024*16
void send_udp(int fd, int port);

	/* convert_to_es
	
	   this is a state-machine, designed to run in-place,
	   able to work across PES buffers in all circumstances.
	   
	   only change this to another implementation if you 
	   have good reasons ;)
	   
	   it skips the PES header.
	*/

int state, pes_length, pes_hdr_length;
int es_mode;

int convert_to_es(unsigned char *buffer, int len)
{
	unsigned char *dst = buffer;
	int new_len = 0;
	while (len)
	{
		unsigned char v = *buffer++; len--;
		switch (state)
		{
		case 0:
			if (v)
				continue;
			++state;
			break;
		case 1:
			if (v)
			{
				state = 0;
				continue;
			}
			++state;
			break;
		case 2:
			if (v == 0)
			{
				state = 1;
				continue;
			}
			if (v != 1)
			{
				state = 0;
				continue;
			}
			++state;
			break;
		case 3:
			++state;
			break;
		case 4:
			pes_length  = v << 8;
			++state;
			break;
		case 5:
			pes_length |= v;
			++state;
			break;
		case 6:
		case 7:
			++state;
			--pes_length;
			break;
		case 8:
			pes_hdr_length = v;
			++state;
			--pes_length;
			break;
		case 9:
			if (!pes_hdr_length--)
				++state;
			else
			{
				--pes_length;
				break;
			}
				/* fall-trough */
		case 10:
		{
			int max_len;
			--buffer; ++len;
			
			/* pes_length DATA is left. */
			max_len = pes_length;
			if (max_len > len)
				max_len = len;
			if (buffer != dst)
				memcpy(dst, buffer, max_len);
			len -= max_len;
			buffer += max_len;
			dst += max_len;
			new_len += max_len;
			pes_length -= max_len;
			if (!pes_length)
				state = 0;
			break;
		}
		}
	}
	return new_len;
}


int main(int argc, char **argv)
{
	int fd,port,ppid;
	unsigned short pid;
	struct dmx_pes_filter_params flt; 
	char buffer[BSIZE], *bp;
	unsigned char c;
	
		/* use pes->es when called as streames. */
	es_mode = strstr(argv[0], "streames") ? 1 : 0;
	
	bp = buffer;

	while (bp-buffer < BSIZE) {

		read(STDIN_FILENO, &c, 1);

		if ((*bp++=c)=='\n')
			break;
	}

	*bp++ = 0;

	bp = buffer;

	if (!strncmp(buffer, "GET /", 5))
	{
		printf("HTTP/1.1 200 OK\r\nServer: d-Box network\r\n\r\n");
		bp += 5;
	}

	fflush(stdout);

	fd = open(DEMUX_DEV, O_RDWR);

	if (fd < 0) {
		perror(DEMUX_DEV);
		return -fd;
	}

	ioctl(fd, DMX_SET_BUFFER_SIZE, 512*1024);
	sscanf(bp, "%hx", &pid);
	
	port = 0;
	if ((bp=strchr(bp,',')) != 0) {
		bp++;
		sscanf(bp, "%d", &port);
	}
	
	flt.pid = pid;
	flt.input = DMX_IN_FRONTEND;
	flt.output = DMX_OUT_TAP;
	flt.pes_type = DMX_PES_OTHER;
	flt.flags = DMX_IMMEDIATE_START;

	if (ioctl(fd, DMX_SET_PES_FILTER, &flt) < 0) {
		perror("DMX_SET_PES_FILTER");
		return errno;
	}
	
	if (port > 1023) {
		ppid = fork();
		if (ppid == 0) {
			send_udp (fd, port);
		}
		else if (ppid > 0) {
			while (read(STDIN_FILENO,buffer,1) >= 0);
		 	kill (ppid,SIGINT);
		 	waitpid(ppid,0,0);
		}
		else {
			perror("streampes:Cannot fork\n");
			return(-1);
		}
	}
	else {

		while (1) {

			int pr = 0, r;
			int tr = BSIZE;

			while (tr) {

				if ((r=read(fd, buffer+pr, tr)) <= 0)
					continue;

				if (es_mode)
					r = convert_to_es(buffer + pr, r);

				pr+=r;
				tr-=r;
				if (tr) {
					usleep(10000); //wait for 10 ms = max. 10 kb 
				}
			}
			tr = BSIZE;
			pr = 0;

			while (tr) {
				r = write(STDOUT_FILENO, buffer+pr, tr);
				
				if (r <= 0) {
					continue;
				}
				pr += r;
				tr -= r;
			}
		}
	}

	close(fd);
	return 0;
}

void
send_udp (int fd, int port) {
	int			pnr = 0;
	int                     sockfd;
        struct sockaddr_in      cli_addr, serv_addr;
	int			addr_len = sizeof (serv_addr);
	static	char 		buffer[1444];
	
	if(getpeername(STDOUT_FILENO, (struct sockaddr *) &serv_addr, &addr_len)) {
		perror("getpeername");
	}

	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_port        = htons(port);

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		perror("client: can't open datagram socket");

	memset((char *) &cli_addr,0, sizeof(cli_addr));
	cli_addr.sin_family      = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port        = htons(0);
	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
		perror("client: can't bind local address");
	}

	while (1) {
		int pr=0, r;
		int tr=1440;
		while (tr) {
			if ((r=read(fd, buffer+pr, tr)) <= 0) {
				perror ("streampes: read error muxer");
				continue;
			}
			
			if (es_mode)
				r = convert_to_es(buffer + pr, r);
			
			pr+=r;
			tr-=r;
			if (tr) {
				usleep(10000); //wait for 10 ms = max. 10 kb 
			}
		}
		*((int *)(&(buffer[1440])))=pnr++;
		sendto(sockfd, buffer, 1444, 0, (struct sockaddr *) &serv_addr, addr_len);
	}
}

