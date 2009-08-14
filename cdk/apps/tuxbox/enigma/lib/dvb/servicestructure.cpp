#include <lib/dvb/servicestructure.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/base/i18n.h>

eServiceStructureHandler::eServiceStructureHandler(): eServiceHandler(eServiceReference::idStructure), cache(*this)
{
        //eDebug("[eServiceStructureHandler] registering serviceInterface %d", id);
	init_eServiceStructureHandler();
}
void eServiceStructureHandler::init_eServiceStructureHandler()
{
	if (eServiceInterface::getInstance()->registerHandler(id, this)<0)
		eFatal("couldn't register serviceHandler %d", id);
	cache.addPersistentService(eServiceReference(eServiceReference::idStructure, eServiceReference::flagDirectory, modeRoot), new eService(_("root node")));
	cache.addPersistentService(eServiceReference(eServiceReference::idStructure, eServiceReference::flagDirectory, modeTV), new eService(_("TV mode")));
	cache.addPersistentService(eServiceReference(eServiceReference::idStructure, eServiceReference::flagDirectory, modeRadio), new eService(_("Radio Mode")));
#ifndef DISABLE_FILE
	cache.addPersistentService(eServiceReference(eServiceReference::idStructure, eServiceReference::flagDirectory, modeFile), new eService(_("File Mode")));
#endif
	cache.addPersistentService(eServiceReference(eServiceReference::idStructure, eServiceReference::flagDirectory, modeTvRadio), new eService(_("TV/Radio")));
	cache.addPersistentService(eServiceReference(eServiceReference::idStructure, eServiceReference::flagDirectory, modeBouquets), new eService(_("Bouquets")));
}

eServiceStructureHandler::~eServiceStructureHandler()
{
	eServiceInterface::getInstance()->unregisterHandler(id);
}

void eServiceStructureHandler::enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback)
{
	cache.enterDirectory(dir, callback);
}

void eServiceStructureHandler::leaveDirectory(const eServiceReference &dir)
{
	cache.leaveDirectory(dir);
}

void eServiceStructureHandler::loadNode(eServiceCache<eServiceStructureHandler>::eNode &n, const eServiceReference &r)
{
	(void)n;
	(void)r;
}
 
eService *eServiceStructureHandler::createService(const eServiceReference &node)
{
	eFatal("structure should create: %d.%d", node.type, node.data[0]);
	return 0;
}

eService* eServiceStructureHandler::addRef(const eServiceReference &c)
{
	return cache.addRef(c);
}

void eServiceStructureHandler::removeRef(const eServiceReference &c)
{
	cache.removeRef(c);
}

eAutoInitP0<eServiceStructureHandler> i_eServiceStructureHandler(eAutoInitNumbers::service+1, "eServiceStructureHandler");
