/*
 * $Id: video.h,v 1.3 2002/09/21 20:20:05 thegoodguy Exp $
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

#ifndef __video_h__
#define __video_h__

#include <time.h>

#include <ost/video.h>

class CVideo
{
	private:
		/* video device */
		int fd;

		/* video status */
		struct videoStatus status;

		/* true when construction was complete */
		bool initialized;

	public:
		/* constructor & destructor */
		CVideo ();
		~CVideo ();

		bool isInitialized () { return initialized; }

		/* aspect ratio */
		videoFormat_t getAspectRatio () { return status.videoFormat; }
		int setAspectRatio (videoFormat_t format);

		/* cropping mode */
		videoDisplayFormat_t getCroppingMode () { return status.displayFormat; }
		int setCroppingMode (videoDisplayFormat_t format);

		/* stream source */
		videoStreamSource_t getSource () { return status.streamSource; }
		int setSource (videoStreamSource_t source);

		/* blank on freeze */
		bool getBlank () { return status.videoBlank; }
		int setBlank (bool blank);

		/* get play state */
		bool isPlaying () { return (status.playState == VIDEO_PLAYING); }

		/* change video play state */
		int start ();
		int stop ();
};

#endif /* __video_h__ */
