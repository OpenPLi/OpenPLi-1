/* 
 * test_front.c - Test program for new API
 *
 * Copyright (C) 2000 Ralph  Metzler <ralph@convergence.de>
 *                  & Marcus Metzler <marcus@convergence.de>
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

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <ost/dmx.h>
#include <ost/frontend.h>
#include <ost/sec.h>
#include <ost/audio.h>
#include <sys/poll.h>



int SecGetStatus (int fd, struct secStatus *state)
{
	int ans;

	if ( (ans = ioctl(fd,SEC_GET_STATUS, state) < 0)){
		perror("QPSK GET EVENT: ");
	//	return ans;
	}

	switch (state->busMode){
	case SEC_BUS_IDLE:
		printf("SEC BUS MODE:  IDLE (%d)\n",state->busMode);
		break;
	case SEC_BUS_BUSY:
		printf("SEC BUS MODE:  BUSY (%d)\n",state->busMode);
		break;
	case SEC_BUS_OFF:
		printf("SEC BUS MODE:  OFF  (%d)\n",state->busMode);
		break;
	case SEC_BUS_OVERLOAD:
		printf("SEC BUS MODE:  OVERLOAD (%d)\n",state->busMode);
		break;
	default:
		printf("SEC BUS MODE:  unknown  (%d)\n",state->busMode);
		break;
	}

	
	switch (state->selVolt){
	case SEC_VOLTAGE_OFF:
		printf("SEC VOLTAGE:  OFF (%d)\n",state->selVolt);
		break;
	case SEC_VOLTAGE_LT:
		printf("SEC VOLTAGE:  LT  (%d)\n",state->selVolt);
		break;
	case SEC_VOLTAGE_13:
		printf("SEC VOLTAGE:  13  (%d)\n",state->selVolt);
		break;
	case SEC_VOLTAGE_13_5:
		printf("SEC VOLTAGE:  13.5 (%d)\n",state->selVolt);
		break;
	case SEC_VOLTAGE_18:
		printf("SEC VOLTAGE:  18 (%d)\n",state->selVolt);
		break;
	case SEC_VOLTAGE_18_5:
		printf("SEC VOLTAGE:  18.5 (%d)\n",state->selVolt);
		break;
	default:
		printf("SEC VOLTAGE:  unknown (%d)\n",state->selVolt);
		break;
	}

	printf("SEC CONT TONE: %s (%d)\n", (state->contTone ? "OFF" : "ON"),state->contTone);
	return 0;
}

int SecResetOverload(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,SEC_RESET_OVERLOAD) < 0)){
		perror("SEC OVERLOAD EVENT: ");
		return ans;
	}
	return 0;
}

int SecSendSequence(int fd, struct secCmdSequence *seq)
{
	int ans;

	if ( (ans = ioctl(fd,SEC_SEND_SEQUENCE, seq) < 0)){
		perror("SEC SEQUENZ EVENT: ");
		return ans;
	}
	return 0;
}
int SecSetTone(int fd, secToneMode tone)
{
	int ans;

	if ( (ans = ioctl(fd,SEC_SET_TONE, tone) < 0)){
		perror("SEC TONE EVENT: ");
		return ans;
	}
	return 0;
}
int SecSetVoltage(int fd, secVoltage voltage)
{
	int ans;

	if ( (ans = ioctl(fd,SEC_SET_VOLTAGE, voltage) < 0)){
		perror("SEC VOLTAGE EVENT: ");
		return ans;
	}
	return 0;
}

main(int argc, char **argv)
{
	int fd_sec;
	struct secStatus sec_state;
	struct secCmdSequence seq;
	struct secCommand cmd;
	struct secDiseqcCmd diseqc;

	if((fd_sec = open("/dev/ost/sec0",O_RDWR)) < 0){
		perror("SEC DEVICE: ");
		return -1;
	}
//	SecResetOverload(fd_sec);
//	SecGetStatus (fd_sec, &sec_state);
//	SecSetVoltage(fd_sec, SEC_VOLTAGE_13_5);
//        SecSetTone(fd_sec, SEC_TONE_ON);       
        SecGetStatus (fd_sec, &sec_state);
/*
	diseqc.addr=0x10;
	diseqc.cmd=0x38;
	diseqc.numParams=1;
	diseqc.params[0]=0xF3;
*/
//F3 / 51 == H 

	cmd.type=SEC_CMDTYPE_DISEQC;
	cmd.u.diseqc=diseqc;
        
	seq.voltage=SEC_VOLTAGE_18;

	seq.miniCommand=SEC_MINI_NONE;
	seq.continuousTone=SEC_TONE_ON;
	seq.numCommands=1;
	seq.commands=&cmd;	

//	SecSendSequence(fd_sec, &seq);
	close(fd_sec);
}

