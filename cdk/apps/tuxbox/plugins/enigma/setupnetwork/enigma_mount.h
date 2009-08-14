/*
 * $Id: enigma_mount.h,v 1.24 2005/10/12 20:46:27 digi_casi Exp $
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

#ifndef __enigma_mount_h__
#define __enigma_mount_h__

#include <string.h>
#include <lib/gui/listbox.h>

#define	MOUNTCONFIGFILE	"/var/tuxbox/config/enigma/mount.conf"

class eMountPoint
{
	public:
		enum mountType
		{
			undefinedMount = -1,
			nfsMount = 0,
			cifsMount = 1,
			smbMount = 2,
			deviceMount = 3
		};
	
		eMountPoint();
		eMountPoint(const eMountPoint& pmp);
		eMountPoint& operator = (const eMountPoint& mountinfo);
	
		eString mount();
		bool isMounted();
		bool isReallyMounted();
		void setMounted(bool ismounted);
		void setIP();
		void save(FILE *, int);
		bool isIdentical(eString, eString);

		int id;			// sequential number
		eString	userName;	// username, only for CIFS
		eString	password;	// password, only for CIFS
		eString	localDir;	// local mount dir
		eString	mountDir;	// directory which should be mounted
		eString	remoteHost;	// hostname for mount server
		eString	remoteIP;	// ip address for mount server
		enum mountType fstype;
		int automount;		// mount at startup
		eString options;	// rw, intr, soft, tcp, nolock
		eString description;    // description
		int linuxExtensions; // for /proc/fs/cifs/LinuxExtensionsEnabled
		
	private:
		// The mounted boolean is unreliable, use isMounted() instead
		bool mounted;		// if already mounted or not
		
		eString tryCifsMount(const eString& cmd, int linuxExt);
};

class eMountMgr
{
	protected:
		std::vector <eMountPoint> mountPoints;
		std::vector <eMountPoint>::iterator mp_it;
};

class eNetworkMountMgr : public eMountMgr
{
public:
	void automountMountPoints();
	void unmountAllMountPoints();
	bool isMounted(int);
	bool isReallyMounted(int);
	eString fixNonRecommendedMountpoint(const eString& _mountpoint);
	void save();

	static eNetworkMountMgr *getInstance() {return (instance) ? instance : (instance = new eNetworkMountMgr());}

	~eNetworkMountMgr();

private:
	eNetworkMountMgr();
	void init();

	static eNetworkMountMgr *instance;
	//void addMountedFileSystems();
};

#endif
