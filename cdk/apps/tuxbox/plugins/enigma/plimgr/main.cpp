/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Oct 2005
    copyright            : (C) 2005 by the PLi(R) team
    email                :
 ***************************************************************************/

#include "configtree.h"
#include "service.h"

#include <string>
#include <iostream>
using namespace ::std;

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/un.h>
#include <unistd.h>
#include <alloca.h>
#include <dirent.h>
#include <errno.h>

#include "main.h"
#include "common.h"

#include "wrappers.h"

/* TODO: get our own autotools target */
#define PACKAGE "plimgr"

const char* EMUD_SOCKET = "/tmp/.emud.socket";
const char* PMT_SOCKET = "/tmp/.listen.camd.socket";

#ifndef VERSION
const char* VERSION = "1.4";
#endif

const char* SERVICESLINKS_DIR 	= "/var/etc/plimgr/services";
const char* EMULINKS_DIR 	= "/var/etc/plimgr/cams";
const char* CARDSERVERLINKS_DIR	= "/var/etc/plimgr/cardservers";

/*
 * ca_pmt_list_management values:
 */
#define LIST_MORE		0x00
					/* CA application should append a 'MORE' CAPMT object the list,
					 * and start receiving the next object
					 */
#define LIST_FIRST		0x01
					/* CA application should clear the list when a 'FIRST' CAPMT object
					 * is received, and start receiving the next object
					 */
#define LIST_LAST		0x02
					/* CA application should append a 'LAST' CAPMT object to the list,
					 * and start working with the list
					 */
#define LIST_ONLY		0x03
					/* CA application should clear the list when an 'ONLY' CAPMT object
					 * is received, and start working with the object
					 */
#define LIST_ADD		0x04
					/* CA application should append an 'ADD' CAPMT object to the current list,
					 * and start working with the updated list
					 */
#define LIST_UPDATE		0x05
					/* CA application should replace an entry in the list with an
					 * 'UPDATE' CAPMT object, and start working with the updated list
					 */

/*
 * ca_pmt_cmd_id's:
 */
#define CMD_OK_DESCRAMBLING	0x01
					/* CA application should start descrambling the service in this CAPMT object,
					 * as soon as the list of CAPMT objects is complete
					 */
#define CMD_OK_MMI		0x02
#define CMD_QUERY		0x03
#define CMD_NOT_SELECTED	0x04
					/* CA application should stop descrambling this service
					 * (used when the last service in a list has left, note
					 * that there is no CI definition to send an empty list)
					 */

CPLiManager *pManager = NULL;

void sigterm_handler(int)
{
	if (pManager) pManager->Exit();
}

int main(int argc, char *argv[])
{
	pManager = new CPLiManager;
	if (pManager)
	{
		pManager->Run(argc, argv);
		delete pManager;
	}
}

CPLiManager::CServiceInfo::CServiceInfo(const char *pcName, const char *pcVersion)
{
	m_pcName = m_pcVersion = NULL;
	if (pcName) m_pcName = strdup(pcName);
	if (pcVersion) m_pcVersion = strdup(pcVersion);
}

CPLiManager::CServiceInfo::CServiceInfo(const char *pcName)
{
	m_pcName = m_pcVersion = NULL;
	if (pcName) m_pcName = strdup(pcName);
}

CPLiManager::CServiceInfo::~CServiceInfo()
{
	if (m_pcName) free(m_pcName);
	if (m_pcVersion) free(m_pcVersion);
}

const char *CPLiManager::CServiceInfo::GetName()
{
	return m_pcName;
}

const char *CPLiManager::CServiceInfo::GetVersion()
{
	return m_pcVersion;
}

CPLiManager::CPLiManager()
{
	m_bStartup = true;
	m_iCaPMTFd = -1;
	m_iCurrentChannel = 0;
	m_iCurrentProvider = 0;
	m_bFTA = false;
	m_pRunServices = NULL;
	m_pConfigTree = NULL;
	m_piCmdPipe[0] = -1;
	m_piCmdPipe[1] = -1;
}

CPLiManager::~CPLiManager()
{
	std::vector<CServiceInfo*>::iterator it;

	if (m_pRunServices) delete m_pRunServices;
	if (m_pConfigTree) delete m_pConfigTree;

	for (it = m_EmuList.begin(); it != m_EmuList.end(); )
	{
		CServiceInfo *pInfo = *it;
		it = m_EmuList.erase(it);
		delete pInfo;
	}
	for (it = m_CardServerList.begin(); it != m_CardServerList.end(); )
	{
		CServiceInfo *pInfo = *it;
		it = m_CardServerList.erase(it);
		delete pInfo;
	}
	for (it = m_ServiceList.begin(); it != m_ServiceList.end(); )
	{
		CServiceInfo *pInfo = *it;
		it = m_ServiceList.erase(it);
		delete pInfo;
	}

	if (m_piCmdPipe[0] >= 0)
	{
		close(m_piCmdPipe[0]);
	}
	if (m_piCmdPipe[1] >= 0)
	{
		close(m_piCmdPipe[1]);
	}
}

void CPLiManager::DaemonInit(const char *pcLogFile)
{
	int i;
	pid_t pid;

	if ((pid = fork()) != 0)
		exit(0);

	setsid(); /* become session leader */

	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	if ((pid = fork()) != 0)
		exit(0);

	for (i = 0; i < 10; i++)
		close(i);

	chdir("/");

	freopen("/dev/null", "a", stdin);
	freopen(pcLogFile, "a", stdout);
	freopen(pcLogFile, "a", stderr);
}

int CPLiManager::linkmatch(const struct dirent *entry)
{
	if (!entry) return 0;
	if (entry->d_name[0] == '.') return 0;
	return 1;
}

void CPLiManager::ScanForEmus()
{
	int i;
	int number;
	int index;
	std::vector<CServiceInfo*>::iterator it;
	struct dirent **namelist = NULL;
	number = scandir(EMULINKS_DIR, &namelist, linkmatch, NULL); /* no sorting */
	index = 0;
	for (it = m_EmuList.begin(); it != m_EmuList.end(); )
	{
		CServiceInfo *pInfo = *it;
		it = m_EmuList.erase(it);
		delete pInfo;
	}
	for (i = 0; i < number; i++)
	{
		FILE *pFile;
		std::string strFileName;
		if (!namelist[i] || !namelist[i]->d_name) continue;
		strFileName = (string)EMULINKS_DIR + (string)"/" + (string)namelist[i]->d_name;
		pFile = fopen(strFileName.c_str(), "r");
		if (pFile)
		{
			char pcContents[64];
			int iResult = 0;
			if ((iResult = fread(pcContents, 1, sizeof(pcContents) - 1, pFile)) > 0)
			{
				pcContents[iResult] = 0;
				m_EmuList.push_back(new CServiceInfo(namelist[i]->d_name, pcContents));
			}
			else
			{
				m_EmuList.push_back(new CServiceInfo(namelist[i]->d_name));
			}
			fclose(pFile);
		}
		free(namelist[i]);
	}
	if (namelist) free(namelist);
}

void CPLiManager::ScanForCardservers()
{
	int i;
	int number;
	int index;
	std::vector<CServiceInfo*>::iterator it;
	struct dirent **namelist = NULL;
	number = scandir(CARDSERVERLINKS_DIR, &namelist, linkmatch, NULL); /* no sorting */
	index = 0;
	for (it = m_CardServerList.begin(); it != m_CardServerList.end(); )
	{
		CServiceInfo *pInfo = *it;
		it = m_CardServerList.erase(it);
		delete pInfo;
	}
	for (i = 0; i < number; i++)
	{
		FILE *pFile;
		std::string strFileName;
		if (!namelist[i] || !namelist[i]->d_name) continue;
		strFileName = (string)CARDSERVERLINKS_DIR + (string)"/" + (string)namelist[i]->d_name;
		pFile = fopen(strFileName.c_str(), "r");
		if (pFile)
		{
			char pcContents[64];
			int iResult = 0;
			if ((iResult = fread(pcContents, 1, sizeof(pcContents) - 1, pFile)) > 0)
			{
				pcContents[iResult] = 0;
				m_CardServerList.push_back(new CServiceInfo(namelist[i]->d_name, pcContents));
			}
			else
			{
				m_CardServerList.push_back(new CServiceInfo(namelist[i]->d_name));
			}
			fclose(pFile);
		}
		free(namelist[i]);
	}
	if (namelist) free(namelist);
}

void CPLiManager::ScanForServices()
{
	int i;
	int number;
	int index;
	std::vector<CServiceInfo*>::iterator it;
	struct dirent **namelist = NULL;
	number = scandir(SERVICESLINKS_DIR, &namelist, linkmatch, NULL); /* no sorting */
	index = 0;
	for (it = m_ServiceList.begin(); it != m_ServiceList.end(); )
	{
		CServiceInfo *pInfo = *it;
		it = m_ServiceList.erase(it);
		delete pInfo;
	}
	for (i = 0; i < number; i++)
	{
		FILE *pFile;
		std::string strFileName;
		if (!namelist[i] || !namelist[i]->d_name) continue;
		strFileName = (string)SERVICESLINKS_DIR + (string)"/" + (string)namelist[i]->d_name;
		pFile = fopen(strFileName.c_str(), "r");
		if (pFile)
		{
			char pcContents[64];
			int iResult = 0;
			if ((iResult = fread(pcContents, 1, sizeof(pcContents) - 1, pFile)) > 0)
			{
				pcContents[iResult] = 0;
				m_ServiceList.push_back(new CServiceInfo(namelist[i]->d_name, pcContents));
			}
			else
			{
				m_ServiceList.push_back(new CServiceInfo(namelist[i]->d_name));
			}
			fclose(pFile);
		}
		free(namelist[i]);
	}
	if (namelist) free(namelist);
}

int CPLiManager::SendReply(int iSocket, int iReply, const void *pData, int iSize)
{
	packet datapacket;
	datapacket.cmd = iReply;
	datapacket.datasize = iSize;
	if (Write(iSocket, &datapacket, sizeof(datapacket)) < 0) return -1;
	if (datapacket.datasize && Write(iSocket, pData, datapacket.datasize) < 0) return -1;
	return 0;
}

void CPLiManager::TransmitEmuName()
{
	int i;
	std::string name;
	char tlvbuf[136];
	tlvbuf[0] = 0x9F;
	tlvbuf[1] = 0x70;
	tlvbuf[2] = 0x10;

	name = m_pRunServices->GetCurrentEmu();

	for (i = 0; i < (int)m_EmuList.size(); i++)
	{
		if (name == m_EmuList[i]->GetName())
		{
			if (m_EmuList[i]->GetVersion())
			{
				name += " ";
				name += m_EmuList[i]->GetVersion();
			}
			break;
		}
	}

	i = name.size() + 1;
	if (i >= 127)
	{
		name[126] = 0;
		i = 127;
	}
	strcpy(tlvbuf + 4, name.c_str());
	tlvbuf[3] = i;
	printf("emuname to enigma:len=%d  =%s=%s=\n", i, tlvbuf + 4, name.c_str());
	fflush(stdout);
	if (Write(m_iCaPMTFd, tlvbuf, i + 4) < 0)
	{
		perror("emuname");
	}
}

int CPLiManager::HandleCommand(int iSocket)
{
	void *pData = NULL;
	packet datapacket;
	if (Read(iSocket, &datapacket, sizeof(datapacket)) <= 0) return -1;
	if (datapacket.datasize < 0 || datapacket.datasize > 4096) return -1;
	if (datapacket.datasize) pData = alloca(datapacket.datasize);
	if (pData && datapacket.datasize && Read(iSocket, pData, datapacket.datasize) <= 0) return -1;
	switch (datapacket.cmd)
	{
		case CMD_PUT_CHANNEL_SETTINGS_AND_RESTART:
		case CMD_PUT_CHANNEL_SETTINGS:
		{
			cout << "received CMD_PUT_CHANNEL_SETTINGS" << endl;
			int result = -1;
			const char *defaultemu = NULL;
			const char *channelemu = NULL;
			const char *provideremu = NULL;
			if (pData)
			{
				putchannelsettings *currentsettings = (struct putchannelsettings*)pData;

				/* ensure names are terminated */
				currentsettings->channelname[sizeof(currentsettings->channelname) - 1] = 0;
				currentsettings->providername[sizeof(currentsettings->providername) - 1] = 0;

				if (currentsettings->defaultemu >= 0 && currentsettings->defaultemu < (signed)m_EmuList.size())
				{
					defaultemu = *m_EmuList[currentsettings->defaultemu];
				}
				if (currentsettings->channelemu >= 0 && currentsettings->channelemu < (signed)m_EmuList.size())
				{
					channelemu = *m_EmuList[currentsettings->channelemu];
				}
				if (currentsettings->provideremu >= 0 && currentsettings->provideremu < (signed)m_EmuList.size())
				{
					provideremu = *m_EmuList[currentsettings->provideremu];
				}
				result = m_pConfigTree->StoreCurrentSettings(defaultemu, m_iCurrentProvider, currentsettings->providername, provideremu, m_iCurrentChannel, currentsettings->channelname, channelemu);
			}
			if (SendReply(iSocket, result, NULL, 0) < 0) return -1;
			/* call ChannelSelected, to force the new settings to be used */
			if (result >= 0)
			{
				if (m_pConfigTree->ChangeCurrentChannel(m_pRunServices, m_iCurrentProvider, m_iCurrentChannel, datapacket.cmd == CMD_PUT_CHANNEL_SETTINGS_AND_RESTART) > 0)
				{
					TransmitEmuName();
				}
			}
			break;
		}
		case CMD_GET_CHANNEL_SETTINGS:
		{
			cout << "received CMD_GET_CHANNEL_SETTINGS" << endl;
			int result = -1;
			char defaultemu[NAMELENGTH];
			char channelemu[NAMELENGTH];
			char provideremu[NAMELENGTH];
			getchannelsettings settings;

			settings.defaultemu = -1;
			settings.channelemu = -1;
			settings.provideremu = -1;
			result = m_pConfigTree->RetrieveCurrentSettings(m_iCurrentProvider, m_iCurrentChannel, defaultemu, (signed)sizeof(defaultemu), provideremu, (signed)sizeof(provideremu), channelemu, (signed)sizeof(channelemu));
			cout << "channel emu " << channelemu << endl;
			for (int i = 0; i < (signed)m_EmuList.size(); i++)
			{
				if (!strcmp(*m_EmuList[i], defaultemu))
				{
					settings.defaultemu = i;
				}
				if (!strcmp(*m_EmuList[i], channelemu))
				{
					settings.channelemu = i;
					cout << "channel emu " << i << endl;
				}
				if (!strcmp(*m_EmuList[i], provideremu))
				{
					settings.provideremu = i;
				}
			}
			if (SendReply(iSocket, result, &settings, sizeof(settings)) < 0) return -1;
			break;
		}
		case CMD_GET_EMU_NAME:
		{
			cout << "received CMD_GET_EMU_NAME" << endl;
			int *id = (int*)pData;
			char name[128];
			int result = -1;
			name[0] = 0;
			if (id && *id >= 0 && *id < (signed)m_EmuList.size())
			{
				strncpy(name, *m_EmuList[*id], sizeof(name) - 1);
				name[sizeof(name) - 1] = 0;
				result = 0;
			}
			if (SendReply(iSocket, result, name, strlen(name) + 1) < 0) return -1;
			break;
		}
		case CMD_GET_EMU_INFO:
		{
			cout << "received CMD_GET_EMU_INFO" << endl;
			int *id = (int*)pData;
			emuinfo info;
			bzero(&info, sizeof(info));
			int result = -1;
			if (id && *id >= 0 && *id < (signed)m_EmuList.size())
			{
				strncpy(info.name, *m_EmuList[*id], sizeof(info.name) - 1);
				info.name[sizeof(info.name) - 1] = 0;
				if (m_EmuList[*id]->GetVersion())
				{
					strncpy(info.version, m_EmuList[*id]->GetVersion(), sizeof(info.version) - 1);
					info.version[sizeof(info.version) - 1] = 0;
				}
				result = 0;
			}
			if (SendReply(iSocket, result, &info, sizeof(info)) < 0) return -1;
			break;
		}
		case CMD_RESTART_EMU:
			cout << "received CMD_RESTART_EMU" << endl;
			m_pConfigTree->ChangeCurrentChannel(m_pRunServices, m_iCurrentProvider, m_iCurrentChannel, true); /* restart = true */
			if (SendReply(iSocket, 0, NULL, 0) < 0) return -1;
			break;
		case CMD_ENUM_SETTINGS:
		{
			cout << "received CMD_ENUM_SETTINGS" << endl;
			int result = -1;
			enumsettings settings;
			if (pData)
			{
				int offset = *(int*)pData;

				result = m_pConfigTree->EnumSettings(offset, settings.provider, settings.channel, settings.settingname, sizeof(settings.settingname), settings.emuname, sizeof(settings.emuname));
				/* ensure names are terminated */
				settings.settingname[sizeof(settings.settingname) - 1] = 0;
				settings.emuname[sizeof(settings.emuname) - 1] = 0;
			}
			if (result < 0)
			{
				settings.settingname[0] = 0;
				settings.emuname[0] = 0;
			}
			if (SendReply(iSocket, result, &settings, sizeof(settings) - sizeof(settings.emuname) + strlen(settings.emuname) + 1) < 0) return -1;
			break;
		}
		case CMD_GET_CARDSERVER_NAME:
		{
			cout << "received CMD_GET_CARDSERVER_NAME" << endl;
			int *id = (int*)pData;
			char name[128];
			int result = -1;
			name[0] = 0;
			if (id && *id >= 0 && *id < (signed)m_CardServerList.size())
			{
				strncpy(name, *m_CardServerList[*id], sizeof(name) - 1);
				name[sizeof(name) - 1] = 0;
				result = 0;
			}
			if (SendReply(iSocket, result, name, strlen(name) + 1) < 0) return -1;
			break;
		}
		case CMD_GET_CARDSERVER_SETTING:
		{
			cout << "received CMD_GET_CARDSERVER_SETTING" << endl;
			char pcCardserver[NAMELENGTH + 1];
			int result = 1;
			m_pConfigTree->GetCardServer(pcCardserver, sizeof(pcCardserver));
			/* ensure name is terminated */
			pcCardserver[sizeof(pcCardserver) - 1] = 0;
			if (SendReply(iSocket, result, pcCardserver, strlen(pcCardserver) + 1) < 0) return -1;
			break;
		}
		case CMD_SET_CARDSERVER_SETTING:
		{
			cout << "received CMD_SET_CARDSERVER_SETTING" << endl;
			int result = -1;
			char pcCardserver[NAMELENGTH + 1];
			pcCardserver[0] = 0;
			if (pData)
			{
				int size = datapacket.datasize;
				if (size > sizeof(pcCardserver)) size = sizeof(pcCardserver);
				strncpy(pcCardserver, (char*)pData, size);
				pcCardserver[sizeof(pcCardserver) - 1] = 0;
			}
			result = m_pConfigTree->SetCardServer(pcCardserver);
			m_pConfigTree->StartCardServer(m_pRunServices);
			if (SendReply(iSocket, result, NULL, 0) < 0) return -1;
			break;
		}
		case CMD_RESTART_CARDSERVER:
		{
			cout << "received CMD_RESTART_CARDSERVER" << endl;
			m_pConfigTree->StartCardServer(m_pRunServices, true);
			if (SendReply(iSocket, 0, NULL, 0) < 0) return -1;
			break;
		}

		case CMD_GET_SERVICE_NAME:
		{
			cout << "received CMD_GET_SERVICE_NAME" << endl;
			int *id = (int*)pData;
			char name[128];
			int result = -1;
			name[0] = 0;
			if (id && *id >= 0 && *id < (signed)m_ServiceList.size())
			{
				strncpy(name, *m_ServiceList[*id], sizeof(name) - 1);
				name[sizeof(name) - 1] = 0;
				result = 0;
			}
			if (SendReply(iSocket, result, name, strlen(name) + 1) < 0) return -1;
			break;
		}
		case CMD_GET_SERVICE_SETTING:
		{
			cout << "received CMD_GET_SERVICE_SETTING" << endl;
			int *id = (int*)pData;
			int setting = 0;
			int result = -1;
			if (id && *id >= 0 && *id < (signed)m_ServiceList.size())
			{
				result = 0;
				if (m_pConfigTree->GetService(*m_ServiceList[*id])) setting = 1;
			}
			if (SendReply(iSocket, result, &setting, sizeof(setting)) < 0) return -1;
			break;
		}
		case CMD_SET_SERVICE_SETTING:
		{
			cout << "received CMD_SET_SERVICE_SETTING" << endl;
			setserversetting *serversetting = (struct setserversetting*)pData;
			int result = -1;
			if (serversetting && serversetting->id >= 0 && serversetting->id < (signed)m_ServiceList.size())
			{
				result = m_pConfigTree->SetService(*m_ServiceList[serversetting->id], serversetting->on);
				if (serversetting->on == 0)
				{
					m_pRunServices->StopService(*m_ServiceList[serversetting->id]);
				}
			}
			if (SendReply(iSocket, result, NULL, 0) < 0) return -1;
			break;
		}
		case CMD_RESTART_SERVICES:
		{
			cout << "received CMD_RESTART_SERVICES" << endl;
			m_pConfigTree->StartConfiguredServices(m_pRunServices, true);
			if (SendReply(iSocket, 0, NULL, 0) < 0) return -1;
			break;
		}

		default: /* unknown command, ignore and reply error */
			cout << "unknown command received" << endl;
			if (SendReply(iSocket, -1, NULL, 0) < 0) return -1;
			break;
	}
	return 0;
}

void CPLiManager::Run(int argc, char *argv[])
{
	int emudfd = -1;
	int emudlistenfd = -1;
	int i;
	int daemon = 1;
	const char *pcLogFile = "/dev/null";

	struct sockaddr_un emudaddr;

	for (i = 1; i < argc; i++)
	{
		if (strstr(argv[i], "-q"))
		{
			char name[64];
			snprintf(name, sizeof(name), "/var/run/%s.pid", PACKAGE);
			name[sizeof(name) - 1] = 0;
			FILE *pidfile = fopen(name, "r");
			if (pidfile)
			{
				int pid;
				if (fscanf(pidfile, "%d", &pid) == 1)
				{
					if (kill(pid, SIGTERM) >= 0)
					{
						printf("stopping %s\n", PACKAGE);
						exit(0);
					}
				}
				fclose(pidfile);
			}
			printf("%s not running\n", PACKAGE);
			exit(-1);
		}
		else if (strstr(argv[i], "-d"))
		{
			daemon = 0;
		}
		else if (strstr(argv[i], "-l"))
		{
			if (i + 1 < argc)
			{
				pcLogFile = argv[i + 1];
				i++;
			}
		}
		else if (strstr(argv[i], "--version"))
		{
			printf("PLi manager\nversion %s\n", VERSION);
			exit(1);
		}
		else /* both for --help and invalid options */
		{
			printf("usage: %s [-d] [-l <logfile>] [--version] [--help]\n", PACKAGE);
			printf("\n-d        = run in the foreground (debug mode)");
			printf("\n-q        = stop running %s", PACKAGE);
			printf("\n-l <file> = log to <file>");
			printf("\n--version = show version and exit\n");
			exit(0);
		}
	}

	if (daemon)
	{
		DaemonInit(pcLogFile);
	}

	signal(SIGTERM, sigterm_handler);

	char name[64];
	snprintf(name, sizeof(name), "/var/run/%s.pid", PACKAGE);
	name[sizeof(name) - 1] = 0;
	FILE *pidfile = fopen(name, "w+");
	if (pidfile)
	{
		fprintf(pidfile, "%d", getpid());
		fclose(pidfile);
	}
	pipe(m_piCmdPipe);

	m_pRunServices = new CServiceTree;
	m_pConfigTree = new CConfigTree;
	if (m_pConfigTree)
	{
		m_pConfigTree->StartCardServer(m_pRunServices, true);
		m_pConfigTree->StartConfiguredServices(m_pRunServices, true);
	}

	sleep(5); /* wait for the services to start */

	if (m_pConfigTree)
	{
		m_pConfigTree->StartDefaultEmu(m_pRunServices, true);
	}

	if ((emudlistenfd = socket(AF_LOCAL, SOCK_STREAM, 0)) >= 0)
	{
		unlink(EMUD_SOCKET);
		memset(&emudaddr, 0, sizeof(emudaddr));
		emudaddr.sun_family = AF_LOCAL;
		strcpy(emudaddr.sun_path, EMUD_SOCKET);
		if (Bind(emudlistenfd, (struct sockaddr *) &emudaddr, sizeof(emudaddr)) >= 0)
		{
			chmod(EMUD_SOCKET, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			Listen(emudlistenfd, 5);
		}
	}

	while (1)
	{
		if (emudfd >= 0)
		{
			close(emudfd);
			emudfd = -1;
		}

		if (m_iCaPMTFd >= 0)
		{
			close(m_iCaPMTFd);
			m_iCaPMTFd = -1;
		}

		while (1)
		{
			struct sockaddr_un pmtaddr;
			bzero(&pmtaddr, sizeof(pmtaddr));
			pmtaddr.sun_family = AF_LOCAL;
			strcpy(pmtaddr.sun_path, PMT_SOCKET);
			if (m_iCaPMTFd >= 0) close(m_iCaPMTFd);
			if ((m_iCaPMTFd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) exit(-1);
			printf("connect\n");
			int iRetval = connect(m_iCaPMTFd, (sockaddr*)&pmtaddr, sizeof(pmtaddr));
			if (iRetval >= 0)
			{
				/* success */
				break;
			}
			else
			{
				if (errno == EINTR || errno == EINPROGRESS)
				{
					/* we should give it a bit more time */
					fd_set rset, wset;
					int maxfd = 0;
					int error;
					socklen_t len = sizeof(error);
					struct timeval timeout;
					timeout.tv_sec = 3;
					timeout.tv_usec = 0;
					FD_ZERO(&rset);
					if (m_piCmdPipe[0] >= 0)
					{
						FD_SET(m_piCmdPipe[0], &rset);
						maxfd = MAX(maxfd, m_piCmdPipe[0]);
					}
					FD_ZERO(&wset);
					if (m_iCaPMTFd >= 0)
					{
						FD_SET(m_iCaPMTFd, &wset);
						maxfd = MAX(maxfd, m_iCaPMTFd);
					}
	
					if (Select(maxfd + 1, &rset, &wset, NULL, &timeout) <= 0)
					{
						close(m_iCaPMTFd);
						m_iCaPMTFd = -1;
						break;
					}

					if (m_piCmdPipe[0] >= 0 && FD_ISSET(m_piCmdPipe[0], &rset))
					{
						/* this pipe is used to signal us that we should quit */
						close(m_iCaPMTFd);
						m_iCaPMTFd = -1;
						return;
					}

					if (getsockopt(m_iCaPMTFd, SOL_SOCKET, SO_ERROR, &error, &len) < 0 || error)
					{
						close(m_iCaPMTFd);
						m_iCaPMTFd = -1;
						break;
					}
					/* we're connected */
					break;
				}
				else
				{
					/* fatal error*/
					close(m_iCaPMTFd);
					m_iCaPMTFd = -1;
					break;
				}
			}
		}

		if (m_iCaPMTFd >= 0)
		{
			TransmitEmuName();
		}
		else
		{
			/* wait a bit, and retry, but only if we're not signaled to stop through the cmdPipe */
			fd_set rset;
			int maxfd = 0;
			struct timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 0;
			FD_ZERO(&rset);
			if (m_piCmdPipe[0] >= 0)
			{
				FD_SET(m_piCmdPipe[0], &rset);
				maxfd = MAX(maxfd, m_piCmdPipe[0]);
			}

			if (Select(maxfd + 1, &rset, NULL, NULL, &timeout) > 0)
			{
				if (m_piCmdPipe[0] >= 0 && FD_ISSET(m_piCmdPipe[0], &rset))
				{
					/* this pipe is used to signal us that we should quit */
					return;
				}
			}

			sleep(1);
			continue;
		}

		while (1)
		{
			fd_set rset;
			int maxfd = 0;

			FD_ZERO(&rset);
			if (emudlistenfd >= 0)
			{
				FD_SET(emudlistenfd, &rset);
				maxfd = MAX(maxfd, emudlistenfd);
			}
			if (emudfd >= 0)
			{
				FD_SET(emudfd, &rset);
				maxfd = MAX(maxfd, emudfd);
			}
			if (m_iCaPMTFd >= 0)
			{
				FD_SET(m_iCaPMTFd, &rset);
				maxfd = MAX(maxfd, m_iCaPMTFd);
			}
			if (m_piCmdPipe[0] >= 0)
			{
				FD_SET(m_piCmdPipe[0], &rset);
				maxfd = MAX(maxfd, m_piCmdPipe[0]);
			}
			if (Select(maxfd + 1, &rset, NULL, NULL, NULL) <= 0) break;
			if (m_iCaPMTFd >= 0 && FD_ISSET(m_iCaPMTFd, &rset))
			{
				int length = 0;
				int programinfo_length = 0;
				int lengthdatasize = 0;
				int readcount;
				unsigned char buffer[4096];
				readcount = 0;
				if (Read(m_iCaPMTFd, &buffer[readcount], 4) <= 0) break;
				readcount += 4;
				if (buffer[3] & 0x80)
				{
					/* multibyte length field */
					int i;
					lengthdatasize = buffer[3] & 0x7f;
					if (Read(m_iCaPMTFd, &buffer[readcount], lengthdatasize) <= 0) break;
					readcount += lengthdatasize;
					for (i = 0; i < lengthdatasize; i++)
					{
						length = (length << 8) | buffer[i + 4];
					}
				}
				else
				{
					/* singlebyte length field */
					length = buffer[3] & 0x7f;
				}
				if (Read(m_iCaPMTFd, &buffer[readcount], length) <= 0) break;
				readcount += length;
				if (!memcmp(buffer, "\x9F\x80\x32", 3)) /* CAPMT object */
				{
					int descriptor_length = 0;
					unsigned char *data = &buffer[4 + lengthdatasize];
#if 1
					if (data[6] == CMD_NOT_SELECTED)
					{
						printf("not_selected\n");
					}
					else if (data[6] == CMD_OK_DESCRAMBLING)
					{
						printf("ok_descrambling\n");
					}

					if (data[0] == LIST_FIRST)
					{
						printf("first\n");
					}
					else if (data[0] == LIST_LAST)
					{
						printf("last\n");
					}
					else if (data[0] == LIST_ONLY)
					{
						printf("only\n");
					}
					else if (data[0] == LIST_UPDATE)
					{
						printf("update\n");
					}
#endif
					if (data[6] == CMD_OK_DESCRAMBLING &&
					    (data[0] == LIST_FIRST || data[0] == LIST_ONLY || data[0] == LIST_UPDATE)) /* ok_descramble command, first or only object in the list, or an update */
					{
						m_iCurrentChannel = *(unsigned short *)&data[1];
						m_iCurrentProvider = 0;
						programinfo_length = *(unsigned short *)&data[4] & 0xfff;
						m_bFTA = true; /* assume m_bFTA, till we find a CA descriptor */
						for (i = 0; i < programinfo_length - 1; i += descriptor_length + 2)
						{
							descriptor_length = data[i + 8];
#if 0
							printf("descriptor %i length %i\n", *(unsigned short *)&data[i + 9], descriptor_length);
							for (int k = 0; k < descriptor_length + 2; k++)
							{
								printf("%02X ", data[i + 7 + k]);
							}
							printf("\n");
#endif
							if (data[i + 7] == 0x81)
							{
								/* private descr: dvb namespace */
								m_iCurrentProvider = *(unsigned short *)&data[i + 15];
							}
							if (data[i + 7] == 9)
							{
								m_bFTA = false;
								break;
							}
						}
						if (m_bFTA)
						{
							int es_info_length = 0;
							int j;
							for (i = programinfo_length + 6; i < length; i += es_info_length + 5)
							{
								es_info_length = *(unsigned short *)&data[i + 3] & 0xfff;
								for (j = 0; j < es_info_length - 1; j += descriptor_length + 2)
								{
									descriptor_length = data[i + j + 7];
#if 0
									printf("descriptor %i length %i\n", *(unsigned short *)&data[i + j + 8], descriptor_length);
									for (int k = 0; k < descriptor_length + 2; k++)
									{
										printf("%02X ", data[i + j + 6 + k]);
									}
									printf("\n");
#endif
									if (data[i + j + 6] == 9)
									{
										m_bFTA = false;
										break;
									}
								}
							}
						}
						printf("program %04X, network %04X, length %i, m_bFTA %i\n", m_iCurrentChannel, m_iCurrentProvider, length, m_bFTA);
 						if (m_bStartup || !m_bFTA)
						{
							if (m_pConfigTree->ChangeCurrentChannel(m_pRunServices, m_iCurrentProvider, m_iCurrentChannel) > 0)
							{
								/* the current emu was changed, we have to make the new emuname known to enigma */
								TransmitEmuName();
							}
							m_bStartup = false;
						}
					}
				}
			}
			if (emudfd >= 0 && FD_ISSET(emudfd, &rset))
			{
				int readcount;
				readcount = 0;
				printf("emufd\n");
				if (HandleCommand(emudfd) < 0)
				{
					close(emudfd);
					emudfd = -1;
				}
			}
			if (emudlistenfd >= 0 && FD_ISSET(emudlistenfd, &rset))
			{
				struct sockaddr_un cliaddr;
				printf("emulistenfd\n");
				socklen_t clilen = sizeof(cliaddr);
				/* check if we already had a client */
				if (emudfd >= 0)
				{
					/* disconnect our current client */
					close(emudfd);
					emudfd = -1;
				}
				/* accept a new client */
				if ((emudfd = Accept(emudlistenfd, (struct sockaddr*)&cliaddr, &clilen)) >= 0)
				{
					/* a new client connected, make sure we have our lists of installed services up to date */
					ScanForEmus();
					ScanForCardservers();
					ScanForServices();
				}
			}
			if (m_piCmdPipe[0] >= 0 && FD_ISSET(m_piCmdPipe[0], &rset))
			{
				/* this pipe is used to signal us that we should quit */
				return;
			}
		}
	}
}

void CPLiManager::Exit()
{
	if (m_piCmdPipe[1] >= 0)
	{
		Write(m_piCmdPipe[1], "\x01", 1);
	}
}
