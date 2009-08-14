/*
 *  GPanel
 *
 *  Copyright (C) dAF2000
 *
 *  Licence see gpanel.cpp
 *
 */

#ifndef _GPANEL_H
#define _GPANEL_H

#include <xmltree.h>
#include <enigma_plugins.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/enumber.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/combobox.h>
#include <lib/gui/listbox.h>
#include <lib/gui/textinput.h>

#define GPANELRESTARTCODE (684)

class eWindowEntry;

class ConfigReader
{
   public:
   
   struct ConfigItem
   {
      eString variable;
      eString value;
   };
   
   ConfigReader(const eString &filename);
   ~ConfigReader();
   void getConfig(eString &variable, eString *value);
   void setConfig(eString &variable, eString &value);
   void addConfig(eString &variable, eString &value);
   void saveConfig(void);
   eString evaluateValue(eString &value);

   protected:
   
   std::vector<ConfigItem> config;
   eString filename;
};

class GPanel:public eWindow
{
   protected:
   
   XMLTreeParser *form;
   
   void loadItems(XMLTreeNode *screen);
   
   public:
   
   GPanel(const eString &xmlFile="/var/etc/gpanel.xml", XMLTreeNode *node=NULL);
   ~GPanel();
   ConfigReader *configReader;
   eWindowEntry *windowEntry; 
};

class eObjectEntry:public Object
{
   protected:

   eString name;
   XMLTreeNode *node;
   GPanel *parent;
   ConfigReader *conf;
   eWidget *widget;
   
   public:

   struct Actions
   {
      Actions()
      {
         save=false; 
	      quit=false;
         reload=false;
	      execute="";
         gpanel=""; 
      };
      
      bool save;
      bool quit;
      bool reload;
      eString execute;
      eString gpanel;
   };

   enum Compares
   {
      EQUAL,
      NOT_EQUAL,
      GREATER_EQUAL,
      LESS
   };
      
   struct Conditions
   {
      Conditions()
      {
         name = "";
         value = "";
      };
      
      eString name;
      Compares compare;
      eString value;
   };

   eObjectEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
   void parseActions(XMLTreeNode *node);
   void executeActions(void);
   void parseCondition(XMLTreeNode *node);
   void executeCondition(void);
   
   std::vector<Actions> actions;   
   struct Conditions condition;
};
   
class eListBoxEntryEntry:public eObjectEntry
{
   protected:
   
   eListBoxEntry *listBoxEntryWidget;
   
   eString helptext;

   public:
   
   eListBoxEntryEntry(eListBox<eListBoxEntry>* listbox, 
      GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
};

class eListBoxEntryTextEntry:public eListBoxEntryEntry
{
   protected:
   
   eString label;

   public:
   
   eListBoxEntryTextEntry(eListBox<eListBoxEntryText>* listbox,
      GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf, int key);

   void createWidget(void);
};

class eWidgetEntry:public eObjectEntry
{
   protected:
   
   eSize size;
   ePoint position;
   eString label;
   eString helptext;
   eString shortcut;

   public:
   
   eWidgetEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
   static ePoint lastPosition;
};

class eWindowEntry:public eWidgetEntry
{
   protected:
   
   eWindow *windowWidget;

   public:
   
   eWindowEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
   void createWidget(void);
   void updateAllWidgets(void);
   std::vector<eObjectEntry *> childs;
};

class eDecoWidgetEntry:public eWidgetEntry
{
   public:

   eDecoWidgetEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
};

class eLabelEntry:public eDecoWidgetEntry
{
   protected:
   
   eLabel *labelWidget;
   
   eString shortcutPixmap;
   
   public:
   
   eLabelEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
   void createWidget(void);
};

class eButtonEntry:public eLabelEntry
{
   protected:
   
   eButton *buttonWidget;
   
   public:
   
   eButtonEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
   void createWidget(void);
   void handleSelected(void);
};

class eCheckboxEntry:public eButtonEntry
{
   protected:
   
   eCheckbox *checkboxWidget;
   
   public:
   
   eCheckboxEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
   void createWidget(void);
   void handleChecked(int state);
};

class eComboboxEntry:public eButtonEntry
{
   protected:
   
   eComboBox *comboboxWidget;
   
   int entries;
   int flags;
   
   public:
   
   eComboboxEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
   void createWidget(void);
   void handleSelChanged(eListBoxEntryText *sel);
};

class eNumberEntry:public eDecoWidgetEntry
{
   protected:
   
   eNumber *numberWidget;
   
   int length;
   int minimal;
   int maximal;
   int maxDigits;
   int flags;
   
   public:
   
   eNumberEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
   void createWidget(void);
   void handleNumberChanged(int *dummy);
};

class eTextInputFieldEntry:public eButtonEntry
{
   protected:
   
   eTextInputField *textInputFieldWidget;
   
   int maxChars;
   eString usableChars;
   
   public:
   
   eTextInputFieldEntry(GPanel *parent, const eString &name, XMLTreeNode *node, ConfigReader *conf);
   void createWidget(void);
   void handleSelected(void);
};

#endif
