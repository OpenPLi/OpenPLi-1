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

	void DaemonInit(const char *pcLogFile);

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
