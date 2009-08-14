#include "event.h"
#include "util.h"
#ifdef DEBUG
#include "si.h"
#endif
#include <iostream>
cEvent::cEvent()
: startTime(0)
, stopTime(0)
, eventId(0)
, byteLength(0)
{
}


cEvent::~cEvent()
{
    for (std::list<BaseDesc*>::iterator it=descriptors.begin(); it != descriptors.end(); ++it)
    {
        delete (*it);
    }
}

void cEvent::addShortDescription(const std::string& title,
                                 const std::string& description,
                                 const std::string& langCode,
                                 int                codePage)
{
    ShortDesc *s = new ShortDesc(title, description, langCode, codePage);
    descriptors.push_back(s);

    byteLength += s->getLen();
}


void cEvent::addDescription(const std::string& description,
                            const std::string& langCode,
                            int                codePage)
{
    std::list<LongDesc*> tmpList;
    int cnt = 0;
    std::string::size_type charCnt = 0;
    std::string::size_type lastCnt = 1;
    while (charCnt < description.length() && lastCnt > 0)
    {
        LongDesc *l = new LongDesc(cnt);
        lastCnt = l->addDescription(description.substr(charCnt, 255), codePage); //skip first bytes, LongDesc will cut of remainder
        charCnt += lastCnt;
        cnt++;
        tmpList.push_back(l);
        byteLength += l->getLen();
    }
    for (std::list<LongDesc*>::const_iterator i = tmpList.begin(); i != tmpList.end(); i++)
    {
        (*i)->setIndexNumbers(cnt-1);
        descriptors.push_back(*i);
    }
}
void cEvent::addRating(int age, const std::string& countrycode)
{
    ParentalDesc *parental = new ParentalDesc(age, countrycode);
    descriptors.push_back(parental);
    byteLength += parental->getLen();
}

void cEvent::addContentType(U8 type)
{
    ContentDesc *content = new ContentDesc(type, 0);
    descriptors.push_back(content);
    byteLength += content->getLen();
}

/* Method for database */
void cEvent::getEventData(U8 *eventData, int maxLen) const
{
    /* Count required bytes */
    int len = 12 + byteLength;   //header is 12 bytes
#ifdef DEBUG
    int tstLen = 12;
    for (std::list<BaseDesc*>::const_iterator i=descriptors.begin(); i!=descriptors.end(); i++)
    {
        tstLen += (*i)->getLen();
    }
    assert(len == tstLen);
#endif
    assert(len <= maxLen);

    eventData[0] = eventId >> 8;                //HI eventId
    eventData[1] = eventId & 0xff;              //LO eventID
    toDVBTime(&eventData[2], startTime);        //data[2].. data[6]: dvb time
    toBCDTime(&eventData[7], getDuration());         //data[7]..data[9]: duration (3x BCD)
    eventData[10] = (2<<4);                      //status - not running
    int loop_length = len - 12;          //number of bytes following this field
    eventData[10] |= ((loop_length  >> 8) & 0x0F);   //data[10]: 0xF0: ca, status; 0x0F: HI looplength
    eventData[11] = loop_length  & 0xFF;             //LO looplength

#ifdef DEBUG
    time_t testTime = parseDVBtime( eventData[2], eventData[3], eventData[4], eventData[5], eventData[6]);
    assert(testTime == startTime);
#endif

    // copy all descriptors to buffer, and make sure we don't overwrite our fancy eit event header :)
    int position = 12;
    int remainingLength = len-12;
    for(std::list<BaseDesc*>::const_iterator it = descriptors.begin(); it != descriptors.end(); ++it)
    {
        int l;
        l = (*it)->copyData(&eventData[position], remainingLength);
        remainingLength -= l;
        position += l;
    }
}

