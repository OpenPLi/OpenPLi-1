#ifndef CRTEPGDATA_H
#define CRTEPGDATA_H

#include <sstream>
#include <string>
#include <vector>
#include "event.h"
#include "cgenremap.h"
using namespace std;

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/


class cRtEpgData
{
public:
    cRtEpgData(const cGenreMap& map, const std::string& key);

    ~cRtEpgData();

    bool setEventData(const std::string& data, int timeOffset, bool isAscii);
    void setEventId(int id) { eventId = id; }
    time_t getStartTime() const { return startTime; }
    time_t getStopTime() const { return stopTime; }
    bool getEitEventData(cEvent *&event);

private:
    cEvent *eitEventData;
    string serviceKey;
    vector<string> strings;
    const cGenreMap& genreMap;

    string name;             //0: Main title  Match of the Day: Community Shield
    string subtitle;         //1: 2nd title   Chelsea v Liverpool
    string episode;          //2: episode (name or something like 3/8)
    int    year;             //3: year of production (films)
    string director;         //4: director
    string cast;             //5: actors (sometimes character*actor's name pairs separated with |  e.g. Jack Wells*Bill Pullman|Kelly Scott*Bridget Fonda)
    bool   premiere;         //6:
    bool   film;             //7:
    bool   repeat;           //8:
    bool   subtitled;        //9:
    bool   widescreen;       //10:
    bool   new_series;       //11:
    bool   deaf_signed;      //12:
    bool   black_white;      //13:
    int    starRating;       //14:
    int    certificate;      //15:  could be age, or a string like 'PG'
    string cert;            
    string genre;            //16: genre
    string description;      //17: description
    bool   choice;           //18:
    string startdate;        //19: startdate (dd/mm/yyy)
    string starttime;        //20: starttime (hh:mm)
    string stoptime;         //21: stoptime (hh:mm)
    int    duration;         //22: duration (minutes)

    int eventId;
    int offset;
    int totalLength;
    time_t startTime;
    time_t stopTime;
};

#endif
