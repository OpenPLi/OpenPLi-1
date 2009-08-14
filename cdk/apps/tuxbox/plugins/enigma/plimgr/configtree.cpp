/***************************************************************************
                          configtree.cpp  -  description
                             -------------------
    begin                : Nov 18th 2005
    copyright            : (C) 2005 by pieterg
 ***************************************************************************/
#include <iostream>
using namespace ::std;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "wrappers.h"
#include "configtree.h"

#ifdef HAVE_LIBXML2
void CConfigTree::StartElement(void *user_data, const xmlChar *name, const xmlChar **attrs)
{
	CConfigTree *pUser = (CConfigTree*)user_data;
	pUser->StartElementHandler((const char*)name, (const char**)attrs);
}

void CConfigTree::EndElement(void *user_data, const xmlChar *name)
{
	CConfigTree *pUser = (CConfigTree*)user_data;
	pUser->EndElementHandler((const char*)name);
}

void CConfigTree::Characters(void *user_data, const xmlChar *ch, int len)
{
	CConfigTree *pUser = (CConfigTree*)user_data;
	pUser->CharacterDataHandler((const char*)ch, len);
}
#endif

CConfigTree::CServiceConfig::CServiceConfig(const char *pcName)
{
	m_pcName = NULL;
	if (pcName) m_pcName = strdup(pcName);
}

CConfigTree::CServiceConfig::~CServiceConfig()
{
	if (m_pcName) delete m_pcName;
}

int CConfigTree::CServiceConfig::Store(CXMLGenerator &xml)
{
	xml.StartElement("service");
	if (m_pcName) xml.ElementData(m_pcName);
	return xml.StopElement("service");
}

CConfigTree::CEmuConfig::CEmuConfig(const char *pcName)
 : CConfigTree::CServiceConfig(pcName)
{
}

int CConfigTree::CEmuConfig::Store(CXMLGenerator &xml)
{
	xml.StartElement("emu");
	if (GetName()) xml.ElementData(GetName());
	return xml.StopElement("emu");
}


CConfigTree::CCardserverConfig::CCardserverConfig(const char *pcName)
 : CConfigTree::CServiceConfig(pcName)
{
}

int CConfigTree::CCardserverConfig::Store(CXMLGenerator &xml)
{
	xml.StartElement("cardserver");
	if (GetName()) xml.ElementData(GetName());
	return xml.StopElement("cardserver");
}

CConfigTree::CStickySetting::CStickySetting(int iID, const char *pcName)
 : m_iID(iID), m_pEmu(NULL)
{
	m_pcName = NULL;
	if (pcName) m_pcName = strdup(pcName);
}

CConfigTree::CStickySetting::~CStickySetting()
{
	if (m_pEmu) delete m_pEmu;
	if (m_pcName) delete m_pcName;
}

void CConfigTree::CStickySetting::SetStickyEmu(const char *pcName)
{
	if (m_pEmu) delete m_pEmu;
	m_pEmu = new CEmuConfig(pcName);
}

CConfigTree::CStickyProvider::CStickyProvider(int iID, const char *pcName)
 : CConfigTree::CStickySetting(iID, pcName)
{
}

CConfigTree::CStickyProvider::~CStickyProvider()
{
	std::list<CStickyChannel*>::iterator it;
	for (it = m_Channels.begin(); it != m_Channels.end(); )
	{
		CStickyChannel *pChannel = *it;
		it = m_Channels.erase(it);
		delete pChannel;
	}
}

int CConfigTree::CStickyProvider::Store(CXMLGenerator &xml)
{
	if (!GetEmu() && m_Channels.empty()) return 0;
	xml.StartElement("provider");
	xml.Attribute("id", GetID());
	if (GetName()) xml.Attribute("name", GetName());
	if (GetEmu()) GetEmu()->Store(xml);

	std::list<CStickyChannel*>::const_iterator it;
	for (it = m_Channels.begin(); it != m_Channels.end(); ++it)
	{
		CStickyChannel *pChannel = *it;
		pChannel->Store(xml);
	}

	return xml.StopElement("provider");
}

CConfigTree::CStickyProvider::CStickyChannel::CStickyChannel(int iID, const char *pcName)
 : CConfigTree::CStickySetting(iID, pcName)
 {
 }

int CConfigTree::CStickyProvider::CStickyChannel::Store(CXMLGenerator &xml)
{
	if (!GetEmu()) return 0;
	xml.StartElement("channel");
	xml.Attribute("id", GetID());
	if (GetName()) xml.Attribute("name", GetName());
	if (GetEmu()) GetEmu()->Store(xml);
	return xml.StopElement("channel");
}

CConfigTree::CConfigTree()
#ifndef HAVE_LIBXML2
 : XML_Parser("ISO-8859-1")
#endif
{
#ifdef HAVE_LIBXML2
	memset(&saxHandler, 0, sizeof(saxHandler));
	saxHandler.startElement = StartElement;
	saxHandler.endElement = EndElement;
	saxHandler.characters = Characters;
#else
	startElementHandler = endElementHandler = characterDataHandler = 1;
#endif
	m_pDefaultEmu = NULL;
	m_pCardserver = NULL;

	ReadConfig();
}

CConfigTree::~CConfigTree()
{
	{
		std::list<CStickyProvider*>::iterator it;
		for (it = m_Providers.begin(); it != m_Providers.end(); )
		{
			CStickyProvider *pProvider = *it;
			it = m_Providers.erase(it);
			delete pProvider;
		}
	}
	{
		std::list<CServiceConfig*>::iterator it;
		for (it = m_Services.begin(); it != m_Services.end(); )
		{
			CServiceConfig *pService = *it;
			it = m_Services.erase(it);
			delete pService;
		}
	}
	if (m_pDefaultEmu) delete m_pDefaultEmu;
	if (m_pCardserver) delete m_pCardserver;
}

void CConfigTree::StartElementHandler(const XML_Char *name, const XML_Char **atts)
{
	const XML_Char **a = atts;
	/* we're starting a new element, empty the element data buffer */
	m_strElementData = "";

	if (!strcmp(name, "service"))
	{
		m_eParseState = SERVICE;
	}
	else if (!strcmp(name, "emu"))
	{
		m_eParseState = EMU;
	}
	else if (!strcmp(name, "cardserver"))
	{
		m_eParseState = CARDSERVER;
	}
	else if (!strcmp(name, "provider"))
	{
		int iID = 0;
		const char *pcName = NULL;
		if (a)
		{
			while (*a)
			{
				if (!strcmp((char*)a[0], "id"))
				{
					iID = atoi((char*)a[1]);
				}
				else if (!strcmp((char*)a[0], "name"))
				{
					pcName = (const char*)a[1];
				}
				a += 2;
			}
		}
		m_eParseState = PROVIDER;
		m_pParsingProvider = CreateStickyProvider(iID, pcName);
		m_pParsingChannel = NULL;
	}
	else if (!strcmp(name, "channel"))
	{
		int iID = 0;
		const char *pcName = NULL;
		if (a)
		{
			while (*a)
			{
				if (!strcmp((char*)a[0], "id"))
				{
					iID = atoi((char*)a[1]);
				}
				else if (!strcmp((char*)a[0], "name"))
				{
					pcName = (const char*)a[1];
				}
				a += 2;
			}
		}
		if (m_pParsingProvider)
		{
			m_eParseState = CHANNEL;
			m_pParsingChannel = m_pParsingProvider->CreateStickyChannel(iID, pcName);
		}
	}
}

void CConfigTree::EndElementHandler(const XML_Char *name)
{
	switch (m_eParseState)
	{
	default:
	case NONE:
		break;
	case SERVICE:
		m_eParseState = NONE;
		if (m_strElementData.size()) CreateService(m_strElementData.c_str());
		break;
	case EMU:
		if (m_pParsingChannel)
		{
			m_eParseState = CHANNEL;
			if (m_strElementData.size()) m_pParsingChannel->SetEmu(m_strElementData.c_str());
		}
		else if (m_pParsingProvider)
		{
			m_eParseState = PROVIDER;
			if (m_strElementData.size()) m_pParsingProvider->SetEmu(m_strElementData.c_str());
		}
		else
		{
			m_eParseState = NONE;
			if (m_strElementData.size()) CreateDefaultEmu(m_strElementData.c_str());
		}
		break;
	case CARDSERVER:
		m_eParseState = NONE;
		if (m_strElementData.size()) CreateCardserver(m_strElementData.c_str());
		break;
	case PROVIDER:
		m_pParsingProvider = NULL;
		break;
	case CHANNEL:
		m_pParsingChannel = NULL;
		m_eParseState = PROVIDER;
		break;
	}
	m_strElementData = "";
}

void CConfigTree::CharacterDataHandler(const XML_Char *s, int len)
{
	m_strElementData.append(s, len);
}

int CConfigTree::ReadConfig()
{
	m_eParseState = NONE;
	m_pParsingProvider = NULL;
	m_pParsingChannel = NULL;
#ifdef HAVE_LIBXML2
	return xmlSAXUserParseFile(&saxHandler, this, CONFIGFILE);
#else
	int iFd = open(CONFIGFILE, O_RDONLY);
	if (iFd >= 0)
	{
		char pcData[512];
		int iSize = 0;
		while ((iSize = Read(iFd, pcData, sizeof(pcData))) >= 0)
		{
			cout << "read " << iSize;
			if (iSize) cout << " " << pcData;
			cout << endl;
			XML_Parser::Parse(pcData, iSize, iSize != sizeof(pcData));
			if (iSize == 0) break;
		}
	}
#endif
	return 0;
}

int CConfigTree::StoreConfig()
{
	{
		CXMLGenerator xml(TMP_CONFIGFILE);
		xml.StartElement("plimgr_config");
		if (m_pDefaultEmu) m_pDefaultEmu->Store(xml);
		if (m_pCardserver) m_pCardserver->Store(xml);

		{
			std::list<CStickyProvider*>::const_iterator it;
			for (it = m_Providers.begin(); it != m_Providers.end(); ++it)
			{
				CStickyProvider *pProvider = *it;
				pProvider->Store(xml);
			}
		}

		{
			std::list<CServiceConfig*>::const_iterator it;
			for (it = m_Services.begin(); it != m_Services.end(); ++it)
			{
				CServiceConfig *pService = *it;
				pService->Store(xml);
			}
		}
		xml.StopElement("plimgr_config");

		if (!xml.IsOK()) return -1;
	}

	return rename(TMP_CONFIGFILE, CONFIGFILE);
}

int CConfigTree::CreateService(const char *pcName)
{
	std::list<CServiceConfig*>::const_iterator it;
	for (it = m_Services.begin(); it != m_Services.end(); ++it)
	{
		CServiceConfig *pService = *it;
		if (pcName && pService->GetName() && !strcmp(pService->GetName(), pcName)) return 0;
	}
	if (pcName && pcName[0]) m_Services.push_back(new CServiceConfig(pcName));
	return 0;
}

int CConfigTree::RemoveService(const char *pcName)
{
	std::list<CServiceConfig*>::iterator it;
	for (it = m_Services.begin(); it != m_Services.end(); ++it)
	{
		CServiceConfig *pService = *it;
		if (pcName && pService->GetName() && !strcmp(pService->GetName(), pcName))
		{
			it = m_Services.erase(it);
			delete pService;
		}
	}
	return -1;
}

int CConfigTree::CreateDefaultEmu(const char *pcName)
{
	if (m_pDefaultEmu)
	{
		delete m_pDefaultEmu;
		m_pDefaultEmu = NULL;
	}
	if (pcName && pcName[0]) m_pDefaultEmu = new CConfigTree::CEmuConfig(pcName);
	return 0;
}

int CConfigTree::CreateCardserver(const char *pcName)
{
	if (m_pCardserver)
	{
		delete m_pCardserver;
		m_pCardserver = NULL;
	}
	if (pcName && pcName[0]) m_pCardserver = new CConfigTree::CCardserverConfig(pcName);
	return 0;
}


CConfigTree::CStickyProvider *CConfigTree::CreateStickyProvider(int iID, const char *pcName)
{
	CStickyProvider *pProvider = NULL;
	std::list<CStickyProvider*>::const_iterator it;
	for (it = m_Providers.begin(); it != m_Providers.end(); ++it)
	{
		pProvider = *it;
		if (pProvider->GetID() == iID) break;
	}

	if (it == m_Providers.end())
	{
		pProvider = new CStickyProvider(iID, pcName);
		m_Providers.push_back(pProvider);
	}
	else
	{
		pProvider->SetName(pcName);
	}
	return pProvider;
}

CConfigTree::CStickyProvider *CConfigTree::FindStickyProvider(int iID)
{
	CStickyProvider *pProvider = NULL;
	std::list<CStickyProvider*>::const_iterator it;
	for (it = m_Providers.begin(); it != m_Providers.end(); ++it)
	{
		pProvider = *it;
		if (pProvider->GetID() == iID) break;
		pProvider = NULL;
	}
	return pProvider;
}

CConfigTree::CStickyProvider::CStickyChannel *CConfigTree::CStickyProvider::CreateStickyChannel(int iID, const char *pcName)
{
	CStickyChannel *pChannel = NULL;
	std::list<CStickyChannel*>::const_iterator it;
	for (it = m_Channels.begin(); it != m_Channels.end(); ++it)
	{
		pChannel = *it;
		if (pChannel->GetID() == iID) break;
	}

	if (it == m_Channels.end())
	{
		pChannel = new CStickyChannel(iID, pcName);
		m_Channels.push_back(pChannel);
	}
	else
	{
		pChannel->SetName(pcName);
	}
	return pChannel;
}

CConfigTree::CStickyProvider::CStickyChannel *CConfigTree::CStickyProvider::FindStickyChannel(int iID)
{
	CStickyChannel *pChannel = NULL;
	std::list<CStickyChannel*>::const_iterator it;
	for (it = m_Channels.begin(); it != m_Channels.end(); ++it)
	{
		pChannel = *it;
		if (pChannel->GetID() == iID) break;
		pChannel = NULL;
	}
	return pChannel;
}

int CConfigTree::StoreCurrentSettings(const char *pcDefaultEmu, int iProviderID, const char *pcProviderName, const char *pcProviderEmu, int iChannelID, const char *pcChannelName, const char *pcChannelEmu)
{
	cout << "create default" << endl;
	CreateDefaultEmu(pcDefaultEmu);

	if (iProviderID && iChannelID)
	{
		cout << "create provider " << iProviderID << endl;
		CStickyProvider *pProvider = NULL;
		if (!pProvider && (pcProviderEmu || pcChannelEmu))
		{
			pProvider = CreateStickyProvider(iProviderID, pcProviderName);
		}
		if (!pProvider) pProvider = FindStickyProvider(iProviderID);

		if (pProvider)
		{
			cout << "create channel " << iChannelID << endl;
			CStickyProvider::CStickyChannel *pChannel = NULL;
			if (pcChannelEmu)
			{
				pChannel = pProvider->CreateStickyChannel(iChannelID, pcChannelName);
			}
			if (!pChannel) pChannel = pProvider->FindStickyChannel(iChannelID);

			if (pChannel)
			{
				cout << "set channel emu" << endl;
				pChannel->SetEmu(pcChannelEmu);
			}

			cout << "set provider emu" << endl;
			pProvider->SetEmu(pcProviderEmu);
		}
	}

	cout << "store config" << endl;
	StoreConfig();
	cout << "stored" << endl;
	return 0;
}

int CConfigTree::RetrieveCurrentSettings(int iProviderID, int iChannelID, char *pcDefaultEmu, int iDefaultEmuSize, char *pcProviderEmu, int iProviderEmuSize, char *pcChannelEmu, int iChannelEmuSize)
{
	CStickyProvider::CStickyChannel *pChannel = NULL;
	CStickyProvider *pProvider = NULL;
	if (pcChannelEmu) pcChannelEmu[0] = 0;
	if (pcProviderEmu) pcProviderEmu[0] = 0;
	if (pcDefaultEmu) pcDefaultEmu[0] = 0;

	std::list<CStickyProvider*>::const_iterator it;
	for (it = m_Providers.begin(); it != m_Providers.end(); ++it)
	{
		pProvider = *it;
		if (pProvider->GetID() == iProviderID) break;
		pProvider = NULL;
	}

	if (pProvider)
	{
		pChannel = pProvider->FindStickyChannel(iChannelID);
	}
	if (pcChannelEmu && pChannel && pChannel->GetEmuName())
	{
		strncpy(pcChannelEmu, pChannel->GetEmuName(), iChannelEmuSize);
		pcChannelEmu[iChannelEmuSize - 1] = 0;
	}
	if (pcProviderEmu && pProvider && pProvider->GetEmuName())
	{
		strncpy(pcProviderEmu, pProvider->GetEmuName(), iProviderEmuSize);
		pcProviderEmu[iProviderEmuSize - 1] = 0;
	}

	if (pcDefaultEmu && m_pDefaultEmu && m_pDefaultEmu->GetName())
	{
		strncpy(pcDefaultEmu, m_pDefaultEmu->GetName(), iDefaultEmuSize);
		pcDefaultEmu[iDefaultEmuSize - 1] = 0;
	}
	return 0;
}

int CConfigTree::ChangeCurrentChannel(CServiceTree *pRunServices, int iProviderID, int iChannelID, bool bRestart)
{
	cout << "change current channel" << endl;
	CStickyProvider::CStickyChannel *pChannel = NULL;
	CStickyProvider *pProvider = NULL;
	std::list<CStickyProvider*>::const_iterator it;
	for (it = m_Providers.begin(); it != m_Providers.end(); ++it)
	{
		pProvider = *it;
		if (pProvider->GetID() == iProviderID) break;
		pProvider = NULL;
	}
	cout << "provider loop exit" << endl;
	if (pProvider)
	{
		cout << "provider found" << endl;
		pChannel = pProvider->FindStickyChannel(iChannelID);
	}
	if (pChannel && pChannel->GetEmuName())
	{
		cout << "channel found" << endl;
		if (pRunServices) return pRunServices->StartEmu(pChannel->GetEmuName(), bRestart);
	}
	else if (pProvider && pProvider->GetEmuName())
	{
		cout << "provider + name" << endl;
		if (pRunServices) return pRunServices->StartEmu(pProvider->GetEmuName(), bRestart);
	}
	else if (m_pDefaultEmu && m_pDefaultEmu->GetName())
	{
		cout << "default emu" << endl;
		if (pRunServices) return pRunServices->StartEmu(m_pDefaultEmu->GetName(), bRestart);
	}
	else
	{
		/* no emu, stop running emu */
		if (pRunServices) return pRunServices->StopEmu();
	}
	return 0;
}

int CConfigTree::EnumSettings(int iOffset, int &iProviderID, int &iChannelID, char *pcSettingName, int iSettingNameSize, char *pcEmuName, int iEmuNameSize)
{
	int iCount = 0;
	if (pcEmuName) pcEmuName[0]= 0;
	if (pcSettingName) pcSettingName[0] = 0;
	std::list<CStickyProvider*>::const_iterator it;
	for (it = m_Providers.begin(); it != m_Providers.end(); ++it)
	{
		CStickyProvider *pProvider = *it;
		if (pProvider->GetEmuName())
		{
			iCount++;
			if (iOffset == iCount - 1)
			{
				iChannelID = 0;
				iProviderID = pProvider->GetID();
				strncpy(pcEmuName, pProvider->GetEmuName(), iEmuNameSize);
				pcEmuName[iEmuNameSize - 1] = 0;
				if (pProvider->GetName())
					snprintf(pcSettingName, iSettingNameSize, "%s", pProvider->GetName());
				else
					snprintf(pcSettingName, iSettingNameSize, "%X", pProvider->GetID());
				return iCount;
			}
		}
		std::list<CStickyProvider::CStickyChannel*>::const_iterator channel_it;
		for (channel_it = pProvider->m_Channels.begin(); channel_it != pProvider->m_Channels.end(); ++channel_it)
		{
			CStickyProvider::CStickyChannel *pChannel = *channel_it;
			if (pChannel->GetEmuName())
			{
				iCount++;
				if (iOffset == iCount - 1)
				{
					iChannelID = pChannel->GetID();
					iProviderID = pProvider->GetID();
					strncpy(pcEmuName, pChannel->GetEmuName(), iEmuNameSize);
					pcEmuName[iEmuNameSize - 1] = 0;
					if (pChannel->GetName())
						snprintf(pcSettingName, iSettingNameSize, "%s", pChannel->GetName());
					else
						snprintf(pcSettingName, iSettingNameSize, "%X", pChannel->GetID());
					return iCount;
				}
			}
		}
	}
	return -1;
}

void CConfigTree::StartConfiguredServices(CServiceTree *pRunServices, bool bRestart)
{
	if (pRunServices)
	{
		std::list<CServiceConfig*>::const_iterator it;
		for (it = m_Services.begin(); it != m_Services.end(); ++it)
		{
			CServiceConfig *pService = *it;
			pRunServices->StartService(pService->GetName(), bRestart);
		}
	}
}

void CConfigTree::StartCardServer(CServiceTree *pRunServices, bool bRestart)
{
	if (pRunServices)
	{
		if (m_pCardserver)
		{
			pRunServices->StartCardserver(m_pCardserver->GetName(), bRestart);
		}
		else
		{
			pRunServices->StopCardserver();
		}
	}
}

void CConfigTree::StartDefaultEmu(CServiceTree *pRunServices, bool bRestart)
{
	if (pRunServices)
	{
		if (m_pDefaultEmu)
		{
			pRunServices->StartEmu(m_pDefaultEmu->GetName(), bRestart);
		}
		else
		{
			pRunServices->StopEmu();
		}
	}
}

int CConfigTree::GetCardServer(char *pcName, int iSize)
{
	if (iSize <= 0) return -1;
	pcName[0] = 0;
	if (m_pCardserver)
	{
		strncpy(pcName, m_pCardserver->GetName(), iSize);
		pcName[iSize - 1] = 0;
	}
	return 0;
}

int CConfigTree::SetCardServer(const char *pcName)
{
	int returnValue = CreateCardserver(pcName);
	StoreConfig();
	return returnValue;
}

int CConfigTree::GetService(const char *pcName)
{
	std::list<CServiceConfig*>::const_iterator it;
	for (it = m_Services.begin(); it != m_Services.end(); ++it)
	{
		CServiceConfig *pService = *it;
		if (pcName && pService->GetName() && !strcmp(pService->GetName(), pcName)) return 1;
	}
	/* not found */
	return 0;
}

int CConfigTree::SetService(const char *pcName, int iOn)
{
	int returnValue;
	if (iOn)
		returnValue = CreateService(pcName);
	else
		returnValue = RemoveService(pcName);
	StoreConfig();
	return returnValue;
}

