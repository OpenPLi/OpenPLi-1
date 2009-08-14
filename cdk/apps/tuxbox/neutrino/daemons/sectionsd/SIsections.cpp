//
// $Id: SIsections.cpp,v 1.28.2.1 2003/02/06 17:57:30 thegoodguy Exp $
//
// classes for SI sections (dbox-II-project)
//
//    Homepage: http://dbox2.elxsi.de
//
//    Copyright (C) 2001 fnbrd (fnbrd@gmx.de)
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
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h> // fuer poll()

#include <set>
#include <algorithm>
#include <string>

#include "SIutils.hpp"
#include "SIservices.hpp"
#include "SIevents.hpp"
#include "SIsections.hpp"
#include <dmxapi.h>

//#define DEBUG

// #pragma pack(1) // fnbrd: geht anscheinend nicht beim gcc

struct descr_generic_header {
  unsigned char descriptor_tag : 8;
  unsigned char descriptor_length : 8;
} __attribute__ ((packed)) ;

struct descr_short_event_header {
  unsigned char descriptor_tag : 8;
  unsigned char descriptor_length : 8;
  unsigned language_code : 24;
  unsigned char event_name_length : 8;
} __attribute__ ((packed)) ;

struct descr_service_header {
  unsigned char descriptor_tag : 8;
  unsigned char descriptor_length : 8;
  unsigned char service_typ : 8;
  unsigned char service_provider_name_length : 8;
} __attribute__ ((packed)) ;

struct descr_extended_event_header {
  unsigned char descriptor_tag : 8;
  unsigned char descriptor_length : 8;
  unsigned char descriptor_number : 4;
  unsigned char last_descriptor_number : 4;
  unsigned iso_639_2_language_code : 24;
  unsigned char length_of_items : 8;
} __attribute__ ((packed)) ;

//#pragma pack()

inline unsigned min(unsigned a, unsigned b)
{
  return b<a ? b : a;
}


//-----------------------------------------------------------------------
// Da es vorkommen kann das defekte Packete empfangen werden
// sollte hier alles ueberprueft werden.
// Leider ist das noch nicht bei allen Descriptoren so.
//-----------------------------------------------------------------------
void SIsectionEIT::parseLinkageDescriptor(const char *buf, SIevent &e, unsigned maxlen)
{
  if(maxlen>=sizeof(struct descr_linkage_header))
  {
    SIlinkage l((const struct descr_linkage_header *)buf);
    e.linkage_descs.insert(e.linkage_descs.end(), l);
//    printf("LinkName: %s\n", l.name.c_str());
  }
}

void SIsectionEIT::parseComponentDescriptor(const char *buf, SIevent &e, unsigned maxlen)
{
  if(maxlen>=sizeof(struct descr_component_header))
    e.components.insert(SIcomponent((const struct descr_component_header *)buf));
}

void SIsectionEIT::parseContentDescriptor(const char *buf, SIevent &e, unsigned maxlen)
{
  struct descr_generic_header *cont=(struct descr_generic_header *)buf;
  if(cont->descriptor_length+sizeof(struct descr_generic_header)>maxlen)
    return; // defekt
  const char *classification=buf+sizeof(struct descr_generic_header);
  while(classification<=buf+sizeof(struct descr_generic_header)+cont->descriptor_length-2) {
    e.contentClassification+=std::string(classification, 1);
//    printf("Content: 0x%02hhx\n", *classification);
    e.userClassification+=std::string(classification+1, 1);
//    printf("User: 0x%02hhx\n", *(classification+1));
    classification+=2;
  }
}

void SIsectionEIT::parseParentalRatingDescriptor(const char *buf, SIevent &e, unsigned maxlen)
{
  struct descr_generic_header *cont=(struct descr_generic_header *)buf;
  if(cont->descriptor_length+sizeof(struct descr_generic_header)>maxlen)
    return; // defekt
  const char *s=buf+sizeof(struct descr_generic_header);
  while(s<buf+sizeof(struct descr_generic_header)+cont->descriptor_length-4) {
    e.ratings.insert(SIparentalRating(std::string(s, 3), *(s+3)));
    s+=4;
  }
}

void SIsectionEIT::parseExtendedEventDescriptor(const char *buf, SIevent &e, unsigned maxlen)
{
  struct descr_extended_event_header *evt=(struct descr_extended_event_header *)buf;
  if((evt->descriptor_length+sizeof(descr_generic_header)>maxlen) || (evt->descriptor_length<sizeof(struct descr_extended_event_header)-sizeof(descr_generic_header)))
    return; // defekt
  unsigned char *items=(unsigned char *)(buf+sizeof(struct descr_extended_event_header));
  while(items<(unsigned char *)(buf+sizeof(struct descr_extended_event_header)+evt->length_of_items)) {
    if(*items) {
      if(*(items+1) < 0x06) // other code table
        e.itemDescription=std::string((const char *)(items+2), min(maxlen-((const char *)items+2-buf), (*items)-1));
      else
        e.itemDescription=std::string((const char *)(items+1), min(maxlen-((const char *)items+1-buf), *items));
//      printf("Item Description: %s\n", e.itemDescription.c_str());
    }
    items+=1+*items;
    if(*items) {
      e.item=std::string((const char *)(items+1), min(maxlen-((const char *)items+1-buf), *items));
//      printf("Item: %s\n", e.item.c_str());
    }
    items+=1+*items;
  }
  if(*items) {
    if(*(items+1) < 0x06) // other code table
      e.extendedText+=std::string((const char *)(items+2), min(maxlen-((const char *)items+2-buf), (*items)-1));
    else
      e.extendedText+=std::string((const char *)(items+1), min(maxlen-((const char *)items+1-buf), *items));
//    printf("Extended Text: %s\n", e.extendedText.c_str());
  }
}

void SIsectionEIT::parseShortEventDescriptor(const char *buf, SIevent &e, unsigned maxlen)
{
  struct descr_short_event_header *evt=(struct descr_short_event_header *)buf;
  if((evt->descriptor_length+sizeof(descr_generic_header)>maxlen) || (evt->descriptor_length<sizeof(struct descr_short_event_header)-sizeof(descr_generic_header)))
    return; // defekt
  buf+=sizeof(struct descr_short_event_header);
  if(evt->event_name_length) {
    if(*buf < 0x06) // other code table
      e.name=std::string(buf+1, evt->event_name_length-1);
    else
      e.name=std::string(buf, evt->event_name_length);
  }
  buf+=evt->event_name_length;
  unsigned char textlength=*((unsigned char *)buf);
  if(textlength) {
    if(*(buf+1) < 0x06) // other code table
      e.text=std::string((++buf)+1, textlength-1);
    else
      e.text=std::string(++buf, textlength);
  }
//  printf("Name: %s\n", e.name.c_str());
//  printf("Text: %s\n", e.text.c_str());

}

void SIsectionEIT::parseDescriptors(const char *des, unsigned len, SIevent &e)
{
  struct descr_generic_header *desc;
  while(len>=sizeof(struct descr_generic_header)) {
    desc=(struct descr_generic_header *)des;
//    printf("Type: %s\n", decode_descr(desc->descriptor_tag));
    if(desc->descriptor_tag==0x4D)
      parseShortEventDescriptor((const char *)desc, e, len);
    else if(desc->descriptor_tag==0x4E)
      parseExtendedEventDescriptor((const char *)desc, e, len);
    else if(desc->descriptor_tag==0x54)
      parseContentDescriptor((const char *)desc, e, len);
    else if(desc->descriptor_tag==0x50)
      parseComponentDescriptor((const char *)desc, e, len);
    else if(desc->descriptor_tag==0x55)
      parseParentalRatingDescriptor((const char *)desc, e, len);
    else if(desc->descriptor_tag==0x4A)
      parseLinkageDescriptor((const char *)desc, e, len);
    if((unsigned)(desc->descriptor_length+2)>len)
      break;
    len-=desc->descriptor_length+2;
    des+=desc->descriptor_length+2;
  }
}

// Die infos aus dem Puffer holen
void SIsectionEIT::parse(void)
{
//  printf("parse\n");
  if(!buffer || parsed)
    return;
  if(bufferLength<sizeof(SI_section_EIT_header)+sizeof(struct eit_event)) {
    delete [] buffer;
    buffer=0;
    bufferLength=0;
    return;
  }
  const char *actPos=buffer+sizeof(SI_section_EIT_header);
  while(actPos<buffer+bufferLength-sizeof(struct eit_event)) {
    struct eit_event *evt=(struct eit_event *)actPos;
    SIevent e(evt);
    e.serviceID=serviceID();
    e.originalNetworkID=originalNetworkID();
//    printf("actpos: %p buf+bl: %p evtid: %hu desclen: %hu\n", actPos, buffer+bufferLength, evt->event_id, evt->descriptors_loop_length);
//    printf("maxlen: %u (%s)\n", min((unsigned)(buffer+bufferLength-actPos), evt->descriptors_loop_length), (unsigned)(buffer+bufferLength-actPos)< evt->descriptors_loop_length ? "bufferLength" : "descriptor_loop_length");
    parseDescriptors(((const char *)evt)+sizeof(struct eit_event), min((unsigned)(buffer+bufferLength-actPos), evt->descriptors_loop_length), e);
    //printf("lds: %d\n", e.linkage_descs.size());
    evts.insert(e);
    actPos+=sizeof(struct eit_event)+evt->descriptors_loop_length;
  }
  parsed=1;
}

void SIsectionSDT::parseNVODreferenceDescriptor(const char *buf, SIservice &s)
{
  struct descr_generic_header *hdr=(struct descr_generic_header *)buf;
  unsigned short *spointer=(unsigned short *)(buf+sizeof(struct descr_generic_header));
  while((const char *)spointer<=buf+sizeof(struct descr_generic_header)+hdr->descriptor_length-2) {
    unsigned short transportStreamID=*spointer++;
    unsigned short originalNetworkID=*spointer++;
    unsigned short sID=*spointer++;
    s.nvods.insert(SInvodReference(transportStreamID, originalNetworkID, sID));
  }
}

void SIsectionSDT::parseServiceDescriptor(const char *buf, SIservice &s)
{
  struct descr_service_header *sv=(struct descr_service_header *)buf;
  buf+=sizeof(struct descr_service_header);
  s.serviceTyp=sv->service_typ;
  if(sv->service_provider_name_length) {
    if(*buf < 0x06) // other code table
      s.providerName=std::string(buf+1, sv->service_provider_name_length-1);
    else
      s.providerName=std::string(buf, sv->service_provider_name_length);
  }
  buf+=sv->service_provider_name_length;
  unsigned char servicenamelength=*((unsigned char *)buf);
  if(servicenamelength) {
    if(*(buf+1) < 0x06) // other code table
      s.serviceName=std::string((++buf)+1, servicenamelength-1);
    else
      s.serviceName=std::string(++buf, servicenamelength);
  }
//  printf("Provider-Name: %s\n", s.providerName.c_str());
//  printf("Service-Name: %s\n", s.serviceName.c_str());
}

void SIsectionSDT::parseDescriptors(const char *des, unsigned len, SIservice &s)
{
  struct descr_generic_header *desc;
  while(len>=sizeof(struct descr_generic_header)) {
    desc=(struct descr_generic_header *)des;
//    printf("Type: %s\n", decode_descr(desc->descriptor_tag));
//    printf("Length: %hhu\n", desc->descriptor_length);
    if(desc->descriptor_tag==0x48) {
//      printf("Found service descriptor\n");
      parseServiceDescriptor((const char *)desc, s);
    }
    else if(desc->descriptor_tag==0x4b) {
//      printf("Found NVOD reference descriptor\n");
      parseNVODreferenceDescriptor((const char *)desc, s);
    }
    len-=desc->descriptor_length+2;
    des+=desc->descriptor_length+2;
  }
}

// Die infos aus dem Puffer holen
void SIsectionSDT::parse(void)
{
//  printf("parse\n");
//  saveBufferToFile("sdt.sec");
//  printf("Size: %d\n", sizeof(SI_section_SDT_header));
  if(!buffer || bufferLength<sizeof(SI_section_SDT_header)+sizeof(struct sdt_service) || parsed)
    return;
  const char *actPos=buffer+sizeof(SI_section_SDT_header);
  while(actPos<=buffer+bufferLength-sizeof(struct sdt_service)) {
    struct sdt_service *sv=(struct sdt_service *)actPos;
    SIservice s(sv);
    s.originalNetworkID=originalNetworkID();
//    printf("actpos: %p buf+bl: %p sid: %hu desclen: %hu\n", actPos, buffer+bufferLength, sv->service_id, sv->descriptors_loop_length);
    parseDescriptors(((const char *)sv)+sizeof(struct sdt_service), sv->descriptors_loop_length, s);
    svs.insert(s);
    actPos+=sizeof(struct sdt_service)+sv->descriptors_loop_length;
  }
  parsed=1;
}

// Liest n Bytes aus einem Socket per read
// Liefert 0 bei timeout
// und -1 bei Fehler
// ansonsten die Anzahl gelesener Bytes
inline int readNbytes(int fd, char *buf, int n, unsigned timeoutInSeconds)
{
int j;
  for(j=0; j<n;) {
    struct pollfd ufds;
//    memset(&ufds, 0, sizeof(ufds));
    ufds.fd=fd;
    ufds.events=POLLIN|POLLPRI;
    ufds.revents=0;
    int rc=poll(&ufds, 1, timeoutInSeconds*1000);
    if(!rc)
      return 0; // timeout
    else if(rc<0) {
      perror ("poll");
      return -1;
    }
    int r=read (fd, buf, n-j);
    if(r<=0) {
      perror ("read");
      return -1;
    }
    j+=r;
    buf+=r;
  }
  return j;
}

//
// Beachtung der Stuffing tables (ST) fehlt noch
//
int SIsections :: readSections(const unsigned short pid, const unsigned char filter, const unsigned char mask, int readNext, unsigned timeoutInSeconds)
{
	int fd;
	struct SI_section_header header;
	unsigned long long firstKey=(unsigned long long)-1;
	SIsections missingSections;
	char *buf;

	if ((fd = open(DEMUX_DEVICE, O_RDWR)) == -1) {
		perror(DEMUX_DEVICE);
		return 1;
	}
	if (!setfilter(fd, pid, filter, mask, DMX_IMMEDIATE_START | DMX_CHECK_CRC))
	{
		close(fd);
		return 2;
	}

  time_t szeit=time(NULL);

//  printf("reading first\n");
  // Erstes Segment lesen
  do {
    if(time(NULL)>szeit+(long)(timeoutInSeconds)) {
      close(fd);
      return 0; // timeout -> kein EPG
    }
    int rc=readNbytes(fd, (char *)&header, sizeof(header), timeoutInSeconds);
    if(!rc) {
      close(fd);
      return 0; // timeout -> kein EPG
    }
    else if(rc<0) {
      close(fd);
//      perror ("read header");
      return 3;
    }
    buf=new char[sizeof(header)+header.section_length-5];
    if(!buf) {
      close(fd);
      fprintf(stderr, "Not enough memory!\n");
      return 4;
    }
    // Den Header kopieren
    memcpy(buf, &header, sizeof(header));
    rc=readNbytes(fd, buf+sizeof(header), header.section_length-5, timeoutInSeconds);
    if(!rc) {
      close(fd);
      delete[] buf;
      return 0; // timeout -> kein EPG
    }
    else if(rc<0) {
      close(fd);
//      perror ("read section");
      delete[] buf;
      return 5;
    }
    if(readNext || header.current_next_indicator) {
      // Wir wollen nur aktuelle sections
      insert(SIsection(sizeof(header)+header.section_length-5, buf));
      firstKey=SIsection::key(&header);
      // Sonderfall: Nur eine Section
      // d.h. wir sind fertig
      if(!header.section_number && !header.last_section_number) {
        close(fd);
        return 0;
      }
    }
    else
      delete[] buf;
  } while (firstKey==(unsigned long long) -1);
  // Die restlichen Segmente lesen

  szeit=time(NULL);
//  printf("reading next\n");

  for(;;) {
    if(time(NULL)>szeit+(long)(timeoutInSeconds))
      break; // timeout
    int rc=readNbytes(fd, (char *)&header, sizeof(header), timeoutInSeconds);
    if(!rc)
      break; // timeout
    else if(rc<0) {
      close(fd);
//      perror ("read header");
      return 6;
    }
    if(firstKey==SIsection::key(&header))
      // Wir haben die 1. section wieder gefunden
      break;
    buf=new char[sizeof(header)+header.section_length-5];
    if(!buf) {
      close(fd);
      fprintf(stderr, "Not enough memory!\n");
      return 7;
    }
    // Den Header kopieren (evtl. malloc und realloc nehmen)
    memcpy(buf, &header, sizeof(header));
    // den Rest der Section einlesen
    rc=readNbytes(fd, buf+sizeof(header), header.section_length-5, timeoutInSeconds);
    if(!rc) {
      delete[] buf;
      break; // timeout
    }
    else if(rc<0) {
      close(fd);
      delete[] buf;
//      perror ("read section");
      return 8;
    }
    if(readNext || header.current_next_indicator)
      insert(SIsection(sizeof(header)+header.section_length-5, buf));
    else
      delete[] buf;
  }
  close(fd);

#ifdef DEBUG
  // Die Sections ausgeben
  printf("----------------Found sections-----------------------\n");
//  for_each(begin(), end(), printSIsection());
  for_each(begin(), end(), printSIsectionEIT());
  printf("-----------------------------------------------------\n");
#endif // DEBUG

  // Jetzt erstellen wir eine Liste der fehlenden Sections
  unsigned actualTableIDextension=(unsigned)-1;
  unsigned actualTableID=(unsigned)-1;
  unsigned maxNr=0;
  unsigned lastNr=0;
  for(SIsections::iterator k=begin(); k!=end(); k++) {
    if(k->tableIDextension()!=actualTableIDextension || k->tableID()!=actualTableID) {
      // Neue Table-ID-Extension
      maxNr=k->lastSectionNumber();
      lastNr=k->sectionNumber();
      actualTableIDextension=k->tableIDextension();
      actualTableID=k->tableID();
    }
    else if(k->sectionNumber()!=lastNr+1) {
      // Es fehlen Sections
      for(unsigned l=lastNr+1; l<k->sectionNumber(); l++) {
//	printf("Debug: t: 0x%02x s: %u nr: %u last: %u max: %u l: %u\n", actualTableID, actualTableIDextension, k->sectionNumber(), lastNr, maxNr, l);
	struct SI_section_header h;
        memcpy(&h, k->header(), sizeof(struct SI_section_header));
	h.section_number=l;
        missingSections.insert(SIsection(&h));
      }
      lastNr=k->sectionNumber();
    }
    else
      lastNr=k->sectionNumber();
  }
#ifdef DEBUG
  printf("Sections read: %d\n\n", size());
#endif // DEBUG
  if(!missingSections.size())
    return 0;


#ifdef DEBUG
  printf("----------------Missing sections---------------------\n");
  for_each(missingSections.begin(), missingSections.end(), printSmallSIsectionHeader());
  printf("-----------------------------------------------------\n");
  printf("Sections read: %d\n\n", size());
  printf("Sections misssing: %d\n", missingSections.size());
  printf("Searching missing sections\n");
#endif // DEBUG

  szeit=time(NULL);
//  printf("reading missing\n");

  if ((fd = open(DEMUX_DEVICE, O_RDWR)) == -1) {
    perror(DEMUX_DEVICE);
    return 9;
  }
	if (!setfilter(fd, pid, filter, mask, DMX_IMMEDIATE_START | DMX_CHECK_CRC))
	{
		close(fd);
		return 10;
	}
  // Jetzt lesen wir die fehlenden Sections ein
  for(;;) {
    if(time(NULL)>szeit+(long)(timeoutInSeconds))
      break; // Timeout
    int rc=readNbytes(fd, (char *)&header, sizeof(header), timeoutInSeconds);
    if(!rc)
      break; // timeout
    else if(rc<0) {
      close(fd);
//      perror ("read header");
      return 11;
    }
    buf=new char[sizeof(header)+header.section_length-5];
    if(!buf) {
      close(fd);
      fprintf(stderr, "Not enough memory!\n");
      return 12;
    }
    // Den Header kopieren (evtl. malloc und realloc nehmen)
    memcpy(buf, &header, sizeof(header));
    // den Rest der Section einlesen
    rc=readNbytes(fd, buf+sizeof(header), header.section_length-5, timeoutInSeconds);
    if(!rc) {
      delete[] buf;
      break; // timeout
    }
    else if(rc<0) {
      close(fd);
      delete[] buf;
//      perror ("read section");
      return 13;
    }
    if(missingSections.find(SIsection(&header))!=missingSections.end()) {
#ifdef DEBUG
      printf("Find missing section:");
      SIsection::dumpSmallSectionHeader(&header);
#endif  // DEBUG
      // War bisher vermisst
      // In die Menge einfuegen
      insert(SIsection(sizeof(header)+header.section_length-5, buf));
      // Und aus der vermissten Menge entfernen
//      printf("Sections misssing: %d\n", missingSections.size());
      missingSections.erase(SIsection(&header));
#ifdef DEBUG
      printf("Sections misssing: %d\n", missingSections.size());
#endif // DEBUG
    }
    else
      // Puffer wieder loeschen
      delete[] buf;
  }
  close(fd);

  return 0;
}

