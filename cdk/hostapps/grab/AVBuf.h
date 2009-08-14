/*
 * $Id: AVBuf.h,v 1.1 2001/12/22 17:14:52 obi Exp $
 *
 * Copyright (C) 2001 Peter Niemayer et al.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Log: AVBuf.h,v $
 * Revision 1.1  2001/12/22 17:14:52  obi
 * - added hostapps
 * - added grab to hostapps
 *
 *
 */

#include <pthread.h>

#define VIDEO_BUF_SIZE (0x10000)
#define AUDIO_BUF_SIZE ( 0x0800)
#define AUX_BUF_SIZE   (   0x20)

class AVBuf {
public:

	pthread_mutex_t mutex;

	bool resync; // if packets were lost before this buffer, resync is set to true;

	unsigned long video_valid;
	unsigned long audio_valid;
	unsigned long aux_valid;

	unsigned char video[VIDEO_BUF_SIZE];
	unsigned char audio[AUDIO_BUF_SIZE];
	unsigned char aux  [AUX_BUF_SIZE];

	void clear()
	{
		resync = false;
		video_valid = 0;
		audio_valid = 0;
		aux_valid = 0;
	}
	
	bool is_used() const
	{
		if (video_valid || audio_valid || aux_valid || resync)
			return true;
		else
			return false;
	}

	AVBuf()
	{
		pthread_mutex_init(&mutex, 0);
		clear();
	}

};
