/*
 * audio.h
 *
 * Copyright (C) 2000 Ralph  Metzler <ralph@convergence.de>
 *		  & Marcus Metzler <marcus@convergence.de>
		      for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Lesser Public License
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

#ifndef _OST_AUDIO_H_
#define _OST_AUDIO_H_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#define boolean int
#define true 1
#define false 0

typedef enum {
	AUDIO_SOURCE_DEMUX, /* Select the demux as the main source */
	AUDIO_SOURCE_MEMORY /* Select internal memory as the main source */
} audioStreamSource_t;

typedef enum {
	AUDIO_STOPPED,	/* Device is stopped */
	AUDIO_PLAYING,	/* Device is currently playing */
	AUDIO_PAUSED	/* Device is paused */
} audioPlayState_t;

typedef enum {
	AUDIO_STEREO,
	AUDIO_MONO_LEFT,
	AUDIO_MONO_RIGHT,
} audioChannelSelect_t;

typedef enum {
	SCMS_COPIES_NONE,
	SCMS_COPIES_ONE,
	SCMS_COPIES_UNLIMITED,
} audioSpdifCopyState_t;

typedef struct audioStatus {
	boolean AVSyncState;			/* sync audio and video? */
	boolean muteState;			/* audio is muted */
	audioPlayState_t playState;		/* current playback state */
	audioStreamSource_t streamSource;	/* current stream source */
	audioChannelSelect_t channelSelect;	/* currently selected channel */
	boolean bypassMode;			/* pass on audio data to separate decoder hardware */
} audioStatus_t;

typedef struct audioMixer {
	unsigned int volume_left;
	unsigned int volume_right;
	// what else do we need? bass, pass-through, ...
} audioMixer_t;

typedef
struct audioKaraoke {	/* if Vocal1 or Vocal2 are non-zero, they get mixed  */
	int vocal1;	/* into left and right t at 70% each */
	int vocal2;	/* if both, Vocal1 and Vocal2 are non-zero, Vocal1 gets  */
	int melody;	/* mixed into the left channel and */
			/* Vocal2 into the right channel at 100% each. */
			/* if Melody is non-zero, the melody channel gets mixed  */
			/* into left and right  */
} audioKaraoke_t;

typedef uint16_t audioAttributes_t;
/*   bits: descr. */
/*   15-13 audio coding mode (0=ac3, 2=mpeg1, 3=mpeg2ext, 4=LPCM, 6=DTS, */
/*   12    multichannel extension */
/*   11-10 audio type (0=not spec, 1=language included) */
/*    9- 8 audio application mode (0=not spec, 1=karaoke, 2=surround) */
/*    7- 6 Quantization / DRC (mpeg audio: 1=DRC exists)(lpcm: 0=16bit,  */
/*    5- 4 Sample frequency fs (0=48kHz, 1=96kHz) */
/*    2- 0 number of audio channels (n+1 channels) */


/* for GET_CAPABILITIES and SET_FORMAT, the latter should only set one bit */
#define AUDIO_CAP_DTS    1
#define AUDIO_CAP_LPCM   2
#define AUDIO_CAP_MP1    4
#define AUDIO_CAP_MP2    8
#define AUDIO_CAP_MP3   16
#define AUDIO_CAP_AAC   32
#define AUDIO_CAP_OGG   64
#define AUDIO_CAP_SDDS 128
#define AUDIO_CAP_AC3  256

#define AUDIO_STOP		_IO('o', 1)
#define AUDIO_PLAY		_IO('o', 2)
#define AUDIO_PAUSE		_IO('o', 3)
#define AUDIO_CONTINUE		_IO('o', 4)
#define AUDIO_SELECT_SOURCE	_IOW('o', 5, audioStreamSource_t)
#define AUDIO_SET_MUTE		_IOW('o', 6, boolean)
#define AUDIO_SET_AV_SYNC	_IOW('o', 7, boolean)
#define AUDIO_SET_BYPASS_MODE	_IOW('o', 8, boolean)
#define AUDIO_CHANNEL_SELECT	_IOW('o', 9, audioChannelSelect_t)
#define AUDIO_GET_STATUS	_IOR('o', 10, audioStatus_t *)
#define AUDIO_GET_CAPABILITIES	_IOR('o', 11, unsigned int *)
#define AUDIO_CLEAR_BUFFER	_IO('o',  12)
#define AUDIO_SET_ID		_IOW('o', 13, int)
#define AUDIO_SET_MIXER		_IOW('o', 14, audioMixer_t *)
#define AUDIO_SET_STREAMTYPE	_IOW('o', 15, unsigned int)
#define AUDIO_SET_EXT_ID	_IOW('o', 16, int)
#define AUDIO_SET_ATTRIBUTES	_IOW('o', 17, audioAttributes_t)
#define AUDIO_SET_KARAOKE	_IOW('o', 18, audioKaraoke_t *)

/* Tuxbox Linux extensions, not in LinuxTV API */
#define AUDIO_SET_SPDIF_COPIES	_IOW('o', 100, audioSpdifCopyState_t)

#endif /* _OST_AUDIO_H_ */
