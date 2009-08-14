/* 
 * dvb_frontend.h
 *
 * Copyright (C) 2001 Ralph  Metzler <ralph@convergence.de>
 *                    for convergence integrated media GmbH
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

#ifndef _DVB_FRONTEND_H_
#define _DVB_FRONTEND_H_

#ifdef __DVB_PACK__
#include <ost/frontend.h>
#else
#include <linux/ost/frontend.h>
#endif

#include <linux/sched.h>
#include <linux/i2c.h>
#include <linux/ioctl.h>
#include <linux/videodev.h>

#ifndef I2C_DRIVERID_STV0299
#define I2C_DRIVERID_STV0299 I2C_DRIVERID_EXP0
#endif

#ifndef I2C_DRIVERID_TDA8083 
#define I2C_DRIVERID_TDA8083 I2C_DRIVERID_EXP1
#endif

#ifndef I2C_DRIVERID_L64781 
#define I2C_DRIVERID_L64781 I2C_DRIVERID_EXP2
#endif

#ifndef I2C_DRIVERID_SP8870 
#define I2C_DRIVERID_SP8870 I2C_DRIVERID_EXP3
#endif

#ifndef I2C_DRIVERID_CX24110
#define I2C_DRIVERID_CX24110 0xf4
#endif

#ifndef I2C_DRIVERID_VES1993
#define I2C_DRIVERID_VES1993 0xf5
#endif

#ifndef I2C_DRIVERID_TDA8044
#define I2C_DRIVERID_TDA8044 0xf6
#endif

#ifndef I2C_DRIVERID_AT76C651
#define I2C_DRIVERID_AT76C651 0xf7
#endif

#define FE_STATE_IDLE   0
#define FE_STATE_TUNE   1
#define FE_STATE_ZIGZAG 2
#define FE_STATE_AFC    3


#define MAX_EVENT 8

typedef struct {
        FrontendEvent     events[MAX_EVENT];
        int               eventw;
        int               eventr;
        int               overflow;
        wait_queue_head_t eventq;
        spinlock_t        eventlock;
} DVBFEEvents;

typedef struct dvb_frontend {
	int                    type;
#define DVB_NONE 0
#define DVB_S    1
#define DVB_C    2
#define DVB_T    3

	int                    capabilities;
	
	void                   *priv;
	void                   (*start_cb)(void *);
	void                   (*complete_cb)(void *);

        struct i2c_adapter     *i2cbus;

	/*
	 * on the dbox2, tuner and sec
	 * are set via demod commands
	 */

        struct i2c_client      *demod;
        int                     demod_type; /* demodulator type */
#define DVB_DEMOD_NONE    0x00
#define DVB_DEMOD_VES1893 0x01
#define DVB_DEMOD_VES1820 0x02
#define DVB_DEMOD_STV0299 0x03
#define DVB_DEMOD_TDA8083 0x04
#define DVB_DEMOD_L64781  0x05
#define DVB_DEMOD_SP8870  0x06
#define DVB_DEMOD_VES1993 0x07
#define DVB_DEMOD_TDA8044 0x08
#define DVB_DEMOD_AT76C651 0x09
#define DVB_DEMOD_CX24110 0x10
	
	struct task_struct     *thread;
        wait_queue_head_t       wait;
        struct semaphore        sem;
        int                     tuning;
        int                     exit;
        int                     zz_count;
        int                     spc_inv;
        unsigned long           delay;
        int                     lock;

	u32 curfreq;
	FrontendParameters      param;
        FrontendParameters      new_param;
        DVBFEEvents             events;

} dvb_front_t;

#define FE_INIT           _IOR('v',  BASE_VIDIOCPRIVATE+0x12, void)
#define FE_RESET          _IOR('v',  BASE_VIDIOCPRIVATE+0x13, void)
#define FE_WRITEREG       _IOR('v',  BASE_VIDIOCPRIVATE+0x14, u8 *)
#define FE_READREG        _IOR('v',  BASE_VIDIOCPRIVATE+0x15, u8 *)
#define FE_READ_AFC       _IOR('v',  BASE_VIDIOCPRIVATE+0x16, s32 *)
#define FE_SET_INVERSION  _IOR('v',  BASE_VIDIOCPRIVATE+0x17, u32 *)

int dvb_frontend_init(dvb_front_t *fe);
void dvb_frontend_exit(dvb_front_t *fe);
int dvb_frontend_tune(dvb_front_t *fe, FrontendParameters *new_param);
void dvb_frontend_stop(dvb_front_t *fe);
int dvb_frontend_get_event(dvb_front_t *fe, FrontendEvent *event, int nonblocking);
int dvb_frontend_poll(dvb_front_t *fe, struct file *file, poll_table * wait);
int dvb_frontend_demod_command(dvb_front_t *fe, unsigned int cmd, void *arg);
extern int register_frontend(dvb_front_t *frontend);
extern int unregister_frontend(dvb_front_t *frontend);

#endif
