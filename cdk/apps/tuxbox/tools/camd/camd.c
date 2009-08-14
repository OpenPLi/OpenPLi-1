/*
 * $Id: camd.c,v 1.9.2.1 2003/03/04 17:12:12 alexw Exp $
 *
 * (C) 2001, 2002 by gillem, Hunz, kwon, tmbinc, TripleDES, obi
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

#include <fcntl.h>
#include <ost/ca.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "camd.h"
#include "cat.h"

#ifdef LOOKUP_ONID
#include "sdt.h"

unsigned short current_onid = 0;
#endif /* LOOKUP_ONID */

#define CAMD_UDS_NAME	"/tmp/camd.socket"
#define MAX_SERVICES	8

int camfd;
pthread_t camlisten;

descrambleservice_s descrambleservice[MAX_SERVICES];

//card-status
unsigned short caid[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
unsigned char caid_count = 0;

unsigned char slot1 = 0;
unsigned char slot2 = 0;

unsigned char card_country[3];
unsigned char card_number[10];
unsigned char card_version[2];
//end cam-status

int _writecam (unsigned char command, unsigned char * data, unsigned short length)
{
	unsigned short i;
	ca_msg_t ca_msg;

	ca_msg.index = 0;
	ca_msg.type = 0;
	ca_msg.length = length + 4;

	/* ca message */
	ca_msg.msg[0] = 0x50;
	ca_msg.msg[1] = (ca_msg.length - 3) | ((command != 0x23) ? 0x80 : 0x00);
	ca_msg.msg[2] = command;
	memcpy(ca_msg.msg + 3, data, length);

	/* message checksum */
	ca_msg.msg[ca_msg.length - 1] = 0x6E;

	for (i = 0; i < ca_msg.length - 1; i++)
	{
		ca_msg.msg[ca_msg.length - 1] ^= ca_msg.msg[i];
	}

#if 0
	printf("[camd] ca_msg (%d):", ca_msg.length);
	for (i = 0; i < ca_msg.length; i++) printf(" %02x", ca_msg.msg[i]);
	printf("\n");
#endif

	if (ioctl(camfd, CA_SEND_MSG, &ca_msg) < 0)
	{
		perror("[camd] CA_SEND_MSG");
		return -1;
	}

	return 0;
}

int writecam (unsigned char * data, unsigned short len)
{
	return _writecam(0x23, data, len);
}

int descramble (unsigned short onID, unsigned short serviceID, unsigned short unknown, unsigned short caID, unsigned short ecmpid, unsigned char numpids, unsigned short * pid)
{
	unsigned char i;
	unsigned char buffer[12 + (numpids << 2)];

	buffer[0] = 0x0F;  //0x0D
	buffer[1] = onID >> 8;
	buffer[2] = onID;
	buffer[3] = serviceID >> 8;
	buffer[4] = serviceID;
	buffer[5] = unknown >> 8;
	buffer[6] = unknown;
	buffer[7] = caID >> 8;
	buffer[8] = caID;
	buffer[9] = ecmpid >> 8;
	buffer[10] = ecmpid;
	buffer[11] = numpids;

	for (i = 0; i < numpids; i++)
	{
		buffer[12 + (i << 2)] = pid[i] >> 8;
		buffer[13 + (i << 2)] = pid[i];
		buffer[14 + (i << 2)] = 0x80;
		buffer[15 + (i << 2)] = 0x00;
	}

	return writecam(buffer, sizeof(buffer));
}

int status (void)
{
	unsigned char buffer[1];

	buffer[0] = 0x03;

	return writecam(buffer, 1);
}

int reset (void)
{
	unsigned char buffer[1];

	unsigned char i;

#ifdef LOOKUP_ONID
	unsigned short onid;
#endif /* LOOKUP_ONID */

	for (i = 0; i < MAX_SERVICES; i++)
	{
		descrambleservice[i].valid = 0;
		descrambleservice[i].onID = 0;
		descrambleservice[i].sID = 0;
		descrambleservice[i].caID = 0;
		descrambleservice[i].ecmPID = 0;
	}

#ifdef LOOKUP_ONID
	onid = parse_sdt();

	if (onid == 0)
	{
		printf("[camd] unable to find original network id\n");
	}
	else
	{
		current_onid = onid;
	}
#endif /* LOOKUP_ONID */

	buffer[0] = 0x09;

	return writecam(buffer, 1);
}

int init (void)
{
	unsigned char buffer[1];

	buffer[0] = 0x39;

	return writecam(buffer, 1);
}

int init2 (void)
{
	unsigned char buffer[1];

	buffer[0] = 0x29;

	return writecam(buffer, 1);
}

int start (unsigned short service_id)
{
	unsigned char buffer[3];

	buffer[0] = 0x3D;
	buffer[1] = service_id >> 8;
	buffer[2] = service_id;

	return writecam(buffer, 3);
}

int setemm (unsigned short unknown, unsigned short ca_system_id, unsigned short pid)
{
	unsigned char buffer[7];

	buffer[0] = 0x84;
	buffer[1] = unknown >> 8;
	buffer[2] = unknown;
	buffer[3] = ca_system_id >> 8;
	buffer[4] = ca_system_id;
	buffer[5] = pid >> 8;
	buffer[6] = pid;

	return writecam(buffer, 7);
}

void startdescramble (void)
{
	unsigned char i;

	for (i = 0; i < MAX_SERVICES; i++)
	{
		if ((descrambleservice[i].valid == 1) && (descrambleservice[i].started == 0))
		{
			printf("[camd] starting onid %04x sid %04x\n", descrambleservice[i].onID, descrambleservice[i].sID);

			descramble
			(
				descrambleservice[i].onID,
				descrambleservice[i].sID,
				0x104,
				descrambleservice[i].caID,
				descrambleservice[i].ecmPID,
				descrambleservice[i].numpids,
				descrambleservice[i].pid
			);

			descrambleservice[i].started = 1;
		}
	}
}

int adddescrambleservice (unsigned char numpids, unsigned short onid, unsigned short sid, unsigned short unkwn, unsigned short caid, unsigned short ecmpid, unsigned short * pid)
{
	unsigned char i;
	unsigned char j;

	for (i = 0; i < MAX_SERVICES; i++)
	{
		if ((descrambleservice[i].caID == caid) && (descrambleservice[i].sID == sid) && (descrambleservice[i].ecmPID == ecmpid))
		{
			printf("[camd] refusing duplicate service\n");
			return -1;
		}

		if (descrambleservice[i].valid == 0)
		{
			break;
		}
	}

	if (i == MAX_SERVICES)
	{
		printf("[camd] no free service, reset needed\n");
		reset();
		i = 0;
	}

	descrambleservice[i].valid = 1;
	descrambleservice[i].started = 0;
	descrambleservice[i].status = 0;
	descrambleservice[i].onID = onid;
	descrambleservice[i].sID = sid;
	descrambleservice[i].Unkwn = unkwn;
	descrambleservice[i].caID = caid;
	descrambleservice[i].ecmPID = ecmpid;
	descrambleservice[i].numpids = numpids;

	for (j = 0; j < numpids; j++)
	{
		descrambleservice[i].pid[j] = pid[j];
	}

	startdescramble();

	return 0;
}

int adddescrambleservicestruct (descrambleservice_s * service)
{
	return adddescrambleservice (service->numpids, service->onID, service->sID, service->Unkwn, service->caID, service->ecmPID, service->pid);
}

void class_23 (unsigned char * buffer, unsigned int len)
{
	int i;

	switch ((buffer[4] & 0x7C) >> 2)
	{
	case 0x00:
		caid_count = buffer[5];

		for (i = 0; i < caid_count; i++)
		{
			caid[i] = (buffer[6 + (i << 1)] << 8) | buffer[7 + (i << 1)];
			printf("[camd] ca system id: %04x\n", caid[i]);
		}
		break;

	case 0x02:
		for (i = 0; i < caid_count; i++)
		{
			unsigned short emmpid = parse_cat(caid[i]);

			if (emmpid != 0x0000)
			{
				setemm(0x104, caid[i], emmpid);
			}
		}
		break;

	case 0x03:
		printf("descramble onid: %02x%02x sid: %02x%02x status: %02x%02x\n", buffer[5], buffer[6], buffer[7], buffer[8], buffer[12], buffer[13]);
		for (i = 0; i < MAX_SERVICES; i++)
		{
			if ((descrambleservice[i].onID == ((buffer[5] << 8) | buffer[6])) && (descrambleservice[i].sID == ((buffer[7] << 8) | buffer[8])))
			{
				descrambleservice[i].status = (buffer[12] << 8) | buffer[13];
			}
		}
		break;

	case 0x07:
		if ((buffer[5] & 0x01) == 0)
		{
			printf("[camd] a card is in slot #2\n");
			slot1 = 1;
		}
		else
		{
			printf("[camd] no card is in slot #2\n");
			slot1 = 0;
		}

		if ((buffer[6] & 0x01) == 0)
		{
			printf("[camd] a card is in slot #1\n");
			slot2 = 1;
		}
		else
		{
			printf("[camd] no card is in slot #1\n");
			slot2 = 0;
		}
		break;

	case 0x08:
		memcpy(card_country, buffer + 5, 3);
		printf("[camd] card_country: %s\n", card_country);
		memcpy(card_number, buffer + 8, 10);
		printf("[camd] card_number: %s\n", card_number);
		memcpy(card_version, buffer + 18, 2);
		printf("[camd] card_version: %02x%02x\n", card_version[0], card_version[1]);
		break;

	default:
		printf("[camd] Unknown 2.CMD-Class: %02x (%02x)\n", buffer[4], (buffer[4] & 0x7C) >> 2);
		break;
	}
}

void class_E0 (unsigned char * buffer, unsigned int len)
{
	printf("[camd] 1.CMD-Class 0xE0\n");
}

void class_E1 (unsigned char * buffer, unsigned int len)
{
	printf("[camd] 1.CMD-Class 0xE1\n");
}

void * camlistenthread (void * thread_arg)
{
	ca_msg_t ca_msg;

	while (1)
	{
		unsigned char buffer[128];
		int len;
		int csum = 0;
		int i;

		memset(buffer, 0, sizeof(buffer));
		memset(&ca_msg, 0, sizeof(ca_msg));
		ca_msg.length = 4;

		if (ioctl(camfd, CA_GET_MSG, &ca_msg) < 0)
		{
			perror("[camd] CA_GET_MSG");
			break;
		}

		len = ca_msg.length;

		if (len <= 0)
		{
			usleep(500);
			continue;
		}
		else
		{
			memcpy(buffer, ca_msg.msg, ca_msg.length);
		}

		if ((buffer[0] != 0x6F) || (buffer[1] != 0x50)) //not good with i2c-addr inside
		{
			printf("[camd] out of sync! %02x %02x %02x %02x\n", buffer[0], buffer[1], buffer[2], buffer[3]);
			break;
		}

		len = buffer[2] & 0x7F;
		ca_msg.length = len;

		if (ioctl(camfd, CA_GET_MSG, &ca_msg) < 0)
		{
			perror("[camd] CA_GET_MSG");
			break;
		}

		if (ca_msg.length != len)
		{
			printf("[camd] incorrect length");
			break;
		}

		memcpy(buffer + 4, ca_msg.msg, ca_msg.length);

		for (i = 0; i < len + 4; i++)
		{
			csum ^= buffer[i];
		}

		if (csum != 0)
		{
			printf("[camd] checksum failed. packet was:");
			for (i = 0; i < len + 4; i++) printf(" %02x", buffer[i]);
			printf("\n");
			continue;
		}

#if 0
		printf("[camd] read buffer:");
		for (i = 0; i < len + 4; i++) printf(" %02x", buffer[i]);
		printf("\n");
#endif

		switch (buffer[3])
		{
			case 0x23:
				class_23(buffer, len + 4);
				break;

			case 0xE0:
				class_E0(buffer, len + 4);
				break;

			case 0xE1:
				class_E1(buffer, len + 4);
				break;

			default:
				printf("[camd] Unknown 1.CMD-Class %02x\n", buffer[3]);
				break;
		}
	}

	return 0;
}

int parse_ca_pmt (const unsigned char * buffer, const unsigned int length)
{
	unsigned short i, j, k;
	ca_pmt * pmt;
	descrambleservice_s service;

	memset(&service, 0, sizeof(descrambleservice_s));

	pmt = (ca_pmt *) malloc(sizeof(ca_pmt));

	pmt->ca_pmt_list_management = buffer[0];
	pmt->program_number = (buffer[1] << 8) | buffer[2];
	pmt->program_info_length = ((buffer[4] & 0x0F) << 8) | buffer[5];

#if 0
	printf("ca_pmt_list_management: %02x\n", pmt->ca_pmt_list_management);
	printf("prugram number: %04x\n", pmt->program_number);
	printf("program_info_length: %04x\n", pmt->program_info_length);
#endif

	switch (pmt->ca_pmt_list_management)
	{
		case 0x01: /* first */
		case 0x03: /* only */
			reset();
			break;

		default:
			break;
	}

	if (pmt->program_info_length != 0)
	{
		pmt->program_info = (ca_pmt_program_info *) malloc(sizeof(ca_pmt_program_info));

		pmt->program_info->ca_pmt_cmd_id = buffer[6];
		pmt->program_info->descriptor = (ca_descriptor *) malloc(sizeof(ca_descriptor));

		for (i = 0; i < pmt->program_info_length - 1; i += pmt->program_info->descriptor->descriptor_length + 2)
		{
			pmt->program_info->descriptor->descriptor_length = buffer[i + 8];
			pmt->program_info->descriptor->ca_system_id = (buffer[i + 9] << 8) | buffer[i + 10];
			pmt->program_info->descriptor->ca_pid = ((buffer[i + 11] & 0x1F) << 8)| buffer[i + 12];

#if 0
			printf("ca_system_id: %04x\n", pmt->program_info->descriptor->ca_system_id);
			printf("ca_pid: %04x\n", pmt->program_info->descriptor->ca_pid);
#endif

			for (j = 0; j < caid_count; j++)
			{
				if (caid[j] == pmt->program_info->descriptor->ca_system_id)
				{
					service.caID = pmt->program_info->descriptor->ca_system_id;
					service.ecmPID = pmt->program_info->descriptor->ca_pid;
					break;
				}
			}

			if (service.caID != 0)
			{
				break;
			}
		}

		free(pmt->program_info->descriptor);
		free(pmt->program_info);
	}

	pmt->es_info = (ca_pmt_es_info *) malloc(sizeof(ca_pmt_es_info));

	for (i = pmt->program_info_length + 6; i < length; i += pmt->es_info->es_info_length + 5)
	{
		if (service.numpids == MAX_PIDS)
		{
			break;
		}

		pmt->es_info->stream_type = buffer[i];
		pmt->es_info->elementary_pid = ((buffer[i + 1] & 0x1F) << 8) | buffer[i + 2];
		pmt->es_info->es_info_length = ((buffer[i + 3] & 0x0F) << 8) | buffer[i + 4];

#if 0
		printf("stream_type: %02x\n", pmt->es_info->stream_type);
		printf("elementary_pid: %04x\n", pmt->es_info->elementary_pid);
		printf("es_info_length: %04x\n", pmt->es_info->es_info_length);
#endif

		service.pid[service.numpids++] = pmt->es_info->elementary_pid;

		if (pmt->es_info->es_info_length != 0)
		{
			pmt->es_info->program_info = (ca_pmt_program_info *) malloc(sizeof(ca_pmt_program_info));

			pmt->es_info->program_info->ca_pmt_cmd_id = buffer[i + 5];
			pmt->es_info->program_info->descriptor = malloc(sizeof(ca_descriptor));

			for (j = 0; j < pmt->es_info->es_info_length - 1; j += pmt->es_info->program_info->descriptor->descriptor_length + 2)
			{
				pmt->es_info->program_info->descriptor->descriptor_length = buffer[i + j + 7];
				pmt->es_info->program_info->descriptor->ca_system_id = (buffer[i + j + 8] << 8) | buffer[i + j + 9];
				pmt->es_info->program_info->descriptor->ca_pid = ((buffer[i + j + 10] & 0x1F) << 8) | buffer[i + j + 11];

#if 0
				printf("ca_system_id: %04x\n", pmt->es_info->program_info->descriptor->ca_system_id);
				printf("ca_pid: %04x\n", pmt->es_info->program_info->descriptor->ca_pid);
#endif

				for (k = 0; k < caid_count; k++)
				{
					if (caid[k] == pmt->es_info->program_info->descriptor->ca_system_id)
					{
						service.caID = pmt->es_info->program_info->descriptor->ca_system_id;
						service.ecmPID = pmt->es_info->program_info->descriptor->ca_pid;
						break;
					}
				}

				if (service.caID != 0)
				{
					break;
				}
			}

			free(pmt->es_info->program_info->descriptor);
			free(pmt->es_info->program_info);
		}
	}

	service.sID = pmt->program_number;

	free(pmt->es_info);
	free(pmt);

	if ((service.numpids != 0) && (service.caID != 0))
	{
#ifdef LOOKUP_ONID
		if (current_onid == 0)
		{
			current_onid = parse_sdt();
		}

		if (current_onid == 0)
		{
			return -1;
		}
		
		service.onID = current_onid;
#else
		service.onID = 0x0085;
#endif /* LOOKUP_ONID */

		service.Unkwn = 0x0104;

		return adddescrambleservicestruct(&service);
	}
	else
	{
		return -1;
	}
}

unsigned int parse_length_field (unsigned char * buffer)
{
	unsigned char size_indicator = (buffer[0] >> 7) & 0x01;
	unsigned int length_value = 0;

	if (size_indicator == 0)
	{
		length_value = buffer[0] & 0x7F;
	}

	else if (size_indicator == 1)
	{
		unsigned char length_field_size = buffer[0] & 0x7F;
		unsigned int i;

		for (i = 0; i < length_field_size; i++)
		{
			length_value = (length_value << 8) | buffer[i + 1];
		}
	}

	return length_value;
}

unsigned char get_length_field_size (unsigned int length)
{
	if (length < 0x80)
	{
		return 0x01;
	}

	if (length < 0x100)
	{
		return 0x02;
	}

	if (length < 0x10000)
	{
		return 0x03;
	}

	if (length < 0x1000000)
	{
		return 0x04;
	}

	else
	{
		return 0x05;
	}
}

void handlesockmsg (unsigned char * buffer, ssize_t len, int connfd)
{
	int i;

#if 0
	printf("handlesockmsg (%02x):", len);
	for (i = 0; i < len; i++) printf(" %02x", buffer[i]);
	printf("\n");
#endif

	switch (buffer[0])
	{
		case 0x9F:
		{
			unsigned int length = parse_length_field(buffer + 3);

			// resource manager, application info, ca support
			if (buffer[1] == 0x80)
			{
				// ca_info_enq
				if (buffer[2] == 0x30)
				{
					unsigned char reply[4 + (caid_count << 1)];

					if (length != 0)
					{
						printf("[camd] warning: invalid length for ca_info_enq\n");
					}

					// ca_info
					reply[0] = 0x9F;
					reply[1] = 0x80;
					reply[2] = 0x31;
					reply[3] = caid_count << 1;

					for (i = 0; i < caid_count; i++)
					{
						reply[(i << 1) + 4] = caid[i] >> 8;
						reply[(i << 1) + 5] = caid[i];
					}

					if (write(connfd, reply, sizeof(reply)) < 0)
					{
						perror("[camd] write");
					}
				}

				// ca_pmt
				else if (buffer[2] == 0x32)
				{
					if ((3 + get_length_field_size(length) + length) == len)
					{
						parse_ca_pmt(buffer + 3 + get_length_field_size(length), length);
					}
					else
					{
						printf("[camd] ca_pmt: invalid length\n");
					}
				}

				else
				{
					printf("[camd] unknown resource manager, application info or ca support command\n");
				}
			}

			else
			{
				printf("[camd] unknown apdu tag\n");
			}

			break;
		}

		default:
			printf("[camd] unknown socket command: %02x\n", buffer[0]);
			break;
	}
}

int main (int argc, char ** argv)
{
	int listenfd;
	int connfd;
	struct sockaddr_un servaddr;
	int clilen;
	unsigned char msgbuffer[1024];

	switch (fork())
	{
	case -1:
		perror("[camd] fork");
		return -1;

	case 0:
		break;

	default:
		return 0;
	}

	if (setsid() < 0)
	{
		perror("[camd] setsid");
		return -1;
	}

	if ((camfd = open("/dev/dvb/card0/ca0", O_RDWR)) < 0)
	{
		perror("[camd] /dev/dvb/card0/ca0");
		return -1;
	}

	pthread_create(&camlisten, 0, camlistenthread, 0);

	/* software reset */
	reset();

	/* read caid */
	status();

	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, CAMD_UDS_NAME);
	clilen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);
	unlink(CAMD_UDS_NAME);

	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("[camd] socket");
		return -1;
	}

	if (bind(listenfd, (struct sockaddr *) &servaddr, clilen) < 0)
	{
		perror("[camd] bind");
		return -1;
	}

	if (listen(listenfd, 5) < 0)
	{
		perror("[camd] listen");
		return -1;
	}

	while (1)
	{
		connfd = accept(listenfd, (struct sockaddr *) &servaddr, (socklen_t *) &clilen);

		ssize_t length = read(connfd, msgbuffer, sizeof(msgbuffer));

		switch (length)
		{
		case -1:
			perror("[camd] read");
			break;

		case 0 ... 3:
			printf("[camd] too short message received\n");
			break;

		default:
			/* if message begins with an apdu_tag and is longer than three bytes */
			if ((msgbuffer[0] == 0x9F) && ((msgbuffer[1] >> 7) == 0x01) && ((msgbuffer[2] >> 7) == 0x00))
			{
				handlesockmsg(msgbuffer, length, connfd);
			}

			else
			{
				unsigned int i;

				printf("[camd] invalid message received:\n");

				for (i = 0; i < length; i++)
				{
					printf("%02x ", msgbuffer[i]);
				}

				printf("\n");
			}

			break;
		}

		close(connfd);
	}
}

