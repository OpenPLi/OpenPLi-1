/*
 * $Id: hotplug.c,v 1.3 2004/05/01 09:03:48 ghostrider Exp $
 *
 * (C) 2002, 2003 by Andreas Monzner <ghostrider@tuxbox.org>
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

int send_to_sock(unsigned char *buf, size_t len) {

	struct sockaddr_un servaddr;
	int clilen;
	int sock;

	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, "/tmp/hotplug.socket");
	clilen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);
	sock = socket(PF_UNIX, SOCK_STREAM, 0);
	connect(sock, (struct sockaddr *) &servaddr, clilen);

	if (write(sock, buf, len) == len)
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

int main(int argc, char **argv) 
{
	unsigned char lenstr[11]="LENGTH = 0";

	char *action=0, *devpath=0, *product=0, *type=0, *interface=0, *devfs=0, *device=0;

	fflush(stdout);
	if (argc < 2 || strcmp(argv[1],"usb") )
	{
		fprintf(stderr, "this programm is called by the kernel...\nread /linux/Documentation/usb/hotplug.txt\n");
		return EXIT_FAILURE;
	}

	lenstr[9]=0;
	// read environment variables
	if ( (action = getenv("ACTION")) )
		lenstr[9]++;
	if ( (devpath = getenv("DEVPATH")) )
		lenstr[9]++;
	if ( (product = getenv("PRODUCT")) )
		lenstr[9]++;
	if ( (type = getenv("TYPE")) )
		lenstr[9]++;
	if ( (interface = getenv("INTERFACE")) )
		lenstr[9]++;
	if ( (devfs = getenv("DEVFS")) )
		lenstr[9]++;
	if ( (device = getenv("DEVICE")) )
		lenstr[9]++;
	if (!lenstr[9])
		return EXIT_FAILURE;

	// first send the count of following messagstrings
	if ( send_to_sock(lenstr, 11) != EXIT_SUCCESS )
		return EXIT_FAILURE;

	if ( action )
	{
		char tmp[6+1+strlen(action)+1];
		strcpy(tmp,"ACTION=");
		strcpy(tmp+7, action);
		if ( send_to_sock(tmp, strlen(tmp)) != EXIT_SUCCESS )
			return EXIT_FAILURE;
	}

	if ( devpath )
	{
		char tmp[7+1+strlen(devpath)+1];
		strcpy(tmp,"DEVPATH=");
		strcpy(tmp+8, devpath);
		if ( send_to_sock(tmp, strlen(tmp)) != EXIT_SUCCESS )
			return EXIT_FAILURE;
	}

	if ( product )
	{
		char tmp[7+1+strlen(product)+1];
		strcpy(tmp,"PRODUCT=");
		strcpy(tmp+8, product);
		if ( send_to_sock(tmp, strlen(tmp)) != EXIT_SUCCESS )
			return EXIT_FAILURE;
	}

	if ( type )
	{
		char tmp[4+1+strlen(type)+1];
		strcpy(tmp,"TYPE=");
		strcpy(tmp+5, type);
		if ( send_to_sock(tmp, strlen(tmp)) != EXIT_SUCCESS )
			return EXIT_FAILURE;
	}

	if ( interface )
	{
		char tmp[9+1+strlen(interface)+1];
		strcpy(tmp,"INTERFACE=");
		strcpy(tmp+10, interface);
		if ( send_to_sock(tmp, strlen(tmp)) != EXIT_SUCCESS )
			return EXIT_FAILURE;
	}
	
	if ( devfs )
	{
		char tmp[5+1+strlen(devfs)+1];
		strcpy(tmp,"DEVFS=");
		strcpy(tmp+6, devfs);
		if ( send_to_sock(tmp, strlen(tmp)) != EXIT_SUCCESS )
			return EXIT_FAILURE;
	}

	if ( device )
	{
		char tmp[6+1+strlen(device)+1];
		strcpy(tmp,"DEVICE=");
		strcpy(tmp+7, device);
		if ( send_to_sock(tmp, strlen(tmp)) != EXIT_SUCCESS )
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

