#include <stdio.h>
#include <sys/stat.h>

#include <lib/gui/ewindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>
#include <lib/dvb/servicefile.h>
#include <lib/gdi/lcd.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/info.h>
#include <lib/dvb/serviceexternal.h>

/*
 * In order to register an external (media)player to a file extension (e.g. ".xyz", place
 * a .cfg file in the plugin directory, with contents:
 *
 *		type=5
 *		pattern=.xyz
 *		command=/usr/bin/xzyplay
 *		needrc=1 (if your player needs the rc)
 *		needlcd=1 (if your player needs the lcd)
 *		needfb=1 (if your player needs the framebuffer)
 *
 * Now when a file "filename.xyz" is selected in filemode, the external player will be started:
 *
 * 		/usr/bin/xyzplay /path/to/file/filename.xyz
 *
 * In order to register an external (media)player to a directory, we need to match a certain
 * file inside the directory (e.g. 'dirname/special.file'). Place a .cfg file in the plugin
 * directory, with contents:
 *
 *		type=5
 *		dirpattern=filename.xyz
 *		command=/usr/bin/xyzplay
 *		needrc=1 (if your player needs the rc)
 *		needlcd=1 (if your player needs the lcd)
 *		needfb=1 (if your player needs the framebuffer)
 *
 * Now at the same level as 'dirname', an entry 'Start player' will be shown in filemode.
 * When this entry is selected, the player will be started with the command:
 *
 *		/usr/bin/xyzplay /path/to/dir/dirname
 *
 *
 * Only one 'pattern' is allowed per type 5 plugin. Also, only one 'dirpattern' is allowed
 * per type 5 plugin.
 * However, it is allowed to have both a 'pattern' and a 'dirpattern' in the same cfg file.
 *
 */

eServiceHandlerExternal *eServiceHandlerExternal::instance = NULL;

eServiceHandlerExternal::eServiceHandlerExternal(): eServiceHandler(0x0fff)
{
	if (eServiceInterface::getInstance()->registerHandler(id, this) < 0)
	{
		eFatal("couldn't register eServiceHandlerExternal %d", id);
	}
	instance = this;
	CONNECT(eServiceFileHandler::getInstance()->fileHandlers, eServiceHandlerExternal::addFile);
	CONNECT(eServiceFileHandler::getInstance()->directoryHandlers, eServiceHandlerExternal::addDirectory);
	eDebug("eServiceHandlerExternal registered");
}

eServiceHandlerExternal::~eServiceHandlerExternal()
{
	eServiceInterface::getInstance()->unregisterHandler(id);
}

void eServiceHandlerExternal::addFileHandler(const eString &pattern, const eString &command, int needfb, int needrc, int needlcd)
{
	FileExtensionScriptInfo info;
	info.needfb = needfb;
	info.needrc = needrc;
	info.needlcd = needlcd;
	info.command = command;
	info.pattern = pattern;
	extensionFileList.push_back(info);
}

void eServiceHandlerExternal::addDirectoryHandler(const eString &pattern, const eString &command, int needfb, int needrc, int needlcd)
{
	FileExtensionScriptInfo info;
	info.needfb = needfb;
	info.needrc = needrc;
	info.needlcd = needlcd;
	info.command = command;
	info.pattern = pattern;
	/*
	 * make sure we have a trailing slash in our pattern,
	 * as this is how we'll get offered our directory names
	 */
	if (info.pattern.right(1) != "/") info.pattern += "/";
	extensionDirectoryList.push_back(info);
}

void eServiceHandlerExternal::addFile(void *node, const eString &filename)
{
	for (unsigned int i = 0; i < extensionFileList.size(); i++)
	{
		int matchsize = extensionFileList[i].pattern.length();
		if (filename.right(matchsize).upper() == extensionFileList[i].pattern.upper())
		{
			eServiceReference ref(id, 0, filename);
			eString filenamestripped = filename;

			uint i = filename.find_last_of('/');
			if (i != eString::npos)
			{
				filenamestripped = filename.mid(i + 1, filename.length());
			}
			ref.descr = filenamestripped.c_str();
			eServiceFileHandler::getInstance()->addReference(node, ref);
			eDebug("Add file: %s", filename.c_str());
			return;
		}
	}
}

void eServiceHandlerExternal::addDirectory(void *node, const eString &filename)
{
	for (unsigned int i = 0; i < extensionDirectoryList.size(); i++)
	{
		int matchsize = extensionDirectoryList[i].pattern.length();
		if (filename.right(matchsize).upper() == extensionDirectoryList[i].pattern.upper())
		{
			eServiceReference ref(id, 0, filename);
			ref.descr = _("Start player");
			eServiceFileHandler::getInstance()->addReference(node, ref);
			eDebug("Add directory: %s", filename.c_str());
			return;
		}
	}
}

int eServiceHandlerExternal::play(const eServiceReference &service, int workaround)
{
	if (service.path)
	{
		struct stat64 s;
		if (::stat64(service.path.c_str(), &s))
		{
			eDebug("file %s does not exist.. don't play", service.path.c_str() );
			return -1;
		}
	}
	else
	{
		return -1;
	}

	if (service.path.right(1) == "/")
	{
		/* trailing slash: service is a directory */
		for (unsigned int i = 0; i < extensionDirectoryList.size(); i++)
		{
			int matchsize = extensionDirectoryList[i].pattern.length();
			if (service.path.right(matchsize).upper() == extensionDirectoryList[i].pattern.upper())
			{
				eString command = extensionDirectoryList[i].command + " \"" + service.path + "\"";
				eDebug("play command %s\n", command.c_str());
				ePlayerThread *p = new ePlayerThread(command, extensionDirectoryList[i].needfb, extensionDirectoryList[i].needrc, extensionDirectoryList[i].needlcd);
				p->start();
				return 0;
			}
		}
	}
	else
	{
		/* service is a normal file */
		for (unsigned int i = 0; i < extensionFileList.size(); i++)
		{
			int matchsize = extensionFileList[i].pattern.length();
			if (service.path.right(matchsize).upper() == extensionFileList[i].pattern.upper())
			{
				eString command = extensionFileList[i].command + " \"" + service.path + "\"";
				eDebug("play command %s\n", command.c_str());
				ePlayerThread *p = new ePlayerThread(command, extensionFileList[i].needfb, extensionFileList[i].needrc, extensionFileList[i].needlcd);
				p->start();
				return 0;
			}
		}
	}
	return -1;
}

int eServiceHandlerExternal::stop(int workaround)
{
	eDebug("Stop file\n");
	return 0;
}

eService *eServiceHandlerExternal::addRef(const eServiceReference &service)
{
	return eServiceFileHandler::getInstance()->addRef(service);
}

void eServiceHandlerExternal::removeRef(const eServiceReference &service)
{
	return eServiceFileHandler::getInstance()->removeRef(service);
}

void ePlayerThread::start()
{
	if (!thread_running())
	{
		if (needrc)
		{
			eRCInput::getInstance()->lock();
		}

		if (needfb)
		{
			fbClass::getInstance()->lock();
		}

#ifndef DISABLE_LCD
		if (needlcd && eSystemInfo::getInstance()->hasLCD())
		{
			eDBoxLCD::getInstance()->lock();
		}
#endif

		run();
	}
	else
	{
		eDebug("don't start player.. another one is running");
	}
}

void ePlayerThread::thread()
{
	if (thread_running())
	{
		eDebug("player thread running.. start player now");
	}
	else
	{
		eDebug("start player now");
	}
	system(command.c_str());
	eDebug("player finished");
}

void ePlayerThread::recv_msg(const int &)
{
	finalize_player();
}

void ePlayerThread::thread_finished()
{
	message.send(1);
}

void ePlayerThread::finalize_player()
{
	if (needfb)
	{
		fbClass::getInstance()->unlock();
	}

#ifndef DISABLE_LCD
	if (needlcd && eSystemInfo::getInstance()->hasLCD())
	{
		eDBoxLCD::getInstance()->unlock();
	}
#endif

	if (needrc)
	{
		eRCInput::getInstance()->unlock();
	}
	delete this;
}

eAutoInitP0<eServiceHandlerExternal> i_eServiceHandlerExternal(eAutoInitNumbers::service + 2, "eServiceHandlerExternal");
