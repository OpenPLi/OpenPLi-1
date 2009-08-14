#include "xmlreader.h"
#include <string>
#include "event.h"
#include <fstream>
#include <iostream>
#include "cdbiface.h"
#include "util.h"
#include "readerfactory.h"
#include "xmlparser.h"
#include "estring.h"
using std::string;

#define XML_SOURCE_ID   0xf4

#define FILENAME_DESC       "<file>=xmltv file to convert"
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
/*
Mode of operation:
SAX based xml-parser
First collect all channel information
At the first programme tag, open the database, and retrieve
information for each channel

Then, parse each event separately, and store it immediately
During this time, the database remains locked.

Refinement:
as soon as we encounter a 'programme tag' open database. as soon as we encounter a 'channel'
tag, close it.  rinse, repeat...

On closer inspection... not possible. We can't rely on channel information being present, it it optional. SO no way to map channel name to service reference, instead, we need the channel id.
However, are we sure this is unique (it is unique in a single file, but can the id's change between runs?)?

*/
cXmlReader::cXmlReader()
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


cXmlReader::~cXmlReader()
{
}

void cXmlReader::OnNewNode(XMLTreeNode *node)
{
    if (node)
    {
        string progType;
        char *tmp = node->GetType();
        if (tmp) progType = tmp;
        /* is it a channel */
        /* in a channel: attr: id; child element: display-name */
        if (progType == "channel")
        {
            handleChannel(node);
        }
        if (progType == "programme")
        {
            handleEvent(node);
        }
    }
}

bool cXmlReader::start()
{
    if (mapFileFound)
    {
        channelMap.readFile(mapFile);
    }
    if (genreMapFound)
    {
        genreMap.readFile(genreMapFile);
    }

    std::string firstline;
    std::ifstream f(fileName.c_str());
    if (f.is_open())
    {
        getline(f, firstline);
        f.close();
    }
    std::string encoding("ISO-8859-1");
    std::string::size_type pos1 = firstline.find("encoding");
    std::string::size_type pos2 = pos1;
    if (pos1 != std::string::npos)
    {
        pos1 = firstline.find('=', pos1);
        pos2 = firstline.find('"', pos1+2);
        if (pos1 != std::string::npos && (pos2 > pos1+2))
        {
            encoding = firstline.substr(pos1+2, pos2-pos1-2);
        }
    }
    if (encoding.find("UTF") != std::string::npos)
    {
        encoding = "UTF-8";
    }
    else if (encoding.find("ISO-8859") != std::string::npos)
    {
        encoding = "ISO-8859-1";
    }
    else
    {
        encoding = "ISO-8859-1";
    }
    std::cout << "Setting encoding to " << encoding << std::endl;
    cXmlTvParser p(encoding);
    p.installCallbackHandler( this);
    if (cDbIface::getInstance()->startTransaction())
    {
        p.parseFile( fileName);
        cDbIface::getInstance()->endTransaction();
    }
    if (exportUnknown && mapFileFound)
    {
        channelMap.writeUnknownChannelMap(mapFile);
    }
    return true;
}

/* Allow user to switch between channel id's and channel names in channel manager!!*/
void cXmlReader::handleChannel(XMLTreeNode *node)
{
    char *tmp;
    string channelId;
    tmp = node->GetAttributeValue("id");
    if (tmp) channelId = tmp;

    string displayName(tmp);

    for (XMLTreeNode *child=node->GetChild(); child; child=child->GetNext() )
    //XMLTreeNode *child = node->GetChild();
    //if (child)
    {
        string type;
        tmp = child->GetType(); 
        if (tmp) type = tmp;
        if (type == "display-name")
        {
            tmp = child->GetData();
            if (tmp) displayName = convertUTF8DVB(std::string(tmp), codePage);
        }
    }
    if (channelId.length() > 0)
    {
        tChannelInfo& info = getChannelInfo(channelId);
        info.displayName = displayName;
    }
}

tChannelInfo& cXmlReader::getChannelInfo(const std::string& channelId)
{
    std::map<std::string, tChannelInfo>::iterator it = channels.find(channelId);
    if (it == channels.end())
    {
        channels.insert(std::pair<std::string, tChannelInfo> (channelId, tChannelInfo(channelId)));
        it = channels.find(channelId);
    }
    return it->second;
}

void cXmlReader::handleEvent(XMLTreeNode *node)
{
    char *tmp;
    string start; 
    string stop; 
    string channelId; 
    time_t startTime;
    time_t stopTime;
    tmp = node->GetAttributeValue("start");
    if (tmp) start = convertUTF8DVB(tmp, codePage);

    tmp = node->GetAttributeValue("stop");
    if (tmp) stop = convertUTF8DVB(tmp, codePage);

    tmp = node->GetAttributeValue("channel");
    if (tmp) channelId = convertUTF8DVB(tmp, codePage);

    //stopTime is implied.... need to read 2 items at least....?? simply reject those files.
    if (channelId.length() > 0 && analyseTime(start, startTime, offset) && analyseTime(stop, stopTime, offset) && stopTime > startTime)
    {
        string title;
        string subtitle;
        string description;
        string category;

        char *tmp;
        for (XMLTreeNode *child = node->GetChild(); child; child = child->GetNext())
        {
            string data;
            string type = child->GetType();
            tmp = child->GetData();
            if (tmp) data = convertUTF8DVB(tmp, codePage);  //convert UTF8 back to DVB stuff...

            if (type == "title")    //required, multiple
            {
                title = data;
            }
            else if (type == "desc")    //multiple
            {
                description = data;
            }
            else if (type == "category")    //multiple
            {
                category = data;
            }
            else if (type == "sub-title")   //multiple
            {
                subtitle = data;
            }
        }
        if (title.length() > 0)
        {
            //get EventID for this channel?
            cEvent *ev = createEvent(title, subtitle, description, category, startTime, stopTime);
            if (ev)
            {
                storeEvent(channelId, ev);
                delete ev;
            }
        }
    }
}

void cXmlReader::storeEvent(const std::string& channelId, cEvent* event)
{
    string key;
    string fullProvider;

    tChannelInfo& info = getChannelInfo(channelId);
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
                if (cDbIface::getInstance()->getNextEventInfo(key, XML_SOURCE_ID, dbStartTime, dbStopTime))
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
            storeData(event,  XML_SOURCE_ID);
        }
    }
}

cEvent* cXmlReader::createEvent(const std::string& title,
                            const std::string& subtitle,
                            const std::string& description,
                            const std::string& genre,
                            time_t start,
                            time_t stop)
{
    cEvent *event = new cEvent();
    event->addShortDescription(title, subtitle, language, codePage);
    event->addDescription(description, language, codePage);
    event->addContentType(genreMap.getContentType(genre, stop - start));

    event->setEventTimes(start, stop);
    return event;
}



bool cXmlReader::parseCmdLine(int argc, char *argv[])
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
        "XMLTV file format\n" \
        "Options:\n" 


void cXmlReader::showHelpMessage()
{
    std::cout << HELP_MESSAGE;
    parser.printOptionDescriptions();
    std::cout << std::endl;
}

static iEpgReader* createXmlReader() { return new cXmlReader(); }

cReaderFactory::tReaderType XmlReader( "XMLTV Converter", "xmltv", &createXmlReader);



