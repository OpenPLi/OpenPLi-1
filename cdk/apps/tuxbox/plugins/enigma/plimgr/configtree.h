/***************************************************************************
                          configtree.h  -  description
                             -------------------
    begin                : Nov 18th 2005
    copyright            : (C) 2005 by pieterg
 ***************************************************************************/

#ifndef _configtree_h
#define _configtree_h

#include <string>
#include <list>
#include <vector>

#ifdef HAVE_LIBXML2
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#define XML_Char char
#else
#include <xmltree.h>
#endif

#include <dirent.h>

#include "xmlgenerator.h"
#include "service.h"

#define CONFIGFILE "/var/etc/plimgr/plimgr.conf"
#define TMP_CONFIGFILE "/var/etc/plimgr/plimgr.conf.new"

#ifdef HAVE_LIBXML2
class CConfigTree
#else
class CConfigTree : public XML_Parser
#endif
{
public:
	class CServiceInfo
	{
		std::string m_strName;
		std::string m_strVersion;
		public:
		CServiceInfo(const char *pcName, const char *pcVersion);
		CServiceInfo(const char *pcName);
		~CServiceInfo();
		std::string GetName();
		std::string GetVersion();
		operator const char * () { return m_strName.c_str(); }
	};

	class CServiceConfig
	{
		const char *m_pcName;
	public:
		CServiceConfig(const char *pcName);
		virtual ~CServiceConfig();
		const char *GetName() { return m_pcName; }
		virtual int Store(CXMLGenerator &xml);
	};

	class CEmuConfig : public CServiceConfig
	{
	public:
		CEmuConfig(const char *pcName);
		int Store(CXMLGenerator &xml);
	};

	class CCardserverConfig : public CServiceConfig
	{
		public:
			CCardserverConfig(const char *pcName);
			int Store(CXMLGenerator &xml);
	};

	class CStickySetting
	{
		int m_iID;
		CEmuConfig *m_pEmu;
		const char *m_pcName;
	public:
		CStickySetting(int iID, const char *pcName);
		virtual ~CStickySetting();
		const char *GetName() { return m_pcName; }
		void SetName(const char *pcName)
		{
			if (m_pcName && strcmp(pcName, m_pcName))
			{
				delete m_pcName;
				m_pcName = NULL;
			}
			if (!m_pcName && pcName) m_pcName = strdup(pcName);
		}
		int GetID() { return m_iID; }
		CEmuConfig *GetEmu() { return m_pEmu; }
		const char *GetEmuName() { if (!m_pEmu) return NULL; return m_pEmu->GetName(); }
		int SetEmu(const char *pcName, std::vector<CServiceInfo*> &emulist);
		virtual int Store(CXMLGenerator &xml) = 0;
	};

	class CStickyProvider : public CStickySetting
	{
	public:
		class CStickyChannel : public CStickySetting
		{
		public:
			CStickyChannel(int iID, const char *pcName);
			int Store(CXMLGenerator &xml);
		};
	protected:
		std::list<CStickyChannel*> m_Channels;

	public:
		CStickyProvider(int iID, const char *pcName);
		~CStickyProvider();
		int Store(CXMLGenerator &xml);
		CStickyChannel *CreateStickyChannel(int iID, const char *pcName);
		CStickyChannel *FindStickyChannel(int iID);

		friend class CConfigTree;
	};

protected:
	std::list<CStickyProvider*> m_Providers;
	std::list<CServiceConfig*> m_Services;
	CEmuConfig* m_pDefaultEmu;
	CCardserverConfig* m_pCardserver;

	std::vector<CServiceInfo*> m_EmuList;
	std::vector<CServiceInfo*> m_CardServerList;
	std::vector<CServiceInfo*> m_ServiceList;

	static int linkmatch(const struct dirent *entry);

	//void LoadConfigTree(XMLTreeNode *pTree);

	int ReadConfig();
	int StoreConfig();
	int CreateService(const char *pcName);
	int RemoveService(const char *pcName);
	int CreateDefaultEmu(const char *pcName);
	int CreateCardserver(const char *pcName);

	CStickyProvider *CreateStickyProvider(int iID, const char *pcName);
	CStickyProvider *FindStickyProvider(int iID);

	enum { NONE = 0, SERVICE, EMU, CARDSERVER, PROVIDER, CHANNEL } m_eParseState;
	CStickyProvider *m_pParsingProvider;
	CStickyProvider::CStickyChannel *m_pParsingChannel;
	std::string m_strElementData;

#ifdef HAVE_LIBXML2
	xmlSAXHandler saxHandler;
	static void StartElement(void *user_data, const xmlChar *name, const xmlChar **attrs);
	static void EndElement(void *user_data, const xmlChar *name);
	static void Characters(void *user_data, const xmlChar *ch, int len);
#endif
	/* XML_Parser functions */
	void StartElementHandler(const XML_Char *name, const XML_Char **atts);
	void EndElementHandler(const XML_Char *name);
	void CharacterDataHandler(const XML_Char *s, int len);

public:
	CConfigTree();
	~CConfigTree();

	/* scan for all types of services */
	void ScanForServices();

	/* emu settings */
	void GetEmuList(std::vector<CServiceInfo*> &list) { list = m_EmuList; }
	int StoreCurrentSettings(const char *pcDefaultEmu, int iProviderID, const char *pcProviderName, const char *pcProviderEmu, int iChannelID, const char *pcChannelName, const char *pcChannelEmu);
	int RetrieveCurrentSettings(int iProviderID, int iChannelID, char *pcDefaultEmu, int iDefaultEmuSize, char *pcProviderEmu, int iProviderEmuSize, char *pcChannelEmu, int iChannelEmuSize);
	int EnumSettings(int iOffset, int &iProviderID, int &iChannelID, char *pcSettingName, int iSettingNameSize, char *pcEmuName, int iEmuNameSize);

	/* cardserver setting */
	void GetCardServerList(std::vector<CServiceInfo*> &list) { list = m_CardServerList; }
	int GetCardServer(char *pcName, int iSize);
	int SetCardServer(const char *pcName);

	/* service settings */
	void GetServiceList(std::vector<CServiceInfo*> &list) { list = m_ServiceList; }
	int GetService(const char *pcName);
	int SetService(const char *pcName, int iOn);

	/* status */
	int ChangeCurrentChannel(CServiceTree *pRunServices, int iProviderID, int iChannelID, bool bRestart = false);
	void StartConfiguredServices(CServiceTree *pRunServices, bool bRestart = false);
	void StartCardServer(CServiceTree *pRunServices, bool bRestart = false);
	void StartDefaultEmu(CServiceTree *pRunServices, bool bRestart = false);
};

#endif
