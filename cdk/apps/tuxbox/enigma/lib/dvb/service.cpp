#include <lib/dvb/service.h>

#include <lib/base/i18n.h>
#include <lib/dvb/dvb.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>

#include <src/parentallock.h>

int eServiceHandler::flags=0;

eServiceHandler::eServiceHandler(int id): id(id)
{
}

eServiceHandler::~eServiceHandler()
{
}

eService *eServiceHandler::createService(const eServiceReference &node)
{
	(void)node;
	return 0;
}

int eServiceHandler::play(const eServiceReference &service, int workaround)
{
	(void)service;
	return -1;
}

int eServiceHandler::serviceCommand(const eServiceCommand &cmd)
{
	(void)cmd;
	return -1;
}

PMT *eServiceHandler::getPMT()
{
	return 0;
}

void eServiceHandler::setPID(const PMTEntry *)
{
	return;
}

SDT *eServiceHandler::getSDT()
{
	return 0;
}

EIT *eServiceHandler::getEIT()
{
	return 0;
}

int eServiceHandler::getFlags()
{
	return flags;
}

int eServiceHandler::getState()
{
	return 0;
}

int eServiceHandler::getAspectRatio()
{
	return 0;
}

int eServiceHandler::getErrorInfo()
{
	return 0;
}

int eServiceHandler::stop( int workaround )
{
	return 0;
}

int eServiceHandler::getPosition(int)
{
	return -1;
}

void eServiceHandler::setAudioStream( unsigned int )
{
}

void eServiceHandler::enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback)
{
	(void)dir;
	(void)callback;
	return;
}

void eServiceHandler::leaveDirectory(const eServiceReference &dir)
{
	(void)dir;
	return;
}

eService *eServiceHandler::addRef(const eServiceReference &service)
{
	(void)service;
	return 0;
}

void eServiceHandler::removeRef(const eServiceReference &service)
{
	(void)service;
}

eString eServiceHandler::getInfo(int id)
{
	(void)id;
	return "";
}

eServiceInterface *eServiceInterface::instance;

eServiceInterface *eServiceInterface::getInstance()
{
	return instance;
}

void eServiceInterface::handleServiceEvent(const eServiceEvent &evt)
{
	serviceEvent(evt);
}

int eServiceInterface::switchServiceHandler(int id, int workaround )
{
	if (currentServiceHandler && (currentServiceHandler->getID() == id))
	{
		currentServiceHandler->stop(workaround);
		return 0;
	}

	stop(workaround);
	eServiceHandler *handler=getServiceHandler(id);
	if (!handler)
		return -1;
	
	conn=CONNECT(handler->serviceEvent, eServiceInterface::handleServiceEvent);
	currentServiceHandler=handler;
	return 0;
}

eServiceInterface::eServiceInterface()
{
	currentServiceHandler = 0;
	if (!instance)
		instance = this;
}

eServiceInterface::~eServiceInterface()
{
	if (instance == this)
		instance = 0;
	stop();
}

int eServiceInterface::registerHandler(int id, eServiceHandler *handler)
{
	if (handlers.count(id))
		return -1;
	handlers.insert(std::pair<int,eServiceHandler*>(id, handler));
	return 0;
}

int eServiceInterface::unregisterHandler(int id)
{
	std::map<int,eServiceHandler*>::iterator i=handlers.find(id);
	if (i == handlers.end())
		return -1;
	if (i->second == currentServiceHandler)
		stop();
	handlers.erase(i);
	return 0;
}

eServiceHandler *eServiceInterface::getServiceHandler(int id)
{
	std::map<int,eServiceHandler*>::iterator i=handlers.find(id);
	if (i == handlers.end())
		return 0;
	return i->second;
}

int eServiceInterface::play(const eServiceReference &s)
{
	return play(s,0);
}

int eServiceInterface::play(const eServiceReference &s, int workaround )
{
	int pLockActive = pinCheck::getInstance()->pLockActive();
	if ( !workaround && s.isLocked() && pLockActive && !pinCheck::getInstance()->checkPin(pinCheck::parental ))
	{
		eWarning("service is parentallocked... don't play");
		return -1;
	}
	if (switchServiceHandler(s.type, workaround))
	{
		eWarning("couldn't play service type %d", s.type);
		return -1;
	}
	service=s;
	return currentServiceHandler->play(s, workaround);
}

int eServiceInterface::stop(int workaround)
{
	if (!currentServiceHandler)
		return -1;
	int res=currentServiceHandler->stop(workaround);
	conn.disconnect();
	currentServiceHandler=0;
	service=eServiceReference();
	return res;
}

void eServiceInterface::enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback)
{
	int pLockActive = pinCheck::getInstance()->pLockActive();
	if ( dir.isLocked() && pLockActive && !pinCheck::getInstance()->checkPin(pinCheck::parental))
	{
		eWarning("directory is parentallocked... don't enter");
		return;
	}
	for (std::map<int,eServiceHandler*>::iterator i(handlers.begin()); i != handlers.end(); ++i)
		i->second->enterDirectory(dir, callback);
}

void eServiceInterface::leaveDirectory(const eServiceReference &dir)
{
	for (std::map<int,eServiceHandler*>::iterator i(handlers.begin()); i != handlers.end(); ++i)
		i->second->leaveDirectory(dir);
}

eService *eServiceInterface::addRef(const eServiceReference &service)
{
	eServiceHandler *handler=getServiceHandler(service.type);
	if (handler)
		return handler->addRef(service);
	else
		return 0;
}

void eServiceInterface::removeRef(const eServiceReference &service)
{
	eServiceHandler *handler=getServiceHandler(service.type);
	if (handler)
		return handler->removeRef(service);
}

eAutoInitP0<eServiceInterface> i_eServiceInteface(eAutoInitNumbers::service, "eServiceInterface");
