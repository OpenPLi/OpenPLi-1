#include "ccmdlinereader.h"
#include "readerfactory.h"
#include "iepgdata.h"
#include "cdbiface.h"
#include "event.h"
#include <iostream>
#include <cstdio>

#define CMD_SOURCE_ID    0xf1
#define SERVICE_REF_DESC  "<service_reference>=format \"sid:onid:tsid\", without leading 0's or 0x"
#define SERVICE_NAME_DESC "<service_name>=name of service"
#define PROVIDER_DESC     "<provider_name>=name of the provider"
#define START_DESC        "<starttime>=start time"
#define DURATION_DESC     "<duration>=duration in minutes"
#define TITLE_DESC        "<title>=name of the event"
#define DESCRIPTION_DESC  "<description>=long description"
#define SUBTITLE_DESC     "<sub>=short description/substitle"
#define GENRE_DESC        "<genre>=numeric value (see below)"
cCmdLineReader::cCmdLineReader()
 : iEpgReader()
 , genre(0)
 , startTime(0)
 , duration(0)
 , nameFound(false)
 , serviceRefFound(false)
 , providerFound(false)
 , descriptionFound(false)
 , titleFound(false)
 , subtitleFound(false)
 , genreFound(false)
 , startFound(false)
 , durationFound(false)
{
    assert(parser.registerOption(SERVICE_NAME,  'n', "name"      , SERVICE_NAME_DESC));
    assert(parser.registerOption(PROVIDER_NAME, 'p', "provider"  , PROVIDER_DESC));
    assert(parser.registerOption(SERVICE_REF,   'r', "serviceref", SERVICE_REF_DESC));
    assert(parser.registerOption(TITLE,           0, "title"     , TITLE_DESC));
    assert(parser.registerOption(SUBTITLE,        0, "subtitle"  , SUBTITLE_DESC));
    assert(parser.registerOption(DESCRIPTION,     0, "desc"      , DESCRIPTION_DESC));
    assert(parser.registerOption(GENRE,           0, "genre"     , GENRE_DESC));
    assert(parser.registerOption(START,           0, "start"     , START_DESC));
    assert(parser.registerOption(DURATION,        0, "duration"  , DURATION_DESC));
}


cCmdLineReader::~cCmdLineReader()
{
}

bool cCmdLineReader::start()
{
    bool res = false;
    //simply create an epgdata from the string....
    cEvent *eventData = new cEvent();
    if (titleFound) eventData->addShortDescription(title, subtitle);    //initialized to "", so this is safe
    if (descriptionFound) eventData->addDescription(description);
    if (genreFound) eventData->addContentType(genre);
    if (startFound && durationFound) eventData->setEventTimes(startTime, startTime + duration*60);

    std::string key;
    std::string fullProvider;
    if (nameFound && ! serviceRefFound)
    {
        cDbIface::getInstance()->getServiceInfo( serviceName, providerName, key, fullProvider);
    }
    if (serviceRefFound)
    {
        key = serviceRef;
    }
    if (key != "")
    {
         time_t t;
         time(&t);   //localtime
         time_t stop = startTime + duration;
         if (stop > t)
         {
             eventData->setEventServiceKey(key);
             res = storeData(eventData, CMD_SOURCE_ID);
         }
    }
    return res;
}


bool cCmdLineReader::parseCmdLine(int argc, char *argv[])
{
    /* skip first argument - program name */
    if (parser.parseOptions( argc-1, &argv[1]))
    {
        nameFound        = parser.getOptionValue(SERVICE_NAME,  serviceName);
        serviceRefFound  = parser.getOptionValue(SERVICE_REF,   serviceRef);
        providerFound    = parser.getOptionValue(PROVIDER_NAME, providerName);
        descriptionFound = parser.getOptionValue(DESCRIPTION,   description);
        titleFound       = parser.getOptionValue(TITLE,         title);
        subtitleFound    = parser.getOptionValue(SUBTITLE,      subtitle);
        genreFound       = parser.getOptionValue(GENRE,         genre);
        startFound       = parser.getOptionValue(START,         startTime);
        durationFound    = parser.getOptionValue(DURATION,      duration);
    }

    return (startFound && durationFound && titleFound && (serviceRefFound || nameFound));
}

#define HELP_MESSAGE_HEAD \
        "Command line reader\n" \
        "Options:"
#define HELP_MESSAGE_TAIL \
        "If no service reference is provided, the first service with the exact same\n" \
        "name will be used. If there are multiple services with the same name, \n"\
        "you can provide the start of the provider name to select the correct service.\n" \
        "Either a service reference or a name must be given.\n" \
        "Starttime and stoptime are standard unix timestamps relative UTC,\n"\
        "The genre is an exadecimal representation of the content_nibble_level 1 and 2,\n"\
        "as described in ETSI EN 300 468 v1.4.1. Some general values are: \n" \
        "10 - movies\n"\
        "20 - news\n"\
        "30 - shows and game shows\n"\
        "40 - sports\n"\
        "50 - children's programmes\n"\
        "60 - music and dance\n"\
        "70 - arts without music\n"\
        "80 - social, political and economics\n"\
        "90 - education, science and factual topics\n"

void cCmdLineReader::showHelpMessage()
{
    std::cout << HELP_MESSAGE_HEAD << std::endl;
    parser.printOptionDescriptions();
    std::cout << HELP_MESSAGE_TAIL << std::endl;
}

static iEpgReader* createCmdReader() { return new cCmdLineReader(); }

cReaderFactory::tReaderType CmdLine( "Command line reader", "cmd", &createCmdReader);




