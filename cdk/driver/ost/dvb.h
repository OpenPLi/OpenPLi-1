/*
 * $Id: dvb.h,v 1.1 2002/09/18 10:18:15 obi Exp $
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

#ifndef __ost_dvb_h__
#define __ost_dvb_h__

#define AUDIO_MIXER_MAX_VOLUME  255
#define AVIA_GT_DMX_FILTER_NUM  32

typedef enum {
	STREAM_TYPE_SPTS_ES,
	STREAM_TYPE_DPES_ES,
	STREAM_TYPE_DPES_PES
} streamType_t;


#define MAX_NUM_DVB 1
#define DVB_DEVS_MAX 16

typedef struct dvb_devs_s {
	int               num;
	int               tab[DVB_DEVS_MAX];
	int               max_users[DVB_DEVS_MAX];
	int               max_writers[DVB_DEVS_MAX];
} dvb_devs_t;

typedef struct sec_s {
	/* Sat line control */
	int power;             /* LNB power 0=off/pass through, 1=on */
	int volt;              /* 14/18V (V=0/H=1) */
	int ttk;               /* 22KHz */
} sec_t;

/* place to store all the necessary device information */
struct dvb_struct {
	int num;
	dmxdev_t dmxdev;
	int secbusy;

	boolean video_blank;
	struct videoStatus videostate;
	struct audioStatus audiostate;
	
	u32 powerstate;
	int registered;

	/* DVB device */

	struct dvb_device dvb_dev;
	dvb_devs_t *dvb_devs;
	int users[DVB_DEVS_MAX];
	int writers[DVB_DEVS_MAX];
	dvb_net_t *dvb_net;

	dvb_front_t *frontend;
	sec_t sec;

	/* dbox2 only */

	struct audioMixer       audiomixer;
	dmxdev_state_t          dmxstate[AVIA_GT_DMX_FILTER_NUM];
	dvb_pid_t               audio_pid;
	dvb_pid_t               video_pid;
	streamType_t            audio_stream_type;
	streamType_t            video_stream_type;
};

static int secSetTone(struct dvb_struct *dvb, secToneMode mode);
static int secSetVoltage(struct dvb_struct *dvb, secVoltage voltage);

#endif /* __ost_dvb_h__ */

