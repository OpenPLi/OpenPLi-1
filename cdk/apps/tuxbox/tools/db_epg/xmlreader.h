#ifndef XMLREADER_H
#define XMLREADER_H

#include "iepgreader.h"
#include "optionparser.h"
#include "xmltree.h"
#include "cgenremap.h"
#include "cchannelmap.h"
#include "xmlparser.h"

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/
class cXmlReader : public iEpgReader,
    public cXmlTvParser::cXmlHandlerCb
{
public:
    cXmlReader();

    ~cXmlReader();

    bool start();
    bool parseCmdLine(int argc, char *argv[]);
    void showHelpMessage();

    void OnNewNode(XMLTreeNode *node);

private:
    enum OptIds
    {
        FILENAME,
        LANGUAGE,
        HOURS,
        OFFSET,
        PROVIDER,
        MAPFILE,
        GENREFILE,
        MATCHNAME,
        EXPORT_UNKNOWN,
        CODEPAGE        //correct xml should specify this, but keep it anyway
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

    void handleChannel(XMLTreeNode *node);
    void handleEvent(XMLTreeNode *node);

    /* Create an event */
    cEvent* createEvent(const std::string& title,
                     const std::string& subtitle,
                     const std::string& description,
                     const std::string& genre,
                     time_t start,
                     time_t stop);
    void storeEvent(const std::string& channelId, cEvent* event);

    tChannelInfo& getChannelInfo(const std::string& channelId);
    bool readAndParseBuffer(std::string filename, XMLTreeParser *parser);

};

#endif
