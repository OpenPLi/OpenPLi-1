#ifndef __http_dyn_h_
#define __http_dyn_h_
#include "httpd.h"
#include <lib/base/estring.h>

class eHTTPDyn: public eHTTPDataSource
{
	eString result;
	int wptr, size;
	void init_eHTTPDyn(eHTTPConnection *c);
public:
	eHTTPDyn(eHTTPConnection *c, eString result);
	~eHTTPDyn();
	int doWrite(int);
};

class eHTTPDynPathResolver: public eHTTPPathResolver
{
	struct eHTTPDynEntry
	{
		eString request, path;
		eString (*function)(eString request, eString path, eString opt, eHTTPConnection *content);
		bool mustAuth;
		
		eHTTPDynEntry(eString request, eString path, eString (*function)(eString, eString, eString, eHTTPConnection *), bool auth)
			: request(request), path(path), function(function), mustAuth(auth)
		{
		}
	};
	ePtrList<eHTTPDynEntry> dyn;
public:
	void addDyn(eString request, eString path, eString (*function)(eString, eString, eString, eHTTPConnection *conn), bool mustAuth=false );
	eHTTPDynPathResolver();
	eHTTPDataSource *getDataSource(eString request, eString path, eHTTPConnection *conn);
};

#endif
