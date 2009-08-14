/*
 * $Id: video.cpp,v 1.4 2002/10/12 20:19:44 obi Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
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

#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <zapit/settings.h>
#include <zapit/video.h>

CVideo::CVideo ()
{
	initialized = false;
	status.playState = VIDEO_STOPPED;
	status.streamSource = VIDEO_SOURCE_DEMUX;

	if ((fd = open(VIDEO_DEVICE, O_RDWR)) < 0)
	{
		perror(VIDEO_DEVICE);
	}
	else if (ioctl(fd, VIDEO_GET_STATUS, &status) < 0)
	{
		perror("VIDEO_GET_STATUS");
		close(fd);
	}
	else
	{
		initialized = true;
	}
}

CVideo::~CVideo ()
{
	if (initialized)
	{
		close(fd);
	}
}

int CVideo::setAspectRatio (videoFormat_t format)
{
	if (status.videoFormat == format)
	{
		return 0;
	}

	if (ioctl(fd, VIDEO_SET_FORMAT, format) < 0)
	{
		perror("VIDEO_SET_FORMAT");
		return -1;
	}

	status.videoFormat = format;
	return 0;
}

int CVideo::setCroppingMode (videoDisplayFormat_t format)
{
	if (status.displayFormat == format)
	{
		return 0;
	}

	if (ioctl(fd, VIDEO_SET_DISPLAY_FORMAT, format) < 0)
	{
		perror("VIDEO_SET_DISPLAY_FORMAT");
		return -1;
	}

	status.displayFormat = format;
	return 0;
}

int CVideo::setSource (videoStreamSource_t source)
{
#ifndef ALWAYS_DO_VIDEO_SELECT_SOURCE
	if (status.streamSource == source)
	{
		return 0;
	}
#endif
	if (status.playState != VIDEO_STOPPED)
	{
		return -1;
	}

	if (ioctl(fd, VIDEO_SELECT_SOURCE, source) < 0)
	{
		perror("VIDEO_SELECT_SOURCE");
		return -1;
	}

	status.streamSource = source;

	return 0;
}

int CVideo::start ()
{
	if (status.playState == VIDEO_PLAYING)
	{
		return 0;
	}

	if (ioctl(fd, VIDEO_PLAY) < 0)
	{
		perror("VIDEO_PLAY");
		return -1;
	}

	status.playState = VIDEO_PLAYING;

	return 0;
}

int CVideo::stop ()
{
	if (status.playState == VIDEO_STOPPED)
	{
		return 0;
	}

	if (ioctl(fd, VIDEO_STOP, status.videoBlank) < 0)
	{
		perror("VIDEO_STOP");
		return -1;
	}

	status.playState = VIDEO_STOPPED;

	return 0;
}

int CVideo::setBlank (bool blank)
{
	if (status.videoBlank == blank)
	{
		return 0;
	}

	if (ioctl(fd, VIDEO_SET_BLANK, blank) < 0)
	{
		perror("VIDEO_SET_BLANK");
		return -1;
	}

	status.videoBlank = blank;

	return 0;
}

