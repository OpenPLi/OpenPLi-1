#ifndef CCMDLINEREADER_H
#define CCMDLINEREADER_H

#include <iepgreader.h>
#include "optionparser.h"

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/
class cCmdLineReader : public iEpgReader
{
public:
    cCmdLineReader();

    ~cCmdLineReader();

    bool start();
    void showHelpMessage();
    bool parseCmdLine(int argc, char *argv[]);
private:
    enum OptIds
    {
        SERVICE_NAME,
        PROVIDER_NAME,
        SERVICE_REF,
        DESCRIPTION,
        TITLE,
        SUBTITLE,
        GENRE,
        START,
        DURATION
    };
    OptionParser parser;
    string error;
    string serviceName;
    string serviceRef;
    string providerName;
    string description;
    string title;
    string subtitle;
    int genre;
    int startTime;
    int duration;
    bool nameFound;
    bool serviceRefFound;
    bool providerFound;
    bool descriptionFound;
    bool titleFound;
    bool subtitleFound;
    bool genreFound;
    bool startFound;
    bool durationFound;
};

#endif
