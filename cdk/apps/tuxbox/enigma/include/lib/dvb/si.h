#ifndef __si_h
#define __si_h

#include <vector>

#define SUPPORT_XML
#include <lib/dvb/esection.h>
#include <lib/base/estring.h>
#include <lib/base/eerror.h>
#include <lib/base/eptrlist.h>
#include <lib/dvb/lowlevel/sdt.h>
#include <lib/dvb/lowlevel/descr.h>
#include <lib/dvb/lowlevel/ca.h>
#include <lib/dvb/lowlevel/pmt.h>
#include <lib/dvb/lowlevel/nit.h>
#include <lib/dvb/lowlevel/eit.h>
#include <lib/dvb/lowlevel/bat.h>

time_t parseDVBtime(__u8 t1, __u8 t2, __u8 t3, __u8 t4, __u8 t5);
int fromBCD(int bcd);
int toBCD(int dec);

class Descriptor
{
	int tag;
protected:
	int len;
public:
	inline Descriptor(descr_gen_t *descr)
		:tag(*((__u8*)descr)), len((__u8)descr->descriptor_length)
	{
		len+=2;
	};
	inline Descriptor(int tag, int len)
		:tag(tag), len(len + 2) {}
	inline virtual ~Descriptor(){};

	static Descriptor *create(descr_gen_t *data, int tsidonid = 0, int type = 0);
	int Tag() { return tag; }

#ifdef SUPPORT_XML	
	eString toXML();
	/*
		<descriptor>
			<raw>...</raw>
			<parsed>
				<ComponentDescriptor>
					...
				</ComponentDescriptor>
			</parsed>
		</descriptor>
	*/
  virtual eString toString()=0;
#endif
};

class UnknownDescriptor: public Descriptor
{
public:
  __u8 *data;
  UnknownDescriptor(descr_gen_t *descr);
  ~UnknownDescriptor();
  eString toString();
  int length() { return len-2; }
};

class ServiceDescriptor: public Descriptor
{
	void init_ServiceDescriptor(sdt_service_desc *descr);
public:
  int service_type, tsidonid;
  eString service_provider, service_name;
  static const int CTag() { return DESCR_SERVICE; }
  ServiceDescriptor(sdt_service_desc *descr, int tsidonid);
  ~ServiceDescriptor();
  eString toString();
};

class CAIdentifierDescriptor: public Descriptor
{
	void init_CAIdentifierDescriptor(descr_gen_t *descr);
public:
  __u16 *CA_system_id;
  int CA_system_ids;
  CAIdentifierDescriptor(descr_gen_t *descr);
  ~CAIdentifierDescriptor();
  eString toString();
};

class LinkageDescriptor: public Descriptor
{
	void init_LinkageDescriptor(descr_linkage_struct *descr);
public:
  int transport_stream_id;
  int original_network_id;
  int service_id;
  int linkage_type;

  __u8 *private_data;
  int priv_len;

  int handover_type;
  int origin_type;

  int network_id;
  int initial_service_id;

  LinkageDescriptor(descr_linkage_struct *descr);
  ~LinkageDescriptor();
  eString toString();
};

class NVODReferenceEntry
{
public:
  bool operator==( const NVODReferenceEntry &e )
  {
    return e.transport_stream_id == transport_stream_id
      && e.original_network_id == original_network_id
      && e.service_id == service_id;
  }
  __u16 transport_stream_id, original_network_id, service_id;
  NVODReferenceEntry(__u16 transport_stream_id, __u16 original_network_id, __u16 service_id);
  ~NVODReferenceEntry();
};

class NVODReferenceDescriptor: public Descriptor
{
	void init_NVODReferenceDescriptor(descr_gen_t *descr);
public:
  NVODReferenceDescriptor(descr_gen_t *descr);
  ~NVODReferenceDescriptor();
  eString toString();

  ePtrList<NVODReferenceEntry> entries;
};

class TimeShiftedServiceDescriptor: public Descriptor
{
public:
  int reference_service_id;
  TimeShiftedServiceDescriptor(descr_time_shifted_service_struct *descr);
  eString toString();
};

class TimeShiftedEventDescriptor: public Descriptor
{
public:
  int reference_service_id;
  int reference_event_id;
  TimeShiftedEventDescriptor(descr_time_shifted_event_struct *descr);
  eString toString();
};


class StreamIdentifierDescriptor: public Descriptor
{
public:
  int component_tag;
  StreamIdentifierDescriptor(descr_stream_identifier_struct *descr);
  eString toString();
};

class CarouselIdentifierDescriptor: public Descriptor
{
public:
  int carousel_id;
  CarouselIdentifierDescriptor(descr_carousel_identifier_struct *descr);
  eString toString();
};

class DataBroadcastIdentifierDescriptor: public Descriptor
{
public:
  int broadcast_id;
  int application_type_code;
  int boot_priority_hint;
  DataBroadcastIdentifierDescriptor(descr_data_broadcast_id_struct *descr);
  eString toString();
};

class CADescriptor: public Descriptor
{
	void init_CADescriptor(ca_descr_t *descr);
public:
  __u16 CA_system_ID, CA_PID;
  __u8 *data;
  CADescriptor(ca_descr_t *descr);
  ~CADescriptor();
  eString toString();
};

class NetworkNameDescriptor: public Descriptor
{
public:
  eString network_name;
  NetworkNameDescriptor(descr_gen_t *descr);
  ~NetworkNameDescriptor();
  eString toString();
};

class CableDeliverySystemDescriptor: public Descriptor
{
	void init_CableDeliverySystemDescriptor(descr_cable_delivery_system_struct *descr);
public:
  __u32 frequency;
  int FEC_outer, modulation, symbol_rate, FEC_inner;
  CableDeliverySystemDescriptor(descr_cable_delivery_system_struct *descr);
  ~CableDeliverySystemDescriptor();
  eString toString();
};

class SatelliteDeliverySystemDescriptor: public Descriptor
{
	void init_SatelliteDeliverySystemDescriptor(descr_satellite_delivery_system_struct *descr);
public:
  __u32 frequency;
  __u16 orbital_position;
  int west_east_flag;
  int polarisation;
  int modulation;
  __u32 symbol_rate;
  int FEC_inner;
  SatelliteDeliverySystemDescriptor(descr_satellite_delivery_system_struct *descr);
  ~SatelliteDeliverySystemDescriptor();
  eString toString();
};

class TerrestrialDeliverySystemDescriptor: public Descriptor
{
	void init_TerrestrialDeliverySystemDescriptor(descr_terrestrial_delivery_system_struct *descr);
public:
  __u32 centre_frequency;
  int bandwidth;
  int constellation;
  int hierarchy_information;
  int code_rate_hp_stream;
  int code_rate_lp_stream;
  int guard_interval;
  int transmission_mode;
  int other_frequency_flag;
  TerrestrialDeliverySystemDescriptor(descr_terrestrial_delivery_system_struct *descr);
  ~TerrestrialDeliverySystemDescriptor();
  eString toString();
};

class ServiceListDescriptorEntry
{
public:
  ServiceListDescriptorEntry(__u16 service_id, __u8 service_type);
  ~ServiceListDescriptorEntry();

  __u16 service_id;
  __u8 service_type;
};

class ServiceListDescriptor: public Descriptor
{
public:
  ServiceListDescriptor(descr_gen_t *descr);
  ~ServiceListDescriptor();
  eString toString();

  ePtrList<ServiceListDescriptorEntry> entries;
};

class ShortEventDescriptor: public Descriptor
{
	void init_ShortEventDescriptor(descr_gen_t *descr);
public:
	ShortEventDescriptor(descr_gen_t *descr, int tsidonid);
	ShortEventDescriptor(int len, int tsidonid): Descriptor(0x4d, len), tsidonid(tsidonid) { };
	eString toString();
	char language_code[3];
	eString event_name;
	eString text;
	int tsidonid;
};

class ISO639LanguageDescriptor: public Descriptor
{
public:
	ISO639LanguageDescriptor(descr_gen_t *descr);
	eString toString();
	char language_code[3];
	int audio_type;
};

class AC3Descriptor: public Descriptor
{
	void init_AC3Descriptor(descr_gen_t *descr);
public:
	AC3Descriptor(descr_gen_t *descr);
	eString toString();
	int AC3_type, bsid, mainid, asvc;
};

class BouquetNameDescriptor: public Descriptor
{
public:
	BouquetNameDescriptor(descr_gen_t *descr);
	eString toString();
	eString name;
};

class ItemEntry
{
public:
	eString item_description;
	eString item;
	ItemEntry(eString &item_description, eString &item);
	~ItemEntry();
};

class ExtendedEventDescriptor: public Descriptor
{
	void init_ExtendedEventDescriptor(descr_gen_t *descr);
public:
	ExtendedEventDescriptor(descr_gen_t *descr, int tsidonid);
	eString toString();
	int descriptor_number;
	int last_descriptor_number;
	char language_code[3];
	ePtrList< ItemEntry > items;
	eString text;
	int tsidonid;
};

class ComponentDescriptor: public Descriptor
{
	void init_ComponentDescriptor(descr_component_struct *descr);
public:
	ComponentDescriptor(descr_component_struct *descr, int tsidonid);
	eString toString();

	int stream_content, component_type, component_tag;
	char language_code[3];
	eString text;
};

class ContentDescriptor: public Descriptor
{
	void init_ContentDescriptor(descr_gen_t *descr);
public:
	ContentDescriptor(descr_gen_t *descr);
	ePtrList< descr_content_entry_struct > contentList;	
	eString toString();
};

class LesRadiosDescriptor: public Descriptor
{
public:
	LesRadiosDescriptor(descr_lesradios_struct *descr);
	eString toString();
	
	int id;
	eString name;
};

class MHWDataDescriptor: public Descriptor
{
public:
	MHWDataDescriptor(descr_mhw_data_struct *desrc);
	eString toString();
	
	char type[8];
};

#ifdef ENABLE_DISH_EPG

class DishEventNameDescriptor: public Descriptor
{
public:
  DishEventNameDescriptor(descr_gen_t *descr, int tableid=1);
  eString toString();
  eString event_name;
  ~DishEventNameDescriptor();
};

class DishDescriptionDescriptor: public ShortEventDescriptor
{
public: 
  DishDescriptionDescriptor(descr_gen_t *descr, int tableid, int tsidonid);
  ~DishDescriptionDescriptor();
};

#endif

class ParentalRatingDescriptor: public Descriptor
{
public:
	ParentalRatingDescriptor(descr_gen_struct *descr);
	eString toString();
	std::map< eString, int > entryMap; // Country Code : age
};

class RegistrationDescriptor: public Descriptor
{
public:
	RegistrationDescriptor(descr_gen_struct *descr);
	eString toString();
	char format_identifier[4];
	eString additional_identification_info;
};

class PrivateDataSpecifierDescriptor: public Descriptor
{
public:
	PrivateDataSpecifierDescriptor(descr_gen_struct *descr);
	eString toString();
	unsigned long private_data_specifier;
};

class SubtitleEntry
{
public:
	SubtitleEntry(__u8 *data);
	char language_code[3];
	int subtitling_type;
	int composition_page_id;
	int ancillary_page_id;
};

class SubtitlingDescriptor: public Descriptor
{
public:
	SubtitlingDescriptor(descr_gen_struct *descr);
	eString toString();
	ePtrList<SubtitleEntry> entries;
};

class PATEntry
{
public:
	PATEntry(int program_number, int program_map_PID): program_number(program_number), program_map_PID(program_map_PID)
	{
	}
	int program_number;
	int program_map_PID;
};

class PAT: public eTable
{
protected:
	int data(__u8 *data);
public:
	PAT();

	PATEntry *searchService(int service_id)
	{
		for (ePtrList<PATEntry>::iterator i(entries); i != entries.end(); ++i)
			if (i->program_number==service_id)
				return *i;
		return 0;
	}

	int transport_stream_id;
	ePtrList<PATEntry> entries;

	__u8 *getRAW();
};

class SDTEntry
{
	void init_SDTEntry(sdt_descr_t *descr, int tsidonid);
public:
	SDTEntry(sdt_descr_t *descr, int tsidonid);
	int service_id;
	int EIT_schedule_flag;
	int EIT_present_following_flag;
	int running_status;
	int free_CA_mode;
	ePtrList< Descriptor > descriptors;
};

class SDT: public eTable
{
protected:
	int data(__u8 *data);
public:
	enum { typeActual=0, typeOther=1, typeBoth=2 };
	SDT(int type=typeActual, int tsid=-1);

	int transport_stream_id, original_network_id;
	ePtrList<SDTEntry> entries;
};

class PMTEntry
{
	void init_PMTEntry(pmt_info_t* info);
public:
	PMTEntry(pmt_info_t* info);
	int stream_type;
	int elementary_PID;
	ePtrList< Descriptor > ES_info;
};

class PMT: public eTable
{
protected:
	int data(__u8 *data);
public:
	PMT(int pid, int service_id, int version=-1);
	~PMT();

	int program_number, PCR_PID, pid, version_number;

	eTable *createNext();
	ePtrList< Descriptor > program_info;
	ePtrList<__u8> program_infoPlain;

	ePtrList<PMTEntry> streams;
	ePtrList<__u8> streamsPlain;

	__u8* getRAW();
};

class NITEntry
{
	void init_NITEntry(nit_ts_t* ts);
public:
	NITEntry(nit_ts_t* ts);

	__u16 transport_stream_id, original_network_id;
	ePtrList< Descriptor > transport_descriptor;
};

class NIT: public eTable
{
protected:
	int data(__u8 *data);
public:
	enum
	{
		typeActual=0, typeOther
	};
	NIT(int pid, int type=0);
	int network_id;
	ePtrList<NITEntry> entries;
	ePtrList< Descriptor > network_descriptor;
};

class EITEvent
{
	void init_EITEvent(const eit_event_struct *event, int tsidonid);
public:
	EITEvent(const eit_event_struct *event, int tsidonid, int type);
	EITEvent();
	int event_id;
	time_t start_time;
	int duration;
	int running_status;
	int free_CA_mode;
	ePtrList< Descriptor > descriptor;
};

class EIT: public eTable
{
protected:
	int data(__u8 *data);
public:
	enum
	{
		tsActual=0, tsOther, tsFaked
	};
	enum
	{
		typeNowNext=0, typeSchedule
	};
	
	EIT(int type, int service_id=-1, int ts=tsActual, int version=-1);
	EIT( const EIT* eit );
	EIT();
	~EIT();
	eTable *createNext();
	
	int type, ts, service_id, version_number, current_next_indicator, transport_stream_id, original_network_id;
	ePtrList<EITEvent> events;
	ePtrList<__u8> eventsPlain;
};

class TDT: public eTable
{
protected:
	int data(__u8 *data);
public:
	TDT();
	
	time_t UTC_time;
};

class BATEntry
{
	void init_BATEntry(bat_loop_struct *entry);
public:
	BATEntry(bat_loop_struct *data);
	int transport_stream_id, original_network_id;
	ePtrList< Descriptor > transport_descriptors;
};

class BAT: public eTable
{
protected:
	int data(__u8 *data);
public:
	BAT();
	
	int bouquet_id;
	ePtrList< Descriptor > bouquet_descriptors;
	ePtrList<BATEntry> entries;
};
#endif
