#ifndef CGENREMAP_H
#define CGENREMAP_H
/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/

#include <map>
#include <string>

/* Should move these to a 'types.h' sort of header */
typedef unsigned char U8;

class cGenreMap
{
public:
    cGenreMap();
    ~cGenreMap();

    void readFile(const std::string& mapfile);
    /* Duration is in seconds!*/
    U8 getContentType(const std::string& genre, int duration = -1) const;

private:
    struct tGenreDetail
    {
        tGenreDetail() : type(0), minDuration(-1) {}
        U8 type;
        int minDuration;
    };
    struct getGenreMap;
    std::map<std::string, tGenreDetail> mapStringToGenre;
};

#endif
