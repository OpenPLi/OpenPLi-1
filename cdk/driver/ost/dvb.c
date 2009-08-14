/*
 * $Id: dvb.c,v 1.85.2.1 2002/11/17 01:59:13 obi Exp $
 *
 * Copyright (C) 2000-2002 tmbinc, gillem, obi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA	02111-1307, USA.
 *
 */

#include <linux/config.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <asm/system.h>
#include <linux/kmod.h>

#include <dbox/avia_av.h>
#include <dbox/avia_gt.h>
#include <dbox/avia_gt_pcm.h>
#include <dbox/cam.h>

#include "dvbdev.h"

#ifdef __DVB_PACK__
#include <ost/video.h>
#include <ost/audio.h>
#include <ost/demux.h>
#include <ost/dmx.h>
#include <ost/sec.h>
#include <ost/frontend.h>
#include <ost/ca.h>
#include <ost/net.h>
#else
#include <linux/ost/video.h>
#include <linux/ost/audio.h>
#include <linux/ost/demux.h>
#include <linux/ost/dmx.h>
#include <linux/ost/sec.h>
#include <linux/ost/frontend.h>
#include <linux/ost/ca.h>
#include <linux/ost/net.h>
#endif

#include "dvb_demux.h"
#include "dmxdev.h"
#include "dvb_net.h"
#include <dbox/dvb_frontend.h>
#include "dvb.h"



struct dvb_struct dvbs[MAX_NUM_DVB];

/****************************************************************************
 * Avia functions
 ****************************************************************************/

int dvb_select_source (struct dvb_struct *dvb, unsigned int source)
{
	switch (source) {
	case 0: /* demux */
		wDR(BITSTREAM_SOURCE, 0);
		wDR(TM_MODE, rDR(TM_MODE) & ~0x01);
		dvb->audiostate.streamSource = AUDIO_SOURCE_DEMUX;
		dvb->videostate.streamSource = VIDEO_SOURCE_DEMUX;
		break;

	case 1: /* memory */
		wDR(BITSTREAM_SOURCE, 2);
		wDR(TM_MODE, rDR(TM_MODE) | 0x01);
		dvb->audiostate.streamSource = AUDIO_SOURCE_MEMORY;
		dvb->videostate.streamSource = VIDEO_SOURCE_MEMORY;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/****************************************************************************
 * Frontend callbacks and initialization
 ****************************************************************************/

void tuning_complete_cb (void *priv)
{
	int i;
	struct dvb_struct *dvb=(struct dvb_struct*)priv;

	for (i=0; i<dvb->dmxdev.filternum; i++)
		if (dvb->dmxstate[i]==DMXDEV_STATE_GO)
			DmxDevFilterStart(&dvb->dmxdev.filter[i]);
}

void tuning_start_cb (void *priv)
{
	int i;
	struct dvb_struct *dvb=(struct dvb_struct*)priv;

	for (i=0; i<dvb->dmxdev.filternum; i++) {
		dvb->dmxstate[i]=dvb->dmxdev.filter[i].state;
		if (dvb->dmxdev.filter[i].state==DMXDEV_STATE_GO)
			DmxDevFilterStop(&dvb->dmxdev.filter[i]);
	}
}


static int
InitFront(struct dvb_struct *dvb)
{
	dvb_front_t *fe;

	fe=dvb->frontend;

	fe->priv=(void*)dvb;
	fe->complete_cb=tuning_complete_cb;
	fe->start_cb=tuning_start_cb;
	dvb_frontend_init(fe);

	dvb->powerstate=FE_POWER_ON;
	dvb->sec.power=1;

	if (!fe->demod)
		return -1;

	return 0;
}



/******************************************************************************
 * SEC device file operations
 ******************************************************************************/

static int
secSetTone(struct dvb_struct *dvb, secToneMode mode)
{
	int val;
	
	switch (mode) {
	case SEC_TONE_ON:
		val=1;
		break;
	case SEC_TONE_OFF:
		val=0;
		break;
	default:
		return -EINVAL;
	}
	dvb->sec.ttk=val;
	return dvb_frontend_demod_command(dvb->frontend, FE_SEC_SET_TONE, (void*)mode);
}

static int
secSetVoltage(struct dvb_struct *dvb, secVoltage voltage)
{
	int power=1, volt=0;

	switch (voltage) {
	case SEC_VOLTAGE_LT:
	case SEC_VOLTAGE_OFF:
		power=0;
		break;
	case SEC_VOLTAGE_13:
		volt=0;
		break;
	case SEC_VOLTAGE_18:
		volt=1;
		break;
	case SEC_VOLTAGE_13_5:
	case SEC_VOLTAGE_18_5:
	default:
		return -EINVAL;
	}
	dvb->sec.volt=volt;
	return dvb_frontend_demod_command(dvb->frontend, FE_SEC_SET_VOLTAGE, (void*)voltage);
}


static int
secSendSequence(struct dvb_struct *dvb, struct secCmdSequence *seq)
{
	int i, ret;
	struct secCommand scommands;

	switch (seq->miniCommand) {
	case SEC_MINI_NONE:
	case SEC_MINI_A:
	case SEC_MINI_B:
		break;

	default:
		return -EINVAL;
	}

	for (i=0; i<seq->numCommands; i++) {
		if (copy_from_user(&scommands, &seq->commands[i],
				   sizeof(struct secCommand)))
			continue;

		dvb_frontend_demod_command(dvb->frontend, FE_SEC_COMMAND, (void*)&scommands);
	}

	if (seq->miniCommand!=SEC_MINI_NONE)
		dvb_frontend_demod_command(dvb->frontend, FE_SEC_MINI_COMMAND, (void*)seq->miniCommand);

	ret=secSetVoltage(dvb, seq->voltage);
	if (ret<0)
		return ret;
	return secSetTone(dvb, seq->continuousTone);
}



/******************************************************************************
 * DVB device file operations
 ******************************************************************************/

#define INFU 32768

static dvb_devs_t dbox2_devs = {
	8,
	{
		DVB_DEVICE_VIDEO_0, DVB_DEVICE_AUDIO_0,
		DVB_DEVICE_SEC_0,   DVB_DEVICE_FRONTEND_0,
		DVB_DEVICE_DEMUX_0, DVB_DEVICE_DVR_0,
		DVB_DEVICE_CA_0,    DVB_DEVICE_NET_0,
	},
	{ INFU, INFU, INFU, INFU, INFU, 1, 1, 1 },
	{    1,    1,    1,    1, INFU, 1, 1, 1 }
};


static inline int
num2type(struct dvb_struct *dvb, int num)
{
	if (!dvb->dvb_devs)
		return -2;
	if (num>=dvb->dvb_devs->num)
		return -3;
	return dvb->dvb_devs->tab[num];
}


static int
dvbdev_open(struct dvb_device *dvbdev, int num,
            struct inode *inode, struct file *file)
{
	struct dvb_struct *dvb=(struct dvb_struct*)dvbdev->priv;
	int type=num2type(dvb, num);
	int ret=0;

	if (type<0)
		return -EINVAL;

	if (dvb->users[num] >= dvb->dvb_devs->max_users[num])
		return -EBUSY;

	if ((file->f_flags&O_ACCMODE)!=O_RDONLY)
		if (dvb->writers[num] >= dvb->dvb_devs->max_writers[num])
			return -EBUSY;

	switch (type) {
	case DVB_DEVICE_VIDEO_0:
		dvb->video_blank=true;
		dvb->audiostate.AVSyncState=true;
		dvb->videostate.streamSource=VIDEO_SOURCE_DEMUX;
		break;
	case DVB_DEVICE_AUDIO_0:
		dvb->audiostate.streamSource=AUDIO_SOURCE_DEMUX;
		break;
	case DVB_DEVICE_SEC_0:
		if (file->f_flags&O_NONBLOCK)
			ret=-EWOULDBLOCK;
		break;
	case DVB_DEVICE_FRONTEND_0:
		break;
	case DVB_DEVICE_DEMUX_0:
		if ((file->f_flags&O_ACCMODE)!=O_RDWR)
			return -EINVAL;
		ret=DmxDevFilterAlloc(&dvb->dmxdev, file);
		break;
	case DVB_DEVICE_DVR_0:
		ret=DmxDevDVROpen(&dvb->dmxdev, file);
		break;
	case DVB_DEVICE_CA_0:
		break;
	case DVB_DEVICE_NET_0:
		break;
	default:
		return -EINVAL;
	}
	if (ret<0)
		return ret;
	if ((file->f_flags&O_ACCMODE)!=O_RDONLY)
		dvb->writers[num]++;
	dvb->users[num]++;
	MOD_INC_USE_COUNT;
	return ret;
}

static int
dvbdev_close(struct dvb_device *dvbdev, int num,
             struct inode *inode, struct file *file)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	int type=num2type(dvb, num);
	int ret=0;

	switch (type) {
	case DVB_DEVICE_VIDEO_0:
		break;
	case DVB_DEVICE_AUDIO_0:
		break;
	case DVB_DEVICE_SEC_0:
		break;
	case DVB_DEVICE_FRONTEND_0:
		break;
	case DVB_DEVICE_DEMUX_0:
		ret=DmxDevFilterFree(&dvb->dmxdev, file);
		break;
	case DVB_DEVICE_DVR_0:
		ret=DmxDevDVRClose(&dvb->dmxdev, file);
		break;
	case DVB_DEVICE_CA_0:
		break;
	case DVB_DEVICE_NET_0:
		break;
	default:
		return -EINVAL;
	}

	if ((file->f_flags&O_ACCMODE)!=O_RDONLY)
		dvb->writers[num]--;
	dvb->users[num]--;
	MOD_DEC_USE_COUNT;
	return ret;
}

static ssize_t
dvbdev_read(struct dvb_device *dvbdev, int num,
            struct file *file, char *buf, size_t count, loff_t *ppos)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	int type=num2type(dvb, num);

	switch (type) {
	case DVB_DEVICE_VIDEO_0:
		return -EINVAL;
	case DVB_DEVICE_AUDIO_0:
		return -EINVAL;
	case DVB_DEVICE_DEMUX_0:
		return DmxDevRead(&dvb->dmxdev, file, buf, count, ppos);
	case DVB_DEVICE_DVR_0:
		return DmxDevDVRRead(&dvb->dmxdev, file, buf, count, ppos);
	default:
		return -EOPNOTSUPP;
	}
	return 0;
}

static ssize_t
dvbdev_write(struct dvb_device *dvbdev, int num,
             struct file *file,
             const char *buf, size_t count, loff_t *ppos)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	int type=num2type(dvb, num);
	int ret=0;

	switch (type) {
	case DVB_DEVICE_VIDEO_0:
		if (dvb->videostate.streamSource!=VIDEO_SOURCE_MEMORY) {
			ret=-EPERM;
			break;
		}
		ret=-EINVAL;
		break;

	case DVB_DEVICE_AUDIO_0:
		if (dvb->audiostate.streamSource!=AUDIO_SOURCE_MEMORY) {
			ret=-EPERM;
			break;
		}
		ret=-EINVAL;
		break;
		
	case DVB_DEVICE_DVR_0:
		ret=DmxDevDVRWrite(&dvb->dmxdev, file, buf, count, ppos);
		break;

	default:
		ret=-EOPNOTSUPP;
		break;
	}
	return ret;
}


#if 0
static int
tune(struct dvb_struct *dvb, FrontendParameters *para)
{
	if (dvb->frontend->type==DVB_S)
		para->Frequency*=1000;
	return dvb_frontend_tune(dvb->frontend, para);
}
#endif


static int
ca_ioctl(struct dvb_device *dvbdev, struct file *file,
         unsigned int cmd, unsigned long arg)
{
	//struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	void *parg=(void *)arg;

	switch (cmd) {
	case CA_RESET:
		return cam_reset();

	case CA_GET_CAP:
	{
		ca_cap_t cap;
		cap.slot_num=2;
		cap.slot_type=CA_SC;
		cap.descr_num=8;
		cap.descr_type=CA_ECD;
		if(copy_to_user(parg, &cap, sizeof(cap)))
			return -EFAULT;
		break;
	}

	case CA_GET_SLOT_INFO:
	{
		ca_slot_info_t info;
		if (copy_from_user(&info, parg, sizeof(info)))
			return -EFAULT;
		if (info.num>1)
			return -EINVAL;
		return -EOPNOTSUPP;
	}

	case CA_GET_MSG:
	{
		ca_msg_t ca_msg;
		ca_msg.index = 0;
		ca_msg.type  = 0;

		if (copy_from_user(&ca_msg, parg, sizeof(ca_msg_t)))
			return -EFAULT;

		ca_msg.length = cam_read_message(ca_msg.msg, ca_msg.length);

		if (copy_to_user(parg, &ca_msg, sizeof(ca_msg_t)))
			return -EFAULT;
		break;
	}

	case CA_SEND_MSG:
	{
		ca_msg_t ca_msg;

		if (copy_from_user(&ca_msg, parg, sizeof(ca_msg_t)))
			return -EFAULT;

		cam_write_message(ca_msg.msg, ca_msg.length);
		break;
	}

	case CA_SET_DESCR:
	{
		ca_descr_t descr;

		if(copy_from_user(&descr, parg, sizeof(descr)))
			return -EFAULT;
		if(descr.index>=8)
			return -EINVAL;
		if(descr.parity>1)
			return -EINVAL;
		return -EOPNOTSUPP;
	}

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static int
demux_ioctl(struct dvb_device *dvbdev, struct file *file,
            unsigned int cmd, unsigned long arg)
{
	
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	void *parg=(void *)arg;

	if (cmd == DMX_SET_PES_FILTER) {
		struct dmxPesFilterParams * p = parg;
		if (p->output == DMX_OUT_DECODER) {
			switch (p->pesType) {
			case DMX_PES_AUDIO:
				dvb->audio_pid = p->pid;
				break;

			case DMX_PES_VIDEO:
				dvb->video_pid = p->pid;
				break;

			case DMX_PES_PCR:
				avia_flush_pcr();
				if (dvb->dmxdev.demux)
					dvb->dmxdev.demux->flush_pcr();
				break;

			default:
				break;
			}
		}
	}

	return DmxDevIoctl(&dvb->dmxdev, file, cmd, arg);
}

static int
net_ioctl(struct dvb_device *dvbdev, struct file *file,
          unsigned int cmd, unsigned long arg)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	void *parg=(void *)arg;

	if (((file->f_flags&O_ACCMODE)==O_RDONLY))
		return -EPERM;

	switch (cmd) {
	case NET_ADD_IF:
	{
		struct dvb_net_if dvbnetif;
		int result;

		if(copy_from_user(&dvbnetif, parg, sizeof(dvbnetif)))
			return -EFAULT;
		result=dvb->dvb_net->dvb_net_add_if(dvb->dvb_net, dvbnetif.pid);
		if (result<0)
			return result;
		dvbnetif.if_num=result;
		if(copy_to_user(parg, &dvbnetif, sizeof(dvbnetif)))
			return -EFAULT;
		break;
	}

	case NET_REMOVE_IF:
		return dvb->dvb_net->dvb_net_remove_if(dvb->dvb_net, (int) arg);

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static int
video_ioctl(struct dvb_device *dvbdev, struct file *file,
            unsigned int cmd, unsigned long arg)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	void *parg=(void *)arg;
	int ret=0;
	u32 new_mode;

	if (((file->f_flags&O_ACCMODE)==O_RDONLY) &&
	    (cmd!=VIDEO_GET_STATUS))
		return -EPERM;

	switch (cmd) {
	case VIDEO_STOP:
		dvb->videostate.playState=VIDEO_STOPPED;
		avia_command(SelectStream, 0x00, 0xFFFF);
		break;

	case VIDEO_PLAY:
		if ((dvb->audiostate.playState != AUDIO_PLAYING) && (dvb->videostate.playState != VIDEO_PLAYING)) {
			switch (dvb->videostate.streamSource) {
			case VIDEO_SOURCE_DEMUX:
				printk("avia: playing vpid 0x%X apid: 0x%X\n", dvb->video_pid, dvb->audio_pid);
#ifdef AVIA_SPTS
				if (dvb->video_stream_type != STREAM_TYPE_SPTS_ES) {
					if (!aviarev) {
						avia_command(SetStreamType, 0x10, dvb->audio_pid);
						avia_command(SetStreamType, 0x11, dvb->video_pid);
					}
					else {
						avia_command(Reset);
					}
					dvb->video_stream_type = STREAM_TYPE_SPTS_ES;
					dvb->audio_stream_type = STREAM_TYPE_SPTS_ES;
				}
#else
				if ( (dvb->video_stream_type != STREAM_TYPE_DPES_PES) ||
					 (dvb->audio_stream_type != STREAM_TYPE_DPES_PES) ) {
					avia_command(SetStreamType, 0x0B, 0x0000);
					dvb->video_stream_type = STREAM_TYPE_DPES_PES;
					dvb->audio_stream_type = STREAM_TYPE_DPES_PES;
				}
#endif
				avia_command(SelectStream, 0x00, dvb->video_pid);
				avia_command(SelectStream, (dvb->audiostate.bypassMode) ? 0x03 : 0x02, dvb->audio_pid);
				avia_command(Play, 0x00, dvb->video_pid, dvb->audio_pid);
				break;

			case VIDEO_SOURCE_MEMORY:
				dvb->video_stream_type = STREAM_TYPE_DPES_PES;
				dvb->audio_stream_type = STREAM_TYPE_DPES_PES;
				avia_command(SelectStream, 0x0B, 0x0000);
				avia_command(Play, 0x00, 0x0000, 0x0000);
				break;

			default:
				return -EINVAL;
			}
			dvb->audiostate.playState = AUDIO_PLAYING;
			dvb->videostate.playState = VIDEO_PLAYING;
		}
		break;

	case VIDEO_FREEZE:
		dvb->videostate.playState=VIDEO_FREEZED;
		avia_command(Freeze, 0x01);
		break;

	case VIDEO_CONTINUE:
		avia_command(Resume);
		dvb->videostate.playState=VIDEO_PLAYING;
		break;

	case VIDEO_SELECT_SOURCE:
		if ((dvb->audiostate.playState == AUDIO_STOPPED) && (dvb->videostate.playState == VIDEO_STOPPED)) {

			switch ((videoStreamSource_t) arg) {
			case VIDEO_SOURCE_DEMUX:
				if (dvb->videostate.streamSource != VIDEO_SOURCE_DEMUX)
					dvb_select_source(dvb, 0);
				break;

			case VIDEO_SOURCE_MEMORY:
				if (dvb->videostate.streamSource != VIDEO_SOURCE_MEMORY)
					dvb_select_source(dvb, 1);
				break;

			default:
				ret=-EINVAL;
				break;
			}
		}
		else {
			ret=-EINVAL;
		}
		break;

	case VIDEO_SET_BLANK:
		dvb->videostate.videoBlank=(boolean) arg;
		break;

	case VIDEO_GET_STATUS:
		if (copy_to_user(parg, &dvb->videostate, sizeof(struct videoStatus)))
			ret=-EFAULT;
		break;

	case VIDEO_GET_EVENT:
		ret=-EOPNOTSUPP;
		break;

	case VIDEO_SET_DISPLAY_FORMAT:
	{
		videoDisplayFormat_t format=(videoDisplayFormat_t) arg;
		u16 val=0;

		switch (format) {
		case VIDEO_PAN_SCAN:
			val=1;
			break;

		case VIDEO_LETTER_BOX:
			val=2;
			break;

		case VIDEO_CENTER_CUT_OUT:
			val=0;
			break;

		default:
			ret=-EINVAL;
			break;
		}
		if (ret<0)
			break;
		dvb->videostate.displayFormat=format;
		wDR(ASPECT_RATIO_MODE, val);
		break;
	}

	case VIDEO_SET_FORMAT:
		dvb->videostate.videoFormat = (videoFormat_t) arg;

		switch (dvb->videostate.videoFormat) {
		case VIDEO_FORMAT_AUTO:
			wDR(FORCE_CODED_ASPECT_RATIO, 0);
			break;

		case VIDEO_FORMAT_4_3:
			wDR(FORCE_CODED_ASPECT_RATIO, 2);
			break;

		case VIDEO_FORMAT_16_9:
			wDR(FORCE_CODED_ASPECT_RATIO, 3);
			break;

		case VIDEO_FORMAT_20_9:
			wDR(FORCE_CODED_ASPECT_RATIO, 4);
			break;

		default:
			ret=-EINVAL;
			break;
		}
		break;

	case VIDEO_STILLPICTURE:
		if (dvb->videostate.playState == VIDEO_STOPPED)
			ret=-EOPNOTSUPP;
		else
			ret=-EINVAL;
		break;

	case VIDEO_FAST_FORWARD:
		ret=-EOPNOTSUPP;
		break;

	case VIDEO_SLOWMOTION:
		ret=-EOPNOTSUPP;
		break;

	case VIDEO_GET_CAPABILITIES:
	{
		int cap=VIDEO_CAP_MPEG1|
			VIDEO_CAP_MPEG2|
			VIDEO_CAP_SYS|
			VIDEO_CAP_PROG;

		if (copy_to_user(parg, &cap, sizeof(cap)))
			ret=-EFAULT;
		break;
	}

	case VIDEO_CLEAR_BUFFER:
		break;

	case VIDEO_SET_STREAMTYPE:
		if ( (streamType_t) arg > STREAM_TYPE_DPES_PES ) {
			ret = -EINVAL;
			break;
		}

		if (dvb->video_stream_type == (streamType_t) arg)
			break;

		if (rDR(PROC_STATE) != PROC_STATE_IDLE)
			avia_command(Abort, 1);

		if ( (streamType_t) arg == STREAM_TYPE_SPTS_ES ) {
			if (!aviarev) {
				avia_command(SetStreamType, 0x10, dvb->audio_pid);
				avia_command(SetStreamType, 0x11, dvb->video_pid);
			}
			else {	// AVIA 500 doesn't support SetStreamType with type 0x10/0x11
				avia_command(Reset);
			}
			dvb->video_stream_type = STREAM_TYPE_SPTS_ES;
			dvb->audio_stream_type = STREAM_TYPE_SPTS_ES;
			break;
		}

		if ( (streamType_t) arg == STREAM_TYPE_DPES_PES) {
			new_mode = 0x09;
		}
		else
		{
			new_mode = 0x08;
		}

		if ( dvb->audio_stream_type == STREAM_TYPE_SPTS_ES )
		{
			dvb->audio_stream_type = (streamType_t) arg;
		}

		if (dvb->audio_stream_type == STREAM_TYPE_DPES_PES) {
			new_mode |= 0x02;
		}

		avia_command(SetStreamType, new_mode, 0x0000);

		dvb->video_stream_type = (streamType_t) arg;

		break;

	default:
		ret=-ENOIOCTLCMD;
		break;
	}
	return ret;
}

static int
audio_ioctl(struct dvb_device *dvbdev, struct file *file,
            unsigned int cmd, unsigned long arg)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	void *parg=(void *)arg;
	int ret=0;
	u32 new_mode;

	if (((file->f_flags&O_ACCMODE)==O_RDONLY) &&
	    (cmd!=AUDIO_GET_STATUS))
		return -EPERM;

	switch (cmd) {
	case AUDIO_STOP:
		avia_command(SelectStream, (dvb->audiostate.bypassMode) ? 0x03 : 0x02, 0xFFFF);
		dvb->audiostate.playState=AUDIO_STOPPED;
		break;

	case AUDIO_PLAY:
		if ( (dvb->audiostate.playState != AUDIO_PLAYING) || (dvb->audio_stream_type == STREAM_TYPE_SPTS_ES) ) {
			switch (dvb->audiostate.streamSource) {
			case AUDIO_SOURCE_DEMUX:
				printk("avia: playing apid: 0x%X\n", dvb->audio_pid);
#ifdef AVIA_SPTS
				if (dvb->audio_stream_type != STREAM_TYPE_SPTS_ES) {
					if (!aviarev) {
						avia_command(SetStreamType, 0x10, dvb->audio_pid);
						avia_command(SetStreamType, 0x11, dvb->video_pid);
					}
					else {
						avia_command(Reset);
					}
					dvb->audio_stream_type = STREAM_TYPE_SPTS_ES;
					dvb->video_stream_type = STREAM_TYPE_SPTS_ES;
				}
#else
				if (dvb->audio_stream_type != STREAM_TYPE_DPES_PES)
				{
					if (dvb->video_stream_type == STREAM_TYPE_DPES_PES) {
						avia_command(SetStreamType, 0x0B, 0x0000);
					}
					else {
						avia_command(SetStreamType, 0x0A, 0x0000);
					}
					dvb->audio_stream_type = STREAM_TYPE_DPES_PES;
				}
#endif
				avia_command(SelectStream, (dvb->audiostate.bypassMode) ? 0x03 : 0x02, dvb->audio_pid);
				if (dvb->audiostate.playState != AUDIO_PLAYING) {
					if (dvb->videostate.playState == VIDEO_PLAYING) {
						avia_command(Play, 0x00, dvb->video_pid, dvb->audio_pid);
					}
					else {
						avia_command(Play, 0x00, 0xFFFF, dvb->audio_pid);
					}
				}
				break;

			case AUDIO_SOURCE_MEMORY:
				avia_command(SelectStream, 0x0B, 0x0000);
				avia_command(Play, 0x00, 0xFFFF, 0x0000);
				break;

			default:
				return -EINVAL;
			}
			dvb->audiostate.playState = AUDIO_PLAYING;
		}
		break;

	case AUDIO_PAUSE:
		if (dvb->audiostate.playState==AUDIO_PLAYING) {
			//avia_command(Pause, 1, 1); // freeze video, pause audio
			avia_command(Pause, 1, 2); // pause audio (v2.0 silicon only)
			dvb->audiostate.playState=AUDIO_PAUSED;
		} else {
			ret=-EINVAL;
		}
		break;

	case AUDIO_CONTINUE:
		if (dvb->audiostate.playState==AUDIO_PAUSED) {
			dvb->audiostate.playState=AUDIO_PLAYING;
			avia_command(Resume);
		}
		break;

	case AUDIO_SELECT_SOURCE:
		if ((dvb->audiostate.playState == AUDIO_STOPPED) && (dvb->videostate.playState == VIDEO_STOPPED)) {
			switch ((audioStreamSource_t) arg) {
			case AUDIO_SOURCE_DEMUX:
				if (dvb->audiostate.streamSource != AUDIO_SOURCE_DEMUX)
					ret = dvb_select_source(dvb, 0);
				break;

			case AUDIO_SOURCE_MEMORY:
				if (dvb->audiostate.streamSource != AUDIO_SOURCE_MEMORY)
					ret = dvb_select_source(dvb, 1);
				break;

			default:
				ret=-EINVAL;
				break;
			}
		}
		else {
			ret=-EINVAL;
		}
		break;

	case AUDIO_SET_MUTE:
		if (arg) {
			/* mute av spdif (2) and analog audio (4) */
			wDR(AUDIO_CONFIG, rDR(AUDIO_CONFIG) & ~6);
			/* mute gt mpeg */
			avia_gt_pcm_set_mpeg_attenuation(0x00, 0x00);
		} else {
			/* unmute av spdif (2) and analog audio (4) */
			wDR(AUDIO_CONFIG, rDR(AUDIO_CONFIG) | 6);
			/* unmute gt mpeg */
			avia_gt_pcm_set_mpeg_attenuation((dvb->audiomixer.volume_left + 1) >> 1, (dvb->audiomixer.volume_right + 1) >> 1);
		}
		wDR(NEW_AUDIO_CONFIG, 1);
		dvb->audiostate.muteState=(boolean) arg;
		break;

	case AUDIO_SET_AV_SYNC:
		dvb->audiostate.AVSyncState=(boolean) arg;
		wDR(AV_SYNC_MODE, arg ? 0x06 : 0x00);
		break;

	case AUDIO_SET_BYPASS_MODE:
		if (arg) {
			avia_command(SelectStream, 0x02, 0xffff);
			avia_command(SelectStream, 0x03, dvb->audio_pid);
			wDR(AUDIO_CONFIG, rDR(AUDIO_CONFIG) | 1);
		} else {
			avia_command(SelectStream, 0x03, 0xffff);
			avia_command(SelectStream, 0x02, dvb->audio_pid);
			wDR(AUDIO_CONFIG, rDR(AUDIO_CONFIG) & ~1);
		}
		wDR(NEW_AUDIO_CONFIG, 1);
		dvb->audiostate.bypassMode=(boolean) arg;
		break;

	case AUDIO_CHANNEL_SELECT:
		dvb->audiostate.channelSelect=(audioChannelSelect_t) arg;

		switch (dvb->audiostate.channelSelect) {
		case AUDIO_STEREO:
			wDR(AUDIO_DAC_MODE, rDR(AUDIO_DAC_MODE) & ~0x30);
			wDR(NEW_AUDIO_CONFIG, 1);
			break;

		case AUDIO_MONO_LEFT:
			wDR(AUDIO_DAC_MODE, (rDR(AUDIO_DAC_MODE) & ~0x30) | 0x10);
			wDR(NEW_AUDIO_CONFIG, 1);
			break;

		case AUDIO_MONO_RIGHT:
			wDR(AUDIO_DAC_MODE, (rDR(AUDIO_DAC_MODE) & ~0x30) | 0x20);
			wDR(NEW_AUDIO_CONFIG, 1);
			break;

		default:
			ret=-EINVAL;
			break;
		}
		break;

	case AUDIO_GET_STATUS:
		if (copy_to_user(parg, &dvb->audiostate, sizeof(struct audioStatus)))
			ret=-EFAULT;
		break;

	case AUDIO_GET_CAPABILITIES:
	{
		int cap=AUDIO_CAP_LPCM|
			AUDIO_CAP_MP1|
			AUDIO_CAP_MP2|
			AUDIO_CAP_AC3;

		if (copy_to_user(parg, &cap, sizeof(cap)))
			ret=-EFAULT;
		break;
	}

	case AUDIO_CLEAR_BUFFER:
		break;
	
	case AUDIO_SET_ID:
		break;

	case AUDIO_SET_MIXER:
		memcpy(&dvb->audiomixer, parg, sizeof(struct audioMixer));

		if (dvb->audiomixer.volume_left > AUDIO_MIXER_MAX_VOLUME)
			dvb->audiomixer.volume_left = AUDIO_MIXER_MAX_VOLUME;
		
		if (dvb->audiomixer.volume_right > AUDIO_MIXER_MAX_VOLUME)
			dvb->audiomixer.volume_right = AUDIO_MIXER_MAX_VOLUME;

		avia_gt_pcm_set_mpeg_attenuation((dvb->audiomixer.volume_left + 1) >> 1,
				(dvb->audiomixer.volume_right + 1) >> 1);
		break;

	case AUDIO_SET_STREAMTYPE:
		if ( (streamType_t) arg > STREAM_TYPE_DPES_PES ) {
			ret = -EINVAL;
			break;
		}

		if (dvb->audio_stream_type == (streamType_t)arg)
			break;

		if (rDR(PROC_STATE) != PROC_STATE_IDLE)
			avia_command(Abort, 1);

		if ( (streamType_t) arg == STREAM_TYPE_SPTS_ES) {
			if (!aviarev) {
				avia_command(SetStreamType, 0x10,dvb->audio_pid);
				avia_command(SetStreamType, 0x11,dvb->video_pid);
			}
			else { // AVIA 500 doesn't support SetStreamType with types 0x10, 0x11
				avia_command(Reset);
			}
			dvb->audio_stream_type = STREAM_TYPE_SPTS_ES;
			dvb->video_stream_type = STREAM_TYPE_SPTS_ES;
			break;
		}

		if ( (streamType_t) arg == STREAM_TYPE_DPES_PES) {
			new_mode = 0x0A;
		}
		else {
			new_mode = 0x08;
		}

		if ( dvb->video_stream_type == STREAM_TYPE_SPTS_ES )
		{
			dvb->video_stream_type = (streamType_t) arg;
		}

		if (dvb->video_stream_type == STREAM_TYPE_DPES_PES) {
			new_mode |= 0x01;
		}

		avia_command(SetStreamType, new_mode, 0x0000);

		dvb->audio_stream_type = (streamType_t) arg;

		break;

	case AUDIO_SET_SPDIF_COPIES:
		switch ((audioSpdifCopyState_t) arg) {
		case SCMS_COPIES_NONE:
			wDR(IEC_958_CHANNEL_STATUS_BITS, (rDR(IEC_958_CHANNEL_STATUS_BITS) & ~1) | 4);
			break;

		case SCMS_COPIES_ONE:
			wDR(IEC_958_CHANNEL_STATUS_BITS, rDR(IEC_958_CHANNEL_STATUS_BITS) & ~5);
			break;

		case SCMS_COPIES_UNLIMITED:
			wDR(IEC_958_CHANNEL_STATUS_BITS, rDR(IEC_958_CHANNEL_STATUS_BITS) | 5);
			break;

		default:
			return -EINVAL;
		}

		wDR(NEW_AUDIO_CONFIG, 1);
		break;

	default:
		ret=-ENOIOCTLCMD;
		break;
	}
	return ret;
}

static int
frontend_ioctl(struct dvb_device *dvbdev, struct file *file,
               unsigned int cmd, unsigned long arg)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	void *parg=(void *)arg;

	switch (cmd) {
	case FE_SELFTEST:
		break;

	case FE_SET_POWER_STATE:
		switch (arg) {
		case FE_POWER_ON:
			dvb->powerstate=arg;
			break;
		case FE_POWER_SUSPEND:
		case FE_POWER_STANDBY:
		case FE_POWER_OFF:
			dvb->powerstate=FE_POWER_OFF;
			break;
		default:
			return -EINVAL;
		}
		dvb_frontend_demod_command(dvb->frontend, FE_SET_POWER_STATE, &arg);
		break;

	case FE_GET_POWER_STATE:
		if(copy_to_user(parg, &dvb->powerstate, sizeof(u32)))
			return -EFAULT;
		break;

	case FE_READ_STATUS:
	{
		FrontendStatus stat=0;

		dvb_frontend_demod_command(dvb->frontend, FE_READ_STATUS, &stat);
		if (dvb->sec.power)
			stat|=FE_HAS_POWER;
		if(copy_to_user(parg, &stat, sizeof(stat)))
			return -EFAULT;
		break;
	}

	case FE_READ_BER:
	{
		uint32_t ber;
		
		if (!dvb->sec.power)
			return -ENOSIGNAL;
		dvb_frontend_demod_command(dvb->frontend, FE_READ_BER, &ber);
		if(copy_to_user(parg, &ber, sizeof(ber)))
			return -EFAULT;
		break;
	}

	case FE_READ_SIGNAL_STRENGTH:
	{
		int32_t signal;
		
		if (!dvb->sec.power)
			return -ENOSIGNAL;
		dvb_frontend_demod_command(dvb->frontend, FE_READ_SIGNAL_STRENGTH, &signal);
		if(copy_to_user(parg, &signal, sizeof(signal)))
			return -EFAULT;
		break;
	}

	case FE_READ_SNR:
	{
		int32_t snr;
		
		if (!dvb->sec.power)
			return -ENOSIGNAL;
		dvb_frontend_demod_command(dvb->frontend, FE_READ_SNR, &snr);
		if(copy_to_user(parg, &snr, sizeof(snr)))
			return -EFAULT;
		break;
	}

	case FE_READ_UNCORRECTED_BLOCKS:
	{
		u32 ublocks;

		if (!dvb->sec.power)
			return -ENOSIGNAL;
		if (dvb_frontend_demod_command(dvb->frontend, FE_READ_UNCORRECTED_BLOCKS,
					&ublocks) < 0)
			return -ENOSYS;
		if(copy_to_user(parg, &ublocks, sizeof(ublocks)))
			return -EFAULT;
		break;
	}

	case FE_GET_NEXT_FREQUENCY:
	{
		uint32_t freq;

		if (copy_from_user(&freq, parg, sizeof(freq)))
			return -EFAULT;
		
		if (dvb->frontend->type==DVB_S)
			// FIXME: how does one calculate this?
			freq+=1000; //FIXME: KHz like in QPSK_TUNE??
		else
			freq+=1000000;

		if (copy_to_user(parg, &freq, sizeof(freq)))
			return -EFAULT;
		break;
	}

	case FE_GET_NEXT_SYMBOL_RATE:
	{
		uint32_t rate;

		if(copy_from_user(&rate, parg, sizeof(rate)))
			return -EFAULT;

		if (dvb->frontend->type==DVB_C) {
			if (rate < 1725000)
				rate = 1725000;
			else if (rate < 3450000)
				rate = 3450000;
			else if (rate < 5175000)
				rate = 5175000;
			else if (rate < 5500000)
				rate = 5500000;
			else if (rate < 6875000)
				rate = 6875000;
			else if (rate < 6900000)
				rate = 6900000;
			else
				return -EINVAL;
		}
		// FIXME: how does one really calculate this?
		else if (rate<5000000)
			rate+=500000;
		else if(rate<10000000)
			rate+=1000000;
		else if(rate<30000000)
			rate+=2000000;
		else
			return -EINVAL;

		if (copy_to_user(parg, &rate, sizeof(rate)))
			return -EFAULT;
		break;
	}

	case FE_GET_FRONTEND:
		if(copy_to_user(parg, &dvb->frontend->param,
				sizeof(FrontendParameters)))
			return -EFAULT;
		break;

	case FE_SET_FRONTEND:
	{
		FrontendParameters para;

		if ((file->f_flags&O_ACCMODE)==O_RDONLY)
			return -EPERM;
		if(copy_from_user(&para, parg, sizeof(para)))
			return -EFAULT;

		return dvb_frontend_tune(dvb->frontend, &para);
	}

	case FE_GET_EVENT:
	{
		FrontendEvent event;
		int ret;
		
		ret=dvb_frontend_get_event(dvb->frontend, &event,
					   file->f_flags&O_NONBLOCK);
		if (ret<0)
			return ret;
		if(copy_to_user(parg, &event, sizeof(event)))
			return -EFAULT;
		break;
	}

	case FE_GET_INFO:
		return dvb_frontend_demod_command(dvb->frontend, FE_GET_INFO, parg);

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static int
sec_ioctl(struct dvb_device *dvbdev, struct file*file,
          unsigned int cmd, unsigned long arg)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	void *parg=(void *)arg;

	if (file->f_flags&O_NONBLOCK)
		return -EWOULDBLOCK;

	switch (cmd) {
	case SEC_GET_STATUS:
	{
		struct secStatus status;

		status.busMode=SEC_BUS_IDLE;

		if (dvb->secbusy)
			status.busMode=SEC_BUS_BUSY;
		if (!dvb->sec.power)
			status.busMode=SEC_BUS_OFF;

		status.selVolt=(dvb->sec.volt ?
				SEC_VOLTAGE_18 :
				SEC_VOLTAGE_13);

		status.contTone=(dvb->sec.ttk ?
				SEC_TONE_ON :
				SEC_TONE_OFF);
		if (copy_to_user(parg, &status, sizeof(status)))
			return -EFAULT;
		break;
	}

	case SEC_RESET_OVERLOAD:
		if ((file->f_flags&O_ACCMODE)==O_RDONLY)
			return -EPERM;
		break;

	case SEC_SEND_SEQUENCE:
	{
		struct secCmdSequence seq;

		if(copy_from_user(&seq, parg, sizeof(seq)))
			return -EFAULT;

		if ((file->f_flags&O_ACCMODE)==O_RDONLY)
			return -EPERM;
		
		dvb_frontend_stop(dvb->frontend);
		return secSendSequence(dvb, &seq);
	}

	case SEC_SET_TONE:
	{
		secToneMode mode = (secToneMode) arg;

		if ((file->f_flags&O_ACCMODE)==O_RDONLY)
			return -EPERM;
		
		dvb_frontend_stop(dvb->frontend);
		return secSetTone(dvb, mode);
	}

	case SEC_SET_VOLTAGE:
	{
		secVoltage val = (secVoltage) arg;

		if ((file->f_flags&O_ACCMODE)==O_RDONLY)
			return -EPERM;

		dvb_frontend_stop(dvb->frontend);
		return secSetVoltage(dvb, val);
	}

	default:
		return -ENOIOCTLCMD;
	}
	return 0;
}

static int
dvbdev_ioctl(struct dvb_device *dvbdev, int num,
             struct file *file,	unsigned int cmd, unsigned long arg)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	//void *parg=(void *)arg;
	int type=num2type(dvb, num);

	switch (type) {
	case DVB_DEVICE_VIDEO_0:
		return video_ioctl(dvbdev, file, cmd, arg);

	case DVB_DEVICE_AUDIO_0:
		return audio_ioctl(dvbdev, file, cmd, arg);

	case DVB_DEVICE_FRONTEND_0:
		return frontend_ioctl(dvbdev, file, cmd, arg);

	case DVB_DEVICE_SEC_0:
		return sec_ioctl(dvbdev, file, cmd, arg);

	case DVB_DEVICE_DEMUX_0:
		return demux_ioctl(dvbdev, file, cmd, arg);

	case DVB_DEVICE_DVR_0:
		return DmxDevDVRIoctl(&dvb->dmxdev, file, cmd, arg);

	case DVB_DEVICE_CA_0:
		return ca_ioctl(dvbdev, file, cmd, arg);

	case DVB_DEVICE_NET_0:
		return net_ioctl(dvbdev, file, cmd, arg);

	default:
		return -EOPNOTSUPP;
	}
	return 0;
}


static unsigned int
dvbdev_poll (struct dvb_device *dvbdev, int num,
             struct file *file, poll_table * wait)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;
	int type=num2type(dvb, num);

	switch (type) {
	case DVB_DEVICE_FRONTEND_0:
		return dvb_frontend_poll(dvb->frontend, file, wait);
		
	case DVB_DEVICE_DEMUX_0:
		return DmxDevPoll(&dvb->dmxdev, file, wait);
		
	case DVB_DEVICE_DVR_0:
		return DmxDevDVRPoll(&dvb->dmxdev, file, wait);
	
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}


static int
dvbdev_device_type(struct dvb_device *dvbdev, unsigned int num)
{
	struct dvb_struct *dvb=(struct dvb_struct *) dvbdev->priv;

	return num2type(dvb, num);
}

/******************************************************************************
 * driver registration
 ******************************************************************************/

static int
dvb_register(struct dvb_struct *dvb)
{
	int i;
	struct dvb_device *dvbd=&dvb->dvb_dev;

	if (dvb->registered)
		return -1;
	dvb->registered=1;

	dvb->secbusy=0;

	dvb->audiostate.AVSyncState=0;
	dvb->audiostate.muteState=0;
	dvb->audiostate.playState=AUDIO_STOPPED;
	dvb->audiostate.streamSource=AUDIO_SOURCE_DEMUX;
	dvb->audiostate.channelSelect=AUDIO_STEREO;
	dvb->audiostate.bypassMode=1;

	dvb->videostate.videoBlank=0;
	dvb->videostate.playState=VIDEO_STOPPED;
	dvb->videostate.streamSource=VIDEO_SOURCE_DEMUX;
	dvb->videostate.videoFormat=VIDEO_FORMAT_4_3;
	dvb->videostate.displayFormat=VIDEO_CENTER_CUT_OUT;

	// init and register dvb device structure
	dvbd->priv=(void *) dvb;
	dvbd->open=dvbdev_open;
	dvbd->close=dvbdev_close;
	dvbd->read=dvbdev_read;
	dvbd->write=dvbdev_write;
	dvbd->ioctl=dvbdev_ioctl;
	dvbd->poll=dvbdev_poll;
	dvbd->device_type=dvbdev_device_type;

	for (i=0; i<DVB_DEVS_MAX; i++)
		dvb->users[i]=dvb->writers[i]=0;

	dvb->dvb_devs=&dbox2_devs;

	dvb->audio_pid=0x0000;
	dvb->video_pid=0x0000;
	dvb->audio_stream_type=STREAM_TYPE_SPTS_ES;
	dvb->video_stream_type=STREAM_TYPE_SPTS_ES;

	return dvb_register_device(dvbd);
}


static void
dvb_unregister(struct dvb_struct * dvb)
{
	if (!dvb->registered)
		return;

	if (dvb->dvb_net)
		dvb->dvb_net->dvb_net_release(dvb->dvb_net);

	DmxDevRelease(&dvb->dmxdev);
	dvb_unregister_device(&dvb->dvb_dev);
}


int
register_frontend (dvb_front_t * frontend)
{
	if (!dvbs[0].frontend) {
		dvbs[0].frontend = frontend;
		return InitFront(&dvbs[0]);
	}
	return -EEXIST;
}


int
unregister_frontend (dvb_front_t * frontend)
{
	if (dvbs[0].frontend == frontend) {
		dvb_frontend_exit(frontend);
		dvbs[0].frontend = 0;
		return 0;
	}
	return -ENXIO;
}


static int
register_dvbnet (dvb_net_t * dvbnet)
{
	if (!dvbs[0].dmxdev.demux)
		return -ENXIO;
	if (!dvbs[0].dvb_net) {
		dvbs[0].dvb_net = dvbnet;
		dvbs[0].dvb_net->card_num = dvbs[0].num;
		return dvbs[0].dvb_net->dvb_net_init(dvbs[0].dvb_net, dvbs[0].dmxdev.demux);
	}
	return -EEXIST;
}


static int
unregister_dvbnet (dvb_net_t * dvbnet)
{
	if (dvbs[0].dvb_net == dvbnet) {
		dvbs[0].dvb_net->dvb_net_release(dvbs[0].dvb_net);
		dvbs[0].dvb_net = 0;
		return 0;
	}
	return -ENXIO;
}


static int
register_demux (dmx_demux_t *demux)
{
	if (!dvbs[0].dmxdev.demux) {
		dvbs[0].dmxdev.filternum = AVIA_GT_DMX_FILTER_NUM;
		dvbs[0].dmxdev.demux = demux;
		dvbs[0].dmxdev.capabilities = 0;
		return DmxDevInit(&dvbs[0].dmxdev);
	}
	return -EEXIST;
}


static int
unregister_demux (dmx_demux_t * demux)
{
	if (dvbs[0].dmxdev.demux == demux) {
		DmxDevRelease(&dvbs[0].dmxdev);
		dvbs[0].dmxdev.demux = 0;
		if (dvbs[0].dvb_net)
			unregister_dvbnet(dvbs[0].dvb_net);
		return 0;
	}
	return -ENXIO;
}



static int __init
init_dvb(void)
{
	struct dvb_struct *dvb = &dvbs[0];

	printk("%s: %s\n", __FILE__, __FUNCTION__);
	memset(dvb, 0, sizeof(struct dvb_struct));
	dvb->num=0;

	return dvb_register(dvb);
}

static void __exit
exit_dvb(void)
{
	printk("%s: %s\n", __FILE__, __FUNCTION__);
	dvb_frontend_exit(dvbs[0].frontend);
	dvb_unregister(&dvbs[0]);
}


#ifdef MODULE
module_init(init_dvb);
module_exit(exit_dvb);
EXPORT_SYMBOL(register_frontend);
EXPORT_SYMBOL(unregister_frontend);
EXPORT_SYMBOL(register_demux);
EXPORT_SYMBOL(unregister_demux);
EXPORT_SYMBOL(register_dvbnet);
EXPORT_SYMBOL(unregister_dvbnet);
MODULE_DESCRIPTION("AViA 50x/60x/eNX/GTX DVB API driver");
MODULE_AUTHOR("tmbinc, gillem, obi");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif /* MODULE_LICENSE */
#endif /* MODULE */

