#include <lib/dvb/servicecache.h>
#include <lib/system/econfig.h>

void eServiceCacheBase::enterDirectory(const eServiceReference &parent, Signal1<void, const eServiceReference&> &callback)
{
//  int pLockActive = eConfig::getInstance()->pLockActive();
	std::map<eServiceReference,eNode>::iterator i=cache.find(parent);
	eNode *node=0;
	if (i == cache.end())
	{
		node=&cache.insert(std::pair<eServiceReference,eNode>(parent, eNode())).first->second;
		loadNode(*node, parent);
	} else
		node=&i->second;
	node->addRef();
	for (std::list<eServiceReference>::iterator i(node->content.begin()); i != node->content.end(); ++i)
		callback(*i);
}

void eServiceCacheBase::leaveDirectory(const eServiceReference &parent)
{
	std::map<eServiceReference,eNode>::iterator i=cache.find(parent);
	if (i == cache.end())
		eDebug("leaveDirectory on non-cached directory!");
	else
	{
		eNode &node=i->second;
		if (node.removeRef() <= 0)
			cache.erase(i);
	}
}

void eServiceCacheBase::addPersistentService(const eServiceReference &serviceref, eService *service)
{
	eCachedService c;
	c.refcnt=-1;
	c.service=service;
	services.insert(std::pair<eServiceReference, eCachedService>(serviceref, c));
}

void eServiceCacheBase::updatePersistentService(const eServiceReference &serviceref, eService *service)
{
	std::map<eServiceReference,eCachedService>::iterator c=services.find(serviceref);
	if(c != services.end())
	{
		// Delete the old service
		delete c->second.service;
		services.erase(c);
	}
	
	// Add the new service
	addPersistentService(serviceref, service);
}

eService *eServiceCacheBase::addRef(const eServiceReference &serviceref)
{
	if (services.find(serviceref) == services.end())  // service not exist in cache ?
	{
		eCachedService c;   // create new Cache Entry
		c.refcnt=1;					// currently one Object holds a reference to the new cache entry
		c.service=createService(serviceref);
		if (!c.service)
		{
			// eDebug("createService failed!");
			return 0;
		}
		services.insert(std::pair<eServiceReference,eCachedService>(serviceref, c));
		return c.service;
	} else
	{
		eCachedService &c=services.find(serviceref)->second;
		if (c.refcnt != -1)
			c.refcnt++;
		return c.service;
	}
}

void eServiceCacheBase::removeRef(const eServiceReference &serviceref)
{
	std::map<eServiceReference,eCachedService>::iterator c=services.find(serviceref);
	if (c == services.end())
	{
		eDebug("removeRef on non-existing service!");
		return;
	}
	if ((c->second.refcnt != -1) && ! --c->second.refcnt)
	{
		delete c->second.service;
		services.erase(c);
	}
}
