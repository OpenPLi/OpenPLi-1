#ifndef __XMLTV_H__
#define __XMLTV_H__

#include <lib/base/thread.h>
#include <lib/base/estring.h>

#include "defs.h"
#include "util.h"
#include "epgui.h"

struct conversionSpec {
	eString from, to;
	char encoding;
};

class XMLTVConverter : public eThread {
	std::list<struct conversionSpec> records;
	int convertedCount;

        bool analyseTime( eString t,  time_t *result );
        bool convert( struct conversionSpec &c );
public:
	XMLTVConverter( void );
        void thread( void );
        void thread_finished( void );
        void start( void );
	void addFile( eString from, char encoding, eString to );

        Signal1<void,int> allDone;
};

#endif
