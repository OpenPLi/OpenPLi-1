/*
 * $Id: bouquets.cpp,v 1.70.2.2 2002/10/30 14:09:08 thegoodguy Exp $
 *
 * BouquetManager for zapit - d-box2 linux project
 *
 * (C) 2002 by Simplex    <simplex@berlios.de>,
 *             rasc       <rasc@berlios.de>,
 *             thegoodguy <thegoodguy@berlios.de>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <map>
#include <set>

/* tuxbox headers */
#include <configfile.h>

#include <zapit/bouquets.h>
#include <zapit/sdt.h>
#include <zapit/settings.h>
#include <zapit/xmlinterface.h>

extern tallchans allchans;   //  defined in zapit.cpp
extern CConfigFile config;   //  defined in zapit.cpp

/**** class CBouquet ********************************************************/

//
// -- servicetype 0 queries TV and Radio Channels
//

CZapitChannel* CBouquet::getChannelByChannelID(const t_channel_id channel_id, const unsigned char serviceType)
{
	CZapitChannel* result = NULL;

	ChannelList* channels = &tvChannels;
	
	switch (serviceType)
	{
		case RESERVED: // ?
		case DIGITAL_TELEVISION_SERVICE:
		case NVOD_REFERENCE_SERVICE:
		case NVOD_TIME_SHIFTED_SERVICE:
			channels = &tvChannels;
			break;
				
		case DIGITAL_RADIO_SOUND_SERVICE:
			channels = &radioChannels;
			break;
	}

	unsigned int i;
	for (i=0; (i<channels->size()) && ((*channels)[i]->getChannelID() != channel_id); i++);

	if (i<channels->size())
		result = (*channels)[i];

	if ((serviceType == RESERVED) && (result == NULL))
	{
		result = getChannelByChannelID(channel_id, 2);
	}

	return result;
}

void CBouquet::addService(CZapitChannel* newChannel)
{
	switch (newChannel->getServiceType())
	{
		case DIGITAL_TELEVISION_SERVICE:
		case NVOD_REFERENCE_SERVICE:
		case NVOD_TIME_SHIFTED_SERVICE:
			tvChannels.push_back(newChannel);
			break;
			
		case DIGITAL_RADIO_SOUND_SERVICE:
			radioChannels.push_back(newChannel);
			break;
	}
}

void CBouquet::removeService(CZapitChannel* oldChannel)
{
	if (oldChannel != NULL)
	{
		ChannelList* channels = &tvChannels;
		switch (oldChannel->getServiceType())
		{
			case DIGITAL_TELEVISION_SERVICE:
			case NVOD_REFERENCE_SERVICE:
			case NVOD_TIME_SHIFTED_SERVICE:
				channels = &tvChannels;
				break;

			case DIGITAL_RADIO_SOUND_SERVICE:
				channels = &radioChannels;
				break;
		}
		(*channels).erase(remove(channels->begin(), channels->end(), oldChannel), channels->end());
	}
}

void CBouquet::moveService(const unsigned int oldPosition, const unsigned int newPosition, const unsigned char serviceType)
{
	ChannelList* channels = &tvChannels;
	switch (serviceType)
	{
		case DIGITAL_TELEVISION_SERVICE:
		case NVOD_REFERENCE_SERVICE:
		case NVOD_TIME_SHIFTED_SERVICE:
			channels = &tvChannels;
			break;
			
		case DIGITAL_RADIO_SOUND_SERVICE:
			channels = &radioChannels;
			break;
	}
	if ((oldPosition < channels->size()) && (newPosition < channels->size()))
	{
		ChannelList::iterator it = channels->begin();

		advance(it, oldPosition);
		CZapitChannel* tmp = *it;
		channels->erase(it);

		advance(it, newPosition - oldPosition);
		channels->insert(it, tmp);
	}
}

int CBouquet::recModeRadioSize (unsigned int tsid)
{
	int size = 0;
	for ( unsigned int i=0; i< tvChannels.size(); i++)
	{
		if ( tsid == tvChannels[i]->getTsidOnid())
			size++;
	}
	return size;
}

int CBouquet::recModeTVSize( unsigned int tsid)
{
	int size = 0;
	for ( unsigned int i=0; i< radioChannels.size(); i++)
	{
		if ( tsid == radioChannels[i]->getTsidOnid())
			size++;
	}
	return size;
}


/**** class CBouquetManager *************************************************/
void CBouquetManager::saveBouquets()
{
	printf("[zapit] creating new bouquets.xml\n");
	FILE* bouq_fd = fopen(BOUQUETS_XML, "w");

	if (bouq_fd == NULL)
	{
		perror("fopen " BOUQUETS_XML);
		return;
	}

	fprintf(bouq_fd, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<zapit>\n");

	for (unsigned int i = 0; i < Bouquets.size(); i++)
	{
		if (Bouquets[i] != remainChannels)
		{
			fprintf(bouq_fd, "\t<Bouquet name=\"%s\" hidden=\"%d\" locked=\"%d\">\n",
				convert_UTF8_To_UTF8_XML(Bouquets[i]->Name).c_str(),
				Bouquets[i]->bHidden ? 1 : 0,
				Bouquets[i]->bLocked ? 1 : 0);
			for ( unsigned int j=0; j<Bouquets[i]->tvChannels.size(); j++)
			{
				fprintf(bouq_fd, "\t\t<channel serviceID=\"%04x\" name=\"%s\" onid=\"%04x\"/>\n",
						Bouquets[i]->tvChannels[j]->getServiceId(),
						convert_UTF8_To_UTF8_XML(Bouquets[i]->tvChannels[j]->getName()).c_str(),
						Bouquets[i]->tvChannels[j]->getOriginalNetworkId());
			}
			for ( unsigned int j=0; j<Bouquets[i]->radioChannels.size(); j++)
			{
				fprintf(bouq_fd, "\t\t<channel serviceID=\"%04x\" name=\"%s\" onid=\"%04x\"/>\n",
						Bouquets[i]->radioChannels[j]->getServiceId(),
						convert_UTF8_To_UTF8_XML(Bouquets[i]->radioChannels[j]->getName()).c_str(),
						Bouquets[i]->radioChannels[j]->getOriginalNetworkId());
			}
			fprintf(bouq_fd, "\t</Bouquet>\n");
		}
	}
	fprintf(bouq_fd, "</zapit>\n");
	fclose(bouq_fd);
}

void CBouquetManager::parseBouquetsXml(const XMLTreeNode *root)
{
	XMLTreeNode *search=root->GetChild();
	XMLTreeNode *channel_node;

	if (search)
	{
		while (strcmp(search->GetType(), "Bouquet"))
		{
			search = search->GetNext();
		}

		unsigned int original_network_id, service_id;

		printf("[zapit] reading Bouquets ");
		while ((search) && (!(strcmp(search->GetType(), "Bouquet"))))
		{
			CBouquet* newBouquet = addBouquet(search->GetAttributeValue("name"));
			char* hidden = search->GetAttributeValue("hidden");
			char* locked = search->GetAttributeValue("locked");
			newBouquet->bHidden = hidden ? (strcmp(hidden, "1") == 0) : false;
			newBouquet->bLocked = locked ? (strcmp(locked, "1") == 0) : false;
			channel_node = search->GetChild();

			while (channel_node)
			{
				sscanf(channel_node->GetAttributeValue("serviceID"), "%x", &service_id);
				sscanf(channel_node->GetAttributeValue("onid"), "%x", &original_network_id);

				CZapitChannel* chan = findChannelByChannelID(CREATE_CHANNEL_ID);

				if (chan != NULL)
					newBouquet->addService(chan);

				channel_node = channel_node->GetNext();
			}
			printf(".");
/*		printf(
			"[zapit] Bouquet %s with %d tv- and %d radio-channels.\n",
			newBouquet->Name.c_str(),
			newBouquet->tvChannels.size(),
			newBouquet->radioChannels.size());
*/
			search = search->GetNext();
		}
	}
	printf("\n[zapit] Found %d bouquets.\n", Bouquets.size());
}

void CBouquetManager::loadBouquets(bool ignoreBouquetFile)
{
	XMLTreeParser* parser;

	if (ignoreBouquetFile == false)
	{
		parser = parseXmlFile(string(BOUQUETS_XML));

		if (parser != NULL)
		{
			parseBouquetsXml(parser->RootNode());
			delete parser;
		}
	}
	renumServices();
}

void CBouquetManager::storeBouquets()
{
	for (unsigned int i=0; i<storedBouquets.size(); i++)
	{
		delete storedBouquets[i];
	}
	storedBouquets.clear();

	for (unsigned int i=0; i<Bouquets.size(); i++)
	{
		storedBouquets.push_back(new CBouquet( *Bouquets[i]));
	}
}

void CBouquetManager::restoreBouquets()
{
	for (unsigned int i=0; i<Bouquets.size(); i++)
	{
		delete Bouquets[i];
	}
	Bouquets.clear();

	for (unsigned int i=0; i<storedBouquets.size(); i++)
	{
		Bouquets.push_back(new CBouquet( *storedBouquets[i]));
	}
}

void CBouquetManager::makeRemainingChannelsBouquet()
{
	ChannelList unusedChannels;
	set<t_channel_id> chans_processed;

	for (vector<CBouquet*>::iterator it = Bouquets.begin(); it != Bouquets.end(); it++)
	{
		for (vector<CZapitChannel*>::iterator jt = (*it)->tvChannels.begin(); jt != (*it)->tvChannels.end(); jt++)
			chans_processed.insert((*jt)->getChannelID());
		for (vector<CZapitChannel*>::iterator jt = (*it) ->radioChannels.begin(); jt != (*it)->radioChannels.end(); jt++)
			chans_processed.insert((*jt)->getChannelID());
	}

	// TODO: use locales
	remainChannels = addBouquet((Bouquets.size() == 0) ? "Alle Kan\xC3\xA4le" : "Andere"); // UTF-8 encoded

	for (tallchans_iterator it=allchans.begin(); it != allchans.end(); it++)
		if (chans_processed.find(it->first) == chans_processed.end())
			unusedChannels.push_back(&(it->second));

	sort(unusedChannels.begin(), unusedChannels.end(), CmpChannelByChName());

	for (ChannelList::iterator it = unusedChannels.begin(); it != unusedChannels.end(); it++)
		remainChannels->addService(findChannelByChannelID((*it)->getChannelID()));

	if ((remainChannels->tvChannels.size() == 0) && (remainChannels->radioChannels.size() == 0))
	{
		deleteBouquet(remainChannels);
		remainChannels = NULL;
	}
}

void CBouquetManager::renumServices()
{
	deleteBouquet(remainChannels);
	
	if (config.getBool("makeRemainingChannelsBouquet", true))
	    makeRemainingChannelsBouquet();

	storeBouquets();
}

CBouquet* CBouquetManager::addBouquet( string name)
{
	CBouquet* newBouquet = new CBouquet(name);
	Bouquets.push_back(newBouquet);
	return newBouquet;
}

void CBouquetManager::deleteBouquet(const unsigned int id)
{
	if (id < Bouquets.size() && Bouquets[id] != remainChannels)
		deleteBouquet(Bouquets[id]);
}

void CBouquetManager::deleteBouquet(const CBouquet* bouquet)
{
	if (bouquet != NULL)
	{
		BouquetList::iterator it = find(Bouquets.begin(), Bouquets.end(), bouquet);

		if (it != Bouquets.end())
		{
			Bouquets.erase(it);
			delete bouquet;
		}
	}
}

//
// -- Find Bouquet-Name, if BQ exists   (2002-04-02 rasc)
// -- Return: Bouqet-ID (found: 0..n)  or -1 (Bouquet does not exist)
//
int CBouquetManager::existsBouquet( string name)
{
	unsigned int i;
	for (i=0; (i<Bouquets.size()) && (Bouquets[i]->Name != name); i++);
	return (i<Bouquets.size()) ?(int)i :(int)-1;
}


//
// -- Check if channel exists in BQ   (2002-04-05 rasc)
// -- Return: True/false
//
bool CBouquetManager::existsChannelInBouquet( unsigned int bq_id, const t_channel_id channel_id)
{
	bool     status = false;
	CZapitChannel  *ch = NULL;

	if (bq_id >= 0 && bq_id <= Bouquets.size()) {
		// query TV-Channels  && Radio channels
		ch = Bouquets[bq_id]->getChannelByChannelID(channel_id, 0);
		if (ch)  status = true;
	}

	return status;

}


void CBouquetManager::moveBouquet(const unsigned int oldId, const unsigned int newId)
{
	if ((oldId < Bouquets.size()) && (newId < Bouquets.size()))
	{
		BouquetList::iterator it = Bouquets.begin();

		advance(it, oldId);
		CBouquet* tmp = *it;
		Bouquets.erase(it);

		advance(it, newId - oldId);
		Bouquets.insert(it, tmp);
	}
}

void CBouquetManager::clearAll()
{
	for (unsigned int i=0; i<Bouquets.size(); i++)
		delete Bouquets[i];

	Bouquets.clear();
	remainChannels = NULL;
}

CZapitChannel* CBouquetManager::findChannelByChannelID(const t_channel_id channel_id)
{
	tallchans_iterator itChannel = allchans.find(channel_id);
	if (itChannel != allchans.end())
		return &(itChannel->second);

	return NULL;
}

CBouquetManager::ChannelIterator::ChannelIterator(CBouquetManager* owner, const bool TV)
{
	Owner = owner;
	tv = TV;
	if (Owner->Bouquets.size() == 0)
		c = -2;
	else
	{
		b = 0;
		c = -1; 
		(*this)++;
	}
}

CBouquetManager::ChannelIterator CBouquetManager::ChannelIterator::operator ++(int)
{
	if (c != -2)  // we can add if it's not the end marker
	{
		c++;
		if ((unsigned int) c >= getBouquet()->size())
		{
			for (b++; b < Owner->Bouquets.size(); b++)
				if (getBouquet()->size() != 0)
				{
					c = 0;
					goto end;
				}
			c = -2;
		}
	}
 end:
	return(*this);
}

CZapitChannel* CBouquetManager::ChannelIterator::operator *()
{
	return (*getBouquet())[c];               // returns junk if we are an end marker !!
}

CBouquetManager::ChannelIterator CBouquetManager::ChannelIterator::FindChannelNr(const unsigned int channel)
{
	c = channel;
	for (b = 0; b < Owner->Bouquets.size(); b++)
		if (getBouquet()->size() > (unsigned int)c)
			goto end;
		else
			c -= getBouquet()->size();
	c = -2;
 end:
	return (*this);
}

int CBouquetManager::ChannelIterator::getLowestChannelNumberWithChannelID(const t_channel_id channel_id)
{
	int i = 0;

	for (b = 0; b < Owner->Bouquets.size(); b++)
		for (c = 0; (unsigned int) c < getBouquet()->size(); c++, i++)
			if ((**this)->getChannelID() == channel_id)
			    return i;
	return -1; // not found
}


int CBouquetManager::ChannelIterator::getNrofFirstChannelofBouquet(const unsigned int bouquet_nr)
{
	if (bouquet_nr >= Owner->Bouquets.size())
		return -1;  // not found

	int i = 0;

	for (b = 0; b < bouquet_nr; b++)
		i += getBouquet()->size();

	return i;
}
