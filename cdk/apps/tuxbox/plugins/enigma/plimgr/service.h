/***************************************************************************
                          service.h  -  description
                             -------------------
    begin                : Nov 18th 2005
    copyright            : (C) 2005 by pieterg
 ***************************************************************************/

#ifndef _service_h
#define _service_h

#include <list>

class CServiceTree
{
public:
	class CService
	{
		const char *m_pcName;
		CServiceTree *m_pParent;
	public:
		CService(CServiceTree *pParent, const char *pcName);
		~CService();
	
		bool operator == (const char *) const;
		operator const char* () const
		{
			if (m_pcName)
				return m_pcName;
			else
				return "";
		}
	};

protected:
	std::list<CService*> m_Services;
	CService *m_pCurrentEmu;
	CService *m_pCurrentCardserver;

	int m_piPipe[2];

	void StartStopRun();

public:
	CServiceTree();
	~CServiceTree();

	int WriteToWorkerThread(void *pData, unsigned int iSize);

	int StartEmu(const char *pcName, bool bRestart = false);
	int StopEmu();
	const char *GetCurrentEmu();

	int StartCardserver(const char *pcName, bool bRestart = false);
	int StopCardserver();

	int StartService(const char *pcName, bool bRestart = false);
	int StopService(const char *pcName);
};

#endif
