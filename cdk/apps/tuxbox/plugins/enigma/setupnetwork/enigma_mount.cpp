/*
 * $Id: enigma_mount.cpp,v 1.58 2005/12/22 08:48:48 digi_casi Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <pthread.h>
#include <string.h>
#include <vector>
#include <lib/base/estring.h>
#include <lib/gui/enumber.h>
#include <configfile.h>
#include <enigma_dyn_utils.h>
#include <enigma_mount.h>

using namespace std;

eNetworkMountMgr *eNetworkMountMgr::instance;

static bool fileSystemIsSupported(eString fsname)
{
	eString s;
	bool found = false;
	fsname = fsname.upper();
	std::ifstream in("/proc/filesystems", std::ifstream::in);

	// Note /proc/filesystems has two columns, first column could be "nodev",
	// the second the real fs. We check both here which doesn't harm
	printf("[MOUNT] looking for %s", fsname.c_str());
	while (in >> s)
	{
		if (found = (s.upper() == fsname))
			break;
	}
			
	in.close();
	return found;
}

eMountPoint::eMountPoint()
	: id(-1), userName(""), password(""), localDir(""), mountDir(""), remoteHost(""), remoteIP(""),
	fstype(undefinedMount), automount(1), options(""), description(""), linuxExtensions(1), mounted(false)
{
}

eMountPoint::eMountPoint(const eMountPoint& mp)
	: id(mp.id), userName(mp.userName), password(mp.password), localDir(mp.localDir), mountDir(mp.mountDir), 
	remoteHost(mp.remoteHost), remoteIP(mp.remoteIP), fstype(mp.fstype), automount(mp.automount), 
	options(mp.options), description(mp.description), linuxExtensions(mp.linuxExtensions), mounted(mp.mounted)
{
	setIP();
};

eMountPoint& eMountPoint::operator = (const eMountPoint& mountinfo)
{
	id = mountinfo.id;
	userName = mountinfo.userName;
	password = mountinfo.password;
	localDir = mountinfo.localDir;
	mountDir = mountinfo.mountDir;
	remoteHost = mountinfo.remoteHost;
	remoteIP = mountinfo.remoteIP;
	fstype = mountinfo.fstype;
	automount = mountinfo.automount;
	options = mountinfo.options;
	mounted = mountinfo.mounted;
	description = mountinfo.description;
	linuxExtensions = mountinfo.linuxExtensions;
	setIP();
	return *this;
}

void eMountPoint::setIP()
{
	struct hostent *server;
	if(remoteHost.length() > 0)
	{
		server = gethostbyname(remoteHost.c_str());
		if(server != NULL)
		{
			//remoteIP = eString().sprintf("%s", inet_ntoa(*( struct in_addr*)( server->h_addr)));
			remoteIP = inet_ntoa(*( struct in_addr*)( server->h_addr));
		}
		else
		{
			remoteIP = "";
		}
		
		printf("[MOUNT] %s has ip %s", remoteHost.c_str(), remoteIP.c_str());
	}
}

void eMountPoint::save(FILE *out, int pid)
{
	id = pid;
#ifndef ENABLE_DEVMOUNTS
	if (fstype != deviceMount)  // skip device mounts.
	{
#endif
		fprintf(out,"ip_%d=%s\n", id, remoteHost.c_str());
		fprintf(out,"fstype_%d=%d\n", id, (int)fstype);
		fprintf(out,"localdir_%d=%s\n", id, localDir.c_str());
		fprintf(out,"mountdir_%d=%s\n", id, mountDir.c_str());
		fprintf(out,"username_%d=%s\n", id, userName.c_str());
		fprintf(out,"password_%d=%s\n", id, password.c_str());
		fprintf(out,"options_%d=%s\n", id, options.c_str());
		fprintf(out,"description_%d=%s\n", id, description.c_str());
		fprintf(out,"automount_%d=%d\n", id, automount);
		fprintf(out,"linuxextensions_%d=%d\n\n", id, linuxExtensions);
#ifndef ENABLE_DEVMOUNTS
	}
#endif
}

bool eMountPoint::isReallyMounted()
{
	std::ifstream in;
	eString mountDev;
	eString mountOn;
	eString mountType;
	eString buffer;
	std::stringstream tmp;

	bool found = false;
	in.open("/proc/mounts", std::ifstream::in);
	while (getline(in, buffer, '\n'))
	{
		mountDev = mountOn = mountType = "";
		tmp.str(buffer);
		tmp >> mountDev >> mountOn >> mountType;
		tmp.clear();
		if (found = isIdentical(mountOn, mountDev))
		{
			break;
		}
	}
	in.close();
	return found;
}

// To set and read the "mounted" boolean.
void eMountPoint::setMounted(bool ismounted)
{
	mounted = ismounted;
}

bool eMountPoint::isMounted()
{
	return mounted;
}

bool eMountPoint::isIdentical(eString mountOn, eString mountDev)
{
	bool found = false;
	
	if ((mountOn == localDir) || (mountOn.strReplace("/media/", "/mnt/") == localDir))
	{
		switch (fstype)
		{
			case nfsMount:
				found =	(eString().sprintf("%s:%s", remoteHost.c_str(), mountDir.c_str()) == mountDev);
				if (!found)
					found =	(eString().sprintf("%s:/%s", remoteHost.c_str(), mountDir.c_str()) == mountDev);
				break;
			case cifsMount:
				found =	(eString().sprintf("//%s/%s", remoteHost.c_str(), mountDir.c_str()) == mountDev);
				if (!found)
					found =	(eString().sprintf("//%s/%s", remoteIP.c_str(), mountDir.c_str()) == mountDev);
				break;
			case smbMount:
				found =	(eString().sprintf("//%s/%s", remoteHost.upper().c_str(), mountDir.upper().c_str()) == mountDev.upper());
				if (!found)
					found =	(eString().sprintf("//%s/%s", remoteIP.c_str(), mountDir.upper().c_str()) == mountDev.upper());
				break;
	#ifdef ENABLE_DEVMOUNTS
			case deviceMount:
				found = ((mountOn == localDir) && (mountDev == mountDir) && remoteHost == (eString) "");
				break;
	#endif
			default:
				break;
		}
	}

	return found;
}

eString eMountPoint::mount()
{
	eString cmd;
	eString ip;
	eString rc;
	
	if (!mounted)
	{
		if (access(localDir.c_str(), R_OK) == -1)
			system(eString("mkdir " + localDir).c_str());
		if (access(localDir.c_str(), R_OK) == 0)
		{
			ip = remoteHost;
			switch (fstype)
			{
				case nfsMount:
					if (fileSystemIsSupported("nfs"))
					{
						cmd = "mount -t nfs ";
						cmd += ip + ":" + mountDir + " " + localDir;
						cmd += (options) ? (" -o " + options) : "";
					}
					else
					{
						rc = ("NFS file system not supported");
					}
					break;
					
				case cifsMount:
					if (fileSystemIsSupported("cifs"))
					{
						cmd = "mount -t cifs //";
						cmd += ip + "/" + mountDir + " " + localDir + " -o user=";
						cmd += (userName) ? userName : "anonymous";
						cmd += (password) ? (",pass=" + password) : "";
						cmd += ",unc=//" + ip + "/" + mountDir;
						cmd += (options) ? ("," + options) : "";
					}
					else
					{
						rc = ("CIFS file system not supported");
					}
					break;
					
				case smbMount:
					if (fileSystemIsSupported("smbfs"))
					{
						cmd = "smbmount ";
						cmd += "//" + ip + "/" + mountDir;
						cmd += " " + ((password) ? password : "guest");
						cmd += " -U " + ((userName) ? userName : "guest");
						cmd += " -c \"mount " + localDir + "\"";
					}
					else
					{
						rc = ("SMBFS file system not supported");
					}
					break;
					
#ifdef ENABLE_DEVMOUNTS
				case deviceMount:
					cmd = "mount " + mountDir + " " + localDir;
					break;
#endif
				default:
					break;
			}

			if (rc.length() == 0)
			{
#if FORK
				switch (fork())
				{
					case -1:
						printf("[MOUNT] fork failed!");
						rc = "Fork failed";
						break;
					case 0:
					{
						for (unsigned int i = 0; i < 90; ++i )
							close(i);

#endif
						if(fstype == cifsMount)
						{
							// Automatic linuxExtensions detection/correction for cifs
							// 1. Try to mount as specified by the user
							// 2. If mount is ok, go to step 6
							// 3. Invert the setting and try to mount again.
							// 4. If mount is ok, store inverted setting and go to step 6
							// 5. Report that mount failed
							// 6. Report that mount succeeded
							
							// Try to mount as specified by the user
							rc = tryCifsMount(cmd, linuxExtensions);
							
							if(!isReallyMounted())
							{
								// Try to mount with the inverse setting
								rc = tryCifsMount(cmd, !linuxExtensions);
								
								if(isReallyMounted())
								{
									// The inverse linuxExtensions worked, use this one.
									linuxExtensions = linuxExtensions ? 0 : 1;
								}
							}
						}
						else
						{
							// Execute the mount command
							printf("[MOUNT] mounting: %s", cmd.c_str());
							if(system(cmd.c_str()) != 0)
							{
								rc = ("Mount failed");
							}
						}						

#if FORK
						_exit(0);
						break;
					}
				}
#endif
				mounted = isReallyMounted();
				//mounted = (rc == 0);
			}
		}
		else
		{
			rc = ("Could not create local directory");
		}
	}
	else
	{
		printf("[MOUNT] %s already mounted!", localDir.c_str());
		rc = ("Mountpoint already mounted");
	}

	return rc;
}

eString eMountPoint::tryCifsMount(const eString& cmd, int linuxExt)
{
	eString rc;
	
	if(linuxExt == 0)
	{
		// If we don't want linux extensions on cifs, disable it first
		printf("[MOUNT] disabling linux extensions");
		eString procCommand = "echo 0 > /proc/fs/cifs/LinuxExtensionsEnabled";
		system(procCommand.c_str());
	}
						
	// Execute the mount command
	printf("[MOUNT] mounting: %s", cmd.c_str());
	if(system(cmd.c_str()) != 0)
	{
		rc = ("Mount failed");
	}

	if(linuxExt == 0)
	{
		// Enable linux extensions afterwards
		printf("[MOUNT] enabling linux extensions");
		eString procCommand = "echo 1 > /proc/fs/cifs/LinuxExtensionsEnabled";
		system(procCommand.c_str());
	}
	
	return rc;
}

eNetworkMountMgr::eNetworkMountMgr()
{
	if (!instance)
		instance = this;

	init();
}

eNetworkMountMgr::~eNetworkMountMgr()
{
	instance = NULL;
	mountPoints.clear();
}

bool eNetworkMountMgr::isReallyMounted(int pid)
{
	bool tmp = false;
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->id == pid)
		{
			tmp = mp_it->isReallyMounted();
			break;
		}
	}
	printf("[MOUNT] entry %d is really %smounted", pid, tmp ? "" : "not ");
	return tmp;
}

void eNetworkMountMgr::automountMountPoints(void)
{
	printf("[MOUNT] automountMountPoints...");
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->automount == 1)
		{
			printf("[MOUNT] automounting %s", mp_it->mountDir.c_str());
			mp_it->mount();
		}
	}
}

void eNetworkMountMgr::init()
{
	printf("[MOUNT] init");
	eMountPoint mp;
	mountPoints.clear();
	CConfigFile *config = new CConfigFile(',');
	if (config->loadConfig(MOUNTCONFIGFILE))
	{
		unsigned int i = 0;
		eString extra_options;
		
		while (config->getString(eString().sprintf("localdir_%d", i)) != "")
		{
			mp.localDir = config->getString(eString().sprintf("localdir_%d", i));
			mp.localDir = fixNonRecommendedMountpoint(mp.localDir);
			mp.fstype = (enum eMountPoint::mountType)config->getInt32(eString().sprintf("fstype_%d", i));
			mp.password = config->getString(eString().sprintf("password_%d", i));
			mp.userName = config->getString(eString().sprintf("username_%d", i));
			mp.mountDir = config->getString(eString().sprintf("mountdir_%d", i));
			mp.automount = config->getInt32(eString().sprintf("automount_%d", i));
			mp.options = config->getString(eString().sprintf("options_%d", i));
			mp.linuxExtensions = config->getInt32(eString().sprintf("linuxextensions_%d", i), 1);
			
			// Compatibility for extra_options in mount.conf
			extra_options = config->getString(eString().sprintf("extra_options_%d", i));
			if(extra_options.length() > 0)
			{
				mp.options = mp.options + "," + extra_options;
			}
			
			mp.description = config->getString(eString().sprintf("description_%d", i));
			mp.remoteHost = config->getString(eString().sprintf("ip_%d", i));
			mp.id = i;
			printf("[MOUNT] adding %d - %s", i, mp.localDir.c_str());
			eMountPoint m = eMountPoint(mp);
			printf("[MOUNT] checkmounted");
			m.setMounted(m.isReallyMounted());
#ifndef ENABLE_DEVMOUNTS
			// Keep this if. Although it's "impossible" to read dev mounts from the
			// config file, somebody could have an old file and we don't want to add them.
			if (mp.fstype != eMountPoint::deviceMount)
#endif
				mountPoints.push_back(m);
			i++;
		}
	}
	delete config;

	//addMountedFileSystems();
	save();
}

void eNetworkMountMgr::save()
{
	FILE *out = fopen(MOUNTCONFIGFILE, "w");
	if (out)
	{
		int i = 0;
		for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
		{
			if ( (mp_it->fstype != eMountPoint::undefinedMount)
#ifndef ENABLE_DEVMOUNTS
				// Keep this line. We never want to save device mounts
				&& (mp_it->fstype != eMountPoint::deviceMount)
#endif
				)
			{
				printf("[MOUNT] saved pid %d entry %d ", i, mp_it->id);
				mp_it->save(out, i);
				i++;
			}
		}
		fclose(out);
	}
}

// Converts non recommended mountpoints like /mnt/server to /media/server and so on
eString eNetworkMountMgr::fixNonRecommendedMountpoint(const eString& _mountpoint)
{
	eString fixedMountpoint;
	eString mountpoint = _mountpoint;

	// Add leading slash
	if(mountpoint.left(1) != "/")
	{
		// xxx -> /xxx
		mountpoint = "/" + mountpoint;
	}
	
	// Remove trailing slash
	if(mountpoint.right(1) == "/")
	{
		// /xxx/ -> /xxx
		mountpoint = mountpoint.left(mountpoint.length() - 1);
	}
	
	if(mountpoint.left(9) == "/var/mnt/")
	{
		// /var/mnt/xxx -> /media/xxx
		fixedMountpoint = "/media/" + mountpoint.mid(9);
	}
	else if(mountpoint == "/hdd")
	{
		// /hdd -> /media/hdd
		fixedMountpoint = "/media/hdd";
	}
	else if((mountpoint == "/mnt") || (mountpoint == "/media") || (mountpoint == "/var/mnt"))
	{
		// /mnt, /media, /var/mnt -> /media/server1
		// Yes, I've seen people mounting on /mnt or /media
		fixedMountpoint = "/media/server1";
	}
	else if(mountpoint.left(5) == "/hdd/")
	{
		// /hdd/xxx -> /media/hdd/xxx
		fixedMountpoint = "/media/hdd/" + mountpoint.mid(5);
	}
	else if(mountpoint.left(5) == "/mnt/")
	{
		// /mnt/xxx -> /media/xxx
		fixedMountpoint = "/media/" + mountpoint.mid(5);
	}
	else
	{
		// All other mountpoints are right
		fixedMountpoint = mountpoint;
	}
	
	printf("[MOUNT] original mountpoint '%s' -> recommended '%s'", _mountpoint.c_str(), fixedMountpoint.c_str());
	return fixedMountpoint;
}

#if 0
bool eNetworkMountMgr::slowAutomounts()
{
	bool slowmount = false;	
	for (unsigned int i = 0; i < mountPoints.size(); i++)
	{
		if (mountPoints[i].mp.fstype == 1 || mountPoints[i].mp.fstype == 2) // CIFS & SMBFS are slow mounters!!
		{
			slowmount = true;
			// we found a slow mounter, makes no sense to go on
			break;
		}
	}
	return slowmount;
}
#endif
