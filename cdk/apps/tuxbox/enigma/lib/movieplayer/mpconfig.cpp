/*
 * $Id: mpconfig.cpp,v 1.4 2005/12/23 17:00:07 digi_casi Exp $
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <errno.h>
#include <xmltree.h>
#include <lib/movieplayer/movieplayer.h>

eMPConfig::eMPConfig()
{
}

eMPConfig::~eMPConfig()
{
}

bool eMPConfig::load()
{
	XMLTreeParser parser("ISO-8859-1");

	eString file = CONFFILE1;
	FILE *in = fopen(file.c_str(), "rt");
	if (!in) 
	{
		file = CONFFILE0;
		in = fopen(file.c_str(), "rt");
		if (!in) 
		{
			eDebug("[MPCONFIG] unable to open %s", file.c_str()); 
			return false;
		}
	}

	videoParmList.clear();
	
	bool done = false;
	while (!done)
	{
		char buf[2048]; 
		unsigned int len = fread(buf, 1, sizeof(buf), in);
		done = len < sizeof(buf);
		if (!parser.Parse(buf, len, done))
		{
			eDebug("[MPCONFIG] parsing settings file: %s at line %d>", parser.ErrorString(parser.GetErrorCode()), parser.GetCurrentLineNumber());
			fclose(in);
			return false;
		}
	}

	fclose(in);

	XMLTreeNode *root = parser.RootNode();
	if (!root)
		return false;

	for (XMLTreeNode *node = root->GetChild(); node; node = node->GetNext())
	{
		if (!strcmp(node->GetType(), "server"))
		{
			serverConf.serverIP = node->GetAttributeValue("ip");
			serverConf.webifPort = node->GetAttributeValue("webif-port");
			serverConf.streamingPort = node->GetAttributeValue("stream-port");

			serverConf.vlcUser = node->GetAttributeValue("user");
			serverConf.vlcPass = node->GetAttributeValue("pass");
		}
		else 
		if (!strcmp(node->GetType(), "config"))
		{
			serverConf.startDir = node->GetAttributeValue("startdir");
			serverConf.CDDrive  = node->GetAttributeValue("cddrive");
		}
		else 
		if (!strcmp(node->GetType(), "codec"))
		{
			avcodecs.mpeg1 = node->GetAttributeValue("mpeg1");
			avcodecs.mpeg2 = node->GetAttributeValue("mpeg2");
			avcodecs.audio = node->GetAttributeValue("audio");
		}
		else 
		if (!strcmp(node->GetType(), "setup"))
		{
			eString tmpname = node->GetAttributeValue("name");
			eString tmpext = node->GetAttributeValue("ext");
			eString tmpvrate = node->GetAttributeValue("Videorate");
			eString tmpvtrans = node->GetAttributeValue("Videotranscode");
			eString tmpvcodec = node->GetAttributeValue("Videocodec");
			eString tmpvsize = node->GetAttributeValue("Videosize");
			eString tmparate = node->GetAttributeValue("Audiorate");
			eString tmpatrans = node->GetAttributeValue("Audiotranscode");
			eString tmpfps = node->GetAttributeValue("fps");
			eString tmpsoutadd = node->GetAttributeValue("soutadd");

			if (!tmpname || !tmpext || !tmpvrate || !tmpvtrans || !tmpvcodec || !tmpvsize || !tmparate || !tmpatrans || !tmpfps )
			{
				eDebug("[MOVIEPLAYER] parse error in settings file");
				return false;
			}
			else
			{
				struct videoTypeParms a;
				a.name = tmpname;
				a.extension = tmpext;
				a.videoRate = tmpvrate;
				a.transcodeVideo = (tmpvtrans == "1");
				a.videoCodec = tmpvcodec;
				a.videoRatio = tmpvsize;
				a.audioRate = tmparate;
				a.transcodeAudio = (tmpatrans == "1");
				a.fps = tmpfps;
				if(!tmpsoutadd)
				        a.soutadd = false; // compatibility for old .xml files
				else
				        a.soutadd = (tmpsoutadd == "1");

				videoParmList.push_back(a);
				
			}
		}
	}

	/*eDebug("\nIP:%s",VLCsend::getInstance()->send_parms.IP.c_str());
	eDebug("WEBIF-PORT:%s",VLCsend::getInstance()->send_parms.IF_PORT.c_str());
	eDebug("Stream-PORT:%s\n",VLCsend::getInstance()->send_parms.STREAM_PORT.c_str());

	eDebug("STARTDIR:%s",startdir.c_str());
	eDebug("CDDRIVE:%s\n",cddrive.c_str());

	eDebug("MPEG1:%s",str_mpeg1.c_str());
	eDebug("MPEG2:%s",str_mpeg2.c_str());
	eDebug("Audio:%s\n",str_audio.c_str());

	for(ExtList::iterator p=extList.begin(); p!=extList.end() ;p++)
	{
		eDebug("name=%s ext=%s vrate=%s vcodec=%s vsize=%s arate=%s item=%d", (*p).NAME.c_str(), (*p).EXT.c_str(), (*p).VRATE.c_str(), (*p).VCODEC.c_str(),
		(*p).VSIZE.c_str(), (*p).ARATE.c_str(), (*p).ITEM);
	}*/

	return true;
}

void eMPConfig::save()
{
	if (FILE *f = fopen(CONFFILE1, "w"))
	{
		fprintf(f, "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\n");
		fprintf(f, "<?xml-stylesheet type=\"text/xsl\" href=\"/XSLMPSettings.xsl\"?>\n");
		fprintf(f, "<vlc>\n");
		fprintf(f, "   <server ip=\"%s\" webif-port=\"%s\" stream-port=\"%s\" user=\"%s\" pass=\"%s\" />\n", serverConf.serverIP.c_str(), serverConf.webifPort.c_str(), serverConf.streamingPort.c_str(), serverConf.vlcUser.c_str(), serverConf.vlcPass.c_str());
		fprintf(f, "   <config startdir=\"%s\" cddrive=\"%s\" />\n", serverConf.startDir.c_str(), serverConf.CDDrive.c_str());
		fprintf(f, "   <codec mpeg1=\"%s\" mpeg2=\"%s\" audio=\"%s\" />\n", avcodecs.mpeg1.c_str(), avcodecs.mpeg2.c_str(), avcodecs.audio.c_str());
		for (unsigned int i = 0; i < videoParmList.size(); i++)
		{
			struct videoTypeParms a = videoParmList[i];
			fprintf(f, "   <setup name=\"%s\" ext=\"%s\" Videorate=\"%s\" Videotranscode=\"%d\" Videocodec=\"%s\" Videosize=\"%s\" Audiorate=\"%s\" Audiotranscode=\"%d\" fps=\"%s\" soutadd=\"%d\" />\n", a.name.c_str(), a.extension.c_str(), a.videoRate.c_str(), a.transcodeVideo, a.videoCodec.c_str(), a.videoRatio.c_str(), a.audioRate.c_str(), a.transcodeAudio, a.fps.c_str(), a.soutadd);
		}
		fprintf(f, "</vlc>\n");
		fclose(f);
	}
}

struct videoTypeParms eMPConfig::getVideoParms(eString name, eString extension)
{
	struct videoTypeParms vparms;
	
	extension = extension.upper();
	
//	eDebug("[MPCONFIG] name = %s, extension = %s", name.c_str(), extension.c_str());
	
	vparms.name = "default";
	vparms.extension = extension;
	vparms.videoRate = "1024";
	vparms.audioRate = "192";
	vparms.videoCodec = "mpeg2";
	vparms.videoRatio = "704x576";
	vparms.transcodeVideo = false;
	vparms.transcodeAudio = false;
	vparms.fps = "25";
	vparms.soutadd = false;

	for ( int i = 0; i < videoParmList.size(); i++)
	{
		if ((videoParmList[i].extension == extension) && (videoParmList[i].name == name))
		{
			vparms = videoParmList[i];
			break;
		}
	}
//	eDebug("[MPCONFIG] vparms.extension: %s",vparms.extension.c_str());
	return vparms;
}

struct serverConfig eMPConfig::getServerConfig()
{
	return serverConf;
}

struct codecs eMPConfig::getAVCodecs()
{
	return avcodecs;
}

void eMPConfig::setVideoParms(struct videoTypeParms vparms)
{
	for (unsigned int i = 0; i < videoParmList.size(); i++)
	{
		if (videoParmList[i].extension == vparms.extension && videoParmList[i].name == vparms.name)
		{
			videoParmList[i] = vparms;
			break;
		}
	}
}

void eMPConfig::setServerConfig(struct serverConfig server)
{
	serverConf = server;
}

void eMPConfig::setAVCodecs(struct codecs avCodecs)
{
	avcodecs = avCodecs;
}
#endif
