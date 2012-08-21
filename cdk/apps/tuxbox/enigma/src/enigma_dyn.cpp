/*
 * $Id: enigma_dyn.cpp,v 1.570 2008/12/06 18:02:21 dbluelle Exp $
 *
 * (C) 2005,2007 by digi_casi <digi_casi@tuxbox.org>
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

// #define SCREENSHOT_PNG
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
#include <linux/input.h>
#include <iomanip>
#include <iostream>
#include <fstream>

#include <enigma.h>
#include <timer.h>
#include <enigma_main.h>
#include <enigma_plugins.h>
#include <enigma_standby.h>
#include <sselect.h>
#include <upgrade.h>
#include <math.h>

#include <lib/dvb/frontend.h>
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
#include <lib/gdi/fb.h>
#include <lib/gdi/glcddc.h>
#include <lib/gdi/gfbdc.h>
#include <lib/gdi/epng.h>
#include <lib/gui/emessage.h>
#include <lib/system/http_dyn.h>
#include <lib/system/econfig.h>
#include <lib/system/info.h>
#include <lib/system/dmfp.h>
#include <lib/system/file_eraser.h>
#include <lib/movieplayer/movieplayer.h>
#include <enigma_dyn.h>
#include <enigma_dyn_utils.h>
#include <enigma_dyn_mount.h>
#include <enigma_dyn_wap.h>
#include <enigma_dyn_conf.h>
#include <enigma_dyn_flash.h>
#include <enigma_dyn_rotor.h>
#include <enigma_dyn_xml.h>
#include <enigma_dyn_misc.h>
#include <enigma_dyn_epg.h>
#include <enigma_dyn_timer.h>
#include <enigma_dyn_pda.h>
#include <enigma_dyn_movieplayer.h>
#include <enigma_dyn_boot.h>
#include <enigma_dyn_chttpd.h>
#include <enigma_streamer.h>
#include <enigma_processutils.h>
#include <epgwindow.h>
#include <streaminfo.h>
#include <enigma_mount.h>
#include <satconfig.h>
#include <media_mapping.h>

#include <parentallock.h>

using namespace std;

#define KEYBOARDTV 0
#define KEYBOARDVIDEO 1

int keyboardMode = KEYBOARDTV;

int pdaScreen = 0;
int screenWidth = 1024;
eString lastTransponder;

int currentBouquet = 0;
int currentChannel = -1;

int zapMode = ZAPMODETV;
int zapSubMode = ZAPSUBMODEBOUQUETS;
eString zapSubModes[6] = {"Name", "Category", "Satellites", "Providers", "Bouquets", "All Services"};

eString zap[6][6] =
{
	{"TV", "0:7:1:0:0:0:0:0:0:0:", /* Satellites */ "1:15:fffffffc:12:0:0:0:0:0:0:", /* Providers */ "1:15:ffffffff:12:ffffffff:0:0:0:0:0:", /* Bouquets */ "4097:7:0:6:0:0:0:0:0:0:", /* All */ "1:15:fffffffe:12:ffffffff:0:0:0:0:0:"},
	{"Radio", "0:7:2:0:0:0:0:0:0:0:", /* Satellites */ "1:15:fffffffc:4:0:0:0:0:0:0:", /* Providers */ "1:15:ffffffff:4:ffffffff:0:0:0:0:0:", /* Bouquets */ "4097:7:0:4:0:0:0:0:0:0:", /* All */ "1:15:fffffffe:4:ffffffff:0:0:0:0:0:"},
	{"Data", "0:7:6:0:0:0:0:0:0:0:", /* Satellites */ "1:15:fffffffc:ffffffe9:0:0:0:0:0:0:", /* Providers */ "1:15:ffffffff:ffffffe9:ffffffff:0:0:0:0:0:", /* Bouquets */ "", /* All */ ""},
	{"Movies", "4097:7:0:1:0:0:0:0:0:0:", /* Satellites */ "", /* Providers */ "", /* Bouquets */ "", /* All */ ""},
	{"Root", "2:47:0:0:0:0:/", /* Satellites */ "", /* Providers */ "", /* Bouquets */ "", /* All */ ""},
	{"Stream", "", /* Satellites */ "", /* Providers */ "", /* Bouquets */ "", /* All */ ""}
};

extern bool onSameTP(const eServiceReferenceDVB& ref1, const eServiceReferenceDVB &ref2); // implemented in timer.cpp
extern bool canPlayService(const eServiceReference & ref); // implemented in timer.cpp

eString firmwareLevel(eString verid)
{
	eString result = "unknown";

	if (verid && verid != "&nbsp;")
	{
		int type = atoi(verid.left(1).c_str());
		char *typea[3];
		typea[0] = "release";
		typea[1] = "beta";
		typea[2] = "internal";
		eString ver = verid.mid(1, 3);
		eString date = verid.mid(4, 8);
//		eString time = verid.mid(12, 4);
		if (eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000)
			result = eString(typea[type%3]) + eString(" ") + ver[0] + "." + ver[1] + "." + ver[2]+ ", " + date.mid(6, 2) + "." + date.mid(4, 2) + "." + date.left(4);
		else
			result = eString().sprintf("%s %c.%d. %s", typea[type%3], ver[0], atoi(eString().sprintf("%c%c", ver[1], ver[2]).c_str()), (date.mid(6, 2) + "." + date.mid(4, 2) + "." + date.left(4)).c_str());
	}
	return result;
}

static eString tvMessageWindow(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	return readFile(TEMPLATE_DIR + "sendMessage.tmp");
}

static int getOSDShot(eString mode)
{
	gPixmap *p = 0;
	eDebug("getOSDShot: take shot, mode=%s", mode.c_str());
#ifndef DISABLE_LCD
	if (mode == "lcd")
		p = &gLCDDC::getInstance()->getPixmap();
	else
#endif
		p = &gFBDC::getInstance()->getPixmap();
		
	if (p)
		if (!savePNG("/tmp/osdshot.png", p))
			return 0;

	return -1;
}

static eString osdshot(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString,eString> opt=getRequestOptions(opts, '&');
	eString display = opt["display"];
	if (display == "")
		display = "yes";

	if (getOSDShot(opt["mode"]) == 0)
	{
		if (display == "yes")
		{
			content->local_header["Location"]="/root/tmp/osdshot.png";
			content->code = 307;
			return "+ok";
		}
		else
			return closeWindow(content, "", 500);
	}
	else
		return "-error";
}

bool playService(const eServiceReference &ref)
{
	// ignore locked service
	if (ref.isLocked() && pinCheck::getInstance()->pLockActive())
		return false;
	eZapMain::getInstance()->playService(ref, eZapMain::psSetMode|eZapMain::psDontAdd);
	return true;
}

void tuneTransponder(eString transponder)
{
	unsigned int frequency, symbol_rate;
	int polarisation, fec, orbital_position, inversion;
	sscanf(transponder.c_str(), "%d:%d:%d:%d:%d:%d:", &frequency, &symbol_rate, &polarisation, &fec, &orbital_position, &inversion);

	// search for the right transponder...
	for (std::list<tpPacket>::iterator it3(eTransponderList::getInstance()->getNetworks().begin()); it3 != eTransponderList::getInstance()->getNetworks().end(); it3++)
	{
		if (it3->orbital_position == orbital_position)
		{
			// ok, we have the right satellite now...
			for (std::list<eTransponder>::iterator it(it3->possibleTransponders.begin()); it != it3->possibleTransponders.end(); it++)
			{
				if (it->satellite.frequency == frequency && it->satellite.symbol_rate == symbol_rate && it->satellite.polarisation == polarisation && it->satellite.fec == fec && it->satellite.inversion == inversion)
				{
					// and this should be the right transponder...
					it->tune();
					lastTransponder = transponder;
				}
			}
		}
	}
}

static eString admin(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString command = opt["command"];
	eString result;
	if (eSystemInfo::getInstance()->canShutdown())
		result =  "Unknown admin command. (valid commands are: shutdown, reboot, restart, standby, wakeup)";
	else
		result =  "Unknown admin command. (valid commands are: reboot, restart, standby, wakeup)";
	if (command == "shutdown")
	{
		if (eSystemInfo::getInstance()->canShutdown())
		{
			eZap::getInstance()->quit();
			result = "Shutdown initiated...";
		}
		else
		{
			result = "No shutdown function available for this box.";
		}
	}
	else
	if (command == "reboot")
	{
		eZap::getInstance()->quit(4);
		result = "Reboot initiated...";
	}
	else
	if (command == "restart")
	{
		eZap::getInstance()->quit(2);
		result = "Restart initiated...";
	}
	else
	if (command == "wakeup")
	{
		if (eZapStandby::getInstance())
		{
			eZapStandby::getInstance()->wakeUp(0);
			result = "Enigma is waking up...";
		}
		else
		{
			result = "Enigma doesn't sleep.";
		}
	}
	else
	if (command == "standby")
	{
		if (eZapStandby::getInstance())
		{
			result = "Enigma is already sleeping.";
		}
		else
		{
			eZapMain::getInstance()->gotoStandby();
			result = "Standby initiated...";
		}
	}

	return "<html>" + eString(CHARSETMETA) + "<head><title>" + command + "</title></head><body>" + result + "</body></html>";
}

#ifndef DISABLE_FILE
static eString videocontrol(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString sReference = opt["sref"];
	eServiceReference sref = string2ref(sReference);
	eString command = opt["command"];
#ifdef ENABLE_EXPERT_WEBIF
	if (eMoviePlayer::getInstance()->getStatus().ACTIVE)
	{
		eMoviePlayer::getInstance()->control(command.c_str(), "");
	}
	else
#endif
	{
		if (command == "rewind")
		{
			eZapMain::getInstance()->startSkip(eZapMain::skipReverse);
		}
		else
		if (command == "forward")
		{
			eZapMain::getInstance()->startSkip(eZapMain::skipForward);
		}
		else
		if (command == "stop")
		{
			if (eZapMain::getInstance()->isRecording())
			{
				if (eSystemInfo::getInstance()->getDefaultTimerType() == ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recDVR)
				{
					if(ENgrab::nGrabActive) // stop ngrab from webif
						eZapMain::getInstance()->stopNGrabRecord();
					else
						eZapMain::getInstance()->recordDVR(0,0);
				}
				else
				if (eSystemInfo::getInstance()->getDefaultTimerType() == ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recNgrab)
					eZapMain::getInstance()->stopNGrabRecord();
			}
			else
				eZapMain::getInstance()->stop();
		}
		else
		if (command == "pause")
		{
			eZapMain::getInstance()->pause();
		}
		else
		if (command == "play")
		{
			eString curChannel = opt["curChannel"];
			if (curChannel)
			{
				currentChannel = atoi(curChannel.c_str());
				currentBouquet = 0;
			}
			if (sref)
			{
				if (eServiceInterface::getInstance()->service == sref)
					eZapMain::getInstance()->play();
				else
					playService(sref);
			}
			else
				eZapMain::getInstance()->play();
		}
		else
		if (command == "record")
		{
			if (eSystemInfo::getInstance()->getDefaultTimerType() == ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recDVR)
				eZapMain::getInstance()->recordDVR(1,0);
			else
			if (eSystemInfo::getInstance()->getDefaultTimerType() == ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recNgrab)
				eZapMain::getInstance()->startNGrabRecord();
		}
		else
		if (command == "ngrab") //recording with nGrab
			eZapMain::getInstance()->startNGrabRecord();
	}

	return closeWindow(content, "", 500);
}
#endif

static eString setAudio(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	int apid = -1;
	sscanf(opt["language"].c_str(), "0x%04x", &apid);

	eString channel = opt["channel"];
	eAVSwitch::getInstance()->selectAudioChannel(atoi(channel.c_str()));

	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		std::list<eDVBServiceController::audioStream> &astreams(sapi->audioStreams);
		std::list<eDVBServiceController::audioStream>::iterator it(astreams.begin());
		for (;it != astreams.end(); ++it)
			if (it->pmtentry->elementary_PID == apid)
			{
				eServiceHandler *service=eServiceInterface::getInstance()->getService();
				if (service)
					service->setPID(it->pmtentry);
				break;
			}
	}

	return WINDOWCLOSE;
}

eString getAudioChannels(void)
{
	eString result;
	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		std::list<eDVBServiceController::audioStream> &astreams(sapi->audioStreams);
		for (std::list<eDVBServiceController::audioStream>::iterator it(astreams.begin())
			;it != astreams.end(); ++it)
		{
			if (it->pmtentry->elementary_PID == Decoder::current.apid)
				result += eString().sprintf("<option selected value=\"0x%04x\">", it->pmtentry->elementary_PID);
			else
				result += eString().sprintf("<option value=\"0x%04x\">", it->pmtentry->elementary_PID);

			result += it->text;
			result += "</option>";
		}
	}
	else
		result = "<option>none</option>";

	return result;
}

static eString selectAudio(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	content->local_header["Cache-Control"] = "no-cache,no-store,must-revalidate,max-age=1";

	eString audioChannels = getAudioChannels();

	eString result = readFile(TEMPLATE_DIR + "audioSelection.tmp");
	result.strReplace("#LANGUAGES#", audioChannels);
	result.strReplace("#SELECTBUTTON#", button(100, "Select", TOPNAVICOLOR, "javascript:audioChange()", "#000000"));

	int channel = eAVSwitch::getInstance()->getAudioChannel();
	result.strReplace(eString().sprintf("#%d#", channel).c_str(), eString("checked"));
	result.strReplace("#0#", "");
	result.strReplace("#1#", "");
	result.strReplace("#2#", "");

	return result;
}

eString getCurrentSubChannel(eString curServiceRef)
{
	eString subChannel;
	if (curServiceRef)
	{
		eString s1 = curServiceRef; int pos; eString nspace;
		for (int i = 0; i < 7 && s1.find(":") != eString::npos; i++)
		{
			pos = s1.find(":");
			nspace = s1.substr(0, pos);
			s1 = s1.substr(pos + 1);
		}
		EIT *eit = eDVB::getInstance()->getEIT();
		if (eit)
		{
			int p=0;
			for (ePtrList<EITEvent>::iterator i(eit->events); i != eit->events.end(); ++i)
			{
				EITEvent *event=*i;
				if ((event->running_status>=2) || ((!p) && (!event->running_status)))
				{
					for (ePtrList<Descriptor>::iterator d(event->descriptor); d != event->descriptor.end(); ++d)
					{
						if (d->Tag() == DESCR_LINKAGE)
						{
							LinkageDescriptor *ld =(LinkageDescriptor *)*d;
							if (ld->linkage_type == 0xB0) //subchannel
							{
								eString subService((char*)ld->private_data, ld->priv_len);
								eString subServiceRef = "1:0:7:" + eString().sprintf("%x", ld->service_id) + ":" + eString().sprintf("%x", ld->transport_stream_id) + ":" + eString().sprintf("%x", ld->original_network_id) + ":"
									+ eString(nspace) + ":0:0:0:";
								if (subServiceRef == curServiceRef)
									subChannel = subService;
							}
						}
					}
				}
				++p;
			}
			eit->unlock();
		}
	}
	return subChannel;
}

eString getSubChannels(void)
{
	eString result;
	eString curServiceRef = ref2string(eServiceInterface::getInstance()->service);
	if (curServiceRef)
	{
		eString s1 = curServiceRef; int pos; eString nspace;
		for (int i = 0; i < 7 && s1.find(":") != eString::npos; i++)
		{
			pos = s1.find(":");
			nspace = s1.substr(0, pos);
			s1 = s1.substr(pos + 1);
		}
		EIT *eit = eDVB::getInstance()->getEIT();
		if (eit)
		{
			int p = 0;
			for (ePtrList<EITEvent>::iterator i(eit->events); i != eit->events.end(); ++i)
			{
				EITEvent *event = *i;
				if ((event->running_status >= 2) || ((!p) && (!event->running_status)))
				{
					for (ePtrList<Descriptor>::iterator d(event->descriptor); d != event->descriptor.end(); ++d)
					{
						if (d->Tag() == DESCR_LINKAGE)
						{
							LinkageDescriptor *ld = (LinkageDescriptor *)*d;
							if (ld->linkage_type == 0xB0) //subchannel
							{
								eString subService((char *)ld->private_data, ld->priv_len);
								eString subServiceRef = "1:0:7:" + eString().sprintf("%x", ld->service_id) + ":" + eString().sprintf("%x", ld->transport_stream_id) + ":" + eString().sprintf("%x", ld->original_network_id) + ":"
									+ eString(nspace) + ":0:0:0:";
								if (subServiceRef == curServiceRef)
									result += "<option selected value=\"" + subServiceRef + "\">";
								else
									result += "<option value=\"" + subServiceRef + "\">";
								result += subService;
								result += "</option>";
							}
						}
					}
				}
				++p;
			}
			eit->unlock();
		}
	}
	if (!result)
		result = "<option>none</option>";

	return result;
}

static eString selectSubChannel(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	content->local_header["Content-Type"] = "text/html; charset=utf-8";
	content->local_header["Cache-Control"] = "no-cache,no-store,must-revalidate,max-age=1";
	eString subChannels = getSubChannels();

	eString result = readFile(TEMPLATE_DIR + "subChannelSelection.tmp");
	result.strReplace("#SUBCHANS#", subChannels);
	result.strReplace("#SELECTBUTTON#", button(100, "Select", TOPNAVICOLOR, "javascript:subChannelChange()", "#000000"));

	return result;
}

eString getCurService(void)
{
	eString result;

	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		eService *current = eDVB::getInstance()->settings->getTransponders()->searchService(sapi->service);
		if (current)
			result = current->service_name;
	}
	return filter_string(result);
}

eString getChanNavi(bool showVLCButton)
{
	eString result;
	result += button(100, _("EPG"), RED, "javascript:openEPG('')", "#FFFFFF");
	result += button(100, _("Video"), GREEN, "javascript:selectSubChannel()", "#FFFFFF");
	result += button(100, _("Audio"), YELLOW, "javascript:selectAudio()", "#FFFFFF");
	result += button(100, _("Info"), BLUE, "javascript:openChannelInfo()", "#FFFFFF");
	result += button(100, _("Stream Info"), TOPNAVICOLOR, "javascript:openSI()", "#000000");
	if (showVLCButton)
		result += button(100, _("VLC"), TOPNAVICOLOR, "javascript:vlc()", "#000000");
	return result;
}

eString getTopNavi()
{
	eString result;
	eString pre, post;

	if (pdaScreen == 0)
	{
		pre = "javascript:topnavi('";
		post = "')";
	}

	result += button(100, _("ZAP"), TOPNAVICOLOR, pre + "?mode=zap" + post);
	result += button(100, _("TIMERS"), TOPNAVICOLOR, pre + "?mode=timers" + post);
	result += button(100, _("CONTROL"), TOPNAVICOLOR, pre + "?mode=control" + post);
	if (pdaScreen == 0)
	{
#ifdef ENABLE_EXPERT_WEBIF
		result += button(100, _("CONFIG"), TOPNAVICOLOR, pre + "?mode=config" + post);
#endif
	}
	result += button(100, _("HELP"), TOPNAVICOLOR, pre + "?mode=help" + post);

	return result;
}

eString getZapNaviButton(eString name, int subMode)
{
	if (zapSubMode == subMode)
		return button(99, name, "buttonSel.png",
				"javascript:zapnavi('?mode=zap&zapmode=" + eString().sprintf("%d", zapMode) + "&zapsubmode=" + eString().sprintf("%d", subMode) + "')",
				"#444444", false, 19, "no-repeat", 0, 1, 0, "bold");
	else
		return button(99, name, "buttonDes.png",
				"javascript:zapnavi('?mode=zap&zapmode=" + eString().sprintf("%d", zapMode) + "&zapsubmode=" + eString().sprintf("%d", subMode) + "')",
				"#666666", false, 18, "no-repeat", 0, 1, -1);
// button(int width, eString buttonText, eString buttonColor, eString buttonRef, eString color, bool xml, int height, eString bgrepeat, int border, int margin_right, int margin_bottom, eString font_weight)
}

eString getZapNavi()
{
	eString result;

	if (zap[zapMode][ZAPSUBMODEALLSERVICES])
	{
		result += getZapNaviButton(_("All Services"), ZAPSUBMODEALLSERVICES );
	}
	if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite && zap[zapMode][ZAPSUBMODESATELLITES])
	{
		result += getZapNaviButton(_("Satellites"), ZAPSUBMODESATELLITES);
	}
	if (zap[zapMode][ZAPSUBMODEPROVIDERS])
	{
		result += getZapNaviButton(_("Providers"), ZAPSUBMODEPROVIDERS);
	}
	if (zap[zapMode][ZAPSUBMODEBOUQUETS])
	{
		result += getZapNaviButton(_("Bouquets"), ZAPSUBMODEBOUQUETS);
	}

	return result;
}


eString getLeftNavi(eString mode)
{
	eString result;
	eString pre, post;

	if (pdaScreen == 0)
	{
		pre = "javascript:leftnavi('";
		post = "')";
	}

	if (mode.find("zap") == 0)
	{
		if (pdaScreen == 0)
		{
			result += button(110, _("TV"), LEFTNAVICOLOR, pre + "?mode=zap&zapmode=" + eString().sprintf("%d", ZAPMODETV) + "&zapsubmode=" + eString().sprintf("%d", ZAPSUBMODEBOUQUETS) + post, "#000000");
			result += "<br>";
			result += button(110, _("Radio"), LEFTNAVICOLOR, pre + "?mode=zap&zapmode=" + eString().sprintf("%d", ZAPMODERADIO) + "&zapsubmode=" + eString().sprintf("%d", ZAPSUBMODEBOUQUETS) + post, "#000000");
			result += "<br>";
			result += button(110, _("Data"), LEFTNAVICOLOR, pre + "?mode=zap&zapmode=" + eString().sprintf("%d", ZAPMODEDATA) + "&zapsubmode=" + eString().sprintf("%d", ZAPSUBMODESATELLITES) + post, "#000000");
			result += "<br>";
#ifndef DISABLE_FILE
			result += button(110, _("Movies"), LEFTNAVICOLOR, pre + "?mode=zap&zapmode=" + eString().sprintf("%d", ZAPMODERECORDINGS) + "&zapsubmode=" + eString().sprintf("%d", ZAPSUBMODECATEGORY) + post, "#000000");
			result += "<br>";
			result += button(110, _("Root"), LEFTNAVICOLOR, pre + "?mode=zap&zapmode=" + eString().sprintf("%d", ZAPMODEROOT) + "&zapsubmode=" + eString().sprintf("%d", ZAPSUBMODECATEGORY) + post, "#000000");
#endif
#ifdef ENABLE_EXPERT_WEBIF
			result += "<br>";
			result += button(110, _("Stream"), LEFTNAVICOLOR, pre + "?mode=zap&zapmode=" + eString().sprintf("%d", ZAPMODESTREAMING) + "&zapsubmode=" + eString().sprintf("%d", ZAPSUBMODECATEGORY) + post, "#000000");
#endif
		}
		else
		{
			result += button(110, _("TV"), LEFTNAVICOLOR, "?mode=zap&path=0:7:1:0:0:0:0:0:0:0:");
			result += "<br>";
			result += button(110, _("Radio"), LEFTNAVICOLOR, "?mode=zap&path=0:7:2:0:0:0:0:0:0:0:");
			result += "<br>";
			result += button(110, _("Data"), LEFTNAVICOLOR, "?mode=zap&path=0:7:6:0:0:0:0:0:0:0:");
#ifndef DISABLE_FILE
			result += "<br>";
			result += button(110, _("Root"), LEFTNAVICOLOR, "?mode=zap&path=2:47:0:0:0:0:%2f");
			result += "<br>";
			result += button(110, _("Movies"), LEFTNAVICOLOR, "?mode=zap&path=4097:7:0:1:0:0:0:0:0:0:");
#endif
		}
	}
	else
	if (mode.find("control") == 0)
	{
		result += button(110, _("Message2TV"), LEFTNAVICOLOR, "javascript:sendMessage2TV()");
		result += "<br>";
#ifdef DEBUG
		int disableSerialOutput = 0;
		eConfig::getInstance()->getKey("/ezap/extra/disableSerialOutput", disableSerialOutput);
		if (disableSerialOutput == 0)
		{
			result += button(110, _("Logging"), LEFTNAVICOLOR, "javascript:logging()");
			result += "<br>";
		}
#endif
		result += button(110, _("Satfinder"), LEFTNAVICOLOR, pre + "?mode=controlSatFinder" + post);
		switch (eSystemInfo::getInstance()->getHwType())
		{
			case eSystemInfo::dbox2Nokia:
			case eSystemInfo::dbox2Sagem:
			case eSystemInfo::dbox2Philips:
				result += "<br>";
				result += button(110, _("Remote Control"), LEFTNAVICOLOR, "javascript:remoteControl('dbox2')");
				break;
			default:
				if (eSystemInfo::getInstance()->hasKeyboard())
				    result += "<br>"+button(110, _("Remote Control"), LEFTNAVICOLOR, "javascript:remoteControl('dreambox')");
				break;
		}
	}
	else
	if (mode.find("config") == 0)
	{
#ifdef ENABLE_EXPERT_WEBIF
		if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7000)
			result += button(110, _("Boot Manager"), LEFTNAVICOLOR, pre + "?mode=configBoot" + post);
		result += button(110, _("Mount Manager"), LEFTNAVICOLOR, pre + "?mode=configMountMgr" + post);
		result += "<br>";
// Only enable Flash Manager in webinterface when bootmenue is available
// Whenever we put it back in the image it is available here automatically
		if (access("/bin/bootmenue", X_OK) == 0)
		{
			result += button(110, _("Flash Manager"), LEFTNAVICOLOR, pre + "?mode=configFlashMgr" + post);
			result += "<br>";
		}
		result += button(110, _("Settings"), LEFTNAVICOLOR, pre + "?mode=configSettings" + post);
// Only show Rotor button when rotor is enabled in config
		eSatelliteConfigurationManager satconfig(false);
		if (satconfig.getRotorEnabled())
		{
			result += "<br>";
			result += button(110, _("Rotor"), LEFTNAVICOLOR, pre + "?mode=configRotor" + post);
		}
// Only enable extra webserver in webinterface when chttpd is available
// Whenever we put it back in the image it is available here automatically
		if (access("/bin/chttpd", X_OK) == 0 || access("/var/bin/chttpd", X_OK) == 0)
		{
			result += "<br>";
			result += button(110, _("Web Server"), LEFTNAVICOLOR, pre + "?mode=configWebserver" + post);
		}
#endif
	}
	else
	if (mode.find("help") == 0)
	{
		if (eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000)
		{
			result += button(110, "DMM Sites", LEFTNAVICOLOR, pre + "?mode=helpDMMSites" + post);
			result += "<br>";
			result += button(110, "Other Sites", LEFTNAVICOLOR, pre + "?mode=helpOtherSites" + post);
			result += "<br>";
		}
		result += button(110, "Forums", LEFTNAVICOLOR, pre + "?mode=helpForums" + post);
		if (eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000)
		{
			result += "<br><br>";
			result += button(110, "Images", LEFTNAVICOLOR, pre + "?mode=helpUpdatesInternet" + post);
		}
	}

#if 0
	if (pdaScreen == 0)
	{
		result += "<img src=\"jewel.gif\" border=\"0\" width=\"110\">";
	}
	result += "<center><b>Emerald beta</b></center>";
	result += "&nbsp;";
#endif
	return result;
}

eString getScrOsdNaviButton(int pdaScreen, eString name, eString shot)
{	
        eString pre, post;

        if (pdaScreen == 0)
        {
                pre = "body";
                post = "";
        }

	return button(99, name, "buttonSel.png",
			pre + "?mode=" + shot + post,
			"#444444", false, 19, "no-repeat", 0, 0, 0, "bold");
#if sel
	return button(99, name, "buttonDes.png",
			pre + "?mode=" + shot + post,
			"#666666", false, 18, "no-repeat", 0, 0, -1);
#endif
}

eString getScrOsdNavi(int pdaScreen)
{
	eString result;

	result += getScrOsdNaviButton(pdaScreen, _("OSD Shot"), "controlFBShot");
	result += getScrOsdNaviButton(pdaScreen, _("Screen Shot"), "controlScreenShot");
	if (pdaScreen)
		result += "<br>";
	result += getScrOsdNaviButton(pdaScreen, _("Screen + OSD"), "controlScreenShot&blendtype=2");

	switch( eSystemInfo::getInstance()->getHwType() )
	{
		case eSystemInfo::DM500:
		case eSystemInfo::DM5600:
		case eSystemInfo::DM5620:
		case eSystemInfo::DM600PVR:
		{
			// Do nothing
			break;
		}
		default:	
			// On the other boxes we want LCD shot possibility
			result += getScrOsdNaviButton(pdaScreen, _("LCD Shot"), "controlLCDShot");
	}

	return result;
}

static eString setVolume(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	eString mute = opt["mute"];
	eString volume = opt["volume"];

	if (mute)
		eAVSwitch::getInstance()->toggleMute();

	if (volume)
	{
		int vol = atoi(volume.c_str());
		eAVSwitch::getInstance()->changeVolume(1, 63 - vol);
	}

	return closeWindow(content, "", 500);
}

static eString setVideo(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	eString video = opt["position"];
	if (video)
	{
		int vid = atoi(video.c_str()); // 1..20

		eServiceHandler *handler=eServiceInterface::getInstance()->getService();
		if (handler)
		{
			int total = handler->getPosition(eServiceHandler::posQueryLength);
			int current = handler->getPosition(eServiceHandler::posQueryCurrent);
			int skipTime = ((total * vid) / 20) - current;

			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekBegin));
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSkip, skipTime * 1000));
			handler->serviceCommand(eServiceCommand(eServiceCommand::cmdSeekEnd));
		}
	}

	return closeWindow(content, "", 500);
}

eString getBoxInfo(eString, eString);
static eString getTimers()
{
	return getTimerList("HTML");
}

static eString getHelpUpdatesInternet()
{
	eString versionFile = "/.version";

	if (eSystemInfo::getInstance()->isOpenEmbedded())
	{
		versionFile = "/etc/image-version";
	}

	std::stringstream result;
	eString imageName = "&nbsp;", imageVersion = "&nbsp;", imageURL = "&nbsp;", imageCreator = "&nbsp;", imageMD5 = "&nbsp;";
	eString myCatalogURL = getAttribute(versionFile, "catalog");

	if (!(eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7020) && myCatalogURL.length())
	{
		system(eString("wget -q -O /tmp/catalog.xml " + myCatalogURL).c_str());
		ifstream catalogFile("/tmp/catalog.xml");
		if (catalogFile)
		{
			result  << "<table id=\"epg\" width=\"100%\" border=\"1\" cellpadding=\"5\" cellspacing=\"0\">"
				<< "<thead>"
				<< "<tr>"
				<< "<th colspan=\"2\">Available Images</th>"
				<< "</tr>"
				<< "</thead>"
				<< "<tbody>";
			eString line;
			while (getline(catalogFile, line, '\n'))
			{
				if (line.find("<image") != eString::npos)
				{
					if (imageVersion != "&nbsp;")
					{
						result  << "<tr>"
							<< "<td>" << imageVersion << "</td>"
							<< "<td>" << imageName << "</td>"
							<< "</tr>";
					}
					imageName = "&nbsp;";
					imageVersion = "&nbsp;";
					imageURL = "&nbsp;";
					imageCreator = "&nbsp;";
					imageMD5 = "&nbsp;";
				}
				else
				if (line.find("version=") != eString::npos)
				{
					imageVersion = getRight(line, '"');
					imageVersion = getLeft(imageVersion, '"');
				}
				else
				if (line.find("name=") != eString::npos)
				{
					imageName = getRight(line, '"');
					imageName = getLeft(imageName, '"');
				}
			}
			result 	<< "</tbody>"
				<< "</table>";
		}
		else
			result << "No image information available.";
	}
	else
		result << "No image information available.";

	return result.str();
}

#ifndef DISABLE_FILE
static eString deleteMovie(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');

	eString sref = opt["ref"];
	eZapMain::getInstance()->deleteFile(string2ref(sref));

	return closeWindow(content, "Please wait...", 2000);
}

static eString renameMovie(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');

	eString sref = opt["ref"];
	eString newname = opt["desc"];
	eServiceReference ref = string2ref(sref);

	if (newname)
	{
		eString dir = ref.path.left(ref.path.find_last_of("/") + 1);
		eZapMain::getInstance()->renameFile(ref.path, eString(dir + newname + ".ts"), newname);
	}

	return closeWindow(content, "Please wait...", 100);
}
#endif

class myService
{
public:
	eString serviceRef;
	eString serviceName;
	myService(eString sref, eString sname)
	{
		serviceRef = sref;
		serviceName = sname;
	};
	~myService() {};
	bool operator < (const myService &a) const {return serviceName < a.serviceName;}
};

void genHTMLServicesList(std::list <myService> &myList, eString &serviceRefList, eString &serviceList)
{
	std::list <myService>::iterator myIt;
	eString serviceRef, serviceName;

	serviceRefList = "";
	serviceList = "";

	if (myList.size() > 0)
	{
		for (myIt = myList.begin(); myIt != myList.end(); ++myIt)
		{
			serviceRefList += "\"" + myIt->serviceRef + "\", ";
			serviceList += "\"" + myIt->serviceName + "\", ";
		}
	}
	else
	{
		serviceRefList = "\"none\", ";
		serviceList = "\"none\", ";
	}

	serviceRefList = serviceRefList.left(serviceRefList.length() - 2);
	serviceList = serviceList.left(serviceList.length() - 2);
}

class eWebNavigatorListDirectory2: public Object
{
	std::list <myService> &myList;
	eString path;
	eServiceInterface &iface;
	bool addEPG;
	bool forceAll;
	bool addSatPos;
	epgAtTime *epgData;
public:
	eWebNavigatorListDirectory2(std::list <myService> &myList, eString path, eServiceInterface &iface, bool addEPG, epgAtTime *epgData, bool forceAll, bool addSatPos): myList(myList), path(path), iface(iface), addEPG(addEPG), forceAll(forceAll), addSatPos(addSatPos), epgData(epgData)
	{
//		eDebug("[eWebNavigatorListDirectory2:] path: %s", path.c_str());
	}
	void addEntry(const eServiceReference &e)
	{
		// sorry.. at moment we dont show any directory.. or locked service in webif
		if (e.isLocked() && pinCheck::getInstance()->pLockActive())
			return;
#ifndef DISABLE_FILE
		if (!forceAll)
		{
			if (eDVB::getInstance()->recorder && !eZapMain::getInstance()->isRecordingPermanentTimeshift() && !e.path && !e.flags)
			{
				if (!onSameTP(eDVB::getInstance()->recorder->recRef,(eServiceReferenceDVB&)e))
					return;
			}
		}
#endif
		eString short_description, event_start, event_duration;

		eServiceReferenceDVB &ref = (eServiceReferenceDVB&)e;
		if (addEPG && epgData)
		{
			int tsidonid = (ref.getTransportStreamID().get()<<16)|ref.getOriginalNetworkID().get();
			const eventData* evt( epgData->channelEpg( ref ) );
			if (evt)
			{
				EITEvent event(evt->get(),tsidonid, evt->type);
				LocalEventData led;
				led.getLocalData(&event, &short_description);
				tm t = *localtime(&event.start_time);
				event_start = getTimeStr(&t, 0);
				event_duration = eString().sprintf("%d", event.duration / 60);
			}
		}

		eString tmp;
		if (ref.descr)
			tmp = filter_string(ref.descr);
		else
		{
			eService *service = iface.addRef(e);
			if (service)
			{
				tmp = filter_string(service->service_name);
				iface.removeRef(e);
			}
		}

		if (addEPG && short_description)
			tmp = tmp + " - " + event_start + " (" + event_duration + ") " + filter_string(short_description);
		tmp.strReplace("\"", "'");
		tmp.strReplace("\n", "-");

		if (zapMode == ZAPMODERECORDINGS)
		{
			eString r = e.toString();
			eString movieLocation;
			
			eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, movieLocation);
			tmp = "[" + eString().sprintf("%05lld", 
				getMovieSize(r.right(r.length() - r.find(movieLocation))) / 
				1024 / 1024) + " MB] " + tmp;
		}

		if (addSatPos)
		{
			if (!(e.flags & eServiceReference::isDirectory))
			{
				int orbitalPosition = e.data[4] >> 16;
				tmp = eString().sprintf("(%d.%d%c) ", abs(orbitalPosition / 10), abs(orbitalPosition % 10), orbitalPosition > 0 ? 'E' : 'W') + tmp;
			}
		}

		if (!(e.data[0] == -1 && e.data[2] != (int)0xFFFFFFFF) && tmp)
			myList.push_back(myService(ref2string(e), tmp));
	}
};

eString getZapContent(eString path, int depth, bool addEPG, bool sortList, bool forceAll)
{
	std::list <myService> myList, myList2;
	std::list <myService>::iterator myIt;
	eString result, result1, result2;
	eString bouquets, bouquetrefs, channels, channelrefs;
	bool addSatPos = false;

	eServiceReference current_service = string2ref(path);
	eServiceInterface *iface = eServiceInterface::getInstance();

	if (zapMode == ZAPMODERECORDINGS)
	{
		// reload recordings
		eZapMain::getInstance()->getRecordings()->lockPlaylist();
		eZapMain::getInstance()->loadRecordings();
		eZapMain::getInstance()->getRecordings()->unlockPlaylist();
	}

	if (!(current_service.flags&eServiceReference::isDirectory))	// is playable
	{
		playService(current_service);
		result = "";
	}
	else
	{
		if (zapMode == ZAPMODETV || zapMode == ZAPMODERADIO || zapMode == ZAPMODEDATA)
		{
			int showSatPos = 1;
			eConfig::getInstance()->getKey("/extras/showSatPos", showSatPos);
			addSatPos = (showSatPos == 1 && eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite);
		}
		// first pass thru is to get all user bouquets. Guess bouquets don't have EPG/Sat so don't ask for it...
		myList.clear();
		eWebNavigatorListDirectory2 navlist(myList, path, *iface, false, 0, forceAll, false);
		Signal1<void, const eServiceReference&> signal;
		signal.connect(slot(navlist, &eWebNavigatorListDirectory2::addEntry));
		iface->enterDirectory(current_service, signal);
//		eDebug("entered");
		iface->leaveDirectory(current_service);
//		eDebug("exited");

		if (sortList)
			myList.sort();

		genHTMLServicesList(myList, result1, result2);
		bouquetrefs = result1;
		bouquets = result2;

		if (depth > 1)
		{
			// go thru all bouquets to get the channels
			int i = 0;
			epgAtTime *epgData = 0;
			if (addEPG)
				epgData = new epgAtTime(eEPGCache::getInstance()->getEpgAtTime(time(0)+eDVB::getInstance()->time_difference));

			for (myIt = myList.begin(); myIt != myList.end(); myIt++)
			{
				result1 = ""; result2 = "";
				path = myIt->serviceRef;
				if (path)
				{
					eServiceReference current_service = string2ref(path);

					myList2.clear();
					eWebNavigatorListDirectory2 navlist(myList2, path, *iface, addEPG, epgData, forceAll, addSatPos);
					Signal1<void, const eServiceReference&> signal;
					signal.connect(slot(navlist, &eWebNavigatorListDirectory2::addEntry));

					iface->enterDirectory(current_service, signal);
//					eDebug("entered");
					iface->leaveDirectory(current_service);
//					eDebug("exited");

					if (sortList)
						myList2.sort();

					genHTMLServicesList(myList2, result1, result2);

					channels += "channels[";
					channels += eString().sprintf("%d", i);
					channels += "] = new Array(";
					channelrefs += "channelRefs[";
					channelrefs += eString().sprintf("%d", i);
					channelrefs += "] = new Array(";

					channels += result2;
					channels += ");";
					channelrefs += result1;
					channelrefs += ");";

					channels += "\n";
					channelrefs += "\n";
					i++;
				}
			}
			delete epgData;
		}
		else
		{
			channels = "channels[0] = new Array(" + bouquets + ");";
			channelrefs = "channelRefs[0] = new Array(" + bouquetrefs + ");";
			bouquets = "\"Dummy bouquet\"";
			bouquetrefs = "\"Dummy bouquet ref\"";
		}

		result = readFile(HTDOCS_DIR + "zapdata.js");
		result.strReplace("#BOUQUETS#", bouquets);
		result.strReplace("#BOUQUETREFS#", bouquetrefs);
		result.strReplace("#CHANNELS#", channels);
		result.strReplace("#CHANNELREFS#", channelrefs);
		result.strReplace("#CURRENTBOUQUET#", eString().sprintf("%d", currentBouquet));
		result.strReplace("#CURRENTCHANNEL#", eString().sprintf("%d", currentChannel));
		int autobouquetchange = 0;
		eConfig::getInstance()->getKey("/elitedvb/extra/autobouquetchange", autobouquetchange);
		result.strReplace("#AUTOBOUQUETCHANGE#", eString().sprintf("%d", autobouquetchange));
		result.strReplace("#ZAPMODE#", eString().sprintf("%d", zapMode));
		result.strReplace("#ZAPSUBMODE#", eString().sprintf("%d", zapSubMode));
	}

	return result;
}

static eString getZap(eString path)
{
	eString result, tmp;
	int selsize = 0;

	if (pdaScreen == 0)
	{
#ifndef DISABLE_FILE
		if (zapMode == ZAPMODERECORDINGS) // recordings
		{
			result = readFile(TEMPLATE_DIR + "movies.tmp");
			result.strReplace("#ZAPDATA#", getZapContent(path, 1, false, false, false));
			selsize = (screenWidth > 1024) ? 25 : 10;
#ifdef ENABLE_EXPERT_WEBIF
			tmp = readFile(TEMPLATE_DIR + "movieSources.tmp");

			eString movieSources;
			movieSources += eDevMountMgr::getInstance()->listMovieSources();
			movieSources += eNetworkMountMgr::getInstance()->listMovieSources();
			
			tmp.strReplace("#OPTIONS#", movieSources);
#endif
			result.strReplace("#MOVIESOURCES#", tmp);
			tmp = button(100, "Delete", RED, "javascript:deleteMovie()", "#FFFFFF");
			result.strReplace("#DELETEBUTTON#", tmp);
			tmp = button(100, "Download", GREEN, "javascript:downloadMovie()", "#FFFFFF");
			result.strReplace("#DOWNLOADBUTTON#", tmp);
			tmp = button(100, "VLC", YELLOW, "javascript:streamMovie()", "#FFFFFF");
			result.strReplace("#STREAMBUTTON#", tmp);
			tmp = button(100, "Recover", BLUE, "javascript:recoverMovies()", "#FFFFFF");
			result.strReplace("#RECOVERBUTTON#", tmp);
			tmp = button(100, "Rename", TOPNAVICOLOR, "javascript:renameMovie()", "#000000");
			result.strReplace("#RENAMEBUTTON#", tmp);
		}
		else
#endif
		if (zapMode == ZAPMODEROOT) // root
		{
			result = readFile(TEMPLATE_DIR + "root.tmp");
			eString tmp = getZapContent(path, 1, false, false, false);
			if (tmp)
			{
				result.strReplace("#ZAPDATA#", tmp);
				selsize = (screenWidth > 1024) ? 25 : 10;
			}
			else
				result = "";
		}
		else
#ifdef ENABLE_EXPERT_WEBIF
		if (zapMode == ZAPMODESTREAMING)
		{
			result = getStreamingServer();
		}
		else
#endif
		{
			result = readFile(TEMPLATE_DIR + "zap.tmp");
			bool sortList = (zapSubMode ==  ZAPSUBMODESATELLITES || zapSubMode == ZAPSUBMODEPROVIDERS || zapSubMode == ZAPSUBMODEALLSERVICES);
			int columns = (zapSubMode == ZAPSUBMODEALLSERVICES) ? 1 : 2;
			int showChannelEPG = 0;
			eConfig::getInstance()->getKey("/ezap/webif/showChannelEPG", showChannelEPG);
			result.strReplace("#ZAPDATA#", getZapContent(path, columns, showChannelEPG, sortList, false));
			selsize = (screenWidth > 1024) ? 30 : 15;
			if (columns == 1)
			{
				result.strReplace("#WIDTH1#", "0");
				result.strReplace("#WIDTH2#", "630");
			}
			else
			{
				result.strReplace("#WIDTH1#", "200");
				result.strReplace("#WIDTH2#", "430");
			}
			tmp = button(100, "EPG-Overview", RED, "javascript:mepg()", "#FFFFFF");
			result.strReplace("#ZAPNAVI#", getZapNavi());
			result.strReplace("#MEPGBUTTON#", tmp);
		}
		result.strReplace("#SELSIZE#", eString().sprintf("%d", selsize));
	}
	else
	{
		eString tmp = getPDAZapContent(path);
		result = (tmp) ? getEITC(readFile(TEMPLATE_DIR + "eit_small.tmp"), "HTML") + tmp : "";
	}

	return result;
}

#ifndef DISABLE_FILE
eString getDiskInfo(int html)
{
	eString sharddisks = "";
	if (eSystemInfo::getInstance()->hasHDD())
	{
		for (int c = 'a'; c < 'h'; c++)
		{
			char line[1024];
			FILE *f = fopen(eString().sprintf("/proc/ide/hd%c/media", c).c_str(), "r");
			if (!f)
				continue;
			fgets(line, 1024, f);
			fclose(f);
			if (!strcmp(line, "disk\n"))
			{
				FILE *f = fopen(eString().sprintf("/proc/ide/hd%c/model", c).c_str(), "r");
				if (!f)
					continue;
				*line = 0;
				fgets(line, 1024, f);
				fclose(f);
				if (!*line)
					continue;
				line[strlen(line) - 1] = 0;
				if (sharddisks != "")
					sharddisks += html ? "<br>" : "\n";
				sharddisks += line;
				f = fopen(eString().sprintf("/proc/ide/hd%c/capacity", c).c_str(), "r");
				if (!f)
					continue;
				int capacity = 0;
				fscanf(f, "%d", &capacity);
				fclose(f);
				sharddisks += " (";
				sharddisks += (c & 1) ? "master" : "slave";
				if (capacity)
				{
					if( capacity > 2*1024*1024 )
						sharddisks += eString().sprintf(", %d GB", capacity / 2048 / 1024);
					else
						sharddisks += eString().sprintf(", %d MB", capacity / 2048);
				}
				sharddisks += ")";
			}
		}
	}
	if (sharddisks == "")
		sharddisks = "none";
	return sharddisks;
}

eString getUSBInfo(void)
{
	eString usbStick = "none";
	eString line;
	ifstream infile("/proc/scsi/usb-storage/0");
	if (infile)
	{
		usbStick = "";
		while (getline(infile, line, '\n'))
		{
			if (line.find("Vendor:") != eString::npos)
				usbStick += "Vendor =" + getRight(line, ':');
			if (line.find("Product:") != eString::npos)
			{
				usbStick += ", ";
				usbStick += "Product =" + getRight(line, ':');
			}
		}
	}
	return usbStick;
}
#endif

eString getBoxInfo(eString skelleton, eString format)
{
	eString versionFile = "/.version";

	if (eSystemInfo::getInstance()->isOpenEmbedded())
	{
		versionFile = "/etc/image-version";
	}

	eString result = readFile(TEMPLATE_DIR + format + skelleton + ".tmp");

	result.strReplace("#VERSION#", getAttribute(versionFile, "version"));
	result.strReplace("#CATALOG#", getAttribute(versionFile, "catalog"));
	result.strReplace("#COMMENT#", getAttribute(versionFile, "comment"));
	result.strReplace("#URL#", getAttribute(versionFile, "url"));

	result.strReplace("#MODEL#", eSystemInfo::getInstance()->getModel());
	result.strReplace("#MANUFACTURER#", eSystemInfo::getInstance()->getManufacturer());
	result.strReplace("#PROCESSOR#", eSystemInfo::getInstance()->getCPUInfo());
#ifndef DISABLE_FILE
	result.strReplace("#DISK#", getDiskInfo(1));
	result.strReplace("#USBSTICK#", getUSBInfo());
#else
	result.strReplace("#DISK#", "none");
	result.strReplace("#USBSTICK#", "none");
#endif
	result.strReplace("#LINUXKERNEL#", readFile("/proc/version"));
	result.strReplace("#FIRMWARE#", firmwareLevel(getAttribute(versionFile, "version")));
	if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7000 ||
		eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7020)
		result.strReplace("#FP#", eString().sprintf(" 1.%02d", eDreamboxFP::getFPVersion()));
	else
		result.strReplace("#FP#", "n/a");
	result.strReplace("#WEBIFVERSION#", WEBIFVERSION);

	return result;
}

#ifndef	DISABLE_FILE
// Extract the description from the filename.
eString getDesc(const eString& str)
{
	unsigned int leftbound, rightbound;
	eString tbtrimmed;

	leftbound = str.find('-');
	rightbound = str.find(".ts", leftbound + 1);
	if ((rightbound == eString::npos) || (rightbound - leftbound < 1))
		tbtrimmed = str;
	else
		tbtrimmed = str.substr(leftbound + 1, rightbound - leftbound - 1);

	leftbound = tbtrimmed.find_first_not_of(' ');
	rightbound = tbtrimmed.find_last_not_of(' ');

	// If the extracted description is empty use the value of str as the description.
	if (rightbound - leftbound < 1)
	{
		tbtrimmed = str;
		leftbound = tbtrimmed.find_first_not_of(' ');
		rightbound = tbtrimmed.find_last_not_of(' ');
	}
	return tbtrimmed.substr(leftbound, rightbound - leftbound + 1);
}

// Recover index with recordings on harddisk in key /pli/mediaMapping/movies.
bool rec_movies()
{
	eString filen;
	int i;
	bool result = false;

	ePlaylist *recordings = eZapMain::getInstance()->getRecordings();
	recordings->lockPlaylist();
	std::list<ePlaylistEntry>& rec_list = recordings->getList();
	struct dirent **namelist;
	
	eString movieLocation;
	eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, movieLocation);
	int n = scandir(movieLocation.c_str(), &namelist, 0, alphasort);

	if (n > 0)
	{
		// Loop through all files, if not in list add to the list
		// in previous versions missing file were deleted here from recordings.epl 
		// but this is allready done when file is saved 
		for (i = 0; i < n; i++)
		{
			filen = namelist[i]->d_name;
			// For every .ts file
			if ((filen.length() >= 3) &&
				(filen.substr(filen.length()-3, 3).compare(".ts") == 0))
			{
				// Check if file is in the list.
				bool file_in_list = false;
				for (std::list<ePlaylistEntry>::iterator it(rec_list.begin()); it != rec_list.end(); ++it)
				{
					std::string::size_type location;
					location = it->service.path.find_last_of('/'); 
					if (location != std::string::npos)
					{	
						eString FileName = it->service.path.substr(location + 1);
						if (FileName == filen)
						{
							file_in_list = true;
							break;
						}
					} 
					
				}
				if (!file_in_list)	// Add file to list.
				{
					eString movieLocation;
					
					eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, movieLocation);
					eServicePath path("1:0:1:0:0:0:000000:0:0:0:" + movieLocation + "/" + filen);
					rec_list.push_back(path);
					rec_list.back().type = 16385;
					rec_list.back().service.descr = getDesc(filen);
					rec_list.back().service.path = movieLocation + "/" + filen;
					struct stat64 s;
					stat64(rec_list.back().service.path.c_str(), &s);
					((eServiceReferenceDVB&)rec_list.back().service).setFileTime(s.st_mtime);
				}
			}
			free(namelist[i]);
		}
		rec_list.sort();
		result = true;
		free(namelist);
	}

	recordings->save();
	recordings->unlockPlaylist();

	return result;
}

static eString recoverRecordings(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	eString result ="<html><head><title>Info</title></head><body onUnload=\"parent.window.opener.location.reload(true)\">Movies ";
 	result += rec_movies() ? "successfully" : "could not be";
	result += " recovered.</body></html>";
	return result;
}
#endif

eString getSatellitesAndTransponders(void)
{
	eTransponder *tp = NULL;
	eString bouquets, bouquetrefs; // satellites
	eString channels, channelrefs; // transponders
	eString chs, chrefs;
	eString transponder;
	int currentSatellite = -1, currentTransponder = -1;
	int j, k;

	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (sapi && sapi->transponder)
		tp = sapi->transponder;

	k = 0;
	for (std::list<eLNB>::iterator it2(eTransponderList::getInstance()->getLNBs().begin()); it2 != eTransponderList::getInstance()->getLNBs().end(); it2++)
	{
		// first go thru all satellites...
		for (ePtrList<eSatellite>::iterator s (it2->getSatelliteList().begin()); s != it2->getSatelliteList().end(); s++)
		{
			bouquets += "\"" + s->getDescription() + "\", ";
			bouquetrefs += "\"" + s->getDescription() + "\", ";
			if (tp && s->getOrbitalPosition() == tp->satellite.orbital_position)
			{
				//this is the current satellite...
				currentSatellite = k;
			}
			// enter sat into satellite list result1...
			channels += "channels[";
			channels += eString().sprintf("%d", k);
			channels += "] = new Array(";
			channelrefs += "channelRefs[";
			channelrefs += eString().sprintf("%d", k);
			channelrefs += "] = new Array(";
			chs = "";
			chrefs = "";

			j = 0;

			// then go thru all transponders...
			for (std::list<tpPacket>::iterator it3(eTransponderList::getInstance()->getNetworks().begin()); it3 != eTransponderList::getInstance()->getNetworks().end(); it3++)
			{
				if (it3->orbital_position == s->getOrbitalPosition())
				{
					for (std::list<eTransponder>::iterator it(it3->possibleTransponders.begin()); it != it3->possibleTransponders.end(); it++)
					{
						if (tp && *tp == *it)
						{
							// this is the current transponder
							currentTransponder = j;
						}
						transponder = eString().sprintf("%d / %d / %c", it->satellite.frequency / 1000, it->satellite.symbol_rate / 1000, it->satellite.polarisation ? 'V' : 'H');
						chs += "\"" + transponder + "\", ";
						chrefs += "\"" + it->satellite.toString() + "\", ";
					}
				}
				j++;
			}

			channels += chs.left(chs.length() - 2);
			channels += ");";
			channelrefs += chrefs.left(chrefs.length() - 2);
			channelrefs += ");";
			k++;
		}
	}
	bouquetrefs = bouquetrefs.left(bouquetrefs.length() - 2);
	bouquets = bouquets.left(bouquets.length() - 2);

	eString zapdata = readFile(HTDOCS_DIR + "zapdata.js");
	zapdata.strReplace("#BOUQUETS#", bouquets);
	zapdata.strReplace("#BOUQUETREFS#", bouquetrefs);
	zapdata.strReplace("#CHANNELS#", channels);
	zapdata.strReplace("#CHANNELREFS#", channelrefs);
	zapdata.strReplace("#CURRENTBOUQUET#", eString().sprintf("%d", currentSatellite));
	zapdata.strReplace("#CURRENTCHANNEL#", eString().sprintf("%d", currentTransponder));
	zapdata.strReplace("#AUTOBOUQUETCHANGE#", eString().sprintf("%d", 0)); // not used on client
	zapdata.strReplace("#ZAPMODE#", eString().sprintf("%d", -1));
	zapdata.strReplace("#ZAPSUBMODE#", eString().sprintf("%d", 0)); // not used on client

	eString result = readFile(TEMPLATE_DIR + "sat.tmp");
	result.strReplace("#ZAPDATA#", zapdata);
	result.strReplace("#SELSIZE#", screenWidth > 1024 ? "30" : "15");

	return result;
}

static eString getControlSatFinder(eString opts)
{
	eString result;
	std::map<eString, eString> opt = getRequestOptions(opts, '&');

	if (pdaScreen == 0)
		result = getSatellitesAndTransponders();
	else
	{
		// pda satfinder
		result += "<table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\">";
		eString display = opt["display"];
		if (display == "transponders")
			result += getTransponders(atoi(opt["sat"].c_str()));
		else
			result += getSatellites();
		result += "</table>";
	}
	return result;
}

#define CLAMP(x)     ((x < 0) ? 0 : ((x > 255) ? 255 : x))

inline unsigned short avg2(unsigned short a, unsigned short b)
{
	return
		(((a & 0xFF) + (b & 0xFF)) >> 1) |
		(((((a>>8) & 0xFF) + ((b>>8) & 0xFF)) >> 1) << 8);
}

struct blasel
{
	int hor, vert;
	char *name;
} subsamplings[]={
	{1, 1, "4:4:4"},
	{2, 1, "4:2:2"},
	{2, 2, "4:2:0"},
	{4, 2, "4:2:0-half"},
	{4, 1, "4:1:1"},
	{4, 4, "4:1:0"}};

int genScreenShot(int index, int blendtype)
{
	unsigned char frame[720 * 576 * 3 + 16]; // max. size

	int fd = open("/dev/video", O_RDONLY);
	if (fd < 0)
	{
		eDebug("genScreenShot: could not open /dev/video");
		return 1;
	}

	eString filename = "/tmp/screenshot" + ((index > 0) ? eString().sprintf("%d", index) : "") + ".bmp";
	FILE *fd2 = fopen(filename.c_str(), "wr");
	if (fd2 < 0)
	{
		eDebug("genScreenShot: could not open %s", filename.c_str());
		return 1;
	}

	int genhdr = 1;

	int r = read(fd, frame, 720 * 576 * 3 + 16);
	if (r < 16)
	{
		eDebug("genScreenShot: read video frame failed");
		return 1;
	}

	int *size = (int*)frame;
	int luma_x = size[0], luma_y = size[1];
	int chroma_x = size[2], chroma_y = size[3];

	unsigned char *luma = frame + 16;
	unsigned short *chroma = (unsigned short*)(frame + 16 + luma_x * luma_y);

	eDebug("genScreenShot: Picture resolution: %dx%d", luma_x, luma_y);

	int sub[2] = {luma_x / chroma_x, luma_y / chroma_y};

	int ssid;
	char *d = "unknown";
	for (ssid = 0; ssid < (int)(sizeof(subsamplings)/sizeof(*subsamplings)); ++ssid)
		if ((subsamplings[ssid].hor == sub[0]) && (subsamplings[ssid].vert == sub[1]))
		{
			d = subsamplings[ssid].name;
			break;
		}

	eDebug("genScreenShot: Chroma subsampling: %s", d);

	if (genhdr)
	{
		eDebug("genScreenShot: generating bitmap header.");
		unsigned char hdr[14 + 40];
		int i = 0;
#define PUT32(x) hdr[i++] = ((x)&0xFF); hdr[i++] = (((x)>>8)&0xFF); hdr[i++] = (((x)>>16)&0xFF); hdr[i++] = (((x)>>24)&0xFF);
#define PUT16(x) hdr[i++] = ((x)&0xFF); hdr[i++] = (((x)>>8)&0xFF);
#define PUT8(x) hdr[i++] = ((x)&0xFF);
		PUT8('B'); PUT8('M');
		PUT32((((luma_x * luma_y) * 3 + 3) &~ 3) + 14 + 40);
		PUT16(0); PUT16(0); PUT32(14 + 40);
		PUT32(40); PUT32(luma_x); PUT32(luma_y);
		PUT16(1);
		PUT16(24);
		PUT32(0); PUT32(0); PUT32(0); PUT32(0); PUT32(0); PUT32(0);
#undef PUT32
#undef PUT16
#undef PUT8
		fwrite(hdr, 1, i, fd2);
	}

	int x, y;

	// prepare for OSD pixmap
	gPixmap *pixmap = 0;
	pixmap = &gFBDC::getInstance()->getPixmap();
	int galpha = gFBDC::getInstance()->getAlpha();
	eDebug("genScreenShot: Have pixmap %s, alpha=%d blendtype=%d", pixmap ? "yes" : "no", galpha, blendtype);

#if defined(SCREENSHOT_PNG)
	gPixmap result;
	}
	result.bpp = 32;
	result.bypp = 4;
	result.stride = luma_x*4;
	result.x = luma_x;
	result.y = luma_y;
	result.data = malloc(luma_y * luma_x * 4);
	if (!result.data)
	{
		eDebug("genScreenShot: cannot allocate memory for screenpixmap");
		return 1;
	eDebug("genScreenShot: allocated pixmap");
#endif

	for (y = luma_y - 1; y >= 0; --y)
	{
		unsigned char line[luma_x * 3];
#if defined(SCREENSHOT_PNG)
		__u8 * resline = ((__u8 *)result.data) + y * result.stride;
#endif
		for (x = 0; x < luma_x; ++x)
		{
			int l = luma[y * luma_x + x];
			int c = 0x8080;
			switch (ssid)
			{
			case 0: // 4:4:4
				c = chroma[y * chroma_x + x];
				break;
			case 1: // 4:2:2
				if (!(x & 1))
					c = chroma[y * chroma_x + (x >> 1)];
				else
					c = avg2(chroma[y * chroma_x + (x >> 1)], chroma[y * chroma_x + (x >> 1) + 1]);
				break;
			case 2: // 4:2:0
				if (!((x|y) & 1))
					c = chroma[(y >> 1) * chroma_x + (x >> 1)];
				else if (!(y & 1))
					c = avg2(chroma[(y >> 1) * chroma_x + (x >> 1)], chroma[(y >> 1) * chroma_x + (x >> 1) + 1]);
				else if (!(x & 1))
					c = avg2(chroma[(y >> 1) * chroma_x + (x >> 1)], chroma[((y >> 1) + 1) * chroma_x + (x >> 1)]);
				else
					c = avg2(
						avg2(chroma[(y >> 1) * chroma_x + (x >> 1)], chroma[(y >> 1) * chroma_x + (x >> 1) + 1]),
						avg2(chroma[((y >> 1) + 1) * chroma_x + (x >> 1)], chroma[((y >> 1) + 1) * chroma_x + (x >> 1) + 1]));
				break;
			case 3:	// 4:2:0-half
				if (!(((x >> 1)|y) & 1))
					c = chroma[(y >> 1) * chroma_x + (x >> 2)];
				else if (!(y & 1))
					c = avg2(chroma[(y >> 1) * chroma_x + (x >> 2)], chroma[(y >> 1) * chroma_x + (x >> 2) + 1]);
				else if (!(x & 2))
					c = avg2(chroma[(y >> 1) * chroma_x + (x >> 2)], chroma[((y >> 1) + 1) * chroma_x + (x >> 2)]);
				else
					c = avg2(
						avg2(chroma[(y >> 1) * chroma_x + (x >> 2)], chroma[(y >> 1) * chroma_x + (x >> 2) + 1]),
						avg2(chroma[((y >> 1) + 1) * chroma_x + (x >> 2)], chroma[((y >> 1) + 1) * chroma_x + (x >> 2) + 1]));
				break;
			case 4:	// 4:1:1
				if (!((x >> 1) & 1))
					c = chroma[y * chroma_x + (x >> 2)];
				else
					c = avg2(chroma[y * chroma_x + (x >> 2)], chroma[y * chroma_x + (x >> 2) + 1]);
				break;
			case 5:
				if (!((x >> 1) & 1))
					c = chroma[(y >> 2) * chroma_x + (x >> 2)];
				else
					c = avg2(chroma[(y >> 2) * chroma_x + (x >> 2)], chroma[(y >> 2) * chroma_x + (x >> 2) + 1]);
				break;
			}

			signed char cr = (c & 0xFF) - 128;
			signed char cb = (c >> 8) - 128;

			l -= 16;

			int r, g, b;

			r = 104635 * cr + l * 76310;
			g = -25690 * cb - 53294 * cr + l * 76310;
			b = 132278 * cb + l * 76310;

			line[x * 3 + 2] = CLAMP(r >> 16);
			line[x * 3 + 1] = CLAMP(g >> 16);
			line[x * 3 + 0] = CLAMP(b >> 16);
#if defined(SCREENSHOT_PNG)
			resline[x * 4 + 0] = 255;
			resline[x * 4 + 1] = CLAMP(r >> 16);
			resline[x * 4 + 2] = CLAMP(g >> 16);
			resline[x * 4 + 3] = CLAMP(b >> 16);
#endif
		}

		if (blendtype && pixmap && y <= pixmap->y)
		{
			__u8 * prow = ((__u8 *)pixmap->data) + y*pixmap->stride;
			
			for (int px = 0; px < luma_x && px < pixmap->x; px++)
			{
				if (pixmap->bpp == 8) {
					int osdbyte = prow[px];
					if (osdbyte)
					{
						int alpha;
						switch (blendtype) {
							default:
							case 1: alpha = 0;
								break;
							case 2: alpha = pixmap->clut.data[osdbyte].a;
								break;
							case 3: alpha = pixmap->clut.data[osdbyte].a;
								alpha <<= 1; // OSD more transparent
								break;
							case 4: alpha = pixmap->clut.data[osdbyte].a;
								alpha >>= 1; // OSD more opaque
								break;
						}
						alpha = (alpha*galpha)/256;
        					int pr = pixmap->clut.data[osdbyte].r;
        					int pg = pixmap->clut.data[osdbyte].g;
        					int pb = pixmap->clut.data[osdbyte].b;
						line[px*3 + 2] = (line[px*3 + 2] * alpha + pr * (255-alpha)) >> 8; 
						line[px*3 + 1] = (line[px*3 + 1] * alpha + pg * (255-alpha)) >> 8; 
						line[px*3 + 0] = (line[px*3 + 0] * alpha + pb * (255-alpha)) >> 8; 
#if defined(SCREENSHOT_PNG)
						resline[px*4 + 1] = (resline[px*4 + 1] * alpha + pr * (255-alpha)) >> 8; 
						resline[px*4 + 2] = (resline[px*4 + 2] * alpha + pg * (255-alpha)) >> 8; 
						resline[px*4 + 3] = (resline[px*4 + 3] * alpha + pb * (255-alpha)) >> 8; 
#endif
					}
				}
        			else if (pixmap->bpp == 32)
        			{
					int osdbyte = ((__u32 *)prow)[px];
					if (osdbyte)
					{
						int alpha;
						switch (blendtype) {
							default:
							case 1: alpha = 255;
								break;
							case 2: alpha = (osdbyte >> 24) & 0xFF;
								break;
							case 3: alpha = (osdbyte >> 24) & 0xFF;
								alpha >>= 1; // OSD extra transparent
								break;
							case 4: alpha = (osdbyte >> 24) & 0xFF;
								alpha <<= 1; // OSD more opaque
								break;
						}
						line[px*3 + 2] = (line[px*3 + 2] * (255 - alpha) + ((osdbyte >> 16) & 0xFF) * alpha) >> 8; 
						line[px*3 + 1] = (line[px*3 + 1] * (255 - alpha) + ((osdbyte >> 8) & 0xFF) * alpha) >> 8; 
						line[px*3 + 0] = (line[px*3 + 0] * (255 - alpha) + (osdbyte & 0xFF) * alpha) >> 8; 
#if defined(SCREENSHOT_PNG)
						resline[px*4 + 1] = (resline[px*4 + 1] * (255 - alpha) + ((osdbyte >> 16) & 0xFF) * alpha) >> 8; 
						resline[px*4 + 2] = (resline[px*4 + 2] * (255 - alpha) + ((osdbyte >> 8) & 0xFF) * alpha) >> 8; 
						resline[px*4 + 3] = (resline[px*4 + 3] * (255 - alpha) + (osdbyte & 0xFF) * alpha) >> 8; 
#endif
					}
        			}
			}
		}

		fwrite(line, 1, luma_x * 3, fd2);
	}
	fclose(fd2);
#if defined(SCREENSHOT_PNG)
	filename = "/tmp/screenshot" + ((index > 0) ? eString().sprintf("%d", index) : "") + ".png";
	savePNG(filename.c_str(), &result);
	free(result.data);
	eDebug("genScreenShot: done");
#endif
	return 0;
}

static eString getControlScreenShot(eString opts)
{
	eString result;
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString display = opt["blendtype"];

	int blendType = atoi(display.c_str());
	int rc = genScreenShot(0, blendType);
	eDebug("getControlScreenshot: rc is %d", rc);
	if (rc != 0)
	{
		eDebug("could not generate /tmp/screenshot.bmp");
	}
	else
	{
		FILE *bitstream = 0;
		int xres = 0, yres = 0, aspect = 0, winxres = 0, winyres = 0, rh = 0, rv = 0;
		if (Decoder::current.vpid != -1)
			bitstream = fopen("/proc/bus/bitstream", "rt");
		if (bitstream)
		{
			char buffer[100];
			while (fgets(buffer, 100, bitstream))
			{
				if (!strncmp(buffer, "H_SIZE:  ", 9))
					xres = atoi(buffer+9);
				if (!strncmp(buffer, "V_SIZE:  ", 9))
					yres = atoi(buffer+9);
				if (!strncmp(buffer, "A_RATIO: ", 9))
					aspect = atoi(buffer+9);
			}
			fclose(bitstream);
			switch (aspect)
			{
				case 1:
					// square
					rh = 4; rv = 4; break;
				case 2:
					// 4:3
					rh = 4; rv = 3; break;
				case 3:
					// 16:9
					rh = 16; rv = 9; break;
				case 4:
					// 20:9
					rh = 20; rv = 9; break;
			}
		}

		winxres = (pdaScreen == 1) ? 160 : xres;
		if(blendType != 0)
		{
			// Something to blend, use size of OSD
			winyres = yres * winxres / xres;
		}
		else
		{
			// Nothing to blend, use an optimal aspect ratio
			winyres = rv * winxres / rh;
		}

		eDebug("[SCREENSHOT] xres = %d, yres = %d, rh = %d, rv = %d, winxres = %d, winyres = %d\n", xres, yres, rh, rv, winxres, winyres);

		result += "<img width=\"" +  eString().sprintf("%d", winxres);
		result += "\" height=\"" + eString().sprintf("%d", winyres);
		result += "\" src=\"/root/tmp/screenshot.bmp\" border=1>";
		result += "<br>";
		result += "Original format: " + eString().sprintf("%d", xres) + "x" + eString().sprintf("%d", yres);
		result += " (" + eString().sprintf("%d", rh) + ":" + eString().sprintf("%d", rv) + ")";
	}

	return result;
}

eString getContent(eString mode, eString path, eString opts)
{
	eString result, tmp;
	lastTransponder = "";

	if (mode == "zap")
	{
		tmp = "ZAP";
		if (pdaScreen == 0)
		{
			if (zapMode >= 0 && zapMode <= 5)
				tmp += ": " + zap[zapMode][ZAPSUBMODENAME];
			if (zapSubMode >= 2 && zapSubMode <= 5)
				tmp += " - " + zapSubModes[zapSubMode];
		}

		result = getTitle(tmp);
		tmp = getZap(path);
		if (tmp)
			result += tmp;
		else
			result = "";
	}
	else
#if ENABLE_EXPERT_WEBIF
	if (mode == "config")
	{
		result = getTitle("CONFIG");
		result += "Select one of the configuration categories on the left";
	}
	else
	if (mode == "configFlashMgr")
	{
		result = getTitle("CONFIG: Flash Manager");
		result += getConfigFlashMgr();
	}
	else
	if (mode == "configMountMgr")
	{
		result = getTitle("CONFIG: Mount Manager");
		result += getConfigMountMgr();
	}
	else
	if (mode == "configSettings")
	{
		result = getTitle("CONFIG: Settings");
		result += getConfigSettings();
	}
	else
	if (mode == "configRotor")
	{
		result = getTitle("CONFIG: Rotor");
		result += getConfigRotor();
	}
	else
	if (mode == "configWebserver")
	{
		result = getTitle("CONFIG: Web Server");
		result += getConfigCHTTPD();
	}
	else
	if (mode == "configBoot")
	{
		if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7000)
		{
			result = getTitle("CONFIG: Boot Manager");
			result += getConfigBoot();
		}
	}
	else
#endif
	if (mode == "help")
	{
		result = getTitle("HELP");
		if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7000
		|| eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7020)
		{
			result += "<img src=\"dm7000.jpg\" width=\"";
			result += pdaScreen ? "160" : "630";
			result += "\" border=\"0\"><br><br>";
		}
		result += getBoxInfo("BoxInfo", "HTML");
	}
	else
	if (mode == "helpDMMSites")
	{
		result = getTitle("HELP: DMM Sites");
		result += readFile(TEMPLATE_DIR + "helpDMMSites.tmp");
	}
	else
	if (mode == "helpOtherSites")
	{
		result = getTitle("HELP: Other Sites");
		result += readFile(TEMPLATE_DIR + "helpOtherSites.tmp");
	}
	else
	if (mode == "helpForums")
	{
		result = getTitle("HELP: Forums");
		result += readFile(TEMPLATE_DIR + "helpForums.tmp");
	}
	else
	if (mode == "control")
	{
		result = getTitle("CONTROL");
		result += "Control your box using the commands on the left";
	}
	else
	if (mode == "controlFBShot")
	{
		result = getTitle("CONTROL: OSDShot");
		result += getScrOsdNavi(pdaScreen);
		if (!getOSDShot("fb"))
		{
			result += "<table bgcolor=\"#000000\" cellpadding=\"0\" cellspacing=\"0\">";
			result += "<tr><td><img width=\"";
			result += pdaScreen ? "160" : "720";
			result += "\" src=\"/root/tmp/osdshot.png\" border=0></td></tr></table>";
		}
	}
	else
#ifndef DISABLE_LCD
	if (mode == "controlLCDShot")
	{
		result = getTitle("CONTROL: LCDShot");
		result += getScrOsdNavi(pdaScreen);
		if (!getOSDShot("lcd"))
		{
			result += "<img width=\"";
			result += pdaScreen ? "160" : "720";
			result += "\" src=\"/root/tmp/osdshot.png\" border=0>";
		}
	}
	else
#endif
	if (mode == "controlScreenShot")
	{
		result = getTitle("CONTROL: Screenshot");
		result += getScrOsdNavi(pdaScreen);
		result += getControlScreenShot(opts);
	}
	else
	if (mode == "controlSatFinder")
	{
		result = getTitle("CONTROL: Satfinder");
		result += getControlSatFinder(opts);
	}
	else
	if (mode == "timers")
	{
		result = getTitle("TIMERS");
		result += getTimers();
	}
	else
	if (mode == "helpUpdatesInternet")
	{
		result = getTitle("HELP: Images");
		result += getHelpUpdatesInternet();
	}
	else
	{
		result = getTitle("GENERAL");
		result += mode + " is not available yet";
	}

	return result;
}

static eString audiopls(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="audio/mpegfile";

	eString serviceName = "Enigma Audio Stream";
	eService* current;
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		current = eDVB::getInstance()->settings->getTransponders()->searchService(sapi->service);
		if (current)
			serviceName = filter_string(current->service_name);
	}

	eString result = "[playlist]\n";
	result += "File1=";
	result += "http://" + getIP() + ":31343/" + eString().sprintf("%02x\n", Decoder::current.apid);
	result += "Title1=" + serviceName + "\n";
	result += "Length1=-1\n";
	result += "NumberOfEntries=1\n";
	result += "Version=2";

	return result;
}

static eString getvideom3u()
{
	eString vpid = eString().sprintf("%04x", Decoder::current.vpid);
	eString pmtpid = eString().sprintf("%04x", Decoder::current.pmtpid);
	eString pcrpid = "," + eString().sprintf("%04x", Decoder::current.pcrpid);

	eString apids;
	eDVBServiceController *sapi = eDVB::getInstance()->getServiceAPI();
	if (sapi)
	{
		std::list<eDVBServiceController::audioStream> &astreams(sapi->audioStreams);
		for (std::list<eDVBServiceController::audioStream>::iterator it(astreams.begin())
			;it != astreams.end(); ++it)
		{
			apids += "," + eString().sprintf("%04x", it->pmtentry->elementary_PID);
		}
	}
	
	if (Decoder::current.pcrpid == Decoder::current.vpid || Decoder::current.pcrpid == Decoder::current.apid)
		pcrpid = "";

	return "http://" + getIP() + ":31339/0," + pmtpid + "," + vpid + apids + pcrpid;
}

static eString getzapm3u()
{
	return "http://" + getIP() + ":31344";
}

static eString videom3u(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
#ifndef DISABLE_FILE
	eZapMain::getInstance()->stopPermanentTimeshift();
#endif
	eProcessUtils::killProcess("streamts");

	content->local_header["Content-Type"] = "video/mpegfile";
	content->local_header["Cache-Control"] = "no-cache,no-store,must-revalidate,max-age=1";

	int zapstream = 0;
	eConfig::getInstance()->getKey("/ezap/webif/useZapStream", zapstream);
	return zapstream ? getzapm3u() : getvideom3u();
}

static eString moviem3u(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString,eString> opt = getRequestOptions(opts, '&');

	eString movieRef = httpUnescape(opt["ref"]);
	eString movieLocation;
	
	eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, movieLocation);
	eString movieFile = movieRef.right(movieRef.length() - movieRef.find(movieLocation));

	eProcessUtils::killProcess("streamts");

	content->local_header["Content-Type"] = "video/mpegfile";
	content->local_header["Cache-Control"] = "no-cache,no-store,must-revalidate,max-age=1";

	return "http://" + getIP() + ":31342" + movieFile.strReplace(" ", "%20");
}

static eString zapStream(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString opt = httpUnescape(opts);
	unsigned int pos = opt.find_last_of('=');
	eString pars = opt.right(opt.size()-pos-1);
	pos = pars.find_last_of('/');
	eString service_ref = pars.right(pars.size()-pos-1);
	eString parameters = pars.left(pos);
	eDebug("[zapStream] parameters=%s service_reference=%s", parameters.c_str(),service_ref.c_str());

	eServiceReference current_service = string2ref(service_ref.c_str());
	if (!(current_service.flags&eServiceReference::isDirectory) && current_service)
	{
		eProcessUtils::killProcess("streamts");
		playService(current_service);
#ifndef DISABLE_FILE
		eZapMain::getInstance()->stopPermanentTimeshift();
#endif
		content->local_header["Content-Type"] = "audio/mpegurl";
		content->local_header["Cache-Control"] = "no-cache,no-store,must-revalidate,max-age=1";

		return "http://" + getIP() + ":31339/" + parameters;
	}
	return closeWindow(content, "Please wait...", 3000);
}

static eString mPlayer(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	eString vpid = eString().sprintf("%04x", Decoder::current.vpid);
	eString apid = eString().sprintf("%04x", Decoder::current.apid);

	content->local_header["Content-Type"] = "video/mpegfile";
	content->local_header["Cache-Control"] = "no-cache,no-store,must-revalidate,max-age=1";
	content->local_header["vpid"] = vpid;
	content->local_header["apid"] = apid;

	return "http://" + getIP() + ":31339/" + vpid  + "," + apid;
}

static eString setStreamingServiceRef(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eStreamer::getInstance()->setServiceReference(string2ref(opt["sref"]));
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	return closeWindow(content, "", 10);
}

eString genBar(int val)
{
	std::stringstream result;
	for (int i = 10; i <= 100; i += 10)
	{
		result << "<td width=\"15\" height=\"8\">";
		if (i <= val)
			result << "<img src=\"led_on.gif\" border=\"0\" width=\"15\" height=\"8\">";
		else
			result << "<img src=\"led_off.gif\" border=\"0\" width=\"15\" height=\"8\">";
		result << "</td>";
	}
	return result.str();
}

static eString satFinder(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	if (opts != lastTransponder)
		tuneTransponder(opts);

	content->local_header["Content-Type"]="text/html; charset=utf-8";
	eString result = readFile(TEMPLATE_DIR + "satFinder.tmp");

	eFrontend *fe = eFrontend::getInstance();
	int status,snrrel,agcrel,berrel;
	eString snrstring,agcstring,berstring;
	fe->getStatus(status,snrrel,snrstring,agcrel,agcstring,berrel,berstring);
	
	bool lock = status & FE_HAS_LOCK;
	bool sync = status & FE_HAS_SYNC;

	result.strReplace("#SNR#", snrstring);
	result.strReplace("#SNRBAR#", genBar(snrrel));
	result.strReplace("#AGC#", agcstring);
	result.strReplace("#AGCBAR#", genBar(agcrel));
	result.strReplace("#BER#", berstring);
	result.strReplace("#BERBAR#", genBar(berrel));
	result.strReplace("#LOCK#", (lock) ? "checked" : "");
	result.strReplace("#SYNC#", (sync) ? "checked" : "");

	return result;
}

static eString message(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	std::map<eString, eString> opts = getRequestOptions(opt, '&');
	eString wait = opts["wait"];
	eString msg = opts["message"];
	if (!msg)
		msg = opt;
	if (!msg)
		msg = "Error: No message text available.";

	int timeout = (wait == "on") ? 0 : 10;
	eZapMain::getInstance()->postMessage(eZapMessage(1, _("External Message"), httpUnescape(msg), timeout), 0);

	return closeWindow(content, "", 10);
}

static eString zapTo(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString,eString> opt = getRequestOptions(opts, '&');

	eString curBouquet = opt["curBouquet"];
	if (curBouquet)
		currentBouquet = atoi(curBouquet.c_str());
	eString curChannel = opt["curChannel"];
	if (curChannel)
		currentChannel = atoi(curChannel.c_str());

	eServiceReference current_service = string2ref(opt["path"]);

	if (!(current_service.flags&eServiceReference::isDirectory) && current_service)
	{
		eProcessUtils::killProcess("streamts");
		playService(current_service);
	}

	return closeWindow(content, "Please wait...", 3000);
}

static eString web_root(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result;

	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	content->local_header["Content-Type"]="text/html; charset=utf-8";

	eConfig::getInstance()->getKey("/ezap/webif/screenWidth", screenWidth);
	pdaScreen = (screenWidth < 800) ? 1 : 0;

	if (opts.find("screenWidth") != eString::npos)
	{
		eString sWidth = opt["screenWidth"];
		screenWidth = atoi(sWidth.c_str());
		eConfig::getInstance()->setKey("/ezap/webif/screenWidth", screenWidth);
		pdaScreen = (screenWidth < 800) ? 1 : 0;
	}
	else
	{
		if ((opts.find("mode") == eString::npos) && (opts.find("path") == eString::npos))
			return readFile(TEMPLATE_DIR + "index.tmp");
	}

	if (pdaScreen == 0)
	{
		result = readFile(TEMPLATE_DIR + "index_big.tmp");
		if (eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000)
			result.strReplace("#BOX#", "Dreambox");
		else
			result.strReplace("#BOX#", "dBox");
		if (eSystemInfo::getInstance()->getHwType() >= eSystemInfo::DM7000)
			result.strReplace("#TOPBALK#", "topbalk.png");
		else
		if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Nokia)
			result.strReplace("#TOPBALK#", "topbalk2.png");
		else
		if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Sagem)
			result.strReplace("#TOPBALK#", "topbalk3.png");
		else
//		if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Philips)
			result.strReplace("#TOPBALK#", "topbalk4.png");
		result.strReplace("#EMPTYCELL#", "&nbsp;");
		result.strReplace("#CHANNAVI#", getChanNavi(true));
		result.strReplace("#TOPNAVI#", getTopNavi());
#ifndef DISABLE_FILE
		result.strReplace("#DVRCONTROLS#", readFile(TEMPLATE_DIR + "dvrcontrols.tmp"));
#else
		result.strReplace("#DVRCONTROLS#", "");
#endif
	}
	else
	{
		content->local_header["Cache-Control"] = "no-cache";
		result = getPDAContent(opts);
	}

	return result;
}

void sendKey(int evd, unsigned int code, unsigned int value)
{
	struct input_event iev;

	iev.type = EV_KEY;
	iev.code = code;
	iev.value = value;
	write (evd, &iev, sizeof(iev));
}

int translateKey(int key)
{
	if (key == 393) // video
	{
		keyboardMode = (keyboardMode) ? 0 : 1;
	}
	else
	if (key == 66) // text
	{
		keyboardMode = KEYBOARDVIDEO;
	}
	else
	{
		if (keyboardMode == KEYBOARDVIDEO)
		{
			switch (key)
			{
				case 385: key = 128; break; // stop
				case 377: key = 167; break; // record
				case 398: key = 168; break; // rewind
				case 399: key = 207; break; // play
				case 400: key = 119; break; // pause
				case 401: key = 208; break; // forward
			}
		}
	}
	return key;
}

static int keyb_evt_dev_num = -1;

static eString remoteControl(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	enum
	{
		KEY_RELEASED = 0,
		KEY_PRESSED,
		KEY_AUTOREPEAT
	};

	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	content->local_header["Content-Type"]="text/html; charset=utf-8";

	eString keysS = opts;
	eString durationS;
	eString reptimeS;

	while (keysS)
	{
		unsigned int pos;
		eString keyS;
		if ((pos = keyS.find(",")) != eString::npos)
		{
			keyS = keysS.left(pos);
			keysS = keysS.right(keysS.length() - pos - 1);
		}
		else
		{
			keyS = keysS;
			keysS = "";
		}

		eString tmp = keyS;
		pos = tmp.find(":");
		if (pos != eString::npos)
		{
			keyS = tmp.left(pos);
			tmp = tmp.right(tmp.length() - pos - 1);
		}
		else
		{
			keyS = tmp;
			tmp = "";
		}

		if ((pos = tmp.find(":")) != eString::npos)
		{
			durationS = tmp.left(pos);
			reptimeS = tmp.right(tmp.length() - pos - 1);
		}
		else
		{
			durationS = tmp;
			reptimeS = "";
		}

		unsigned long duration = 0;
		if (durationS)
			duration = atol(durationS.c_str());

		unsigned long reptime = 500;
		if (reptimeS)
			atol(reptimeS.c_str());

		unsigned long time = duration * 1000 / reptime;

		int key = atoi(keyS.c_str());
		key = translateKey(key);

		if (eSystemInfo::getInstance()->getHwType() < 3) // dbox2
			keyb_evt_dev_num = 0;

		int evd=-1;
		if (keyb_evt_dev_num == -1)
		{
			int cnt=0;
			while (true)
			{
				struct stat s;
				int fd;
				char tmp[128];
				sprintf(tmp, "/dev/input/event%d", cnt);
				if (stat(tmp, &s))
					break;
				if ((fd=open(tmp, O_RDWR|O_NONBLOCK)) == -1)
					eDebug("open %s failed(%m)", tmp);
				else 
				{
					if (ioctl(fd, EVIOCGNAME(128), tmp) < 0)
						eDebug("EVIOCGNAME failed(%m)");
					else
					{
						int idx=0;
						int len = strlen(tmp);
						while(idx <= len-8)
						{
							if (!strncasecmp(&tmp[idx++], "KEYBOARD", 8))
							{
								keyb_evt_dev_num = cnt;
								evd = fd;
								goto InputDevFound;
							}
						}
					}
					close(fd);
				}
				++cnt;
			}
		}
		else
		{
			char tmp[128];
			sprintf(tmp, "/dev/input/event%d", keyb_evt_dev_num);
			if ((evd=open(tmp, O_RDWR|O_NONBLOCK)) < 0)
				eDebug("open %s failed(%m)", tmp);
		}
InputDevFound:
		if (evd > -1)
		{
			sendKey(evd, key, KEY_PRESSED);
			while (time--)
			{
				usleep(reptime * 1000);
				sendKey(evd, key, KEY_AUTOREPEAT);
			}
			sendKey(evd, key, KEY_RELEASED);
			close(evd);
		}
	}
	return closeWindow(content, "", 10);
}

static eString showRemoteControl(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result;
	std::map<eString,eString> opt = getRequestOptions(opts, '&');
	content->local_header["Content-Type"]="text/html; charset=utf-8";

	if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Nokia
	 || eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Sagem
	 || eSystemInfo::getInstance()->getHwType() == eSystemInfo::dbox2Philips)
		result = readFile(TEMPLATE_DIR + (pdaScreen ? "pda" : "") + "remoteControlDbox2.tmp");
	else
	{
		if (pdaScreen == 0)
		{
			result = readFile(TEMPLATE_DIR + "remoteControl.tmp");
			int osdshotenabled = 1;
			eConfig::getInstance()->getKey("/enigma/osdshotenabled", osdshotenabled);
			result.strReplace("#OSDSHOTENABLED#", eString().sprintf("%d", osdshotenabled));
			if ((access("/tmp/osdshot.png", R_OK) == 0) && (osdshotenabled == 1))
				result.strReplace("#OSDSHOTPNG#", "/root/tmp/osdshot.png");
			else
				result.strReplace("#OSDSHOTPNG#", "trans.gif\" width=\"0\" height=\"0");
		}
		else
			result = readFile(TEMPLATE_DIR + "pdaRemoteControl.tmp");
	}

	return result;
}

static eString leftnavi(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString mode = opt["mode"];
	if (!mode)
		mode = "zap";
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	eString result = readFile(TEMPLATE_DIR + "leftnavi.tmp");

	result.strReplace("#LEFTNAVI#", getLeftNavi(mode));
	return result;
}

static eString webxtv(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	eString result = readFile(TEMPLATE_DIR + "webxtv" + opt["browser"] + ".tmp");
	result.strReplace("#ZAPDATA#", getZapContent(zap[ZAPMODETV][ZAPSUBMODEBOUQUETS], 2, true, false, false));
	result.strReplace("#CHANNAVI#", getChanNavi(false));
	return result;
}

#ifndef TUXTXT_CFG_STANDALONE
// methods from libtuxtxt
#include <tuxtxt/tuxtxt_def.h>
extern "C" {
	tuxtxt_cache_struct tuxtxt_cache;
	tstHTML* tuxtxt_InitHTML();
	void tuxtxt_RenderStylesHTML(tstHTML* pHTML,char* styles);
	void tuxtxt_RenderHTML(tstHTML* pHTML,char* result );
	void tuxtxt_EndHTML(tstHTML* pHTML);
}

static eString teletext(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result = readFile(TEMPLATE_DIR + "teletext.tmp");
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	eString colors = "";
	eString result1 = "";
	eString subpages = "";
	eString page = opt["page"];

	if (page)
		sscanf(page.c_str(),"%x",&tuxtxt_cache.page);
	eString subpage = opt["subpage"];
	if (subpage)
		sscanf(subpage.c_str(),"%x",&tuxtxt_cache.subpage);
	else 
		tuxtxt_cache.subpage=tuxtxt_cache.subpagetable[tuxtxt_cache.page];
	result.strReplace("#CURPAGE#", eString().sprintf("%x",tuxtxt_cache.page));
	tstHTML* pHTML = tuxtxt_InitHTML();
	if (pHTML)
	{
		char tmpresult[4000];
		char tmpstyle[1000];
		while (pHTML->row < 24)
		{
			tuxtxt_RenderStylesHTML(pHTML,tmpstyle);
			colors+= eString(tmpstyle);
		}
		pHTML->row = 0;
		pHTML->col = 0;
		while (pHTML->row < 24)
		{
			tuxtxt_RenderHTML(pHTML,tmpresult);
			result1+= eString(tmpresult);
		}
		for (int loop = 0; loop < 0x80; loop++)
		{
			if (tuxtxt_cache.astCachetable[tuxtxt_cache.page][loop])
			{
				subpages += eString().sprintf("<input type=button value=\"%2x\" style=\"width:20px;height:22px; background-image:url(/%s.png);background-repeat:repeat-x\"  onclick=\"setpage(0,%2x)\">",loop,(loop == tuxtxt_cache.subpage ? "yellow":"green"),loop); 
			}
		}
	}
	result.strReplace("#SUBPAGES#", subpages);
	result.strReplace("#COLORTABLE#", colors);
	result.strReplace("#TELETEXT#", result1);
	tuxtxt_EndHTML(pHTML);
	return result;
}
#endif

eString getBoxStatus(eString format)
{
	eString result = readFile(TEMPLATE_DIR + format + "Data.tmp");

	// mode
	result.strReplace("#MODE#", eString().sprintf("%d", eZapMain::getInstance()->getMode()));

	// time
	time_t atime;
	time(&atime);
	result.strReplace("#TIME#", eString(ctime(&atime)));

	// epg data
	result = getEITC(result, format);

	// webif update cycle
	int updateCycle = 10000;
	eConfig::getInstance()->getKey("/ezap/webif/updateCycle", updateCycle);
	result.strReplace("#UPDATECYCLE#", eString().sprintf("%d", updateCycle));

	// standby
	result.strReplace("#STANDBY#", (eZapMain::getInstance()->isSleeping()) ? "1" : "0");

	// uptime
	int sec = atoi(readFile("/proc/uptime").c_str());
	result.strReplace("#UPTIME#", eString().sprintf("%d:%02d h up", sec / 3600, (sec % 3600) / 60));

	// IP
	result.strReplace("#IP#", getIP());

	// webif lock
	int lockWebIf = 1;
	eConfig::getInstance()->getKey("/ezap/webif/lockWebIf", lockWebIf);
	result.strReplace("#LOCK#", (lockWebIf == 1) ? "locked" : "unlocked");

	// vpid
	result.strReplace("#VPID#", (Decoder::current.vpid == -1) ? "none" : eString().sprintf("0x%x", Decoder::current.vpid));

	// apid
	result.strReplace("#APID#", (Decoder::current.apid == -1) ? "none" : eString().sprintf("0x%x", Decoder::current.apid));

	int zapstream = 0;
	eConfig::getInstance()->getKey("/ezap/webif/useZapStream", zapstream);
	// vlc parameters
	result.strReplace("#VLCPARMS#", zapstream ? getzapm3u() : getvideom3u());

	// free recording space on disk
#ifndef DISABLE_FILE
	int fds = freeRecordSpace();
#else
	int fds = 0;
#endif
	if (fds != -1)
	{
		if (fds < 1024)
			result.strReplace("#DISKGB#", eString().sprintf("%d MB", fds));
		else
			result.strReplace("#DISKGB#", eString().sprintf("%d.%02d GB", fds/1024, (int)((fds % 1024) / 10.34)));

		int min = fds / 33;
		if (min < 60)
			result.strReplace("#DISKH#", eString().sprintf("~%d min", min));
		else
			result.strReplace("#DISKH#", eString().sprintf("~%d h, %02d min", min/60, min%60));
	}
	else
	{
		result.strReplace("#DISKGB#", "n/a");
		result.strReplace("#DISKH#", "n/a");
	}

	// volume
	result.strReplace("#VOLUME#", (eAVSwitch::getInstance()->getMute()) ? "0" : eString().sprintf("%d", 63 - eAVSwitch::getInstance()->getVolume()));

	// mute
	result.strReplace("#MUTE#", (eAVSwitch::getInstance()->getMute()) ? "1" : "0");

	// channel stats
	result.strReplace("#DOLBY#", (eZapMain::getInstance()->getAC3Logo()) ? "1" : "0");
	result.strReplace("#CRYPT#", (eZapMain::getInstance()->getSmartcardLogo()) ? "1" : "0");
	result.strReplace("#FORMAT#", (eZapMain::getInstance()->get16_9Logo()) ? "1" : "0");

	// recording status
#ifndef DISABLE_FILE
	result.strReplace("#RECORDING#", (eZapMain::getInstance()->isRecording()) ? "1" : "0");
	result.strReplace("#RECCHANNEL#", eZapMain::getInstance()->RecordingChannel());
#else
	result.strReplace("#RECORDING#", "0");
	result.strReplace("#RECCHANNEL#", "");
#endif
	// vlc streaming
	result.strReplace("#SERVICEREFERENCE#", (eServiceInterface::getInstance()->service) ? eServiceInterface::getInstance()->service.toString() : "");

	// dvr info
	int videopos = 0;
	int min = 0, sec2 = 0;
	int total = 0, current = 0;

#ifndef DISABLE_FILE
	if (eServiceHandler *handler = eServiceInterface::getInstance()->getService())
	{
		total = handler->getPosition(eServiceHandler::posQueryLength);
		current = handler->getPosition(eServiceHandler::posQueryCurrent);
	}

	if ((total > 0) && (current != -1))
	{
		min = total - current;
		sec2 = min % 60;
		min /= 60;
		videopos = (current * 20) / total;
	}
#endif

	result.strReplace("#VIDEOPOSITION#", eString().sprintf("%d", videopos));
	result.strReplace("#VIDEOTIME#", eString().sprintf("%d:%02d", min, sec2));

	eFrontend *fe = eFrontend::getInstance();
	int status,snrrel,agcrel,berrel;
	eString snrstring,agcstring,berstring;
	fe->getStatus(status,snrrel,snrstring,agcrel,agcstring,berrel,berstring);

	bool lock = status & FE_HAS_LOCK;
	bool sync = status & FE_HAS_SYNC;

	result.strReplace("#SNR#", snrstring);
	result.strReplace("#SNRBAR#", eString().sprintf("%i",snrrel));
	result.strReplace("#AGC#", agcstring);
	result.strReplace("#AGCBAR#", eString().sprintf("%i",agcrel));
	result.strReplace("#BER#", berstring);
	result.strReplace("#BERBAR#", eString().sprintf("%i",berrel));
	result.strReplace("#SATLOCK#", lock ? "on" : "off");
	result.strReplace("#SATSYNC#", sync ? "on" : "off");

#ifdef ENABLE_EXPERT_WEBIF
	// streaming client status
	result.strReplace("#STREAMINGCLIENTSTATUS#", eString().sprintf("%d", eMoviePlayer::getInstance()->getStatus()));
#else
	result.strReplace("#STREAMINGCLIENTSTATUS#", "0");
#endif

	return result;
}


static eString data(eString request, eString dirpath, eString opt, eHTTPConnection *content)
{
	content->local_header["Content-Type"]="text/html; charset=utf-8";
	content->local_header["Cache-Control"] = "no-cache,no-store,must-revalidate,max-age=1";
	return getBoxStatus("HTML");
}

static eString body(eString request, eString dirpath, eString opts, eHTTPConnection *content)
{
	eString result;
	std::map<eString, eString> opt = getRequestOptions(opts, '&');
	content->local_header["Content-Type"]="text/html; charset=utf-8";

	eConfig::getInstance()->getKey("/ezap/webif/screenWidth", screenWidth);

	int previousZapMode = zapMode;
	int previousZapSubMode = zapSubMode;

	eString mode = opt["mode"];
	if (!mode)
		mode = "zap";

	eString path = opt["path"];

	if (mode == "zap")
	{
		eString zapModeS = opt["zapmode"];
		if (zapModeS)
			zapMode = atoi(zapModeS.c_str());

		eString zapSubModeS = opt["zapsubmode"];
		if (zapSubModeS)
			zapSubMode = atoi(zapSubModeS.c_str());

		eString curBouquet = opt["curBouquet"];
		if (curBouquet)
			currentBouquet = atoi(curBouquet.c_str());

		eString curChannel = opt["curChannel"];
		if (curChannel)
			currentChannel = atoi(curChannel.c_str());

		if ((zapMode >= 0) && (zapMode <= 5) && (zapSubMode >= 0) && (zapSubMode <= 5))
		{
			if (!path)
				path = zap[zapMode][zapSubMode];
		}
		else
		{
			zapMode = ZAPMODETV;
			zapSubMode = ZAPSUBMODEBOUQUETS;
			path = zap[zapMode][zapSubMode];
		}

		if (zapMode != previousZapMode || zapSubMode != previousZapSubMode)
		{
			currentBouquet = 0;
			currentChannel = -1;
		}

		result = getContent(mode, path, opts);
	}
	else
	{
		result = readFile(TEMPLATE_DIR + "index2.tmp");
		eString tmp = getContent(mode, path, opts);
		if (tmp)
			result.strReplace("#CONTENT#", tmp);
		else
			result = "";
	}

	if (!result)
		result = closeWindow(content, "Please wait...", 3000);

	return result;
}


void ezapInitializeDyn(eHTTPDynPathResolver *dyn_resolver)
{
	int lockWebIf = 1;
	if (eConfig::getInstance()->getKey("/ezap/webif/lockWebIf", lockWebIf))
		eConfig::getInstance()->setKey("/ezap/webif/lockWebIf", lockWebIf);

	eDebug("[ENIGMA_DYN] lockWebIf = %d", lockWebIf);
	bool lockWeb = (lockWebIf == 1) ? true : false;

	dyn_resolver->addDyn("GET", "/", web_root, lockWeb);
#ifndef DISABLE_FILE
	dyn_resolver->addDyn("GET", "/cgi-bin/videocontrol", videocontrol, lockWeb);
#endif
	dyn_resolver->addDyn("GET", "/setVolume", setVolume, lockWeb);
	dyn_resolver->addDyn("GET", "/setVideo", setVideo, lockWeb);
	dyn_resolver->addDyn("GET", "/tvMessageWindow", tvMessageWindow, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/zapTo", zapTo, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/zapStream.m3u", zapStream, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/admin", admin, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/selectAudio", selectAudio, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/setAudio", setAudio, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/selectSubChannel", selectSubChannel, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/message", message, lockWeb);
	dyn_resolver->addDyn("GET", "/control/message", message, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/rc", remoteControl, lockWeb);
	dyn_resolver->addDyn("GET", "/showRemoteControl", showRemoteControl, lockWeb);
	dyn_resolver->addDyn("GET", "/satFinder", satFinder, lockWeb);
	dyn_resolver->addDyn("GET", "/audio.pls", audiopls, lockWeb);
	dyn_resolver->addDyn("GET", "/video.m3u", videom3u, lockWeb);
	dyn_resolver->addDyn("GET", "/movie.m3u", moviem3u, lockWeb);
	dyn_resolver->addDyn("GET", "/mplayer.mply", mPlayer, lockWeb);
	dyn_resolver->addDyn("GET", "/body", body, lockWeb);
	dyn_resolver->addDyn("GET", "/data", data, lockWeb);
	dyn_resolver->addDyn("GET", "/leftnavi", leftnavi, lockWeb);
	dyn_resolver->addDyn("GET", "/webxtv", webxtv, lockWeb);
#ifndef TUXTXT_CFG_STANDALONE
	dyn_resolver->addDyn("GET", "/teletext", teletext, lockWeb);
#endif
	dyn_resolver->addDyn("GET", "/cgi-bin/setStreamingServiceRef", setStreamingServiceRef, lockWeb);
#ifndef DISABLE_FILE
	dyn_resolver->addDyn("GET", "/cgi-bin/recoverRecordings", recoverRecordings, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/deleteMovie", deleteMovie, lockWeb);
	dyn_resolver->addDyn("GET", "/cgi-bin/renameMovie", renameMovie, lockWeb);
#endif
	dyn_resolver->addDyn("GET", "/cgi-bin/osdshot", osdshot, lockWeb);

	ezapWapInitializeDyn(dyn_resolver, lockWeb);
	ezapXMLInitializeDyn(dyn_resolver, lockWeb);
	ezapEPGInitializeDyn(dyn_resolver, lockWeb);
	ezapMiscInitializeDyn(dyn_resolver, lockWeb);
	ezapTimerInitializeDyn(dyn_resolver, lockWeb);
	ezapPDAInitializeDyn(dyn_resolver, lockWeb);
#ifdef ENABLE_EXPERT_WEBIF
	ezapBootManagerInitializeDyn(dyn_resolver, lockWeb);
	ezapMoviePlayerInitializeDyn(dyn_resolver, lockWeb);
	ezapMountInitializeDyn(dyn_resolver, lockWeb);
	ezapConfInitializeDyn(dyn_resolver, lockWeb);
	ezapFlashInitializeDyn(dyn_resolver, lockWeb);
	ezapRotorInitializeDyn(dyn_resolver, lockWeb);
	ezapCHTTPDInitializeDyn(dyn_resolver, lockWeb);
#endif
}

