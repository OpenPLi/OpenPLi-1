/*
 * video.h
 *
 * Copyright (C) 2000 Marcus Metzler <marcus@convergence.de>
 *		  & Ralph  Metzler <ralph@convergence.de>
		      for convergence integrated media GmbH
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

#ifndef _OST_VIDEO_H_
#define _OST_VIDEO_H_

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

#define boolean int
#define true 1
#define false 0

typedef enum {
	VIDEO_FORMAT_4_3,	/* Select 4:3 format */
	VIDEO_FORMAT_16_9,	/* Select 16:9 format. */
	VIDEO_FORMAT_20_9,	/* Select 20:9 format. */
	VIDEO_FORMAT_AUTO	/* Use information from bitstream */
} videoFormat_t;

typedef enum {
	 VIDEO_SYSTEM_PAL,
	 VIDEO_SYSTEM_NTSC,
	 VIDEO_SYSTEM_PALN,
	 VIDEO_SYSTEM_PALNc,
	 VIDEO_SYSTEM_PALM,
	 VIDEO_SYSTEM_NTSC60,
	 VIDEO_SYSTEM_PAL60,
	 VIDEO_SYSTEM_PALM60
} videoSystem_t;

typedef enum {
	VIDEO_PAN_SCAN,       /* use pan and scan format */
	VIDEO_LETTER_BOX,     /* use letterbox format */
	VIDEO_CENTER_CUT_OUT  /* use center cut out format */
} videoDisplayFormat_t;

typedef enum {
	VIDEO_SOURCE_DEMUX, /* Select the demux as the main source */
	VIDEO_SOURCE_MEMORY /* If this source is selected, the stream
			       comes from the user through the write
			       system call */
} videoStreamSource_t;

typedef enum {
	VIDEO_STOPPED, /* Video is stopped */
	VIDEO_PLAYING, /* Video is currently playing */
	VIDEO_FREEZED  /* Video is freezed */
} videoPlayState_t;

struct videoEvent {
	int32_t type;
	time_t timestamp;
	union {
		videoFormat_t videoFormat;
	} u;
};

struct videoStatus {
	boolean videoBlank;			/* blank video on freeze? */
	videoPlayState_t playState;		/* current state of playback */
	videoStreamSource_t streamSource;	/* current source (demux/memory) */
	videoFormat_t videoFormat;		/* current aspect ratio of stream */
	videoDisplayFormat_t displayFormat;	/* selected cropping mode */
};

/* pointer to and size of a single iframe in memory */
struct videoDisplayStillPicture {
	char *iFrame;
	int32_t size;
};


typedef
struct videoHighlight {
	boolean active;      /*    1=show highlight, 0=hide highlight */
	uint8_t contrast1;   /*    7- 4  Pattern pixel contrast */
			     /*    3- 0  Background pixel contrast */
	uint8_t contrast2;   /*    7- 4  Emphasis pixel-2 contrast */
			     /*    3- 0  Emphasis pixel-1 contrast */
	uint8_t color1;      /*    7- 4  Pattern pixel color */
			     /*    3- 0  Background pixel color */
	uint8_t color2;      /*    7- 4  Emphasis pixel-2 color */
			     /*    3- 0  Emphasis pixel-1 color */
	uint32_t ypos;       /*   23-22  auto action mode */
			     /*   21-12  start y */
			     /*    9- 0  end y */
	uint32_t xpos;       /*   23-22  button color number */
			     /*   21-12  start x */
			     /*    9- 0  end x */
} videoHighlight_t;


typedef
struct videoSPU {
	boolean active;
	int streamID;
} videoSPU_t;

typedef
struct videoSPUPalette{      /* SPU Palette information */
	int length;
	uint8_t *palette;
} videoSPUPalette_t;

typedef
struct videoNaviPack{
	int length;	  /* 0 ... 1024 */
	uint8_t data[1024];
} videoNaviPack_t;

typedef
struct videoDigest {
	int16_t x;
	int16_t y;
	int32_t skip;
	int16_t decimation;
	int16_t threshold;
	int16_t pictureID;
} videoDigest_t;

typedef uint16_t videoAttributes_t;
/*   bits: descr. */
/*   15-14 Video compression mode (0=MPEG-1, 1=MPEG-2) */
/*   13-12 TV system (0=525/60, 1=625/50) */
/*   11-10 Aspect ratio (0=4:3, 3=16:9) */
/*    9- 8 permitted display mode on 4:3 monitor (0=both, 1=only pan-sca */
/*    7    line 21-1 data present in GOP (1=yes, 0=no) */
/*    6    line 21-2 data present in GOP (1=yes, 0=no) */
/*    5- 3 source resolution (0=720x480/576, 1=704x480/576, 2=352x480/57 */
/*    2    source letterboxed (1=yes, 0=no) */
/*    0    film/camera mode (0=camera, 1=film (625/50 only)) */


/* bit definitions for capabilities: */
/* can the hardware decode MPEG1 and/or MPEG2? */
#define VIDEO_CAP_MPEG1   1
#define VIDEO_CAP_MPEG2   2
/* can you send a system and/or program stream to video device?
   (you still have to open the video and the audio device but only
    send the stream to the video device) */
#define VIDEO_CAP_SYS     4
#define VIDEO_CAP_PROG    8
/* can the driver also handle SPU, NAVI and CSS encoded data?
   (CSS API is not present yet) */
#define VIDEO_CAP_SPU    16
#define VIDEO_CAP_NAVI   32
#define VIDEO_CAP_CSS    64


#define VIDEO_STOP			_IOW('o', 21, boolean)
#define VIDEO_PLAY			_IO('o', 22)
#define VIDEO_FREEZE			_IO('o', 23)
#define VIDEO_CONTINUE			_IO('o', 24)
#define VIDEO_SELECT_SOURCE		_IOW('o', 25, videoStreamSource_t)
#define VIDEO_SET_BLANK			_IOW('o', 26, boolean)
#define VIDEO_GET_STATUS		_IOR('o', 27, struct videoStatus *)
#define VIDEO_GET_EVENT			_IOR('o', 28, struct videoEvent *)
#define VIDEO_SET_DISPLAY_FORMAT	_IOW('o', 29, videoDisplayFormat_t)
#define VIDEO_STILLPICTURE		_IOW('o', 30, struct videoDisplayStillPicture *)
#define VIDEO_FAST_FORWARD		_IOW('o', 31, int)
#define VIDEO_SLOWMOTION		_IOW('o', 32, int)
#define VIDEO_GET_CAPABILITIES		_IOR('o', 33, unsigned int *)
#define VIDEO_CLEAR_BUFFER		_IO('o',  34)
#define VIDEO_SET_ID			_IOW('o', 35, unsigned char)
#define VIDEO_SET_STREAMTYPE		_IOW('o', 36, int)
#define VIDEO_SET_FORMAT		_IOW('o', 37, videoFormat_t)
#define VIDEO_SET_SYSTEM		_IOW('o', 38, videoSystem_t)
#define VIDEO_SET_HIGHLIGHT		_IOW('o', 39, videoHighlight_t *)
#define VIDEO_SET_SPU			_IOW('o', 50, videoSPU_t *)
#define VIDEO_SET_SPU_Palette		_IOW('o', 51, videoSPUPalette_t *)
#define VIDEO_GET_NAVI			_IOR('o', 52, videoNaviPack_t *)
#define VIDEO_SET_ATTRIBUTES		_IOW('o', 53, videoAttributes_t)
#define VIDEO_DIGEST			_IOW('o', 54, struct videoDigest *)
#endif /*_OST_VIDEO_H_*/
