/*
 * $Id: channel.h,v 1.15 2002/10/12 20:19:44 obi Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
 *	& Steffen Hehn <mcclean@berlios.de>
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

#ifndef __channel_h__
#define __channel_h__

/* system */
#include <string>
#include <stdint.h>

/* zapit */
#include "ci.h"
#include "types.h"

class CZapitAudioChannel
{
	public:
		unsigned short pid;
		bool isAc3;
		std::string description;
		unsigned char componentTag;
};

class CZapitChannel
{
	private:
		/* channel name */
		std::string name;

		/* pids of this channel */
		std::vector <CZapitAudioChannel *> audioChannels;
		unsigned short pcrPid;
		unsigned short pmtPid;
		unsigned short teletextPid;
		unsigned short videoPid;

		/* set true when pids are set up */
		bool pidsFlag;

		/* last selected audio channel */
		unsigned char currentAudioChannel;

		/* read only properties, set by constructor */
		t_service_id          service_id;
		t_transport_stream_id transport_stream_id;
		t_original_network_id original_network_id;
		unsigned char DiSEqC;

		/* read/write properties (write possibility needed by scan) */
		unsigned char serviceType;

		/* the conditional access program map table of this channel */
		CCaPmt * caPmt;

	public:
		/* constructor, desctructor */
		CZapitChannel (std::string p_name, t_service_id p_sid, t_transport_stream_id p_tsid, t_original_network_id p_onid, unsigned char p_service_type, unsigned char p_DiSEqC);
		~CZapitChannel ();

		/* get methods - read only variables */
		t_service_id          getServiceId()         { return service_id; }
		t_transport_stream_id getTransportStreamId() { return transport_stream_id; }
		t_original_network_id getOriginalNetworkId() { return original_network_id; }
		unsigned char         getServiceType()       { return serviceType; }
		unsigned char         getDiSEqC()            { return DiSEqC; }
		t_channel_id          getChannelID()         { return CREATE_CHANNEL_ID; }
		uint32_t              getTsidOnid()          { return (transport_stream_id << 16) | original_network_id; }

		/* get methods - read and write variables */
		std::string getName()			{ return name; }
		unsigned char getAudioChannelCount()	{ return audioChannels.size(); }
		unsigned short getPcrPid()		{ return pcrPid; }
		unsigned short getPmtPid()		{ return pmtPid; }
		unsigned short getTeletextPid()		{ return teletextPid; }
		unsigned short getVideoPid()		{ return videoPid; }
		bool getPidsFlag()			{ return pidsFlag; }
		CCaPmt * getCaPmt()			{ return caPmt; }

		CZapitAudioChannel * getAudioChannel (unsigned char index = 0xFF);
		unsigned short getAudioPid (unsigned char index = 0xFF);
		unsigned char  getAudioChannelIndex()	{ return currentAudioChannel; }

		int addAudioChannel(unsigned short pid, bool isAc3, std::string description, unsigned char componentTag);

		/* set methods */
		void setServiceType(const unsigned char pserviceType)   { serviceType = pserviceType; }
		void setName(std::string pName)				{ name = pName; }
		void setAudioChannel(unsigned char pAudioChannel)	{ if (pAudioChannel < audioChannels.size()) currentAudioChannel = pAudioChannel; }
		void setPcrPid(unsigned short pPcrPid)			{ pcrPid = pPcrPid; }
		void setPmtPid(unsigned short pPmtPid)			{ pmtPid = pPmtPid; }
		void setTeletextPid(unsigned short pTeletextPid)	{ teletextPid = pTeletextPid; }
		void setVideoPid(unsigned short pVideoPid)		{ videoPid = pVideoPid; }
		void setPidsFlag()					{ pidsFlag = true; }
		void setCaPmt(CCaPmt * pCaPmt)				{ caPmt = pCaPmt; }

		/* cleanup methods */
		void resetPids();
};

#endif /* __channel_h__ */
