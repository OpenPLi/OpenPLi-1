/*
 *  PLi extension to Enigma: media mapping
 *
 *  Copyright (C) 2007 dAF2000 <daf2000@daf2000.nl>
 *  Copyright (C) 2007 PLi team <http://www.pli-images.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <media_mapping.h>
#include <lib/system/info.h>

eMediaMapping *eMediaMapping::instance;

const eString mediaKeys[] =
{
	"/pli/mediaMapping/movies",
	"/pli/mediaMapping/timeshifts"
};

eMediaMapping::eMediaMapping()
{
	if (!instance)
	{
		instance = this;
	}
	
	init();
}

eMediaMapping::~eMediaMapping()
{
	instance = NULL;
}

void eMediaMapping::init()
{
	econfig = eConfig::getInstance();
}

bool eMediaMapping::getStorageLocation(
	eMediaMapping::mediaTypeEnum mediaType,
	eString& storageLocation)
{
	bool found = false;
	
	// Default fall back if storage location cannot be found
	switch(mediaType)
	{
		case mediaMovies:
			storageLocation = "/media/hdd/movie";
			break;
		
		case mediaTimeshifts:
			storageLocation = "/media/hdd/timeshift";
			break;
			
		default:
			break;
	}

	if(mediaType < maxMediaTypeEnum)
	{
		found = (econfig->getKey(mediaKeys[(int)mediaType].c_str(), storageLocation) == 0);
	}
	
	return found;
}

void eMediaMapping::setStorageLocation(
	eMediaMapping::mediaTypeEnum mediaType,
	const eString& storageLocation)
{
	if(mediaType < maxMediaTypeEnum)
	{
		econfig->setKey(mediaKeys[(int)mediaType].c_str(), storageLocation);
		econfig->flush();
	}
}

	
