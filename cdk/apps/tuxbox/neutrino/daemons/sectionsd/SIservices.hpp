#ifndef SISERVICES_HPP
#define SISERVICES_HPP
//
// $Id: SIservices.hpp,v 1.8.2.1 2003/02/06 18:59:43 thegoodguy Exp $
//
// classes SIservices and SIservices (dbox-II-project)
//
//    Homepage: http://dbox2.elxsi.de
//
//    Copyright (C) 2001 fnbrd (fnbrd@gmx.de),
//                  2002 thegoodguy (thegoodguy@berlios.de)
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//


#include <algorithm>

#include <sectionsdclient/sectionsdMsg.h>


// forward references
class SIservice;
class SIevent;

struct sdt_service {
	unsigned short service_id : 16;
	// 2 Byte
	unsigned char reserved_future_use : 6;
	unsigned char EIT_schedule_flag : 1;
	unsigned char EIT_present_following_flag : 1;
	// 3 Bytes
	unsigned char running_status : 3;
	unsigned char free_CA_mode : 1;
	unsigned short descriptors_loop_length : 12;
} __attribute__ ((packed)) ; // 5 Bytes


class SInvodReference
{
private:
	t_service_id          service_id;
	t_original_network_id original_network_id;
	t_transport_stream_id transport_stream_id;

public:
	SInvodReference(const t_transport_stream_id new_transport_stream_id, const t_original_network_id new_original_network_id, const t_service_id new_service_id)
		{
			service_id          = new_service_id;
			original_network_id = new_original_network_id;
			transport_stream_id = new_transport_stream_id;
		}

	SInvodReference(const SInvodReference &ref)
		{
			service_id          = ref.service_id;
			original_network_id = ref.original_network_id;
			transport_stream_id = ref.transport_stream_id;
		}

	bool operator < (const SInvodReference& ref) const
		{
			return uniqueKey() < ref.uniqueKey();
		}

	void dump(void) const
		{
			printf("NVOD Ref. Service-ID: %hu\n", service_id);
			printf("NVOD Ref. Original-Network-ID: %hu\n", original_network_id);
			printf("NVOD Ref. Transport-Stream-ID: %hu\n", transport_stream_id);
		}

	void toStream(char * &p) const
		{
			*(t_service_id          *)p = service_id;			p += sizeof(t_service_id);
			*(t_original_network_id *)p = original_network_id;		p += sizeof(t_original_network_id);
			*(t_transport_stream_id *)p = transport_stream_id;		p += sizeof(t_transport_stream_id);
		}

	const t_service_id getServiceID() const
		{
			return service_id;
		}

	t_channel_id uniqueKey(void) const {
		return CREATE_CHANNEL_ID; // cf. zapittypes.h
	}
};

// Fuer for_each
struct printSInvodReference : public std::unary_function<class SInvodReference, void>
{
	void operator() (const SInvodReference &ref) { ref.dump();}
};

typedef std::set <SInvodReference, std::less<SInvodReference> > SInvodReferences;

class SIservice {
public:
	SIservice(const struct sdt_service *s) {
		serviceID=s->service_id;
		originalNetworkID=0;
		serviceTyp=0;
		flags.EIT_schedule_flag=s->EIT_schedule_flag;
		flags.EIT_present_following_flag=s->EIT_present_following_flag;
		flags.running_status=s->running_status;
		flags.free_CA_mode=s->free_CA_mode;
	}
	// Um einen service zum Suchen zu erstellen
	SIservice(const t_service_id sid, const t_original_network_id onid) {
		serviceID=sid;
		originalNetworkID=onid;
		serviceTyp=0;
		memset(&flags, 0, sizeof(flags));
	}
	// Std-Copy
	SIservice(const SIservice &s) {
		serviceID=s.serviceID;
		originalNetworkID=s.originalNetworkID;
		serviceTyp=s.serviceTyp;
		providerName=s.providerName;
		serviceName=s.serviceName;
		flags=s.flags;
		nvods=s.nvods;
	}
	t_service_id          serviceID;
	t_original_network_id originalNetworkID; // Ist innerhalb einer section unnoetig
	unsigned char serviceTyp;
	SInvodReferences nvods;
	std::string serviceName; // Name aus dem Service-Descriptor
	std::string providerName; // Name aus dem Service-Descriptor
	int eitScheduleFlag(void) {return (int)flags.EIT_schedule_flag;}
	int eitPresentFollowingFlag(void) {return (int)flags.EIT_present_following_flag;}
	int runningStatus(void) {return (int)flags.running_status;}
	int freeCAmode(void) {return (int)flags.free_CA_mode;}

	bool operator < (const SIservice& s) const {
		return uniqueKey() < s.uniqueKey();
	}

	static t_channel_id makeUniqueKey(t_original_network_id original_network_id, t_service_id service_id) {
		return CREATE_CHANNEL_ID; // cf. zapittypes.h
	}

	t_channel_id uniqueKey(void) const {
		return makeUniqueKey(originalNetworkID, serviceID);
	}

	void dump(void) const {
		printf("Original-Network-ID: %hu\n", originalNetworkID);
		printf("Service-ID: %hu\n", serviceID);
		printf("Service-Typ: %hhu\n", serviceTyp);
		if(providerName.length())
			printf("Provider-Name: %s\n", providerName.c_str());
		if(serviceName.length())
			printf("Service-Name: %s\n", serviceName.c_str());
		for_each(nvods.begin(), nvods.end(), printSInvodReference());
		printf("\n");
	}
protected:
	struct {
		unsigned char EIT_schedule_flag : 1;
		unsigned char EIT_present_following_flag : 1;
		unsigned char running_status : 3;
		unsigned char free_CA_mode : 1;
	} flags;
};

// Fuer for_each
struct printSIservice : public std::unary_function<SIservice, void>
{
	void operator() (const SIservice &s) { s.dump();}
};

// Als Klasse, da ich nicht weiss, wie man eine Forward-Referenz auf ein typedef macht
class SIservices : public std::set <SIservice, std::less<SIservice> >
{
};

#endif // SISERVICES_HPP
