#ifndef CMAIN_H
#define CMAIN_H

#include <string>
#include "optionparser.h"


using namespace std;

class iEpgReader;
class cDbIface;

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/
class cMain
{
public:
    cMain();
    ~cMain();

    int start(int argc, char *argv[]);

private:
    void showHelpMessage();
    enum OptIds
    {
        FILENAME = -1,
        FILETYPE = -2,
        DATABASE = -3,
        HELP     = -4
    };

    OptionParser parser;
    cDbIface *db;

    /* Negative option ids are only allowed for main application. Every reader must use positive values*/

    string error;
    string type;
    string db_filename;
    iEpgReader *reader;
    bool dbFound;
    bool typeFound;
    bool helpRequested;
};

#endif
