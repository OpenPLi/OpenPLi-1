/* 
 * (C) Andrei Darashenka. <adorosh@users.sourceforge.net>
 *
 * based on ...
 *
 * test_switch.c - Test program for new API
 *
 * Copyright (C) 2001 Ralph  Metzler <ralph@convergence.de>
 *				  & Marcus Metzler <marcus@convergence.de>
 *					  for convergence integrated media GmbH
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
#include <ost/net.h>

int main(int argc,char **argv)
{
	struct dvb_net_if net1={0x200,0};
	int fd_net,ret;
	
	if(argc!=2){
		printf("usage: %s <hexpid>|-<dev#>\n", argv[0]);
		exit(1);
	};

	if (argv[1][0]=='-')
	{
		fd_net = open("/dev/dvb/card0/net0", O_RDWR|O_NONBLOCK);
		if(fd_net <0){
			printf("couldn't open /dev/dvb/card0/net0");
			exit(2);
		};
		ret=ioctl(fd_net,NET_REMOVE_IF,atoi(argv[1]+1));
		
		if(ret){
			printf("cannot remove interface.\n");
		}else{
			printf("device removed\n");
		}
	} else
	{
		int pid;
		sscanf(argv[1], "%x", &pid);
		net1.pid = pid;
		printf("pid %d (%s)\n", net1.pid, argv[1]);

		fd_net 		= open("/dev/dvb/card0/net0", O_RDWR|O_NONBLOCK);
		if(fd_net <0){
			printf("cannot open /dev/dvb/card0/net0\n");
			exit(2);
		};
		ret=ioctl(fd_net,NET_ADD_IF,&net1);
		
		if(ret){
			printf("cannot add interface for pid 0x%x. check that drivers loaded ok, DVB card found, and '/dev/dvb/card0/net0' have major number 250 and minor 9 or 73 or 137 %m\n\n",net1.pid);
		}else{
			printf("device for pid %x created successfully \n\n",net1.pid);
		}
	};
	close(fd_net);
}
