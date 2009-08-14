/***************************************************************************
                          common.h  -  description
                             -------------------
    begin                : Dec 03th 2005
    copyright            : (C) 2005 by PLi
 ***************************************************************************/

#ifndef _common_h
#define _common_h

typedef enum
{
	CMD_PUT_CHANNEL_SETTINGS	=	0,
	CMD_GET_CHANNEL_SETTINGS,
	CMD_GET_EMU_NAME,
	CMD_RESTART_EMU,
	CMD_ENUM_SETTINGS,
	CMD_GET_CARDSERVER_NAME,
	CMD_GET_CARDSERVER_SETTING,
	CMD_SET_CARDSERVER_SETTING,
	CMD_RESTART_CARDSERVER,
	CMD_GET_SERVICE_NAME,
	CMD_GET_SERVICE_SETTING,
	CMD_SET_SERVICE_SETTING,
	CMD_RESTART_SERVICES,
	CMD_PUT_CHANNEL_SETTINGS_AND_RESTART,
	CMD_GET_EMU_INFO,
};

const int NAMELENGTH	= 32;

struct packet
{
	int cmd;
	int datasize;
};

struct putchannelsettings
{
	int defaultemu;
	int channelemu;
	int provideremu;
	char channelname[NAMELENGTH];
	char providername[NAMELENGTH];
};

struct getchannelsettings
{
	int defaultemu;
	int channelemu;
	int provideremu;
};

struct enumsettings
{
	int channel;
	int provider;
	char settingname[NAMELENGTH];
	char emuname[NAMELENGTH];
};

struct setserversetting
{
	int id;
	int on;
};

struct emuinfo
{
	char name[32];
	char version[32];
};

#endif
