#include <lib/dvb/si.h>

#include <config.h>
#include <stdio.h>
#include <time.h>
#include <sstream>
#include <iomanip>
#include <lib/system/info.h>
#include <lib/system/econfig.h>

extern "C"
{
	time_t my_mktime (struct tm *tp);
}
#define HILO(x) (x##_hi << 8 | x##_lo) 
#include <lib/dvb/lowlevel/decode.h>
#include <lib/dvb/lowlevel/dvb.h>
#include <lib/dvb/lowlevel/pat.h>
#include <lib/dvb/lowlevel/tdt.h>

static int getEncodingTable( const char * language_code )
{
	std::map<eString, int>::iterator it =
		eString::CountryCodeDefaultMapping.find(eString(language_code,3));
	if ( it != eString::CountryCodeDefaultMapping.end() )
		return it->second;
	return 0;  // ISO8859-1 / Latin1
}

static eString qHex(int v)
{
	return eString().sprintf("%04x", v);
}

int fromBCD(int bcd)
{
	if ((bcd&0xF0)>=0xA0)
		return -1;
	if ((bcd&0xF)>=0xA)
		return -1;
	return ((bcd&0xF0)>>4)*10+(bcd&0xF);
}

int toBCD(int dec)
{
	if (dec >= 100)
		return -1;
	return int(dec/10)*0x10 + dec%10;
}


time_t parseDVBtime(__u8 t1, __u8 t2, __u8 t3, __u8 t4, __u8 t5)
{
	tm t;
	t.tm_sec=fromBCD(t5);
	t.tm_min=fromBCD(t4);
	t.tm_hour=fromBCD(t3);
	int mjd=(t1<<8)|t2;
	int k;

	t.tm_year = (int) ((mjd - 15078.2) / 365.25);
	t.tm_mon = (int) ((mjd - 14956.1 - (int)(t.tm_year * 365.25)) / 30.6001);
	t.tm_mday = (int) (mjd - 14956 - (int)(t.tm_year * 365.25) - (int)(t.tm_mon * 30.6001));
	k = (t.tm_mon == 14 || t.tm_mon == 15) ? 1 : 0;
	t.tm_year = t.tm_year + k;
	t.tm_mon = t.tm_mon - 1 - k * 12;
	t.tm_mon--;

	t.tm_isdst =  0;
	t.tm_gmtoff = 0;

	return timegm(&t);
//	return my_mktime(&t)-timezone;
}

unsigned int crc32_table[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};

static unsigned int crc32_be(unsigned int crc, unsigned char const *data, unsigned int len)
{
	for (unsigned int i=0; i<len; i++)
		crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *data++) & 0xff];

	return crc;
}

Descriptor *Descriptor::create(descr_gen_t *descr, int tsidonid, int type)
{
	switch (descr->descriptor_tag)
	{
	case DESCR_SERVICE:
		return new ServiceDescriptor((sdt_service_desc*)descr, tsidonid);
	case DESCR_CA_IDENT:
		return new CAIdentifierDescriptor(descr);
	case DESCR_LINKAGE:
		return new LinkageDescriptor((descr_linkage_struct*)descr);
	case DESCR_NVOD_REF:
		return new NVODReferenceDescriptor(descr);
	case DESCR_TIME_SHIFTED_SERVICE:
		return new TimeShiftedServiceDescriptor((descr_time_shifted_service_struct*)descr);
	case DESCR_TIME_SHIFTED_EVENT:
		return new TimeShiftedEventDescriptor((descr_time_shifted_event_struct*)descr);	
	case DESCR_STREAM_ID:
		return new StreamIdentifierDescriptor((descr_stream_identifier_struct*)descr);
	case 9:
		return new CADescriptor((ca_descr_t*)descr);
	case DESCR_NW_NAME:
		return new NetworkNameDescriptor(descr);
	case DESCR_CABLE_DEL_SYS:
		return new CableDeliverySystemDescriptor((descr_cable_delivery_system_struct*)descr);
	case DESCR_SERVICE_LIST:
		return new ServiceListDescriptor(descr);
	case DESCR_SAT_DEL_SYS:
	  return new SatelliteDeliverySystemDescriptor((descr_satellite_delivery_system_struct*)descr);
	case DESCR_SHORT_EVENT:
		return new ShortEventDescriptor(descr,tsidonid);
	case DESCR_ISO639_LANGUAGE:
		return new ISO639LanguageDescriptor(descr);
	case DESCR_AC3:
		return new AC3Descriptor(descr);
	case DESCR_BOUQUET_NAME:
		return new BouquetNameDescriptor(descr);
	case DESCR_EXTENDED_EVENT:
		return new ExtendedEventDescriptor(descr,tsidonid);
	case DESCR_COMPONENT:
		return new ComponentDescriptor((descr_component_struct*)descr, tsidonid);
	case DESCR_LESRADIOS:
		return new LesRadiosDescriptor((descr_lesradios_struct*)descr);
	case DESCR_MHW_DATA:
		return new MHWDataDescriptor((descr_mhw_data_struct*)descr);
	case DESCR_PARENTAL_RATING:
		return new ParentalRatingDescriptor((descr_gen_struct*)descr);
	case DESCR_CONTENT:
		return new ContentDescriptor((descr_gen_struct*)descr);
	case DESCR_REGISTRATION:
		return new RegistrationDescriptor((descr_gen_struct*)descr);
	case DESCR_TERR_DEL_SYS:
		return new TerrestrialDeliverySystemDescriptor((descr_terrestrial_delivery_system_struct*)descr);
	case DESCR_SUBTITLING:
		return new SubtitlingDescriptor((descr_gen_struct*)descr);
	case DESCR_PRIV_DATA_SPEC:
		return new PrivateDataSpecifierDescriptor((descr_gen_struct*)descr);
#ifdef ENABLE_DISH_EPG
	case DESCR_DISH_EVENT_NAME:
		return new DishEventNameDescriptor(descr, type);
	case DESCR_DISH_DESCRIPTION:
		return new DishDescriptionDescriptor(descr, type, tsidonid);
#endif
	case DESCR_CAROUSEL_ID:
		return new CarouselIdentifierDescriptor((descr_carousel_identifier_struct*)descr);
	case DESCR_DATA_BROADCAST_ID:
		return new DataBroadcastIdentifierDescriptor((descr_data_broadcast_id_struct*)descr);
	case DESCR_STUFFING:
	case DESCR_COUNTRY_AVAIL:
	case DESCR_MOSAIC:
	case DESCR_TELETEXT:
	case DESCR_TELEPHONE:
	case DESCR_LOCAL_TIME_OFF:
	case DESCR_ML_NW_NAME:
	case DESCR_ML_BQ_NAME:
	case DESCR_ML_SERVICE_NAME:
	case DESCR_ML_COMPONENT:
	case DESCR_SERVICE_MOVE:
	case DESCR_SHORT_SMOOTH_BUF:
	case DESCR_FREQUENCY_LIST:
	case DESCR_PARTIAL_TP_STREAM:
	case DESCR_DATA_BROADCAST:
	case DESCR_CA_SYSTEM:
	default:
		return new UnknownDescriptor(descr);
	}
}

eString Descriptor::toXML()
{
	return "<descriptor><parsed>" + toString() + "</parsed></descriptor>\n";
}

UnknownDescriptor::UnknownDescriptor(descr_gen_t *descr)
	:Descriptor(descr), data(0)
{
	if ( len > 2 )
	{
		data = new __u8[len-2];
		memcpy(data, (__u8*) (descr+1), len-2);
	}
}

UnknownDescriptor::~UnknownDescriptor()
{
	if (len>2)
	{
		delete [] data;
		data=0;
	}
}

#ifdef SUPPORT_XML
eString UnknownDescriptor::toString()
{
	return "";
}
#endif

ServiceDescriptor::ServiceDescriptor(sdt_service_desc *descr, int tsidonid)
	:Descriptor((descr_gen_t*)descr), tsidonid(tsidonid)
{
	init_ServiceDescriptor(descr);
}
void ServiceDescriptor::init_ServiceDescriptor(sdt_service_desc *descr)
{
	int spl=descr->service_provider_name_length;
	service_type=descr->service_type;
	service_provider=convertDVBUTF8((unsigned char*)(descr+1), spl, 0, tsidonid);
	sdt_service_descriptor_2 *descr2=(sdt_service_descriptor_2*)((__u8*)(descr+1)+spl);
	spl=descr2->service_name_length;
	service_name=convertDVBUTF8((unsigned char*)(descr2+1), spl, 0, tsidonid);
}

ServiceDescriptor::~ServiceDescriptor()
{
}

#ifdef SUPPORT_XML
eString ServiceDescriptor::toString()
{
	eString res=
	"<ServiceDescriptor>"
	"<service_type>" + qHex(service_type);
	res+="</service_type><service_provider>";
	res+=service_provider;
	res+="</service_provider><service_name>";
	res+=service_name;
	res+="</service_name></ServiceDescriptor>\n";
	return res;
}
#endif

CAIdentifierDescriptor::CAIdentifierDescriptor(descr_gen_t *descr)
	:Descriptor(descr)
{
	init_CAIdentifierDescriptor(descr);
}
void CAIdentifierDescriptor::init_CAIdentifierDescriptor(descr_gen_t *descr)
{
	CA_system_ids=(len-2)/2;
	if ( CA_system_ids > 0 )
	{
		CA_system_id=new __u16[CA_system_ids];
		for (int i=0; i<CA_system_ids; i++)
			CA_system_id[i]=(((__u8*)(descr+1))[i*2]<<8)|(((__u8*)(descr+1))[i*2+1]);
	}
	else
	{
		CA_system_id=0;
		CA_system_ids=0;
	}
}

#ifdef SUPPORT_XML
eString CAIdentifierDescriptor::toString()
{
	eString res="<CAIdentifier>";
	for (int i=0; i<CA_system_ids; i++)
		res+="<ca_system_id>"+qHex(CA_system_id[i])+"</ca_system_id>";
	res+="</CAIdentifier>\n";
	return res;
}
#endif

CAIdentifierDescriptor::~CAIdentifierDescriptor()
{
	delete[] CA_system_id;
}

LinkageDescriptor::LinkageDescriptor(descr_linkage_struct *descr)
	:Descriptor((descr_gen_t*)descr)
{
	init_LinkageDescriptor(descr);
}
void LinkageDescriptor::init_LinkageDescriptor(descr_linkage_struct *descr)
{
	private_data=0;
	priv_len=0;
	transport_stream_id=HILO(descr->transport_stream_id);
	original_network_id=HILO(descr->original_network_id);
	service_id=HILO(descr->service_id);
	linkage_type=descr->linkage_type;
	if (linkage_type!=8)
	{
		priv_len=len-LINKAGE_LEN;
		if (priv_len>0)
		{
			private_data=new __u8[priv_len+1];
			private_data[priv_len]=0;
			memcpy(private_data, ((__u8*)descr)+LINKAGE_LEN, priv_len);
		}
		else
			priv_len=0;
	} else
	{
		handover_type=descr->handover_type;
		origin_type=descr->origin_type;
		__u8 *ptr=((__u8*)descr)+LINKAGE_LEN+1;
		if ((handover_type == 1) ||
				(handover_type == 2) ||
				(handover_type == 3))
		{
			network_id=*ptr++ << 8;
			network_id|=*ptr++;
		}
		if (!origin_type)
		{
			initial_service_id=*ptr++ << 8;
			initial_service_id|=*ptr++;
		}
		priv_len=((__u8*)descr)+len-ptr;
		if (priv_len>0)
		{
			private_data=new __u8[priv_len+1];
			private_data[priv_len]=0;
			memcpy(private_data, ptr, priv_len);
		}
		else
			priv_len=0;
	}
}

LinkageDescriptor::~LinkageDescriptor()
{
	if (private_data)
		delete[] private_data;
}

#ifdef SUPPORT_XML
eString LinkageDescriptor::toString()
{
	eString res="<LinkageDescriptor>";
	res+="<transport_stream_id>"+qHex(transport_stream_id)+"</transport_stream_id>";
	res+="<original_network_id>"+qHex(original_network_id)+"</original_network_id>";
	res+="<service_id>"+qHex(service_id)+"</service_id>";
	res+="<linkage_type>"+qHex(linkage_type)+"</linkage_type>";
	if (linkage_type==8)
	{
		res+="<handover_type>" + qHex(handover_type) + "</handover_type>";
		res+="<origin_type>" + qHex(origin_type) + "</handover_type>";
		if (!origin_type)
		{
			res+="<network_id>" + qHex(network_id)  + "</network_id>";
			res+="<initial_service_id>" + qHex(initial_service_id)  + "</intial_service_id>";
		}
	}
	if (priv_len)
	{
		res+="<private>";
		for (int i=0; i<priv_len; i++)
			res+=eString().sprintf(" %02x", private_data[i]);
		res+="</private>";
	}
	res+="</LinkageDescriptor>\n";
	return res;
}
#endif

NVODReferenceEntry::NVODReferenceEntry(__u16 transport_stream_id, __u16 original_network_id, __u16 service_id)
	:transport_stream_id(transport_stream_id), original_network_id(original_network_id), service_id(service_id)
{
}

NVODReferenceEntry::~NVODReferenceEntry()
{
}

NVODReferenceDescriptor::NVODReferenceDescriptor(descr_gen_t *descr)
	:Descriptor(descr)
{
	init_NVODReferenceDescriptor(descr);
}
void NVODReferenceDescriptor::init_NVODReferenceDescriptor(descr_gen_t *descr)
{
	entries.setAutoDelete(true);
	for (int i=0; i<(len-2); i+=6)
		entries.push_back(new NVODReferenceEntry((((__u8*)(descr+1))[i]<<8) | (((__u8*)(descr+1))[i+1]),
			(((__u8*)(descr+1))[i+2]<<8) | (((__u8*)(descr+1))[i+3]),	(((__u8*)(descr+1))[i+4]<<8) | (((__u8*)(descr+1))[i+5])));
}

NVODReferenceDescriptor::~NVODReferenceDescriptor()
{
}

#ifdef SUPPORT_XML
eString NVODReferenceDescriptor::toString()
{
	eString res;
	res="<NVODReferenceDescriptor>";
	for (ePtrList<NVODReferenceEntry>::iterator i(entries); i != entries.end(); ++i)
	{
		res+="<NVODReferenceEntry>";
		res+="<transport_stream_id>" + qHex(i->transport_stream_id) + "</transport_stream_id>";
		res+="<original_network_id>" + qHex(i->original_network_id) + "</original_network_id>";
		res+="<service_id>" + qHex(i->service_id) + "</service_id>";
	}
	res+="</NVODReferenceDescriptor>\n";
	return res;
}
#endif

TimeShiftedServiceDescriptor::TimeShiftedServiceDescriptor(descr_time_shifted_service_struct *descr)
	:Descriptor((descr_gen_t*)descr)
{
	reference_service_id=HILO(descr->reference_service_id);
}

#ifdef SUPPORT_XML
eString TimeShiftedServiceDescriptor::toString()
{
	eString res="<TimeShiftedServiceDescriptor>";
	res+="<reference_service_id>" + qHex(reference_service_id) + "</reference_service_id>";
	res+="</TimeShiftedServiceDescriptor>\n";
	return res;
}
#endif

TimeShiftedEventDescriptor::TimeShiftedEventDescriptor(descr_time_shifted_event_struct *descr)
	:Descriptor((descr_gen_t*)descr)
{
	reference_service_id=HILO(descr->reference_service_id);
	reference_event_id=HILO(descr->reference_event_id);
}

#ifdef SUPPORT_XML
eString TimeShiftedEventDescriptor::toString()
{
	eString res="<TimeShiftedEventDescriptor>";
	res+="<reference_service_id>" + qHex(reference_service_id) + "</reference_service_id>";
	res+="<reference_event_id>" + qHex(reference_event_id) + "</reference_event_id>";
	res+="</TimeShiftedEventDescriptor>\n";
	return res;
}
#endif

StreamIdentifierDescriptor::StreamIdentifierDescriptor(descr_stream_identifier_struct *descr)
	:Descriptor((descr_gen_t*)descr)
{
	component_tag=descr->component_tag;
}

#ifdef SUPPORT_XML
eString StreamIdentifierDescriptor::toString()
{
	eString res="<StreamIdentifierDescriptor>";
	res+="<component_tag>" + qHex(component_tag) + "</component_tag>";
	res+="</StreamIdentifierDescriptor>\n";
	return res;
}
#endif

CarouselIdentifierDescriptor::CarouselIdentifierDescriptor(descr_carousel_identifier_struct *descr)
 : Descriptor((descr_gen_t*)descr)
{
	carousel_id = descr->carousel_id[0] << 24 | descr->carousel_id[1] << 16 | descr->carousel_id[2] << 8 | descr->carousel_id[3];
#if 1
	eDebug(toString().c_str());
#endif
}

#ifdef SUPPORT_XML
eString CarouselIdentifierDescriptor::toString()
{
	eString res="<CarouselIdentifierDescriptor>";
	res += "<carousel_id>" + qHex(carousel_id) + "</carousel_id>";
	res += "</CarouselIdentifierDescriptor>\n";
	return res;
}
#endif

DataBroadcastIdentifierDescriptor::DataBroadcastIdentifierDescriptor(descr_data_broadcast_id_struct *descr)
 : Descriptor((descr_gen_t*)descr)
{
	broadcast_id = descr->data_broadcast_id_hi << 8 | descr->data_broadcast_id_lo;
	application_type_code = descr->application_type_hi << 8 | descr->application_type_lo;
	boot_priority_hint = descr->boot_priority_hint;
#if 1
	eDebug(toString().c_str());
#endif
}

#ifdef SUPPORT_XML
eString DataBroadcastIdentifierDescriptor::toString()
{
	eString res="<DataBroadcastIdentifierDescriptor>";
	res+="<broadcast_id>" + qHex(broadcast_id) + "</broadcast_id>";
	res+="<application_type_code>" + qHex(application_type_code) + "</application_type_code>";
	res+="<boot_priority_hint>" + qHex(boot_priority_hint) + "</boot_priority_hint>";
	res += "DataBroadcastIdentifierDescriptor\n";
	return res;
}
#endif

CADescriptor::CADescriptor(ca_descr_t *descr)
	:Descriptor((descr_gen_t*)descr)
{
	init_CADescriptor(descr);
}
void CADescriptor::init_CADescriptor(ca_descr_t *descr)
{
	if ( len > 0 )
	{
		data=new __u8[len];
		memcpy(data, descr, len);
		CA_system_ID=HILO(descr->CA_system_ID);
		CA_PID=HILO(descr->CA_PID);
	}
	else
		data=0;
}

CADescriptor::~CADescriptor()
{
	delete[] data;
}

#ifdef SUPPORT_XML
eString CADescriptor::toString()
{
	eString res="<CADescriptor>";
	res+="<CA_system_ID>"+qHex(CA_system_ID)+"</CA_system_ID>";
	res+="<CA_PID>"+qHex(CA_PID)+"</CA_PID></CADescriptor>\n";
	return res;
}
#endif

NetworkNameDescriptor::NetworkNameDescriptor(descr_gen_t *descr)
	:Descriptor(descr)
{
	network_name=convertDVBUTF8((unsigned char*)(descr+1), len-2, 0);
}

NetworkNameDescriptor::~NetworkNameDescriptor()
{
}

#ifdef SUPPORT_XML
eString NetworkNameDescriptor::toString()
{
	eString res="<NetworkNameDescriptor>";
	res+="<network_name>" + eString(network_name) + "</network_name>";
	res+="</NetworkNameDescriptor>\n";
	return res;
}
#endif

CableDeliverySystemDescriptor::CableDeliverySystemDescriptor(descr_cable_delivery_system_struct *descr)
	:Descriptor((descr_gen_t*)descr)
{
	init_CableDeliverySystemDescriptor(descr);
}
void CableDeliverySystemDescriptor::init_CableDeliverySystemDescriptor(descr_cable_delivery_system_struct *descr)
{
	frequency= (descr->frequency1>>4) *1000000;
	frequency+=(descr->frequency1&0xF)*100000;
	frequency+=(descr->frequency2>>4) *10000;
	frequency+=(descr->frequency2&0xF)*1000;
	frequency+=(descr->frequency3>>4) *100;
	frequency+=(descr->frequency3&0xF)*10;
	frequency+=(descr->frequency4>>4) *1;
//	frequency+=(descr->frequency4&0xF)*1;
	FEC_outer=descr->fec_outer;
	modulation=descr->modulation;
	symbol_rate=(descr->symbol_rate1>>4)   * 100000000;
	symbol_rate+=(descr->symbol_rate1&0xF) * 10000000;
	symbol_rate+=(descr->symbol_rate2>>4)  * 1000000;
	symbol_rate+=(descr->symbol_rate2&0xF) * 100000;
	symbol_rate+=(descr->symbol_rate3>>4)  * 10000;
	symbol_rate+=(descr->symbol_rate3&0xF) * 1000;
	symbol_rate+=(descr->symbol_rate4&0xF) * 100;
	FEC_inner=descr->fec_inner;
}

CableDeliverySystemDescriptor::~CableDeliverySystemDescriptor()
{
}

#ifdef SUPPORT_XML
eString CableDeliverySystemDescriptor::toString()
{
	eString res="<CableDeliverySystemDescriptor>";
	res+=eString().sprintf("<frequency>%d</frequency>", frequency);
	res+=eString().sprintf("<FEC_outer>%d</FEC_outer>", FEC_outer);
	res+=eString().sprintf("<modulation>QAM%d</modulation>", 8<<modulation);
	res+=eString().sprintf("<symbol_rate>%d</symbol_rate>", symbol_rate);
	res+=eString().sprintf("<FEC_inner>%d</FEC_inner>", FEC_inner);
	res+="</CableDeliverySystemDescriptor>\n";
	return res;
}
#endif

SatelliteDeliverySystemDescriptor::SatelliteDeliverySystemDescriptor(descr_satellite_delivery_system_struct *descr)
	:Descriptor((descr_gen_t*)descr)
{
	init_SatelliteDeliverySystemDescriptor(descr);
}
void SatelliteDeliverySystemDescriptor::init_SatelliteDeliverySystemDescriptor(descr_satellite_delivery_system_struct *descr)
{
	frequency= (descr->frequency1>>4) *100000000;
	frequency+=(descr->frequency1&0xF)*10000000;
	frequency+=(descr->frequency2>>4) *1000000;
	frequency+=(descr->frequency2&0xF)*100000;
	frequency+=(descr->frequency3>>4) *10000;
	frequency+=(descr->frequency3&0xF)*1000;
	frequency+=(descr->frequency4>>4) *100;
	frequency+=(descr->frequency4&0xF);
	orbital_position =(descr->orbital_position1>>4)*1000;
	orbital_position+=(descr->orbital_position1&0xF)*100;
	orbital_position+=(descr->orbital_position2>>4)*10;
	orbital_position+=(descr->orbital_position2&0xF)*1;
	west_east_flag=descr->west_east_flag;
	polarisation=descr->polarization;
	modulation=descr->modulation;
	symbol_rate=(descr->symbol_rate1>>4)   * 100000000;
	symbol_rate+=(descr->symbol_rate1&0xF) * 10000000;
	symbol_rate+=(descr->symbol_rate2>>4)  * 1000000;
	symbol_rate+=(descr->symbol_rate2&0xF) * 100000;
	symbol_rate+=(descr->symbol_rate3>>4)  * 10000;
	symbol_rate+=(descr->symbol_rate3&0xF) * 1000;
	symbol_rate+=(descr->symbol_rate4&0xF) * 100;
	FEC_inner=descr->fec_inner;
}

SatelliteDeliverySystemDescriptor::~SatelliteDeliverySystemDescriptor()
{
}

#ifdef SUPPORT_XML
eString SatelliteDeliverySystemDescriptor::toString()
{
	eString res="<SatelliteDeliverySystemDescriptor>";
	res+=eString().sprintf("<frequency>%d</frequency>", frequency);
	res+=eString().sprintf("<orbital_position>%3d.%d%c</orbital_position>", orbital_position/10, orbital_position%10, west_east_flag?'E':'W');
	res+="<polarisation>";
	switch (polarisation)
	{
	case 0: res+="horizontal"; break;
	case 1: res+="vertical"; break;
	case 2: res+="left"; break;
	case 3: res+="right"; break;
	}
	res+=eString().sprintf("</polarisation><modulation>%d</modulation>", modulation);
	res+=eString().sprintf("<symbol_rate>%d</symbol_rate>", symbol_rate);
	res+=eString().sprintf("<FEC_inner>%d/%d</FEC_inner></SatelliteDeliverySystemDescriptor>\n", FEC_inner, FEC_inner+1);
	return res;
}
#endif

TerrestrialDeliverySystemDescriptor::TerrestrialDeliverySystemDescriptor(descr_terrestrial_delivery_system_struct *descr)
	:Descriptor((descr_gen_t*)descr) 
{
	init_TerrestrialDeliverySystemDescriptor(descr);
}
void TerrestrialDeliverySystemDescriptor::init_TerrestrialDeliverySystemDescriptor(descr_terrestrial_delivery_system_struct *descr)
{
		centre_frequency=(descr->centre_frequency1<<24)|
				(descr->centre_frequency2<<16)|
				(descr->centre_frequency3<<8)|
				(descr->centre_frequency4);
		centre_frequency*=10;
		bandwidth=descr->bandwidth;
		constellation=descr->constellation;
		hierarchy_information=descr->hierarchy_information;
		code_rate_hp_stream=descr->code_rate_hp_stream;
		code_rate_lp_stream=descr->code_rate_lp_stream;
		guard_interval=descr->guard_interval;
		transmission_mode=descr->transmission_mode;
		other_frequency_flag=descr->other_frequency_flag;
	}

TerrestrialDeliverySystemDescriptor::~TerrestrialDeliverySystemDescriptor()
{ 
}

#ifdef SUPPORT_XML
eString TerrestrialDeliverySystemDescriptor::toString()
{
	eString res=eString().sprintf("<TerrestrialDeliverySystemDescriptor><centre_frequency>%u</centre_frequency><bandwidth>", centre_frequency);
	switch (bandwidth)
	{
	case 0: res+="8MHz"; break;
	case 1: res+="7MHz"; break;
	case 2: res+="6MHz"; break;
	}
	res+="</bandwidth><constellation>";
	switch (constellation)
	{
		case 0: res+="QPSK"; break;
		case 1: res+="QAM16"; break;
		case 2: res+="QAM64"; break;
	} 
	res+="</constellation><hierarchy_information>";
	switch (hierarchy_information)
	{
		case 0: res+="none"; break;
		case 1: res+="1"; break;
		case 2: res+="2"; break;
		case 3: res+="3"; break;
	}
	res+="</hierarchy_information><code_rate_hp_stream>";
	switch (code_rate_hp_stream)
	{
		case 0: res+="1/2"; break;
		case 1: res+="2/3"; break;
		case 2: res+="3/4"; break;
		case 3: res+="5/6"; break;
		case 4: res+="7/8"; break;
	} 
	res+="</code_rate_hp_stream><code_rate_lp_stream>";
	switch (code_rate_lp_stream)
	{
		case 0: res+="1/2"; break;
		case 1: res+="2/3"; break;
		case 2: res+="3/4"; break;
		case 3: res+="5/6"; break;
		case 4: res+="7/8"; break;
	}
	res+="</code_rate_lp_stream><guard_interval>"; 
	switch (guard_interval)
	{
		case 0: res+="1/32"; break;
		case 1: res+="1/16"; break;
		case 2: res+="1/8"; break;
		case 3: res+="1/4"; break;
	}
	res+="</guard_interval><transmission_mode>";
	switch (transmission_mode)
	{
		case 0: res+="2k"; break;
		case 1: res+="8k"; break;
	}
	res+=eString().sprintf("</transmission_mode><other_frequency_flag>%d</other_frequency_flag></TerrestrialDeliverySystemDescriptor>", other_frequency_flag);
	return res;
}
#endif
									  
ServiceListDescriptorEntry::ServiceListDescriptorEntry(__u16 service_id, __u8 service_type)
	:service_id(service_id), service_type(service_type)
{
}

ServiceListDescriptorEntry::~ServiceListDescriptorEntry()
{
}

ServiceListDescriptor::ServiceListDescriptor(descr_gen_t *descr)
 :Descriptor(descr)
{
	entries.setAutoDelete(true);
	for (int i=0; i<(len-2); i+=3)
		entries.push_back(new ServiceListDescriptorEntry((((__u8*)(descr+1))[i]<<8) | (((__u8*)(descr+1))[i+1]), ((__u8*)(descr+1))[i+2]));
}

ServiceListDescriptor::~ServiceListDescriptor()
{
}

#ifdef SUPPORT_XML
eString ServiceListDescriptor::toString()
{
	eString res="<ServiceListDescriptor>";
	for (ePtrList<ServiceListDescriptorEntry>::iterator i(entries); i != entries.end(); ++i)
	{
		res+=eString().sprintf("<ServiceListDescriptorEntry>");
		res+=eString().sprintf("<service_id>%04x</service_id>", i->service_id);
		res+=eString().sprintf("<service_type>%04x</service_type>", i->service_type);
		res+="</ServiceListDescriptorEntry>";
	}
	res+="</ServiceListDescriptor>\n";
	return res;
}
#endif

ShortEventDescriptor::ShortEventDescriptor(descr_gen_t *descr, int tsidonid)
	:Descriptor(descr), tsidonid(tsidonid)
{
	init_ShortEventDescriptor(descr);
}
void ShortEventDescriptor::init_ShortEventDescriptor(descr_gen_t *descr)
{
	__u8 *data=(__u8*)descr;
	memcpy(language_code, data+2, 3);
	int ptr=5;
	int len=data[ptr++];

	int table=getEncodingTable(language_code);

	event_name.strReplace("\n", " - ");
	event_name=convertDVBUTF8((unsigned char*)data+ptr, len, table, tsidonid);
	// filter newlines in ARD ShortEventDescriptor event_name
	event_name.strReplace("\xc2\x8a",": ");
	ptr+=len;

	len=data[ptr++];
	text=convertDVBUTF8((unsigned char*) data+ptr, len, table, tsidonid);

	unsigned int start = text.find_first_not_of("\x0a");
	if (start > 0 && start != std::string::npos)
	{
		text.erase(0, start);
	}
}

#ifdef SUPPORT_XML
eString ShortEventDescriptor::toString()
{
	eString res="<ShortEventDescriptor>";
	res+="<event_name>"+event_name+"</event_name>";
	res+="<text>"+text+"</text>";
	res+="</ShortEventDescriptor>\n";
	return res;
}
#endif

ISO639LanguageDescriptor::ISO639LanguageDescriptor(descr_gen_t *descr)
	:Descriptor(descr)
{
	__u8 *data=(__u8*)descr;
	memcpy(language_code, data+2, 3);
	audio_type=data[5];
}

#ifdef SUPPORT_XML
eString ISO639LanguageDescriptor::toString()
{
	eString res;
	res+=eString().sprintf("<ISO639LangugageDescriptor>");
	res+=eString().sprintf("<language_code>%c%c%c</language_code>\n", language_code[0], language_code[1], language_code[2]);
	res+=eString().sprintf("<audio_type>%d</audio_type></ISO639LangugageDescriptor>\n", audio_type);
	return res;
}
#endif

AC3Descriptor::AC3Descriptor(descr_gen_t *descr)
	:Descriptor(descr)
{
	init_AC3Descriptor(descr);
}
void AC3Descriptor::init_AC3Descriptor(descr_gen_t *descr)
{
	__u8 *data=(__u8*)descr;
	data+=2;
	int flags=*data++;
	if (flags&0x80)
		AC3_type=*data++;
	else
		AC3_type=-1;
	if (flags&0x40)
		bsid=*data++;
	else
		bsid=-1;
	if (flags&0x20)
		mainid=*data++;
	else
		mainid=-1;
	if (flags&0x10)
		asvc=*data++;
	else
		asvc=-1;
}

#ifdef SUPPORT_XML
eString AC3Descriptor::toString()
{
	eString res="<AC3Descriptor>";
	if (AC3_type!=-1)
		res+=eString().sprintf("<AC3_type>%d</AC3_type>", AC3_type);
	if (bsid!=-1)
		res+=eString().sprintf("<bsid>%d</asvc>", bsid);
	if (mainid!=-1)
		res+=eString().sprintf("<mainid>%d</asvc>", mainid);
	if (asvc!=-1)
		res+=eString().sprintf("<asvc>%d</asvc>", asvc);
	res+="</AC3Descriptor>\n";
	return res;
}
#endif

BouquetNameDescriptor::BouquetNameDescriptor(descr_gen_t *descr)
	:Descriptor(descr)
{
	if (len > 2)
		name.assign((char*)(descr+1), len-2);
}

#ifdef SUPPORT_XML
eString BouquetNameDescriptor::toString()
{
	eString res="<BouquetNameDescriptor>";
	res+="<name>"+name+"</name></BouquetNameDescriptor>\n";
	return res;
}
#endif

ItemEntry::ItemEntry(eString &item_description, eString &item)
	:item_description(item_description), item(item)
{
}

ItemEntry::~ItemEntry()
{
}

ExtendedEventDescriptor::ExtendedEventDescriptor(descr_gen_t *descr, int tsidonid)
	:Descriptor(descr), tsidonid(tsidonid)
{
	init_ExtendedEventDescriptor(descr);
}
void ExtendedEventDescriptor::init_ExtendedEventDescriptor(descr_gen_t *descr)
{
	struct eit_extended_descriptor_struct *evt=(struct eit_extended_descriptor_struct *)descr;
	items.setAutoDelete(true);
	descriptor_number = evt->descriptor_number;
	last_descriptor_number = evt->last_descriptor_number;
	language_code[0]=evt->iso_639_2_language_code_1;
	language_code[1]=evt->iso_639_2_language_code_2;
	language_code[2]=evt->iso_639_2_language_code_3;

	int table=getEncodingTable(language_code);

	int ptr = sizeof(struct eit_extended_descriptor_struct);
	__u8* data = (__u8*) descr;

	int length_of_items=data[ptr++];
	int item_ptr=ptr;
	int item_description_len;
	int item_len;

	while (ptr < item_ptr+length_of_items)
	{
		eString item_description;
		eString item;

		item_description_len=data[ptr++];
		item_description=convertDVBUTF8((unsigned char*) data+ptr, item_description_len, table, tsidonid );
		ptr+=item_description_len;

		item_len=data[ptr++];
		item=convertDVBUTF8((unsigned char*) data+ptr, item_len, table, tsidonid);
		ptr+=item_len;

		items.push_back(new ItemEntry(item_description, item));
	}

	int text_length=data[ptr++];
	text=convertDVBUTF8((unsigned char*) data+ptr, text_length, table, tsidonid);
	ptr+=text_length;
}

#ifdef SUPPORT_XML
eString ExtendedEventDescriptor::toString()
{
	eString res="<ExtendedEventDescriptor>";
	res+=eString().sprintf("<language_code>%c%c%c</language_code>", language_code[0], language_code[1], language_code[2]);
	res+=eString().sprintf("<descriptor>%i</descriptor><last_descriptor_number>%i</last_descriptor_number>\n", descriptor_number, last_descriptor_number);

	for (ePtrList<ItemEntry>::iterator i(items); i != items.end(); ++i)
	{
		res+="<ItemEntry>";
		res+="<item_description>" + i->item_description + "</item_description>";
		res+="<item>" + i->item + "</item>";
		res+="</ItemEntry>";
	}
	res+="<text>"+text+"</text></ExtendedEventDescriptor>\n";
	return res;
}
#endif

ComponentDescriptor::ComponentDescriptor(descr_component_struct *descr, int tsidonid)
	:Descriptor((descr_gen_t*)descr)
{
	init_ComponentDescriptor(descr);
}
void ComponentDescriptor::init_ComponentDescriptor(descr_component_struct *descr)
{
	stream_content=descr->stream_content;
	component_type=descr->component_type;
	component_tag=descr->component_tag;
	language_code[0]=descr->lang_code1;
	language_code[1]=descr->lang_code2;
	language_code[2]=descr->lang_code3;

	int llen = len - sizeof(descr_component_struct); 
	if ( llen > 0 )
		text=convertDVBUTF8((unsigned char*)(descr+1), llen, getEncodingTable(language_code));
	
}

#ifdef SUPPORT_XML
eString ComponentDescriptor::toString()
{
	eString res="<ComponentDescriptor>";
	res+=eString().sprintf("<stream_content>%d</stream_content>", stream_content);
	res+=eString().sprintf("<component_type>%d</component_type>", component_type);
	res+=eString().sprintf("<component_tag>%d</component_tag>\n", component_tag);
	res+="<text>"+text+"</text></ComponentDescriptor>\n";
	return res;
}
#endif

ContentDescriptor::ContentDescriptor(descr_gen_t *descr)
	:Descriptor(descr)
{
	init_ContentDescriptor(descr);
}
void ContentDescriptor::init_ContentDescriptor(descr_gen_t *descr)
{
	contentList.setAutoDelete(true);
	__u8 *data=(__u8*)(descr+1);
	__u8 *work=data;

	while( work < (data+len-2) )
	{
		descr_content_entry_struct *tmp = new descr_content_entry_struct();
		memcpy(tmp, work, sizeof(descr_content_entry_struct) );
		contentList.push_back( tmp );
		work+=2;
	}
}

#ifdef SUPPORT_XML
eString ContentDescriptor::toString()
{
	eString res="<ContentDescriptor>";
	for (ePtrList<descr_content_entry_struct>::iterator it( contentList.begin() ); it != contentList.end(); it++)
		res+=eString().sprintf("nibble1 = %02x, nibble2 = %02x, user1 = %02x, user2 = %02x\n",
																	it->content_nibble_level_1, it->content_nibble_level_2, it->user_nibble_1, it->user_nibble_2 );
	res+="<!-- don't ask --></ContentDescriptor>\n";
	return res;
}	
#endif

LesRadiosDescriptor::LesRadiosDescriptor(descr_lesradios_struct *descr)
	:Descriptor((descr_gen_t*)descr)
{
	int llen=len;
	id=descr->id;
	llen-=sizeof(descr_lesradios_struct);
	if ( llen > 0 )
		name.assign((char*)(descr+1), llen);
}

#ifdef SUPPORT_XML
eString LesRadiosDescriptor::toString()
{
	eString res;
	res="<LesRadioDescriptor>";
	res+=eString().sprintf("<id>%d</id>", id);
	res+="<name>";
	res+=name;
	res+="</name></LesRadioDescriptor>\n";
	return res;
}
#endif

MHWDataDescriptor::MHWDataDescriptor(descr_mhw_data_struct *descr)
	:Descriptor((descr_gen_t*)descr)
{
	memcpy(type, descr->type, 8);
}

#ifdef SUPPORT_XML
eString MHWDataDescriptor::toString()
{
	eString res;
	res="<MHWDataDescriptor><data>";
	for (int i=0; i<8; i++)
		res+=type[i];
	res+="</data></MHWDataDescriptor>\n";
	return res;
}
#endif

ParentalRatingDescriptor::ParentalRatingDescriptor( descr_gen_struct *descr)
	: Descriptor((descr_gen_t*)descr)
{
	const char *data = (char*)(descr+1);
	const char *work = data;
	while( work < (data+len-2) )
	{
		entryMap[ eString(work, 3) ] = *(work+3)+3;
		work+=4;
	}
}

#ifdef SUPPORT_XML
eString ParentalRatingDescriptor::toString()
{
	eString res="<ParentalRatingDescriptor>";
	for ( std::map<eString,int>::iterator it(entryMap.begin()); it != entryMap.end(); it++)
	{
		res += eString().sprintf("<entry><country>%s</country><age>%i</age></entry>",it->first.c_str(), it->second);
	}
	res+="</ParentalRatingDescriptor>\n";
	return res;
}
#endif

RegistrationDescriptor::RegistrationDescriptor( descr_gen_struct *descr)
	: Descriptor(descr)
{
	const char *data = (char*)(descr+1);
	if (len < 6)
		return;
	memcpy(format_identifier, data, 4);
	additional_identification_info.assign(data+4, len-6);
}

#ifdef SUPPORT_XML
eString RegistrationDescriptor::toString()
{
	eString res="<RegistrationDescriptor><format_identifier>";
	res+=eString().assign(format_identifier, 4);
	res+="</format_identifier><additional_identification_info>";
	res+=additional_identification_info;
	res+="</additional_identification_info></RegistrationDescriptor>\n";
	return res;
}
#endif

SubtitleEntry::SubtitleEntry(__u8* data)
{
	memcpy(language_code, data, 3);
	subtitling_type = *(data+3);
	composition_page_id = (*(data+4) << 8) | *(data+5);
	ancillary_page_id = (*(data+6) << 8) | *(data+7);
}

SubtitlingDescriptor::SubtitlingDescriptor(descr_gen_struct *descr)
	:Descriptor(descr)
{
	entries.setAutoDelete(true);
	for (int i=2; i<len; i+=8)
		entries.push_back(new SubtitleEntry(((__u8*)descr)+i));
}

#ifdef SUPPORT_XML
eString SubtitlingDescriptor::toString()
{
	eString res;
	res += "<SubtitlingDescriptor>";
	for ( ePtrList<SubtitleEntry>::iterator it(entries.begin()); it != entries.end(); ++it )
		res+=eString().sprintf(
			"<ISO639_language_code>%c%c%c</ISO639_language_code>"
			"<subtitling_type>%d</subtitling_type>"
			"<composition_page_id>%d</composition_page_id>"
			"<ancillary_page_id>%d</ancillary_page_id",
			it->language_code[0], it->language_code[1], it->language_code[2],
			it->subtitling_type, it->composition_page_id, it->ancillary_page_id);
	res += "</SubtitlingDescriptor>";
	return res;
}
#endif

PrivateDataSpecifierDescriptor::PrivateDataSpecifierDescriptor(descr_gen_struct *descr)
	:Descriptor(descr), private_data_specifier(0)
{
	if (len == 6)
	{
		__u8 *data = (__u8*) (descr+1);
		private_data_specifier = data[3];
		private_data_specifier |= data[0] << 24;
		private_data_specifier |= data[1] << 16;
		private_data_specifier |= data[2] << 8;
	}
}

eString PrivateDataSpecifierDescriptor::toString()
{
	return eString().setNum(private_data_specifier);
}

#ifdef ENABLE_DISH_EPG

struct _table
{
	unsigned int  encoded_sequence;
	unsigned char character;
	unsigned char number_of_bits;
};

struct _table Table128[] =
{
    { 0x0000, 0x20, 0x03 },  // ' ' 
    { 0x0002, 0x65, 0x04 },  // 'e' 
    { 0x0003, 0x74, 0x04 },  // 't' 
    { 0x0004, 0x61, 0x04 },  // 'a' 
    { 0x0005, 0x6F, 0x04 },  // 'o' 
    { 0x0006, 0x73, 0x04 },  // 's' 
    { 0x0007, 0x6E, 0x04 },  // 'n' 
    { 0x0020, 0x72, 0x06 },  // 'r' 
    { 0x0021, 0x69, 0x06 },  // 'i' 
    { 0x0022, 0x6C, 0x06 },  // 'l' 
    { 0x0023, 0x63, 0x06 },  // 'c' 
    { 0x0024, 0x68, 0x06 },  // 'h' 
    { 0x0025, 0x75, 0x06 },  // 'u' 
    { 0x0026, 0x64, 0x06 },  // 'd' 
    { 0x0027, 0x70, 0x06 },  // 'p' 
    { 0x0028, 0x6D, 0x06 },  // 'm' 
    { 0x0029, 0x67, 0x06 },  // 'g' 
    { 0x002A, 0x79, 0x06 },  // 'y' 
    { 0x002B, 0x76, 0x06 },  // 'v' 
    { 0x002C, 0x0A, 0x06 },  // '''
    { 0x002D, 0x2E, 0x06 },  // '.' 
    { 0x002E, 0x77, 0x06 },  // 'w' 
    { 0x002F, 0x66, 0x06 },  // 'f' 
    { 0x0060, 0x53, 0x07 },  // 'S' 
    { 0x0061, 0x62, 0x07 },  // 'b' 
    { 0x0062, 0x54, 0x07 },  // 'T' 
    { 0x0063, 0x22, 0x07 },  // '"' 
    { 0x0064, 0x6B, 0x07 },  // 'k' 
    { 0x0065, 0x50, 0x07 },  // 'P' 
    { 0x0066, 0x41, 0x07 },  // 'A' 
    { 0x0067, 0x43, 0x07 },  // 'C' 
    { 0x0068, 0x44, 0x07 },  // 'D' 
    { 0x0069, 0x4C, 0x07 },  // 'L' 
    { 0x006A, 0x4D, 0x07 },  // 'M' 
    { 0x006B, 0x49, 0x07 },  // 'I' 
    { 0x006C, 0x4E, 0x07 },  // 'N' 
    { 0x006D, 0x3A, 0x07 },  // ':' 
    { 0x006E, 0x52, 0x07 },  // 'R' 
    { 0x006F, 0x2C, 0x07 },  // ',' 
    { 0x00E0, 0x45, 0x08 },  // 'E' 
    { 0x00E1, 0x55, 0x08 },  // 'U' 
    { 0x00E2, 0x46, 0x08 },  // 'F' 
    { 0x00E3, 0x48, 0x08 },  // 'H' 
    { 0x00E4, 0x59, 0x08 },  // 'Y' 
    { 0x00E5, 0x56, 0x08 },  // 'V' 
    { 0x00E6, 0x2D, 0x08 },  // '-' 
    { 0x00E7, 0x7A, 0x08 },  // 'z' 
    { 0x00E8, 0x78, 0x08 },  // 'x' 
    { 0x00E9, 0x2F, 0x08 },  // '/' 
    { 0x00EA, 0x4F, 0x08 },  // 'O' 
    { 0x00EB, 0x3F, 0x08 },  // '?' 
    { 0x00EC, 0x57, 0x08 },  // 'W' 
    { 0x00ED, 0x47, 0x08 },  // 'G' 
    { 0x00EE, 0x42, 0x08 },  // 'B' 
    { 0x00EF, 0x33, 0x08 },  // '3' 
    { 0x01E0, 0x31, 0x09 },  // '1' 
    { 0x01E1, 0x71, 0x09 },  // 'q' 
    { 0x01E2, 0x30, 0x09 },  // '0' 
    { 0x01E3, 0x21, 0x09 },  // '!' 
    { 0x01E4, 0x6A, 0x09 },  // 'j' 
    { 0x01E5, 0x5A, 0x09 },  // 'Z' 
    { 0x01E6, 0x39, 0x09 },  // '9' 
    { 0x01E7, 0x34, 0x09 },  // '4' 
    { 0x01E8, 0x4B, 0x09 },  // 'K' 
    { 0x01E9, 0x2A, 0x09 },  // '*' 
    { 0x01EA, 0x37, 0x09 },  // '7' 
    { 0x01EB, 0x36, 0x09 },  // '6' 
    { 0x01EC, 0x35, 0x09 },  // '5' 
    { 0x01ED, 0x4A, 0x09 },  // 'J' 
    { 0x01EE, 0x38, 0x09 },  // '8' 
    { 0x01EF, 0x29, 0x09 },  // ')' 
    { 0x03E0, 0x28, 0x0A },  // '(' 
    { 0x03E1, 0x58, 0x0A },  // 'X' 
    { 0x03E2, 0x51, 0x0A },  // 'Q' 
    { 0x03E3, 0x3C, 0x0A },  // '<' 
    { 0x03E4, 0x32, 0x0A },  // '2' 
    { 0x03E5, 0x27, 0x0A },  // ''' 
    { 0x03E6, 0x26, 0x0A },  // '&' 
    { 0x07CE, 0x20, 0x0B },  // '' 
    { 0x07CF, 0x7E, 0x0B },  // '~' 
    { 0x07D0, 0x7D, 0x0B },  // '}' 
    { 0x07D1, 0x7C, 0x0B },  // '|' 
    { 0x07D2, 0x7B, 0x0B },  // '{' 
    { 0x07D3, 0x60, 0x0B },  // '`' 
    { 0x07D4, 0x5F, 0x0B },  // '_' 
    { 0x07D5, 0x5E, 0x0B },  // '^' 
    { 0x07D6, 0x5D, 0x0B },  // ']' 
    { 0x07D7, 0x5C, 0x0B },  // '\' 
    { 0x07D8, 0x5B, 0x0B },  // '[' 
    { 0x07D9, 0x40, 0x0B },  // '@' 
    { 0x07DA, 0x3E, 0x0B },  // '>' 
    { 0x07DB, 0x3D, 0x0B },  // '=' 
    { 0x07DC, 0x3B, 0x0B },  // ';' 
    { 0x07DD, 0x2B, 0x0B },  // '+' 
    { 0x07DE, 0x25, 0x0B },  // '%' 
    { 0x07DF, 0x24, 0x0B },  // '$' 
    { 0x07E0, 0x23, 0x0B },  // '#' 
    { 0x07E1, 0x20, 0x0B },  // '' 
    { 0x07E2, 0x20, 0x0B },  // '' 
    { 0x07E3, 0x20, 0x0B },  // '' 
    { 0x07E4, 0x20, 0x0B },  // '' 
    { 0x07E5, 0x20, 0x0B },  // '' 
    { 0x07E6, 0x20, 0x0B },  // '' 
    { 0x07E7, 0x20, 0x0B },  // '' 
    { 0x07E8, 0x20, 0x0B },  // '' 
    { 0x07E9, 0x20, 0x0B },  // '' 
    { 0x07EA, 0x20, 0x0B },  // '' 
    { 0x07EB, 0x20, 0x0B },  // '' 
    { 0x07EC, 0x20, 0x0B },  // '' 
    { 0x07ED, 0x20, 0x0B },  // '' 
    { 0x07EE, 0x20, 0x0B },  // '' 
    { 0x07EF, 0x20, 0x0B },  // '' 
    { 0x07F0, 0x20, 0x0B },  // '' 
    { 0x07F1, 0x20, 0x0B },  // '' 
    { 0x07F2, 0x20, 0x0B },  // '' 
    { 0x07F3, 0x20, 0x0B },  // ''  
    { 0x07F4, 0x20, 0x0B },  // '' 
    { 0x07F5, 0x20, 0x0B },  // '' 
    { 0x07F6, 0x20, 0x0B },  // '' 
    { 0x07F7, 0x20, 0x0B },  // '' 
    { 0x07F8, 0x20, 0x0B },  // '' 
    { 0x07F9, 0x20, 0x0B },  // '' 
    { 0x07FA, 0x20, 0x0B },  // '' 
    { 0x07FB, 0x20, 0x0B },  // '' 
    { 0x07FC, 0x20, 0x0B },  // '' 
    { 0x07FD, 0x20, 0x0B },  // '' 
    { 0x07FE, 0x20, 0x0B },  // '' 
    { 0x07FF, 0x20, 0x0B },  // '' 
};

struct _table Table255[] =
{
    { 0x0000, 0x20, 0x02 },  // ' ' 
    { 0x0004, 0x65, 0x04 },  // 'e' 
    { 0x0005, 0x72, 0x04 },  // 'r' 
    { 0x0006, 0x6E, 0x04 },  // 'n' 
    { 0x0007, 0x61, 0x04 },  // 'a' 
    { 0x0010, 0x74, 0x05 },  // 't' 
    { 0x0011, 0x6F, 0x05 },  // 'o' 
    { 0x0012, 0x73, 0x05 },  // 's' 
    { 0x0013, 0x69, 0x05 },  // 'i' 
    { 0x0014, 0x6C, 0x05 },  // 'l' 
    { 0x0015, 0x75, 0x05 },  // 'u' 
    { 0x0016, 0x63, 0x05 },  // 'c' 
    { 0x0017, 0x64, 0x05 },  // 'd' 
    { 0x0060, 0x70, 0x07 },  // 'p' 
    { 0x0061, 0x6D, 0x07 },  // 'm' 
    { 0x0062, 0x76, 0x07 },  // 'v' 
    { 0x0063, 0x67, 0x07 },  // 'g' 
    { 0x0064, 0x68, 0x07 },  // 'h' 
    { 0x0065, 0x2E, 0x07 },  // '.' 
    { 0x0066, 0x66, 0x07 },  // 'f' 
    { 0x0067, 0x0A, 0x07 },  // '' 
    { 0x0068, 0x53, 0x07 },  // 'S' 
    { 0x0069, 0x41, 0x07 },  // 'A' 
    { 0x006A, 0x45, 0x07 },  // 'E' 
    { 0x006B, 0x43, 0x07 },  // 'C' 
    { 0x006C, 0x27, 0x07 },  // ''' 
    { 0x006D, 0x7A, 0x07 },  // 'z' 
    { 0x006E, 0x52, 0x07 },  // 'R' 
    { 0x006F, 0x22, 0x07 },  // '"' 
    { 0x00E0, 0x4C, 0x08 },  // 'L' 
    { 0x00E1, 0x49, 0x08 },  // 'I' 
    { 0x00E2, 0x4F, 0x08 },  // 'O' 
    { 0x00E3, 0x62, 0x08 },  // 'b' 
    { 0x00E4, 0x54, 0x08 },  // 'T' 
    { 0x00E5, 0x4E, 0x08 },  // 'N' 
    { 0x00E6, 0x55, 0x08 },  // 'U' 
    { 0x00E7, 0x79, 0x08 },  // 'y' 
    { 0x00E8, 0x44, 0x08 },  // 'D' 
    { 0x00E9, 0x50, 0x08 },  // 'P' 
    { 0x00EA, 0x71, 0x08 },  // 'q' 
    { 0x00EB, 0x56, 0x08 },  // 'V' 
    { 0x00EC, 0x2D, 0x08 },  // '-' 
    { 0x00ED, 0x3A, 0x08 },  // ':' 
    { 0x00EE, 0x2C, 0x08 },  // ',' 
    { 0x00EF, 0x48, 0x08 },  // 'H' 
    { 0x01E0, 0x4D, 0x09 },  // 'M' 
    { 0x01E1, 0x78, 0x09 },  // 'x' 
    { 0x01E2, 0x77, 0x09 },  // 'w' 
    { 0x01E3, 0x42, 0x09 },  // 'B' 
    { 0x01E4, 0x47, 0x09 },  // 'G' 
    { 0x01E5, 0x46, 0x09 },  // 'F' 
    { 0x01E6, 0x30, 0x09 },  // '0' 
    { 0x01E7, 0x3F, 0x09 },  // '?' 
    { 0x01E8, 0x33, 0x09 },  // '3' 
    { 0x01E9, 0x2F, 0x09 },  // '/' 
    { 0x01EA, 0x39, 0x09 },  // '9' 
    { 0x01EB, 0x31, 0x09 },  // '1' 
    { 0x01EC, 0x38, 0x09 },  // '8' 
    { 0x01ED, 0x6B, 0x09 },  // 'k' 
    { 0x01EE, 0x6A, 0x09 },  // 'j' 
    { 0x01EF, 0x21, 0x09 },  // '!' 
    { 0x03E0, 0x36, 0x0A },  // '6' 
    { 0x03E1, 0x35, 0x0A },  // '5' 
    { 0x03E2, 0x59, 0x0A },  // 'Y' 
    { 0x03E3, 0x51, 0x0A },  // 'Q' 
    { 0x07C8, 0x34, 0x0B },  // '4' 
    { 0x07C9, 0x58, 0x0B },  // 'X' 
    { 0x07CA, 0x32, 0x0B },  // '2' 
    { 0x07CB, 0x2B, 0x0B },  // '+' 
    { 0x07CC, 0x2A, 0x0B },  // '*' 
    { 0x07CD, 0x5A, 0x0B },  // 'Z' 
    { 0x07CE, 0x4A, 0x0B },  // 'J' 
    { 0x07CF, 0x29, 0x0B },  // ')' 
    { 0x0FA0, 0x28, 0x0C },  // '(' 
    { 0x0FA1, 0x23, 0x0C },  // '#' 
    { 0x0FA2, 0x57, 0x0C },  // 'W' 
    { 0x0FA3, 0x4B, 0x0C },  // 'K' 
    { 0x0FA4, 0x3C, 0x0C },  // '<' 
    { 0x0FA5, 0x37, 0x0C },  // '7' 
    { 0x0FA6, 0x7D, 0x0C },  // '}' 
    { 0x0FA7, 0x7B, 0x0C },  // '{' 
    { 0x0FA8, 0x60, 0x0C },  // '`' 
    { 0x0FA9, 0x26, 0x0C },  // '&' 
    { 0x1F54, 0xFE, 0x0D },  // 'þ' 
    { 0x1F55, 0xFD, 0x0D },  // 'ý' 
    { 0x1F56, 0xFC, 0x0D },  // 'ü' 
    { 0x1F57, 0xFB, 0x0D },  // 'û' 
    { 0x1F58, 0xFA, 0x0D },  // 'ú' 
    { 0x1F59, 0xF9, 0x0D },  // 'ù' 
    { 0x1F5A, 0xF8, 0x0D },  // 'ø' 
    { 0x1F5B, 0xF7, 0x0D },  // '÷' 
    { 0x1F5C, 0xF6, 0x0D },  // 'ö' 
    { 0x1F5D, 0xF5, 0x0D },  // 'õ' 
    { 0x1F5E, 0xF4, 0x0D },  // 'ô' 
    { 0x1F5F, 0xF3, 0x0D },  // 'ó' 
    { 0x1F60, 0xF2, 0x0D },  // 'ò' 
    { 0x1F61, 0xF1, 0x0D },  // 'ñ' 
    { 0x1F62, 0xF0, 0x0D },  // 'ð' 
    { 0x1F63, 0xEF, 0x0D },  // 'ï' 
    { 0x1F64, 0xEE, 0x0D },  // 'î' 
    { 0x1F65, 0xED, 0x0D },  // 'í' 
    { 0x1F66, 0xEC, 0x0D },  // 'ì' 
    { 0x1F67, 0xEB, 0x0D },  // 'ë' 
    { 0x1F68, 0xEA, 0x0D },  // 'ê' 
    { 0x1F69, 0xE9, 0x0D },  // 'é' 
    { 0x1F6A, 0xE8, 0x0D },  // 'è' 
    { 0x1F6B, 0xE7, 0x0D },  // 'ç' 
    { 0x1F6C, 0xE6, 0x0D },  // 'æ' 
    { 0x1F6D, 0xE5, 0x0D },  // 'å' 
    { 0x1F6E, 0xE4, 0x0D },  // 'ä' 
    { 0x1F6F, 0xE3, 0x0D },  // 'ã' 
    { 0x1F70, 0xE2, 0x0D },  // 'â' 
    { 0x1F71, 0xE1, 0x0D },  // 'á' 
    { 0x1F72, 0xE0, 0x0D },  // 'à' 
    { 0x1F73, 0xDF, 0x0D },  // 'ß' 
    { 0x1F74, 0xDE, 0x0D },  // 'Þ' 
    { 0x1F75, 0xDD, 0x0D },  // 'Ý' 
    { 0x1F76, 0xDC, 0x0D },  // 'Ü' 
    { 0x1F77, 0xDB, 0x0D },  // 'Û' 
    { 0x1F78, 0xDA, 0x0D },  // 'Ú' 
    { 0x1F79, 0xD9, 0x0D },  // 'Ù' 
    { 0x1F7A, 0xD8, 0x0D },  // 'Ø' 
    { 0x1F7B, 0xD7, 0x0D },  // '×' 
    { 0x1F7C, 0xD6, 0x0D },  // 'Ö' 
    { 0x1F7D, 0xD5, 0x0D },  // 'Õ' 
    { 0x1F7E, 0xD4, 0x0D },  // 'Ô' 
    { 0x1F7F, 0xD3, 0x0D },  // 'Ó' 
    { 0x1F80, 0xD2, 0x0D },  // 'Ò' 
    { 0x1F81, 0xD1, 0x0D },  // '' 
    { 0x1F82, 0xD0, 0x0D },  // '' 
    { 0x1F83, 0xCF, 0x0D },  // '' 
    { 0x1F84, 0xCE, 0x0D },  // '' 
    { 0x1F85, 0xCD, 0x0D },  // '' 
    { 0x1F86, 0xCC, 0x0D },  // '' 
    { 0x1F87, 0xCB, 0x0D },  // '' 
    { 0x1F88, 0xCA, 0x0D },  // '' 
    { 0x1F89, 0xC9, 0x0D },  // '' 
    { 0x1F8A, 0xC8, 0x0D },  // '' 
    { 0x1F8B, 0xC7, 0x0D },  // '' 
    { 0x1F8C, 0xC6, 0x0D },  // '' 
    { 0x1F8D, 0xC5, 0x0D },  // '' 
    { 0x1F8E, 0xC4, 0x0D },  // '' 
    { 0x1F8F, 0xC3, 0x0D },  // '' 
    { 0x1F90, 0xC2, 0x0D },  // '' 
    { 0x1F91, 0xC1, 0x0D },  // '' 
    { 0x1F92, 0xC0, 0x0D },  // '' 
    { 0x1F93, 0xBF, 0x0D },  // '' 
    { 0x1F94, 0xBE, 0x0D },  // '' 
    { 0x1F95, 0xBD, 0x0D },  // '' 
    { 0x1F96, 0xBC, 0x0D },  // '' 
    { 0x1F97, 0xBB, 0x0D },  // '' 
    { 0x1F98, 0xBA, 0x0D },  // '' 
    { 0x1F99, 0xB9, 0x0D },  // '' 
    { 0x1F9A, 0xB8, 0x0D },  // '' 
    { 0x1F9B, 0xB7, 0x0D },  // '' 
    { 0x1F9C, 0xB6, 0x0D },  // '' 
    { 0x1F9D, 0xB5, 0x0D },  // '' 
    { 0x1F9E, 0xB4, 0x0D },  // '' 
    { 0x1F9F, 0xB3, 0x0D },  // '' 
    { 0x1FA0, 0xB2, 0x0D },  // '' 
    { 0x1FA1, 0xB1, 0x0D },  // '' 
    { 0x1FA2, 0xB0, 0x0D },  // '' 
    { 0x1FA3, 0xAF, 0x0D },  // '' 
    { 0x1FA4, 0xAE, 0x0D },  // '' 
    { 0x1FA5, 0xAD, 0x0D },  // '' 
    { 0x1FA6, 0xAC, 0x0D },  // '' 
    { 0x1FA7, 0xAB, 0x0D },  // '' 
    { 0x1FA8, 0xAA, 0x0D },  // '' 
    { 0x1FA9, 0xA9, 0x0D },  // '' 
    { 0x1FAA, 0xA8, 0x0D },  // '' 
    { 0x1FAB, 0xA7, 0x0D },  // '' 
    { 0x1FAC, 0xA6, 0x0D },  // '' 
    { 0x1FAD, 0xA5, 0x0D },  // '' 
    { 0x1FAE, 0xA4, 0x0D },  // '' 
    { 0x1FAF, 0xA3, 0x0D },  // '' 
    { 0x1FB0, 0xA2, 0x0D },  // '' 
    { 0x1FB1, 0xA1, 0x0D },  // '' 
    { 0x1FB2, 0xA0, 0x0D },  // '' 
    { 0x1FB3, 0x9F, 0x0D },  // '' 
    { 0x1FB4, 0x9E, 0x0D },  // '' 
    { 0x1FB5, 0x9D, 0x0D },  // '' 
    { 0x1FB6, 0x9C, 0x0D },  // '' 
    { 0x1FB7, 0x9B, 0x0D },  // '' 
    { 0x1FB8, 0x9A, 0x0D },  // '' 
    { 0x1FB9, 0x99, 0x0D },  // '' 
    { 0x1FBA, 0x98, 0x0D },  // '' 
    { 0x1FBB, 0x97, 0x0D },  // '' 
    { 0x1FBC, 0x96, 0x0D },  // '' 
    { 0x1FBD, 0x95, 0x0D },  // '' 
    { 0x1FBE, 0x94, 0x0D },  // '' 
    { 0x1FBF, 0x93, 0x0D },  // '' 
    { 0x1FC0, 0x92, 0x0D },  // '' 
    { 0x1FC1, 0x91, 0x0D },  // '' 
    { 0x1FC2, 0x90, 0x0D },  // '' 
    { 0x1FC3, 0x8F, 0x0D },  // '' 
    { 0x1FC4, 0x8E, 0x0D },  // '' 
    { 0x1FC5, 0x8D, 0x0D },  // '' 
    { 0x1FC6, 0x8C, 0x0D },  // '' 
    { 0x1FC7, 0x8B, 0x0D },  // '' 
    { 0x1FC8, 0x8A, 0x0D },  // '' 
    { 0x1FC9, 0x89, 0x0D },  // '' 
    { 0x1FCA, 0x88, 0x0D },  // '' 
    { 0x1FCB, 0x87, 0x0D },  // '' 
    { 0x1FCC, 0x86, 0x0D },  // '' 
    { 0x1FCD, 0x85, 0x0D },  // '' 
    { 0x1FCE, 0x84, 0x0D },  // '' 
    { 0x1FCF, 0x83, 0x0D },  // '' 
    { 0x1FD0, 0x82, 0x0D },  // '' 
    { 0x1FD1, 0x81, 0x0D },  // '' 
    { 0x1FD2, 0x80, 0x0D },  // '' 
    { 0x1FD3, 0x7F, 0x0D },  // '' 
    { 0x1FD4, 0x7E, 0x0D },  // '' 
    { 0x1FD5, 0x7C, 0x0D },  // '' 
    { 0x1FD6, 0x5F, 0x0D },  // '' 
    { 0x1FD7, 0x5E, 0x0D },  // '' 
    { 0x1FD8, 0x5D, 0x0D },  // '' 
    { 0x1FD9, 0x5C, 0x0D },  // '' 
    { 0x1FDA, 0x5B, 0x0D },  // '' 
    { 0x1FDB, 0x40, 0x0D },  // '' 
    { 0x1FDC, 0x3E, 0x0D },  // '' 
    { 0x1FDD, 0x3D, 0x0D },  // '' 
    { 0x1FDE, 0x3B, 0x0D },  // '' 
    { 0x1FDF, 0x25, 0x0D },  // '' 
    { 0x1FE0, 0x24, 0x0D },  // '' 
    { 0x1FE1, 0x1F, 0x0D },  // '' 
    { 0x1FE2, 0x1E, 0x0D },  // '' 
    { 0x1FE3, 0x1D, 0x0D },  // '' 
    { 0x1FE4, 0x1C, 0x0D },  // '' 
    { 0x1FE5, 0x1B, 0x0D },  // '' 
    { 0x1FE6, 0x1A, 0x0D },  // '' 
    { 0x1FE7, 0x19, 0x0D },  // '' 
    { 0x1FE8, 0x18, 0x0D },  // '' 
    { 0x1FE9, 0x17, 0x0D },  // '' 
    { 0x1FEA, 0x16, 0x0D },  // '' 
    { 0x1FEB, 0x15, 0x0D },  // '' 
    { 0x1FEC, 0x14, 0x0D },  // '' 
    { 0x1FED, 0x13, 0x0D },  // '' 
    { 0x1FEE, 0x12, 0x0D },  // '' 
    { 0x1FEF, 0x11, 0x0D },  // '' 
    { 0x1FF0, 0x10, 0x0D },  // '' 
    { 0x1FF1, 0x0F, 0x0D },  // '' 
    { 0x1FF2, 0x0E, 0x0D },  // '' 
    { 0x1FF3, 0x0D, 0x0D },  // '' 
    { 0x1FF4, 0x0C, 0x0D },  // '' 
    { 0x1FF5, 0x0B, 0x0D },  // '' 
    { 0x1FF6, 0x09, 0x0D },  // '' 
    { 0x1FF7, 0x08, 0x0D },  // '' 
    { 0x1FF8, 0x07, 0x0D },  // '' 
    { 0x1FF9, 0x06, 0x0D },  // '' 
    { 0x1FFA, 0x05, 0x0D },  // '' 
    { 0x1FFB, 0x04, 0x0D },  // '' 
    { 0x1FFC, 0x03, 0x0D },  // '' 
    { 0x1FFD, 0x02, 0x0D },  // '' 
    { 0x1FFE, 0x01, 0x0D },  // '' 
    { 0x1FFF, 0x00, 0x0D }  // '' 
};

//
// returns the bit value of any bit in the byte array 
//
// for example 3 byte array the bits are as follows
// [0][1][2][3][4][5][6][7]   [8][9][10][11][12][13][14][15] [16][17][18][19][20][21][22][23]
//

int get_bit(int bit_index, unsigned char *byteptr)
{
	int byte_offset;
	int bit_number;
	
	byte_offset = bit_index / 8;
	bit_number = bit_index - (byte_offset * 8);

	if (byteptr[byte_offset] & (1 << (7 - bit_number)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//
//  returns the value of a sequence of bits in the byte array
//
unsigned int get_bits(int bit_index, int bit_count, unsigned char *byteptr)
{
	int i;
	unsigned int bits = 0;
    
	for (i = 0 ; i < bit_count ; i++)
	{
		bits = (bits << 1) | get_bit(bit_index + i, byteptr);
	}

	return bits;
}

//
//  decompress the byte arrary and returns the result to a text string
//
int dishDecompress(unsigned char *compressed, int length, int table, eString &result)
{
	int             i;
	int             total_bits = 0;
	int             current_bit = 0;
	unsigned int    bits;
	int             table_size;
	struct _table 	*ptrTable;

	result = "";

	if ( table == 1 )
	{
		table_size = sizeof(Table128) / sizeof(_table);
		ptrTable   = Table128;
	}
	else
	{
		table_size = sizeof(Table255) / sizeof(_table);
		ptrTable   = Table255;
	}

	total_bits = length * 8;
	// walk thru all the bits in the byte array, finding each sequence in the
	// list and decoding it to a character.
	while ( current_bit < total_bits - 3 )
	{
		// starting from the current bit
		// try to find the sequence in the decode list

		for ( i = 0; i < table_size; i++ )
		{
			bits = get_bits( current_bit, ptrTable[i].number_of_bits, compressed );
			if ( bits == ptrTable[i].encoded_sequence )
			{
				result += ptrTable[i].character;
				current_bit += ptrTable[i].number_of_bits;
				break;
			}
		}

		if (i == table_size) // if we get here then the bit sequence was not found ... problem try to recover
		{
			++current_bit;
		}
	}
	return 0;
}

DishEventNameDescriptor::DishEventNameDescriptor(descr_gen_t *descr, int type)
	:Descriptor(descr)
{
	/*
	 * HACK HACK: we use the EPG source field from eEPGCache (aka type),
	 * to determine which decryption table we should use.
	 * type: 0 = now/next, 1 = schedule (both pid 0x12) , 3 = dish EEPG (pid 0x300)
	 */
	int tableid = (type > 1) ? 2 : 1;
	__u8 *data = (__u8*)descr;
	if (tableid != 1)
	{
		int disheepg = 0;
		eConfig::getInstance()->getKey("/extras/disheepg", disheepg);
		if (!disheepg)
		{
			/* we don't have EEPG, so this descriptor SHOULD use table 1 */
			eDebug("Force Dish table from %d to 1, because of disheepg setting", tableid);
			tableid = 1;
		}
	}
	dishDecompress(&data[3], data[1] - 1, tableid, event_name);
}

DishEventNameDescriptor::~DishEventNameDescriptor()
{
}

#ifdef SUPPORT_XML
eString DishEventNameDescriptor::toString()
{
	eString res = "";
	return res;
}
#endif

DishDescriptionDescriptor::DishDescriptionDescriptor(descr_gen_t *descr, int type, int tsidonid)
	: ShortEventDescriptor((int)descr->descriptor_length, tsidonid)
{
	/*
	 * HACK HACK: we use the EPG source field from eEPGCache (aka type),
	 * to determine which decryption table we should use.
	 * type: 0 = now/next, 1 = schedule (both pid 0x12) , 3 = dish EEPG (pid 0x300)
	 */
	int tableid = (type > 1) ? 2 : 1;
	__u8 *data = (__u8*)descr;
	unsigned char *ptr;
	int len;
	if (tableid != 1)
	{
		int disheepg = 0;
		eConfig::getInstance()->getKey("/extras/disheepg", disheepg);
		if (!disheepg)
		{
			/* we don't have EEPG, so this descriptor SHOULD use table 1 */
			eDebug("Force Dish table from %d to 1, because of disheepg setting", tableid);
			tableid = 1;
		}
	}
	if ((data[3]&0xf8) == 0x80)
	{
		ptr = &data[4];
		len = data[1] - 2;
	}
	else
	{
		ptr = &data[3];
		len = data[1] - 1;
	}

	dishDecompress(ptr, len, tableid, text);

	text.removeChars('\x0d');
	language_code[0] = 'e';
	language_code[1] = 'n';
	language_code[2] = 'g';
}

DishDescriptionDescriptor::~DishDescriptionDescriptor()
{
}

#endif /* ENABLE_DISH_EPG */

PAT::PAT()
	:eTable(PID_PAT, TID_PAT)
{
	entries.setAutoDelete(true);
}

int PAT::data(__u8* data)
{
	pat_t &pat=*(pat_t*)data;
	if (pat.table_id!=TID_PAT)
		return -1;
	int slen=HILO(pat.section_length)+3;
	transport_stream_id=HILO(pat.transport_stream_id);
	
	pat_prog_t *prog=(pat_prog_t*)(data+PAT_LEN);
	
	for (int ptr=PAT_LEN; ptr<slen-4; ptr+=PAT_PROG_LEN, prog++)
		entries.push_back(new PATEntry(HILO(prog->program_number), HILO(prog->network_pid)));
	return 0;
}

__u8 *PAT::getRAW()
{
	__u8 *data = new __u8[4096];
	int slen = PAT_LEN;  // 8
	data[0] = 0x00;                      // table ID;
	data[3] = (transport_stream_id >> 8);// tsid hi
	data[4] = transport_stream_id & 0xFF;// tsid lo
	data[5] = version;                   // version,cur/next
	data[6] = 0;                         // section no
	data[7] = 0;                         // last section no
	for ( ePtrList<PATEntry>::iterator it(entries);
		it != entries.end(); ++it)
	{
		data[slen++] = it->program_number >> 8;
		data[slen++] = it->program_number & 0xFF;
		data[slen++] = 0xE0 | (it->program_map_PID >> 8);
		data[slen++] = it->program_map_PID & 0xFF;
	}
	data[1] = 0xB0 | ((slen-3+4) >> 8);   // section length hi
	data[2] = (slen-3+4) & 0xFF;          // section length lo

	unsigned int crc32 = crc32_be(~0, data, slen);

	data[slen++] = crc32 >> 24;
	data[slen++] = crc32 >> 16;
	data[slen++] = crc32 >> 8;
	data[slen++] = crc32 & 0xFF;

	return data;
}

SDTEntry::SDTEntry(sdt_descr_t *descr, int tsidonid)
{
	init_SDTEntry(descr,tsidonid);
}	
void SDTEntry::init_SDTEntry(sdt_descr_t *descr, int tsidonid)
{
	descriptors.setAutoDelete(true);
	service_id=HILO(descr->service_id);
	EIT_schedule_flag=descr->EIT_schedule_flag;
	EIT_present_following_flag=descr->EIT_present_following_flag;
	running_status=descr->running_status;
	free_CA_mode=descr->free_ca_mode;
	int dlen=HILO(descr->descriptors_loop_length)+SDT_DESCR_LEN;
	int ptr=SDT_DESCR_LEN;
	while (ptr<dlen)
	{
		descr_gen_t *d=(descr_gen_t*)(((__u8*)descr)+ptr);
		descriptors.push_back(Descriptor::create(d,tsidonid));
		ptr+=d->descriptor_length+2;
	}
}

SDT::SDT(int type, int tsid)
	:eTable(PID_SDT, type == 1 ? TID_SDT_OTH : TID_SDT_ACT,
	type == typeBoth ? 0xFB : 0xFF, tsid, -1)
{
	entries.setAutoDelete(true);
}

int SDT::data(__u8 *data)
{
	sdt_t &sdt=*(sdt_t*)data;
	int slen=HILO(sdt.section_length)+3;
	transport_stream_id=HILO(sdt.transport_stream_id);
	original_network_id=HILO(sdt.original_network_id);
	
	int ptr=SDT_LEN;
	while (ptr<slen-4)
	{
		sdt_descr_t *descr=(sdt_descr_t*)(data+ptr);
		entries.push_back(new SDTEntry(descr,(transport_stream_id<<16)|original_network_id));
		int dlen=HILO(descr->descriptors_loop_length);
		ptr+=SDT_DESCR_LEN+dlen;
	}
	if (ptr != (slen-4))
		return -1;
	
	return 0;
}

PMTEntry::PMTEntry(pmt_info_t* info)
{
	init_PMTEntry(info);
}
void PMTEntry::init_PMTEntry(pmt_info_t* info)
{
	ES_info.setAutoDelete(true);
	stream_type=info->stream_type;
	elementary_PID=HILO(info->elementary_PID);
	int elen=HILO(info->ES_info_length);
	int ptr=0;
	while (ptr<elen)
	{
		descr_gen_t *d=(descr_gen_t*)(((__u8*)info)+PMT_info_LEN+ptr);
		ES_info.push_back(Descriptor::create(d));
		ptr+=d->descriptor_length+2;
	}
}

PMT::PMT(int pid, int service_id, int version)
	:eTable(pid, TID_PMT, service_id, version), pid(pid)
{
	program_info.setAutoDelete(true);
	streams.setAutoDelete(true);
}

PMT::~PMT()
{
	for(ePtrList<__u8>::iterator i(program_infoPlain); i!=program_infoPlain.end(); ++i )
		delete [] *i;
	for(ePtrList<__u8>::iterator i(streamsPlain); i!=streamsPlain.end(); ++i )
		delete [] *i;
}

__u8* PMT::getRAW()
{
	__u8 *data = new __u8[4096];
	int slen = PMT_LEN;   // 12
	data[0] = 0x02;                      // table ID;
	data[3] = (program_number >> 8);     // prog_no hi
	data[4] = program_number & 0xFF;     // prog_no lo
	data[5] = version;                   // version,cur/next
	data[6] = 0;                         // section no
	data[7] = 0;                         // last section no
	data[8] = 0xE0 | (PCR_PID >> 8);     // PCR hi
	data[9] = PCR_PID & 0xFF;            // PCR lo
	int prog_info_len=0;
	for ( ePtrList<__u8>::iterator it(program_infoPlain);
		it != program_infoPlain.end(); ++it)
	{
		descr_gen_t *d=(descr_gen_t*)(*it);
		int len = d->descriptor_length+2;
		memcpy(data+slen, d, len);
		prog_info_len+=len;
		slen+=len;
	}
	data[10] = 0xF0 | (prog_info_len>>8); // prog_info len hi
	data[11] = prog_info_len&0xFF;        // prog_info len lo
	for ( ePtrList<__u8>::iterator it(streamsPlain);
		it != streamsPlain.end(); ++it)
	{
		int len = HILO(((pmt_info_t*)(*it))->ES_info_length)+PMT_info_LEN;
		memcpy(data+slen,*it, len);
		slen+=len;
	}
	data[1] = 0xB0 | ((slen-3+4) >> 8);   // section length hi
	data[2] = (slen-3+4) & 0xFF;          // section length lo

	unsigned int crc32 = crc32_be(~0, data, slen);

	data[slen++] = crc32 >> 24;
	data[slen++] = crc32 >> 16;
	data[slen++] = crc32 >> 8;
	data[slen++] = crc32 & 0xFF;

	return data;
}

eTable *PMT::createNext()
{
	if ( eSystemInfo::getInstance()->hasNegFilter() )
	{
		eDebug("PMT version = %d", version_number);
		return new PMT(pid, tableidext, version_number);
	}
	int newversion = incrementVersion(version);
	eDebug("PMT oldversion=%d, newversion=%d",version, newversion);
	return new PMT(pid, tableidext, newversion);
}

int PMT::data(__u8 *data)
{
	pmt_struct *pmt=(pmt_struct*)data;
	version_number=pmt->version_number;
	program_number=HILO(pmt->program_number);
	PCR_PID=HILO(pmt->PCR_PID);

	int program_info_len=HILO(pmt->program_info_length);
	int len=HILO(pmt->section_length)+3-4;
	if ( len < 0 )
		len=0;
	int ptr=PMT_LEN;
	while (ptr<(program_info_len+PMT_LEN))
	{
		descr_gen_t *d=(descr_gen_t*)(data+ptr);
		int len = (__u8)d->descriptor_length;
		len+=2;
		program_info.push_back(Descriptor::create(d));

		// store plain data
		__u8 *plain = new __u8[len];
		memcpy(plain, data+ptr, len);
		program_infoPlain.push_back(plain);

		ptr+=len;
	}
	while (ptr<len)
	{
		int len = HILO(((pmt_info_t*)(data+ptr))->ES_info_length)+PMT_info_LEN;

		streams.push_back(new PMTEntry((pmt_info_t*)(data+ptr)));

		// store plain data
		__u8 *plain = new __u8[len];
		memcpy(plain, data+ptr, len);
		streamsPlain.push_back(plain);

		ptr+=len;
	}
	return ptr!=len;
}

NITEntry::NITEntry(nit_ts_t* ts)
{
	init_NITEntry(ts);
}
void NITEntry::init_NITEntry(nit_ts_t* ts)
{
	transport_descriptor.setAutoDelete(true);
	transport_stream_id=HILO(ts->transport_stream_id);
	original_network_id=HILO(ts->original_network_id);
	int elen=HILO(ts->transport_descriptors_length);
	int ptr=0;
	while (ptr<elen)
	{
		descr_gen_t *d=(descr_gen_t*)(((__u8*)ts)+NIT_TS_LEN+ptr);
		transport_descriptor.push_back(Descriptor::create(d,(transport_stream_id<<16)|original_network_id));
		ptr+=d->descriptor_length+2;
  }
}

NIT::NIT(int pid, int type)
	:eTable(pid, type?TID_NIT_OTH:TID_NIT_ACT)
{
	entries.setAutoDelete(true);
	network_descriptor.setAutoDelete(true);
}

int NIT::data(__u8* data)
{
	nit_t *nit=(nit_t*)data;
	network_id=HILO(nit->network_id);
	int network_descriptor_len=HILO(nit->network_descriptor_length);
	int len=HILO(nit->section_length)+3-4;
	int ptr=NIT_LEN;
	while (ptr<(network_descriptor_len+NIT_LEN))
	{
		descr_gen_t *d=(descr_gen_t*)(data+ptr);
		network_descriptor.push_back(Descriptor::create(d));
		ptr+=d->descriptor_length+2;
	}
	ptr+=2;
	while (ptr<len)
	{
		entries.push_back(new NITEntry((nit_ts_t*)(data+ptr)));
		ptr+=HILO(((nit_ts_t*)(data+ptr))->transport_descriptors_length)+NIT_TS_LEN;
	}
	return ptr!=len;
}

EITEvent::EITEvent(const eit_event_struct *event, int tsidonid, int type)
{
	init_EITEvent(event, tsidonid);
}
void EITEvent::init_EITEvent(const eit_event_struct *event, int tsidonid)
{
	descriptor.setAutoDelete(true);
	event_id=HILO(event->event_id);
	if (event->start_time_5!=0xFF)
		start_time=parseDVBtime(event->start_time_1, event->start_time_2, event->start_time_3,event->start_time_4, event->start_time_5);
	else
		start_time=-1;
	if ((event->duration_1==0xFF) || (event->duration_2==0xFF) || (event->duration_3==0xFF))
		duration=-1;
	else
		duration=fromBCD(event->duration_1)*3600+fromBCD(event->duration_2)*60+fromBCD(event->duration_3);
	running_status=event->running_status;
	free_CA_mode=event->free_CA_mode;
	int ptr=0;
	int len=HILO(event->descriptors_loop_length);
	ShortEventDescriptor *sdescr = NULL;
#ifdef ENABLE_DISH_EPG
	DishEventNameDescriptor *ndescr = NULL;
#endif
	while (ptr<len)
	{
		descr_gen_t *d=(descr_gen_t*) (((__u8*)(event+1))+ptr);
		Descriptor *descr = Descriptor::create(d,tsidonid);
		descriptor.push_back(descr);
// HACK
		if ( descr->Tag() == DESCR_SHORT_EVENT )
		{
			sdescr = (ShortEventDescriptor*)descr;
#ifdef ENABLE_DISH_EPG
			//eDebug("EITEvent: short event, name: %s, description: %s\n", sdescr->event_name.c_str(), sdescr->text.c_str());
			if (ndescr)
			{
				/* we already have a dish event name descriptor, use it's name */
				sdescr->event_name = ndescr->event_name;
				ndescr->event_name.clear();
				//eDebug("EITEvent: completed short event, name: %s, description: %s\n", sdescr->event_name.c_str(), sdescr->text.c_str());
				ndescr = NULL;
			}
#endif
		}
		else if ( descr->Tag() == DESCR_EXTENDED_EVENT && sdescr )
		{
			ExtendedEventDescriptor *edescr = (ExtendedEventDescriptor*) descr;
			unsigned int s1 = sdescr->text.size();
			unsigned int s2 = edescr->text.size();
			if ( s2 && !strncmp( sdescr->text.c_str(), edescr->text.c_str(), s2 < s1 ? s2 : s1 ) )
				sdescr->text.clear();
			sdescr = NULL;
			//eDebug("EITEvent: extended event, text: %s\n", edescr->text.c_str());
		}
#ifdef ENABLE_DISH_EPG
		else if (descr->Tag() == DESCR_DISH_EVENT_NAME)
		{
			ndescr = (DishEventNameDescriptor*)descr;
			//eDebug("EITEvent: dish event name: %s\n", ndescr->event_name.c_str());
			if (sdescr)
			{
				/* we already have a short event descriptor, fill in the name */
				sdescr->event_name = ndescr->event_name;
				ndescr->event_name.clear();
				//eDebug("EITEvent: completed short event, name: %s, description: %s\n", sdescr->event_name.c_str(), sdescr->text.c_str());
				sdescr = NULL;
			}
		}
#endif
///
		ptr+=d->descriptor_length+2;
	}
}

EITEvent::EITEvent()
{
	descriptor.setAutoDelete(true);
}

int EIT::data(__u8 *data)
{
	eit_t *eit=(eit_t*)data;
	service_id=HILO(eit->service_id);
	version_number=eit->version_number;
	current_next_indicator=eit->current_next_indicator;
	transport_stream_id=HILO(eit->transport_stream_id);
	original_network_id=HILO(eit->original_network_id);
	int len=HILO(eit->section_length)+3-4;
	int ptr=EIT_SIZE;
      
	//
	// This fixed the EPG on the Multichoice irdeto systems
	// the EIT packet is non-compliant.. their EIT packet stinks
	if (data[ptr-1] < 0x40)
		--ptr;

	while (ptr<len)
	{
		int evLength=HILO(((eit_event_struct*)(data+ptr))->
			descriptors_loop_length)+EIT_LOOP_SIZE;

		events.push_back(new EITEvent((eit_event_struct*)(data+ptr), (transport_stream_id<<16)|original_network_id, type));

		// store plain data
		__u8 *plain = new __u8[evLength];
		memcpy(plain, data+ptr, evLength);
		eventsPlain.push_back(plain);

		ptr+=evLength;
	}
	return ptr!=len;
}

EIT::EIT(int type, int service_id, int ts, int version)
	:eTable(PID_EIT, ts?TID_EIT_OTH:TID_EIT_ACT,
#ifdef ENABLE_DISH_EPG
	/*
	 * Dish networks appears to sends now/next for the current serviceid in the 'other' table (4F)
	 * This should not cause any harm for non-Dish, as long as we're filtering on serviceid.
	 * (the same serviceid is unlikely to be used multiple times by the same provider)
	 * If we're not filtering on a specific serviceid, it's not a good idea to accept both 4E and 4F.
	 */
	(service_id != -1) ? 0xFE : 0xFF,
#endif
	service_id, version)
	,type(type), ts(ts)
{
	events.setAutoDelete(true);
}

EIT::EIT()
{
	events.setAutoDelete(true);
}

EIT::EIT(const EIT* eit)
{
	// Vorsicht !! Hier wird autoDelete nicht auf true gesetzt...
	// Lebenszeit der Source EIT beachten !
	if (eit)
	{
		current_next_indicator = eit->current_next_indicator;
		events = eit->events;
		original_network_id = eit->original_network_id;
		service_id = eit->service_id;
		transport_stream_id = eit->transport_stream_id;
		ts = eit->ts;
		type = eit->type;
		version_number = eit->version_number;
	}
	else
	{
		current_next_indicator=0;
		original_network_id=0;
		service_id=0;
		transport_stream_id=0;
		ts=0;
		type=0;
		version_number=0;
	}
}

EIT::~EIT()
{
	for(ePtrList<__u8>::iterator i(eventsPlain); i!=eventsPlain.end(); ++i )
		delete [] *i;
}

eTable *EIT::createNext()
{
	if (ts != tsFaked)
	{
		if ( eSystemInfo::getInstance()->hasNegFilter() )
		{
			eDebug("EIT version = %d", version_number);
			return new EIT(type, service_id, ts, version_number);
		}
		int newversion = incrementVersion(version);
		eDebug("EIT oldversion=%d, newversion=%d",version, newversion);
		return new EIT(type, service_id, ts, newversion);
	}
	return 0;
}

int TDT::data(__u8 *data)
{
	tdt_t *tdt=(tdt_t*)data;

	// only table id 0x70(TDT) and 0x73(TOT) are valid
	if ( data[0] != 0x70 && data[0] != 0x73 )
		return -1;
		
	if (tdt->utc_time5!=0xFF)
	{
		UTC_time=parseDVBtime(tdt->utc_time1, tdt->utc_time2, tdt->utc_time3,
			tdt->utc_time4, tdt->utc_time5);
		return 1;
	} 
	else
	{
		eDebug("invalide TDT::data");
		UTC_time=-1;
		return -1;
	}
}

TDT::TDT()
	:eTable(PID_TDT, TID_TDT, 0xFC, -1, -1)
{
}

BATEntry::BATEntry(bat_loop_struct *entry)
{
	init_BATEntry(entry);
}
void BATEntry::init_BATEntry(bat_loop_struct *entry)
{
	transport_stream_id=HILO(entry->transport_stream_id);
	original_network_id=HILO(entry->original_network_id);
	int len=HILO(entry->transport_descriptors_length);
	int ptr=0;
	__u8 *data=(__u8*)(entry+1);
	
	transport_descriptors.setAutoDelete(true);
	
	while (ptr<len)
	{
		descr_gen_t *d=(descr_gen_t*) (data+ptr);
		transport_descriptors.push_back(Descriptor::create(d));
		ptr+=d->descriptor_length+2;
	}
}

int BAT::data(__u8 *data)
{
	bat_t *bat=(bat_t*)data;
	bouquet_id=HILO(bat->bouquet_id);
	int looplen=HILO(bat->bouquet_descriptors_length);
	int ptr=0;
	while (ptr<looplen)
	{
		descr_gen_t *d=(descr_gen_t*) (((__u8*)(bat+1))+ptr);
		bouquet_descriptors.push_back(Descriptor::create(d));
		ptr+=d->descriptor_length+2;
	}
	data+=looplen+BAT_SIZE;
	looplen=((data[0]&0xF)<<8)|data[1];
	data+=2;
	ptr=0;
	while (ptr<looplen)
	{
		entries.push_back(new BATEntry((bat_loop_struct*)(data+ptr)));
		ptr+=HILO(((bat_loop_struct*)(data+ptr))->transport_descriptors_length)+BAT_LOOP_SIZE;
	}
	return ptr!=looplen;
}

BAT::BAT()
	:eTable(PID_BAT, TID_BAT)
{
	bouquet_descriptors.setAutoDelete(true);
	entries.setAutoDelete(true);
}
