/*
	DreamData - Enigma Plugin

	Copyright (C) 2005 'mechatron' (mechatron@gmx.net)

	Homepage: http://mechatron.6x.to/

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#include "dreamdata.h"
#include <plugin.h>
#include <lib/dvb/decoder.h>

#if HAVE_DVB_API_VERSION < 3 //DREAMBOX
#include <ost/net.h>
#define NET_DEV "/dev/dvb/card0/net0"
#else //DBOX 2
#include <linux/dvb/net.h>
#define NET_DEV "/dev/dvb/adapter0/net0"
#endif

#define MROUTEDCONF	"/tmp/mrouted.conf"
#define CONFIGFILE1	CONFIGDIR"/dreamdata.xml"
#define CONFIGFILE0	"/var/tuxbox/config/dreamdata.xml"
#define SCANPIC0	"/var/tuxbox/config/enigma/pictures/scan.mvi"
#define SCANPIC1	DATADIR"/enigma/pictures/scan.mvi"

extern "C" int plugin_exec( PluginParam *par );

struct getservicenumber
{
	int &cha;
	dataservice m_dataservice;
	getservicenumber(int &cha, dataservice m_dataservice)
		:cha(cha), m_dataservice(m_dataservice)
	{
	}
	void operator()(eServiceDVB& s)
	{
		//if(s.service_name == m_dataservice.name) printf("[DD]Name=%s SID=%d TSID=%d\n",s.service_name.c_str(), abs(s.service_id.get()), s.transport_stream_id.get());
		if(s.service_id.get() == m_dataservice.sid && s.transport_stream_id.get() == m_dataservice.tsid)
			cha = s.service_number;
	}
};

dreamdata::dreamdata()
	:con(false), s_zap(eServiceInterface::getInstance()->service)
{
	cmove(ePoint(220, 200)); cresize(eSize(260, 230)); setText("Dreamdata v0.1.1");

	p_liste=new eListBox<eListBoxEntryData>(this);
	p_liste->move(ePoint(5, 5));
	p_liste->resize(eSize(clientrect.width()-10, clientrect.height()-60));
	p_liste->setHelpText(_("to change service press ok"));
	CONNECT(p_liste->selected, dreamdata::p_listeselected);

	status = new eStatusBar(this);
	status->move( ePoint(0, clientrect.height()-50) );status->resize( eSize( clientrect.width(), 50) );status->loadDeco();

	showPic();

	if(FILE *f = fopen(CONFIGFILE0, "r"))
	{
		fclose(f);
		read_xml(CONFIGFILE0);
	}
	else
	{
		if(FILE *f = fopen(CONFIGFILE1, "r"))
		{
			fclose(f);
			read_xml(CONFIGFILE1);
		}
	}
}

void dreamdata::p_listeselected(eListBoxEntryData *item)
{
	if(!item) {return;close(0);}

	p_liste->setHelpText(_("One moment please..."));

	if(con)
	{
		del_net();
		unlink(MROUTEDCONF);
		sleep(1);
	}

	int cha=0;
	eTransponderList::getInstance()->forEachService(getservicenumber(cha, item->m_dataservice));

	if(cha)
	{
		eServiceReferenceDVB s=eDVB::getInstance()->settings->getTransponders()->searchServiceByNumber(cha);
		if(s)
		{
			eZapMain::getInstance()->switchToNum(cha);
			usleep(500000);
			con = true;
			old_dataservice = item->m_dataservice;

			for(int i=0; i<5 ; i++)
			{
				if(old_dataservice.pid[i])
				{
					if(add_net(i,old_dataservice.pid[i]))
					{
						old_dataservice.pid_set[i] = true;
						if ( system( eString().sprintf("/sbin/ifconfig dvb0_%d %s mtu %s",i, dvb_ip[i].c_str(), dvb_mtu[i].c_str()).c_str() ) >> 8 )
						{
							show_error(eString().sprintf(_("ifconfig failed\ndvb0_%d\nIP: %s\nMTU: %s"),i, dvb_ip[i].c_str(), dvb_mtu[i].c_str() ).c_str());
							goto notok;
						}
						else system( eString().sprintf("echo 0 > /proc/sys/net/ipv4/conf/dvb0_%d/rp_filter",i).c_str() );
					}
					else
					{
						show_error(eString().sprintf(_("not opened network device dvb0_%d"), i).c_str());
						goto notok;
					}
				}
			}

			usleep(500000);
			system("/sbin/route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0");
			usleep(500000);
		}
		else { show_error(_("Service not found !")); goto notok;}
	}
	else { show_error(_("Service could not be found !")); goto notok;}

	if(FILE *f=fopen(MROUTEDCONF, "w"))
	{
		for(int i=0; i<5 ; i++)
			if(old_dataservice.conf[i])
				fprintf(f,"%s\n", old_dataservice.conf[i].c_str());

		fclose(f);

		if(system("mrouted -c "MROUTEDCONF)>>8) { show_error(_("mrouted not started !")); goto notok;}

		eServiceDVB *bla=eDVB::getInstance()->settings->getTransponders()->searchService(eServiceInterface::getInstance()->service);

		if (bla)	p_liste->setHelpText(bla->service_name + " activated");
		else { show_error(_("sorry, service not found !")); goto notok;}
	}
	else { show_error(_("sorry, couldn't create mrouted conf file")); goto notok;}

	return;
notok:
	p_liste->setHelpText("Data service not set");
}

void dreamdata::show_error(eString mess)
{
	hide(); eMessageBox msg(mess, _("Error"), eMessageBox::iconWarning|eMessageBox::btOK);
	msg.show(); msg.exec(); msg.hide(); show();
}

int dreamdata::del_net()
{
	eProcessUtils::killProcess("mrouted");

	int fdn;

	if((fdn = ::open(NET_DEV,O_RDWR|O_NONBLOCK)) < 0)
	{
		eDebug("[DREAMDATA] Failed to open DVB NET DEVICE");
		return 0;
	}

	for(int i=0; i<5 ; i++)
	{
		if(old_dataservice.pid_set[i] == true)
			if (::ioctl( fdn, NET_REMOVE_IF, i) < 0)
				eDebug("[DREAMDATA] not closed network device");
	}
	::close (fdn);
	usleep(50000);
	return 0;
}

int dreamdata::add_net(int dev,int dpid)
{
	struct dvb_net_if netif;
	netif.if_num = dev;
	netif.pid = dpid;
	int fdn;

	if((fdn = ::open(NET_DEV,O_RDWR|O_NONBLOCK)) < 0)
	{
		eDebug("[DREAMDATA] Failed to open DVB NET DEVICE");
		return 0;
	}

	if (::ioctl( fdn, NET_ADD_IF, &netif) < 0)
	{
		eDebug("[DREAMDATA] not opened network device");
		return 0;
	}

	::close (fdn);

	return 1;
}

void dreamdata::read_xml(eString file)
{
	XMLTreeParser * parser;
	FILE *in = fopen(file.c_str(), "rt");

	parser = new XMLTreeParser("ISO-8859-1");
	char buf[2048];

	int done;
	do
	{
		unsigned int len=fread(buf, 1, sizeof(buf), in);
		done = ( len < sizeof(buf) );
		if ( ! parser->Parse( buf, len, done ) )
		{
			show_error(_("Configfile parse error"));
			delete parser;
			parser = NULL;
			return;
		}
	}
	while (!done);

	fclose(in);

	XMLTreeNode * root = parser->RootNode();
	if(!root)
	{
		show_error(_("Configfile parse error"));
		return;
	}

	for(XMLTreeNode * node = root->GetChild(); node; node = node->GetNext())
	{
		if(!strcmp(node->GetType(), "dvb_ip"))
		{
			dvb_ip[0] = node->GetAttributeValue("dvb0_0");
			dvb_ip[1] = node->GetAttributeValue("dvb0_1");
			dvb_ip[2] = node->GetAttributeValue("dvb0_2");
			dvb_ip[3] = node->GetAttributeValue("dvb0_3");
			dvb_ip[4] = node->GetAttributeValue("dvb0_4");
		}
		if(!strcmp(node->GetType(), "dvb_mtu"))
		{
			dvb_mtu[0] = node->GetAttributeValue("dvb0_0");
			dvb_mtu[1] = node->GetAttributeValue("dvb0_1");
			dvb_mtu[2] = node->GetAttributeValue("dvb0_2");
			dvb_mtu[3] = node->GetAttributeValue("dvb0_3");
			dvb_mtu[4] = node->GetAttributeValue("dvb0_4");
		}
		if(!strcmp(node->GetType(), "service"))
		{
			for(XMLTreeNode * i = node->GetChild(); i; i = i->GetNext())
			{
				if(!strcmp(i->GetType(), "name")) m_dataservice.name = i->GetData();
				if(!strcmp(i->GetType(), "SID")) m_dataservice.sid = atoi(i->GetData());
				if(!strcmp(i->GetType(), "TSID")) m_dataservice.tsid = atoi(i->GetData());
				if(!strcmp(i->GetType(), "pid"))
				{
					m_dataservice.pid[0] = atoi(i->GetAttributeValue("pid_0"));
					m_dataservice.pid[1] = atoi(i->GetAttributeValue("pid_1"));
					m_dataservice.pid[2] = atoi(i->GetAttributeValue("pid_2"));
					m_dataservice.pid[3] = atoi(i->GetAttributeValue("pid_3"));
					m_dataservice.pid[4] = atoi(i->GetAttributeValue("pid_4"));
				}
				if(!strcmp(i->GetType(), "mroutedconf"))
				{
					m_dataservice.conf[0] = i->GetAttributeValue("conf0");
					m_dataservice.conf[1] = i->GetAttributeValue("conf1");
					m_dataservice.conf[2] = i->GetAttributeValue("conf2");
					m_dataservice.conf[3] = i->GetAttributeValue("conf3");
					m_dataservice.conf[4] = i->GetAttributeValue("conf4");
				}
			}
			new eListBoxEntryData(p_liste, m_dataservice.name, m_dataservice);
		}
	}
	delete parser;
}

void dreamdata::showPic()
{
	Decoder::parms.apid=0x1ffe;
	Decoder::Set();
	
	if(FILE *f = fopen(SCANPIC0, "r"))
	{
		fclose(f);
		Decoder::displayIFrameFromFile(SCANPIC0);
	}
	else
	{
		if(FILE *f = fopen(SCANPIC1, "r"))
		{
			fclose(f);
			Decoder::displayIFrameFromFile(SCANPIC1);
		}
	}
}

int dreamdata::eventHandler( const eWidgetEvent &e )
{
	switch( e.type )
	{
	case eWidgetEvent::evtAction:
		if ( e.action == &i_cursorActions->cancel)
		{
			delete p_liste;
			if(con)
			{
				del_net();
				unlink(MROUTEDCONF);
			}
			eZapMain::getInstance()->playService(s_zap, eZapMain::psDontAdd);
			close(0);
		}
		else break;

		return 1;
	default: break;
	}
	return eWindow::eventHandler(e);
}


int plugin_exec( PluginParam *par )
{
	dreamdata dlg; dlg.show(); dlg.exec(); dlg.hide(); return 0;
}

