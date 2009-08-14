#ifndef _main_h
#define _main_h

#include <vector>
#include "configtree.h"
#include "service.h"

class CPLiManager
{
private:
	bool m_bStartup;
protected:
	int m_iCurrentChannel;
	int m_iCurrentProvider;
	bool m_bFTA;

	int m_iCaPMTFd;
	int m_piCmdPipe[2];

	CServiceTree *m_pRunServices;
	CConfigTree *m_pConfigTree;

	class CServiceInfo
	{
		char *m_pcName;
		char *m_pcVersion;
		public:
		CServiceInfo(const char *pcName, const char *pcVersion);
		CServiceInfo(const char *pcName);
		~CServiceInfo();
		const char *GetName();
		const char *GetVersion();
		operator const char * () { return m_pcName; }
	};

	std::vector<CServiceInfo*> m_EmuList;
	std::vector<CServiceInfo*> m_CardServerList;
	std::vector<CServiceInfo*> m_ServiceList;

	static int linkmatch(const struct dirent *entry);

	void DaemonInit(const char *pcLogFile);

	void ScanForEmus();
	void ScanForCardservers();
	void ScanForServices();

	int HandleCommand(int iSocket);
	int SendReply(int iSocket, int iReply, const void *pData, int iSize);
	void TransmitEmuName();

public:
	CPLiManager();
	~CPLiManager();

	void Run(int argc, char *argv[]);
	void Exit();
};

#endif
