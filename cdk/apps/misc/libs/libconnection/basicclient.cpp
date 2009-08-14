/*
 * $Header: /cvs/tuxbox/apps/misc/libs/libconnection/basicclient.cpp,v 1.5.2.1 2003/02/06 19:54:58 thegoodguy Exp $
 *
 * Basic Client Class (Neutrino) - DBoxII-Project
 *
 * (C) 2002 by thegoodguy <thegoodguy@berlios.de>
 *
 * License: GPL
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
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "basicclient.h"
#include "basicmessage.h"

CBasicClient::CBasicClient()
{
	sock_fd = -1;
}

bool CBasicClient::open_connection()
{
	close_connection();

	struct sockaddr_un servaddr;
	int clilen;

	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, getSocketName());              // no length check !!!
	clilen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);

	if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		printf("[CBasicClient] socket failed.\n");
		perror(getSocketName());
		sock_fd = -1;
		return false;
	}

	if (connect(sock_fd, (struct sockaddr*) &servaddr, clilen) < 0)
	{
		printf("[CBasicClient] connect failed.\n");
		perror(getSocketName());
		close_connection();
		return false;
	}
	return true;
}

void CBasicClient::close_connection()
{
	if (sock_fd != -1)
	{
		close(sock_fd);
		sock_fd = -1;
	}
}

bool CBasicClient::send_data(const char* data, const size_t size)
{
	if (sock_fd == -1)
	    return false;

	if (::send(sock_fd, data, size, MSG_NOSIGNAL) < 0) // better: == -1
	{
		printf("[CBasicClient] send failed.\n");
		perror(getSocketName());
		return false;
	}
	
	return true;
}

bool CBasicClient::receive_data(char* data, const size_t size)
{
	if (sock_fd == -1)
		return false;
	else
		return (read(sock_fd, data, size) > 0);  // case size == 0 uncorrect handled ?
}

bool CBasicClient::send(const unsigned char command, const char* data, const unsigned int size)
{
	CBasicMessage::Header msgHead;
	msgHead.version = getVersion();
	msgHead.cmd     = command;

	open_connection(); // if the return value is false, the next send_data call will return false, too

	if (!send_data((char*)&msgHead, sizeof(msgHead)))
	    return false;
	
	if (size != 0)
	    return send_data(data, size);

	return true;
}

