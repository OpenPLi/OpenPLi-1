#ifndef CDBIFACE_H
#define CDBIFACE_H

#include <sqlite3.h>
#include <string>

using std::string;

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/

typedef unsigned char U8;

class cDbIface
{
public:
    static cDbIface *getInstance();

    class queriedValue
    {
    public:
       int integer;
       string resultString;
       queriedValue() { };
       queriedValue( int i ) : integer( i ) { };
       queriedValue( string s ) : resultString( s ) { };
    };

    bool startTransaction();
    bool endTransaction();
    bool insertEpgData(const string& key, int eventId, int startTime, int stopTime, int source, const U8* eitData, int len);
    bool getServiceInfo(const string& name, const string& providerStart, string &serviceKey, string &fullProvider);

    /* Retrieve the start and stop time of last event with higher priority than maxPriority (numerical value < maxPriority) */
    bool getNextEventInfo(const string& serviceKey, int maxPriority, time_t &startTime, time_t &stopTime);

    string getLastErrorMsg();
    bool openDb(const string& fileName);
    void closeDb();
    const bool runSqlCmd( const string, queriedValue* queryResult = 0 );

private:
    enum { BUSY_TIMEOUT = 1000 };
    cDbIface();
    ~cDbIface();
    static cDbIface* instance;

    sqlite3 *dbInst;
};

#endif
