#ifndef CRTREADER_H
#define CRTREADER_H

#include "iepgreader.h"
#include "optionparser.h"
#include "cgenremap.h"

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/


/* Concrete reader */
class cRadioTimesReader : public iEpgReader
{
    public:
        cRadioTimesReader();
        ~cRadioTimesReader();

        bool start();
        void showHelpMessage();
        bool parseCmdLine(int argc, char *argv[]);
    private:
        OptionParser parser;
        cGenreMap genreMap;
        enum OptIds
        {
            SERVICE_NAME,
            SERVICE_REF,
            PROVIDER,
            FILENAME,
            HOURS,
            GENREFILE,
            TIME_OFFSET,
            IS_ASCII
        };
        string error;
        string serviceName;
        string serviceRef;
        string fileName;
        string provider;
        string genreMapFile;
        int timeOffset;
        int hours;
        bool nameFound;
        bool providerFound;
        bool serviceRefFound;
        bool fileFound;
        bool timeOffsetFound;
        bool hoursFound;
        bool genreMapFound;
        bool isAscii;

        //void printOptions();
 
};

#endif
