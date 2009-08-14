#include "cchannelmap.h"
#include <fstream>
#include "util.h"

cChannelMap::cChannelMap()
{
}


cChannelMap::~cChannelMap()
{
}

bool cChannelMap::findChannelReference(const std::string& channelId, std::string& reference) const
{
    std::map<std::string, std::string>::const_iterator it=mapChanIdToRef.find(channelId);
    if (it != mapChanIdToRef.end())
    {
        reference = it->second;
        return true;
    }
    return false;
}

void cChannelMap::addUnknownChannel(const std::string& channelId)
{
    std::map<std::string, std::string>::const_iterator it=mapChanIdToRef.find(channelId);
    if (it == mapChanIdToRef.end())
    {
        mapChanIdToRef[channelId] = "UNKNOWN";
        unknownChannels.push_back(channelId);
    }
}
void cChannelMap::addTemporaryChannel(const std::string& channelId, const std::string& key)
{
    mapChanIdToRef[channelId] = key;
}

struct cChannelMap::getChanMap : public std::unary_function<std::string, void>
{
    getChanMap(std::map<std::string, std::string>& m) : map(m){}
    void operator()(std::string line)
    {
        std::string::size_type pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string dbServiceRef = line.substr(0, pos);
            std::string epgName = line.substr(pos+1);
            if ((map.find(epgName) == map.end()))
            {
                map[epgName] = dbServiceRef;
            }
        }
    }
    private:
        std::map<std::string, std::string>& map;
};


void cChannelMap::readFile(const std::string& mapfile)
{
    readGenericMapFile(mapfile, getChanMap(mapChanIdToRef));
}


void cChannelMap::writeUnknownChannelMap(const std::string& mapfile) const
{
    std::ofstream f(mapfile.c_str(), std::ios_base::app);
    if (f.is_open())
    {
        for (std::list<std::string>::const_iterator it=unknownChannels.begin();
             it != unknownChannels.end(); ++it)
        {
            f << "UNKNOWN=" << *it << std::endl;
        }
        f.close();
    }
}



