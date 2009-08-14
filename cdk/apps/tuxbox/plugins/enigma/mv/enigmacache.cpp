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

#include "defs.h"
#include "enigmacache.h"


eServiceReferenceDVB getPlayingServiceRef( void )
{
        eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI
();
        return sapi->service;
}

EnigmaCache::EnigmaCache( char *userLangPref, bool loadServices ) :  userLangPref( userLangPref), serviceCount(0)
{
	cacheP = eEPGCache::getInstance();

	if ( loadServices ) {
		Signal1<void,const eServiceReference&> callback;
		CONNECT( callback, EnigmaCache::addToList );
		// bool is 'from beginning'
		eZap::getInstance()->getServiceSelector()->forEachServiceRef( callback,  true );
	}
}

/*
void EnigmaCache::convertDataChannels( void )
{
	std::list<eServiceReferenceDVB>::iterator curService = serviceList.begin();

	bool needSaveServices = false;

        while ( curService != serviceList.end() ) {
			dmsg( (char*)getServiceName( *curService ).c_str(), (int)curService->getServiceType());
		// If not TV or RADIO...
		if ( curService->getServiceType() > 2  ) {


			// Confirm on first one
			if ( ! needSaveServices ) {
				if ( userConfirms( strConvertDataChannelConfirm ) )
					needSaveServices = true;
				else
					break;
			}
			needSaveServices = true;
			curService->setServiceType( 1 );
			break;
		}
		curService++;
	}
	if ( needSaveServices )
		eDVB::getInstance()->settings->saveServices();

//		eDVB::getInstance()->settings->loadServices();
}
*/

bool EnigmaCache::getServiceRef( eString &name, eServiceReferenceDVB *eref )
{
	std::list<eServiceReferenceDVB>::iterator curService = serviceList.begin();

	while ( curService != serviceList.end() ) {
		if ( getServiceName( *curService ) == name ) {
			*eref = *curService;
			return true;
		}
		curService++;
	}
	return false;
}
bool EnigmaCache::isInServiceList( char *name )
{
	std::list<eServiceReferenceDVB>::iterator curService = serviceList.begin();

	while ( curService != serviceList.end() ) {
		if ( strcmp( getServiceName( *curService ).c_str(), name ) == 0 )
			return true;
		curService++;
	}
	return false;
}

eString EnigmaCache::nextBouquetName( bool fromStartFlag ) 
{
	static std::list<eServiceReferenceDVB>::iterator curService;

	if ( fromStartFlag ) 
		curService = serviceList.begin();
	if ( curService == serviceList.end() )
		return "";

	eString name = getServiceName( *curService );
	curService++;
	if (!name.length()) name=".";
	return name;
}

eServiceReferenceDVB *EnigmaCache::nextBouquetRef( bool fromStartFlag ) 
{
	static std::list<eServiceReferenceDVB>::iterator curService;
	static eServiceReferenceDVB ref;

	if ( fromStartFlag ) 
		curService = serviceList.begin();
	if ( curService == serviceList.end() )
		return NULL;

	ref = *curService;
	curService++;
	return &ref;
}


eString getFullServiceName( const eServiceReferenceDVB &ref )
{
	eService *sv = eServiceInterface::getInstance()->addRef( ref );
	eString name;
	if ( sv ) {
		name = sv->service_name;
		eServiceInterface::getInstance()->removeRef( ref );
	}
	else {
		name = getServiceName( ref );
	}

	return name;
}

eString getServiceName( const eServiceReferenceDVB &ref )
{
	eString serviceName;
        if ( ref.descr )
		serviceName = ref.descr; 
	else {
		eService *sv = eServiceInterface::getInstance()->addRef( ref );
		if ( sv ) {
			eString shortName = buildShortServiceName( sv->service_name );
			serviceName = shortName ? shortName : sv->service_name;
			eServiceInterface::getInstance()->removeRef( ref );
		}
	}
	return serviceName;
}



eString buildShortServiceName( const eString &str )
{
	eString tmp;
	static char stropen[3] = { 0xc2, 0x86, 0x00 };
	static char strclose[3] = { 0xc2, 0x87, 0x00 };
	unsigned int open=eString::npos-1;

	while ( (open = str.find(stropen, open+2)) != eString::npos ) {
	        unsigned int close = str.find(strclose, open);
	        if ( close != eString::npos )
			tmp += str.mid( open+2, close-(open+2) );
	}
	if ( tmp == "" )
		tmp = str;

	return tmp;
}

inline bool EnigmaCache::langCodesMatch( char *first, char *second )
{
	return (
		( first[0] == second[0] ) &&
		( first[1] == second[1] ) &&
		( first[2] == second[2] )
	);
}

bool EnigmaCache::eventIsInCache( eString serviceName, time_t startTime )
{
	eServiceReferenceDVB ref;
	bool ret = getServiceRef( serviceName, &ref );
	EITEvent *ev = cacheP->lookupEvent( ref, startTime );
	ret = ret && ev;
	delete ev;
	return ret;
}

bool EnigmaCache::getProgramDescription( eServiceReferenceDVB ref, time_t startTime, char * buffer )
{

	// Without the 60, sometimes get previous program desc

	EITEvent *ev = cacheP->lookupEvent( ref, startTime + 60);
	
	bool gotFlag = false;

	if ( ev ) {
		struct ProgramData p;
		if ( parseProgramData( ev, p, true, false ) ) {
			safeStrncpy( buffer, (char *)p.descr.c_str(), DC_MAX_DESCR_LENGTH );
			gotFlag = true;
		}
	}
	delete ev;	

	return gotFlag;
}

void EnigmaCache::readProgramData( time_t from, time_t to, int maxChannels, bool includeDescriptionsFlag ) 
{
	if ( serviceCount == 0 )
		return;

	std::list<eServiceReferenceDVB>::iterator curService = serviceList.begin();

	int channelNo = 0;
	while ( 
		( channelNo < maxChannels ) &&
		( curService != serviceList.end() )
	) {
		readOneChannelProgramData( true, *curService, from, to, includeDescriptionsFlag );

		curService++;
		channelNo++;
	}
}

void EnigmaCache::readOneChannelProgramData( bool partOfGlobal, eServiceReferenceDVB ref, time_t from, time_t to, bool includeDescriptionsFlag )
{
        timeMapPtr evmap = cacheP->getTimeMapPtr( ref, from, to );
        if ( !evmap )
		return;

	timeMap::const_iterator ibegin = evmap->lower_bound( from );
	if ((ibegin != evmap->end()) && (ibegin != evmap->begin())) {
		if ( ibegin->first != from )
                	--ibegin;
	}
        else
		ibegin=evmap->begin();

	timeMap::const_iterator iend = evmap->lower_bound( to );

	struct ProgramData p;
	p.channelName = getServiceName( ref );

	eServiceReferenceDVB &rref=(eServiceReferenceDVB&)ref;
        int tsidonid =
              (rref.getTransportStreamID().get()<<16)|rref.getOriginalNetworkID().get();

	for (timeMap::const_iterator event(ibegin); event != iend; ++event) {
		EITEvent *ev = new EITEvent(*event->second,tsidonid, event->second->type);

		// This will be done in receiveData, but doing
		// it also here prevents all the work in between
		if (((ev->start_time+ev->duration)>= from) && (ev-> start_time <= to)) {

			p.name = "";
			p.flags = 0;

			if ( parseProgramData( ev, p, includeDescriptionsFlag, true ) ) {
				p.flags |= programDataFlagFromCache;
				p.startTime = ev->start_time;
				p.duration = ev->duration;
				p.idxFic=-1;
				p.offset=-1;
				if ( partOfGlobal )
					/*emit*/ gotData( p );
				else
					/*emit*/ gotOneChannelData( p );
			}
		}
		delete ev;
	}
}

bool EnigmaCache::parseProgramData( EITEvent *ev, struct ProgramData &p, bool descrFlag, bool contentFlag )
{
	if ( descrFlag )
		p.descr = "";

	eString otherLangName, otherLangDescr;

	for (ePtrList<Descriptor>::const_iterator d(ev->descriptor); d != ev->descriptor.end(); ++d ) {
		if ( d->Tag() == DESCR_SHORT_EVENT ) {
			const ShortEventDescriptor *s= (const ShortEventDescriptor*)*d;

			if ( langCodesMatch( (char *)(s->language_code), userLangPref ) )
				p.name = s->event_name;
			else
				otherLangName = s->event_name;

                        if ( descrFlag ) {
				eString descrTemp = s->text;

				while ( descrTemp[0] == ' ' )
					descrTemp.erase(0);

				if (
					( descrTemp.length() > 0 ) &&
					( ! ( descrTemp.left(15) == "Geen informatie" ) ) &&
					( descrTemp != p.name ) &&
					( descrTemp != otherLangName )
				) {
					if ( langCodesMatch( (char *) (s->language_code), userLangPref ) ) 
       	                        		p.descr = p.descr + descrTemp + eString( "\n\n" );
					else
       	                        		otherLangDescr = otherLangDescr + descrTemp + eString( "\n\n" );
				}
			}
		}
                else if ( 
			( descrFlag ) &&
			( d->Tag() == DESCR_EXTENDED_EVENT ) 
		) {
              		ExtendedEventDescriptor *ss=(ExtendedEventDescriptor*)*d;

			if ( langCodesMatch( ss->language_code, userLangPref ) )
#if ENIGMAVERSION > 1074
               		        p.descr += ss->text;
#else
               		        p.descr += ss->item_description;
#endif
			else
#if ENIGMAVERSION > 1074
             		        otherLangDescr += ss->text;
#else
              		        otherLangDescr += ss->item_description;
#endif
               	}
		else if ( 
			( contentFlag ) &&
			( d->Tag() == DESCR_CONTENT ) 
		) {
                        ContentDescriptor *cod=(ContentDescriptor*)*d;

                        for( ePtrList<descr_content_entry_struct>::iterator ce(cod->contentList.begin()); ce != cod->contentList.end(); ++ce) {
				if ( 
					( ce->content_nibble_level_1 == 1 ) &&
					( ce->content_nibble_level_2 < 4 )
				) {
					p.flags |= programDataFlagFilm;
					break;
				}
       	                 }
		}
	}

	// If we only got name in non-preferred language,
	// better use it
	if ( 		
		( p.name.length() == 0 ) &&
		( otherLangName.length() > 0 )
	)
		p.name = otherLangName;

	// If we only got descr in non-preferred language,
	// better use it
	if ( 		
		( descrFlag ) &&
		( p.descr.length() == 0 ) &&
		( otherLangDescr.length() > 0 )
	)
		p.descr = otherLangDescr;

	return ( p.name.length() > 0 );
}

void EnigmaCache::addToList( const eServiceReference& ref )
{
	if ( 
		( ref.type == eServiceReference::idDVB ) &&
		( ! ( ref.flags & eServiceReference::isMarker ) )
	) {
                serviceList.push_back( (const eServiceReferenceDVB&) ref);
		serviceCount++;
	}
}
