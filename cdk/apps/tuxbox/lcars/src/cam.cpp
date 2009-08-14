/***************************************************************************
 *                                                                         *
 *   You are not allowed to change this file in any way when used          *
 *                               with LCARS                                *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <memory.h>
#include <ost/ca.h>
#include <config.h>
#include "devices.h"

#include "cam.h"

	struct cmdCAID_s
	{
		unsigned char inita; // 0x6e50
		unsigned char initb; // 0x6e50
		unsigned char len; // (len+1)| 0x80
		unsigned char cmd; // 0x23
		unsigned char cmd2; // 0x3
		unsigned char crc;
	};
	cmdCAID_s cmdCAID;

	struct cmdReset_s
	{
		unsigned char inita; // 0x6e50
		unsigned char initb; // 0x6e50
		unsigned char len; // (len+1)| 0x80
		unsigned char cmd; // 0x23
		unsigned char cmd2; // 0x9;
		unsigned char crc;
	};
	cmdReset_s cmdReset;

	struct cmdInit_s
	{
		unsigned char inita; // 0x6e50
		unsigned char initb; // 0x6e50
		unsigned char len; // (len+1)| 0x80
		unsigned char cmd; // 0x23
		unsigned char cmd2; // 0x39
		unsigned char crc;
	};
	cmdInit_s cmdInit;

	struct cmdInit2_s
	{
		unsigned char inita; // 0x6e50
		unsigned char initb; // 0x6e50
		unsigned char len; // (len+1)| 0x80
		unsigned char cmd; // 0x23
		unsigned char cmd2; // 0x29
		unsigned char crc;
	};
	cmdInit2_s cmdInit2;

	struct cmdStart_s
	{
		unsigned char inita; // 0x6e50
		unsigned char initb; // 0x6e50
		unsigned char len; // (len+1)| 0x80
		unsigned char cmd; // 0x23
		unsigned char cmd2; // 0x3d;
		unsigned char SID1;
		unsigned char SID2;
		unsigned char crc;
	};
	cmdStart_s cmdStart;

	struct cmdDescramble_s
	{
		unsigned char inita; // 0x6e50
		unsigned char initb; // 0x6e50
		unsigned char len; // (len+1)| 0x80
		unsigned char cmd; // 0x23
		unsigned char cmd2; // 0x0d
		unsigned char ONID1;
		unsigned char ONID2;
		unsigned char SID1;
		unsigned char SID2;
		unsigned char unknowna; // 0x104
		unsigned char unknownb; // 0x104
		unsigned char CAID1;
		unsigned char CAID2;
		unsigned char ECM1;
		unsigned char ECM2;
		unsigned char numpids;
		unsigned char APID1;
		unsigned char APID2;
		unsigned char unknown3a; // 0x8000
		unsigned char unknown3b; // 0x8000
		unsigned char VPID1;
		unsigned char VPID2;
		unsigned char unknown4a; // 0x8000
		unsigned char unknown4b; // 0x8000
		unsigned char crc;
	};
	cmdDescramble_s cmdDescramble;

	struct cmdEMM_s
	{
		unsigned char inita; // 0x6e50
		unsigned char initb; // 0x6e50
		unsigned char len; // (len+1)| 0x80
		unsigned char cmd; // 0x23
		unsigned char cmd2; // 0x84
		unsigned char unknowna; // 0x104
		unsigned char unknownb; // 0x104
		unsigned char CAID1;
		unsigned char CAID2;
		unsigned char EMM1;
		unsigned char EMM2;
		unsigned char crc;
	};
	cmdEMM_s cmdEMM;

cam::cam()
{
	cmdCAID.cmd2 = 0x3;

	cmdReset.cmd2 = 0x9;

	cmdInit.cmd2 = 0x39;

	cmdInit2.cmd2 = 0x29;

	cmdStart.cmd2 = 0x3d;

	cmdDescramble.cmd2 = 0xd;
	cmdDescramble.unknowna = 0x1;
	cmdDescramble.unknownb = 0x04;

	cmdEMM.cmd2 = 0x84;
	cmdEMM.unknowna = 0x1;
	cmdEMM.unknownb = 0x4;

#ifdef HAVE_DREAMBOX_HARDWARE
	camfd = open(CAM_DEV, O_RDWR);

	if (camfd < 0) {
		perror(CAM_DEV);
		exit(1);
	}
#endif
}

cam::~cam()
{
	close(camfd);
}

void cam::initialize()
{
	cmdCAID.cmd2 = 0x3;

	cmdReset.cmd2 = 0x9;

	cmdInit.cmd2 = 0x39;

	cmdInit2.cmd2 = 0x29;

	cmdStart.cmd2 = 0x3d;

	cmdDescramble.cmd2 = 0xd;
	cmdDescramble.unknowna = 0x1;
	cmdDescramble.unknownb = 0x04;

	cmdEMM.cmd2 = 0x84;
	cmdEMM.unknowna = 0x1;
	cmdEMM.unknownb = 0x4;

	pid_count = 0;
}

void cam::cam_answer()
{
	ca_msg_t ca_msg;
	int len = 0;
	unsigned char buffer[128];

	while (len <=0)
	{
		ca_msg.length = 4;

		if (ioctl(camfd,CA_GET_MSG,&ca_msg) != 0)
		{
			perror("ioctl");
			return;
		}
	
		len = ca_msg.length;

		if ( len <= 0 )
		{
			usleep(500);
		 
		}
	
	}

	memcpy(buffer, ca_msg.msg, ca_msg.length);

	len=buffer[2] & 0x7F;

	ca_msg.length = len;

	if (ioctl(camfd, CA_GET_MSG, &ca_msg) < 0)
	{
		perror("ioctl");
		return;
	}

	if ( (int)ca_msg.length != len )
	{
		printf("%s: length mismatch\n", __FUNCTION__);
		return;
	}

	memcpy(buffer+4,ca_msg.msg,ca_msg.length);
}

void cam::sendCAM(void *data, unsigned int len)
{
	ca_msg_t ca_msg;
	int crc = 0;
	unsigned char command[255];

	memcpy(command, data, len);

	command[0] = 0x6e;
	command[1] = 0x50;
	command[2] = (len - 4);
	command[3] = 0x23;

	for (unsigned int i = 0; i < len - 1; i++)
		crc ^= command[i];

	command[len - 1] = crc;

	ca_msg.index = 0;
	ca_msg.type = 0;

	ca_msg.length = len - 1;
	memcpy(ca_msg.msg, command + 1, len - 1);
	
	if (ioctl(camfd, CA_SEND_MSG, &ca_msg) < 0)
		perror("CA_SEND_MSG");
}

void cam::readCAID()
{
	CAID = 0;

	while(CAID < 1)
	{
		sendCAM(&cmdCAID, sizeof(cmdCAID_s));

		int len = 0;
		ca_msg_t ca_msg;
		unsigned char buffer[128];
	
		while (len <=0)
		{
			ca_msg.length = 4;

			if (ioctl(camfd, CA_GET_MSG, &ca_msg) < 0)
			{
				perror("CA_GET_MSG");
				return;
			}
		
			len = ca_msg.length;
			if (len <= 0 )
			{
				usleep(500);
				 
			}
			
		}
		memcpy(buffer, ca_msg.msg, ca_msg.length);
		
		if (buffer[2] == 4) // CAM-Error... Shit!!!
			continue;

		len=buffer[2] & 0x7F;
		
		ca_msg.length = len;
		
		if (ioctl(camfd, CA_GET_MSG, &ca_msg) < 0)
		{
			perror("CA_GET_MSG");
			return;
		}
		
		if ((int)ca_msg.length != len)
		{
			printf("%s: length mismatch", __FUNCTION__);
			return;
		}
		
		memcpy(buffer+4,ca_msg.msg,ca_msg.length);
		CAID = (buffer[6]<<8)|buffer[7];
	}
}

bool cam::isfree()
{
	ca_msg_t ca_msg;
	int len = 0;
	unsigned char buffer[128];

	while (len <= 0)
	{
		ca_msg.length = 4;

		if (ioctl(camfd,CA_GET_MSG,&ca_msg) < 0)
		{
			perror("CA_GET_MSG");
			return true;
		}
	
		len = ca_msg.length;
		if ( len <= 0 )
		{
			usleep(500);
		 
		}
	
	}
	memcpy(buffer, ca_msg.msg, ca_msg.length);
	
	len=buffer[2] & 0x7F;

	ca_msg.length = len;

	if (ioctl(camfd, CA_GET_MSG, &ca_msg) < 0)
	{
		perror("CA_GET_MSG");
		return true;
	}

	if ((int)ca_msg.length != len)
	{
		printf("%s: length mismatch", __FUNCTION__);
		return true;
	}

	memcpy(buffer+4,ca_msg.msg,ca_msg.length);
	
	return (buffer[12] == 0x1d);
}

void cam::init()
{
	sendCAM(&cmdInit, sizeof(cmdInit_s));
}

void cam::init2()
{
	sendCAM(&cmdInit2, sizeof(cmdInit2_s));
}

void cam::reset()
{
	sendCAM(&cmdReset, sizeof(cmdReset_s));
}

void cam::start()
{
	cmdStart.SID1 = SID >> 8;
	cmdStart.SID2 = SID & 0xff;

	sendCAM(&cmdStart, sizeof(cmdStart_s));
}

void cam::descramble()
{
	unsigned char buffer[100];

	cmdDescramble.ONID1 = ONID >> 8;
	cmdDescramble.ONID2 = ONID & 0xff;

	cmdDescramble.SID1 = SID >> 8;
	cmdDescramble.SID2 = SID & 0xff;
	cmdDescramble.CAID1 = CAID >> 8;
	cmdDescramble.CAID2 = CAID & 0xff;
	cmdDescramble.ECM1 = ECM >> 8;
	cmdDescramble.ECM2 = ECM & 0xff;
	cmdDescramble.numpids = pid_count;

	memcpy(buffer, &cmdDescramble, sizeof(cmdDescramble_s));

	
	for (int i = 0; i < pid_count; i++)
	{
		buffer[sizeof(cmdDescramble) + i * 4] = PIDs[i] >> 8;
		buffer[sizeof(cmdDescramble) + i * 4 + 1] = PIDs[i] & 0xff;
		buffer[sizeof(cmdDescramble) + i * 4 + 2] = 0x80;
		buffer[sizeof(cmdDescramble) + i * 4 + 3] = 0x0;
	}

	sendCAM(&buffer, sizeof(cmdDescramble_s) + pid_count * 4 + 1);
}

void cam::startEMM()
{
	cmdEMM.CAID1 = CAID >> 8;
	cmdEMM.CAID2 = CAID & 0xff;
	cmdEMM.EMM1 = EMM >> 8;
	cmdEMM.EMM2 = EMM & 0xff;

	sendCAM(&cmdEMM, sizeof(cmdEMM_s));
}
