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
*/
/*************************************************************/

#include "filemap.h"
#include <fstream>
FileMap::~FileMap()
{
}

FileMap::FileMap(const eString& path, const eString& file, const eString& type )
: mapPath( path )
, mapFile(file)
, isDirty( false )
, initDone(false)
{
    if (type=="rt_uk")
        readRadioTimesChannels();

	readMapFile(type != "rt_uk");
    initDone = true;
}


bool FileMap::readRadioTimesChannels( void )
{
    std::ifstream f;
    eString fileName = mapPath + "/channels.dat";
    f.open(fileName.c_str());

    if ( ! f.is_open())
    {
        std::cout << "FileMap: failed to read " << fileName << std::endl;
        return false;
    }
    eString tmp;
    while (!f.eof() && getline(f, tmp))
    {
        if (tmp.length() > 0)
        {
            int pos = tmp.find('|');
            eString chanId;
            eString name;
            if (pos > 0)
            {
                chanId = tmp.substr(0, pos).c_str();
                name = tmp.substr(pos+1);
                updateNameIdPair(chanId, name);
            }
        }
    }
    f.close();

    return true;
}

bool FileMap::readMapFile( bool idEqualsName  )
{
    std::ifstream f;
    eString fileName = mapPath + "/" + mapFile;
    f.open(fileName.c_str());

    if ( ! f.is_open())
    {
        std::cout << "FileMap: failed to read " << fileName << std::endl;
        return false;
    }
    eString tmp;
    while (!f.eof() && getline(f, tmp))
    {
        if (tmp.length() > 0)
        {
            if (tmp[0] != '#')
            {
                int pos = tmp.find('=');
                eString chanId;
                eString ref;
                if (pos > 0)
                {
                    ref = tmp.substr(0, pos).c_str();
                    chanId = tmp.substr(pos+1);
                    if (ref == "UNKNOWN")
                        addUnknownChannel(chanId);
                    else if (ref == "IGNORE")
                        addIgnoredChannel(chanId);
                    else
                        addMappedChannel(chanId, ref);

                    if (idEqualsName)
                    {
                        //Channel names are the channel ids -> fill the id and name maps too
                        updateNameIdPair(chanId, chanId);
                    }
                }
            }
        }
    }
    f.close();
	return true;
}

void FileMap::updateNameIdPair(const eString& chanId, const eString& name)
{
    if (chanId.length() > 0 && name.length() > 0)
    {
        channelId2Names[chanId] = name;
        channelName2Id[name] = chanId;
    }

}

bool FileMap::writeMapFile( void )
{
	if ( !isDirty)
		return true;

    std::ofstream f;
    eString fileName = mapPath + "/" + mapFile;
    f.open(fileName.c_str());

    if (!f.is_open())
    {
        std::cout << "FileMap: failed to write " << fileName << std::endl;
        return false;
    }

    for (std::map<eString, eString>::const_iterator it = map.begin(); it != map.end(); it++)
    {
        //Write mapped channels in format: ref=chanId
        f << it->second << "=" << it->first<< std::endl;
	}
    for (std::set<eString>::const_iterator it = ignored.begin(); it != ignored.end(); it++)
    {
        f << "IGNORE" << "=" << *it << std::endl;
    }
    for (std::set<eString>::const_iterator it = unknown.begin(); it != unknown.end(); it++)
    {
        f << "UNKNOWN" << "=" << *it << std::endl;
    }
	f.close();

    isDirty = false;
	return true;
}

const eString& FileMap::getMappedChannel(const eString& chanId)
{
    return map[chanId];   //returns empty string if chanId does not exist
}

void FileMap::removeMappedChannel(const eString& chanId)
{
    int count = mapped.erase(chanId);
    map.erase(chanId);
    eString s = isMapped(chanId) ? "failed" : "succeeded";
    std::cout << "Removing map " << chanId << ": " << s << std::endl;
    std::cout << "Removing map " << chanId << ": removed " << count << " items." << std::endl;
}

void FileMap::addMappedChannel(const eString& chanId, const eString& ref)
{
    isDirty = initDone;
    std::cout << "Adding map " << chanId << ", " << ref << std::endl;
    if (isMapped(chanId))   removeMappedChannel(chanId);
    if (isIgnored(chanId))  removeIgnoredChannel(chanId);
    if (isUnknown(chanId))  removeUnknownChannel(chanId);
    mapped.insert(chanId);
    map[chanId] = ref;
}

void FileMap::removeIgnoredChannel(const eString& chanId)
{
    ignored.erase(chanId);
    eString s = isIgnored(chanId) ? "failed" : "succeeded";
    std::cout << "Removing ignore " << chanId << ": " << s << std::endl;
}

void FileMap::addIgnoredChannel(const eString& chanId)
{
    isDirty = initDone;
    if (!isIgnored(chanId)) ignored.insert(chanId);
    if (isMapped(chanId))   removeMappedChannel(chanId);
    if (isUnknown(chanId))  removeUnknownChannel(chanId);
    eString s = (isIgnored(chanId) && !isUnknown(chanId) && !isMapped(chanId)) ? "succeeded" : "failed";
    std::cout << "Adding ignore " << chanId << ": " << s << std::endl;
}
void FileMap::removeUnknownChannel(const eString& chanId)
{
    unknown.erase(chanId);
    eString s = isUnknown(chanId) ? "failed" : "succeeded";
    std::cout << "Removing unknown " << chanId << ": " << s << std::endl;
}

void FileMap::addUnknownChannel(const eString& chanId)
{
    isDirty = initDone;
    if (!isUnknown(chanId)) unknown.insert(chanId);
    if (isIgnored(chanId))  removeIgnoredChannel(chanId);
    if (isMapped(chanId))   removeMappedChannel(chanId);
    eString s = (isUnknown(chanId) && !isIgnored(chanId) && !isMapped(chanId)) ? "succeeded" : "failed";
    std::cout << "Adding unknown " << chanId << ": " << s << std::endl;
}

