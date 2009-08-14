#ifndef __lib_dvb_servicecache_h
#define __lib_dvb_servicecache_h

#include <map>
#include <list>
#include <lib/dvb/service.h>

class eService;

class eServiceCacheBase
{
public:
	struct eCachedService
	{
		int refcnt;
		eService *service;
	};
	struct eNode
	{
		int refcnt;
		int addRef()
		{
			return ++refcnt;
		}
		int removeRef()
		{
			return --refcnt;
		}
		std::list<eServiceReference> content;
		eNode()
		{
			refcnt=0;
		}
	};
protected:
	virtual eService *createService( const eServiceReference &ref )=0;
	virtual void loadNode( eNode& node, const eServiceReference &ref )=0;
private:
	std::map<eServiceReference,eCachedService> services;
	std::map<eServiceReference,eNode> cache;
public:
	eServiceCacheBase(){};
	virtual ~eServiceCacheBase(){};
	void enterDirectory(const eServiceReference &parent, Signal1<void, const eServiceReference&> &callback);
	void leaveDirectory(const eServiceReference &parent);
	void addPersistentService(const eServiceReference &serviceref, eService *service);
	eService *addRef(const eServiceReference &serviceref);
	void removeRef(const eServiceReference &serviceref);
	void addToNode(eNode &node, const eServiceReference &ref)
	{
		node.content.push_back(ref);
	}
	void updatePersistentService(const eServiceReference &serviceref, eService *service);
};

template <class Factory>
class eServiceCache: public eServiceCacheBase
{
	Factory &factory;
	
public:
	eServiceCache(Factory &factory): factory(factory)
	{
	}
	eService *createService( const eServiceReference &ref )
	{
		return factory.createService( ref );
	}
	void loadNode( eNode &node, const eServiceReference &ref )
	{
		factory.loadNode( node, ref );
	}
};

#endif
