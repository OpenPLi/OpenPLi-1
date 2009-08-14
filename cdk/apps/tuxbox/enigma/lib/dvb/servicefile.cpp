#include <lib/dvb/servicefile.h>
#include <lib/dvb/servicestructure.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>
#include <lib/base/i18n.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <unistd.h>
// TODO: Bad include on a higher level, but mounts should be in lib/system anyway
#include <src/enigma_mount.h>

/*
		eServiceFileHandler is some kind of a "multplexer", it gives
		you a filesystem structure (with ref.path = path), and calls
		"addFile" with every file. various file handlers can hook into
		this handler and parse files and return a eServiceReference.
		
		services itself are addRef'd first through the real handler (e.g.
		the mp3 handler), then reflected back into serviceFileHandler,
		then parsed by the cache (different file handlers to not need
		to have their own), calls to createService will be forwared to
		the module handlers.
*/

eServiceFileHandler *eServiceFileHandler::instance;

static const int dirflags=eServiceReference::isDirectory|eServiceReference::canDescent|eServiceReference::mustDescent|eServiceReference::shouldSort|eServiceReference::sort1;

eServiceFileHandler::eServiceFileHandler(): eServiceHandler(eServiceReference::idFile), cache(*this)
{
        //eDebug("[eServiceFileHandler] registering serviceInterface %d", id);
	if (eServiceInterface::getInstance()->registerHandler(id, this)<0)
		eFatal("couldn't register serviceHandler %d", id);
	instance=this;

	cache.addPersistentService(eServiceReference(eServiceReference::idFile, dirflags, "/"), new eService("Root filesystem"));
}

eServiceFileHandler::~eServiceFileHandler()
{
	eServiceInterface::getInstance()->unregisterHandler(id);
}

void eServiceFileHandler::loadNode(eServiceCache<eServiceFileHandler>::eNode &node, const eServiceReference &ref)
{
	switch (ref.type)
	{
	case eServiceReference::idStructure:
		switch (ref.data[0])
		{
			case eServiceStructureHandler::modeRoot:
			case eServiceStructureHandler::modeFile:
			{
				cache.addToNode(node, eServiceReference(eServiceReference::idFile, dirflags, "/"));

				eDevMountMgr *devMountMgr = eDevMountMgr::getInstance();
				int nrDevs = devMountMgr->mountPointCount();
	
				char *fullText = _("(%d%% in use)");
				for(int i = 0; i < nrDevs; ++i)
				{
					eMountPoint mnt = devMountMgr->getMountPointData(i);
					
					// a bit overhead... as described below in the network mounts
					// Also: does this lead to memory leaks?
					
					eServiceReference sRef(eServiceReference::idFile, dirflags, mnt.localDir + "/");
					
					cache.updatePersistentService(sRef,
						new eService(mnt.getLongDescription() + " " +
						eString().sprintf(fullText, getFsFullPerc(mnt.localDir.c_str()))));
					cache.addToNode(node, sRef);
				}

				eNetworkMountMgr *mountMgr = eNetworkMountMgr::getInstance();
				int mountCount = mountMgr->mountPointCount();
				for(int i = 0; i < mountCount; ++i)
				{
					eMountPoint mountinfo = mountMgr->getMountPointData(i);
					if(mountinfo.fstype != eMountPoint::deviceMount && mountinfo.isMounted()) // If no dev mount
					{
						eString mountDesc = mountinfo.getLongDescription() +
							" - " + _("network mount") + " " +
							eString().sprintf(fullText, getFsFullPerc(mountinfo.localDir.c_str()));
						
						// a bit overhead here since addPersistentService is called everytime
						// "My Dreambox" is entered but this is the only way a mount is immediately being
						// shown without restarting enigma.
						// Preferable we would call addPersistentService when the mount is made but
						// then eMountMgr should use eServiceFileHandler and vice versa.
						// until a better solution is being build this will do.
						
						eServiceReference sRef(eServiceReference::idFile, dirflags, mountinfo.localDir + "/");
						
						cache.updatePersistentService(sRef, new eService(mountDesc));
						cache.addToNode(node, sRef);
					}
				}
				// Add all links present in $CONFIGDIR/enigma/root/ to root-menu
				DIR *d=opendir(CONFIGDIR "/enigma/root/");
				if (d)
				{
					while (struct dirent64 *e=readdir64(d))
					{
						if (!(strcmp(e->d_name, ".") && strcmp(e->d_name, "..")))
							continue;
						eString filename;
						filename.sprintf("%s/enigma/root/%s", CONFIGDIR,e->d_name);
						struct stat64	 s;
						if (stat64(filename.c_str(), &s)<0)
							continue;
						if (S_ISDIR(s.st_mode))
						{
							filename+="/";
							eServiceReference service(eServiceReference::idFile, dirflags, filename);
							service.data[0]=!!S_ISDIR(s.st_mode);
							cache.addToNode(node, service);
						}	
					}
					closedir(d);
				}
				break;
			}
		}
		break;
	case eServiceReference::idFile:
	{
		DIR *d=opendir(ref.path.c_str());
		if (!d)
			return;
		while (struct dirent64 *e=readdir64(d))
		{
			if (!(strcmp(e->d_name, ".") && strcmp(e->d_name, "..")))
				continue;
			eString filename;
			
			filename=ref.path;
			filename+=e->d_name;
			
			struct stat64	 s;
			if (stat64(filename.c_str(), &s)<0)
				continue;
		
			if (S_ISDIR(s.st_mode))
				filename+="/";
				
			if (S_ISDIR(s.st_mode))
			{
				/* we allow a servicehandler to be registered to a directory */
				directoryHandlers((void*)&node, filename);
				eServiceReference service(eServiceReference::idFile, dirflags, filename);
				service.data[0]=!!S_ISDIR(s.st_mode);
				cache.addToNode(node, service);
			} else
				fileHandlers((void*)&node, filename);
		}
		closedir(d);
		break;
	}
	default:
		break;
	}
}

void eServiceFileHandler::addReference(void *node, const eServiceReference &ref)
{
	if (!node)
		result=ref;	// super unthreadsafe und nichtmal reentrant.
	else
		cache.addToNode(*(eServiceCache<eServiceFileHandler>::eNode*)node, ref);
}

eService *eServiceFileHandler::createService(const eServiceReference &node)
{
	if (node.type == id)
	{
		int n=node.path.size()-2;
		if (n<0)
			n=0;
		eString path=node.path.mid(node.path.rfind("/", n)+1);
		if (!isUTF8(path))
			path=convertLatin1UTF8(path);
		return new eService(path.c_str());
	}
	eServiceHandler *handler=eServiceInterface::getInstance()->getServiceHandler(node.type);
	if (!handler)
		return 0;
	return handler->createService(node);
}

void eServiceFileHandler::enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback)
{
	cache.enterDirectory(dir, callback);
}

void eServiceFileHandler::leaveDirectory(const eServiceReference &dir)
{
	cache.leaveDirectory(dir);
}

eService *eServiceFileHandler::addRef(const eServiceReference &service)
{
	return cache.addRef(service);
}

void eServiceFileHandler::removeRef(const eServiceReference &service)
{
	return cache.removeRef(service);
}

int eServiceFileHandler::lookupService(eServiceReference &ref, const char *filename)
{
	result=eServiceReference();
	fileHandlers(0, filename);
	if (result)
	{
		ref=result;
		return 1;
	}
	return 0;
}

int eServiceFileHandler::getFsFullPerc(const char* filesystem)
{
	long blocksUsed = 0;
	long blocksPercentUsed = 0;
	struct statfs s;
	
	if(statfs(filesystem, &s) == 0)
	{
		if((s.f_blocks > 0))
		{
			blocksUsed = s.f_blocks - s.f_bfree;
			blocksPercentUsed = 0;
			if(blocksUsed + s.f_bavail)
			{
				blocksPercentUsed = (int)((((long long) blocksUsed) * 100
				+ (blocksUsed + s.f_bavail)/2) / (blocksUsed + s.f_bavail));
			}
		}
	}
	return blocksPercentUsed;
}

eAutoInitP0<eServiceFileHandler> i_eServiceFileHandler(eAutoInitNumbers::service+1, "eServiceFileHandler");
