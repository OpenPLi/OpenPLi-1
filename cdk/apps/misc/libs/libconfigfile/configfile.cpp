/*
 * $Id: configfile.cpp,v 1.11 2002/10/16 16:47:59 thegoodguy Exp $
 *
 * configuration object for the d-box 2 linux project
 *
 * Copyright (C) 2001, 2002 Andreas Oberritter <obi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <stdint.h>
#include "configfile.h"

CConfigFile::CConfigFile (const char p_delimiter)
{
	modifiedFlag = false;
	unknownKeyQueryedFlag = false;
	delimiter = p_delimiter;
}

void CConfigFile::clear ()
{
	configData.clear();
}

//
// public file operation methods
//
const bool CConfigFile::loadConfig (std::string filename)
{
	FILE * fd = fopen(filename.c_str(), "r");

	if (fd == NULL)
	{
		perror(filename.c_str());
		return false;
	}

	clear();
	modifiedFlag = false;

	char buf[1000];
	char keystr[1000];
	char valstr[1000];

	while (!feof(fd))
	{
		if (fgets(buf, sizeof(buf), fd) != NULL)
		{
			char * tmpptr;
			char * key = (char *) &keystr;
			char * val = (char *) &valstr;
			bool keyfound = false;

			for (tmpptr = buf; (*tmpptr != 10) && (*tmpptr != 13) && (*tmpptr != '#'); tmpptr++)
			{
				if ((*tmpptr == '=') && (keyfound == false))
				{
					keyfound = true;
				}
				else if (keyfound == false)
				{
					*key = *tmpptr;
					key++;
				}
				else
				{
					*val = *tmpptr;
					val++;
				}
			}

			*val = 0;
			*key = 0;
			configData[keystr] = valstr;
		}
	}

	fclose(fd);
	return true;
}

const bool CConfigFile::saveConfig (std::string filename)
{
	std::ofstream configFile (filename.c_str());

	if (configFile != NULL)
	{
		ConfigDataMap::iterator it;

		for (it = configData.begin(); it != configData.end(); it++)
		{
			configFile << it->first << "=" << it->second << std::endl;
		}

		configFile.close();
		return true;
	}
	else
	{
		std::cerr << "unable to open file " << filename << "for writing." << std::endl;
		return false;
	}
}



//
// private "store" methods
// 
void CConfigFile::storeBool (const std::string key, const bool val)
{
	if (val == true)
		configData[key] = std::string("true");
	else
		configData[key] = std::string("false");
}

void CConfigFile::storeInt32 (const std::string key, const int32_t val)
{
	char tmp[11];
	sprintf(tmp, "%d", val);
	configData[key] = std::string(tmp);
}

void CConfigFile::storeInt64 (const std::string key, const int64_t val)
{
	char tmp[21];
	sprintf(tmp, "%lld", val);
	configData[key] = std::string(tmp);
}

void CConfigFile::storeString (const std::string key, const std::string val)
{
	configData[key] = val;
}



//
// public "get" methods
//
bool CConfigFile::getBool (const std::string key, const bool defaultVal)
{
	if (configData.find(key) == configData.end())
	{
		unknownKeyQueryedFlag = true;
		storeBool(key, defaultVal);
	}

	return !((configData[key] == "false") || (configData[key] == "0"));
}

int32_t CConfigFile::getInt32 (const std::string key, const int32_t defaultVal)
{
	if (configData.find(key) == configData.end())
	{
		unknownKeyQueryedFlag = true;
		storeInt32(key, defaultVal);
	}

	return atoi(configData[key].c_str());
}

int64_t CConfigFile::getInt64 (const std::string key, const int64_t defaultVal)
{
	if (configData.find(key) == configData.end())
	{
		unknownKeyQueryedFlag = true;
		storeInt64(key, defaultVal);
	}

	return atoll(configData[key].c_str());
}

std::string CConfigFile::getString (const std::string key, const std::string defaultVal)
{
	if (configData.find(key) == configData.end())
	{
		unknownKeyQueryedFlag = true;
		storeString(key, defaultVal);
	}

	return configData[key];
}

std::vector <int32_t> CConfigFile::getInt32Vector (const std::string key)
{
	std::string val = configData[key];
	std::vector <int32_t> vec;
	uint16_t length = 0;
	uint16_t pos = 0;
	uint16_t i;

	for (i = 0; i < val.length(); i++)
	{
		if (val[i] == delimiter)
		{
			vec.push_back(atoi(val.substr(pos, length).c_str()));
			pos = i + 1;
			length = 0;
		}
		else
		{
			length++;
		}
	}

	if (length == 0)
		unknownKeyQueryedFlag = true;
	else
		vec.push_back(atoi(val.substr(pos, length).c_str()));

	return vec;
}

std::vector <std::string> CConfigFile::getStringVector (const std::string key)
{
	std::string val = configData[key];
	std::vector <std::string> vec;
	uint16_t length = 0;
	uint16_t pos = 0;
	uint16_t i;

	for (i = 0; i < val.length(); i++)
	{
		if (val[i] == delimiter)
		{
			vec.push_back(val.substr(pos, length));
			pos = i + 1;
			length = 0;
		}
		else
		{
			length++;
		}
	}

	if (length == 0)
		unknownKeyQueryedFlag = true;
	else
		vec.push_back(val.substr(pos, length));

	return vec;
}



//
// public "set" methods
//
void CConfigFile::setBool (const std::string key, const bool val)
{
	bool tmpUnknownKeyQueryedFlag = unknownKeyQueryedFlag;
	unknownKeyQueryedFlag = false;
	bool oldVal = getBool(key);

	if ((oldVal != val) || (unknownKeyQueryedFlag))
	{
		modifiedFlag = true;
		storeBool(key, val);
	}

	unknownKeyQueryedFlag = tmpUnknownKeyQueryedFlag;
}

void CConfigFile::setInt32 (const std::string key, int32_t val)
{
	bool tmpUnknownKeyQueryedFlag = unknownKeyQueryedFlag;
	unknownKeyQueryedFlag = false;
	int32_t oldVal = getInt32(key);

	if ((oldVal != val) || (unknownKeyQueryedFlag))
	{
		modifiedFlag = true;
		storeInt32(key, val);
	}

	unknownKeyQueryedFlag = tmpUnknownKeyQueryedFlag;
}

void CConfigFile::setInt64 (const std::string key, const int64_t val)
{
	bool tmpUnknownKeyQueryedFlag = unknownKeyQueryedFlag;
	unknownKeyQueryedFlag = false;
	int64_t oldVal = getInt64(key);

	if ((oldVal != val) || (unknownKeyQueryedFlag))
	{
		modifiedFlag = true;
		storeInt64(key, val);
	}

	unknownKeyQueryedFlag = tmpUnknownKeyQueryedFlag;
}

void CConfigFile::setString (const std::string key, const std::string val)
{
	bool tmpUnknownKeyQueryedFlag = unknownKeyQueryedFlag;
	unknownKeyQueryedFlag = false;
	std::string oldVal = getString(key);
	
	if ((oldVal != val) || (unknownKeyQueryedFlag))
	{
		modifiedFlag = true;
		storeString(key, val);
	}

	unknownKeyQueryedFlag = tmpUnknownKeyQueryedFlag;
}

void CConfigFile::setInt32Vector (const std::string key, std::vector <int32_t> vec)
{
	uint16_t i;
	char tmp[11];

	for (i = 0; i < vec.size(); i++)
	{
		if (i > 0)
		{
			configData[key] += delimiter;
		}

		sprintf(tmp, "%d", vec[i]);
		configData[key] += std::string(tmp);
	}
}

void CConfigFile::setStringVector (const std::string key, const std::vector <std::string> vec)
{
	uint16_t i;

	for (i = 0; i < vec.size(); i++)
	{
		if (i > 0)
		{
			configData[key] += delimiter;
		}

		configData[key] += vec[i];
	}
}

