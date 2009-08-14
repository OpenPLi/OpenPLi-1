/***************************************************************************
                          wrappers.cpp  -  description
                             -------------------
    begin                : Oct 2005
    copyright            : (C) 2005 by pieterg
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/poll.h>
#include <sys/uio.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/select.h>

#include "wrappers.h"

ssize_t Write(int fd, const void *buf, size_t count)
{
	int iRetval;
	char *ptr = (char*)buf;
	size_t handledcount = 0;
	while (handledcount < count)
	{
		iRetval = write(fd, &ptr[handledcount], count - handledcount);

		if (iRetval == 0) return -1;
		if (iRetval < 0)
		{
			if (errno == EINTR) continue;
			printf("write failed %i\n", errno);
			return iRetval;
		}
		handledcount += iRetval;
	}
	return handledcount;
}

ssize_t Read(int fd, void *buf, size_t count)
{
	int iRetval;
	char *ptr = (char*)buf;
	size_t handledcount = 0;
	while (handledcount < count)
	{
		iRetval = read(fd, &ptr[handledcount], count - handledcount);

		if (iRetval == 0) return handledcount;
		if (iRetval < 0)
		{
			if (errno == EINTR) continue;
			printf("read failed %i\n", errno);
			return iRetval;
		}
		handledcount += iRetval;
	}
	return handledcount;
}

int Select(int iMaxfd, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
	int iRetVal;
	fd_set rset, wset, xset;
	struct timeval interval;

	/* make a backup of all fd_set's and timeval struct */
	if (readfds) rset = *readfds;
	if (writefds) wset = *writefds;
	if (exceptfds) xset = *exceptfds;
	if (timeout) interval = *timeout;

	while (1)
	{
		iRetVal = select(iMaxfd, readfds, writefds, exceptfds, timeout);

		if (iRetVal < 0)
		{
			/* restore the backup before we continue */
			if (readfds) *readfds = rset;
			if (writefds) *writefds = wset;
			if (exceptfds) *exceptfds = xset;
			if (timeout) *timeout = interval;
			if (errno == EINTR) continue;
			printf("select failed %i\n", errno);
			break;
		}

		break;
	}
	return iRetVal;
}

int Connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
	int iRetval;
	while (1)
	{
		iRetval = connect(sockfd, serv_addr, addrlen);

		if (iRetval < 0)
		{
			if (errno == EINTR || errno == EINPROGRESS)
			{
				int error;
				socklen_t len = sizeof(error);
				timeval timeout;
				fd_set wset;
				FD_ZERO(&wset);
				FD_SET(sockfd, &wset);
	
				timeout.tv_sec = 10; /* 10 second connect timeout */
				timeout.tv_usec = 0;
	
				if (Select(sockfd + 1, NULL, &wset, NULL, &timeout) <= 0) break;
	
				if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) < 0) break;
	
				if (error) break;
				iRetval = 0;
				break;
			}
			printf("connect failed %i\n", errno);
		}
		break;
	}
	return iRetval;
}

int Bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen)
{
	int iRetval;
	while (1)
	{
		iRetval = bind(sockfd, my_addr, addrlen);

		if (iRetval < 0)
		{
			if (errno == EINTR) continue;
			printf("bind failed %i\n", errno);
		}
		break;
	}
	return iRetval;
}

int Listen(int s, int backlog)
{
	int iRetval;
	while (1)
	{
		iRetval = listen(s, backlog);

		if (iRetval < 0)
		{
			if (errno == EINTR) continue;
			printf("listen failed %i\n", errno);
		}
		break;
	}
	return iRetval;
}

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	int iRetval;
	while (1)
	{
		iRetval = accept(s, addr, addrlen);

		if (iRetval < 0)
		{
			if (errno == EINTR) continue;
			printf("accept failed %i\n", errno);
		}
		break;
	}
	return iRetval;
}

