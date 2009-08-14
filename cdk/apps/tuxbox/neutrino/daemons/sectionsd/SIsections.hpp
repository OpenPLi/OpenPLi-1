#ifndef SISECTIONS_HPP
#define SISECTIONS_HPP
//
// $Id: SIsections.hpp,v 1.14 2002/10/15 20:39:47 woglinde Exp $
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
// $Log: SIsections.hpp,v $
// Revision 1.14  2002/10/15 20:39:47  woglinde
//
//
// mostly coding styles, adding license to some files,
// using dos2unix on one file
//
// Revision 1.13  2001/11/03 03:13:52  field
// Auf Perspektiven vorbereitet
//
// Revision 1.12  2001/09/20 09:14:07  fnbrd
// Small changes.
//
// Revision 1.11  2001/09/19 15:59:03  fnbrd
// virtual Destructor
//
// Revision 1.10  2001/07/26 21:36:59  fnbrd
// Ein paar Absicherungen gegen defekte EIT-Packete.
//
// Revision 1.9  2001/07/25 11:39:17  fnbrd
// Added unique keys to Events and Services
//
// Revision 1.8  2001/07/23 00:21:23  fnbrd
// removed using namespace std.
//
// Revision 1.7  2001/06/27 11:59:44  fnbrd
// Angepasst an gcc 3.0
//
// Revision 1.6  2001/06/11 01:15:16  fnbrd
// NVOD reference descriptors und Service-Typ
//
// Revision 1.5  2001/06/10 23:00:45  fnbrd
// Nur ein Kommentar mehr (in Beschreibung von readSections)
//
// Revision 1.4  2001/05/21 22:44:44  fnbrd
// Timeout verbessert.
//
// Revision 1.3  2001/05/20 14:40:15  fnbrd
// Mit parental_rating
//
// Revision 1.2  2001/05/18 13:11:46  fnbrd
// Fast komplett, fehlt nur noch die Auswertung der time-shifted events
// (Startzeit und Dauer der Cinedoms).
//
// Revision 1.1  2001/05/16 15:23:47  fnbrd
// Alles neu macht der Mai.
//
//

// #pragma pack(1) // fnbrd: geht anscheinend nicht beim gcc

struct SI_section_SDT_header {
      unsigned char table_id : 8;
      // 1 byte
      unsigned char section_syntax_indicator : 1;
      unsigned char reserved_future_use : 1;
      unsigned char reserved1 : 2;
      unsigned short section_length : 12;
      // 3 bytes
      unsigned short transport_stream_id : 16;
      // 5 bytes
      unsigned char reserved2 : 2;
      unsigned char version_number : 5;
      unsigned char current_next_indicator : 1;
      // 6 bytes
      unsigned char section_number : 8;
      // 7 bytes
      unsigned char last_section_number : 8;
      // 8 bytes
      unsigned short original_network_id : 16;
      // 10 bytes
      unsigned char reserved_future_use2 : 8;
} __attribute__ ((packed)) ; // 11 bytes

struct SI_section_NIT_header {
      unsigned char table_id : 8;
      // 1 byte
      unsigned char section_syntax_indicator : 1;
      unsigned char reserved_future_use : 1;
      unsigned char reserved1 : 2;
      unsigned short section_length : 12;
      // 3 bytes
      unsigned short network_id : 16;
      // 5 bytes
      unsigned char reserved2 : 2;
      unsigned char version_number : 5;
      unsigned char current_next_indicator : 1;
      // 6 bytes
      unsigned char section_number : 8;
      // 7 bytes
      unsigned char last_section_number : 8;
      // 7 bytes
      unsigned char reserved_future_use2 : 4;
      unsigned short network_descriptors_length : 12;
} __attribute__ ((packed)) ; // 9 bytes

struct SI_section_BAT_header {
      unsigned char table_id : 8;
      // 1 byte
      unsigned char section_syntax_indicator : 1;
      unsigned char reserved_future_use : 1;
      unsigned char reserved1 : 2;
      unsigned short section_length : 12;
      // 3 bytes
      unsigned short bouquet_id : 16;
      // 5 bytes
      unsigned char reserved2 : 2;
      unsigned char version_number : 5;
      unsigned char current_next_indicator : 1;
      // 6 bytes
      unsigned char section_number : 8;
      // 7 bytes
      unsigned char reserved_future_use2 : 4;
      unsigned short bouquet_descriptors_length : 12;
} __attribute__ ((packed)) ; // 9 bytes

struct SI_section_EIT_header {
      unsigned char table_id : 8;
      // 1 byte
      unsigned char section_syntax_indicator : 1;
      unsigned char reserved_future_use : 1;
      unsigned char reserved1 : 2;
      unsigned short section_length : 12;
      // 3 bytes
      unsigned short service_id : 16;
      // 5 bytes
      unsigned char reserved2 : 2;
      unsigned char version_number : 5;
      unsigned char current_next_indicator : 1;
      // 6 bytes
      unsigned char section_number : 8;
      // 7 bytes
      unsigned char last_section_number : 8;
      // 8 bytes
      unsigned short transport_stream_id : 16;
      // 10 bytes
      unsigned short original_network_id : 16;
      // 12 bytes
      unsigned char segment_last_section_number : 8;
      // 13 bytes
      unsigned char last_table_id : 8;
} __attribute__ ((packed)) ; // 14 bytes

// Muss evtl. angepasst werden falls damit RST, TDT und TOT gelesen werden sollen
struct SI_section_header {
      unsigned char table_id : 8;
      // 1 byte
      unsigned char section_syntax_indicator : 1;
      unsigned char reserved_future_use : 1;
      unsigned char reserved1 : 2;
      unsigned short section_length : 12;
      // 3 bytes
      unsigned short table_id_extension : 16; // Je nach Tabellentyp, bei EIT z.B. service_id
      // 5 bytes
      unsigned char reserved2 : 2;
      unsigned char version_number : 5;
      unsigned char current_next_indicator : 1;
      // 6 bytes
      unsigned char section_number : 8;
      // 7 bytes
      unsigned char last_section_number : 8;
} __attribute__ ((packed)) ; // 8 bytes

//#pragma pack()

class SIsection {

  public:
    SIsection() { buffer=0; bufferLength=0;}
    // Kopierte den Puffer in eigenen Puffer
    SIsection(const char *buf, unsigned bufLength) {
      buffer=0; bufferLength=0;
      if(buf && bufLength>=sizeof(struct SI_section_header)) {
        buffer=new char[bufLength];
	if(buffer) {
	  bufferLength=bufLength;
	  memcpy(buffer, buf, bufLength);
        }
      }
    }
    // Benutzt den uebergebenen Puffer (sollte mit new char[n] allokiert sein)
    SIsection(unsigned bufLength, char *buf) {
      buffer=0; bufferLength=0;
      if(buf && bufLength>=sizeof(struct SI_section_header)) {
        buffer=buf;
        bufferLength=bufLength;
      }
    }
    // Konstruktor um eine (leere) SIsection mit den fuer Vergleiche
    // noetigen Inhalte (s. key) zu erstellen
    SIsection(const struct SI_section_header *header) {
      bufferLength=0;
      buffer=new char[sizeof(struct SI_section_header)];
      if(buffer) {
        memcpy(buffer, header, sizeof(struct SI_section_header));
	bufferLength=sizeof(struct SI_section_header);
      }
    }
    // Std-Copy
    SIsection(const SIsection &s) {
      buffer=0; bufferLength=0;
      if(s.buffer) {
        buffer=new char[s.bufferLength];
	if(buffer) {
	  bufferLength=s.bufferLength;
	  memcpy(buffer, s.buffer, bufferLength);
        }
      }
    }
    // Destruktor
    virtual ~SIsection() {
      if (buffer) {
        delete [] buffer;
	buffer=0;
	bufferLength=0;
      }
    }
    unsigned tableID(void) const {
      return buffer ? ((struct SI_section_header *)buffer)->table_id : (unsigned)-1;
    }
    unsigned tableIDextension(void) const {
      return buffer ? ((struct SI_section_header *)buffer)->table_id_extension : (unsigned)-1;
    }
    unsigned sectionNumber(void) const {
      return buffer ? ((struct SI_section_header *)buffer)->section_number : (unsigned)-1;
    }
    unsigned versionNumber(void) const {
      return buffer ? ((struct SI_section_header *)buffer)->version_number : (unsigned)-1;
    }
    unsigned currentNextIndicator(void) const {
      return buffer ? ((struct SI_section_header *)buffer)->current_next_indicator : (unsigned)-1;
    }
    unsigned lastSectionNumber(void) const {
      return buffer ? ((struct SI_section_header *)buffer)->last_section_number : (unsigned)-1;
    }
    struct SI_section_header const *header(void) const {
      return buffer ? (struct SI_section_header *)buffer : (struct SI_section_header *)0;
    }
    static unsigned long long key(const struct SI_section_header *header) {
      // Der eindeutige Key einer SIsection besteht aus 1 Byte Table-ID,
      // 2 Byte Table-ID-extension, 1 Byte Section number
      // 1 Byte Version number und 1 Byte current_next_indicator
      return (((unsigned long long)header->table_id)<<40)+
        (((unsigned long long)header->table_id_extension)<<32)+
        (header->section_number<<16)+(header->version_number<<8)+header->current_next_indicator;
    }
    unsigned long long key(void) const {
      return buffer ? key(header()) : (unsigned long long) -1;
    }
    // Der Operator zum sortieren
    bool operator < (const SIsection& s) const {
      return key() < s.key();
    }

    static void dumpSmallSectionHeader(const struct SI_section_header *header) {
      printf("\ntable_id: 0x%02hhx ", header->table_id);
      printf("table_id_extension: %hu ", header->table_id_extension);
      printf("section_number: %hhu\n", header->section_number);
    }
    static void dumpSmallSectionHeader(const SIsection &s) {
      dumpSmallSectionHeader((struct SI_section_header *)s.buffer);
    }
    void dumpSmallSectionHeader(void) const {
      dumpSmallSectionHeader((struct SI_section_header *)buffer);
    }
    int saveBufferToFile(FILE *file) const {
      if(!file)
        return 1;
      return (fwrite(buffer, bufferLength, 1, file)!=1);
    }
    int saveBufferToFile(const char *filename) const {
      FILE *file=fopen(filename, "wb");
      if(file) {
        int rc=saveBufferToFile(file);
        fclose(file);
        return rc;
      }
      return 2;
    }
    static void dump1(const struct SI_section_header *header) {
      printf("\ntable_id: 0x%02hhx\n", header->table_id);
      printf("section_syntax_indicator: %hhu\n", header->section_syntax_indicator);
      printf("section_length: %hu\n", header->section_length);
    }
    static void dump2(const struct SI_section_header *header) {
      printf("version_number: %hhu\n", header->version_number);
      printf("current_next_indicator: %hhu\n", header->current_next_indicator);
      printf("section_number: %hhu\n", header->section_number);
      printf("last_section_number: %hhu\n", header->last_section_number);
    }
    static void dump(const struct SI_section_header *header) {
      dump1(header);
      printf("table_id_extension: %hu\n", header->table_id_extension);
      dump2(header);
    }
    static void dump(const SIsection &s) {
      dump((struct SI_section_header *)s.buffer);
    }
    void dump(void) const {
      dump((struct SI_section_header *)buffer);
    }

  protected:
    char *buffer;
    unsigned bufferLength;
};

// Fuer for_each
struct printSIsection : public std::unary_function<SIsection, void>
{
  void operator() (const SIsection &s) { s.dump();}
};

// Fuer for_each
struct printSmallSIsectionHeader : public std::unary_function<SIsection, void>
{
  void operator() (const SIsection &s) { s.dumpSmallSectionHeader();}
};

class SIsections : public std::set <SIsection, std::less<SIsection> >
{
  public:
    // Liefert 0 falls kein Fehler
    // Algo:
    // (1) Segment lesen (wird zum ersten Segment deklariert)
    // (2) Falls Segmentnummer = letze Segmentnummer = 0 dann fertig, sonst
    // (3) alle Segment lesen bis erstes wieder kommt
    // (4) fehlende Segmente (s. last_section_number) versuchen zu lesen
    // Der Timeout gilt fuer jeden der 3 Abschnitte, d.h. maximal dauert
    // es 3 x timeout.
    // Mit readNext=0 werden nur aktuelle Sections gelesen (current_next_indicator = 1)
    int readSections(unsigned short pid, unsigned char filter, unsigned char mask, int readNext=0, unsigned timeoutInSeconds=10);
};

class SIsectionEIT : public SIsection
{
  public:
    SIsectionEIT(const SIsection &s) : SIsection(s) {
      parsed=0;
      parse();
    }
    // Std-Copy
    SIsectionEIT(const SIsectionEIT &s) : SIsection(s) {
      evts=s.evts;
      parsed=s.parsed;
    }

    unsigned short serviceID(void) const {
      return buffer ? ((struct SI_section_EIT_header *)buffer)->service_id : (unsigned short)-1;
    }
    unsigned short originalNetworkID(void) const {
      return buffer ? ((struct SI_section_EIT_header *)buffer)->original_network_id : (unsigned short)-1;
    }
    struct SI_section_EIT_header const *header(void) const {
      return buffer ? (struct SI_section_EIT_header *)buffer : (struct SI_section_EIT_header *)0;
    }
    static void dump(const struct SI_section_EIT_header *header) {
      SIsection::dump1((const struct SI_section_header *)header);
      printf("service_id: %hu\n", header->service_id);
      SIsection::dump2((const struct SI_section_header *)header);
      printf("transport_stream_id %hu\n", header->transport_stream_id);
      printf("original_network_id %hu\n", header->original_network_id);
      printf("segment_last_section_number: %hhu\n", header->segment_last_section_number);
      printf("last_table_id 0x%02hhx\n", header->last_table_id);
    }
    static void dump(const SIsectionEIT &s) {
      dump((struct SI_section_EIT_header *)s.buffer);
      for_each(s.evts.begin(), s.evts.end(), printSIevent());
    }
    void dump(void) const {
      dump((struct SI_section_EIT_header *)buffer);
      for_each(evts.begin(), evts.end(), printSIevent());
    }
    const SIevents &events(void) const {
//      if(!parsed)
//        parse(); -> nicht const
      return evts;
    }
  protected:
    SIevents evts;
    int parsed;
    void parse(void);
    void parseDescriptors(const char *desc, unsigned len, SIevent &e);
    void parseShortEventDescriptor(const char *buf, SIevent &e, unsigned maxlen);
    void parseExtendedEventDescriptor(const char *buf, SIevent &e, unsigned maxlen);
    void parseContentDescriptor(const char *buf, SIevent &e, unsigned maxlen);
    void parseComponentDescriptor(const char *buf, SIevent &e, unsigned maxlen);
    void parseParentalRatingDescriptor(const char *buf, SIevent &e, unsigned maxlen);
    void parseLinkageDescriptor(const char *buf, SIevent &e, unsigned maxlen);
};

// Fuer for_each
struct printSIsectionEIT : public std::unary_function<SIsectionEIT, void>
{
  void operator() (const SIsectionEIT &s) { s.dump();}
};

/*
// Fuer for_each
struct parseSIsectionEIT : public std::unary_function<SIsectionEIT, void>
{
  void operator() (const SIsectionEIT &s) { s.parse();}
};
*/

// Menge aller present/following EITs (actual TS)
class SIsectionsEIT : public std::set <SIsectionEIT, std::less<SIsectionEIT> >
{
  public:
    int readSections(void) {
      SIsections sections;
      int rc=sections.readSections(0x12, 0x4e, 0xff);
      for(SIsections::iterator k=sections.begin(); k!=sections.end(); k++)
        insert(*k);
      return rc;
    }
};

// Menge aller schedule EITs (actual TS)
class SIsectionsEITschedule : public std::set <SIsectionEIT, std::less<SIsectionEIT> >
{
  public:
    int readSections(void) {
      SIsections sections;
      int rc=sections.readSections(0x12, 0x50, 0xf0);
      for(SIsections::iterator k=sections.begin(); k!=sections.end(); k++)
        insert(*k);
      return rc;
    }
};

class SIsectionSDT : public SIsection
{
  public:
    SIsectionSDT(const SIsection &s) : SIsection(s) {
      parsed=0;
      parse();
    }
    // Std-Copy
    SIsectionSDT(const SIsectionSDT &s) : SIsection(s) {
      svs=s.svs;
      parsed=s.parsed;
    }
    unsigned transportStreamID(void) const {
      return buffer ? ((struct SI_section_SDT_header *)buffer)->transport_stream_id : (unsigned)-1;
    }
    struct SI_section_SDT_header const *header(void) const {
      return buffer ? (struct SI_section_SDT_header *)buffer : (struct SI_section_SDT_header *)0;
    }
    unsigned short originalNetworkID(void) const {
      return buffer ? ((struct SI_section_SDT_header *)buffer)->original_network_id : (unsigned short)-1;
    }
    static void dump(const struct SI_section_SDT_header *header) {
      SIsection::dump1((const struct SI_section_header *)header);
      printf("transport_stream_id: %hu\n", header->transport_stream_id);
      SIsection::dump2((const struct SI_section_header *)header);
      printf("original_network_id %hu\n", header->original_network_id);
    }
    static void dump(const SIsectionSDT &s) {
      dump((struct SI_section_SDT_header *)s.buffer);
      for_each(s.svs.begin(), s.svs.end(), printSIservice());
    }
    void dump(void) const {
      dump((struct SI_section_SDT_header *)buffer);
      for_each(svs.begin(), svs.end(), printSIservice());
    }
    const SIservices &services(void) const {
//      if(!parsed)
//        parse(); -> nicht const
      return svs;
    }
  private:
    SIservices svs;
    int parsed;
    void parse(void);
    void parseDescriptors(const char *desc, unsigned len, SIservice &s);
    void parseServiceDescriptor(const char *buf, SIservice &s);
    void parseNVODreferenceDescriptor(const char *buf, SIservice &s);
};

// Fuer for_each
struct printSIsectionSDT : public std::unary_function<SIsectionSDT, void>
{
  void operator() (const SIsectionSDT &s) { s.dump();}
};

// Menge aller SDTs (actual TS)
class SIsectionsSDT : public std::set <SIsectionSDT, std::less<SIsectionSDT> >
{
  public:
    int readSections(void) {
      SIsections sections;
      int rc=sections.readSections(0x11, 0x42, 0xff);
      for(SIsections::iterator k=sections.begin(); k!=sections.end(); k++)
        insert(*k);
      return rc;
    }
};

class SIsectionBAT : public SIsection
{
  public:
    SIsectionBAT(const SIsection &s) : SIsection(s) {}
    unsigned bouquetID(void) const {
      return buffer ? ((struct SI_section_BAT_header *)buffer)->bouquet_id : (unsigned)-1;
    }
    struct SI_section_BAT_header const *header(void) const {
      return buffer ? (struct SI_section_BAT_header *)buffer : (struct SI_section_BAT_header *)0;
    }
    static void dump(const struct SI_section_BAT_header *header) {
      SIsection::dump1((const struct SI_section_header *)header);
      printf("bouquet_id: %hu\n", header->bouquet_id);
      SIsection::dump2((const struct SI_section_header *)header);
      printf("bouquet_descriptors_length %hu\n", header->bouquet_descriptors_length);
    }
    static void dump(const SIsectionBAT &s) {
      dump((struct SI_section_BAT_header *)s.buffer);
    }
    void dump(void) const {
      dump((struct SI_section_BAT_header *)buffer);
    }
};

// Fuer for_each
struct printSIsectionBAT : public std::unary_function<SIsectionBAT, void>
{
  void operator() (const SIsectionBAT &s) { s.dump();}
};

// Menge aller BATs
class SIsectionsBAT : public std::set <SIsectionBAT, std::less<SIsectionBAT> >
{
  public:
    int readSections(void) {
      SIsections sections;
      int rc=sections.readSections(0x11, 0x4a, 0xff);
      for(SIsections::iterator k=sections.begin(); k!=sections.end(); k++)
        insert(*k);
      return rc;
    }
};

class SIsectionNIT : public SIsection
{
  public:
    SIsectionNIT(const SIsection &s) : SIsection(s) {}
    unsigned networkID(void) const {
      return buffer ? ((struct SI_section_NIT_header *)buffer)->network_id : (unsigned)-1;
    }
    struct SI_section_NIT_header const *header(void) const {
      return buffer ? (struct SI_section_NIT_header *)buffer : (struct SI_section_NIT_header *)0;
    }
    static void dump(const struct SI_section_NIT_header *header) {
      SIsection::dump1((const struct SI_section_header *)header);
      printf("network_id: %hu\n", header->network_id);
      SIsection::dump2((const struct SI_section_header *)header);
      printf("network_descriptors_length %hu\n", header->network_descriptors_length);
    }
    static void dump(const SIsectionNIT &s) {
      dump((struct SI_section_NIT_header *)s.buffer);
    }
    void dump(void) const {
      dump((struct SI_section_NIT_header *)buffer);
    }
};

// Fuer for_each
struct printSIsectionNIT : public std::unary_function<SIsectionNIT, void>
{
  void operator() (const SIsectionNIT &s) { s.dump();}
};

// Menge aller NITs (actual network)
class SIsectionsNIT : public std::set <SIsectionNIT, std::less<SIsectionNIT> >
{
  public:
    int readSections(void) {
      SIsections sections;
      int rc=sections.readSections(0x10, 0x40, 0xff);
      for(SIsections::iterator k=sections.begin(); k!=sections.end(); k++)
        insert(*k);
      return rc;
    }
};

#endif // SISECTIONS_HPP
