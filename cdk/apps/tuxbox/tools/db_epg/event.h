#ifndef EVENT_H
#define EVENT_H
#include <string>
#include <list>
#include <time.h>
#include "iepgdata.h"

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/

/* An event class is a simple container and access class to manipulate
   the properties of an event.

   The database expects objects of this type, and extracts the eit_event
   structures from it
*/
class cEvent
{
public:
    cEvent();
    ~cEvent();

    void setEventTimes(time_t start, time_t stop)   { startTime = start; stopTime = stop; eventId = (startTime/60) & 0xffff; };
    void setEventServiceKey(const std::string& key) { serviceKey = key; }

    std::string getKey()    const { return serviceKey; }
    int getEventId()        const { return eventId; }
    time_t getStartTime()   const { return startTime; }
    time_t getStopTime()    const { return stopTime; }
    int getEventDataLen()   const { return byteLength + 12; }   //12 byte header

    void getEventData(U8 *eventData, int maxLen) const;

    void addShortDescription(const std::string& title,
                             const std::string& description,
                             const std::string& langCode = "eng",
                             int codePage = 0);

    void addDescription(const std::string& description,
                        const std::string& lang = "eng",
                        int                codepage = 0);

    /* Parental guidance: age is the minimum required age minus _3_ to watch this show.*/
    void addRating(int age, const std::string& countrycode);
    void addContentType(U8 type);


private:
    time_t getDuration() const { return stopTime - startTime; }

    std::list<BaseDesc*> descriptors;   //the list of eit descriptors, used to build eit_event data
    std::string serviceKey;
    time_t      startTime;
    time_t      stopTime;
    int         eventId;
    int         byteLength;

};

#endif
