/*
 * dvb_frontend.c: DVB frontend driver module
 *
 * Copyright (C) 1999-2001 Ralph  Metzler 
 *                       & Marcus Metzler for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/smp_lock.h>
#include <linux/string.h>
#include <linux/module.h>
#include <dbox/dvb_frontend.h>

#ifdef MODULE
MODULE_DESCRIPTION("");
MODULE_AUTHOR("Ralph Metzler");
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#endif

#define dprintk	if(0) printk

static inline void ddelay(int i) 
{
        current->state=TASK_INTERRUPTIBLE;
        schedule_timeout((HZ*i)/100);
}

#if 0
static void 
set_tuner_freq(dvb_front_t *fe)
{
	u8 config;
        u32 div2;
	u16 div;
	struct tunertype *tun;
	struct tuner *t = c->data;
        unsigned char buffer[4];
	int rc;

	tun=&tuners[t->type];

        freq*=tun->step; // turn into Hz

	if (freq < tun->thresh1) 
		config = tun->VHF_L;
	else if (freq < tun->thresh2) 
		config = tun->VHF_H;
	else
		config = tun->UHF;

#if 1   // Fix colorstandard mode change
	if (t->type == TUNER_PHILIPS_SECAM && t->mode)
		config |= tun->mode;
	else
		config &= ~tun->mode;
#else
		config &= ~tun->mode;
#endif

	div2=freq + tun->IFPCoff;
        div2/=tun->res;
        div=(u16)div2;

    /*
     * Philips FI1216MK2 remark from specification :
     * for channel selection involving band switching, and to ensure
     * smooth tuning to the desired channel without causing
     * unnecessary charge pump action, it is recommended to consider
     * the difference between wanted channel frequency and the
     * current channel frequency.  Unnecessary charge pump action
     * will result in very low tuning voltage which may drive the
     * oscillator to extreme conditions.
     */
    /*
     * Progfou: specification says to send config data before
     * frequency in case (wanted frequency < current frequency).
     */

	if (t->type == TUNER_PHILIPS_SECAM && freq < t->freq) {
		buffer[0] = tun->config;
		buffer[1] = config;
		buffer[2] = (div>>8) & 0x7f;
		buffer[3] = div      & 0xff;
	} else {
		buffer[0] = (div>>8) & 0x7f;
		buffer[1] = div      & 0xff;
		buffer[2] = tun->config;
		buffer[3] = config;
	}
	if (4 != (rc = i2c_master_send(c,buffer,4)))
                printk("tuner: i2c i/o error: rc == %d (should be 4)\n",rc);

}
#endif

#if 0
static int 
dvb_frontend_tuner_scan(dvb_front_t *fe)
{
	int i;
        struct i2c_msg msgs;
			
	msgs.flags=0;
	msgs.len=0;
	msgs.buf=0;
	for (i=0x60; i<0x67; i++) {
		msgs.addr=i;
		if (i2c_transfer(fe->i2cbus, &msgs, 1)==1)
			printk("tuner found @ 0x%02x\n", i);
	}
	return 0;
}
#endif


static void 
fe_add_event(DVBFEEvents *events, FrontendEvent *ev)
{
        int wp;
        struct timeval tv;

        do_gettimeofday(&tv);
        ev->timestamp=tv.tv_sec;
        
        spin_lock(&events->eventlock);
        wp=events->eventw;
        wp=(wp+1)%MAX_EVENT;
        if (wp==events->eventr) {
                events->overflow=1;
                events->eventr=(events->eventr+1)%MAX_EVENT;
        }
        memcpy(&events->events[events->eventw], ev, sizeof(FrontendEvent));
        events->eventw=wp;
        spin_unlock(&events->eventlock);
        wake_up(&events->eventq);
}

int 
dvb_frontend_demod_command(dvb_front_t *fe, unsigned int cmd, void *arg)
{
        if (!fe	|| !fe->demod || 
	    !fe->demod->driver || 
	    !fe->demod->driver->command)
                return -1;
        return fe->demod->driver->command(fe->demod, cmd, arg);
}

#if 0
static inline u8
tuner_stat(struct dvb_struct *dvb, u8 id)
{
        struct i2c_adapter *adap=dvb->i2cbus;
        u8 mm1[] = {0x00};
        struct i2c_msg msgs[1];

        msgs[0].flags=I2C_M_RD;
        msgs[0].addr=id/2;
        msgs[0].len=1;
        msgs[0].buf=mm1;
        if (i2c_transfer(adap, msgs, 1)<0)
          printk("dvb: tuner_stat error\n");

        return mm1[0];
}
#endif

#if 0
static int 
tuner_command(dvb_front_t *fe, unsigned int cmd, void *arg)
{
        if (!fe->tuner || 
	    !fe->tuner->driver || 
	    !fe->tuner->driver->command)
                return -1;

        if (cmd == TUNER_SET_TVFREQ &&
            fe->demod && 
            fe->demod_type==DVB_DEMOD_STV0299) {
                u8 msg[2]={ 0x05, 0xb5 }; 
                
                dvb_frontend_demod_command(fe, FE_WRITEREG, msg); 
                fe->tuner->driver->command(fe->tuner, cmd, arg);
                return 0;
        }

        return fe->tuner->driver->command(fe->tuner, cmd, arg);
}
#endif

static int
fe_lock(dvb_front_t *fe)
{
        int lock;
        FrontendStatus status;

        if (dvb_frontend_demod_command(fe, FE_READ_STATUS, &status))
		return 0;
	lock=(status&FE_HAS_LOCK) ? 1 : 0;
	fe->lock=lock;
	return lock;
}

static unsigned long 
fe_afc(dvb_front_t *fe)
{
        s32 afc;

        dvb_frontend_demod_command(fe, FE_READ_STATUS, &afc);

        if (!fe_lock(fe)) {
                fe->tuning=FE_STATE_TUNE;   // re-tune
                fe->delay=HZ/2;
                return -1;
        }
        fe->delay=HZ*60;
        if (fe->type==DVB_C)
                return 0;
#if 0
        /* this does not work well for me, after the first adjustment I
           lose sync in most cases */

        if (!afc) {
                fe->delay=HZ*60;
                return 0;  
        }
        fe->delay=HZ*10;
        
        fe->curfreq -= (afc/2); 
        dvb_frontend_demod_command(fe, FE_SETFREQ, &fe->curfreq); 

        /* urghh, this prevents loss of lock but is visible as artifacts */
        dvb_frontend_demod_command(fe, FE_RESET, 0); 
#endif
	return 0;
}

static int
fe_complete(dvb_front_t *fe)
{
	FrontendEvent ev;

        ev.type=FE_COMPLETION_EV;
	memcpy (&ev.u.completionEvent, &fe->param,
		sizeof(FrontendParameters));
	
	ev.u.completionEvent.Frequency=fe->curfreq;

#if 0	
        if (fe->type==DVB_S)
                ev.u.completionEvent.Frequency=fe->param.Frequency/1000;
#endif

	fe_add_event(&fe->events, &ev);

        fe->tuning=FE_STATE_AFC;
        fe->delay=HZ*10;
	if (fe->complete_cb)
		fe->complete_cb(fe->priv);
        return 0;
}

static int
fe_fail(dvb_front_t *fe)
{
	FrontendEvent ev;

	fe->tuning=FE_STATE_IDLE;
	ev.type=FE_FAILURE_EV;
	ev.u.failureEvent = 0;
	dvb_frontend_demod_command(fe, FE_READ_STATUS, &ev.u.failureEvent);
	fe_add_event(&fe->events, &ev);

	return -1;
}

static int
fe_zigzag(dvb_front_t *fe)
{
        int i=fe->zz_count;
        u32 sfreq=fe->param.Frequency;
        u32 soff;

        if (fe_lock(fe)) 
                return fe_complete(fe);
        if (i==20) {
                /* return to requested frequency, maybe it locks when the user
                 * retries the tuning operation
                 */
                dvb_frontend_demod_command(fe, FE_SETFREQ, &sfreq);
                dvb_frontend_demod_command(fe, FE_RESET, 0);
                return fe_fail(fe);
        }

	/* this is skipped if inversion is AUTO and was not set to OFF below */
        dprintk("fe_zigzag: i=%.2d spc_inv: %d\n", i, fe->spc_inv);
        if (fe->spc_inv == INVERSION_OFF) {
                fe->spc_inv = INVERSION_ON;
                dvb_frontend_demod_command(fe, FE_SET_INVERSION,
                                           (void*) fe->spc_inv);
                return i;
        }
        else if (fe->spc_inv == INVERSION_ON) {
                fe->spc_inv = INVERSION_OFF;
                dvb_frontend_demod_command(fe, FE_SET_INVERSION,
                                           (void*) fe->spc_inv);
        }

        if (fe->type == DVB_S)
                soff=fe->param.u.qpsk.SymbolRate/16000;
        else if (fe->type == DVB_C)
                soff=fe->param.u.qam.SymbolRate/16000;
        else if (fe->type == DVB_T)
                /* FIXME finish fe_zigzag for ofdm */
                soff=0;
        else 
                soff=0;

        if (i&1) 
                sfreq=fe->param.Frequency+soff*(i/2);
        else
                sfreq=fe->param.Frequency-soff*(i/2);
        dprintk("fe_zigzag: i=%.2d freq=%d, spc_inv: %d\n",
                i, sfreq, fe->spc_inv);

        dvb_frontend_demod_command(fe, FE_SETFREQ, &sfreq); 
        dvb_frontend_demod_command(fe, FE_RESET, 0);
        fe->curfreq=sfreq;
        fe->zz_count++;

	switch (fe->type) {
	case DVB_S:
                fe->delay = HZ/20;
		break;
	case DVB_T:
                fe->delay = HZ/2;
		break;
	default:
	case DVB_C:
                fe->delay = HZ;
		break;
	}
        //if (fe->demod_type==DVB_DEMOD_STV0299)
	//              dvb_frontend_demod_command(fe, FE_RESET, 0); 
        return i;
}

static int 
fe_tune(dvb_front_t *fe)
{
        int kickit = 0, locked = fe_lock(fe), do_set_front = 0;
        FrontendParameters *param, *new_param;
        
        param=&fe->param;
        new_param=&fe->new_param;
        
	if (fe->start_cb)
		fe->start_cb(fe->priv);
        if (!locked
            || param->Frequency != new_param->Frequency) {
                dvb_frontend_demod_command(fe, FE_SETFREQ,
                              &new_param->Frequency); 
                kickit = 1;
        }
        /* if locked == 1, dvb->front.fec is guaranteed to be != FEC_AUTO,
         * so using FEC_AUTO will always reset the demod
         */
        switch (fe->type) {
        case DVB_S:
		do_set_front =
			param->u.qpsk.SymbolRate != new_param->u.qpsk.SymbolRate
			|| param->u.qpsk.FEC_inner != new_param->u.qpsk.FEC_inner
			|| param->Inversion != new_param->Inversion;
                break;
        case DVB_C:
                do_set_front =
			param->u.qam.SymbolRate != new_param->u.qam.SymbolRate
			|| param->u.qam.FEC_inner != new_param->u.qam.FEC_inner
			|| param->Inversion != new_param->Inversion;
                break;
        case DVB_T:
                /* FIXME optimize fe_tune for ofdm */
                do_set_front = 1;
                break;
        default:
                break;
        }

	memcpy(param, new_param, sizeof(FrontendParameters));

        if (!locked || do_set_front) {
                dvb_frontend_demod_command(fe, FE_SET_FRONTEND, param);
                kickit = 1;
        }
        
        if (kickit)
                dvb_frontend_demod_command(fe, FE_RESET, 0);

        if (fe->type == DVB_S) {
                mdelay(10);
                fe->zz_count = 1;
        }
        else if (fe->type == DVB_C || fe->type == DVB_T) {
                /* FIXME optimize fe_tune for ofdm */
                mdelay(30);
                fe->zz_count = 0;
        }
        fe->curfreq=param->Frequency;

        if (fe_lock(fe)) {
                dprintk("mon_tune: locked on 1st try\n");
                return fe_complete(fe);
        }
        dprintk("need zigzag, freq=%d, spc_inv: %d\n",
                new_param->Frequency, new_param->Inversion & 1);

	/* if inversion is AUTO and demod cannot handle it, set spc_inv to OFF */
        fe->spc_inv = new_param->Inversion;
	if (fe->spc_inv == INVERSION_AUTO) 
		if (fe->demod_type==DVB_DEMOD_STV0299 ||
		    fe->demod_type==DVB_DEMOD_VES1820)
			fe->spc_inv = INVERSION_OFF;
			
        fe->tuning=FE_STATE_ZIGZAG;
	switch (fe->type) {
	case DVB_S:
                fe->delay = HZ/10;
		break;
	case DVB_T:
                fe->delay = HZ/2;
		break;
	default:
	case DVB_C:
                fe->delay = HZ;
		break;
	}

        return 0;
}


static int 
fe_thread(void *data)
{
	dvb_front_t *fe = (dvb_front_t *) data;
    
	lock_kernel();
	daemonize();
	sigfillset(&current->blocked);
	strcpy(current->comm,"fe_thread");
	fe->thread = current;
	unlock_kernel();

	for (;;) {
                interruptible_sleep_on_timeout(&fe->wait, fe->delay);
		if (fe->exit || signal_pending(current))
			break;

                if (down_interruptible(&fe->sem))
			break;

		switch (fe->tuning) {
                case FE_STATE_TUNE:
                        fe_tune(fe);
                        break;
                case FE_STATE_ZIGZAG:
                        fe_zigzag(fe);
                        break;
                case FE_STATE_AFC:
                        fe_afc(fe);
                        break;
                default:
                        fe->delay=HZ;
                        break;
                }
                up(&fe->sem);
	}
	fe->thread = NULL;
	return 0;
}

void
dvb_frontend_stop(dvb_front_t *fe)
{
        if (fe->tuning==FE_STATE_IDLE)
                return;

	if (down_interruptible(&fe->sem))
		return;

        fe->tuning=FE_STATE_IDLE;
	wake_up_interruptible(&fe->wait);
        up(&fe->sem);
}

int
dvb_frontend_tune(dvb_front_t *fe, FrontendParameters *new_param)
{
        if (fe->type==DVB_S && new_param->u.qpsk.FEC_inner>FEC_NONE)
                return -EINVAL;
        if (fe->type==DVB_C && new_param->u.qam.FEC_inner>FEC_NONE)
                return -EINVAL;
        if (fe->type==DVB_T && 
	    (new_param->u.ofdm.Constellation!=QPSK &&
	     new_param->u.ofdm.Constellation!=QAM_16 && 
	     new_param->u.ofdm.Constellation!=QAM_64))
                return -EINVAL;

	memcpy(&fe->new_param, new_param, sizeof(FrontendParameters));

	if (down_interruptible(&fe->sem))
		return -ERESTARTSYS;

	fe->tuning=FE_STATE_TUNE;
	wake_up_interruptible(&fe->wait);
        up(&fe->sem);
	return 0;
}

int
dvb_frontend_get_event(dvb_front_t *fe, FrontendEvent *event, int nonblocking)
{
	int ret;
	DVBFEEvents *events=&fe->events;
	
	if (events->overflow) {
		events->overflow=0;
		return -EBUFFEROVERFLOW;
	}
	if (events->eventw==events->eventr) {
		if (nonblocking) 
			return -EWOULDBLOCK;
		
		ret=wait_event_interruptible(events->eventq,
					     events->eventw!=
					     events->eventr);
		if (ret<0)
			return ret;
	}
	
	spin_lock(&events->eventlock);
	memcpy(event, 
	       &events->events[events->eventr],
	       sizeof(FrontendEvent));
	events->eventr=(events->eventr+1)%MAX_EVENT;
	spin_unlock(&events->eventlock);
	return 0;
}

int dvb_frontend_poll(dvb_front_t *fe, struct file *file, poll_table * wait)
{
	if (fe->events.eventw!=fe->events.eventr)
		return (POLLIN | POLLRDNORM | POLLPRI);
	
	poll_wait(file, &fe->events.eventq, wait);
	
	if (fe->events.eventw!=fe->events.eventr)
		return (POLLIN | POLLRDNORM | POLLPRI);
	return 0;
}

int 
dvb_frontend_init(dvb_front_t *fe)
{
	init_waitqueue_head(&fe->wait);
        sema_init(&fe->sem, 1);
	fe->tuning=FE_STATE_IDLE;
	fe->exit=0;
	fe->delay=HZ;
	
        init_waitqueue_head(&fe->events.eventq);
        spin_lock_init (&fe->events.eventlock);
        fe->events.eventw=fe->events.eventr=0;
        fe->events.overflow=0;
	
        dvb_frontend_demod_command(fe, FE_INIT, 0); 
        //if (fe->tuner_type>=0)
        //        tuner_command(fe, TUNER_SET_TYPE, &fe->tuner_type); 

	//dvb_frontend_tuner_scan(fe);
	kernel_thread(fe_thread, fe, 0);
	return 0;
}

void
dvb_frontend_exit(dvb_front_t *fe)
{
	if (!fe->thread)
		return;
	fe->exit=1;
	wake_up_interruptible(&fe->wait);
	while (fe->thread)
		ddelay(1);
}

#ifdef MODULE
#ifdef EXPORT_SYMTAB
EXPORT_SYMBOL(dvb_frontend_init);
EXPORT_SYMBOL(dvb_frontend_exit);
EXPORT_SYMBOL(dvb_frontend_tune);
EXPORT_SYMBOL(dvb_frontend_stop);
EXPORT_SYMBOL(dvb_frontend_get_event);
EXPORT_SYMBOL(dvb_frontend_poll);
EXPORT_SYMBOL(dvb_frontend_demod_command);
#endif
#endif
