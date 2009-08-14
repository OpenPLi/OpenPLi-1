#ifndef IEPGDATA_H
#define IEPGDATA_H

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/
#include "util.h"
#include <string>
#include <algorithm>
using std::string;
using std::min;

typedef unsigned char U8;


/* Base class for descriptors */
class BaseDesc
{
    public:
        BaseDesc() :_data(0), totalLen(0) {}
        virtual ~BaseDesc(){ if (_data) delete _data; }
        int copyData(U8 *data, int maxLen) const
        {
            assert(totalLen <= maxLen);
            memcpy(data, _data, totalLen);
            return totalLen;
        }

        int getLen() const { return totalLen; }
    protected:
        U8 *createData(int len) { if (_data) delete [] _data; _data = new U8[totalLen]; return _data; }
        U8 *_data;
        int totalLen;
};

/* Short description, name and possibly a short description
Max length 255 */
class ShortDesc : public BaseDesc
{
    public:
        ShortDesc(const string& name)
        {
            fillDataFields(name, "", "eng");
        }
        ShortDesc(const string& name, const string& desc, const string& langCode, int codePage = 0)
        {
            fillDataFields(name, desc, langCode, codePage);
        }

    private:
        void fillDataFields(const string& name, const string& desc, const string& langCode, int codePage = 0)
        {
        //name and desc together no more than 248 chars to leave room for header bytes
            int cpLen = getCodePageOverhead(codePage);
            //string dvbName = convertControlCodes(name);
            int nameLen = std::min((int)name.length() + cpLen, 248);
            int descLen = std::min((int)desc.length() + cpLen, 248-nameLen); //total descriptor length cannot exceed 255 bytes
            if ((nameLen >= 0) &&
                (descLen >= 0) &&
                 (nameLen + descLen <=248) &&
                (langCode.length() == 3))
            {
                totalLen = 7 + nameLen + descLen;
                assert (totalLen < 256);

                _data = createData(totalLen);
                _data[0] = 0x4d;
                _data[1] = totalLen - 2;
                _data[2] = langCode[0];
                _data[3] = langCode[1];
                _data[4] = langCode[2];

                _data[5] = nameLen;
                insertCodePageMarker(&_data[6], codePage);
                for (int i = 0; i<nameLen; i++)
                {
                    _data[6+i+cpLen] = name[i];
                }
                _data[6+nameLen] = descLen;

                insertCodePageMarker(&_data[7+nameLen], codePage);
                for (int i = 0; i<descLen-cpLen; i++)
                {
                    _data[7+nameLen+cpLen+i] = desc[i];
                }
            }
        }
};

/* Long description needs some special treatment */
/* Splitting up messages should be done here */
/* String must contain fully formatted text */
/* Add some sort of template/factory method which creates a list of descriptors? */

/*Long description. Max length is 4kB, split over a number of descriptors */
class LongDesc : public BaseDesc
{
    public:
        enum  { MAX_LEN = 255-9 }; //payload part must be less than 256 bytes, 
        LongDesc(int index) : _index(index) {};

        void setIndexNumbers(U8 last)
        {
            _data[2] = (_index << 4) | (last | 0x0f);
        }
        int addDescription(const string& desc, int codePage = 0)
        {
            int cpLen = getCodePageOverhead(codePage);
            int descLen = std::min((int)(desc.length() + cpLen), (int)MAX_LEN);   //max is 246 characters, 245 if we insert a codetable
            /* descLen is number of characters in this message */
            /* no formatting is done! */

            totalLen = 8 + descLen;
            assert(totalLen < 256);

            _data = new U8[totalLen];
            _data[0] = 0x4e;
            _data[1] = totalLen - 2;
            _data[2] = (_index << 4) | (_index | 0x0f);
            _data[3] = 'e';
            _data[4] = 'n';
            _data[5] = 'g';
            _data[6] = 0;   //no support for items yet (think of cast, year, director, etc...)
            _data[7] = descLen;
            insertCodePageMarker(&_data[8], codePage);
            for (int i = 0; i<descLen-cpLen; i++)
            {
                _data[8+cpLen+i] = desc[i];
            }
            return descLen-cpLen; /* number of characters written */
        }
    private:
        int _index;
};

/* */
struct ParentalDesc : public BaseDesc
{
    ParentalDesc(int age, const string& countrycode)
    {
        if (countrycode.length() == 3)
        {
            totalLen = 6;
            _data = new U8[totalLen];
            _data[0] = 0x55;
            _data[1] = totalLen - 2;
            _data[2] = countrycode[0]; //'G';
            _data[3] = countrycode[1]; //'B';
            _data[4] = countrycode[2]; //'R';
            if (age > 3)
                _data[5] = age - 3;
            else if (age == 3)
                _data[5] = 0x01;
            else
                _data[5] = 0x00;    //undefined
        }
    }
};

struct ContentDesc : public BaseDesc
{
    ContentDesc(U8 byte1, U8 byte2)
    {
        totalLen = 4;
        _data = new U8[totalLen];
        _data[0] = 0x54;
        _data[1] = totalLen - 2;
        _data[2] = byte1;
        _data[3] = byte2;
    }
};

#endif
