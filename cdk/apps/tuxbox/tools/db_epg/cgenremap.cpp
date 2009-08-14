#include "cgenremap.h"
#include "util.h"
#include <algorithm>

cGenreMap::cGenreMap() 
{
}


cGenreMap::~cGenreMap()
{
}

U8 cGenreMap::getContentType(const std::string& genre, int duration) const
{
    std::string tmp = upperCase(genre);
    std::map<std::string, tGenreDetail>::const_iterator it = mapStringToGenre.find(tmp);
    if (it != mapStringToGenre.end())
    {
        if (duration < it->second.minDuration || it->second.minDuration == 0)
        {
            return it->second.type;
        }
    }
    return 0;
}



struct cGenreMap::getGenreMap : public std::unary_function<const std::string&, void>
{
    getGenreMap(std::map<std::string, cGenreMap::tGenreDetail>& m) : map(m){}
    void operator()(const std::string& line)
    {
        std::string::size_type pos = line.find(',');
        std::string::size_type pos2 = line.find(',', pos+1);
        if (pos != std::string::npos)
        {
            std::string genre = upperCase(line.substr(0, pos));
            std::string val;
            std::string val2;
            if (pos2 != std::string::npos)
            {
                val = line.substr(pos+1, pos2-pos-1);
                val2 = line.substr(pos2+1);
            }
            else
            {
                val = line.substr(pos+1);
            }
            int type;
            int minDuration = 0;
            if (stringToInt(val, type))
            {
                U8 t = (U8) type;
                stringToInt(val2, minDuration);

                if (map.find(genre) == map.end())
                {
                    map[genre].type = t;
                    map[genre].minDuration = minDuration*60;
                }
            }
        }
    }
    private:
        std::map<std::string, cGenreMap::tGenreDetail>& map;
};

void cGenreMap::readFile(const std::string& mapfile)
{
    readGenericMapFile(mapfile, getGenreMap(mapStringToGenre));
}


