#include "cdbiface.h"
#include <iostream>

cDbIface* cDbIface::instance = 0;

cDbIface* cDbIface::getInstance()
{
    if (instance == 0)
    {
        instance = new cDbIface();
    }
    return instance;
}


cDbIface::cDbIface()
    : dbInst(0)
{}

bool cDbIface::openDb(const string& file)
{
    bool res = (sqlite3_open(file.c_str(), &dbInst) == SQLITE_OK);
    if (res)
    {
        sqlite3_busy_timeout(dbInst, BUSY_TIMEOUT);    //if db locked, retry multiple times until BUSY_TIMEOUT msecs elapsed
    }
    if (dbInst)
        runSqlCmd( "PRAGMA synchronous=OFF" );

    return res;
}

void cDbIface::closeDb()
{
    if (dbInst != 0)
    {
        sqlite3_close(dbInst);
        dbInst = 0;
    }
}

string cDbIface::getLastErrorMsg()
{
    return sqlite3_errmsg(dbInst);
}


cDbIface::~cDbIface()
{
    if (dbInst != 0)
    {
        (void) sqlite3_close(dbInst); //can't do much if this fails...
    }
}

const bool cDbIface::runSqlCmd( const string command, queriedValue* queryResult )
{
// queryResult is an option parameter, its type should match the first field in the query result.
// For now this can only be int.

	sqlite3_stmt *byteCode;
	const char *nextCmd;
	bool ret = false;
	int finalizeResult;

	if ( queryResult )
	{
		queryResult->integer = 0;
		queryResult->resultString = "";
	}	
	
	if ( sqlite3_prepare( dbInst, command.c_str(), command.length(), &byteCode, &nextCmd ) == SQLITE_OK )
	{
		int resultCode;
		do
		{
			resultCode = sqlite3_step( byteCode );
			if ( queryResult && resultCode == SQLITE_ROW && sqlite3_column_count( byteCode ) > 0 )
			{
				if ( sqlite3_column_type( byteCode, 0 ) == SQLITE_INTEGER )
					queryResult->integer = sqlite3_column_int( byteCode, 0 );
				else if ( sqlite3_column_type( byteCode, 0 ) == SQLITE_TEXT )
					queryResult->resultString = (const char*) sqlite3_column_text( byteCode, 0 );
			}
		} while ( resultCode == SQLITE_BUSY || resultCode == SQLITE_ROW );
		ret = ( resultCode == SQLITE_DONE );
	}
	else
	{
		std::cout << "Could not prepare " << command.c_str() <<std::endl;
		std::cout << "Error msg: " << sqlite3_errmsg(dbInst) << std::endl;
	}
	finalizeResult = sqlite3_finalize( byteCode );
	return ( ret && finalizeResult == SQLITE_OK );
}

bool cDbIface::getNextEventInfo(const string& serviceKey, int maxPriority, time_t &startTime, time_t &stopTime)
{
    string sql = "select max(startTime), max(stopTime) from epgcache where serviceKey = ? and source < ?";
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare(dbInst, sql.c_str(), -1, &stmt, 0);
    if (res == SQLITE_OK)
        res = sqlite3_bind_text(stmt, 1, serviceKey.c_str(), -1, SQLITE_TRANSIENT);
    if (res == SQLITE_OK)
        res = sqlite3_bind_int(stmt, 2, maxPriority);
    if (res == SQLITE_OK)
        res = sqlite3_step(stmt);
    if (res == SQLITE_ROW)
    {
        startTime = sqlite3_column_int(stmt, 0);
        stopTime  = sqlite3_column_int(stmt, 1);
    }
    sqlite3_finalize(stmt);
    return (res == SQLITE_ROW || res == SQLITE_DONE);
}


bool cDbIface::getServiceInfo(const string& name, const string& providerStart, string &key, string &provider)
{
    bool haveProvider = (providerStart.length() > 0);
    string p=providerStart+'%';
    //database must be open
    string sql;
    if (haveProvider)
    {
        sql = "select serviceKey, name, provider from services where name = ? and provider like ?";
    }
    else
        sql = "select serviceKey, name, provider from services where name = ?";

    sqlite3_stmt *stmt;
    int res = sqlite3_prepare(dbInst, sql.c_str(), -1, &stmt, 0);
    if (res == SQLITE_OK)
    {
        res = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        if (res == SQLITE_OK && haveProvider)
            res = sqlite3_bind_text(stmt, 1, p.c_str(), -1, SQLITE_TRANSIENT);
        if (res == SQLITE_OK)
            res = sqlite3_step(stmt);

        if (res == SQLITE_ROW)
        {
            const char *tmp = (const char*) sqlite3_column_text(stmt, 0);
            key = tmp;

            tmp = (char*) sqlite3_column_text(stmt, 2);
            provider = tmp;
        }
        if (res == SQLITE_DONE)
        {
            sqlite3_reset(stmt);
        }
    }
    if (stmt != NULL)
    {
        res = sqlite3_finalize(stmt);
    }
    return (res == SQLITE_OK);
}

bool cDbIface::startTransaction()
{
    return runSqlCmd( "BEGIN TRANSACTION" );
}

bool cDbIface::endTransaction()
{
    return runSqlCmd( "COMMIT TRANSACTION" );
}

bool cDbIface::insertEpgData(const string& key, int eventId, int startTime, int stopTime, int source, const U8* eitData, int len)
{
    /* Check if data already exists for this timeslot? - no simply replace existing data*/
    /* replacing is not a major problem, if used correctly, the original dreambox data will not be touched, only
       the items we put in there ourselves  */
    //Should I escape the strings, or does sqlite3_bind do that for me?
    string sql = "insert or replace into epgcache (serviceKey, eventId, startTime, stopTime, source, eitData)  values (?, ?, ?, ?, ?, ?)";
    sqlite3_stmt *stmt;
    int res = sqlite3_prepare(dbInst, sql.c_str(), -1, &stmt, 0);
    if (res == SQLITE_OK)
    {
        res = sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
        if (res == SQLITE_OK)
            res = sqlite3_bind_int(stmt, 2, eventId);
        if (res == SQLITE_OK)
            res = sqlite3_bind_int(stmt, 3, startTime);
        if (res == SQLITE_OK)
            res = sqlite3_bind_int(stmt, 4, stopTime);
        if (res == SQLITE_OK)
            res = sqlite3_bind_int(stmt, 5, source);
        if (res == SQLITE_OK)
            res = sqlite3_bind_blob(stmt, 6, eitData, len, SQLITE_TRANSIENT);
        if (res == SQLITE_OK)
            res = sqlite3_step(stmt);

        sqlite3_finalize(stmt);
    }
    if (res != SQLITE_DONE)
    {
        std::cout << "SQL error: " << sqlite3_errmsg(dbInst) << std::endl;
    }
    return (res == SQLITE_DONE);
}
