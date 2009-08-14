#include "cepgdatreader.h"
#include "readerfactory.h"
#include "cdbiface.h"
#include "event.h"

#include <iostream>
#include <fstream>


# define EPGDAT_SOURCE_ID   0xf2
#define FILENAME_DESC       "<file>=epg.dat file to convert"
#define MAPFILE_DESC        "<file>=file with service references for each channel"
#define SERVICE_REF_DESC    "<sid:onid:tsid>=service reference if this file contains only 1 channel"
#define TIME_OFFSET_DESC    "<offset>=time offset in seconds (defaults to 0)"
#define HOURS_DESC          "<hours>=number of hours to store (default 0: no limit)"
#define LANGUAGE_DESC       "<code>=language code (according to ISO 639-2)"
#define PROVIDER_DESC       "<provider>=start of provider name"
#define GENRE_DESC          "<file>=file with a list of genre strings (format <genre string>,<value>[,min duration])"
#define MATCHNAME_DESC      "if enabled, tries to match unknown channelnames with entries in database"
#define EXPORT_UNKNOWN_DESC "append mapfile with all unknown channels"
#define CODEPAGE_DESC       "<num>=iso8859 codepage, defaults to 0"
#define CODE_UTF8_DESC      "set if input is encoded in UTF8"


cEpgDatReader::cEpgDatReader()
 : iEpgReader()
 , language("eng")
 , offset(0)
 , hours(0)
 , fileFound(false)
 , offsetFound(false)
 , langFound(false)
 , providerFound(false)
 , mapFileFound(false)
 , genreMapFound(false)
 , matchName(false)
 , exportUnknown(false)
 , codePage(0)
 , entryCnt(0)
{
    assert(parser.registerOption( FILENAME, 'f', "file",    FILENAME_DESC));
    assert(parser.registerOption( OFFSET,   'o', "offset",  TIME_OFFSET_DESC));
    assert(parser.registerOption( HOURS,    'h', "hours",   HOURS_DESC));
    assert(parser.registerOption( MAPFILE,  'm', "map",     MAPFILE_DESC));
    assert(parser.registerOption( LANGUAGE, 'l', "lang",    LANGUAGE_DESC));
    assert(parser.registerOption( PROVIDER, 'p', "provider",PROVIDER_DESC));
    assert(parser.registerOption( GENREFILE,'g', "genre",   GENRE_DESC));
    assert(parser.registerOption( MATCHNAME,  0, "matchname", MATCHNAME_DESC));
    assert(parser.registerOption( EXPORT_UNKNOWN, 'e', "export", EXPORT_UNKNOWN_DESC));
    assert(parser.registerOption( CODEPAGE, 'c', "cp", CODEPAGE_DESC));
}


cEpgDatReader::~cEpgDatReader()
{
}


bool cEpgDatReader::start()
{
    if (mapFileFound)
    {
        channelMap.readFile(mapFile);
    }
    if (genreMapFound)
    {
        genreMap.readFile(genreMapFile);
    }

    std::ifstream f(fileName.c_str());
    std::string line;
    std::string channelName;
    std::string title;
    std::string description;
    std::string genre;
    int eventId = 0;
    int startTime = 0;
    int duration = 0;
    int channelNum = 0;
    int fileOffset = 0;
    int entryLength = 0;
    bool inEntry = false;
    bool inChannel = false;

    if (f.is_open())
    {
        long begin = f.tellg();
        f.seekg (0, std::ios::end);
        long end = f.tellg();
        f.seekg(0, std::ios::beg);

        if (cDbIface::getInstance()->startTransaction())
        {

            while (!f.eof() && getline(f, line))
            {
                long cur = f.tellg();
                int fpos = (100*(cur-begin) / (end-begin));   //truncate 0..99
                if ((fpos % 10) == 0) std::cout << "#pos:" << fpos << std::endl;
                bool hasVal = (line.length() > 2);  //Identifier, followed by a space, and the value itself...
                std::string val;
                if (hasVal)
                    val =  line.substr(2);
    
                std::string::size_type pos = 0;
                std::string::size_type pos2 = 0;
                switch (line[0])
                {
                    case 'C':       //start of a channel
                        inChannel = true;
                        channelName = "";
                        channelNum = 0;
                        if (hasVal)
                        {
                            pos = val.find(' ');
                            channelNum = atoi(val.substr(0, pos).c_str());  //can't rely on this being unique or consistent between runs
                            channelName = val.substr(pos+1);
                            if (channelName .length() > 0)
                            {
                                tChannelInfo& info = getChannelInfo(channelName);
                                info.displayName = channelName;
                            }
                        }
                        break;
                    case 'E':
                        inEntry = inChannel;    //can only be in an entry if we're already in a channel...
                        title = "";
                        description = "";
                        genre = "";
                        eventId = 0;
                        fileOffset = 0;
                        entryLength = 0;
                        startTime = 0;
                        duration = 0;
    
                        if (hasVal)
                        {
                            pos = val.find(' ');
                            if (pos != std::string::npos)
                            {
                                eventId = atoi(val.substr(0, pos).c_str());
                                pos2 = val.find(' ', pos);
                                if (pos2 != std::string::npos)
                                {
                                    fileOffset = atoi(val.substr(pos+1, pos2-pos).c_str());
                                    entryLength = atoi(val.substr(pos2+1).c_str());
                                }
                            }
                        }
                        break;
                    case 'T':
                        if (hasVal)
                        {
                            title = val;
                        }
                        break;
                    case 'X':
                        if (hasVal)
                        {
                            startTime = atoi(val.c_str()) + offset;
                        }
                        break;
                    case 'S':
                        if (hasVal)
                        {
                            genre = val;
                        }
                        break;
                    case 'L':
                        if (hasVal)
                        {
                            duration = atoi(val.c_str());
                        }
                        //make sure this is a valid duration...
                        //if possible, check with offsets given in entry...
                        break;
                    case 'D':
                        if (hasVal)
                        {
                            description = val;
                        }
                        break;
                    case 'e':
                    {
                        /* first checks to see if this entry is valid (createEvent will do some more)*/
                        /* do we have offsets? if so, is their difference equal to the duration?*/
                        /* do we have an eventId - if not, let db decide..*/
                        /* */
                        bool valid = inEntry;
                        entryCnt++;
                        if (duration == 0 && entryLength == 0)
                        {
                            valid = false;
                        }
                        if (duration == 0 && entryLength > 0)
                            duration = entryLength;
    
                        if (entryLength > 0 && valid)
                        {
                            valid = (duration*60 <= entryLength); //next event cannot start before this one is over
                        }
                        if (valid && startTime != 0 && duration > 0 && title.length() > 0)
                        {
                            cEvent *event = createEvent(title, description, genre, startTime, duration);
                            if (event)
                            {
                                storeEvent(channelName, event);
                                delete event;
                            }
                        }
                        else
                        {
                            std::cout <<"Entry " << entryCnt << " invalid" << std::endl;
                        }
                        inEntry = false;
                        break;
                    }
                    case 'c':
                        inChannel = false;
                        break;
                }
            }
            cDbIface::getInstance()->endTransaction();
        }

        if (exportUnknown && mapFileFound)
        {
            channelMap.writeUnknownChannelMap(mapFile);
        }
        return true;
    }
    return false;
}

tChannelInfo& cEpgDatReader::getChannelInfo(const std::string& channelId)
{
    std::map<std::string, tChannelInfo>::iterator it = channels.find(channelId);
    if (it == channels.end())
    {
        channels.insert(std::pair<std::string, tChannelInfo> (channelId, tChannelInfo(channelId)));
        it = channels.find(channelId);
    }
    return it->second;
}

void cEpgDatReader::storeEvent(const std::string& channelId, cEvent* event)
{
    string key;
    string fullProvider;

    tChannelInfo& info = getChannelInfo(channelId);
    //if lastEventId != -1, then we already retrieved data...
    time_t t;
    time(&t);   //localtime

    if (info.alreadySeen && !info.unknownChannel)
    {
        key = info.serviceKey;
    }
    else if (!info.alreadySeen)
    {
        info.alreadySeen = true;
        info.unknownChannel = true;

        std::string reference;
        if (channelMap.findChannelReference(channelId, reference))
        {
            if (reference != "UNKNOWN")
            {
                key = reference;
            }
        }
        else
        {
            channelMap.addUnknownChannel(channelId); // mapChanNameToRef[channelName] = "UNKNOWN";
            if (matchName)
            {
                cDbIface::getInstance()->getServiceInfo(channelId, provider, key, fullProvider);
                if (key != "")
                {
                    channelMap.addTemporaryChannel(channelId, key);
                }
            }
        }

        if (key != "")
        {
            info.unknownChannel = false;
            info.serviceKey = key;
            if (info.startTime == 0)
            {
                time_t dbStartTime, dbStopTime;
                if (cDbIface::getInstance()->getNextEventInfo(key, EPGDAT_SOURCE_ID, dbStartTime, dbStopTime))
                {
                    if (dbStartTime == 0)
                    {
                        dbStartTime = t;
                        dbStopTime = t;
                    }
                    info.startTime = dbStartTime;
                    info.stopTime = dbStopTime;
                }
            }
        }
    }

    if (key != "")
    {
        time_t start = event->getStartTime();
        time_t stop  = event->getStopTime();
        if ((stop > info.stopTime) &&
             (start > info.startTime) &&
             (start >= info.stopTime) &&
             (start < (t+3600*hours) || hours == 0))
        {
            event->setEventServiceKey(key);
            storeData(event,  EPGDAT_SOURCE_ID);
        }
    }
}

cEvent* cEpgDatReader::createEvent(const std::string& title,
                                const std::string& description,
                                const std::string& genre,
                                int start,
                                int duration)
{
    cEvent *event = new cEvent();
    event->addShortDescription(title, "", language, codePage);
    event->addDescription(description, language, codePage);
    event->addContentType(genreMap.getContentType(genre, duration));
    event->setEventTimes(start, start+(duration * 60));
    return event;
}


bool cEpgDatReader::parseCmdLine(int argc, char *argv[])
{
    if (parser.parseOptions( argc-1, &argv[1]))
    {
        fileFound     = parser.getOptionValue(FILENAME, fileName);
        offsetFound   = parser.getOptionValue(OFFSET,   offset);
        langFound     = parser.getOptionValue(LANGUAGE, language);
        providerFound = parser.getOptionValue(PROVIDER, provider);
        mapFileFound  = parser.getOptionValue(MAPFILE,  mapFile);
        genreMapFound = parser.getOptionValue(GENREFILE,genreMapFile);
        matchName     = parser.getOptionFound(MATCHNAME);
        exportUnknown = parser.getOptionFound(EXPORT_UNKNOWN);
        parser.getOptionValue(CODEPAGE, codePage);
        parser.getOptionValue( HOURS, hours);
    }
    return fileFound;
}


#define HELP_MESSAGE \
        "epg.dat file format\n" \
        "Options:\n" 


void cEpgDatReader::showHelpMessage()
{
    std::cout << HELP_MESSAGE;
    parser.printOptionDescriptions();
    std::cout << std::endl;
}

static iEpgReader* createEpgDatReader() { return new cEpgDatReader(); }

cReaderFactory::tReaderType EpgDat( "epg.dat Converter", "epgdat", &createEpgDatReader);
