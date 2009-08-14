#ifndef CEPGDATREADER_H
#define CEPGDATREADER_H

#include "iepgreader.h"
#include "optionparser.h"
#include "cgenremap.h"
#include "cchannelmap.h"

typedef unsigned char U8;

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/
class cEpgDatReader : public iEpgReader
{
public:
    cEpgDatReader();

    ~cEpgDatReader();

    bool start();
    bool parseCmdLine(int argc, char *argv[]);
    void showHelpMessage();

private:
    enum OptIds
    {
        FILENAME,
        OFFSET,
        LANGUAGE,
        HOURS,
        PROVIDER,
        MAPFILE,
        GENREFILE,
        MATCHNAME,
        EXPORT_UNKNOWN,
        CODEPAGE
    };
    OptionParser parser;
    cGenreMap genreMap;
    cChannelMap channelMap;

    std::string fileName;
    std::string language;
    std::string provider;
    std::string mapFile;
    std::string genreMapFile;
    std::string exportUnknownFile;
    int offset;
    int hours;
    bool fileFound;
    bool offsetFound;
    bool langFound;
    bool providerFound;
    bool mapFileFound;
    bool genreMapFound;
    bool matchName;
    bool exportUnknown;
    int codePage;
    int entryCnt;

    std::map<std::string, tChannelInfo> channels;
    tChannelInfo& getChannelInfo(const std::string& channelId);

    cEvent* createEvent(const std::string& title,
                    const std::string& description,
                    const std::string& genre,
                    int start,
                    int duration);

    void storeEvent(const std::string& channelName, cEvent *event);
};

#endif
