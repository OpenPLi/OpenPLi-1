/*
 * Ronald's setup plugin for dreambox
 * Copyright (c) 2004 Ronaldd <Ronaldd@sat4all.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef __setup_rc_h
#define __setup_rc_h

#include <string>

class TRC_Config
{
	public:
		TRC_Config();

		void ReadConfig();
		void WriteConfig();

		int getStartNfs() { return iStartNfs; };
		int getStartApache() { return iStartApache; };
		int getStartCron() { return iStartCron; };
		int getStartGsub() { return iStartGsub; };
		int getStartFirewall() { return iStartFirewall; };
		int getStartInadyn() { return iStartInadyn; };
		int getStartDropbear() { return iStartDropbear; };

		int getSleep() { return iSleep; };

		int getAutoZap() { return iAutoZap; };

		int getEnableEmuDaemonLog() { return iEnableEmuDaemonLog; };
		int getEnableEnigmaLog() { return iEnableEnigmaLog; };
		int getEnableSysLog() { return iEnableSysLog; };
		const std::string getRemoteSyslogHost() { return m_strRemoteSyslogHost; };

		const char* getVarOn() { return strVarOn.c_str(); };
		const char* getSwapOn() { return strSwapOn.c_str(); };
		const char* getSwapSize() { return strSwapSize.c_str(); };

		const char* getLogPath() { return strLogPath.c_str(); };

		void setStartNfs(const int iValue);
		void setStartApache(const int iValue);
		void setStartCron(const int iValue);
		void setStartGsub(const int iValue);
		void setStartFirewall(const int iValue);
		void setStartInadyn(const int iValue);
		void setStartDropbear(const int iValue);

		void setSleep(const int iValue);

		void setVarOn(const char* szValue);
		void setSwapOn(const char* szValue);
		void setSwapSize(const char* szValue);

		void setAutoZap(const int iValue);

		void setEnableEmuDaemonLog(const int iValue);
		void setEnableEnigmaLog(const int iValue);
		void setEnableSysLog(const int iValue);
		void setRemoteSyslogHost(const std::string& strRemoteSyslogHost);

		void setLogPath(const int iValue);

	private:
		int iStartNfs;
		int iStartApache;
		int iStartCron;
		int iStartGsub;
		int iStartFirewall;
		int iStartInadyn;
		int iStartDropbear;
		int iSleep;
		int iOsdTime;
		int iAutoZap;
		int iEnableEmuDaemonLog;
		int iEnableEnigmaLog;
		int iEnableSysLog;
		
		std::string m_strRemoteSyslogHost;

		std::string strVarOn;
		std::string strSwapOn;
		std::string strSwapSize;

		bool fStartNfsChanged;
		bool fStartApacheChanged;
		bool fStartCronChanged;
		bool fStartGsubChanged;
		bool fStartFirewallChanged;
		bool fStartInadynChanged;
		bool fStartDropbearChanged;
		bool fSleepChanged;

		std::string strLogPath;
		std::string strConfigPath;
};

#endif
