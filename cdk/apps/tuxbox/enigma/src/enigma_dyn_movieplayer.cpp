/*
 * $Id: enigma_dyn_movieplayer.cpp,v 1.20 2009/05/29 17:54:31 dbluelle Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
 
#ifdef ENABLE_EXPERT_WEBIF

#include <map>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <enigma.h>
#include <enigma_main.h>
#include <enigma_standby.h>
#include <timer.h>
#include <lib/driver/eavswitch.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/epgcache.h>
#include <lib/dvb/servicestructure.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/service.h>
#include <lib/dvb/record.h>
#include <lib/dvb/serviceplaylist.h>

#include <lib/system/info.h>
#include <lib/system/http_dyn.h>
#include <lib/system/econfig.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <enigma_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_movieplayer.h>
#include <lib/movieplayer/movieplayer.h>
#include <lib/movieplayer/mpconfig.h>
#include <configfile.h>

using namespace std;

eAutoInitP0<eMoviePlayer> init_movieplayer(eAutoInitNumbers::actions, "HTTP Movieplayer");

eString streamingServerSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/xml; charset=iso-8859-1";
	eMoviePlayer::getInstance()->mpconfig.load();
	eString result = readFile("/var/tuxbox/config/movieplayer.xml");
	if (!result)
		result = readFile(TEMPLATE_DIR + "movieplayer.xml");
	return result;
}


eString XSLMPSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/xml; charset=iso-8859-1";
	eString result = readFile(TEMPLATE_DIR + "XSLMPSettings.xsl");
	result.strReplace("#SERVEREDITBUTTON#", button(100, "Edit", NOCOLOR, "javascript:editStreamingServerSettings()", "#000000", true));
	result.strReplace("#VLCEDITBUTTON#", button(100, "Edit", NOCOLOR, "javascript:editStreamingServerVLCSettings()", "#000000", true));
	return result;
}


eString setStreamingServerSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=iso-8859-1";
	
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	eMoviePlayer::getInstance()->mpconfig.load();
	struct serverConfig server = eMoviePlayer::getInstance()->mpconfig.getServerConfig();
	
	server.serverIP = opt["serverIP"];
	server.startDir = opt["startDir"];
	server.CDDrive = opt["CDDrive"];
	
	eMoviePlayer::getInstance()->mpconfig.setServerConfig(server);
	eMoviePlayer::getInstance()->mpconfig.save();
	
	return closeWindow(content, "", 500);
}

eString setStreamingServerVideoSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=iso-8859-1";
	
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	struct videoTypeParms videoParms;
	
	videoParms.name = opt["name"];
	videoParms.extension = opt["extension"];
	videoParms.videoRate = opt["videoRate"];
	videoParms.audioRate = opt["audioRate"];
	videoParms.videoCodec = opt["videoCodec"];
	videoParms.videoRatio = opt["videoRatio"];
	videoParms.transcodeVideo = (opt["transcodeVideo"] == "on");
	videoParms.transcodeAudio = (opt["transcodeAudio"] == "on");
	videoParms.fps = opt["fps"];
	videoParms.soutadd = (opt["soutadd"] == "on");
	
	eMoviePlayer::getInstance()->mpconfig.setVideoParms(videoParms);
	eMoviePlayer::getInstance()->mpconfig.save();
	
	return closeWindow(content, "", 500);
}


eString setStreamingServerVLCSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=iso-8859-1";
	
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	eMoviePlayer::getInstance()->mpconfig.load();
	struct serverConfig server = eMoviePlayer::getInstance()->mpconfig.getServerConfig();
	
	server.webifPort = opt["webifPort"];
	server.streamingPort = opt["streamingPort"];
	server.vlcUser = opt["vlcUser"];
	server.vlcPass = opt["vlcPass"];
	
	eMoviePlayer::getInstance()->mpconfig.setServerConfig(server);
	eMoviePlayer::getInstance()->mpconfig.save();
	
	return closeWindow(content, "", 500);
}

eString editStreamingServerSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=iso-8859-1";
	
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	eString result = readFile(TEMPLATE_DIR + "editStreamingServerSettings.tmp");
	
	eMoviePlayer::getInstance()->mpconfig.load();
	struct serverConfig server = eMoviePlayer::getInstance()->mpconfig.getServerConfig();
	
	result.strReplace("#SERVERIP#", server.serverIP);
	result.strReplace("#STARTDIR#", server.startDir);
	result.strReplace("#CDDRIVE#", server.CDDrive);
	
	result.strReplace("#CHANGEBUTTON#", button(100, "Change", TOPNAVICOLOR, "javascript:submitSettings()", "#000000"));
	
	return result;
}

eString editStreamingServerVideoSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=iso-8859-1";
	
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString tmp;
	eString name = opt["name"];
	eString extension = opt["extension"];
	
	eString result = readFile(TEMPLATE_DIR + "editStreamingServerVideoSettings.tmp");
	
	struct videoTypeParms videoParms;
	eMoviePlayer::getInstance()->mpconfig.load();
	videoParms = eMoviePlayer::getInstance()->mpconfig.getVideoParms(name, extension);
	
	struct codecs avCodecs = eMoviePlayer::getInstance()->mpconfig.getAVCodecs();
	
	result.strReplace("#NAME#", videoParms.name);
	result.strReplace("#EXTENSION#", videoParms.extension);
	result.strReplace("#VIDEORATE#", videoParms.videoRate);
	result.strReplace("#AUDIORATE#", videoParms.audioRate);
	tmp = "";
	eString videoCodecs[2];
	videoCodecs[0] = avCodecs.mpeg1;
	videoCodecs[1] = avCodecs.mpeg2;
	for (int i = 0; i < 2; i++)
	{
		if (videoCodecs[i] == videoParms.videoCodec)
			tmp += "<option selected value=\"" + videoCodecs[i] + "\">";
		else
			tmp += "<option value=\"" + videoCodecs[i] + "\">";

		tmp += videoCodecs[i];
		tmp += "</option>";
	}
	result.strReplace("#VIDEOCODECS#", tmp);
	tmp = "";
	static eString videoRatios[] = {"352x288", "352x576", "480x576", "576x576", "704x576", "320x240", "352x240", "352x480", "480x480", "640x480", "704x480"};
	for (int i = 0; i < 11; i++)
	{
		if (videoRatios[i] == videoParms.videoRatio)
			tmp += "<option selected value=\"" + videoRatios[i] + "\">";
		else
			tmp += "<option value=\"" + videoRatios[i] + "\">";

		tmp += videoRatios[i];
		tmp += "</option>";
	}
	result.strReplace("#VIDEORATIOS#", tmp);
	result.strReplace("#TRANSCODEVIDEO#", (videoParms.transcodeVideo ? "checked" : ""));
	result.strReplace("#TRANSCODEAUDIO#", (videoParms.transcodeAudio ? "checked" : ""));
	result.strReplace("#FPS#", videoParms.fps);
	result.strReplace("#SOUTADD#", (videoParms.soutadd ? "checked" : ""));

	result.strReplace("#CHANGEBUTTON#", button(100, "Change", TOPNAVICOLOR, "javascript:submitSettings()", "#000000"));
	
	return result;
}

eString editStreamingServerVLCSettings(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=iso-8859-1";
	
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	
	eString result = readFile(TEMPLATE_DIR + "editStreamingServerVLCSettings.tmp");
	
	eMoviePlayer::getInstance()->mpconfig.load();
	struct serverConfig server = eMoviePlayer::getInstance()->mpconfig.getServerConfig();
	
	result.strReplace("#WEBIFPORT#", server.webifPort);
	result.strReplace("#STREAMINGPORT#", server.streamingPort);
	result.strReplace("#VLCUSER#", server.vlcUser);
	result.strReplace("#VLCPASS#", server.vlcPass);
	
	result.strReplace("#CHANGEBUTTON#", button(100, "Change", TOPNAVICOLOR, "javascript:submitSettings()", "#000000"));
	
	return result;
}

eString getStreamingServer()
{
	eString result = readFile(TEMPLATE_DIR + "streamingServer.tmp");
	
	eMoviePlayer::getInstance()->mpconfig.load();
	struct serverConfig server = eMoviePlayer::getInstance()->mpconfig.getServerConfig();

	result.strReplace("#DRIVE#", server.CDDrive);
	result.strReplace("#FILEBUTTON#", button(100, "File", NOCOLOR, "javascript:playFile()", "#000000"));
	result.strReplace("#DVDBUTTON#", button(100, "DVD", NOCOLOR, "javascript:playDVD()", "#000000"));
	result.strReplace("#VCDBUTTON#", button(100, "(S)VCD", NOCOLOR, "javascript:playVCD()", "#000000"));
	result.strReplace("#SETTINGSBUTTON#", button(100, "Settings", NOCOLOR, "javascript:settings()", "#000000"));
	eString tmp = button(100, "Terminate", RED, "javascript:terminateStreaming()", "#FFFFFF");
	result.strReplace("#TERMINATEBUTTON#", tmp);
	return result;
}

eString streamingServer(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	return getStreamingServer();
}

eString movieplayerm3u(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString command = opt["command"];
	eString mrl = httpUnescape(opt["mrl"]);
	eString result;
	
	eDebug("[MOVIEPLAYERPLS] command = %s, mrl = %s", command.c_str(), mrl.c_str());
	eMoviePlayer::getInstance()->control(command.c_str(), mrl.c_str());
	
	if (command == "start")
	{
		content->local_header["Content-Type"] = "video/mpegfile";
		content->local_header["Cache-Control"] = "no-cache";
		result = "#EXTM3U\n";
		result += "#EXTVLCOPT:sout=" + eMoviePlayer::getInstance()->sout(mrl) + "\n";
		result += mrl;
	}
	else
	{
		content->local_header["Content-Type"] = "text/html; charset=utf-8";
		content->local_header["Cache-Control"] = "no-cache";
		result = closeWindow(content, "", 500);
	}
	
	return result;
}

void ezapMoviePlayerInitializeDyn(eHTTPDynPathResolver *dyn_resolver, bool lockWeb)
{
	dyn_resolver->addDyn("GET", "/cgi-bin/movieplayer.m3u", movieplayerm3u, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/streamingServer", streamingServer, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/streamingServerSettings", streamingServerSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/editStreamingServerSettings", editStreamingServerSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/XSLMPSettings.xsl", XSLMPSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/setStreamingServerSettings", setStreamingServerSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/editStreamingServerVideoSettings", editStreamingServerVideoSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/setStreamingServerVideoSettings", setStreamingServerVideoSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/setStreamingServerVLCSettings", setStreamingServerVLCSettings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/editStreamingServerVLCSettings", editStreamingServerVLCSettings, lockWeb);
}
#endif

