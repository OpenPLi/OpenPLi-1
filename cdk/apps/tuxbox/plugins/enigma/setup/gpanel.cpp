/*
 *  GPanel
 *
 *  Copyright (C) dAF2000
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
 */

#include <stdio.h>
#include <plugin.h>
#include <xmltree.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/emessage.h>

#include "gpanel.h"

#ifdef COMPILEASPLUGIN
extern "C" int plugin_exec(PluginParam *par);

int plugin_exec(PluginParam *par)
{
   int rc = GPANELRESTARTCODE;
   
   while(rc == GPANELRESTARTCODE)
   {
      GPanel *myGPanel =  new GPanel;
      myGPanel->show();   
      rc = myGPanel->exec();   
      myGPanel->hide();
      delete myGPanel;
   }

   return 0;
}
#endif

//-----------------------------------------------------------------------------

ConfigReader::ConfigReader(const eString &filename)
   :filename(filename)
{
   const int LINE_SIZE = 256;
   char line[LINE_SIZE+1];
   char _variable[64];
   char _value[128];
   eString variable, value;
   
   FILE *fp = fopen(filename.c_str(), "r");
   if(fp)
   {
      while(fgets(line, LINE_SIZE, fp) != NULL)
      {
         if(sscanf(line, "%[^=]%*[=]%[^\n]", _variable, _value))
         {
            variable = _variable;
            value = _value;
            addConfig(variable, value);
         }
      }
      fclose(fp);
   }
   else
   {
      // ERROR: Cannot open file
   }
}

ConfigReader::~ConfigReader()
{
}

void ConfigReader::getConfig(eString &variable, eString *value)
{
   if(variable.length())
   {   
      bool found = false;
      
      for(unsigned int i=0; i<config.size(); ++i)
      {
         if(config[i].variable == variable)
         {
            if(value) *value = config[i].value;
            found = true;
            break;
         }
      }
   
      if(!found)
      {
         // Entry not found, add it to the config file
         *value = "0";
         addConfig(variable, *value);
      }
   }
}

void ConfigReader::setConfig(eString &variable, eString &value)
{
   for(unsigned int i=0; i<config.size(); ++i)
   {
      if(config[i].variable == variable)
      {
         if(value.length()) config[i].value = value;
         break;
      }
   }
}

void ConfigReader::addConfig(eString &variable, eString &value)
{
   ConfigItem configItem;

   configItem.variable = variable;         
   configItem.value = value;
   config.push_back(configItem);                     
}

void ConfigReader::saveConfig(void)
{
   FILE *fp = fopen(filename.c_str(), "w");
   if(fp)
   {
      for(unsigned int i=0; i<config.size(); ++i)
      {
         fprintf(fp, "%s=%s\n", config[i].variable.c_str(), config[i].value.c_str());
      }
      fclose(fp);
   }
   else
   {
      // ERROR: Cannot open file
   }
}

eString ConfigReader::evaluateValue(eString &value)
{
   eString retValue = value;
   
   if(value.length())
   {
      if(value.left(1) == "$")
      {
         eString _value = value.mid(1);
         
         for(unsigned int i=0; i<config.size(); ++i)
         {
            if(config[i].variable == _value)
            {
               retValue = config[i].value;
               break;
            }
         }
      }
   }
   
   return retValue;
}

//-----------------------------------------------------------------------------

GPanel::GPanel(const eString &xmlFile, XMLTreeNode *node)
   :eWindow(1),
   form(0), configReader(0)
{
   if(!node)
   {
      char line[100];
      eString data;

      FILE *fh = fopen(xmlFile.c_str(), "r");

      if(fh == 0)
      {
         // File not found
         eString errorstring;
         errorstring.sprintf("File %s not found.", xmlFile.c_str());
         
         eMessageBox box(errorstring, _("Error!"),
            eMessageBox::btOK|eMessageBox::iconError);
         box.show();
         box.exec();
         box.hide();
         return;
      }

      while(fgets(line, 100, fh) != NULL)
         data += line;

      fclose(fh);

      int len = strlen(data.c_str());

      if(form) delete form;
      form = new XMLTreeParser("ISO-8859-1");

      if(!form->Parse(data.c_str(), len, !(data.c_str())))
      {
         // Parse error
         eString errorstring;
         errorstring.sprintf("XML parse error: %s at line %d",
            form->ErrorString(form->GetErrorCode()),
            form->GetCurrentLineNumber());
         
         eMessageBox box(errorstring, _("Error!"),
            eMessageBox::btOK|eMessageBox::iconError);
         box.show();
         box.exec();
         box.hide();
      }
      else
      {
         node = form->RootNode();
         if(!node)
         {
            eMessageBox box("XML parse error", _("Error!"),
               eMessageBox::btOK|eMessageBox::iconError);
            box.show();
            box.exec();
            box.hide();
         }
      }
   }

   if(node)
   {
      loadItems(node);
   }
}

GPanel::~GPanel()
{
   if(form) delete form;
   if(configReader) delete configReader;
}

void GPanel::loadItems(XMLTreeNode *screen)
{
   if(screen)
   {
      for(XMLTreeNode *r=screen->GetChild(); r; r=r->GetNext())
      {
         const char *type = r->GetType();
         const char *name = r->GetAttributeValue("name");
         const char *data = r->GetData();

         if(!strcmp(type, "eWindow"))
         {
            if(configReader)
            {
               windowEntry = new eWindowEntry(this, name, r, configReader);
               windowEntry->createWidget();
               windowEntry->updateAllWidgets();
            }
         }
         else if(!strcmp(type, "configFile"))
         {
            configReader = new ConfigReader(data);
         }
	      else if(!strcmp(type, "execute"))
         {
            system(data);
         }
      }
   }
}

//-----------------------------------------------------------------------------

eObjectEntry::eObjectEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :name(name), node(node), parent(parent), conf(conf), widget(NULL)
{
   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())
   {
      const char *type = r->GetType();
      
      if(!strcmp(type, "onSelected"))
      {
         parseActions(r);
      }
      else if(!strcmp(type, "condition"))
      {
	      parseCondition(r);
      }
   }
}

void eObjectEntry::parseActions(XMLTreeNode *node)
{
   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())      
   {                                                                 
      const char *type = r->GetType();
      const char *data = r->GetData();
      struct Actions action;
                                                                     
      if(!strcmp(type, "save"))                                   
      {                                                              
         action.save = true;
         actions.push_back(action);                         
      }                                                              
      else if(!strcmp(type, "quit"))                              
      {                                                              
         action.quit = true;                              
         actions.push_back(action);                         
      }
      else if(!strcmp(type, "reload"))                              
      {                                                              
         action.reload = true;                              
         actions.push_back(action);                         
      }
      else if(!strcmp(type, "execute"))
      {
         action.execute = data;
         actions.push_back(action);                         
      }
      else if(!strcmp(type, "gpanel"))
      {
         action.gpanel = data;
         actions.push_back(action);                         
      }
   }                                                                 
}

void eObjectEntry::parseCondition(XMLTreeNode *node)
{
   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())      
   {
      const char *type = r->GetType();
      
      if(!strcmp(type, "widget"))
      {
         const char *name = r->GetAttributeValue("name");
         const char *compare = r->GetAttributeValue("compare");
         const char *value = r->GetAttributeValue("value");
         
         condition.name = name;
         if(!strcmp(compare, "=="))
         {
            condition.compare = EQUAL;
         }
         else if(!strcmp(compare, "!="))
         {
            condition.compare = NOT_EQUAL;
         }
         else if(!strcmp(compare, ">="))
         {
            condition.compare = GREATER_EQUAL;
         }
         else if(!strcmp(compare, "<"))
         {
            condition.compare = LESS;
         }
         condition.value = value;
      }
   }                                                         
}

void eObjectEntry::executeActions(void)
{
   for(unsigned int i=0; i<actions.size(); ++i)
   {
      if(actions[i].save)
      {
         conf->saveConfig();
      }

      if(actions[i].execute.length())
      {
         system(actions[i].execute.c_str());
      }
   
      if(actions[i].gpanel.length())
      {
         int rc = GPANELRESTARTCODE;
   
         while(rc == GPANELRESTARTCODE)
         {
            GPanel *myGPanel =  new GPanel(actions[i].gpanel);
            myGPanel->show();   
            rc = myGPanel->exec();   
            myGPanel->hide();
            delete myGPanel;
         }
      }
   
      if(actions[i].quit)
      {
         parent->accept();         
      }
   
      if(actions[i].reload)
      {
         parent->close(GPANELRESTARTCODE);         
      }
   }
}

void eObjectEntry::executeCondition(void)
{
   if(condition.name.length())
   {
      eString value;
      bool toBeShowed = false;
   
      conf->getConfig(condition.name, &value);
      switch(condition.compare)
      {
         case EQUAL:
            toBeShowed = (value == condition.value);
            break;
            
         case NOT_EQUAL:
            toBeShowed = !(value == condition.value);
            break;
            
         case GREATER_EQUAL:
            toBeShowed = (atoi(value.c_str()) >= atoi(condition.value.c_str()));
            break;
            
         case LESS:
            toBeShowed = (atoi(value.c_str()) < atoi(condition.value.c_str()));
            break;
      }

      if(widget)
      {   
         if(toBeShowed)
         {
            widget->show();
         }
         else
         {
            widget->hide();
         }
      }
   }
}

//-----------------------------------------------------------------------------

eWindowEntry::eWindowEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eWidgetEntry(parent, name, node, conf),
   windowWidget(parent)
{
   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())
   {
      const char *type = r->GetType();
      const char *_name = r->GetAttributeValue("name");
      
      if(!strcmp(type, "eLabel"))
      {                          
         eLabelEntry *child = new eLabelEntry(parent, _name, r, conf);
         child->createWidget();
         childs.push_back(child);
      }     
      else if(!strcmp(type, "eButton"))                         
      {                          
         eButtonEntry *child = new eButtonEntry(parent, _name, r, conf);
         child->createWidget();
         childs.push_back(child);    
      }     
      else if(!strcmp(type, "eNumber"))                         
      {                          
         eNumberEntry *child = new eNumberEntry(parent, _name, r, conf);
         child->createWidget();
         childs.push_back(child);    
      }     
      else if(!strcmp(type, "eCheckbox"))                       
      {                          
         eCheckboxEntry *child = new eCheckboxEntry(parent, _name, r, conf);   
         child->createWidget();
         childs.push_back(child);    
      }     
      else if(!strcmp(type, "eCombobox"))                       
      {                          
         eComboboxEntry *child = new eComboboxEntry(parent, _name, r, conf);
         child->createWidget();
         childs.push_back(child);    
      }     
      else if(!strcmp(type, "eTextInputField"))                       
      {                          
         eTextInputFieldEntry *child = new eTextInputFieldEntry(parent, _name, r, conf);
         child->createWidget();
         childs.push_back(child);    
      }     
   }                                                            
}

void eWindowEntry::createWidget(void)
{
   windowWidget->setText(label);
   windowWidget->cmove(position);
   windowWidget->cresize(size);
}

void eWindowEntry::updateAllWidgets(void)
{
   for(unsigned int i=0; i<childs.size(); ++i)         
   {                                                                
      childs[i]->executeCondition();
   }                                                                
}

//-----------------------------------------------------------------------------

eListBoxEntryEntry::eListBoxEntryEntry
   (eListBox<eListBoxEntry>* listbox, GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eObjectEntry(parent, name, node, conf),
    helptext("")
{
   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())
   {
      if(!strcmp(r->GetType(), "helptext"))
      {
         const char *str = r->GetData();
         helptext = str;
      }
   }
}

//-----------------------------------------------------------------------------

eListBoxEntryTextEntry::eListBoxEntryTextEntry
   (eListBox<eListBoxEntryText>* listbox, GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf, int key)
   :eListBoxEntryEntry((eListBox<eListBoxEntry>*)listbox, parent, name, node, conf),
   label("undefined")
{
   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())
   {
      if(!strcmp(r->GetType(), "label"))
      {
         const char *str = r->GetData();
         eString tmp;
         tmp = str;
         label = conf->evaluateValue(tmp);
      }
   }

   listBoxEntryWidget = new eListBoxEntryText(listbox, label, (void *)key, 0, helptext);
}

void eListBoxEntryTextEntry::createWidget(void)
{
}

//-----------------------------------------------------------------------------

ePoint eWidgetEntry::lastPosition = ePoint(10, 10);

eWidgetEntry::eWidgetEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eObjectEntry(parent, name, node, conf),
    size(eSize(120, 40)), position(ePoint(10, 10)),
    label("undefined"), helptext(""), shortcut("")
{
   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())
   {
      const char *type = r->GetType();
      const char *str = r->GetData();
      const char *x = r->GetAttributeValue("x");
      const char *y = r->GetAttributeValue("y");
      
      if(!strcmp(type, "size"))
      {
         size = eSize(atoi(x), atoi(y));
      }
      else if(!strcmp(type, "position"))
      {
         int newX = atoi(x);
         int newY = atoi(y);
         
         if((x[0] == '+') || (x[0] == '-'))
         {
            newX = lastPosition.x() + atoi(x);
         }

         if((y[0] == '+') || (y[0] == '-'))
         {
            newY = lastPosition.y() + atoi(y);
         }

         position = ePoint(newX, newY);
         lastPosition = position;
      }
      else if(!strcmp(type, "label"))
      {
         eString tmp;
         tmp = str;
         label = conf->evaluateValue(tmp);
      }
      else if(!strcmp(type, "helptext"))
      {
         helptext = str;
      }
      else if(!strcmp(type, "shortcut"))
      {
         shortcut = str;
      }
   }
}

//-----------------------------------------------------------------------------

eDecoWidgetEntry::eDecoWidgetEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eWidgetEntry(parent, name, node, conf)
{
}

//-----------------------------------------------------------------------------

eButtonEntry::eButtonEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eLabelEntry(parent, name, node, conf)
{
}

void eButtonEntry::createWidget(void)
{
   buttonWidget = new eButton(parent);
   buttonWidget->setText(label); //eWidget
   buttonWidget->setShortcut(shortcut); //eWidget
   buttonWidget->setShortcutPixmap(shortcutPixmap); //eLabel
   buttonWidget->move(position); //eWidget
   buttonWidget->resize(size); //eWidget
   buttonWidget->loadDeco(); //eDecoWidget
   buttonWidget->show(); //eWidget
   CONNECT(buttonWidget->selected, eButtonEntry::handleSelected);
   widget = buttonWidget;
}

void eButtonEntry::handleSelected(void)
{
   executeActions();
   parent->windowEntry->updateAllWidgets();
}

//-----------------------------------------------------------------------------

eCheckboxEntry::eCheckboxEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eButtonEntry(parent, name, node, conf)
{
}

void eCheckboxEntry::createWidget(void)
{
   eString checkedStr = "0";
   
   conf->getConfig(name, &checkedStr);
   checkboxWidget = new eCheckbox(parent, atoi(checkedStr.c_str()));
   checkboxWidget->setText(label); //eWidget
   checkboxWidget->move(position); //eWidget
   checkboxWidget->resize(size); //eWidget
   checkboxWidget->loadDeco(); //eDecoWidget
   checkboxWidget->show(); //eWidget
   CONNECT(checkboxWidget->checked, eCheckboxEntry::handleChecked);
   widget = checkboxWidget;
}

void eCheckboxEntry::handleChecked(int state)
{
   eString stateStr = state ? "1":"0";
   conf->setConfig(name, stateStr);
   executeActions();
   parent->windowEntry->updateAllWidgets();
}

//-----------------------------------------------------------------------------

eComboboxEntry::eComboboxEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eButtonEntry(parent, name, node, conf),
   entries(3), flags(0)
{
   int key = 0;
   const char *str = node->GetAttributeValue("entries");
   
   if(str) entries = atoi(str);
   
   comboboxWidget = new eComboBox(parent, entries);

   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())
   {
      char *type = r->GetType();
      
      if(!strcmp(type, "eListBoxEntryText"))
      {
         new eListBoxEntryTextEntry(*comboboxWidget, parent, name, r, conf, key++);
      }
      else if(!strcmp(type, "flagShowEntryHelp"))
      {
         flags |= eComboBox::flagShowEntryHelp;
      }
      // Also: flagSorted and (flagVCenter in eLabel)
   }
}

void eComboboxEntry::createWidget(void)
{
   eString currentStr = "0";
   
   comboboxWidget->move(position); //eWidget
   comboboxWidget->resize(size); //eWidget
   comboboxWidget->loadDeco(); //eDecoWidget
   conf->getConfig(name, &currentStr);
   comboboxWidget->setCurrent(atoi(currentStr.c_str()));
   comboboxWidget->setFlags(flags);
   comboboxWidget->show(); //eWidget
   CONNECT(comboboxWidget->selchanged, eComboboxEntry::handleSelChanged);
   widget = comboboxWidget;
}

void eComboboxEntry::handleSelChanged(eListBoxEntryText *sel)
{
   if(sel)
   {
      eString selStr;
      selStr.sprintf("%d", (int)sel->getKey());
      conf->setConfig(name, selStr);
      executeActions();
      parent->windowEntry->updateAllWidgets();
   }
}

//-----------------------------------------------------------------------------

eLabelEntry::eLabelEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eDecoWidgetEntry(parent, name, node, conf)
{
   shortcutPixmap = shortcut;
}

void eLabelEntry::createWidget(void)
{
   labelWidget = new eLabel(parent);
   labelWidget->setText(label); //eWidget
   labelWidget->setShortcut(shortcut); //eWidget
   labelWidget->move(position); //eWidget
   labelWidget->resize(size); //eWidget
   labelWidget->loadDeco(); //eDecoWidget
   labelWidget->show(); //eWidget
   widget = labelWidget;
}

//-----------------------------------------------------------------------------

eNumberEntry::eNumberEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eDecoWidgetEntry(parent, name, node, conf),
    length(1), minimal(0), maximal(9), maxDigits(1), flags(0)
{
   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())
   {
      const char *type = r->GetType();
      const char *str = r->GetData();
      
      if(!strcmp(type, "length"))
      {
         length = atoi(str);
      }
      else if(!strcmp(type, "range"))
      {
         const char *minimalStr = r->GetAttributeValue("minimal");
         const char *maximalStr = r->GetAttributeValue("maximal");
         minimal = atoi(minimalStr);
         maximal = atoi(maximalStr);
      }
      else if(!strcmp(type, "maxDigits"))
      {
         maxDigits = atoi(str);
      }
      else if(!strcmp(type, "flagDrawPoints"))
      {
         flags |= eNumber::flagDrawPoints;
      }
      // Also: flagDrawBoxes, flagFillWithZeros, flagTime, flagPosNeg,
      // flagHideInput and flagFixedNum.
   }
}

void eNumberEntry::createWidget(void)
{
   int defaults[20] = {0}; // Bit stupid to do this way
   
   for(unsigned int i=0; i<(unsigned int)length; ++i)
   {
      eString _name, valueStr="0";
      _name.sprintf("%s_%d", name.c_str(), i);
      
      conf->getConfig(_name, &valueStr);
      defaults[i] = atoi(valueStr.c_str());
   }
   
   numberWidget = new eNumber(parent, length, minimal, maximal, maxDigits, defaults);
   numberWidget->move(position); //eWidget
   numberWidget->resize(size); //eWidget
   numberWidget->loadDeco(); //eDecoWidget
   numberWidget->setFlags(flags);
   numberWidget->show(); //eWidget
   CONNECT(numberWidget->selected, eNumberEntry::handleNumberChanged);
   widget = numberWidget;
}

void eNumberEntry::handleNumberChanged(int *dummy)
{
   for(unsigned int i=0; i<(unsigned int)length; ++i)
   {
      int number;
      eString _name, numberStr;
      
      number = numberWidget->getNumber(i);
      _name.sprintf("%s_%d", name.c_str(), i);
      numberStr.sprintf("%d", number);
      conf->setConfig(_name, numberStr);
   }
   
   parent->windowEntry->updateAllWidgets();
}

//-----------------------------------------------------------------------------

eTextInputFieldEntry::eTextInputFieldEntry
   (GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf)
   :eButtonEntry(parent, name, node, conf),
   maxChars(0), usableChars("")
{
   for(XMLTreeNode *r=node->GetChild(); r; r=r->GetNext())
   {
      const char *type = r->GetType();
      const char *str = r->GetData();
      
      if(!strcmp(type, "maxChars"))
      {
         maxChars = atoi(str);
      }
      else if(!strcmp(type, "usableChars"))
      {
         usableChars = str;
      }
   }
}

void eTextInputFieldEntry::createWidget(void)
{
   eString textStr = "";
   
   textInputFieldWidget = new eTextInputField(parent);
   conf->getConfig(name, &textStr);
   textInputFieldWidget->move(position); //eWidget
   textInputFieldWidget->resize(size); //eWidget
   textInputFieldWidget->loadDeco(); //eDecoWidget
   if(maxChars)
   {
      textInputFieldWidget->setMaxChars(maxChars);
   }
   if(usableChars.length())
   {
      textInputFieldWidget->setUseableChars(usableChars.c_str());
   }
   textInputFieldWidget->setText(textStr);
   textInputFieldWidget->show(); //eWidget
   CONNECT(textInputFieldWidget->selected, eTextInputFieldEntry::handleSelected);
   widget = textInputFieldWidget;
}

void eTextInputFieldEntry::handleSelected(void)
{
   eString textStr = textInputFieldWidget->getText();
   conf->setConfig(name, textStr);
   executeActions();
   parent->windowEntry->updateAllWidgets();
}

//-----------------------------------------------------------------------------
