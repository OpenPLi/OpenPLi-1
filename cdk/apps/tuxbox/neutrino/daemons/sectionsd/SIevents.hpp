#ifndef SIEVENTS_HPP
#define SIEVENTS_HPP
//
// $Id: SIevents.hpp,v 1.19 2002/02/28 01:52:21 field Exp $
//
// classes SIevent and SIevents (dbox-II-project)
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
// $Log: SIevents.hpp,v $
// Revision 1.19  2002/02/28 01:52:21  field
// Verbessertes Umschalt-Handling
//
// Revision 1.18  2002/02/23 20:23:23  McClean
// fix up
//
// Revision 1.17  2002/02/23 14:53:18  McClean
// add fsk
//
// Revision 1.16  2001/11/05 17:12:05  field
// Versuch zu Wiederholungen
//
// Revision 1.15  2001/11/03 03:13:52  field
// Auf Perspektiven vorbereitet
//
// Revision 1.14  2001/07/25 11:39:17  fnbrd
// Added unique keys to Events and Services
//
// Revision 1.13  2001/07/23 00:21:23  fnbrd
// removed using namespace std.
//
// Revision 1.12  2001/07/16 13:33:40  fnbrd
// removeOldEvents geaendert.
//
// Revision 1.11  2001/07/14 22:59:58  fnbrd
// removeOldEvents() in SIevents
//
// Revision 1.10  2001/06/27 11:59:44  fnbrd
// Angepasst an gcc 3.0
//
// Revision 1.9  2001/06/11 19:22:54  fnbrd
// Events haben jetzt mehrere Zeiten, fuer den Fall von NVODs (cinedoms)
//
// Revision 1.8  2001/06/11 01:15:16  fnbrd
// NVOD reference descriptors und Service-Typ
//
// Revision 1.7  2001/06/10 14:55:51  fnbrd
// Kleiner Aenderungen und Ergaenzungen (epgMini).
//
// Revision 1.6  2001/05/20 14:40:15  fnbrd
// Mit parental_rating
//
// Revision 1.5  2001/05/19 20:15:08  fnbrd
// Kleine Aenderungen (und epgXML).
//
// Revision 1.4  2001/05/18 20:31:04  fnbrd
// Aenderungen fuer -Wall
//
// Revision 1.3  2001/05/18 13:11:46  fnbrd
// Fast komplett, fehlt nur noch die Auswertung der time-shifted events
// (Startzeit und Dauer der Cinedoms).
//
// Revision 1.2  2001/05/17 01:53:35  fnbrd
// Jetzt mit lokaler Zeit.
//
// Revision 1.1  2001/05/16 15:23:47  fnbrd
// Alles neu macht der Mai.
//

#include <vector>

// forward references
class SIservice;
class SIservices;

struct eit_event {
  unsigned short event_id : 16;
  unsigned long long start_time : 40;
  unsigned int duration : 24;
  unsigned char running_status : 3;
  unsigned char free_CA_mode : 1;
  unsigned short descriptors_loop_length : 12;
} __attribute__ ((packed)) ;


struct descr_component_header {
  unsigned char descriptor_tag : 8;
  unsigned char descriptor_length : 8;
  unsigned char reserved_future_use : 4;
  unsigned char stream_content : 4;
  unsigned char component_type : 8;
  unsigned char component_tag : 8;
  unsigned iso_639_2_language_code : 24;
} __attribute__ ((packed)) ;

struct descr_linkage_header {
  unsigned char descriptor_tag : 8;
  unsigned char descriptor_length : 8;
  unsigned short transport_stream_id : 16;
  unsigned short original_network_id : 16;
  unsigned short service_id : 16;
  unsigned char linkage_type : 8;
} __attribute__ ((packed)) ;

class SIlinkage {
  public:
    SIlinkage(const struct descr_linkage_header *link) {
      linkageType=link->linkage_type;
      transportStreamId=link->transport_stream_id;
      originalNetworkId=link->original_network_id;
      serviceId=link->service_id;
      if(link->descriptor_length>sizeof(struct descr_linkage_header)-2)
        name=std::string(((const char *)link)+sizeof(struct descr_linkage_header), link->descriptor_length-(sizeof(struct descr_linkage_header)-2));
    }
    // Std-copy
    SIlinkage(const SIlinkage &l) {
      linkageType=l.linkageType;
      transportStreamId=l.transportStreamId;
      originalNetworkId=l.originalNetworkId;
      serviceId=l.serviceId;
      name=l.name;
    }
    // Der Operator zum sortieren
    bool operator < (const SIlinkage& l) const {
      return name < l.name;
//      return component < c.component;
    }

    void dump(void) const {
      printf("Linakge Type: 0x%02hhx\n", linkageType);
      if(name.length())
        printf("Name: %s\n", name.c_str());
      printf("Transport Stream Id: 0x%04hhx\n", transportStreamId);
      printf("Original Network Id: 0x%04hhx\n", originalNetworkId);
      printf("Service Id: 0x%04hhx\n", serviceId);
    }
    int saveXML(FILE *file) const {
      if(fprintf(file, "    <linkage type=\"0x%02hhx\" linkage descriptor=\"%s\" transport_stream_id=\"0x%04hhx\" original_network_id=\"0x%04hhx\" service_id=\"0x%04hhx\" />\n", linkageType, name.c_str(), transportStreamId, originalNetworkId, serviceId)<0)
        return 1;
      return 0;
    }
    unsigned char linkageType; // Linkage Descriptor
    std::string name; // Text aus dem Linkage Descriptor
    unsigned short transportStreamId; // Linkage Descriptor
    unsigned short originalNetworkId; // Linkage Descriptor
    unsigned short serviceId; // Linkage Descriptor
};

// Fuer for_each
struct printSIlinkage : public std::unary_function<class SIlinkage, void>
{
  void operator() (const SIlinkage &l) { l.dump();}
};

// Fuer for_each
struct saveSIlinkageXML : public std::unary_function<class SIlinkage, void>
{
  FILE *f;
  saveSIlinkageXML(FILE *fi) { f=fi;}
  void operator() (const SIlinkage &l) { l.saveXML(f);}
};

//typedef std::multiset <SIlinkage, std::less<SIlinkage> > SIlinkage_descs;
typedef std::vector<class SIlinkage> SIlinkage_descs;

class SIcomponent {
  public:
    SIcomponent(const struct descr_component_header *comp) {
      streamContent=comp->stream_content;
      componentType=comp->component_type;
      componentTag=comp->component_tag;
      if(comp->descriptor_length>sizeof(struct descr_component_header)-2)
        component=std::string(((const char *)comp)+sizeof(struct descr_component_header), comp->descriptor_length-(sizeof(struct descr_component_header)-2));
    }
    // Std-copy
    SIcomponent(const SIcomponent &c) {
      streamContent=c.streamContent;
      componentType=c.componentType;
      componentTag=c.componentTag;
      component=c.component;
    }
    // Der Operator zum sortieren
    bool operator < (const SIcomponent& c) const {
      return streamContent < c.streamContent;
//      return component < c.component;
    }
    void dump(void) const {
      if(component.length())
        printf("Component: %s\n", component.c_str());
      printf("Stream Content: 0x%02hhx\n", streamContent);
      printf("Component type: 0x%02hhx\n", componentType);
      printf("Component tag: 0x%02hhx\n", componentTag);
    }
    int saveXML(FILE *file) const {
      if(fprintf(file, "    <component tag=\"0x%02hhx\" type=\"0x%02hhx\" stream_content=\"0x%02hhx\" />\n", componentTag, componentType, streamContent)<0)
        return 1;
      return 0;
    }
    std::string component; // Text aus dem Component Descriptor
    unsigned char componentType; // Component Descriptor
    unsigned char componentTag; // Component Descriptor
    unsigned char streamContent; // Component Descriptor
};

// Fuer for_each
struct printSIcomponent : public std::unary_function<class SIcomponent, void>
{
  void operator() (const SIcomponent &c) { c.dump();}
};

// Fuer for_each
struct saveSIcomponentXML : public std::unary_function<class SIcomponent, void>
{
  FILE *f;
  saveSIcomponentXML(FILE *fi) { f=fi;}
  void operator() (const SIcomponent &c) { c.saveXML(f);}
};

typedef std::multiset <SIcomponent, std::less<SIcomponent> > SIcomponents;

class SIparentalRating {
  public:
    SIparentalRating(const std::string &cc, unsigned char rate) {
      rating=rate;
      countryCode=cc;
    }
    // Std-Copy
    SIparentalRating(const SIparentalRating &r) {
      rating=r.rating;
      countryCode=r.countryCode;
    }
    // Der Operator zum sortieren
    bool operator < (const SIparentalRating& c) const {
      return countryCode < c.countryCode;
    }
    void dump(void) const {
      printf("Rating: %s %hhu (+3)\n", countryCode.c_str(), rating);
    }
    int saveXML(FILE *file) const {
      if(fprintf(file, "    <parental_rating country=\"%s\" rating=\"%hhu\" />\n", countryCode.c_str(), rating)<0)
        return 1;
      return 0;
    }
    std::string countryCode;
    unsigned char rating; // Bei 1-16 -> Minumim Alter = rating +3
};

// Fuer for_each
struct printSIparentalRating : public std::unary_function<SIparentalRating, void>
{
  void operator() (const SIparentalRating &r) { r.dump();}
};

// Fuer for_each
struct saveSIparentalRatingXML : public std::unary_function<SIparentalRating, void>
{
  FILE *f;
  saveSIparentalRatingXML(FILE *fi) { f=fi;}
  void operator() (const SIparentalRating &r) { r.saveXML(f);}
};

typedef std::set <SIparentalRating, std::less<SIparentalRating> > SIparentalRatings;

class SItime {
  public:
    SItime(time_t s, unsigned d) {
      startzeit=s;
      dauer=d; // in Sekunden, 0 -> time shifted (cinedoms)
    }
    // Std-Copy
    SItime(const SItime &t) {
      startzeit=t.startzeit;
      dauer=t.dauer;
    }
    // Der Operator zum sortieren
    bool operator < (const SItime& t) const {
      return startzeit < t.startzeit;
    }
    void dump(void) const {
      printf("Startzeit: %s", ctime(&startzeit));
      printf("Dauer: %02u:%02u:%02u (%umin, %us)\n", dauer/3600, (dauer%3600)/60, dauer%60, dauer/60, dauer);
    }
    int saveXML(FILE *file) const { // saves the time
      // Ist so noch nicht in Ordnung, das sollte untergliedert werden,
      // da sonst evtl. time,date,duration,time,date,... auftritt
      // und eine rein sequentielle Ordnung finde ich nicht ok.
      struct tm *zeit=localtime(&startzeit);
      fprintf(file, "    <time>%02d:%02d:%02d</time>\n", zeit->tm_hour, zeit->tm_min, zeit->tm_sec);
      fprintf(file, "    <date>%02d.%02d.%04d</date>\n", zeit->tm_mday, zeit->tm_mon+1, zeit->tm_year+1900);
      fprintf(file, "    <duration>%u</duration>\n", dauer);
      return 0;
    }
    time_t startzeit;  // lokale Zeit, 0 -> time shifted (cinedoms)
    unsigned dauer; // in Sekunden, 0 -> time shifted (cinedoms)
};

typedef std::set <SItime, std::less<SItime> > SItimes;

// Fuer for_each
struct printSItime : public std::unary_function<SItime, void>
{
  void operator() (const SItime &t) { t.dump();}
};

// Fuer for_each
struct saveSItimeXML : public std::unary_function<SItime, void>
{
  FILE *f;
  saveSItimeXML(FILE *fi) { f=fi;}
  void operator() (const SItime &t) { t.saveXML(f);}
};

class SIevent {
  public:
    SIevent(const struct eit_event *);
    // Std-Copy
    SIevent(const SIevent &);
    SIevent(void) {
      serviceID=eventID=originalNetworkID=0;
//      dauer=0;
//      startzeit=0;
    }
    unsigned short eventID;
    std::string name; // Name aus dem Short-Event-Descriptor
    std::string text; // Text aus dem Short-Event-Descriptor
    std::string itemDescription; // Aus dem Extended Descriptor
    std::string item; // Aus dem Extended Descriptor
    std::string extendedText; // Aus dem Extended Descriptor
    std::string contentClassification; // Aus dem Content Descriptor, als String, da mehrere vorkommen koennen
    std::string userClassification; // Aus dem Content Descriptor, als String, da mehrere vorkommen koennen
//    time_t startzeit; // lokale Zeit, 0 -> time shifted (cinedoms)
//    unsigned dauer; // in Sekunden, 0 -> time shifted (cinedoms)
    unsigned short serviceID;
    unsigned short originalNetworkID; // braucht man, wenn man events mehrerer Networks in einer Menge speichert, innerhalb einer section ist das unnoetig
    static unsigned long long makeUniqueKey(unsigned short onID, unsigned short sID, unsigned short eID) {
      return (((unsigned long long)onID)<<32) + (((unsigned)sID)<<16) + eID;
    }
    unsigned long long uniqueKey(void) const {
      return makeUniqueKey(originalNetworkID, serviceID, eventID);
    }
    SIcomponents components;
    SIparentalRatings ratings;
    SIlinkage_descs linkage_descs;
    SItimes times;
    // Der Operator zum sortieren
    bool operator < (const SIevent& e) const {
      return uniqueKey()<e.uniqueKey();
    }
    int saveXML(FILE *file) const { // saves the event
      return saveXML0(file) || saveXML2(file);
    }
    int saveXML(FILE *file, const char *serviceName) const; // saves the event
    void dump(void) const; // dumps the event to stdout
    void dumpSmall(void) const; // dumps the event to stdout (not all information)
    // Liefert das aktuelle EPG des senders mit der uebergebenen serviceID,
    // bei Fehler ist die serviceID des zurueckgelieferten Events 0
    static SIevent readActualEvent(unsigned short serviceID, unsigned timeoutInSeconds=2);

    char getFSK() const;
 protected:
    int saveXML0(FILE *f) const;
    int saveXML2(FILE *f) const;
};

// Fuer for_each
struct printSIevent : public std::unary_function<SIevent, void>
{
  void operator() (const SIevent &e) { e.dump();}
};

// Fuer for_each
struct saveSIeventXML : public std::unary_function<SIevent, void>
{
  FILE *f;
  saveSIeventXML(FILE *fi) { f=fi;}
  void operator() (const SIevent &e) { e.saveXML(f);}
};

// Fuer for_each
struct saveSIeventXMLwithServiceName : public std::unary_function<SIevent, void>
{
  FILE *f;
  const SIservices *s;
  saveSIeventXMLwithServiceName(FILE *fi, const SIservices &svs) {f=fi; s=&svs;}
  void operator() (const SIevent &e) {
    SIservices::iterator k=s->find(SIservice(e.serviceID, e.originalNetworkID));
    if(k!=s->end()) {
      if(k->serviceName.length())
      e.saveXML(f, k->serviceName.c_str());
    }
    else
      e.saveXML(f);
  }
};

// Fuer for_each
struct printSIeventWithService : public std::unary_function<SIevent, void>
{
  printSIeventWithService(const SIservices &svs) { s=&svs;}
  void operator() (const SIevent &e) {
    SIservices::iterator k=s->find(SIservice(e.serviceID, e.originalNetworkID));
    if(k!=s->end()) {
      char servicename[50];
      strncpy(servicename, k->serviceName.c_str(), sizeof(servicename)-1);
      servicename[sizeof(servicename)-1]=0;
      removeControlCodes(servicename);
      printf("Service-Name: %s\n", servicename);
//      printf("Provider-Name: %s\n", k->providerName.c_str());
    }
    e.dump();
//    e.dumpSmall();
    printf("\n");
  }
  const SIservices *s;
};

class SIevents : public std::set <SIevent, std::less<SIevent> >
{
  public:
    // Entfernt anhand der Services alle time shifted events (Service-Typ 0)
    // und sortiert deren Zeiten in die Events mit dem Text ein.
    void mergeAndRemoveTimeShiftedEvents(const SIservices &);
    // Loescht alte Events (aufgrund aktueller Zeit - seconds und Zeit im Event)
    void removeOldEvents(long seconds);
};

#endif // SIEVENTS_HPP
