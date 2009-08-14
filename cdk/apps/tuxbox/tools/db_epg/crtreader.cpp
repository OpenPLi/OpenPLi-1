#include <cstdlib>

#include <iostream>
#include <fstream>

#include "readerfactory.h"

#include "crtepgdata.h"
#include "crtreader.h"
#include "cdbiface.h"


#define RT_SOURCE_ID    0xf0
#define FILENAME_DESC     "<filename>=file to convert"
#define SERVICE_REF_DESC  "<service_reference>=format \"sid:onid:tsid\", without leading 0's or 0x"
#define SERVICE_NAME_DESC "<service_name>=name of service"
#define PROVIDER_DESC     "<provider_name>=name of the provider"
#define TIME_OFFSET_DESC  "<offset>=time offset in seconds (defaults to 0)"
#define HOURS_DESC        "<hours>=number of hours to store (default 0: no limit)"
#define GENRE_DESC        "<file>=file with a list of genre strings (format <genre string>,<value>[,min duration])"
#define IS_ASCII_DESC     "set this option if input file is not in UTF-8"

cRadioTimesReader::cRadioTimesReader()
 : timeOffset(0)
 , hours(0)
 , nameFound(false)
 , providerFound(false)
 , serviceRefFound(false)
 , fileFound(false)
 , timeOffsetFound(false)
 , hoursFound(false)
 , genreMapFound(false)
 , isAscii(false)
{
    assert(parser.registerOption(FILENAME,      'f', "file"     , FILENAME_DESC));
    assert(parser.registerOption(SERVICE_NAME,  'n', "name"     , SERVICE_NAME_DESC));
    assert(parser.registerOption(SERVICE_REF,   'r', "reference", SERVICE_REF_DESC));
    assert(parser.registerOption(PROVIDER,      'p', "provider" , PROVIDER_DESC));
    assert(parser.registerOption(HOURS,         'h', "hours"    , HOURS_DESC));
    assert(parser.registerOption(TIME_OFFSET,   'o', "offset"   , TIME_OFFSET_DESC));
    assert(parser.registerOption(GENREFILE,     'g', "genre"    , GENRE_DESC));
    assert(parser.registerOption(IS_ASCII,      'a', "ascii"    , IS_ASCII_DESC));
}

cRadioTimesReader::~cRadioTimesReader()
{
}
/* Arguments passed in at command line: uniqueEpgKey data (serviceID, onid, tsid), */
bool cRadioTimesReader::start()
{
    string key;
    string fullProvider;
    if (nameFound && ! serviceRefFound)
    {
        cDbIface::getInstance()->getServiceInfo( serviceName, provider, key, fullProvider);
    }
    if (genreMapFound)
    {
        genreMap.readFile(genreMapFile);
    }
    if (serviceRefFound)
    {
        key = serviceRef;
    }
    if (key != "")
    {
        time_t dbStartTime = 0;
        time_t dbStopTime = 0;
        if (cDbIface::getInstance()->getNextEventInfo(key, RT_SOURCE_ID, dbStartTime, dbStopTime))
        {
            fstream f(fileName.c_str());
            string tmp;
            if (f.is_open())
            {
                long begin = f.tellg();
                f.seekg (0, ios::end);
                long end = f.tellg();
                f.seekg(0, ios::beg);

                if (cDbIface::getInstance()->startTransaction())
                {
                    bool success = true;
                    while (!f.eof() && getline(f, tmp) && success)
                    {
                        long cur = f.tellg();
                        int pos = (100*(cur-begin) / (end-begin));   //truncate 0..99
                        if ((pos % 10) == 0) std::cout << "#pos:" << pos << std::endl;
                        //do not store cRtEpgData instances, these are only used to create the appropriate cEvent instances...
                        cRtEpgData data(genreMap, key);   //temporary object to convert data to cEvent

                        if (data.setEventData(tmp, timeOffset, isAscii))
                        {
                            time_t t;
                            time(&t);   //localtime
                            time_t start = data.getStartTime() ;
                            time_t stop = data.getStopTime();
                            //Get 3 days data, only insert items which are not yet in db
                            //need to check: is there an event which overlaps with this one?
                            //overlap: starttimes are equal, or existing event A.start < this->start < A.end

                            if ((stop > t) &&
                                (start > dbStartTime) &&
                                (start >= dbStopTime) &&
                                (start < (t+3600*hours) || hours == 0))
                            {
                                cEvent* event = 0;
                                if (data.getEitEventData(event))
                                {
                                    success = storeData(event, RT_SOURCE_ID);
                                }
                            }
                        }
                        else
                        {
                            cout << "error setting data" << endl;
                        }
                    }
                    cDbIface::getInstance()->endTransaction();
                }
                f.close();
            }
            return true;
        }
    }
    return false;
}

bool cRadioTimesReader::parseCmdLine(int argc, char *argv[])
{
    parser.parseOptions(argc-1, &argv[1]);

    nameFound       = parser.getOptionValue(SERVICE_NAME, serviceName);
    serviceRefFound = parser.getOptionValue(SERVICE_REF,  serviceRef);
    providerFound   = parser.getOptionValue(PROVIDER,     provider);
    fileFound       = parser.getOptionValue(FILENAME,     fileName );
    hoursFound      = parser.getOptionValue(HOURS,        hours);
    timeOffsetFound = parser.getOptionValue(TIME_OFFSET,  timeOffset);
    genreMapFound   = parser.getOptionValue(GENREFILE,    genreMapFile);
    isAscii         = parser.getOptionFound(IS_ASCII);

    return (fileFound && (serviceRefFound || nameFound));
}

#define HELP_MESSAGE_HEAD \
        "Radiotimes file format\n" \
        "Options:"
#define HELP_MESSAGE_TAIL \
        "Either a service reference or a service name must be given.\n" \
        "If no service reference is provided, the first service with the exact same\n" \
        "name will be used. If there are multiple services with the same name, \n"\
        "you can provide the start of the provider name to select the correct service.\n"


void cRadioTimesReader::showHelpMessage()
{
    cout << HELP_MESSAGE_HEAD << endl;
    parser.printOptionDescriptions();
    cout << HELP_MESSAGE_TAIL << endl;
}

static iEpgReader* createRtReader() { return new cRadioTimesReader(); }

cReaderFactory::tReaderType Radiotimes( "UK Radiotimes", "uk_rt", &createRtReader);

