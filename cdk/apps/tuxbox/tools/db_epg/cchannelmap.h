#ifndef CCHANNELMAP_H
#define CCHANNELMAP_H

/**
	@author Ruud Nabben <r.nabben@gmail.com>
*/
#include "util.h"
#include <string>
#include <list>
#include <map>

class cChannelMap
{
public:
    cChannelMap();

    ~cChannelMap();
    void readFile(const std::string& mapfile);

    /* Function to access/modify unknownChannels and mapChanNameToRef */
    void writeUnknownChannelMap(const std::string& mapfile) const;
    bool findChannelReference(const std::string& channelId, std::string& reference) const;
    void addUnknownChannel(const std::string& channelId);
    void addTemporaryChannel(const std::string& channelName, const std::string& key);

private:
    struct getChanMap;
    std::list<std::string> unknownChannels;
    std::map<std::string, std::string> mapChanIdToRef;        //use 'displayNames' or channelIds? ==> channelIds

    /* Actually, I may need a list of id's, names and references, if I want to support mapping on id and/or name */
};

#endif
