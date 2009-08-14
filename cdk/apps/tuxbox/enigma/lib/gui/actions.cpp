#include <lib/gui/actions.h>

#include <xmltree.h>
#include <lib/base/eerror.h>
#include <lib/dvb/edvb.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/nconfig.h>

eAction::eAction(eActionMap &map, char *identifier, char *description, int priority)
	: description(description), identifier(identifier), map(&map), priority(priority)
{
	map.add(this);
}

eAction::~eAction()
{
	map->remove(this);
}

/*eAction::keylist &eAction::getKeyList()
{
	std::map<eString, keylist>::iterator it=keys.find( eActionMapList::getInstance()->getCurrentStyle() );
	if ( it != keys.end() )
		return it->second;
	it = keys.find("default);
	return it->second;
}*/

int eAction::containsKey(const eRCKey &key, const eString &style ) const
{
	std::map<eString, keylist>::const_iterator it=keys.find( style );
	if ( it != keys.end() )
	{
		if (it->second.find(key)!=it->second.end())
			return 1;
	}
	return 0;
}

// ---------------------------------------------------------------------------

void eActionMap::findAction(eActionPrioritySet &list, const eRCKey &key, void *context, const eString &style) const
{
	for (std::list<eAction*>::const_iterator i=actions.begin(); i!=actions.end(); ++i)
	{
		if ((*i)->containsKey(key, style))
			list.insert(eAction::directedAction(context, *i));
	}
}

eAction *eActionMap::findAction(const char *id) const
{
	for (std::list<eAction*>::const_iterator i=actions.begin(); i!=actions.end(); ++i)
		if (!strcmp((*i)->getIdentifier(), id))
			return *i;
	return 0;
}

eActionMap::eActionMap(const char *identifier, char *description): 
		identifier(identifier), description(description)
{
	eActionMapList::getInstance()->addActionMap(identifier, this);
} 

eActionMap::~eActionMap()
{
	eActionMapList::getInstance()->removeActionMap(identifier);
}

void eActionMap::loadXML(eRCDevice *device, eKeymap &keymap, const XMLTreeNode *node, const eString& style)
{
	for (XMLTreeNode *xaction=node->GetChild(); xaction; xaction=xaction->GetNext())
	{
		if (strcmp(xaction->GetType(), "action"))
		{
			eFatal("illegal type %s, expected 'action'", xaction->GetType());
			continue;
		}
		const char *name=xaction->GetAttributeValue("name");
		eAction *action=0;
		if (name)
			action=findAction(name);
		if (!action)
		{
			eWarning("please specify a valid action with name=. valid actions are:");
			for (actionList::iterator i(actions.begin()); i != actions.end(); ++i)
				eWarning("  %s (%s)", (*i)->getIdentifier(), (*i)->getDescription());
			eWarning("but NOT %s", name);
			continue;
		}
		const char *code=xaction->GetAttributeValue("code");
		int icode=-1;
		eString picture="";
		if (!code)
		{
			const char *key=xaction->GetAttributeValue("key");
			if (!key)
			{
				eFatal("please specify a number as code= or a defined key with key=.");
				continue;
			}

			eKeymap::iterator i=keymap.find(eString(key));

			if (i == keymap.end())
			{
				eFatal("undefined key %s specified!", key);
				continue;
			}
			icode=i->second.first;
			picture=i->second.second;
		} else
			sscanf(code, "%x", &icode);
	
		const char *flags=xaction->GetAttributeValue("flags");
		if (!flags || !*flags)
			flags="b";
		if (strchr(flags, 'm'))
			action->insertKey( style, eRCKey(device, icode, 0, picture) );
		if (strchr(flags, 'b'))
			action->insertKey( style, eRCKey(device, icode, eRCKey::flagBreak, picture) );
		if (strchr(flags, 'r'))
			action->insertKey( style, eRCKey(device, icode, eRCKey::flagRepeat, picture) );
	}
}

// ---------------------------------------------------------------------------

eActionMapList *eActionMapList::instance;

eActionMapList::eActionMapList()
{
	eActionMapList::init_eActionMapList();
}
void eActionMapList::init_eActionMapList()
{
	if (!instance)
		instance=this;
	char * tmp;
	currentStyles.insert("");
	
	if ( eConfig::getInstance()->getKey("/ezap/rc/style", tmp ) )
		currentStyles.insert("default");
	else
	{
		currentStyles.insert(tmp);
		free(tmp);
	}
	xmlfiles.setAutoDelete(true);
}

eActionMapList::~eActionMapList()
{
	xmlfiles.clear();
	if (instance==this)
		instance=0;
}

void eActionMapList::addActionMap(const char *id, eActionMap *am)
{
	actionmaps.insert(std::pair<const char*,eActionMap*>(id,am));
}

void eActionMapList::removeActionMap(const char *id)
{
	actionmaps.erase(id);
}

int eActionMapList::loadXML(const char *filename)
{
	FILE *in=fopen(filename, "rt");
	if (!in)
	{
//		eDebug("cannot open %s", filename);
		return -1;
	}

	XMLTreeParser *parser=new XMLTreeParser("ISO-8859-1");
	char buf[2048];
	
	int done;
	do
	{
		unsigned int len=fread(buf, 1, sizeof(buf), in);
		done=len<sizeof(buf);
		if (!parser->Parse(buf, len, done))
		{
			eFatal("parse error: %s at line %d",
				parser->ErrorString(parser->GetErrorCode()),
				parser->GetCurrentLineNumber());
			delete parser;
			parser=0;
			fclose(in);
			return -1;
		}
	} while (!done);
	fclose(in);

	XMLTreeNode *root=parser->RootNode();
	if (!root)
	{
		delete parser;
		return -1;
	}
	if (strcmp(root->GetType(), "rcdefaults"))
	{
		eFatal("not a rcdefaults file.");
		delete parser;
		return -1;
	}

	xmlfiles.push_back(parser);

	return 0;
}

XMLTreeNode *eActionMapList::searchDevice(const eString &id)
{
	for (ePtrList<XMLTreeParser>::iterator parser(xmlfiles.begin()); parser != xmlfiles.end(); ++parser)
	{
		XMLTreeNode *node=parser->RootNode();
		for (node=node->GetChild(); node; node=node->GetNext())
		if (!strcmp(node->GetType(), "device"))
		{
			eKeymap keymap;
			const char *identifier=node->GetAttributeValue("identifier");
			if (!identifier)
			{
				eFatal("please specify a remote control identifier!");
				continue;
			}
			if (id == identifier)
				return node;
		}
	}
	return 0;
}

int eActionMapList::loadDevice(eRCDevice *device)
{
		/* Don't load any mappings for (ascii) console */
	if (device->getIdentifier() == "Console")
		return 0;
		
	XMLTreeNode *node=searchDevice(device->getIdentifier());

	if (!node)
		node=searchDevice("generic");

	if (!node)
	{
		eWarning("couldn't load key bindings for device %s", device->getDescription());
		return -1;
	}

	eKeymap keymap;

	for (XMLTreeNode *xam=node->GetChild(); xam; xam=xam->GetNext())
		if (!strcmp(xam->GetType(), "actionmap"))
		{
			eActionMap *am=0;
			const char *name=xam->GetAttributeValue("name");
			if (name)
				am=findActionMap(name);
			if (!am)
			{
				eWarning("please specify a valid actionmap name (with name=)");
				eWarning("valid actionmaps are:");
				for (actionMapList::iterator i(actionmaps.begin()); i != actionmaps.end(); ++i)
					eWarning("  %s", i->first);
				eWarning("end.");
				eFatal("invalid actionmap: \"%s\"", name?name:"");
				continue;
			}
			const char *style=xam->GetAttributeValue("style");
			if (style)
			{
				const char *descr=xam->GetAttributeValue("descr");
				if (descr)
				{
					std::map<eString,eString>::iterator it = existingStyles.find(style);
					if ( it == existingStyles.end() ) // not in map..
						existingStyles[style]=descr;
				}
				am->loadXML(device, keymap, xam, style );
			}
			else
				am->loadXML( device, keymap, xam );
		}
		else if (!strcmp(xam->GetType(), "keys"))
		{
			for (XMLTreeNode *k=xam->GetChild(); k; k=k->GetNext())
			{
				if (!strcmp(k->GetType(), "key"))
				{
					const char *name=k->GetAttributeValue("name");
					if (name)
					{
						const char *acode=k->GetAttributeValue("code");
						int code=-1;
						if (acode)
							sscanf(acode, "%x", &code);
						else
						{
							acode=k->GetAttributeValue("icode");
							sscanf(acode, "%d", &code);
						}
						if ( code != -1 )
						{
							const char *apng=k->GetAttributeValue("picture");
							keymap.insert(std::pair<eString, std::pair<int, eString> >(name, std::pair<int, eString> (code, apng ? apng : "")));
						}
						else
							eFatal("no code specified for key %s!", name);
					}
					else
						eFatal("no name specified in keys!");
				}
			}
		}

	return 0;
}

eActionMap *eActionMapList::findActionMap(const char *id) const
{
	std::map<const char *,eActionMap*>::const_iterator i;
	i=(actionmaps.find(id));
	if (i==actionmaps.end())
		return 0;
	return i->second;
}

eAutoInitP0<eActionMapList> init_eActionMapList(eAutoInitNumbers::lowlevel, "eActionMapList");
