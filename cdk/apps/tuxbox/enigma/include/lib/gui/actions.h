#ifndef __actions_h
#define __actions_h

#include <string.h>
#include <list>
#include <functional>
#include <set>

#include <libsig_comp.h>
#include <lib/base/i18n.h>
#include <lib/base/estring.h>
#include <lib/driver/rc.h>

typedef std::set<eRCKey> keylist;

class eActionMap;
class eActionMapList;

/**
 * \brief An action
 *
 * An action is an action of the user. It might raise by a keypress or otherwise. bla.
 */
class eAction
{
	char *description, *identifier;
	eActionMap *map;
	friend struct priorityComperator;
	int priority;
public:
	typedef std::pair<void*,eAction*> directedAction;
	std::map<eString, keylist> keys;
	/**
	 * \brief Functor to compare by priority.
	 *
	 * This is useful in sorted priority lists.
	 */
	struct priorityComperator
	{
		bool operator()(const directedAction &a, const directedAction &b)
		{
			return a.second->priority > b.second->priority;
		}
	};

	enum
	{
		prioGlobal=0, prioDialog=10, prioWidget=20, prioDialogHi=30
	};
	/**
	 * \param priority The priority of this action. 0 should be used for
	 * global actions, 10 for dialog-local actions, 20 for widget-local actions,
	 * 30 for dialog-hi-priority actions.
	 */
	eAction(eActionMap &map, char *identifier, char *description, int priority=0 );
	~eAction();
	const char *getDescription() const { return description; }
	const char *getIdentifier() const { return identifier; }

	void insertKey(const eString& style, const eRCKey& key )
	{
		keys[style].insert(key);
	}
	eAction* setDescription(char *desc) { description = desc; return this;}
	Signal0<void> handler;

//	keylist &getKeyList();
	int containsKey(const eRCKey &key, const eString& style ) const;
};

typedef std::multiset<eAction::directedAction,eAction::priorityComperator> eActionPrioritySet;

typedef std::map<eString, std::pair<int, eString> > eKeymap;

class XMLTreeNode;
class XMLTreeParser;

class eActionMap
{
	typedef std::list<eAction*> actionList;
	actionList actions;
	const char *identifier, *description;
public:
	eActionMap(const char *identifier, char *description);
	~eActionMap();
	void add(eAction *action)
	{
		actions.push_back(action);
	}
	void remove(eAction *action)
	{
		actions.remove(action);
	}
	void findAction(eActionPrioritySet &list, const eRCKey &key, void *context, const eString &style) const;
	eAction *findAction(const char *id) const;
	const char *getDescription() const { return description; }
	const char *getIdentifier() const { return identifier; }
	void loadXML(eRCDevice *device, eKeymap &keymap, const XMLTreeNode *node, const eString& s="");
};

class eActionMapList
{
	struct lstr
	{
		bool operator()(const char *a, const char *b) const
		{
			return strcmp(a, b)<0;
		}
	};
	typedef std::map<const char*,eActionMap*,lstr> actionMapList;

	actionMapList actionmaps;

	std::map<eString, eString> existingStyles;
	std::set<eString> currentStyles;
	ePtrList<XMLTreeParser> xmlfiles;
	XMLTreeNode *searchDevice(const eString &id);

	static eActionMapList *instance;
	void init_eActionMapList();
public:
	eActionMapList();
	~eActionMapList();
	const std::set<eString> &getCurrentStyles() const { return currentStyles; }
	void activateStyle( const eString& style ) { currentStyles.insert(style); }
	void deactivateStyle( const eString& style ) { currentStyles.erase(style); }
	eString getStyleDescription(const eString &style) const { std::map<eString, eString>::const_iterator i=existingStyles.find(style); if (i==existingStyles.end()) return ""; else return i->second; }
	const std::map<eString, eString> &getExistingStyles() const { return existingStyles; }
	void addActionMap(const char *, eActionMap * );
	void removeActionMap(const char *);
	int loadXML(const char *filename);
	int loadDevice(eRCDevice *device);
	eActionMap *findActionMap(const char *id) const;
	actionMapList &getActionMapList() { return actionmaps; }

	static eActionMapList *getInstance() { return instance; }
};

#endif
