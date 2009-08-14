/***************************************************************************
                          service.cpp  -  description
                             -------------------
    begin                : Nov 18th 2005
    copyright            : (C) 2005 by pieterg
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "wrappers.h"
#include "service.h"

const char* START_STOP_SCRIPT = "/var/etc/plimgr/scripts/emusrv";

CServiceTree::CService::CService(CServiceTree *pParent, const char *pcName)
{
	m_pParent = pParent;
	if (pcName)
		m_pcName = strdup(pcName);
	else
		m_pcName = NULL;

	if (m_pParent && m_pcName)
	{
		char pcCmd[256];
		snprintf(pcCmd, sizeof(pcCmd), " %s start", m_pcName);
		m_pParent->WriteToWorkerThread(pcCmd, strlen(pcCmd));
	}
}

CServiceTree::CService::~CService()
{
	if (m_pParent && m_pcName)
	{
		char pcCmd[256];
		// Only send stop command if a name was supplied
		if (strlen(m_pcName) > 0)
		{
			snprintf(pcCmd, sizeof(pcCmd), " %s stop", m_pcName);
			m_pParent->WriteToWorkerThread(pcCmd, strlen(pcCmd));
		}

		delete m_pcName;
	}
}

bool CServiceTree::CService::operator == (const char *pcName) const
{
	if (!m_pcName || !pcName) return false;
	return (strcmp(m_pcName, pcName) == 0);
}

CServiceTree::CServiceTree()
: m_pCurrentEmu(NULL), m_pCurrentCardserver(NULL)
{
	pipe(m_piPipe);

	if (fork() == 0)
	{
		/* close our write end of the pipe */
		if (m_piPipe[1] >= 0)
		{
			close(m_piPipe[1]);
			m_piPipe[1] = -1;
		}
		StartStopRun();
		exit(0);
	}

	/* close our read end of the pipe */
	if (m_piPipe[0] >= 0)
	{
		close(m_piPipe[0]);
		m_piPipe[0] = -1;
	}
}

CServiceTree::~CServiceTree()
{
	if (m_pCurrentEmu) delete m_pCurrentEmu;
	if (m_pCurrentCardserver) delete m_pCurrentCardserver;

	std::list<CService*>::iterator it;
	for (it = m_Services.begin(); it != m_Services.end(); )
	{
		CService *pService = *it;
		it = m_Services.erase(it);
		delete pService;
	}

	/* close the write end of the pipe, our client will exit on EOF */
	if (m_piPipe[1] >= 0)
	{
		close(m_piPipe[1]);
		m_piPipe[1] = -1;
	}
}

int CServiceTree::StartEmu(const char *pcName, bool bRestart)
{
	if (!pcName) return -1;
	if (!bRestart && m_pCurrentEmu && *m_pCurrentEmu == pcName) return 0; /* no change */
	if (m_pCurrentEmu) delete m_pCurrentEmu;
	m_pCurrentEmu = new CService(this, pcName);
	return 1; /* change */
}

int CServiceTree::StopEmu()
{
	if (m_pCurrentEmu)
	{
		delete m_pCurrentEmu;
		m_pCurrentEmu = NULL;
		return 1; /* change */
	}
	return 0; /* no change */
}

const char *CServiceTree::GetCurrentEmu()
{
	if (m_pCurrentEmu)
		return (const char*)*m_pCurrentEmu;
	else
		return "";
}

int CServiceTree::StartCardserver(const char *pcName, bool bRestart)
{
	if (!pcName) return -1;
	if (!bRestart && m_pCurrentCardserver && *m_pCurrentCardserver == pcName) return 0; /* no change */
	if (m_pCurrentCardserver) delete m_pCurrentCardserver;
	m_pCurrentCardserver = new CService(this, pcName);
	return 1; /* change */
}

int CServiceTree::StopCardserver()
{
	if (m_pCurrentCardserver)
	{
		delete m_pCurrentCardserver;
		m_pCurrentCardserver = NULL;
		return 1; /* change */
	}
	return 0; /* no change */
}

int CServiceTree::StartService(const char *pcName, bool bRestart)
{
	if (!pcName) return -1;
	std::list<CService*>::iterator it;
	for (it = m_Services.begin(); it != m_Services.end(); ++it)
	{
		CService *pService = *it;
		if (*pService == pcName)
		{
			/* service already running */
			if (!bRestart) return 0; /* no change */
			m_Services.erase(it);
			delete pService;
			break;
		}
	}
	m_Services.push_back(new CService(this, pcName));
	return 1; /* change */
}

int CServiceTree::StopService(const char *pcName)
{
	if (!pcName) return -1;
	std::list<CService*>::iterator it;
	for (it = m_Services.begin(); it != m_Services.end(); )
	{
		CService *pService = *it;
		if (*pService == pcName)
		{
			it = m_Services.erase(it);
			delete pService;
			return 1; /* change */
		}
		else
		{
			++it;
		}
	}
	return 0; /* no change */
}

int CServiceTree::WriteToWorkerThread(void *pData, unsigned int iSize)
{
	return Write(m_piPipe[1], pData, iSize);
}

void CServiceTree::StartStopRun()
{
	while (1)
	{
		fd_set rset;
		int iMaxFd = 0;

		FD_ZERO(&rset);
		if (m_piPipe[0] >= 0)
		{
			FD_SET(m_piPipe[0], &rset);
			iMaxFd = MAX(iMaxFd, m_piPipe[0]);
		}
		if (Select(iMaxFd + 1, &rset, NULL, NULL, NULL) <= 0) break;
		if (m_piPipe[0] >= 0 && FD_ISSET(m_piPipe[0], &rset))
		{
			char pcCmd[512];
			int iReadCount = 0;
			int iResult = 0;
			snprintf(pcCmd, sizeof(pcCmd), "%s", START_STOP_SCRIPT);
			iReadCount = strlen(pcCmd);
			while (1)
			{
				if ((iResult = read(m_piPipe[0], &pcCmd[iReadCount], sizeof(pcCmd) - iReadCount)) <= 0)
				{
					if (errno == EINTR) continue;
					return;
				}
				if (iResult == 0) break;
				iReadCount += iResult;
				pcCmd[iReadCount] = 0;
				if (!strcmp(&pcCmd[iReadCount - 6], " start") || !strcmp(&pcCmd[iReadCount - 5], " stop"))
				{
					system(pcCmd);
					break;
				}
			}
		}
	}
}
