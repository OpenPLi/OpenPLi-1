/*	Ipkgpl - Ipkg Enigma Plugin

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

#include "ipkg.h"

bool sortBySeleInst(const INSTLIST& a, const INSTLIST& b) { return a.select > b.select ; }
bool sortByNameInst(const INSTLIST& a, const INSTLIST& b) { return a.name < b.name ; }
bool sortBySeleAvai(const AVAILIST& a, const AVAILIST& b) { return a.select > b.select ; }
bool sortByStatAvai(const AVAILIST& a, const AVAILIST& b) { return a.stat > b.stat ; }
bool sortByNameAvai(const AVAILIST& a, const AVAILIST& b) { return a.name < b.name ; }

//-------------------------------------------Log-File-------------------------------------------
void write_ipkg_log_file(const char* str, ...)
{
	time_t tt;
	time(&tt);
	char time_str[256];
	strftime(time_str, 20, "[%H:%M:%S] ", localtime(&tt));

	char buf[1024];
	va_list ap;
	va_start(ap, str);
	vsnprintf(buf, 1024, str, ap);
	va_end(ap);

	if(FILE *f=fopen(IPKG_LOG_FILE,"a"))
	{
		fprintf(f, "%s%s\n", time_str, buf);
		fclose(f);
	}
}

CIPKG::CIPKG()
{
	write_ipkg_log_file("start ipkg plugin %s",REL);
	loadConfiguration();
}

CIPKG::~CIPKG()
{
	mainliste.clear();
	instliste.clear();
	availiste.clear();
	templiste.clear();

	confliste.clear();
	commliste.clear();
}

void CIPKG::mainPackages()
{
	mainliste.clear();

	MAINLIST a;
	a.name = "installed packages";
	mainliste.push_back(a);
	a.name = "available packages";
	a.feed = "TEMP";
	a.path = IPKG_TEMP_PATH;
	mainliste.push_back(a);

	for(ConfList::iterator p=confliste.begin(); p!=confliste.end() ;p++)
		if((*p).type == SOURCE)
		{
			a.name = "available packages";
			a.feed = (*p).name;
			a.path = (*p).value;
			mainliste.push_back(a);
		}
}

void CIPKG::tempPackages()
{
	templiste.clear();

	TEMPLIST a;
	a.name = _("[GO UP]");
	templiste.push_back(a);

	DIR *d=opendir(IPKG_TEMP_PATH);
	if(d)
	{
		while (struct dirent *e=readdir(d))
		{
			if (!(strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))) continue;

			eString tmp(e->d_name); tmp.upper();

			if (tmp.find(".IPK")!= eString::npos || tmp.find(".IPKG")!= eString::npos)
			{
				a.name = e->d_name;
				a.select = 0;
				templiste.push_back(a);
			}
		}
		closedir(d);
	}
}

void CIPKG::availablePackages(eString feed)
{
	int sort_avai=0;
	bool newPackage = false;
	eString line, key, value, name, version, section, arch, maintainer, md5_sum, size, filename, desc, depends;
	availiste.clear();

	AVAILIST a;
	a.name = _("[GO UP]");
	a.feed = feed;
	availiste.push_back(a);

	if(FILE *f=fopen(eString().sprintf("%s/%s", IPKG_PKG_PATH, feed.c_str()).c_str(), "r"))
	{
		char tmp_line[256]; *tmp_line=0;
		while((fgets(tmp_line,256, f)!=NULL))
		{
			tmp_line[strlen(tmp_line)-1]=0;
			line=tmp_line;
			int pos = line.find( ':', 0 );

			if ( pos > -1 ) key = line.mid( 0, pos );
			else key = "";

			value = line.mid( pos+2, line.length()-pos );

			if ( newPackage  && key && name )
			{
				a.stat = compareVersions( name, version);
				a.name = name;
				a.version = version;
				a.feed = feed;
				a.section = section;
				a.select = 0;
				a.arch = arch;
				a.maintainer = maintainer;
				a.md5_sum = md5_sum;
				a.size = atoi(size.c_str());
				a.filename = filename;
				a.desc = desc;
				a.depends = depends;
				availiste.push_back(a);

				name = version = section = arch = maintainer = md5_sum = size = filename = desc = depends = "";
				newPackage = false;
			}

			if(key == "Package" )		name = value;
			else if(key == "Version" )	version = value;
			else if(key == "Section" )	section = value;
			else if(key == "Architecture")	arch = value;
			else if(key == "Maintainer" )	maintainer = value;
			else if(key == "MD5Sum" )	md5_sum = value;
			else if(key == "Size" )		size = value;
			else if(key == "Filename" )	filename = value;
			else if(key == "Description" )	desc = value;
			else if(key == "Depends" )	depends = value;
			else if(!key && !value)		newPackage = true;
		}
		fclose(f);

		if ( newPackage && name )//last Package
		{
			a.stat = compareVersions( name, version);
			a.name = name;
			a.version = version;
			a.feed = feed;
			a.section = section;
			a.select = 0;
			a.arch = arch;
			a.maintainer = maintainer;
			a.md5_sum = md5_sum;
			a.size = atoi(size.c_str());
			a.filename = filename;
			a.desc = desc;
			a.depends = depends;
			availiste.push_back(a);
		}
	}
}

void CIPKG::installedPackages()
{
	int sort_inst=0;
	bool newPackage = false;
	eString name, version, status, time, arch, prov, depends;
	instliste.clear();

	INSTLIST a;
	a.name = _("[GO UP]");
	instliste.push_back(a);

	for(ConfList::iterator p=confliste.begin(); p!=confliste.end() ;++p)
	{
		if((*p).type == DESTINATION && (*p).value.right(1) == "/")
		{
			eString statusfile = (*p).value.c_str(); statusfile += IPKG_STATUS_PATH;

			char tmp_line[256]; *tmp_line=0;
			if(FILE *f=fopen(statusfile.c_str(), "r"))
			{
				while((fgets(tmp_line,256, f)!=NULL))
				{
					tmp_line[strlen(tmp_line)-1]=0;
					eString line=tmp_line;
					int pos = line.find( ':', 0 );
					eString key, value;

					if ( pos > -1 ) key = line.mid( 0, pos );
					else key = "";

					value = line.mid( pos+2, line.length()-pos );

					if ( newPackage  && key.size() && name && status.find( " installed" ) )
					{
						a.name = name;
						a.version = version;
						a.status = status;
						a.arch = arch;
						a.depends = depends;
						a.prov = prov;
						a.Time = atoi(time.c_str());
						a.feed = (*p).name;
						a.path = (*p).value;
						a.select = 0;
						instliste.push_back(a);

						name = version = status = time = arch =  prov = depends = "";
						newPackage = false;
					}

					if ( key == "Package" )			name = value;
					else if ( key == "Version" )		version = value;
					else if ( key == "Status" )		status = value;
					else if ( key == "Installed-Time" )	time = value;
					else if ( key == "Architecture" )	arch = value;
					else if ( key == "Provides" )		prov = value;
					else if ( key == "Depends" )		depends = value;
					else if ( !key && !value)		newPackage = true;
				}
				fclose(f);

				if ( newPackage  && name && status.find( " installed" ) )//last Package
				{
					a.name = name;
					a.version = version;
					a.status = status;
					a.arch = arch;
					a.depends = depends;
					a.prov = prov;
					a.Time = atoi(time.c_str());
					a.feed = (*p).name;
					a.path = (*p).value;
					a.select = 0;
					instliste.push_back(a);

					name = version = status = time = arch =  prov = depends = "";
					newPackage = false;
				}
			}
		}
	}
}

void CIPKG::startupdate()
{
	eString target = IPKG_TEMP_PATH; target += "Packages";
	eString targetfull = target;
	bool isok = false;

	for(ConfList::iterator p=confliste.begin(); p!=confliste.end() ;p++)
	{
		if((*p).name == availiste[0].feed)
		{
			eString urlfull = (*p).value + "/Packages" ;
			if((*p).features == "Compressed")
			{
				targetfull += ".gz";
				urlfull += ".gz";
			}

			eDownload dlg(urlfull, targetfull, false);
			dlg.show();
			int is = dlg.exec();

			dlg.hide();

			if(is == 1)
			{
				isok = true;

				if((*p).features == "Compressed")
				{
					if(system(eString().sprintf("/bin/gunzip %s", targetfull.c_str()).c_str())>>8)
					{
						write_ipkg_log_file("couldn't unzip %s", targetfull.c_str());
						isok = false;
					}
				}

				if(system(eString().sprintf("/bin/cp -a %s %s/%s", target.c_str(), IPKG_PKG_PATH, availiste[0].feed.c_str()).c_str())>>8)
				{
					write_ipkg_log_file("couldn't copy %s", target.c_str());
					isok = false;
				}

			}
			break;
		}
	}

	if(isok)
	{
		unlink(target.c_str());
		write_ipkg_log_file("feed \"%s\" updated", availiste[0].feed.c_str());
		availablePackages(availiste[0].feed);
	}
	else write_ipkg_log_file("feed \"%s\" not updated", availiste[0].feed.c_str());

}

void CIPKG::loadConfiguration()
{
	std::list<eString> configFileList;

	DIR *d=opendir(IPKG_CONF_DIR);
	if(d)
	{
		while (struct dirent *e=readdir(d))
		{
			if (!(strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))) continue;

			eString tmp(e->d_name); tmp.upper();

			if (tmp.find(".CONF")!= eString::npos)
				configFileList.push_back(eString().sprintf("%s/%s", IPKG_CONF_DIR, e->d_name).c_str());
		}
		closedir(d);
	}
	configFileList.push_back(IPKG_CONF);

	for (std::list<eString>::iterator it = configFileList.begin(); it != configFileList.end(); ++it)
	{
		eString absFile = (*it);
		//eDebug("[IPKG] configfiles: %s", absFile.c_str());

		char line[256]; *line=0;
		if(FILE *f=fopen(absFile.c_str(), "r"))
		{
			while((fgets(line,256, f)!=NULL))
			{
				line[strlen(line)-1]=0;
				eString tmp=line;
				int pos = tmp.find( ' ', 1 );

				CONFLIST c;
				//type
				eString typeStr = tmp.left( pos );
				if ( typeStr == "src" || typeStr == "#src" )
					c.type = SOURCE;
				else if ( typeStr == "src/gz" || typeStr == "#src/gz" )
				{
					c.type = SOURCE;
					c.features = "Compressed";
				}
				else if ( typeStr == "dest" || typeStr == "#dest" )	c.type = DESTINATION;
				else if ( typeStr == "option" || typeStr == "#option" )	c.type = OPTION;
				else if ( typeStr == "arch" || typeStr == "#arch" )	c.type = ARCH;
				else							c.type = NOTDEFINED;

				++pos;
				int endpos = tmp.find( ' ', pos );

				c.name = tmp.mid( pos, endpos - pos );
				c.value = "";
				if ( endpos > -1 )
					c.value = tmp.right( tmp.length() - endpos - 1 );

				confliste.push_back(c);
			}
			fclose(f);
		}
	}

	configFileList.clear();

	//clear
	for(ConfList::iterator p=confliste.begin(); p!=confliste.end() ;p++)
		if((*p).type == NOTDEFINED)
		{
			confliste.erase(p);
			p--;
		}


	m_ipkgExecOptions = 0;
	eConfig::getInstance()->getKey("/enigma/plugins/ipkg/ExecOptions", m_ipkgExecOptions);
	m_ipkgExecVerbosity = 1;
	eConfig::getInstance()->getKey("/enigma/plugins/ipkg/Verbosity", m_ipkgExecVerbosity);
	m_ipkgExecDelete = 1;
	eConfig::getInstance()->getKey("/enigma/plugins/ipkg/Delete", m_ipkgExecDelete);
	m_ipkgExecDestPath = "root";
}

void CIPKG::linkPackage_no_root()
{
	eString rootDir = "/";
	eString destDir, packageDir, packageFileName;

	for(ConfList::iterator p=confliste.begin(); p!=confliste.end() ;p++)
	{
		if((*p).name == m_ipkgExecDestPath)
		{
			destDir = (*p).value;
			packageDir = destDir; packageDir += IPKG_INFO_PATH;
			//eDebug("[IPKG] destination directory:%s", packageDir.c_str());
		}

		if((*p).name == "root") rootDir = (*p).value;
	}

	std::list<eString> packageFiles;

	DIR *d=opendir(packageDir.c_str());
	if(d)
	{
		while (struct dirent *e=readdir(d))
		{
			if (!(strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))) continue;
			eString tmp(e->d_name); tmp.upper();
			if (tmp.find(".LIST")!= eString::npos)
				packageFiles.push_back(e->d_name);
		}
		closedir(d);
	}

	for (std::list<eString>::iterator it = packageFiles.begin(); it != packageFiles.end(); ++it)
	{
		packageFileName = packageDir + "/" + (*it);

		char line[256]; *line=0;
		if(FILE *f=fopen(packageFileName.c_str(), "r"))
		{
			while((fgets(line,256, f)!=NULL))
			{
				line[strlen(line)-1]=0;
				eString linkFile = line;
				eString linkDest = rootDir + ( linkFile.right( linkFile.length() - destDir.length() ) );
				eString tmp = linkDest.mid(linkDest.rfind('/')+1);
				eString linkDestDir = linkDest.mid(0, linkDest.length() - tmp.length() );
				//eDebug("[IPKG] Link Destination:<%s>\n[IPKG] Link File:<%s>\n[IPKG] Link Destination Dir:<%s>", linkDest.c_str(), linkFile.c_str(),linkDestDir.c_str());
				if(DIR *d=opendir(linkDestDir.c_str()))
					closedir(d);
				else
				{
					if ( mkdir(linkDestDir.c_str(),0755) )
						write_ipkg_log_file("couldn't create directory %s", linkDestDir.c_str());
				}

				if(FILE *fi=fopen(linkDest.c_str(), "r"))
				{
					fclose(fi);
					if ( remove(linkDest.c_str()) )
						write_ipkg_log_file("couldn't remove %s", linkDest.c_str());
				}

				if ( symlink( linkFile.c_str(), linkDest.c_str() ) == -1 )
					write_ipkg_log_file("couldn't create symlink %s", linkFile.c_str());
			}
			fclose(f);
		}
	}
}

void CIPKG::unlinkPackage_no_root(eString name, eString path)
{
	if(name && path)
	{
		eString rootDir = "/";
		for(ConfList::iterator p=confliste.begin(); p!=confliste.end() ;p++)
		{
			if((*p).type == DESTINATION && (*p).name == "root")
			{
				rootDir = (*p).value;
				break;
			}
		}

		eString destInfoFileName = path + IPKG_INFO_PATH + "/" + name + ".list";
		write_ipkg_log_file("destInfoFileName: %s",destInfoFileName.c_str());

		char line[256]; *line=0;
		if(FILE *f=fopen(destInfoFileName.c_str(), "r"))
		{
			while((fgets(line,256, f)!=NULL))
			{
				line[strlen(line)-1]=0;
				eString linkFile = line;
				eString linkDest = rootDir + ( linkFile.right( linkFile.length() -path.length() ) );
				unlink(linkDest.c_str());
			}
			fclose(f);
		}
	}
}

int CIPKG::compareVersions( eString name, eString version)
{
	for(InstList::iterator p=instliste.begin()+1; p!=instliste.end() ;p++)
		if( name == (*p).name)
		{
			if(version == (*p).version) return 1;
			else
			{
				long version1, version2;
				eString revision1, revision2;

				parseVersion( version, &version1, &revision1 );
				parseVersion( (*p).version, &version2, &revision2 );

				if(version1 > version2) return 2;
				if(version1 < version2) return 3;

				int r = verrevcmp( revision1.c_str(), revision2.c_str() );
				if(r > 0) return 2;
				if(r < 0) return 3;

				/*eDebug("name:%s\n",name.c_str());
				eDebug("avia:%s inst:%s",version.c_str(),(*p).version.c_str());
				eDebug("avia:%d %s inst:%d %s revision:%d\n", version1, revision1.c_str(), version2, revision2.c_str(), r);*/
			}
		}

	return 0;
}

void CIPKG::parseVersion(eString verstr, long *version, eString *revision )
{
	if(verstr.find("cvs-") != eString::npos)
		verstr=verstr.mid(4);

	int hyphenpos = verstr.find( '-');
	int verlen = verstr.length();
	if ( hyphenpos > -1 )
	{
		eString tmp = verstr.mid(0, hyphenpos);

		eString rec="";
		char buffer[1024];
		strcpy(buffer,tmp.c_str());
		for(unsigned int a=0; a < strlen(buffer); a++)//clear '.'
		{
			if(buffer[a] != '.') rec+=buffer[a];
		}

		*version = atoi(rec.c_str());
		*revision = verstr.right( verlen - hyphenpos - 1 );
	}
}

int CIPKG::verrevcmp( const char *val, const char *ref )
{
	int vc, rc;
	long vl, rl;
	const char *vp, *rp;
	const char *vsep, *rsep;

	if (!val) val= "";
	if (!ref) ref= "";
	for (;;)
	{
		vp= val;
		while (*vp && !isdigit(*vp)) vp++;
		rp= ref;
		while (*rp && !isdigit(*rp)) rp++;
		for (;;)
		{
			vc= (val == vp) ? 0 : *val++;
			rc= (ref == rp) ? 0 : *ref++;
			if (!rc && !vc) break;
			if (vc && !isalpha(vc)) vc += 256; /* assumes ASCII character set */
			if (rc && !isalpha(rc)) rc += 256;
			if (vc != rc) return vc - rc;
		}
		val= vp;
		ref= rp;
		vl=0;
		if (isdigit(*vp)) vl= strtol(val,(char**)&val,10);
		rl=0;
		if (isdigit(*rp)) rl= strtol(ref,(char**)&ref,10);
		if (vl != rl) return vl - rl;

		vc = *val;
		rc = *ref;
		vsep = strchr(".-", vc);
		rsep = strchr(".-", rc);
		if (vsep && !rsep) return -1;
		if (!vsep && rsep) return +1;

		if (!*val && !*ref) return 0;
		if (!*val) return -1;
		if (!*ref) return +1;
	}
}

void CIPKG::startremove()
{
	commliste.clear();
	for(InstList::iterator p=instliste.begin()+1; p!=instliste.end() ;p++)
		if((*p).select)
		{
			if((*p).feed != "root")
				unlinkPackage_no_root((*p).name, (*p).path);

			fillcommlist((*p).name, false);
		}
}


void CIPKG::install_temp(bool all)
{
	commliste.clear();
	for(TempList::iterator p=templiste.begin()+1; p!=templiste.end() ;p++)
	{
		if((*p).select && !all)	fillcommlist(IPKG_TEMP_PATH + (*p).name, true);
		if(all)			fillcommlist(IPKG_TEMP_PATH + (*p).name, true);
	}
}

void CIPKG::install_avai(bool upgrade)
{
	commliste.clear();
	eString url="";
	
	for(ConfList::iterator p=confliste.begin(); p!=confliste.end() ;p++)
		if((*p).name == availiste[0].feed) url = (*p).value;

	for(AvaiList::iterator p=availiste.begin()+1; p!=availiste.end() ;p++)
	{
		eString urlfull="";
		if((*p).stat == 2 && upgrade)	urlfull = url + "/" + (*p).filename;
		if((*p).select && !upgrade)	urlfull = url + "/" + (*p).filename;

		if(urlfull)
		{
			//download
			eString target = IPKG_TEMP_PATH + (*p).filename;
			//eDebug(urlfull.c_str());
			eDownload dlg(urlfull, target, true);
			dlg.show();
			int isok = dlg.exec();
			dlg.hide();

			if(isok != 1 ) unlink(target.c_str());
			else fillcommlist(target, true);
		}
	}
}

void CIPKG::fillcommlist(eString file, bool install)
{
	eString cmd_ipkg = eString().sprintf("ipkg -V %d", m_ipkgExecVerbosity).c_str();

	if(m_ipkgExecDestPath != "root")
		cmd_ipkg += " -dest " + m_ipkgExecDestPath;

	if(m_ipkgExecOptions & FORCE_DEPENDS)	cmd_ipkg += " -force-depends";
	//if(m_ipkgExecOptions & FORCE_REMOVE)	cmd_ipkg += " -force-removal-of-dependent-packages";
	if(m_ipkgExecOptions & FORCE_REINSTALL) cmd_ipkg += " -force-reinstall";
	if(m_ipkgExecOptions & FORCE_OVERWRITE) cmd_ipkg += " -force-overwrite";

	if(install)	cmd_ipkg += " install " + file;
	else		cmd_ipkg += " remove " + file;

	COMMLIST a;
	a.comm = cmd_ipkg;
	a.file = file;
	commliste.push_back(a);
}

void CIPKG::sort_list(int menu)
{
	switch(menu)
	{
		case INST:
			sort_inst++;
			if(sort_inst > 2) sort_inst = 1;

			if(sort_inst == 1) sort(instliste.begin()+1,instliste.end(),sortBySeleInst);
			if(sort_inst == 2) sort(instliste.begin()+1,instliste.end(),sortByNameInst);
			break;
		case AVAI:
			sort_avai++;
			if(sort_avai > 3) sort_avai = 1;

			if(sort_avai == 1) sort(availiste.begin()+1,availiste.end(),sortByStatAvai);
			if(sort_avai == 2) sort(availiste.begin()+1,availiste.end(),sortBySeleAvai);
			if(sort_avai == 3) sort(availiste.begin()+1,availiste.end(),sortByNameAvai);
			break;
	}
}




