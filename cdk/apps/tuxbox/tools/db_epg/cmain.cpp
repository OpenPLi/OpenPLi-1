#include "cmain.h"
#include "iepgdata.h"
#include "iepgreader.h"
#include "cdbiface.h"
#include "readerfactory.h"

#include <iostream>
#include <cstdio>

typedef unsigned char U8;
using namespace std;
#define VERSION     "0.5"
#define HELP_MESSAGE_HEAD \
        "EPG Converter "  VERSION \
        "\nGeneral options:\n"
#define HELP_MESSAGE_TAIL \
        "Some readers may require additional arguments. \n" \
        "Use -? with the selected type to see a list of options\n"

/* Descriptions for each option */
#define HELP_DESC   "=help(this page)"
#define DBFILE_DESC "<file>=location of database"
#define TYPE_DESC   "<type>=type of reader to use (available types are listed below)"

cMain::cMain()
: db(0)
, reader(0)
, dbFound(false)
, typeFound(false)
, helpRequested(false)
{
    parser.registerOption(FILETYPE, 't', "type",     TYPE_DESC);
    parser.registerOption(DATABASE, 'd', "database", DBFILE_DESC);
    parser.registerOption(HELP,     '?', "help",     HELP_DESC);
}


int cMain::start(int argc, char *argv[])
{
    int result = -1;
    if (parser.parseOptions( argc-1, &argv[1]))
    {
        dbFound       = parser.getOptionValue(DATABASE, db_filename);
        typeFound     = parser.getOptionValue(FILETYPE, type);
        helpRequested = parser.getOptionFound(HELP);

        if (typeFound)
        {
            reader = cReaderFactory::createReader(type);
            if (!reader)
            {
                cout << "Unknown type of reader '" << type << "'" << endl;
                showHelpMessage();
                return -1;
            }
        }

        if (helpRequested)
        {
            if (reader)
                reader->showHelpMessage();
            else
                showHelpMessage();
            return 0;
        }

        if (!typeFound || !dbFound)
        {
            cout << "Error: required argument missing." << std::endl;
            cout << "Please specify ";
            if (!typeFound) cout <<  "filetype";
            if (!dbFound && !typeFound) cout << " and database location";
            else if (!dbFound && typeFound) cout << " database location";
            cout << "." << endl;
            showHelpMessage();
            return -1;
        }

        if (helpRequested)
        {
            showHelpMessage();
            if (reader) reader->showHelpMessage();
            return 0;
        }


        db = cDbIface::getInstance();
        if (!db->openDb(db_filename))
        {
            cout << "Unable to open database" << endl;
            cout << "Message from sqlite: " << db->getLastErrorMsg() << endl;
            return -1;
        }

        /* Here we have the file to import, the database to write to, and the type of file */
        /* Give control to the correct reader*/
        if (reader->parseCmdLine(argc, argv))
        {
            result = (reader->start() ? 0 : -1);
        }
        else
        {
            cout << "Unsupported argument for this type of reader" << endl;
            //showHelpMessage();
            reader->showHelpMessage();
            result = -1;
        }

        db->closeDb();
    }
    else
    {
        showHelpMessage();
        result = -1;
    }
    return result;
}
cMain::~cMain()
{
    if (reader) delete reader;
}

struct printReaderInfo
{
    void operator()(const std::pair<string, cReaderFactory::tReaderType*>& readerInfo)
    {
        /* must set width here so it all lines up nicely */
        cout.width(15);
        cout.setf(iostream::left);
        cout << readerInfo.first << readerInfo.second->name << endl;
    }
};

void cMain::showHelpMessage()
{
    //Use OptionParser to get possible values?
    cout << HELP_MESSAGE_HEAD << endl;
    parser.printOptionDescriptions(true);
    cout << HELP_MESSAGE_TAIL << endl;
    cout << "Supported readers:" << endl;
    const std::map<string, cReaderFactory::tReaderType*>& readers = cReaderFactory::getReaders();
    for_each(readers.begin(), readers.end(), printReaderInfo());
}
