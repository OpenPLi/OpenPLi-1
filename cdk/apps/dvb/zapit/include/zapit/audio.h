/*
 * $Id: audio.h,v 1.9 2002/10/03 02:04:22 obi Exp $
 *
 * (C) 2002 by Steffen Hehn 'McClean' &
 *	Andreas Oberritter <obi@tuxbox.org>
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

#ifndef __audio_h__
#define __audio_h__

/* nokia api */
#include <ost/audio.h>

class CAudio
{
	private:
		/* dvb audio device */
		int fd;

		/* current audio settings */
		struct audioStatus status;
		struct audioMixer mixer;

		/* internal methods */
		int setMute (bool mute);
		int setBypassMode (bool bypass);

		/* true if constructor had success */
		bool initialized;

	public:
		/* construct & destruct */
		CAudio();
		~CAudio();

		/* check if initialitation failed before rocking */
		bool isInitialized () { return initialized; }

		/* shut up */
		int mute ();
		int unmute ();

		/* bypass audio to external decoder */
		int enableBypass ();
		int disableBypass ();

		/* volume, min = 0, max = 255 */
		int setVolume (unsigned char left, unsigned char right);

		/* start and stop audio */
		int start ();
		int stop ();

		/* stream source */
		audioStreamSource_t getSource () { return status.streamSource; }
		int setSource (audioStreamSource_t source);

		/* select channels */
		int selectChannel (audioChannelSelect_t sel);
		audioChannelSelect_t getSelectedChannel ();
};

#endif /* __audio_h__ */
