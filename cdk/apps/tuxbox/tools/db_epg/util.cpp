#include "util.h"
#include <time.h>
#include <iostream>

//read lines, for each line, invokes function object

/* Convert a date and time pair into a unix timestamp */
/* date is given in dd/mm/yyy, time in hh:mm */
/* Time is interpreted as a local time, DST*/
/* */
time_t parseDateTime(const std::string& date, const std::string& time)  //give offset of source material (not taking into account dst - for radiotimes, offset is 0)
{
    //get local time zone - relative to UTC
    //determine 
    //date: dd/mm/yyyy
    //std::cout << "pdt: Timezone info: " << tzname << "; offset: " << timezone << "." << std::endl;
    tm t;
    t.tm_mday = atoi(date.substr(0, 2).c_str());
    t.tm_mon = atoi(date.substr(3, 2).c_str()) - 1;
    t.tm_year = atoi(date.substr(6, 4).c_str()) - 1900;
    t.tm_hour = atoi(time.substr(0, 2).c_str());
    t.tm_min = atoi(time.substr(3, 2).c_str());
    t.tm_sec = 0;
    t.tm_isdst = -1;    //try to determine if DST is in effect
    time_t local = mktime(&t);
    long int localOffset = timezone;
    //We change DST at the same time, so don't do anything with it...
    //if (t.tm_isdst == 1) localOffset -= 3600;         //we are in +1 during winter, +2 in summer..
    if (local > -1)
    {
        local -= localOffset;  //convert to real GMT
    }
    //std::cout << "Timezone info after mktime: localOffset" << localOffset << "; offset: " << timezone << "." << std::endl;
    return local;
}

bool analyseTime(const std::string& t, time_t& result, int offset)
{
    //std::cout << "at: Timezone info: " << tzname << "; offset: " << timezone << "." << std::endl;
    tm tStruct;
    std::string timePart = t.substr(0, 12 );

    if ( timePart.length() != 12 ) // || ( ! isNumericString( (char *)timePart.c_str() ) ))
        return false;
	
    tStruct.tm_year = atoi( timePart.substr(0, 4 ).c_str() ) - 1900;
    tStruct.tm_mon = atoi( timePart.substr( 4,2 ).c_str() ) - 1;
    tStruct.tm_mday = atoi( timePart.substr( 6,2 ).c_str() );

    tStruct.tm_hour = atoi( timePart.substr(8, 2 ).c_str() );
    tStruct.tm_min = atoi( timePart.substr(10, 2 ).c_str() );
    tStruct.tm_sec = 0;
    //tStruct.tm_isdst = -1;
    //Don't do anything with daylight savings time - just pray that we all switch at the same time..

    result = mktime( &tStruct ) + offset;
    //std::cout << "Timezone info after mktime: " << tzname << "; offset: " << timezone << "." << std::endl;
    //timezone info has now been set... use it to compute gmt from offset if given
    long int localOffset = timezone;
    if (tStruct.tm_isdst == 1) localOffset -= 3600;         //we are in +1 during winter, +2 in summer..
    
	// Analyse offset if it looks like there is one
	// Test 14 not 12 because some formats include 
	// 2 digits for seconds

    /* If there is an offset, the time is given as an offset wrt to GMT. Our dreambox calculates gmt time wrt
    to our local timezone. We must use the difference between the shift time and our local timezone... */
    if ( t.length() > 14 ) {
		// Apply shift field

        std::string shiftPart = t.substr(t.length()-5, 5);
        std::string shiftTime = shiftPart.substr(shiftPart.length()-4, 4);

        if ( shiftTime.length() != 4 )// || ( ! isNumericString( (char *)shiftTime.c_str() ) )) {
        {
            return false;
        }

        time_t shiftHour = atoi( shiftTime.substr(0, 2).c_str() );
        time_t shiftMin = atoi( shiftTime.substr(shiftTime.length()-2, 2).c_str() );

        time_t shiftTimeT = ( shiftHour * 3600 ) + ( shiftMin * 60 );

        //Imagine: we're in +2, creator is in +1... so 6.45 for us is 5.45 for creator.
        //We must ADD the difference (us-creator)
        //So, first add our own timezone offset, then substract the creators offset...
        result -= localOffset;
        if ( shiftPart.substr(0, 1) == "+" )
            result -= shiftTimeT;
        else if ( shiftPart.substr(0, 1) == "-" )
            result += shiftTimeT;
        else
            return false;
    }
    return true;
}


int toBCD(int dec)
{
    if (dec < 0 || dec >99)
        return -1;
    return ( ((dec / 10) << 4) | (dec%10) );
}



void toDVBTime(U8 *d, const time_t t)
{
    tm *time = gmtime(&t);

    int l = 0;
    int month = time->tm_mon + 1;
    if (month == 1 || month == 2)
        l = 1;
    int mjd = 14956 + time->tm_mday + (int)((time->tm_year - l) * 365.25) + (int)((month + 1 + l*12) * 30.6001);
    d[0] = mjd >> 8;
    d[1] = mjd & 0xFF;

    d[2] = toBCD(time->tm_hour);
    d[3] = toBCD(time->tm_min);
    d[4] = toBCD(time->tm_sec);

}

void toBCDTime(U8 *d, int time)
{
    //time is given in second
    //convert to hour, minutes, seconds
    d[0] = toBCD(time / 3600);
    d[1] = toBCD((time % 3600) / 60);
    d[2] = toBCD((time % 3600) % 60);
}

int getCodePageOverhead(int codePage)
{
    switch (codePage)
    {
    case 0: //latin1
        return 0;
    case 5 ... 16:     //codepages 8859-5 to 8859-16 are represented with 1..12
        return 1;
    case 1 ... 4:  //
        return 3;
    }
    return 0;
}

void insertCodePageMarker(U8* data, int codePage)
{
    switch (codePage)
    {
    case 0:
        return;
    case 5 ... 16:
        data[0] = codePage-4;
        return;
    case 1 ... 4:
        data[0] = 0x10;
        data[1] = 0;    //upper 8 bits, but they're always 0...
        data[2] = codePage; //dreambox only supports iso8859-1 to 16...
        return;
    }
    return;
}

bool stringToInt(const std::string& str, int& i)
{
    const char *tmp1 = str.c_str();
    char *tmp2 = 0;
    int tmpInt = strtol(tmp1, &tmp2, 0);
    bool result = (tmpInt != 0 || (tmp2 && (tmp1 != tmp2)));    //conversion succeeded
    if (result)
        i = tmpInt;
    return result;
}

std::string upperCase(std::string s)
{
    for (std::string::size_type j=0; j<s.length(); ++j)
    {
        s[j]=toupper(s[j]);
    } // s now contains "HELLO"
    return s;
}
