/*
	DBoxII-Project

	2002 by Zwen
	Homepage: http://www.dbox2.info/

	Kommentar:

	Lircd Client Klasse. Verpackung des LIRC Tools RC in client Klasse
	
	License: GPL

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

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <cstdlib>

#include "liblircdclient.h"

#define PACKET_SIZE 256
/* three seconds */
#define TIMEOUT 3

static int debug = 1;
#define dprintf(fmt, args...) {if(debug) printf( "[lircdclient] " fmt, ## args);}

const char* CLircdClient::ReadString()
{
	static char buffer[PACKET_SIZE+1]="";
	char *end;
	static int ptr=0;
	ssize_t ret=0;
	fd_set rfds;
	struct timeval tv;
	int retval;
		
	if(ptr>0)
	{
		memmove(buffer,buffer+ptr,strlen(buffer+ptr)+1);
		ptr=strlen(buffer);
		end=strchr(buffer,'\n');
	}
	else
	{
		end=NULL;
	}
	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;


	while(end==NULL)
	{
		if(PACKET_SIZE<=ptr)
		{
			dprintf("lirc bad packet\n");
			ptr=0;
			return(NULL);
		}
		retval = select(fd+1, &rfds, NULL, NULL, &tv);
		if(retval)
		{
			ret=read(fd,buffer+ptr,PACKET_SIZE-ptr);
			if(ret<=0)
			{
				dprintf("error reading lirc answer\n");
				ptr=0;
				return(NULL);
			}
		}
		else
		{
			dprintf("lirc timeout on read answer\n");
			ptr=0;
			return(NULL);
		}
		buffer[ptr+ret]=0;
		ptr=strlen(buffer);
		end=strchr(buffer,'\n');
	}

	end[0]=0;
	ptr=strlen(buffer)+1;
	return(buffer);
}

int CLircdClient::SendOnce(string device, string key)
{
	return Send("SEND_ONCE",device,key);
}
int CLircdClient::SendUsecs(string device, string key, unsigned long usecs)
{
	int ret1=Send("SEND_START",device,key);
	usleep(usecs);
	int ret2=Send("SEND_STOP",device,key);
	return ret1+ret2;
}
int CLircdClient::Send(string cmd, string device, string key)
{
	enum packet_state
	{
		P_BEGIN,
		P_MESSAGE,
		P_STATUS,
		P_DATA,
		P_N,
		P_DATA_N,
		P_END
	};
	int done,todo;
	const char *str,*data;
	char *endptr;
	enum packet_state state;
	int status,n;
	unsigned long data_n=0;
	char packet[PACKET_SIZE];

	sprintf(packet,"%s %s %s\n",cmd.c_str(),device.c_str(),key.c_str());
	dprintf("sending to lircd %s",packet);
	todo=strlen(packet);
	data=packet;
	while(todo>0)
	{
		done=write(fd,(void *) data,todo);
		if(done<0)
		{
			dprintf("lirc could not send packet\n");
			return(-1);
		}
		data+=done;
		todo-=done;
	}

	/* get response */
	status=0;
	state=P_BEGIN;
	n=0;
	while(1)
	{
		str=ReadString();
		if(str==NULL) return(-1);
		switch(state)
		{
		case P_BEGIN:
			if(strcasecmp(str,"BEGIN")!=0)
			{
				continue;
			}
			state=P_MESSAGE;
			break;
		case P_MESSAGE:
			if(strncasecmp(str,packet,strlen(str))!=0 ||
			   strlen(str)+1!=strlen(packet))
			{
				state=P_BEGIN;
				continue;
			}
			state=P_STATUS;
			break;
		case P_STATUS:
			if(strcasecmp(str,"SUCCESS")==0)
			{
				status=0;
			}
			else if(strcasecmp(str,"END")==0)
			{
				status=0;
				return(status);
			}
			else if(strcasecmp(str,"ERROR")==0)
			{
				dprintf("lirc command failed: %s",packet);
				status=-1;
			}
			else
			{
				goto bad_packet;
			}
			state=P_DATA;
			break;
		case P_DATA:
			if(strcasecmp(str,"END")==0)
			{
				return(status);
			}
			else if(strcasecmp(str,"DATA")==0)
			{
				state=P_N;
				break;
			}
			goto bad_packet;
		case P_N:
			errno=0;
			data_n=strtoul(str,&endptr,0);
			if(!*str || *endptr)
			{
				goto bad_packet;
			}
			if(data_n==0)
			{
				state=P_END;
			}
			else
			{
				state=P_DATA_N;
			}
			break;
		case P_DATA_N:
			dprintf("lirc: %s\n",str);
			n++;
			if(n==(int)data_n) state=P_END;
			break;
		case P_END:
			if(strcasecmp(str,"END")==0)
			{
				return(status);
			}
			goto bad_packet;
			break;
		}
	}
 bad_packet:
	dprintf("lirc bad return packet\n");
	return(-1);
}
//-------------------------------------------------
void CLircdClient::Disconnect()
{
	close(fd);
}

//-------------------------------------------------------------------------
bool CLircdClient::Connect()
{
	struct sockaddr_un addr;

	addr.sun_family=AF_UNIX;
	strcpy(addr.sun_path, "/dev/lircd");
	fd = socket(AF_UNIX,SOCK_STREAM,0);
	if(!fd)
	{
		dprintf("could not open lircd-socket\n");
		return false;
	};

	if(connect(fd,(struct sockaddr *)&addr,sizeof(addr))==-1)
	{
		dprintf("could not connect to lircd-socket\n");
		return false;
	};
	return true;

}

