#include "crtepgdata.h"
#include <time.h>
#include "util.h"
#include "estring.h"

#include <iostream>

cRtEpgData::cRtEpgData(const cGenreMap& map, const std::string& key)
: eitEventData(0)
, serviceKey(key)
, genreMap(map)
, year(0)
, premiere(false)
, film(false)
, repeat(false)
, subtitled(false)
, widescreen(false)
, new_series(false)
, deaf_signed(false)
, black_white(false)
, starRating(-1)
, certificate(-1)
, choice(false)
, duration(0)
, eventId(0)
, offset(0)
, totalLength(0)
, startTime(0)
, stopTime(0)
{
}

cRtEpgData::~cRtEpgData()
{
    if (eitEventData)
    {
        delete eitEventData;
        eitEventData = 0;
    }
}


bool cRtEpgData::setEventData(const std::string& data, int timeOffset, bool isAscii)
{
    std::string latin = data;
    if (!isAscii)
    {
        latin = convertUTF8DVB(data, 1);
    }
    string::size_type index = data.find('~');
    int previous = 0;
    while (index != string::npos)
    {
        strings.push_back( data.substr(previous, index-previous));
        previous = index + 1;
        index = data.find('~', previous);
    }
    strings.push_back( data.substr(previous));  //don't forget last one - not terminated with a '~'

    if (strings.size() == 23)
    {
        name        = strings[0];
        subtitle    = strings[1];
        episode     = strings[2];
        year        = atoi(strings[3].c_str());
        director    = strings[4];
        cast        = strings[5];
        premiere    = strings[6] == "true";
        film        = strings[7] == "true";
        repeat      = strings[8] == "true";
        subtitled   = strings[9] == "true";
        widescreen  = strings[10] == "true";
        new_series  = strings[11] == "true";
        deaf_signed = strings[12] == "true";
        black_white = strings[13] == "true";
        starRating  = atoi(strings[14].c_str());
        certificate = atoi(strings[15].c_str());
        genre       = strings[16];
        description = strings[17];
        choice      = strings[18] == "true";
        startdate   = strings[19];
        starttime   = strings[20];
        stoptime    = strings[21];
        duration    = atoi(strings[22].c_str());
        offset      = timeOffset;

        /* start time is given in UK local time (GMT or GMT+1)
        I need gmt time, so I convert the time as if it was a local time, and then
        account for the timezone difference.
        Because our daylight saving times/summer times move in sync, the difference
        remains constant. ==> this means we can use our local timezone to determine
        the offset.
        can this go wrong? esp during changeovers?
        */
        startTime = parseDateTime(startdate, starttime) + offset;
        stopTime = startTime + duration*60;
        return true;
    }

    return false;
}

bool cRtEpgData::getEitEventData(cEvent *&event)
{
    if (eitEventData)
    {
        delete eitEventData;
        eitEventData = 0;
    }
    eitEventData = new cEvent();
    if (!eitEventData)
        return false;

    eitEventData->setEventTimes(startTime, stopTime);
    eitEventData->setEventServiceKey(serviceKey);
    //eitEventData->setEventId(eventId);

    string desc = subtitle;
    if (subtitle.length() == 0 && episode.length() > 0)
    {
        //Make this a bit nicer - if episode is a title, leave it as it is, else if it is in form x/y, add 'Episode: '
        desc = episode;     //default
//        string::size_type pos = episode.find('/');
//         if (pos != string::npos)
//         {
//             int num = atoi(episode.substr(0, pos).c_str());
//             int max = atoi(episode.substr(pos+1).c_str());
//             if (num > 0 && max > 0)
//             {
//                 stringstream s;
//                 s << "Episode " << num << " of " << max;
//                 desc = s.str();
//             }
//         }
    }
    else if (episode.length() > 0)
    {
        description =  episode + "\212" + description;  //insert a CR/LF: control code 0x8A, 212 octal
    }
    eitEventData->addShortDescription(name, desc, "eng");
    eitEventData->addDescription(description, "eng");

    if (certificate > 0)
    {
        eitEventData->addRating(certificate, "GBR");
    }

    if (genre.length() > 0)
    {
        U8 g = genreMap.getContentType(genre, duration);
        eitEventData->addContentType(g);
    }
    event = eitEventData;
    return true;
}


