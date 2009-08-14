/*
 * $Id: enigma_dyn_boot.cpp,v 1.21 2005/12/28 10:42:06 digi_casi Exp $
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
 
#ifdef ENABLE_EXPERT_WEBIF
#include <map>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/vfs.h>
#include <net/if.h>
#include <sys/mount.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <lib/system/info.h>
#include <lib/system/http_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_boot.h>
#include <bootmenue/bmconfig.h>
#define INSTIMAGESUPPORT
#include <bootmenue/bmimage.h>
#include <bootmenue/bmboot.h>

extern eString firmwareLevel(eString versionString);

using namespace std;

eString getSkins(eString skinPath, eString skinName, eString mountPoint)
{	
	bmboot bmgr;
	eString skins;
	
	bmgr.getSkins(skinPath, mountPoint);
	for (unsigned int i = 0; i < bmgr.skinList.size(); i++)
	{
		eString skin = bmgr.skinList[i];
		unsigned int pos = skin.find_last_of('/');
		eString name = skin.right(skin.length() - pos - 1);
		eString skinPath2 = skin.left(pos);
		
		skins = skins + "<option value=\"" + skin + "\"" + eString((skinName == name && skinPath == skinPath2) ? " selected" : "") + ">" + name + "</option>";
	}

	if (!skins)
		skins = "<option value=\"" + skinPath + "\">none</option>";
	
	return skins;
}

eString getMenus()
{
	bool bm = false;
	bool fw = false;
	bool fwInstalled = false;
	eString line;
	eString result;
	bmboot bmgr;
	
	bmgr.mountJFFS2();
		
	ifstream initFile ("/tmp/jffs2/etc/init");
	if (initFile)
	{
		while (getline(initFile, line, '\n'))
		{
			if (line.find("fwpro") != eString::npos)
			{
				int pos = line.find_first_not_of(" ");
				fw = line[pos] != '#' && line[pos] != ':';
				fwInstalled = true;
			}
			if (line.find("bm.sh") != eString::npos)
			{
				int pos = line.find_first_not_of(" ");
				bm = line[pos] != '#' && line[pos] != ':';
			}
		}
	}
	initFile.close();
	
	if (bm && fw)
	{
		bmgr.activateMenu("BM");
		fw = false;
	}
	
	eString menus = "<option value=\"none\">none</option>";
	menus += "<option value=\"BM\"" + eString((bm) ? " selected" : "") + ">BootManager</option>";
	if (fwInstalled)
		menus += "<option value=\"FW\"" + eString((fw) ? " selected" : "") + ">FlashWizard</option>";
	result = readFile(TEMPLATE_DIR + "bootMenus.tmp");
	result.strReplace("#OPTIONS#", menus);
	if (bm)
		result.strReplace("#BMSETTINGSBUTTON#", button(100, "Settings", TOPNAVICOLOR, "javascript:editBootManagerSettings('')", "#000000"));
	else
		result.strReplace("#BMSETTINGSBUTTON#", "&nbsp;");
	
	bmgr.unmountJFFS2();
		
	return result;
}

eString getInstalledImages()
{
	eString images;
	bmconfig cfg;
	bmimages imgs;
	bmboot bmgr;
	
	bmgr.mountJFFS2();
	cfg.load();
	bmgr.unmountJFFS2();
	
	imgs.load(cfg.mpoint, true);
	
	for (unsigned int i = 0; i < imgs.imageList.size(); i++)
	{
		eString name = imgs.imageList[i].name;
		eString location = imgs.imageList[i].location;
		eString image = readFile(TEMPLATE_DIR + "image.tmp");
		image.strReplace("#NAME#", name);
		eString version = firmwareLevel(getAttribute(location + "/.version", "version"));
		image.strReplace("#VERSION#", version);
		if (location)
		{
			image.strReplace("#LOCATION#", location);
			image.strReplace("#SETTINGSBUTTON#", button(80, "Settings", GREEN, "javascript:editImageSettings('" + location + "')", "#FFFFFF"));
			image.strReplace("#DELETEBUTTON#", button(80, "Delete", RED, "javascript:deleteImage('" + location + "')", "#FFFFFF"));
		}
		else
		{
			image.strReplace("#LOCATION#", "&nbsp;");
			image.strReplace("#SETTINGSBUTTON#", "&nbsp;");
			image.strReplace("#DELETEBUTTON#", "&nbsp;");
		}
		images += image;
	}
	
	if (!images)
		images = "<tr><td colspan=\"5\">No images found on selected boot device.</td></tr>";

	return images;
}

eString getConfigBoot(void)
{
	eString result = readFile(TEMPLATE_DIR + "bootMgr.tmp");
	result.strReplace("#MENU#", getMenus());
	result.strReplace("#IMAGES#", getInstalledImages());
	result.strReplace("#ADDIMAGEBUTTON#", button(100, "Add", GREEN, "javascript:showAddImageWindow('')", "#FFFFFF"));

	return result;
}

eString installImage(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	bmimages imgs;
	bmconfig cfg;
	bmboot bmgr;
	eString result;
	
	bmgr.mountJFFS2();
	cfg.load();
	bmgr.unmountJFFS2();
	
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString sourceImage = opt["image"];
	if (sourceImage != "none")
	{
		eString mountDir = opt["target"];
		if (!mountDir)
			mountDir = cfg.mpoint;
		eString imageName = opt["name"];
		if (!imageName)
		{
			unsigned int pos = sourceImage.find_last_of("/");
			imageName = sourceImage.right(sourceImage.length() - pos - 1);
			imageName = imageName.left(imageName.length() - 4);
		}
		content->local_header["Content-Type"]="text/html; charset=utf-8";
	
		int rc = imgs.add(sourceImage, imageName, mountDir);
		if (rc == 0)
			result = "<html><head><title>Image installation in process</title></head><body>Enigma will shut down during the installation process and restart once installation is complete.<br><br>Please wait...</body></html>";
		else
			result = "<html><head><title>Error during image installation</title></head><body>The image could not be installed due to error: " + eString().sprintf("%d", rc) + "</body></html>";
	}
	else
		result =  closeWindow(content, "", 10);
	
	return result;
}

eString deleteImage(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	bmimages imgs;
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString image = opt["image"];
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	imgs.discard(image);
	
	return closeWindow(content, "", 10);
}

eString editImageSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	bmimages imgs;
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString image = opt["image"];
	
	eString result = readFile(TEMPLATE_DIR + "editImageSettingsWindow.tmp");
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";

	bool reload_modules = (access(eString(image + "/lib/modules/2.6.9/.reload_modules").c_str(), R_OK) == 0);
	bool fast_boot = (access(eString(image + "/var/etc/.dont_mount_hdd").c_str(), R_OK) == 0);
	bool dont_start_dccamd = (access(eString(image + "/var/etc/.dont_start_dccamd").c_str(), R_OK) == 0);
	
	result.strReplace("#IMAGE#", image);
	result.strReplace("#NAME#", imgs.getImageName(image));
	result.strReplace("#RELOADMODULES#", (reload_modules) ? "checked" : "");
	result.strReplace("#FASTBOOT#", (fast_boot) ? "checked" : "");
	result.strReplace("#DONTSTARTDCCAMD#", (dont_start_dccamd) ? "checked" : "");
	
	return result;
}


eString setImageSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	bmimages imgs;
	
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	eString image = opt["image"];
	eString name = opt["name"];
	eString reload_modules = opt["reload_modules"];
	eString fast_boot = opt["fast_boot"];
	eString dont_start_dccamd = opt["dont_start_dccamd"];
	
	imgs.setImageName(image, name);
	if (reload_modules == "on")
		system(eString("touch " + image + "/lib/modules/2.6.9/.reload_modules").c_str());
	else
		system(eString("rm " + image + "/lib/modules/2.6.9/.reload_modules").c_str());
	
	if (fast_boot == "on")
		system(eString("touch " + image + "/var/etc/.dont_mount_hdd").c_str());
	else
		system(eString("rm " + image + "/var/etc/.dont_mount_hdd").c_str());
	
	if (dont_start_dccamd == "on")
		system(eString("touch " + image + "/var/etc/.dont_start_dccamd").c_str());
	else
		system(eString("rm " + image + "/var/etc/.dont_start_dccamd").c_str());
	
	return WINDOWCLOSE;
}

eString editBootManagerSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";

	bmconfig cfg;
	bmboot bmgr;
	
	bmgr.mountJFFS2();
	cfg.load();
	bmgr.unmountJFFS2();
	
	eString result = readFile(TEMPLATE_DIR + "bootMgrSettings.tmp");
	result.strReplace("#SKINOPTIONS#", getSkins(SKINDIR, cfg.skinName, cfg.mpoint));
	result.strReplace("#MPOINT#", cfg.mpoint);
	result.strReplace("#SELECTEDENTRY#", cfg.selectedEntry);
	result.strReplace("#RANDOMSKIN#", cfg.randomSkin);
	result.strReplace("#TIMEOUTVALUE#", cfg.timeoutValue);
	result.strReplace("#VIDEOFORMAT#", cfg.videoFormat);
	result.strReplace("#SKINPATH1#", cfg.skinPath);
	result.strReplace("#SKINPATH2#", cfg.mpoint + "/boot/skins");
	result.strReplace("#SKINNAME#", cfg.skinName);
	
	result.strReplace("#BUTTONSUBMIT#", button(100, "Change", TOPNAVICOLOR, "javascript:submitSettings()", "#000000"));
	
	return result;
}

eString setBootManagerSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	bmconfig cfg;
	bmboot bmgr;
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	cfg.mpoint = opt["mpoint"];
	cfg.selectedEntry = opt["selectedEntry"];
	cfg.randomSkin = opt["randomSkin"];
	cfg.timeoutValue = opt["timeoutValue"];
	cfg.videoFormat = opt["videoFormat"];
	cfg.skinPath = opt["skinPath1"];
	cfg.skinName = opt["skinName"];
	unsigned int pos = cfg.skinName.find_last_of("/");
	if (pos != eString::npos && pos > 0)
		cfg.skinName = cfg.skinName.right(cfg.skinName.length() - pos - 1);
	
	bmgr.mountJFFS2();
	cfg.save();
	bmgr.unmountJFFS2();
	
	return WINDOWCLOSE;
}

eString selectBootMenu(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	bmboot bmgr;
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString menu = opt["menu"];
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	bmgr.activateMenu(menu);
	
	return closeWindow(content, "", 10);
}

eString showAddImageWindow(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	bmconfig cfg;
	bmboot bmgr;
	
	bmgr.mountJFFS2();
	cfg.load();
	bmgr.unmountJFFS2();
	
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	
	eString result = readFile(TEMPLATE_DIR + "addImageWindow.tmp");
	
	eString images;
	if (DIR *p = opendir(cfg.mpoint.c_str()))
	{
		while (struct dirent *e = readdir(p))
		{
			eString name = cfg.mpoint + "/" + e->d_name;
			eString tmp = name; 
			tmp.upper();
			if (tmp.find(".IMG") != eString::npos)
				images += "<option value=\"" + name + "\">" + e->d_name + "</option>\n";
		}
		closedir(p);
	}
	if (!images)
		images = "<option value=\"none\">none</option>";
	
	result.strReplace("#MPOINT#", cfg.mpoint);
	result.strReplace("#IMAGES#", images);
	
	return result;
}

void ezapBootManagerInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/cgi-bin/installimage", installImage, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/deleteimage", deleteImage, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/editimagesettings", editImageSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/setimagesettings", setImageSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/selectbootmenu", selectBootMenu, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/showaddimagewindow", showAddImageWindow, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/editbootmanagersettings", editBootManagerSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/setbootmanagersettings", setBootManagerSettings, lockWeb);
}
#endif
