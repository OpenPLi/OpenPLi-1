/*
 * $Id: enigma_mount.cpp,v 1.64 2007/02/18 18:00:57 digi_casi Exp $
 *
 * (C) 2005, 2007 by digi_casi <digi_casi@tuxbox.org>
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

#ifdef ENABLE_EXPERT_WEBIF
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <dirent.h>
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
#include <enigma_main.h>
#include <sselect.h>
#include <lib/system/info.h>
#include <media_mapping.h>

using namespace std;

eNetworkMountMgr *eNetworkMountMgr::instance;
eDevMountMgr *eDevMountMgr::instance;

static bool fileSystemIsSupported(eString fsname)
{
	eString s;
	bool found = false;
	fsname = fsname.upper();
	std::ifstream in("/proc/filesystems", std::ifstream::in);

	// Note /proc/filesystems has two columns, first column could be "nodev",
	// the second the real fs. We check both here which doesn't harm
	eDebug("[MOUNT] looking for %s", fsname.c_str());
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
		
		eDebug("[MOUNT] %s has ip %s", remoteHost.c_str(), remoteIP.c_str());
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
						rc = _("NFS file system not supported");
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
						rc = _("CIFS file system not supported");
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
						rc = _("SMBFS file system not supported");
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
						eDebug("[MOUNT] fork failed!");
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
							eDebug("[MOUNT] mounting: %s", cmd.c_str());
							if(system(cmd.c_str()) != 0)
							{
								rc = _("Mount failed");
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
			rc = _("Could not create local directory");
		}
	}
	else
	{
		eDebug("[MOUNT] %s already mounted!", localDir.c_str());
		rc = _("Mountpoint already mounted");
	}

	return rc;
}

eString eMountPoint::tryCifsMount(const eString& cmd, int linuxExt)
{
	eString rc;
	
	if(linuxExt == 0)
	{
		// If we don't want linux extensions on cifs, disable it first
		eDebug("[MOUNT] disabling linux extensions");
		eString procCommand = "echo 0 > /proc/fs/cifs/LinuxExtensionsEnabled";
		system(procCommand.c_str());
	}
						
	// Execute the mount command
	eDebug("[MOUNT] mounting: %s", cmd.c_str());
	if(system(cmd.c_str()) != 0)
	{
		rc = _("Mount failed");
	}

	if(linuxExt == 0)
	{
		// Enable linux extensions afterwards
		eDebug("[MOUNT] enabling linux extensions");
		eString procCommand = "echo 1 > /proc/fs/cifs/LinuxExtensionsEnabled";
		system(procCommand.c_str());
	}
	
	return rc;
}

eString eMountPoint::unmount()
{
	eString rcString;
	int rc = umount2(localDir.c_str(), MNT_FORCE);
	if (rc != 0)
	{
		eDebug("[MOUNT] failed to unmount %s: %s", mountDir.c_str(), strerror(errno));
		rcString = (eString)strerror(errno);
	}
	else
	{
		mounted = isReallyMounted();
		eDebug("[MOUNT] unmounted %s", mountDir.c_str());
	}

#if 0	
	if(!mounted)
	{
		// Let MediaMapping know a directory has been unmounted
		// and a default storage location may be needed
		eMediaMapping::getInstance()->setAllDefaultForMount(localDir);
	}
#endif
	
	return rcString;
}

eMountPoint eMountMgr::getMountPointData(int pid)
{
	eMountPoint tmp;
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->id == pid)
		{
			tmp = *mp_it;
			break;
		}
	}
	return tmp;
}

eString eMountPoint::getLongDescription()
{
	eString mountDesc;

	if(fstype == deviceMount)
	{
		mountDesc = description;
	}
	else // Network mount
	{
		if(description.length() > 0)
		{
			mountDesc = description;
		}
		else
		{
			mountDesc = mountDir + " on ";
					
			if(remoteIP.length() > 0)
			{
				mountDesc += remoteIP;
			}
			else
			{
				mountDesc += remoteHost;
			}
		}
	}
	
	return mountDesc;
}

int eMountMgr::searchMountPoint(eString localDir)
{
	int mountId = -1; // -1 is not found
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->localDir == localDir)
		{
			mountId = mp_it->id;
			break;
		}
	}
	return mountId;
}

eString eMountMgr::listMovieSources()
{
	eString result;
	if (mountPoints.size() > 0)
	{
		// TODO: This should become general code together with setup playing and recording
		eString mountPoint = "";
		
		eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, mountPoint);
		for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
		{
			if(mp_it->isMounted())
			{
				eString tmp = "<option #SEL# value=\"" + mp_it->localDir + "\">" + mp_it->getLongDescription() + "</option>";
				if (mountPoint == mp_it->localDir + "/movie")
				{
					tmp.strReplace("#SEL#", "selected");
				}
				else
				{
					tmp.strReplace("#SEL#", "");
				}

				result += tmp + "\n";
			}
		}
	}

	return result;
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
	eDebug("[MOUNT] entry %d is really %smounted", pid, tmp ? "" : "not ");
	return tmp;
}

bool eNetworkMountMgr::isMounted(int pid)
{
	bool tmp = false;
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->id == pid)
		{
			tmp = mp_it->isMounted();
			break;
		}
	}
	eDebug("[MOUNT] entry %d is %smounted", pid, tmp ? "" : "not ");
	return tmp;
}

eMountPoint eNetworkMountMgr::fixServerDirectory(eMountPoint pmp)
{
	eMountPoint fixed = pmp;
	
	switch(pmp.fstype)
	{
		case eMountPoint::nfsMount:
			if(pmp.mountDir.left(1) != "/")
			{
				fixed.mountDir = "/" + pmp.mountDir;
			}
			break;
			
		case eMountPoint::cifsMount:
			if(pmp.mountDir.left(1) == "/")
			{
				fixed.mountDir = pmp.mountDir.mid(1);
			}
			break;
			
		default:
			break;
	}
	
	return fixed;
}

int eNetworkMountMgr::addMountPoint(eMountPoint pmp)
{
	int id = -1;
	
	// Fix server directory and convert mountpoint to recommended mountpoint
	pmp = fixServerDirectory(pmp);
	pmp.localDir = fixNonRecommendedMountpoint(pmp.localDir);
	
	// First check if localDir mount point is already in use
	if(searchMountPoint(pmp.localDir) == -1)
	{
		if(!mountPoints.empty())
		{
			// Create a new mount id, one higher than the last used
			pmp.id = mountPoints.back().id + 1;
			id = pmp.id;
		}
		else
		{
			pmp.id = 0;
			id = pmp.id;
		}
	
		mountPoints.push_back(pmp);
		save();
	}
	
	return id;
}

void eNetworkMountMgr::changeMountPoint(int pid, eMountPoint pmp)
{
	// Fix server directory and convert mountpoint to recommended mountpoint
	pmp = fixServerDirectory(pmp);
	pmp.localDir = fixNonRecommendedMountpoint(pmp.localDir);
	
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		eDebug("[MOUNT] pid=%d id=%d fstype=%d", pid, mp_it->id, (int)mp_it->fstype);
		if (mp_it->id == pid)
		{
			// If mountpoint is mounted, unmount it first
			if(mp_it->isMounted())
			{
				mp_it->unmount();
			}

			*mp_it = pmp;
			mp_it->setMounted(false);
			mp_it->id = pid;
			
			break;
		}
	}

	save();
}

void eNetworkMountMgr::removeMountPoint(int id)
{
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->id == id)
		{
			// If mountpoint is mounted, unmount it first
			if(mp_it->isMounted())
			{
				mp_it->unmount();
			}
			
			mountPoints.erase(mp_it);
			break;
		}
	}
	save();
}

eString eNetworkMountMgr::mountMountPoint(int id)
{
	eString rc = _("Mountpoint not found");
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->id == id)
		{
			rc = mp_it->mount();
			break;
		}
	}
	return rc;
}

eString eNetworkMountMgr::mountMountPoint(eString localDir)
{
	eString rc = _("Mountpoint not found");
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->localDir == localDir)
		{
			if (!mp_it->isMounted())
			{
				rc = mp_it->mount();
			}
			break;
		}
	}
	return rc;
}

bool eNetworkMountMgr::isMountPointMounted(eString localDir)
{
	bool rc = false;
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->localDir == localDir)
		{
			if (mp_it->isMounted())
				rc = true;
			break;
		}
	}
	return rc;
}

eString eNetworkMountMgr::unmountMountPoint(int id)
{
	eString rc = _("Mountpoint not found");
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->id == id)
		{
			rc = mp_it->unmount();
			break;
		}
	}
	return rc;
}

void eNetworkMountMgr::automountMountPoints()
{
	eDebug("[MOUNT] automountMountPoints...");
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		if (mp_it->automount == 1)
		{
			eDebug("[MOUNT] automounting %s", mp_it->mountDir.c_str());
			mp_it->mount();
		}
	}
}

void eNetworkMountMgr::unmountAllMountPoints()
{
	for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
	{
		// Don't unmount /media/var. /media/var is only mounted after a reboot
		if (mp_it->isMounted() && (mp_it->localDir != "/media/var"))
		{
			mp_it->unmount();
		}
	}
}

eString eNetworkMountMgr::listMountPoints(eString skelleton)
{
	eString result, mountStatus, action;
	if (mountPoints.size() > 0)
	{
		for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
		{
			if (mp_it->fstype != eMountPoint::undefinedMount)
			{
				eString tmp = skelleton;
				if (mp_it->isMounted())
				{
					mountStatus = "<img src=\"on.gif\" alt=\"online\" border=0>";
					action = button(75, "Unmount", RED, "javascript:unmountMountPoint('" + eString().sprintf("%d", mp_it->id) + "')", "#FFFFFF");
				}
				else
				{
					mountStatus = "<img src=\"off.gif\" alt=\"offline\" border=0>";
					action = button(75, "Mount", BLUE, "javascript:mountMountPoint('" + eString().sprintf("%d", mp_it->id) + "')", "#FFFFFF");
				}
				
				tmp.strReplace("#ACTIONBUTTON#", action);
				tmp.strReplace("#CHANGEID#", eString().sprintf("%d", mp_it->id));
				tmp.strReplace("#DELETEID#", eString().sprintf("%d", mp_it->id));
				tmp.strReplace("#MOUNTED#", mountStatus);
				tmp.strReplace("#LDIR#", mp_it->localDir);
				tmp.strReplace("#MDIR#", mp_it->mountDir);
				tmp.strReplace("#IP#", mp_it->remoteHost);
				tmp.strReplace("#USER#", mp_it->userName);
				tmp.strReplace("#PW#", mp_it->password);
				
				eString type = "DEV";
				if (mp_it->fstype == eMountPoint::undefinedMount)
					type = "UNUSED";
				else if (mp_it->fstype == eMountPoint::nfsMount)
					type = "NFS";
				else if (mp_it->fstype == eMountPoint::cifsMount)
					type = "CIFS";
				else if (mp_it->fstype == eMountPoint::smbMount)
					type = "SMBFS";
					
				tmp.strReplace("#FSTYPE#", type);
				tmp.strReplace("#AUTO#", eString().sprintf("%d", mp_it->automount));
				tmp.strReplace("#OPTIONS#", mp_it->options);
				tmp.strReplace("#DESCRIPTION#", mp_it->description);
				tmp.strReplace("#LINUXEXTENSIONS#", eString().sprintf("%d", mp_it->linuxExtensions));
				result += tmp + "\n";
			}
		}
	}
	else
	{
		result = "<tr><td>No mount points available.</td></tr>";
	}

	return result;
}

#if 0
void eNetworkMountMgr::addMountedFileSystems()
{
	std::ifstream in;
	eString mountDev;
	eString mountOn;
	eString mountType;
	eString buffer;
	std::stringstream tmp;
	bool found = false;
	eMountPoint mp;

	eDebug("[MOUNT] Collect mounted filesystems");
	in.open("/proc/mounts", std::ifstream::in);
	while (getline(in, buffer, '\n'))
	{
		mountDev = mountOn = mountType = "";
		tmp.str(buffer);
		tmp >> mountDev >> mountOn >> mountType;
		tmp.clear();
		
		for (mp_it = mountPoints.begin(); mp_it != mountPoints.end(); mp_it++)
		{
			if (found = mp_it->isIdentical(mountOn, mountDev))
				break;	
		}
		
		if (!found)
		{
			mp.setMounted(true); // TODO: why do we set it to mounted here?
			mp.localDir = mountOn;

			//add the mount point
			if (mountType.upper() == "NFS")
			{
				if (mountDev.find("/dev") != eString::npos)
				{
					eDebug("[MOUNT] Add NFS %s", mountDev.c_str());
					mp.remoteHost = getLeft(mountDev, ':');
					mp.setIP();
					mp.mountDir = getRight(mountDev, ':');
					mp.fstype = eMountPoint::nfsMount;
					addMountPoint(mp);
				}
			}
			else if (mountType.upper() == "CIFS")
			{
				eDebug("[MOUNT] Add CIFS %s", mountDev.c_str());
				mountDev = mountDev.right(mountDev.length() - 2); //strip off leading slashes
				mp.remoteHost = getLeft(mountDev, '/');
				mp.setIP();
				mp.mountDir = getRight(mountDev, '/');
				mp.fstype = eMountPoint::cifsMount;
				addMountPoint(mp);
			}
			else if (mountType.upper() == "SMBFS")
			{
				eDebug("[MOUNT] Add SMBFS %s", mountDev.c_str());
				mountDev = mountDev.right(mountDev.length() - 2); //strip off leading slashes
				mp.remoteHost = getLeft(mountDev, '/');
				mp.setIP();
				mp.mountDir = getRight(mountDev, '/');
				mp.fstype = eMountPoint::smbMount;
				addMountPoint(mp);
			}
#ifdef ENABLE_DEVMOUNTS
			else if (!((mountOn == "/") ||(mountOn == "/dev") || (mountOn == "/tmp") || (mountOn == "/proc") || (mountOn == "/dev/pts") ||
			     (mountDev.find("/dev/mtdblock") != eString::npos) || (mountDev == "usbfs") || (mountOn == "")))
			{
				//other file system
				mp.remoteHost = "";
				mp.mountDir = mountDev;
				mp.fstype = eMountPoint::deviceMount;
				addMountPoint(mp);
			}
#endif
			else {
				eDebug("[MOUNT] Not a network mount, skipping: %s %s %s", mountType.c_str(), mountDev.c_str(), mountOn.c_str());
			}
		}
	}
	in.close();
}
#endif

void eNetworkMountMgr::init()
{
	eDebug("[MOUNT] init");
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
			eDebug("[MOUNT] adding %d - %s", i, mp.localDir.c_str());
			eMountPoint m = eMountPoint(mp);
			eDebug("[MOUNT] checkmounted");
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
				eDebug("[MOUNT] saved pid %d entry %d ", i, mp_it->id);
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
	
	eDebug("[MOUNT] original mountpoint '%s' -> recommended '%s'", _mountpoint.c_str(), fixedMountpoint.c_str());
	return fixedMountpoint;
}

eDevMountMgr::eDevMountMgr()
{
	mountPoints.clear();
}

eDevMountMgr::~eDevMountMgr()
{
	instance = NULL;
	mountPoints.clear();
}

int eDevMountMgr::addMountPoint(eMountPoint pmp)
{
	pmp.id = mountPoints.size();
	mountPoints.push_back(pmp);
	return pmp.id;
}

eMountSelectionComboBox::eMountSelectionComboBox(
	eWidget* parent, 
	int openEntries,
	eLabel* desc, 
	int showFlags,
	const eString& noSelectionText,
	const eString& customLocationText)
	: eComboBox(parent, openEntries, desc),
	currentLocation("/"), customLocationIndex(0), customLocationCreated(false)
{
	// Start at key 1, key 0 is free for use "No storage device found" for example
	int key = 1;
	eDevMountMgr* devMountMgr = eDevMountMgr::getInstance();
	eNetworkMountMgr* networkMountMgr = eNetworkMountMgr::getInstance();

	/*	
	if(showFlags & ShowHarddisk)
	{	
		int HDDAvailable(0);
		eConfig::getInstance()->getKey("/pli/HDDAvailable", HDDAvailable);
		
		if(HDDAvailable)
		{
			eMountPoint mnt;
			mnt.localDir = "/media/hdd";
			mnt.description = _("Internal harddisk");

			new eListBoxEntryText(
				*this,
				_(mnt.description.c_str()),
				(void *)key);
			
			mountInfoList.push_back(mnt);
			key++;
		}
	}
	*/

	if(showFlags & ShowDevices)
	{
		int nrDevMounts = devMountMgr->mountPointCount();

		for(int i = 0; i < nrDevMounts; ++i)
		{
			eMountPoint mnt = devMountMgr->getMountPointData(i);
			
			new eListBoxEntryText(
				*this, 
				mnt.getLongDescription(), 
				(void *)key,
				0,
				mnt.localDir);
			
			mountInfoList.push_back(mnt);
			key++;
		}
	}

	if(showFlags & ShowNetwork)
	{
		int nrNetworkMounts = networkMountMgr->mountPointCount();
	
		for(int i = 0; i < nrNetworkMounts; ++i)
		{
			eMountPoint mnt = networkMountMgr->getMountPointData(i);
			
			if(networkMountMgr->isMounted(i))
			{
				new eListBoxEntryText(
					*this, 
					mnt.getLongDescription(), 
					(void *)key,
					0,
					mnt.localDir);
			
				mountInfoList.push_back(mnt);
				key++;
			}
		}
	}
	
	if(showFlags & ShowMountpoints)
	{
		DIR* d = opendir("/media");
		if(d)
		{
			while(struct dirent* e = readdir(d))
			{
				eString filename = eString(e->d_name);
			
				if((filename != ".") && (filename != ".."))
				{
					struct stat s;
					eString fullFilename = "/media/" + filename;
				
					if(lstat(fullFilename.c_str(), &s) < 0)
					{
						continue;
					}
				
					if(S_ISDIR(s.st_mode) || S_ISLNK(s.st_mode))
					{
						eMountPoint mnt;
						mnt.localDir = fullFilename;
			
						new eListBoxEntryText(
							*this,
							mnt.localDir,
							(void *)key);

						mountInfoList.push_back(mnt);
						key++;
					}
				}
			}
		
			closedir(d);
		}		
	}
	
	if(showFlags & ShowCustomLocation)
	{
		eMountPoint mnt;
		mnt.localDir = "";

		new eListBoxEntryText(
			*this,
			 customLocationText ? customLocationText : _("Browse..."), 
			 (void *)key);

		mountInfoList.push_back(mnt);
		customLocationIndex = key - 1;
	}

	if(getCount() == 0)
	{
		// No device found
		new eListBoxEntryText(
			*this,
			noSelectionText ? noSelectionText : _("No device found"),
			(void *)0);
	}
	
	loadDeco();
	((eWindow*)this)->setProperty("showEntryHelp", "");
}

eMountSelectionComboBox::~eMountSelectionComboBox()
{
}

void eMountSelectionComboBox::setCurrentLocation(const eString& directory)
{
	int comboboxIndex = -1;
	
	eDebug("eMountSelectionComboBox::setCurrentLocation directory=%s", directory.c_str());
	for(unsigned int i=0; i<mountInfoList.size(); ++i)
	{
		if(directory == mountInfoList[i].localDir)
		{
			comboboxIndex = i;
			break;
		}
	}
	
	if(comboboxIndex == -1)
	{
		// Directory not found, add a new location
		addCustomDirectory(directory);
		comboboxIndex = customLocationIndex + 1;
	}
	
	currentLocation = directory;
	setCurrent(comboboxIndex);
}

void eMountSelectionComboBox::locationChanged(eListBoxEntryText *sel)
{
	eDebug("eMountSelectionComboBox::locationChanged");
	if(sel)
	{
		int locationSelection = (int)sel->getKey() - 1;
		if(locationSelection >= 0)
		{
			eMountPoint mountInfo = mountInfoList[locationSelection];
		
			if(mountInfo.localDir == "")
			{
				// Browse selected
				eFileSelector sel("/");
				#ifndef DISABLE_LCD
				//sel.setLCD(LCDTitle, LCDElement);
				#endif
				const eServiceReference *ref = sel.choose(-1);
					
				if(ref)
				{
					currentLocation = sel.getPath().current().path;
					
					// Strip last slash
					if(currentLocation.length() > 1)
					{
						currentLocation = currentLocation.left(currentLocation.length() - 1);
					}

					addCustomDirectory(currentLocation);
					setCurrent(customLocationIndex + 1);
				}
			}
			else
			{
				// All other selections
				currentLocation = mountInfo.localDir;
			}
		}
		eDebug("eMountSelectionComboBox::locationChanged currentLocation = '%s'", currentLocation.c_str());
	}
}

eString eMountSelectionComboBox::getCurrentLocation()
{
	return currentLocation;
}

void eMountSelectionComboBox::addCustomDirectory(const eString& directory)
{
	eDebug("eMountSelectionComboBox::addCustomDirectory directory=%s", directory.c_str());
	if(customLocationCreated)
	{
		removeEntry((void *)(customLocationIndex + 2));
		mountInfoList.pop_back();
	}
	
	new eListBoxEntryText(
		*this,
		 directory, 
		 (void *)(customLocationIndex + 2));		

	eMountPoint mnt;
	mnt.localDir = directory;
	mountInfoList.push_back(mnt);
	customLocationCreated = true;
}

#endif
