/*
 *  PPanel
 *
 *  Original source: The_Hydra
 *  Modified and extended for PLi: dAF2000
 *
 *  Copyright (C) The_Hydra
 *  All additions to original CPanel: Copyright (C) dAF2000
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  $Date$
 *  $Revision$
 */

#define RELEASENAME "jade"
#define EHTTPDOWNLOAD

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <plugin.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <xmltree.h>

#include <lib/base/console.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/emessage.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/combobox.h>
#include <lib/gdi/font.h>
#include <src/enigma.h>
#include <src/enigma_main.h>
#include <src/upgrade.h>
#include <lib/gui/listbox.h>
#include <lib/dvb/servicefile.h>
#include <lib/dvb/service.h>
#include <lib/system/httpd.h>
#include <lib/system/econfig.h>
#include <lib/gui/ExecuteOutput.h>
#include <lib/gui/myListBoxEntryText.h>

#include <parentallock.h>

#define PPANELRESTARTCODE (684)
#include "ppanel.h"

InstalledPackages installedPackages;

/******************************************************************************/

#ifdef COMPILEASPLUGIN
extern "C" int plugin_exec(PluginParam *par);

int plugin_exec(PluginParam *par)
{
	PPanel::runPPanel("/var/etc/ppanel.xml");
   return 0;
}
#endif

/******************************************************************************/

// Fixed version of system()
// system() returns in some cases errno ECHILD caused by the child finished
// before system() could do a waitpid()
int systemFixed(const char *command)
{
   int retval;
   eString newCommand;
   const char *ppr = "/tmp/returnvalue";

   newCommand = (eString)command + (eString)" ;echo $? > " + (eString)ppr;
   
   char temp[1024];
   FILE *pipe = popen(newCommand.c_str(), "r");
   if(pipe)
   {
      while(fgets(temp, 1023, pipe));
      pclose(pipe);
      
      FILE *fp = fopen(ppr, "r");
      if(fp)
      {
         fscanf(fp, "%d", &retval);
         fclose(fp);
      }
      else
      {
         retval = -1;
      }
   }
   else
   {
      retval = -1;
   }
   
   unlink(ppr);
   
   return retval;
}

/******************************************************************************/

// Runs a script with or without output to the screen
int runScript(
   eString command, 
   bool output, 
   PPanel &parentdlg, 
   eString title=_("Script output"))
{
   int rc = 0;

   if(command.length() > 0)
   {
      // Script available
      if(output)
      {
         // Show output
         ExecuteOutput dlg(title, command);
         parentdlg.hide();
         dlg.show();
         dlg.exec();
         dlg.hide();
         parentdlg.show();
      }
      else
      {
         // No output
         rc = systemFixed(command.c_str());
      }
   }

   return rc;
}

/******************************************************************************/

eString descramble(eString text)
{
   const unsigned int mask[16]=
      { 89, 52, 178, 9, 56, 86, 114, 5, 29, 98, 156, 209, 243, 73, 198, 101 };

   eString descrambled;
   char kar;
   int masknr = 0;
   
   if(text[0] == '*')
   {
      for(unsigned int i=1; i<text.length(); i+=2)
      {
         kar = (text[i] - 65) * 16 + (text[i+1] -65);
         
         kar ^= mask[masknr++];
         masknr &= 15;
         
         kar = 128 * (unsigned int)(kar&64)/64+
               64  * (unsigned int)(kar&2)/2+
               32  * (unsigned int)(kar&1)+
               16  * (unsigned int)(kar&4)/4+
               8   * (unsigned int)(kar&32)/32+
               4   * (unsigned int)(kar&128)/128+
               2   * (unsigned int)(kar&8)/8+
                     (unsigned int)(kar&16)/16;
                     
         descrambled += kar;
      }
   }
   else
   {
      descrambled = text;
   }
   
   return descrambled;
}

/******************************************************************************/

int downloadFile(
   BaseWindow &parentdlg,
   eString filename, 
   eString url, 
   eString title=_("Downloading"),
	bool dontCheckDownload = false)
{
   int rc = 0;
   
	url.strReplace("#RELEASE#", RELEASENAME);

   if(url.left(6) == "ftp://")
   {
      // FTP download (without download progressbar)
      eString ftpCommand;
      
      ftpCommand = "wget -O \"" + filename + "\" " + url;
      rc = systemFixed(ftpCommand.c_str());
   }
   else if(url.left(7) == "http://")
   {
      // HTTP download (with download progressbar)
      DownloadIndicator mb(_("Please wait"), 
         title, filename, url);
      mb.show();
      rc = mb.exec();
      mb.hide();
   }
   else
   {
      // Unknown protocol
      rc = -1;
   }

	if(!dontCheckDownload)
	{
		if(rc == 0)
		{
			struct stat fst;

			if (stat(filename.c_str(), &fst))
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}

		if(rc != 0)                                                                    
		{                                                                              
			parentdlg.setError(_("Download failed.\n"                
				"Please check available space and your communication settings."));       
		}
	}                                                                              

   return rc;
}

/******************************************************************************/

int installAddon(
   ePPanelEntry *entry, 
   BaseWindow &parentdlg, 
   eString packageName, 
   eString version, 
   eString packageType, 
   eString action,
	eString url,
   bool addPackage,
   bool showAsInstalled)
{
   int rc = 0;
   eString command;
   eString title;

   title.sprintf(_("Installing %s"), packageName.c_str());
   
   command.sprintf("install.sh %s '%s_%s' %s '%s'",
      action.c_str(),
      packageType.c_str(),
      packageName.c_str(),
      version.c_str(),
		url.c_str());

   ExecuteOutput dlg(title, command);
   parentdlg.hide();
   dlg.show();
   dlg.exec();
   dlg.hide();
   parentdlg.show();

   // The remove part of an upgrade is assumed never to fail
   if(addPackage && action == "upgrade")                                        
   {                                                     
      installedPackages.removePackage(packageName);   
   }                                                  

   // Check if installer returned an error
   if(access("/tmp/installerror", R_OK) != 0)
   {
      if(addPackage)
      {
         // Add package to installed packages
         installedPackages.addPackage(InstalledPackages::PackageInfo(packageName));
      }
      
      if(showAsInstalled)
      {
         // Set checked to installed and refresh screen
         entry->setChecked(true);
         parentdlg.list.invalidate();
      }
   }
   else
   {
      parentdlg.setError(_("Install failed."));
   }

   return rc;
}

/******************************************************************************/

bool confirmUser(eString confirmation, eString question)
{
   bool confirmed = false;
   eString actualQuestion;

   if(confirmation == "true")
   {
      actualQuestion = question;
   }
   else if(confirmation.length() != 0)
   {
      actualQuestion = confirmation;
   }
   else
   {
      confirmed = true;
   }

   if(!confirmed)
   {
      eMessageBox mb(
         actualQuestion,
         _("Continue"),
         eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion,
         eMessageBox::btNo);

      mb.show();
      int res=mb.exec();
      mb.hide();

      if(res == eMessageBox::btYes)
         confirmed = true;
   }

   return confirmed;
}

/******************************************************************************/

ePPanelEntry::ePPanelEntry(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext, 
   XMLTreeNode *node, 
   bool checked)
   : eListBoxEntryCheck(listbox, name, helptext), 
   parentdlg(parentdlg), node(node), name(name)
{
	setChecked(checked);
   confirmation=node->GetAttributeValue("confirmation");
   quit=node->GetAttributeValue("quit");
   runBefore = node->GetAttributeValue("runBefore");
   runAfter = node->GetAttributeValue("runAfter");
   runBeforeOut = node->GetAttributeValue("runBeforeOut");
   runAfterOut = node->GetAttributeValue("runAfterOut");
}

int ePPanelEntry::preActions(void)
{
   int rc;
   
   rc = runScript(runBefore, false, parentdlg);
   if(rc == 0) rc = runScript(runBeforeOut, true, parentdlg);
   
   return rc;
}

int ePPanelEntry::postActions(void)
{
   int rc;
   
   rc = runScript(runAfter, false, parentdlg);
   if(rc == 0) rc = runScript(runAfterOut, true, parentdlg);
   
   if(quit == "exit")                       
   {                                        
      parentdlg.close(0);                   
   }                                        
   else if(quit == "restart")               
   {                                        
      parentdlg.close(PPANELRESTARTCODE);   
   }
	else if(quit == "hide")
	{
		parentdlg.hide();
		MiniPressOKWindow dlg;
		dlg.show();
		dlg.exec();
		dlg.hide();
		parentdlg.show();
	}                                        

   return rc;
}
	
/******************************************************************************/

MiniPressOKWindow::MiniPressOKWindow()
{
	int xpos = 20;
	int ypos = 20;
	
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/left", xpos);
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/top", ypos);
	
	move(ePoint(xpos, ypos));
	cresize(eSize(100, 0));
	setText(_("Press OK"));

	btRestore = new eButton(this);
	CONNECT(btRestore->selected, eWidget::reject);
}

/******************************************************************************/

eListBoxEntryPPanel::eListBoxEntryPPanel(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext, 
   XMLTreeNode *node)
   : ePPanelEntry(parentdlg, listbox, name, helptext, node)
{
   target = descramble((eString)node->GetAttributeValue("target"));
}

void eListBoxEntryPPanel::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;

   int rc = 0;
   eString downloadFilename;
   bool removeAfterwards = false;

   if(confirmUser(confirmation, 
		eString().sprintf(_("Are you sure to open\n%s?"), name.c_str())))
   {
      // Execute script before
      rc = preActions();

      parentdlg.hide();
		if(target.length() > 0)
		{
			if(target.left(1) == "/")
			{
				// It's a PPanel file
				downloadFilename = target;
			}
			else
			{
				// It's a URL, download it
				downloadFilename.sprintf("/tmp/ppanel%p.xml", (void*)this);
				
				rc = downloadFile(parentdlg, downloadFilename, target,
            	eString().sprintf(_("Downloading %s"), getText().c_str()));
         	
				removeAfterwards = true;
      	}
			
			// A real PPanel
			PPanel::runPPanel(downloadFilename, parentdlg.LCDTitle, parentdlg.LCDElement);
		}
		else
		{
			// A category
			PPanel::runPPanel(node, parentdlg.LCDTitle, parentdlg.LCDElement);
		}
      parentdlg.show();

      // Execute script after
      if(rc == 0) rc = postActions();

      // Clean up temporary file
      if(removeAfterwards)
      {
         unlink(downloadFilename.c_str());
      }
   }
}

/******************************************************************************/

eListBoxEntryFile::eListBoxEntryFile(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext, 
   XMLTreeNode *node)
   : ePPanelEntry(parentdlg, listbox, name, helptext, node)
{
   url=descramble((eString)node->GetAttributeValue("url"));
   target=node->GetAttributeValue("target");
	dontCheck=node->GetAttributeValue("dontCheck");
	
	if(target == "")
	{
		// Always safe
		target = "/dev/null";
	}
}

void eListBoxEntryFile::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;
   int rc = 0;
   eString command;
   eString status;

   if(confirmUser(confirmation, 
      eString().sprintf(_("Are you sure to download\n%s?"), target.c_str())))
   {
      // Execute script before
      rc = preActions();

      if(rc == 0)
      {
			if(dontCheck == "true")
			{
				rc = downloadFile(parentdlg, target, url, getText(), true);
			}
			else
			{
				rc = downloadFile(parentdlg, target, url, 
					eString().sprintf(_("Downloading %s"), getText().c_str()));
			}
      }

      // Execute script after
      if(rc == 0) rc = postActions();
   }
}

/******************************************************************************/

eListBoxEntryPlugins::eListBoxEntryPlugins(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext, 
   XMLTreeNode *node)
   : ePPanelEntry(parentdlg, listbox, name, helptext, node)
{
}

void eListBoxEntryPlugins::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;
   parentdlg.hide();
   eZapPlugins plugins(eZapPlugins::StandardPlugin);
   plugins.exec();
   parentdlg.show();
}

/******************************************************************************/

eListBoxEntryPlugin::eListBoxEntryPlugin(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext, 
   XMLTreeNode *node)
   : ePPanelEntry(parentdlg, listbox, name, helptext, node)
{
   target=node->GetAttributeValue("target");
}

void eListBoxEntryPlugin::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;
   int rc = 0;
   eString path;
   const char *pluginPath[] =
   {
      "/var/tuxbox/plugins/",
      PLUGINDIR "/",
      ""
   };

   if(confirmUser(confirmation,
      eString().sprintf(_("Are you sure to execute\n%s?"), target.c_str())))
   {
      // Execute script before
      rc = preActions();

      parentdlg.hide();
      
      for(int i=0; i<3; ++i)
      {
         path = pluginPath[i];
         path += target;
         if (access(path.c_str(), R_OK) == 0)
         {
            eSimpleConfigFile config(path.c_str());
            ePlugin p(0, path.c_str(), config);
            ePluginThread *pt = new ePluginThread(&p, pluginPath, (eZapPlugins *)&parentdlg);
            if(pt)
            {
               pt->start();
            }
            
            break;
         }
      }
      
      parentdlg.show();

      // Execute script after
      if(rc == 0) rc = postActions();
   }
}

/******************************************************************************/

eListBoxEntryExecute::eListBoxEntryExecute(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext, 
   XMLTreeNode *node)
   : ePPanelEntry(parentdlg, listbox, name, helptext, node)
{
   target=node->GetAttributeValue("target");
}

void eListBoxEntryExecute::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;
   int rc = 0;

   if(confirmUser(confirmation, 
      eString().sprintf(_("Are you sure to execute\n%s?"), target.c_str())))
   {
      // Execute script before
      rc = preActions();

      ExecuteOutput dlg(getText(), target);
      parentdlg.hide();
      dlg.show();
      dlg.exec();
      dlg.hide();
      parentdlg.show();

      // Execute script after
      if(rc == 0) rc = postActions();
   }
}

/******************************************************************************/

eListBoxEntryTarball::eListBoxEntryTarball(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString extendedName, 
   eString helptext, 
   XMLTreeNode *node, 
   bool installed)
   : ePPanelEntry(parentdlg, listbox, extendedName, helptext, node, installed)
{
   url=descramble((eString)node->GetAttributeValue("url"));
   packageType=node->GetAttributeValue("packageType");
   action=node->GetAttributeValue("action");
   package=node->GetAttributeValue("package");
   version=node->GetAttributeValue("version");
   
   // If no package is specified, take the menu text as package name      
   if(package.length() == 0)                                              
   {                                                                      
      package = name;                                  
   }                                                                      

   // Default actions
   if(action.length() == 0)
   {
      action="upgrade";
   }
   
   if(packageType.length() == 0)
   {
      packageType="var";
   }
	
	if(version.length() == 0)
	{
		version="latest";
	}   
}

void eListBoxEntryTarball::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;
   eString status;
   eString command;
   eString installLocation;
   eString newPackage = package;
   bool addPackage = true;
   bool showAsInstalled = true;
   bool alreadyInstalled = false;
   int rc = 0;

   if(confirmUser(confirmation, 
      eString().sprintf(_("Are you sure to install\n%s?"), getText().c_str())))
   {
      // Execute script before
      rc = preActions();

      if(packageType == "set")
      {
         installLocation = "var";
         
         // Don't add settings to package manager
         addPackage = false;
         showAsInstalled = false;
      }
      else
      {
         installLocation = packageType;
      }

      if(action == "install")
      {
         // Installs need a check before if the package is not installed

         InstalledPackages::PackageInfo foundPackage;
         foundPackage = installedPackages.findPackage(newPackage);
         alreadyInstalled = (foundPackage.name != "");
      }

      if(!alreadyInstalled)
      {
         command = "mkdir -p /" + installLocation + "/tmp/TempDirForInstaller";

         // Check for which kind of install
         if(url.left(1) == "/")
         {
            // Manual install
         	systemFixed(command.c_str());

            eListBoxEntryTarballManual dlg(url);
            parentdlg.hide();
            dlg.show();
            rc = dlg.exec();
            dlg.hide();
            parentdlg.show();

            // Retrieve selected package name
            newPackage = dlg.selectedPackage;
            
            // Do not show manual installs as installed
            showAsInstalled = false;
         }
         else if((url.left(6) == "ftp://") || (url.left(7) == "http://"))
         {
            // Download from internet
         	systemFixed(command.c_str());
            
				eString postfix;
            
            postfix = (url.right(4) == ".ipk") ? "ipk" : "tgz";
            command = "/" + installLocation + "/tmp/TempDirForInstaller/tmp." + postfix;
            rc = downloadFile(parentdlg, command, url,
               eString().sprintf(_("Downloading %s"), getText().c_str()));
         }
			else
			{
				// IPK feed install
				packageType = "ipk";
				action = "ipkfeedinstall";
			}

         if(rc == 0)
         {
           bool ipk = false;
           if (url.right(4) == ".ipk")
           {
             ipk = true;
           }

           rc = installAddon(this, parentdlg, newPackage, version, packageType,
			     ipk ? ("ipk"+action) : action, url, addPackage, showAsInstalled);
         }
      
         if (rc == 0 && packageType == "set")
         {
            systemFixed("touch /tmp/reloadUserBouquets");
         }
      }
      else
      {
         eMessageBox::ShowBox(
            _("The package cannot be installed because it's already installed. Please remove the previous version first."),
            _("Installing not possible"), eMessageBox::btOK|eMessageBox::iconInfo);
      }

      // Execute script after
      if(rc == 0) rc = postActions();
   }
}

/******************************************************************************/

eListBoxEntryTarballManual::eListBoxEntryTarballManual(eString directory)
   : BaseWindow(_("Manual Install")), 
   installDirectory(directory)
{
   FILE* F;
   char line[256];
   eString command;
   eListBox<eListBoxEntryMenu> *_list = (eListBox<eListBoxEntryMenu>*)&list;
   eString _helptext = _("Select package to install");

   _list->setText(_("Manual Install"));

   command = "cd "+installDirectory+"; ls *.tar.gz *.ipk";
   F = popen(command.c_str(), "r");

   if (F)
   {
      while (fgets (line, 256, F) != NULL)
      {
         new eListBoxEntryMenu((eListBox<eListBoxEntryMenu>*)_list, line, _helptext);
      }
      pclose (F);
   }
   CONNECT (list.selected, eListBoxEntryTarballManual::doInstall);
}

void eListBoxEntryTarballManual::doInstall(eListBoxEntryMenu *item)
{
   if(item)
   {
      eString command;
      eString package;

      package = item->getText();
      package = package.left(package.length()-1);

      if(package.right(4) == ".ipk")
      {
         // ipkg package
         command = "cp \""+installDirectory+"/"+package+"\" /var/tmp/TempDirForInstaller/tmp.ipk; "+
            "rm -f \""+installDirectory+"/"+package+"\"";
         systemFixed(command.c_str());
         
         package.replace(package.find(".ipk"), 4, "");
      }
      else
      {
         // tarball package
         command = "cp \""+installDirectory+"/"+package+"\" /var/tmp/TempDirForInstaller/tmp.tgz; "+
            "rm -f \""+installDirectory+"/"+package+"\"";
         systemFixed(command.c_str());

         package.replace(package.find(".tar.gz"), 7, "");
      }
      
      selectedPackage = package;
      close(0);
   }
}

/******************************************************************************/

eListBoxEntryComment::eListBoxEntryComment(
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name)
   : eListBoxEntryMenu(listbox, name)
{
   selectable = 0;
}

/******************************************************************************/

eListBoxEntryRemove::eListBoxEntryRemove(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext, 
   XMLTreeNode *node)
   : ePPanelEntry(parentdlg, listbox, name, helptext, node)
{
}

void eListBoxEntryRemove::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;
   eListBoxEntryRemoveScreen dlg(confirmation);
   parentdlg.hide();
   dlg.show();
   dlg.exec();
   dlg.hide();
   parentdlg.show();
}

/******************************************************************************/

eListBoxEntryRemoveScreen::eListBoxEntryRemoveScreen(const eString &confirm)
   : BaseWindow(_("Remove Software")),
   confirmation(confirm)
{
   eListBox<eListBoxEntryText> *_list = (eListBox<eListBoxEntryText>*)&list;
   eString _helptext = _("Select package to remove");

   _list->setText(_("Remove Software"));

	installedPackages.reload();

   for(std::vector<InstalledPackages::PackageInfo>::iterator i=installedPackages.packages.begin(); 
      i!=installedPackages.packages.end(); ++i)
   {
      new eListBoxEntryText(_list, i->name, 0, 0, _helptext);
   }
   
   CONNECT (list.selected, eListBoxEntryRemoveScreen::doRemove);
}

void eListBoxEntryRemoveScreen::doRemove(eListBoxEntryMenu * item)
{
   eString package;
   
   if(item)
   {
      package = item->getText();
      
      if(confirmUser(confirmation, 
         eString().sprintf(_("Are you sure to remove\n%s?"), package.c_str())))
      {
         eString command;

         command.sprintf("install.sh remove 'var_%s' x",
            package.c_str());

         eString title;
         title.sprintf(_("Removing %s"), package.c_str());
         ExecuteOutput dlg(title, command);
         hide();
         dlg.show();
         dlg.exec();
         dlg.hide();
         show();
         
         installedPackages.removePackage(package);
         list.remove(item, false);
      }
   }
}

/******************************************************************************/

eListBoxEntryMedia::eListBoxEntryMedia(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext,
   XMLTreeNode *node)
   : ePPanelEntry(parentdlg, listbox, name, helptext, node)
{
   target = descramble((eString)node->GetAttributeValue("target"));
   stream = node->GetAttributeValue("stream");
}

void eListBoxEntryMedia::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;
   int rc = 0;
   eString downloadFilename;
   bool removeAfterwards = false;

   if(confirmUser(confirmation, 
      eString().sprintf(_("Are you sure to play\n%s?"), target.c_str())))
   {
      // Execute script before
      rc = preActions();
      
      if((target.left(1) == "/") || (stream == "true"))
      {
         // It's a file or a mp3 stream
         downloadFilename = target;
      }
      else
      {
         // It's a URL, download it
         unsigned int pos = target.find_last_of(".");
         if(pos == eString::npos)
         {
            pos = target.length()-1;
         }
         downloadFilename = "/tmp/media" + target.substr(pos);

         rc = downloadFile(parentdlg, downloadFilename, target,
            eString().sprintf(_("Downloading %s"), getText().c_str()));
         removeAfterwards = true;
      }

		if(rc == 0)
		{
			if(downloadFilename.right(4) == ".txt")
			{
				// Just show text files
				eString command = "cat " + downloadFilename;
				ExecuteOutput dlg(name, command);  
				parentdlg.hide();                                                                     
				dlg.show();                                                                 
				dlg.exec();                                                                 
				dlg.hide();                                                                 
				parentdlg.show();      
			}
			else
			{
				eServiceFileHandler *handler = eServiceFileHandler::getInstance();
				eServiceReference serverRef;
				handler->lookupService(serverRef, downloadFilename.c_str());
      
				if(serverRef.type == 0x2000)
				{
					parentdlg.hide();
					ePictureViewer e(serverRef.path);
					e.show();
					e.exec();
					e.hide();
					parentdlg.show();
				}
				else
				{
					eZapMain *ez = eZapMain::getInstance();
					ez->playService(serverRef, eZapMain::psDontAdd);
				}
			}
		}

      // Execute script after
      if(rc == 0) rc = postActions();

      // Clean up temporary file
      if(removeAfterwards)
      {
         unlink(downloadFilename.c_str());
      }
   }
}

/******************************************************************************/

eListBoxEntryConfigWrite::eListBoxEntryConfigWrite(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext,
   XMLTreeNode *node)
   : ePPanelEntry(parentdlg, listbox, name, helptext, node)
{
   target = node->GetAttributeValue("target");
   value = node->GetAttributeValue("value");
}

void eListBoxEntryConfigWrite::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;
   int rc = 0;

   if(confirmUser(confirmation, 
      eString().sprintf(_("Are you sure to write\n%s?"), target.c_str())))
   {
      // Execute script before
      rc = preActions();
		
		const char *strippedTarget = &(target.c_str()[2]);
		eConfig* config = eConfig::getInstance();
		
		switch(target[0])
		{
			case 'i':
				config->setKey(
					strippedTarget,
					(int)strtoul(value.c_str(), 0, 0x10));
				break;
			
			case 'u':
				config->setKey(
					strippedTarget,
					(unsigned int)strtoul(value.c_str(), 0, 0x10));
				break;
			
			case 'd':
				config->setKey(
					strippedTarget,
					strtod(value.c_str(), 0));
				break;
			
			case 's':
				config->setKey(
					strippedTarget,
					value.c_str());
				break;
			
			default:
				break;
		}

      // Execute script after
      if(rc == 0) rc = postActions();
   }
}

/******************************************************************************/

eListBoxEntryEnigmaMenu::eListBoxEntryEnigmaMenu(
   PPanel &parentdlg, 
   eListBox<eListBoxEntryMenu> *listbox, 
   eString name, 
   eString helptext, 
   XMLTreeNode *node)
   : ePPanelEntry(parentdlg, listbox, name, helptext, node)
{
   target=node->GetAttributeValue("target");
}

void eListBoxEntryEnigmaMenu::LBSelected(eListBoxEntry* t)
{
	if (t != this) return;
   int rc = 0;

   if(confirmUser(confirmation, 
      eString().sprintf(_("Are you sure to open\n%s?"), target.c_str())))
   {
      // Execute script before
      rc = preActions();
		
		parentdlg.hide();
		eCallableMenuFactory::showMenu(target, parentdlg.LCDTitle, parentdlg.LCDElement);
		parentdlg.show();

      // Execute script after
      if(rc == 0) rc = postActions();
   }
}

/******************************************************************************/

BaseWindow::BaseWindow(eString title)
   : eListBoxWindow<eListBoxEntryMenu>(title, 13, 410, true)
{
	valign();
}

void BaseWindow::setError(const eString& errmsg)
{
   if(errmsg.length())
   {
      eMessageBox::ShowBox(errmsg, _("Error!"), 
         eMessageBox::btOK|eMessageBox::iconError);
   }
}

/******************************************************************************/

PPanel::PPanel(const eString &xmlFile) 
	: BaseWindow("PPanel"),
	directory(0)
{
	FILE* fh;
	XMLTreeNode *node = 0;
	
	// Check if file is there
	if(access(xmlFile.c_str(), R_OK) != 0)
	{
		// Automatic PPanel recovery
		recover(xmlFile);                                             
	}
         
	fh = fopen(xmlFile.c_str(), "r");
	if(fh)
	{
		node = loadFile(fh);
		if(!node)
		{
			// File is corrupt, automatic PPanel recovery
			recover(xmlFile);                                             
           
			// Second try
			if(fh)
			{
				fclose(fh);
			}
				
			fh = fopen(xmlFile.c_str(), "r");
			node = loadFile(fh);                                                                 
		}
	}
	else
	{                                                                
		// File not found                                             
		setError("Cannot open " + xmlFile);                                
	}
		                                                              
	if(fh)
  	{
     	fclose(fh);
  	}
	
	loadItems(node);
	CONNECT(list.selected, PPanel::itemSelected);
}

PPanel::PPanel(XMLTreeNode *node)
	: BaseWindow("PPanel"),
	directory(0)
{
	loadItems(node);
	CONNECT(list.selected, PPanel::itemSelected);
}

PPanel::~PPanel()
{
   if(directory) delete directory;
}

void PPanel::runPPanel(const eString &xmlFile, eWidget* lcdTitle, eWidget* lcdElement)
{
	int rc = PPANELRESTARTCODE;

	while(rc == PPANELRESTARTCODE)
	{
		PPanel *myPPanel = new PPanel(xmlFile);
#ifndef DISABLE_LCD
		myPPanel->setLCD(lcdTitle, lcdElement);
#endif
		myPPanel->show();
		rc = myPPanel->exec();
		myPPanel->hide();
		delete myPPanel;
	}
}

void PPanel::runPPanel(XMLTreeNode *node, eWidget* lcdTitle, eWidget* lcdElement)
{
	int rc = PPANELRESTARTCODE;

	while(rc == PPANELRESTARTCODE)
	{
		PPanel *myPPanel = new PPanel(node);
#ifndef DISABLE_LCD
		myPPanel->setLCD(lcdTitle, lcdElement);
#endif
		myPPanel->show();
		rc = myPPanel->exec();
		myPPanel->hide();
		delete myPPanel;
	}
}

eString PPanel::getPPanelName(const eString &xmlFile)
{
	eString ppanelName = xmlFile;
	char line[500];                                                     
	int len;                                                            
	bool error = false;                                                 

	XMLTreeParser* dir = new XMLTreeParser("ISO-8859-1");
	FILE* fh = fopen(xmlFile.c_str(), "r");                

	if(fh && dir)
	{
		while(true)                                                           
		{                                                                   
	   	len = fread(line, 1, sizeof(line), fh);                          
		   if (!dir->Parse(line, len, (len < (signed)sizeof(line))))  
		   {                                                                
	   	   error = true;                                                 
	      	break;                                                        
		   }
                                                                      
		   if (len < (signed)sizeof(line))
      	{
         	break;
	      }                        
		}

		if(!error)                                                          
		{                                                                   
	   	XMLTreeNode* rootnode = dir->RootNode();
			if(rootnode)
			{
				char* name = rootnode->GetAttributeValue("name");
				if(name)
				{
					ppanelName = name;
				}
			}
		}
		
		fclose(fh);
		delete dir;
	}                                                                  

   return ppanelName;
}

void PPanel::recover(const eString &xmlFile)
{
	eString command = "ppanelrecovery.sh " + xmlFile;                                  
	ExecuteOutput dlg(_("PPanel recovery, internet connection needed"), command);         
	hide();                                                                            
	dlg.show();                                                                        
	dlg.exec();                                                                        
	dlg.hide();                                                                        
	show();                                                                            
}

XMLTreeNode* PPanel::loadFile(FILE *fh)
{
   XMLTreeNode *node = 0;
	char line[500];                                                     
	int len;                                                            
	bool error = false;                                                 

	if (directory)                                                      
	{                                                                   
	   delete directory;                                                
	}                                                                   
	directory = new XMLTreeParser("ISO-8859-1");                        

	while(true)                                                           
	{                                                                   
	   len = fread(line, 1, sizeof(line), fh);                          
	   if (!directory->Parse(line, len, (len < (signed)sizeof(line))))  
	   {                                                                
	      error = true;                                                 
	      break;                                                        
	   }
                                                                      
	   if (len < (signed)sizeof(line))
      {
         break;
      }                        
	}
                                                                      
	if (error)                                                          
	{                                                                   
	   // Parse error                                                   
	   eString errorstring;                                             

	   errorstring.sprintf("XML parse error: %s at line %d",            
	   directory->ErrorString(directory->GetErrorCode()),               
	   directory->GetCurrentLineNumber());                              
	   setError(errorstring.c_str());                                   
	}                                                                   
	else                                                                
	{                                                                   
	   node = directory->RootNode();                                 
	}                                                                   

   return node;
}

void PPanel::loadItems(XMLTreeNode *category)
{
   if (category)
   {
      eString title = category->GetAttributeValue("name");
      if(title.length() > 0)
      {
         setText(title);
      }
		
		eString lock = category->GetAttributeValue("lock");
		if(lock == "true")
		{
			if (!pinCheck::getInstance()->checkPin(pinCheck::setup))
			{
				// Too bad I can't send an event here to close the BaseWindow
				return;
			}
		}
		
      list.beginAtomic();
      list.clearList();

      for (XMLTreeNode *r=category->GetChild(); r; r=r->GetNext())
      {
         // Check if entry should be displayed or not
         eString condition = r->GetAttributeValue("condition");
         if (condition.length() > 0)
         {
            if(systemFixed(condition.c_str())) continue;
         }
         
         if (!strcmp(r->GetType(), "separator"))
         {
            new eListBoxEntryMenuSeparator(&list, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
            continue;
         }
         else if (!strcmp(r->GetType(), "update"))
         {
            if (access("/var/etc/ppanelautodownload", R_OK) == 0)
            {
               eString url = r->GetAttributeValue("url");
               eString target = r->GetAttributeValue("target");
               updatePPanel(url, target);
            }
            continue;
         }

         const char *name = r->GetAttributeValue("name");
         if (!name)
            continue;
         
         eString helptext;   
         const char *helptmp = r->GetAttributeValue("helptext");
         helptext = helptmp ? helptmp : _("Please select an item");

         if(!strcmp(r->GetType(), "category") ||
				!strcmp(r->GetType(), "ppanel"))
			   // category is basically the same as ppanel
				// except that ppanel has a target and category a node
            new eListBoxEntryPPanel(*this, &list, name, helptext, r);
         else if (!strcmp(r->GetType(), "file"))
            new eListBoxEntryFile(*this, &list, name, helptext, r);
         else if (!strcmp(r->GetType(), "tarball"))
         {
            eString extendedName = (eString)name+" (";
            const char *version = r->GetAttributeValue("version");
            const char *package = r->GetAttributeValue("package");
            bool packageInstalled = false;
            
            if(version)
            {
               extendedName += (eString)version+")";
            }
            else
            {
               extendedName += (eString)_("latest")+")";
            }
            
            if(package)
            {
               // Search for package name in installed packages
               packageInstalled = (installedPackages.findPackage((eString)package).name.length() != 0);
            }
            else
            {
               // Search for menu text in installed packages
               packageInstalled = (installedPackages.findPackage((eString)name).name.length() != 0);
            }
            
            new eListBoxEntryTarball(*this, &list, name, extendedName, helptext, r, packageInstalled);
         }
         else if (!strcmp(r->GetType(), "plugins"))
            new eListBoxEntryPlugins(*this, &list, name, helptext, r);
         else if (!strcmp(r->GetType(), "plugin"))
            new eListBoxEntryPlugin(*this, &list, name, helptext, r);
         else if (!strcmp(r->GetType(), "execute"))
            new eListBoxEntryExecute(*this, &list, name, helptext, r);
         else if (!strcmp(r->GetType(), "comment"))
            new eListBoxEntryComment(&list, name);
         else if (!strcmp(r->GetType(), "remove"))
            new eListBoxEntryRemove(*this, &list, name, helptext, r);
         else if (!strcmp(r->GetType(), "media") ||
				!strcmp(r->GetType(), "picture"))
            // picture is handled by "media" for backward compatibility
            new eListBoxEntryMedia(*this, &list, name, helptext, r);
         else if (!strcmp(r->GetType(), "configwrite"))
            new eListBoxEntryConfigWrite(*this, &list, name, helptext, r);
			else if (!strcmp(r->GetType(), "menu"))
				new eListBoxEntryEnigmaMenu(*this, &list, name, helptext, r);
      }

      setFocus(&list);
      list.endAtomic();
   }
}

void PPanel::itemSelected(eListBoxEntryMenu *item)
{
   const char* ppr = "/tmp/ppanelrefresh";
   if (access(ppr, R_OK) == 0)
   {
      eMessageBox::ShowBox(
         _("A new menu is available. The new menu will be visible if you return to the setup panel."
         "You can continue your work here after pressing OK."),
         _("Automatic menu update"), eMessageBox::btOK|eMessageBox::iconInfo);

      unlink(ppr);
   }
}

void PPanel::updatePPanel(eString &url, eString &target)
{
   eString command = "ppanelupdate.sh " + url + " " + target;
   
   updateRunner = new eConsoleAppContainer(command);
   CONNECT(updateRunner->appClosed, PPanel::runnerClosed);
}

void PPanel::runnerClosed(int state)
{
   if(updateRunner) delete updateRunner;
}

/******************************************************************************/

#ifdef EHTTPDOWNLOAD

DownloadIndicator::DownloadIndicator(
   eString caption, 
   eString message, 
   eString filename, 
   eString url)
   : eWindow(1),
   filename(filename), url(url), http(0)
{
   setText(caption);
   resize(eSize(500, 140));
	move(ePoint((parent->width()-size.width())/2, 130));

   downloadtext = new eLabel(this);
   downloadtext->setText(message);
   downloadtext->resize(eSize(clientrect.width(), clientrect.height()));
   downloadtext->move(ePoint(10, 10));
   downloadtext->show();

	progresstext = new eLabel(this);
	progresstext->setName("progresstext");
   progresstext->resize(eSize(clientrect.width(), clientrect.height()));
   progresstext->move(ePoint(10, 30));
   progresstext->setText("Waiting...");
	progresstext->show();
	
	progress = new eProgress(this);
	progress->setName("progress");
   progress->move(ePoint(10, 60));
   progress->resize(eSize(eSize(clientrect.width()-20, 20)));
	progress->show();
	
   startDownload(url.c_str(), filename.c_str());
}

DownloadIndicator::~DownloadIndicator()
{
	if(http)
	{
		delete http;
		http = 0;
	}
}

void DownloadIndicator::startDownload(const char *url, const char *filename)
{
	if(http)
   {
		delete http;
   }
   
	http = eHTTPConnection::doRequest(url, eApp, &error);

	if(!http)
	{
		downloadDone(error);
	}
   else
	{
		CONNECT(http->transferDone, DownloadIndicator::downloadDone);
		CONNECT(http->createDataSource, DownloadIndicator::createDownloadDataSink);
		http->local_header["User-Agent"]="PLi-download/1.0.0";
		http->start();
	}
}

void DownloadIndicator::downloadDone(int err)
{
   sleep(1);
   
	if(err || !http || http->code != 200)
   {
		close(1);
   }
	else
   {
		close(0);
   }
   
	http = 0;
}

eHTTPDataSource *DownloadIndicator::createDownloadDataSink(eHTTPConnection *conn)
{
	download = new eHTTPDownload(conn, filename.c_str());
	lasttime = 0;
	CONNECT(download->progress, DownloadIndicator::downloadProgress);
	return download;
}

void DownloadIndicator::downloadProgress(int received, int total)
{
	if((time(0) == lasttime) && (received != total))
   {
		return;
   }
   
	lasttime = time(0);
	if (total > 0)
	{
		eString pt;
		int perc = received*100/total;
		pt.sprintf("%d/%d kB (%d%%)", received/1024, total/1024, perc);
		progress->setPerc(perc);
		progresstext->setText(pt);
	} 
   else
	{
		eString pt;
		pt.sprintf("%d kB", received/1024);
		progress->setPerc(0);
		progresstext->setText(pt);
	}
}

#else

DownloadIndicator::DownloadIndicator(
   eString caption, 
   eString message, 
   eString filename, 
   eString url)
   : eWindow(1), updateTimer(0), downloader(0),
   originalText(message), filename(filename), url(url)
{
   setText(caption);
	int posx = eSkin::getActive()->queryValue("eMessageBox.pos.x", 100);
	int posy = eSkin::getActive()->queryValue("eMessageBox.pos.y", 70);
   move(ePoint(posx, posy));
   resize(eSize(450, 430));
   
   etext = new eLabel(this);
   // Set largest text expected
   etext->setText(eString().sprintf(_("%s\n%d bytes downloaded\nPress exit to cancel download"),
      originalText.c_str(), 99999999));      
   etext->resize(eSize(clientrect.width(), clientrect.height()));
   etext->setFlags(RS_WRAP | eLabel::flagVCenter);

	eSize txtSize = etext->getExtend();
	txtSize += eSize(15, 10);
	etext->resize(txtSize);
   etext->setText(originalText);
   etext->move(ePoint(20, 20));

	eSize ext;
   ext.setWidth(etext->getPosition().x() + etext->getSize().width() + 20);
   ext.setHeight(etext->getPosition().y() + etext->getSize().height() + 20);
   if(ext.width()<150)
   {
      ext.setWidth(150);
   }
   cresize(ext);

   downloadDone = false;
   updateTimer = new eTimer(eApp);                                         
}
 
DownloadIndicator::~DownloadIndicator()
{
   if(updateTimer)                   
   {     
      updateTimer->stop();                            
      delete updateTimer;            
      updateTimer = 0;               
   }                                 
                                  
   if(downloader)                    
   {                                 
      if(downloader->running())      
      {                              
         downloader->kill();         
      }                              
      delete downloader;             
      downloader = 0;                
   }                                 
}

int DownloadIndicator::exec()
{
   eString command;

   command = "wget -O \"" + filename + "\" \"" + url + "\"";      
   downloader = new eConsoleAppContainer(command);                
   if(downloader)                                                                
   {                                                                             
      if(downloader->running())                                                  
      {                                                                          
         CONNECT(downloader->appClosed, DownloadIndicator::downloadClosed); 
      }
   }
        
   if(updateTimer)                                                               
   {                                                                             
      CONNECT(updateTimer->timeout, DownloadIndicator::update);                  
      updateTimer->start(1000, true);                                            
   }
                                                                               
   return eWindow::exec();
}

int DownloadIndicator::eventHandler(const eWidgetEvent &e)
{
   switch(e.type)
   {
      case eWidgetEvent::evtAction:
         if(e.action == &i_cursorActions->cancel)
         {
            updateTimer->stop();
            close(1);
         }
         break;
         
      default:
         return eWindow::eventHandler(e);
   }
   
   return 1;
}

void DownloadIndicator::update(void)
{
   struct stat fst;
   eString newText;
   
   if(stat(filename.c_str(), &fst))
   {
      fst.st_size = (off_t)0;
   }
   
   if(fst.st_size)
   {
      newText.sprintf(_("%s\n%d bytes downloaded\nPress exit to cancel download"), originalText.c_str(), fst.st_size);
      etext->setText(newText);
   }   

   if(downloadDone)
   {
      // Wait one second to show download size and then close window
      sleep(1);
      close(0);
   }
   else
   {   
      updateTimer->start(1000, true);
   }
}

void DownloadIndicator::downloadClosed(int state)
{
   downloadDone = true;
}

#endif // EHTTPDOWNLOAD

/******************************************************************************/

InstalledPackages::InstalledPackages()
{
	reload();
}

void InstalledPackages::reload()
{
   FILE *F;
   char line[256];
   
	packages.clear();
   systemFixed("ls /var/tuxbox/installer >/var/tmp/installed");

   F = fopen("/var/tmp/installed", "r");
   if(F)
   {
      while(fgets(line, 256, F) != NULL)
      {
         line[strlen(line)-1] = '\0';
         addPackage(PackageInfo(line));
      }
      fclose(F);
   }
   
   unlink("/var/tmp/installed");
}

void InstalledPackages::addPackage(PackageInfo packageInfo)
{
   packages.push_back(packageInfo);
   #ifdef PPANELDEBUG
   eDebug("InstalledPackages::addPackage");
   eDebug(packageInfo.name.c_str());
   #endif
}

void InstalledPackages::removePackage(const eString& packageName)
{
   for(std::vector<PackageInfo>::iterator i=packages.begin(); i!=packages.end(); ++i)
   {
      if(i->name == packageName)
      {
         #ifdef PPANELDEBUG
         eDebug("InstalledPackages::removePackage");
         eDebug(packageName.c_str());
         #endif
         packages.erase(i);
         break;
      }
   } 
}

InstalledPackages::PackageInfo InstalledPackages::findPackage(const eString& packageName)
{
   PackageInfo packageInfo;
   
   #ifdef PPANELDEBUG
   eDebug("InstalledPackages::findPackage");
   eDebug(packageName.c_str());
   #endif
   for(std::vector<PackageInfo>::iterator i=packages.begin(); i!=packages.end(); ++i)
   {
      #ifdef PPANELDEBUG
      eDebug(i->name.c_str());
      #endif
      if(i->name == packageName)
      {
         packageInfo = *i;
         #ifdef PPANELDEBUG
         eDebug("InstalledPackages::findPackage FOUND");
         #endif
         break;
      }
   }
   
   return packageInfo;
}

/******************************************************************************/
// END OF FILE
/******************************************************************************/
