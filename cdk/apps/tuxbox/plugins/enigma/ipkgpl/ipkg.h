/*
	Ipkgpl - Ipkg Enigma Plugin

	Copyright (C) 2005 'mechatron' (mechatron@gmx.net)

	Homepage: http://mechatron.6x.to/

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#ifndef __IPKG_H_
#define __IPKG_H_

#include <dirent.h>
#include <vector>
#include <lib/system/econfig.h>

#define REL "v0.2.1"

#define FORCE_DEPENDS		0x0001
#define FORCE_REMOVE		0x0002
#define FORCE_REINSTALL		0x0004
#define FORCE_OVERWRITE 	0x0008

#define IPKG_CONF		"/etc/ipkg.conf"      // Fully-qualified name of Ipkg primary configuration file
#define IPKG_CONF_DIR		"/etc/ipkg"          // Directory of secondary Ipkg configuration files
#define IPKG_PKG_PATH		"/usr/lib/ipkg/lists" // Directory containing server package lists
#define IPKG_STATUS_PATH	"usr/lib/ipkg/status" // Destination status file location
#define IPKG_INFO_PATH		"usr/lib/ipkg/info"  // Package file lists location
#define IPKG_TEMP_PATH		"/tmp/"
#define IPKG_LOG_FILE		IPKG_TEMP_PATH "ipkg.log"

#include "ipkgdownload.h"

void write_ipkg_log_file(const char* str, ...);

class CONFLIST
{
 public:
 	int type;
	eString features, name, value;
};

class MAINLIST
{
 public:
 	eString name, feed, path;
 };

class INSTLIST
{
 public:
 	int select;
	eString name, version, status, arch, prov, depends, feed, path;
	time_t Time;
};

class AVAILIST
{
 public:
 	int stat, select, size;
	eString name, version, section, arch, maintainer, md5_sum, filename, desc, feed, depends;
};

class TEMPLIST
{
 public:
 	int select;
	eString name;
};

class COMMLIST
{
 public:
 	int select;
	eString comm, file;
};

typedef std::vector<CONFLIST> ConfList;
typedef std::vector<MAINLIST> MainList;
typedef std::vector<INSTLIST> InstList;
typedef std::vector<AVAILIST> AvaiList;
typedef std::vector<TEMPLIST> TempList;
typedef std::vector<COMMLIST> CommList;

class CIPKG
{
	int sort_avai, sort_inst;
	void fillcommlist(eString file, bool install);
	void loadConfiguration();
	void unlinkPackage_no_root(eString name, eString path);
	int compareVersions( eString name, eString version);
	void parseVersion(eString verstr, long *version, eString *revision );
	int verrevcmp( const char *val, const char *ref );
public:
	enum {SOURCE, DESTINATION, OPTION, ARCH, NOTDEFINED};
	enum {MAIN, INST, AVAI, TEMP};
	
	CIPKG();
	~CIPKG();

	MainList mainliste;
	InstList instliste;
	AvaiList availiste;
	TempList templiste;
	ConfList confliste;
	CommList commliste;

	int m_ipkgExecOptions, m_ipkgExecVerbosity, m_ipkgExecDelete;
	eString m_ipkgExecDestPath;

	void mainPackages();
	void availablePackages(eString feed);
	void installedPackages();
	void tempPackages();

	void sort_list(int menu);
	void linkPackage_no_root();

	void startupdate();
	void startremove();
	void install_temp(bool all);
	void install_avai(bool upgrade);
};

#endif


