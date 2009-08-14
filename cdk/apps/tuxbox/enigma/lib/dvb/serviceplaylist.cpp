#include <lib/dvb/serviceplaylist.h>
#include <lib/dvb/servicefile.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>
#include <lib/system/file_eraser.h>
#include <lib/base/i18n.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <sstream>
#include <iostream>
/*
		eServicePlaylistHandler hooks into the file handler eServiceFileHandler
		and implements an "addFile" for various playlist formats.
		
		it implements also enterDirectory and leaveDirectory for reading
		the playlists.
		
		playlists are stored in eServiceFileHandler's cache in an eService-based
		structure (ePlaylist)
*/

ePlaylist::ePlaylist(): eService("playlist"), changed(255), lockActive(false), lockCount(0)
{
	current=list.end();
}

ePlaylist::~ePlaylist()
{
	eDebug("destroy %s", filename.c_str() );
}

void ePlaylist::clear()
{
	changed=1;
	list.clear();
	current = list.end();
}

int ePlaylist::load(const char *filename)
{
	this->filename=filename;
	// first determine the filepath
	filepath = filename;
	int service_name_set=service_name!="playlist";
	if (!service_name_set)
		service_name=eString("playlist: ") + filepath.mid(filepath.rfind('/')+1);
	filepath = filepath.left(filepath.rfind('/') + 1);

	lockPlaylist();

	eDebug("loading playlist... %s", filename);
	list.clear();
		
	FILE *fp=fopen(filename, "rt");
 
	int entries=0;
	if (!fp)	
	{
		eDebug("failed to open.");
		unlockPlaylist();
		return -1;
	}
	int ignore_next=0;
	int indescr = 0;
	char line[4096];
	eString FullFilename = "";
	line[255] = 0;
	while (1)
	{
		if (!fgets(line, sizeof(line), fp))
			break;
		if (strlen(line) && line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = 0;
		if (strlen(line) && line[strlen(line) - 1] == '\r') line[strlen(line) - 1] = 0;
		if (!strlen(line))
			continue;
		if (line[0]=='#')
		{
			indescr = 0;
			if (!strncmp(line, "#SERVICE", 8))
			{
				int offs = line[8] == ':' ? 10 : 9;
				eString Line = line + offs;
				// If this is recordings.epl we change the filepath in the read line
				std::string::size_type pos_lastslash = this->filename.rfind('/');
				if (pos_lastslash != std::string::npos)
				{
					if (this->filename.substr(pos_lastslash + 1) == "recordings.epl")
					{
						// replace path from recordings.epl with path of recordings.epl
						// this way it can also be used for shares
						std::string::size_type pos1 = Line.rfind('/');
						std::string::size_type pos2 = Line.rfind(':');
						if (pos1 != std::string::npos && pos2 != std::string::npos)
						{
							std::string::size_type num = pos1 - pos2;
							if (num > 0)
							{
								Line = Line.replace(pos2 + 1, num, filepath);
							}
						}	
						std::string::size_type index = Line.find_first_of('/');
						if (index != std::string::npos)
						{ 
							FullFilename = Line.substr(index);
						}
					}
				}
				eServicePath path(Line.c_str());
				entries++;
				list.push_back(path);
#ifndef DISABLE_FILE
				std::string::size_type location;
				location = Line.find_first_of('/'); 
				if (location != std::string::npos)
				{
					int slice = 0;
					struct stat64 s;
					int filelength = 0;

					eString FileName = Line.substr(location); 
					// Check if the filename contains '.ts'
					location = FileName.find(".ts", 0);
					time_t filetime = 0;
					if (location != std::string::npos)
					{
						while (!::stat64((FileName + (slice ? eString().sprintf(".%03d", slice) : eString(""))).c_str(), &s))
						{
							filelength += s.st_size / 1024;
							// Use the filetime of the first slice
							if (slice == 0)
							{
								filetime = s.st_mtime;
							}
							slice++;
						}
					}				
					((eServiceReferenceDVB&)list.back().service).setFileLength(filelength);
					((eServiceReferenceDVB&)list.back().service).setFileTime(filetime);
				}
#endif
				ignore_next=1;
			}
			else if (!strncmp(line, "#DESCRIPTION", 12))
			{
				int offs = line[12] == ':' ? 14 : 13;
				list.back().service.descr=line+offs;
				indescr = 1;
			}
			else if (!strcmp(line, "#CURRENT"))
			{
				current=list.end();
				current--;
			}
			else if (!strncmp(line, "#LAST_ACTIVATION ", 17))
				list.back().last_activation=atoi(line+17);
			else if (!strncmp(line, "#CURRENT_POSITION ", 18))
				list.back().current_position=atoi(line+18);
			else if (!strncmp(line, "#TYPE ", 6))
				list.back().type=atoi(line+6);
			else if (!strncmp(line, "#EVENT_ID ", 10))
				list.back().event_id=atoi(line+10);
			else if (!strncmp(line, "#TIME_BEGIN ", 12))
				list.back().time_begin=atoi(line+12);
			else if (!strncmp(line, "#DURATION ", 10))
				list.back().duration=atoi(line+10);
			else if (!strncmp(line, "#NAME ", 6))
			{
				service_name=line+6;
				service_name_set=1;
			}
			continue;
		}
		if (indescr)
		{
			list.back().service.descr+=line;
			continue;
		}

		if (!line[0])
			break;
                // if we weren't reading a recordings.epl the filename is not yet filled in, see if it is in the current line
                std::string::size_type pos = this->filename.rfind('/');
                if (pos != std::string::npos)
                {
			if (this->filename.substr(pos + 1) != "recordings.epl")
			{
				// reset a previous filled in filename
				FullFilename = "";
				if (line[0] != '/' && (!strstr(line, "://")))
				{
					FullFilename = filepath;
				}
				FullFilename += line;
			}
			// Check if file does actually exist when in recordings.epl, we do not want 0 bytes files to be shown
			if (this->filename.substr(pos + 1) == "recordings.epl")
			{
				if (access(FullFilename.c_str(), R_OK) != 0)	// File could not be accessed
				{
					list.pop_back();
				}
			}
		}

		if (ignore_next)
		{
			ignore_next=0;
			continue;
		}

		eServiceReference ref;
		if (eServiceFileHandler::getInstance()->lookupService(ref, FullFilename.c_str()))
		{
			entries++;
			list.push_back(ref);
		} else
			eDebug("can't handle service: %s", FullFilename.c_str());
	}
	if (!service_name_set)
		service_name += eString().sprintf(_(" (%d entries)"), entries);
	fclose(fp);

	if (changed != 255)
		changed=0;
	unlockPlaylist();
	return 0;
}

int ePlaylist::save(const char *filename)
{
	if (!changed)
		return 0;
	if (!filename)
		filename=this->filename.c_str();

	lockPlaylist();

	int fd = 0;

	fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC | O_SYNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

	if (fd == -1)	// File cannot be created
	{
		unlockPlaylist();
		return -1;
	}

	std::stringstream writebuffer;

	writebuffer << "#NAME " << service_name.c_str() << "\r\n";

	for (std::list<ePlaylistEntry>::iterator i(list.begin()); i != list.end(); ++i)
	{
		if ( i->services.size() > 1 )
		{
			eServicePath p = i->services;
			writebuffer << "#SERVICE: " << p.toString().c_str() << "\r\n";
		}
		else
		{
			writebuffer << "#SERVICE: " << i->service.toString().c_str() << "\r\n";
		}
		if ( i->service.descr)
		{
			writebuffer << "#DESCRIPTION: " << i->service.descr.c_str() << "\r\n";
		}
		if ( i->type & ePlaylistEntry::PlaylistEntry && i->current_position != -1 )
		{
			writebuffer << "#CURRENT_POSITION " << i->current_position << "\r\n";
		}
		else if ( i->type & ePlaylistEntry::isRepeating && i->last_activation != -1 )
		{
			writebuffer << "#LAST_ACTIVATION " << i->last_activation << "\r\n";
		}
		else if ( i->event_id != -1)
		{
			writebuffer << "#EVENT_ID " << i->event_id <<"\r\n";
		}
		if ((int)i->type != ePlaylistEntry::PlaylistEntry)
		{
			writebuffer << "#TYPE " << i->type << "\r\n";
		}
		if ((int)i->time_begin != -1)
		{
			writebuffer << "#TIME_BEGIN " << (int)i->time_begin << "\r\n";
		}
		if ((int)i->duration != -1)
		{
			writebuffer << "#DURATION " << (int)i->duration << "\r\n";
		}
		if (current == i)
		{
			writebuffer << "#CURRENT\n";
		}
		if (i->service.path.size())
		{
			writebuffer << i->service.path.c_str() << "\r\n";
		}
	}
	unsigned int written_bytes = write(fd, writebuffer.str().c_str(), writebuffer.str().length());
	fsync(fd);
	close(fd);
	changed=0;
	unlockPlaylist();
	if (written_bytes == writebuffer.str().length())
	{
		return 0;
	}
	else
	{
		eDebug("couldn't write file %s", filename);
		return -1;
	}
}

int ePlaylist::deleteService(std::list<ePlaylistEntry>::iterator it)
{
	if (it != list.end())
	{
		if ((it->type & ePlaylistEntry::boundFile) && (it->service.path.size()))
		{
			int slice=0;
			eString filename;
			while (1)
			{
				filename=it->service.path;
				if (slice)
					filename+=eString().sprintf(".%03d", slice);
				slice++;
				struct stat64 s;
				if (::stat64(filename.c_str(), &s) < 0)
					break;
				eBackgroundFileEraser::getInstance()->erase(filename.c_str());
			}
		}
		list.erase(it);
		changed=1;
		return 0;
	}
	return -1;
}

int ePlaylist::moveService(std::list<ePlaylistEntry>::iterator it, std::list<ePlaylistEntry>::iterator before)
{
	if (current == it)
		current=list.insert(before, *it);
	else
		list.insert(before, *it);
	list.erase(it);
	changed=1;
	return 0;
}

void ePlaylist::lockPlaylist()
{
	lockCount++;

	// If we allready have the lock quit immediatly
	if (lockActive)
	{
		return;
	}
	
	// Try to create a lock file, if this does not work within 5 secs we assume an error
	// of a previous enigma and lock anyway

	// Only lock on loading recordings.epl
	if (filename.substr(filename.rfind('/') + 1) != "recordings.epl")
	{
		// Since we are not keeping the lock decrement lockCount
		lockCount--;
		return;
	}

	eString lockFile = filepath + "recordings.epl.lck";
#ifdef DEBUG
	eDebug("Try to create %s", lockFile.c_str());
#endif
	bool lockfileCreated = false;
	bool timeoutOccured = false;
	bool error = false;
	struct timeval tim;
	gettimeofday(&tim, NULL);

	while (!lockfileCreated && !timeoutOccured && !error)
	{
		struct timeval tim2;
		gettimeofday(&tim2, NULL);
		
		if (tim2.tv_sec - tim.tv_sec >= 5)
		{
#ifdef DEBUG
			eDebug("Timeout on creating %s", lockFile.c_str());
#endif
			timeoutOccured = true;
			// We assume lock is ours, we waited long enough ;-)
			lockActive = true;
			// If we have waited for the lock be sure to now set the lockcount to 1
			lockCount = 1;
		}
		else
		{
			int fd = open(lockFile.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_SYNC, 0644);
			
			if (fd < 0 && errno==EEXIST)
			{
				// Somebody else has the lock
#ifdef DEBUG
				eDebug("%s busy, wait 100ms before trying again", lockFile.c_str());
#endif
				usleep(100000);
			}
			else if (fd < 0)
			{
				// An error has occured
				eDebug("%s could not be created!", lockFile.c_str());
				error = true;
			}
			else
			{
				// File is ours!
				lockActive = true;
				lockfileCreated = true;
				// Close the lockfile
				close(fd);
			}
		}
	}
}

void ePlaylist::unlockPlaylist()
{
	if (lockActive)
	{
		lockCount--;
		if (lockCount == 0)
		{
			eString lockFile = filepath + "recordings.epl.lck";
			unlink(lockFile.c_str());
			lockActive = false;
		}
	}
}

void eServicePlaylistHandler::addFile(void *node, const eString &filename)
{
	if (filename.right(4).upper()==".M3U")
		eServiceFileHandler::getInstance()->addReference(node, eServiceReference(id,
			eServiceReference::mustDescent|
			eServiceReference::canDescent|
			eServiceReference::sort1, filename));
}

eService *eServicePlaylistHandler::createService(const eServiceReference &node)
{
	ePlaylist *list=new ePlaylist();
	if (!node.path.empty())
	{
		if (!list->load(node.path.c_str()))
			return list;
		delete list;
		return 0;
	} else
		return list;
}

eServicePlaylistHandler *eServicePlaylistHandler::instance;

eServicePlaylistHandler::eServicePlaylistHandler(): eServiceHandler(ID)
{
        //eDebug("[eServicePLaylistHandler] registering serviceInterface %d", id);
	if (eServiceInterface::getInstance()->registerHandler(id, this)<0)
		eFatal("couldn't register serviceHandler %d", id);
	CONNECT(eServiceFileHandler::getInstance()->fileHandlers, eServicePlaylistHandler::addFile);
	instance=this;
}

eServicePlaylistHandler::~eServicePlaylistHandler()
{
	instance=0;
}

eService *eServicePlaylistHandler::addRef(const eServiceReference &service)
{
	return eServiceFileHandler::getInstance()->addRef(service);
}

void eServicePlaylistHandler::removeRef(const eServiceReference &service)
{
	return eServiceFileHandler::getInstance()->removeRef(service);
}

void eServicePlaylistHandler::enterDirectory(const eServiceReference &dir, Signal1<void,const eServiceReference&> &callback)
{
	if (dir.type == id)  // for playlists in other playlists..
	{
		ePlaylist *service=(ePlaylist*)addRef(dir);
		if (!service)
			return;

		for (std::list<ePlaylistEntry>::const_iterator i(service->getConstList().begin()); i != service->getConstList().end(); ++i)
			callback(*i);

		removeRef(dir);
		return;
	}
	// for playlists in any other root.. but not in another playlist..
	std::pair<std::multimap<eServiceReference,eServiceReference>::const_iterator,std::multimap<eServiceReference,eServiceReference>::const_iterator>
		range=playlists.equal_range(dir);
	while (range.first != range.second)
	{
		callback(range.first->second);
		++range.first;
	}
}

void eServicePlaylistHandler::leaveDirectory(const eServiceReference &dir)
{
	(void)dir;
}

int eServicePlaylistHandler::addNum( int uniqueID )
{
	if ( usedUniqueIDs.find( uniqueID ) != usedUniqueIDs.end() )
		return -1;
	usedUniqueIDs.insert(uniqueID);
	return 0;
}

eServiceReference eServicePlaylistHandler::newPlaylist(const eServiceReference &parent, const eServiceReference &ref)
{
	if (parent)
	{
		playlists.insert(std::pair<eServiceReference,eServiceReference>(parent, ref));
		return ref;
	}
	else
	{
		int uniqueNum=0;
		do
		{
			timeval now;
			gettimeofday(&now,0);
			uniqueNum = now.tv_usec;
			if ( uniqueNum < 21 && uniqueNum >=0 )
				continue;
		}
		while( usedUniqueIDs.find( uniqueNum ) != usedUniqueIDs.end() );
		usedUniqueIDs.insert(uniqueNum);
		return eServiceReference( ID, eServiceReference::flagDirectory, 0, uniqueNum );
	}
}

void eServicePlaylistHandler::removePlaylist(const eServiceReference &service)
{
		// och menno.
	int found=1;
	while (found)
	{
		found=0;
		for (std::multimap<eServiceReference,eServiceReference>::iterator i(playlists.begin()); i != playlists.end();)
		{
			if (i->second == service)
			{
				usedUniqueIDs.erase( service.data[1] );
				found=1;
				playlists.erase(i++);
			}
			else
				++i;
		}
	}
}

eAutoInitP0<eServicePlaylistHandler> i_eServicePlaylistHandler(eAutoInitNumbers::service+2, "eServicePlaylistHandler");
