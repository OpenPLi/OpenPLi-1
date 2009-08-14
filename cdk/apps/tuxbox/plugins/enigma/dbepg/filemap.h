/***************************************************************/
/*
    * Copyright (C) 2004 Lee Wilmot <lee@dinglisch.net>

    This file is part of MV.

    MV is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    MV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You can find a copy of the GNU General Public License in the
    file gpl.txt distributed with this file.


    Modified by Ruud Nabben, for Dreambox EPG plugin. 
*/
/*************************************************************/

#ifndef __FILEMAP_H__
#define __FILEMAP_H__

#define FILEMAP_MAX_NAME_LENGTH 2048

#include "util.h"
#include <set>
class FileMap
{
	eString mapPath;
    eString mapFile;
	bool isDirty;
    bool initDone;
    std::map<eString, eString> map;             //list of chanId, servicerefs
    std::map<eString, eString> channelId2Names; //list of all channels names first: id, second: name
    std::map<eString, eString> channelName2Id;      //list of all radiotimes channels

    std::set<eString> mapped;
    std::set<eString> ignored;
    std::set<eString> unknown;

public:
	FileMap( const eString& path, const eString& file, const eString& type );
	~FileMap();

    template <class T> void forEachChan(T ob)
    {
        for (std::map<eString, eString>::const_iterator i=channelId2Names.begin(); i!=channelId2Names.end(); i++)
        {
            ob(i->first, i->second);
        }
    };

    template <class T> void forEachChanAbc(T ob)
    {
        for (std::map<eString, eString>::const_iterator i=channelName2Id.begin(); i!=channelName2Id.end(); i++)
        {
            ob(i->second, i->first);
        }
    };

    eString& getRadioTimesChanName(const eString& chanId) { return channelId2Names[chanId]; }

    /**/
    const eString& getMappedChannel(const eString& chanId);
    void addMappedChannel( const eString& chanId, const eString& ref);
    void addIgnoredChannel(const eString& chanId);
    void addUnknownChannel(const eString& chanId);

    bool writeMapFile( void );
private:
    void updateNameIdPair(const eString& chanId, const eString& name);
    bool readRadioTimesChannels();
    void readChannelIds();
    bool readMapFile(bool idEqualsName);

    void removeMappedChannel(const eString& chanId);
    void removeIgnoredChannel(const eString& chanId);
    void removeUnknownChannel(const eString& chanId);
public:
    bool isUnknown(const eString& chanId) const { return unknown.find(chanId) != unknown.end(); }
    bool isMapped(const eString& chanId) const  { return mapped.find(chanId) != mapped.end(); }
    bool isIgnored(const eString& chanId) const { return ignored.find(chanId) != ignored.end(); }
    int getMappedCount() { return mapped.size(); }
	eString getMappedName( const eString &original );
};

#endif
