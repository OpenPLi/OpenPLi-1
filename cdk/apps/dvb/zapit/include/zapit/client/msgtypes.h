/*
 * $Header: /cvs/tuxbox/apps/dvb/zapit/include/zapit/client/msgtypes.h,v 1.6 2002/10/18 09:35:23 thegoodguy Exp $
 *
 * types used for clientlib <-> zapit communication - d-box2 linux project
 *
 * (C) 2002 by thegoodguy <thegoodguy@berlios.de>
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

#ifndef __msgtypes_h__
#define __msgtypes_h__


#include "zapittypes.h"
#include "basicmessage.h"
#include "zapitclient.h"


#define ZAPIT_UDS_NAME "/tmp/zapit.sock"


class CZapitMessages
{
 public:
	static const char ACTVERSION = 4;

	enum commands
		{
			CMD_SHUTDOWN		 = 1,

			CMD_REGISTEREVENTS,
			CMD_UNREGISTEREVENTS,

			CMD_ZAPTO,
			CMD_ZAPTO_CHANNELNR,
			CMD_ZAPTO_SERVICEID,
			CMD_ZAPTO_SUBSERVICEID,
			CMD_ZAPTO_SERVICEID_NOWAIT,
			CMD_ZAPTO_SUBSERVICEID_NOWAIT,

			CMD_STOP_VIDEO,					// not supported yet
			CMD_SET_MODE,
			CMD_GET_MODE,
			CMD_GET_LAST_CHANNEL,
			CMD_GET_APID_VPID,				// not supported yet
			CMD_GET_VTXT_PID,				// not supported yet
			CMD_GET_NVOD_CHANNELS,				// not supported yet
			CMD_REINIT_CHANNELS,
			CMD_GET_CHANNELS,
			CMD_GET_BOUQUETS,
			CMD_GET_BOUQUET_CHANNELS,
			CMD_GET_CA_INFO,					// not supported yet
			CMD_GET_CURRENT_SERVICEID,
			CMD_GET_CURRENT_SERVICEINFO,

			CMD_SCANSTART,
			CMD_SCANREADY,
			CMD_SCANGETSATLIST,
			CMD_SCANSETSCANSATLIST,
			CMD_SCANSETDISEQCTYPE,
			CMD_SCANSETDISEQCREPEAT,
			CMD_SCANSETBOUQUETMODE,

			CMD_BQ_ADD_BOUQUET,
			CMD_BQ_MOVE_BOUQUET,
			CMD_BQ_MOVE_CHANNEL,
			CMD_BQ_DELETE_BOUQUET,
			CMD_BQ_RENAME_BOUQUET,
			CMD_BQ_EXISTS_BOUQUET,					// Check if BouquetName existiert
			CMD_BQ_SET_LOCKSTATE,
			CMD_BQ_SET_HIDDENSTATE,
			CMD_BQ_ADD_CHANNEL_TO_BOUQUET,
			CMD_BQ_EXISTS_CHANNEL_IN_BOUQUET,			// Check if Channel already in BQ
			CMD_BQ_REMOVE_CHANNEL_FROM_BOUQUET,
			CMD_BQ_RENUM_CHANNELLIST,
			CMD_BQ_RESTORE,
			CMD_BQ_COMMIT_CHANGE,
			CMD_BQ_SAVE_BOUQUETS,

			CMD_SET_RECORD_MODE,
			CMD_GET_RECORD_MODE,
			CMD_SB_START_PLAYBACK,
			CMD_SB_STOP_PLAYBACK,
			CMD_SB_GET_PLAYBACK_ACTIVE,
			CMD_SET_DISPLAY_FORMAT,
			CMD_SET_AUDIO_MODE,
			CMD_READY,
			CMD_GETPIDS,
			CMD_SETSUBSERVICES,
			CMD_SET_AUDIOCHAN,
			CMD_MUTE,
			CMD_SET_VOLUME
		};

	struct commandBoolean
	{
		bool truefalse;
	};

	struct commandInt
	{
		int val;
	};

	struct commandVolume
	{
		unsigned int left;
		unsigned int right;
	};

	struct commandSetRecordMode
	{
		bool activate;
	};

	struct commandZapto
	{
		unsigned int bouquet;
		unsigned int channel;
	};

	struct commandZaptoChannelNr
	{
		unsigned int channel;
	};

	struct commandZaptoServiceID
	{
		t_channel_id channel_id;
	};

	struct commandSetAudioChannel
	{
		unsigned int channel;
	};

	struct commandGetBouquets
	{
		bool emptyBouquetsToo;
	};

	struct commandSetMode
	{
		CZapitClient::channelsMode mode;
	};

	struct commandGetBouquetChannels
	{
		unsigned int               bouquet;
		CZapitClient::channelsMode mode;
	};

	struct commandGetChannels
	{
		CZapitClient::channelsMode  mode;
		CZapitClient::channelsOrder order;
	};

	struct commandAddBouquet
	{
		char name[30];
	};

	struct commandExistsBouquet
	{
		char name[30];
	};

	struct commandExistsChannelInBouquet
	{
		unsigned int bouquet;
		t_channel_id channel_id;
	};


	struct commandAddChannelToBouquet
	{
		unsigned int bouquet;
		t_channel_id channel_id;
	};

	struct commandRemoveChannelFromBouquet
	{
		unsigned int bouquet;
		t_channel_id channel_id;
	};

	struct commandDeleteBouquet
	{
		unsigned int bouquet;
	};

	struct commandRenameBouquet
	{
		unsigned int bouquet;
		char         name[30];
	};

	struct commandMoveBouquet
	{
		unsigned int bouquet;
		unsigned int newPos;
	};

	struct commandStartScan
	{
		unsigned int satelliteMask;
	};

	struct commandBouquetState
	{
		unsigned int bouquet;
		bool	     state;
	};

	struct commandMoveChannel
	{
		unsigned int               bouquet;
		unsigned int               oldPos;
		unsigned int               newPos;
		CZapitClient::channelsMode mode;
	};




	struct responseGeneralTrueFalse
	{
		bool status;
	};

	struct responseGeneralInteger
	{
		int number;
	};

	struct responseGetRecordModeState
	{
		bool activated;
	};

	struct responseGetMode
	{
		CZapitClient::channelsMode  mode;
	};

	struct responseGetPlaybackState
	{
		bool activated;
	};

	struct responseGetCurrentServiceID
	{
		t_channel_id channel_id;
	};

	struct responseZapComplete
	{
		unsigned int zapStatus;
	};

	struct responseCmd
	{
		unsigned char cmd;
	};

	struct responseIsScanReady
	{
		bool scanReady;
		unsigned int satellite;
		unsigned int transponder;
		unsigned int services;
	};

};


#endif /* __msgtypes_h__ */
