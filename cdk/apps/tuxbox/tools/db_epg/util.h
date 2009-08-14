#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <time.h>
#include <fstream>

typedef unsigned char U8;
/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/
/* Some helper functions, accesible by all (located here untill I find a better place)... */

/* Read a file line by line, and execute function object ob on it */
template <class T> void readGenericMapFile(const std::string& mapfile, T ob)
{
    std::ifstream f(mapfile.c_str());
    if (f.is_open())
    {
        std::string line;
        while (!f.eof() && getline(f, line))
        {
            //Ignore commments
            if (line.length() > 0 && line[0] != '#')
            {
                ob(line);
            }
        }
        f.close();
    }
}


int toBCD(int dec);
void toDVBTime(U8 *d, const time_t t);
void toBCDTime(U8 *d, int time);
time_t parseDateTime(const std::string& date, const std::string& time);
bool analyseTime(const std::string& t, time_t& result, int offset);
bool stringToInt(const std::string& str, int& i);
std::string upperCase(std::string s);

void insertCodePageMarker(U8* data, int codePage);
int getCodePageOverhead(int codePage);
#endif
