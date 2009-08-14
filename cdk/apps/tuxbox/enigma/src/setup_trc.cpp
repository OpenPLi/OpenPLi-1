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

#include "setup_trc.h"

//#define DEBUG 1

#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <lib/system/info.h>

//---------------------------------------------------------

TRC_Config::TRC_Config() :
	iStartNfs(0),
	iStartApache(0),
	iStartCron(0),
	iStartGsub(0),
	iStartFirewall(0),
	iStartInadyn(0),
	iStartDropbear(0),
	iSleep(0),
	iAutoZap(0),
	iEnableEmuDaemonLog(0),
	iEnableEnigmaLog(0),
	iEnableSysLog(0),
	fStartNfsChanged(false),
	fStartApacheChanged(false),
	fStartCronChanged(false),
	fStartGsubChanged(false),
	fStartFirewallChanged(false),
	fStartInadynChanged(false),
	fStartDropbearChanged(false),
	fSleepChanged(false),
	strLogPath("")
{
	strVarOn = "flash";
	strSwapOn = "";
	strConfigPath = "/var/etc/rc.config";
}
//---------------------------------------------------------

void TRC_Config::ReadConfig()
{
 	FILE *file;
 	char *ptr;
 	char left[32], right[64], line[128];

#ifdef DEBUG
 	printf ("ReadConfig()\n");
#endif

	file = fopen (strConfigPath.c_str(), "r");
	if (file)
   {
      while (fgets (line, 127, file) != NULL)
      {
      	if ((ptr = strstr (line, "="))) {
            *ptr = ' ';
			}
         if (sscanf (line, "%s %s", left, right) == 2) {
#ifdef DEBUG
				printf ("TRC_Config: %s=%s\n", left, right);
#endif
            if (strcmp (left, "SWAP_ON") == 0) {
					strSwapOn = right;
				}

				if (strcmp (left, "SWAP_SIZE") == 0) {
					strSwapSize = right;
				}

				if (strcmp (left, "VAR_ON") == 0) {
					strVarOn = right;
				}

				if (strcmp (left, "SLEEP") == 0) {
					sscanf (right, "%d", &iSleep);
				}

				if (strcmp (left, "CRON") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iStartCron = 1;
					}
				}

				if (strcmp (left, "GSUB") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iStartGsub = 1;
					}
				}

				if (strcmp (left, "FIREWALL") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iStartFirewall = 1;
					}
				}

				if (strcmp (left, "INADYN") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iStartInadyn = 1;
					}
				}

				if (strcmp (left, "DROPBEAR") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iStartDropbear = 1;
					}
				}

				if (strcmp (left, "NFSSERVER") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iStartNfs = 1;
					}
				}

				if (strcmp (left, "APACHE") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iStartApache = 1;
					}
				}

				if (strcmp (left, "DAEMON_LOG") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iEnableEmuDaemonLog = 1;
					}
				}
				if (strcmp (left, "ENIGMA_LOG") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iEnableEnigmaLog = 1;
					}
				}
				if (strcmp (left, "SYSLOG_LOG") == 0) {
					if (strcmp (right, "enabled") == 0) {
						iEnableSysLog = 1;
					}
				}
				if (strcmp (left, "SYSLOG_HOST") == 0) {
					m_strRemoteSyslogHost = right;
				}
				if (strcmp (left, "LOGDIR") == 0) {
					strLogPath = right;
				}
			}
		}
     	fclose (file);
	}

#ifdef DEBUG
	printf ("StartCron=%d\n", iStartCron);
	printf ("RC_CONFIG2: CRON=%s\n", iStartCron ? "enabled" : "disabled");
	printf ("RC_CONFIG2: GSUB=%s\n", iStartGsub ? "enabled" : "disabled");
	printf ("RC_CONFIG2: FIREWALL=%s\n", iStartFirewall ? "enabled" : "disabled");
	printf ("RC_CONFIG2: INADYN=%s\n", iStartInadyn ? "enabled" : "disabled");
	printf ("RC_CONFIG2: DROPBEAR=%s\n", iStartDropbear ? "enabled" : "disabled");
	printf ("RC_CONFIG2: NFSSERVER=%s\n", iStartNfs ? "enabled" : "disabled");
	printf ("RC_CONFIG2: APACHE=%s\n", iStartApache ? "enabled" : "disabled");
	printf ("RC_CONFIG2: DAEMON_LOG=%d\n", iEnableEmuDaemonLog);
	printf ("RC_CONFIG2: ENIGMA_LOG=%d\n", iEnableEnigmaLog);
	printf ("RC_CONFIG2: SYSLOG_LOG=%d\n", iEnableSysLog);
	printf ("RC_CONFIG2: SYSLOG_HOST=%s\n", m_strRemoteSyslogHost.c_str());
#endif

}

void TRC_Config::WriteConfig()
{
	FILE *out;
#ifdef DEBUG
	printf ("WriteConfig(): StartCron=%d\n", iStartCron);
#endif
	out = fopen ((strConfigPath + ".new").c_str(), "w");

	if (out) {
		fprintf (out, "CRON=%s\n", iStartCron ? "enabled" : "disabled");
		fprintf (out, "GSUB=%s\n", iStartGsub ? "enabled" : "disabled");
		fprintf (out, "FIREWALL=%s\n", iStartFirewall ? "enabled" : "disabled");
		fprintf (out, "INADYN=%s\n", iStartInadyn ? "enabled" : "disabled");
		fprintf (out, "DROPBEAR=%s\n", iStartDropbear ? "enabled" : "disabled");
		fprintf (out, "NFSSERVER=%s\n", iStartNfs ? "enabled" : "disabled");
		fprintf (out, "SLEEP=%d\n", iSleep);
		fprintf (out, "VAR_ON=%s\n", strVarOn.c_str());
		fprintf (out, "SWAP_ON=%s\n", strSwapOn.c_str());
		fprintf (out, "SWAP_SIZE=%s\n", strSwapSize.c_str());
		fprintf (out, "APACHE=%s\n", iStartApache ? "enabled" : "disabled");
		fprintf (out, "DAEMON_LOG=%s\n", iEnableEmuDaemonLog ? "enabled" : "disabled");
		fprintf (out, "ENIGMA_LOG=%s\n", iEnableEnigmaLog ? "enabled" : "disabled");
		fprintf (out, "SYSLOG_LOG=%s\n", iEnableSysLog ? "enabled" : "disabled");
		fprintf (out, "SYSLOG_HOST=%s\n", m_strRemoteSyslogHost.c_str());
		fprintf (out, "LOGDIR=%s\n", strLogPath.c_str());
		if (fclose (out) == 0) {
			out = NULL;
		}
		rename ((strConfigPath + ".new").c_str(), strConfigPath.c_str());
	}

#ifdef DEBUG
	printf ("End WriteRC()\n");
#endif
}
//---------------------------------------------------------

void TRC_Config::setStartNfs(const int iValue)
{
	iStartNfs = iValue;
	fStartNfsChanged = true;
}
//---------------------------------------------------------

void TRC_Config::setStartApache(const int iValue)
{
	iStartApache = iValue;
	fStartApacheChanged = true;
}
//---------------------------------------------------------

void TRC_Config::setStartCron(const int iValue)
{
	iStartCron = iValue;
	fStartCronChanged = true;
}
//---------------------------------------------------------

void TRC_Config::setStartGsub(const int iValue)
{
	iStartGsub = iValue;
	fStartGsubChanged = true;
}
//---------------------------------------------------------

void TRC_Config::setStartFirewall(const int iValue)
{
	iStartFirewall = iValue;
	fStartFirewallChanged = true;
}
//---------------------------------------------------------

void TRC_Config::setStartInadyn(const int iValue)
{
	iStartInadyn = iValue;
	fStartInadynChanged = true;
}
//---------------------------------------------------------

void TRC_Config::setStartDropbear(const int iValue)
{
	iStartDropbear = iValue;
	fStartDropbearChanged = true;
}
//---------------------------------------------------------

void TRC_Config::setSleep(const int iValue)
{
	iSleep = iValue;
	fSleepChanged = true;
}
//---------------------------------------------------------

void TRC_Config::setVarOn(const char* szValue)
{
	if (szValue)
	{
		strVarOn = szValue;
	}
	else
	{
		strVarOn = "";
	}
}
//---------------------------------------------------------

void TRC_Config::setSwapOn(const char* szValue)
{
	if (szValue)
	{
		strSwapOn = szValue;
	}
	else
	{
		strSwapOn = "";
	}
}
//---------------------------------------------------------

void TRC_Config::setSwapSize(const char* szValue)
{
	if (szValue)
	{
		strSwapSize = szValue;
	}
	else
	{
		strSwapSize = "";
	}
}
//---------------------------------------------------------

void TRC_Config::setEnableEmuDaemonLog(const int iValue)
{
	iEnableEmuDaemonLog = iValue;
}
//---------------------------------------------------------

void TRC_Config::setEnableEnigmaLog(const int iValue)
{
	iEnableEnigmaLog = iValue;
}
//---------------------------------------------------------


void TRC_Config::setEnableSysLog(const int iValue)
{
	iEnableSysLog = iValue;
}
//---------------------------------------------------------

void TRC_Config::setRemoteSyslogHost(const std::string& strRemoteSyslogHost)
{
	m_strRemoteSyslogHost = strRemoteSyslogHost;
}
//---------------------------------------------------------

void TRC_Config::setLogPath(const int iValue)
{
	switch (iValue) {
		case 0:
			strLogPath = "/tmp/log";
			break;
		case 1:
			strLogPath = "/media/hdd/log";
			break;
		case 2:
			strLogPath = "/media/usb/log";
			break;
		default:
			strLogPath = "";
			break;
	}
}
//---------------------------------------------------------

