#ifndef DISABLE_NETWORK

#include <setupnetwork.h>

#include <netinet/in.h>
#include <linux/route.h>

#ifndef DISABLE_NFS
#include <sys/mount.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <lib/gui/multipage.h>
#include <lib/gui/emessage.h>
#include <lib/base/console.h>
#endif

#ifdef USE_IFUPDOWN
// in Makefile.am INCLUDES @NET_CFLAGS@
// in configure.ac TUXBOX_APPS_LIB_PKGCONFIG(NET,tuxbox-net)
#include <sys/types.h>
#include <sys/wait.h>
#include <network_interfaces.h> /* getInetAttributes, setInetAttributes */
#endif

#include <enigma.h>
#include <setup_mounts.h>
#include <lib/base/i18n.h>
#include <lib/dvb/edvb.h>
#include <lib/gui/elabel.h>
#include <lib/gui/enumber.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/textinput.h>
#include <lib/gui/combobox.h>
#include <lib/gui/eskin.h>
#include <lib/driver/rc.h>
#include <lib/system/econfig.h>
#include <lib/system/init_num.h>
#include <lib/gui/guiactions.h>
#include <lib/system/info.h>

#define MENUNAME N_("Network")

class eZapNetworkSetupFactory : public eCallableMenuFactory
{
public:
	eZapNetworkSetupFactory() : eCallableMenuFactory("eZapNetworkSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eZapNetworkSetup;
	}

	bool isAvailable()
	{
		return eSystemInfo::getInstance()->hasNetwork();
	}
};

eZapNetworkSetupFactory eZapNetworkSetup_factory;

#ifdef ENABLE_PPPOE

class eTOnlineDialog: public eWindow
{
	eTextInputField *Kennung, *tOnlineNummer, *Mitbenutzer;
	eButton *ok;
	eStatusBar *sbar;
	void init_eTOnlineDialog(eString Login);
public:
	eTOnlineDialog( eString Login )
	{
		init_eTOnlineDialog(Login);
	}
	void eTOnlineDialog::init_eTOnlineDialog(eString Login)
	{
		setText("T - Online");
		cresize(eSize(450,270));
		valign();

		eLabel *l = new eLabel(this);
		l->move(ePoint(10,10));
		l->resize(eSize(220,30));
		l->setText("Anschlusskennung:");

		Kennung = new eTextInputField(this,l);
		Kennung->move(ePoint(230,10));
		Kennung->resize(eSize(200,35));
		Kennung->setMaxChars(12);
		Kennung->setUseableChars("1234567890");
		Kennung->loadDeco();
		Kennung->setHelpText("Anschlusskennung eingeben mit OK (12 Stellen)");
		Kennung->setEditHelpText("Anschlusskennung eingeben (0..9, ok)");

		l = new eLabel(this);
		l->move(ePoint(10,60));
		l->resize(eSize(220,30));
		l->setText("T-Online Nummer:");

		tOnlineNummer = new eTextInputField(this,l);
		tOnlineNummer->move(ePoint(230,60));
		tOnlineNummer->resize(eSize(200,35));
		tOnlineNummer->setMaxChars(12);
		tOnlineNummer->setUseableChars("1234567890");
		tOnlineNummer->loadDeco();
		tOnlineNummer->setHelpText("T-Online Nummer eingeben mit OK (12 Stellen)");
		tOnlineNummer->setEditHelpText("T-Online Nummer eingeben (0..9, ok)");

		l = new eLabel(this);
		l->move(ePoint(10,110));
		l->resize(eSize(220,30));
		l->setText("Mitbenutzernummer:");

		Mitbenutzer = new eTextInputField(this,l);
		Mitbenutzer->move(ePoint(230,110));
		Mitbenutzer->resize(eSize(70,35));
		Mitbenutzer->setMaxChars(4);
		Mitbenutzer->setUseableChars("1234567890");
		Mitbenutzer->loadDeco();
		Mitbenutzer->setHelpText("Mitbenutzernummer eingeben mit OK (4 Stellen)");
		Mitbenutzer->setEditHelpText("Mitbenutzernummer eingeben (0..9, ok)");

		ok = new eButton(this);
		ok->move(ePoint(10,160));
		ok->resize(eSize(170,40));
		ok->setShortcut("green");
		ok->setShortcutPixmap("green");
		ok->setText("speichern");
		ok->setHelpText("Daten bernehmen und Fenster schliessen");
		ok->loadDeco();
		CONNECT(ok->selected, eWidget::accept);

		sbar = new eStatusBar(this);
		sbar->move( ePoint(0, clientrect.height()-50) );
		sbar->resize( eSize( clientrect.width(), 50) );
		sbar->loadDeco();

		if (Login)
		{
			unsigned int pos1 = Login.find("#"),
									pos2 = Login.find("@");
			if ( pos1 != eString::npos && pos2 != eString::npos )
			{
				Kennung->setText(Login.left(12));
				tOnlineNummer->setText(Login.mid(12,12));
				Mitbenutzer->setText(Login.mid(pos1+1,4));
			}
		}
	}
	eString getLogin()
	{
		eString tmp =
			Kennung->getText() +
			tOnlineNummer->getText() + '#' +
			Mitbenutzer->getText() +
			"@t-online.de";
		return tmp;
	}
};

static bool readSecretString( eString &str )
{
	FILE *f = fopen("/etc/ppp/pap-secrets", "r");
	if (!f)
		return false;

	char buf[100];

	size_t readed=
		fread(buf, 1, sizeof(buf), f );
	if ( !readed )
		return false;

	fclose(f);
	str.assign( buf, readed );
	str.removeChars(' ');
	str.removeChars('\t');
	str.removeChars('\n');
	str.removeChars('\"');
	return true;
}

static void writeSecretString( const eString &str )
{
	FILE *f = fopen("/etc/ppp/pap-secrets", "w");
	if (!f)
		eFatal("couldn't create /etc/ppp/pap-secrets");
	eString tmp;
	unsigned int pos =
		str.find('*');
	if ( pos != eString::npos )
	{
		tmp = '\"' + str.left(pos) + "\"\t*\t\"" +
			str.mid(pos+1, str.length()-pos ) + "\"\n";
	}
	fwrite( tmp.c_str(), 1, tmp.length(), f );

	fclose(f);
}

static void helper( char *&source, char *&dest, uint &spos, uint &dpos, const char* option, const char* value )
{
	char *p = strstr( source+spos, option );
	if( !p )
		eDebug("couldn't find '%s' Option", option);
	else
	{
		p+=strlen(option);
		int cnt = p - (source+spos);
		memcpy( dest+dpos, source+spos, cnt );
		dpos += cnt;
		spos += cnt;
		cnt = strlen(value);
		memcpy( dest+dpos, value, cnt );
		dpos+=cnt;
		p = strchr( source+spos, '\n' );
		if ( p )
		{
			spos = p - source;
			++spos;
		}
	}
}

static char *getOptionString( char *buf, const char *option )
{
	char *p = strstr( buf, option );
	if( !p )
		eDebug("couldn't find '%s' Option", option);
	else
		p+=strlen(option);
	return p;
}

static int getRejectFlags()
{
	char buf[8192];
	FILE *f = fopen("/etc/ppp/pppoe.conf", "r" );
	if ( !f )
	{
		eDebug("couldn't open '/etc/ppp/pppoe.conf' for read");
		return 0;
	}
	size_t readed = fread(buf, 1, sizeof(buf), f );
	if ( !readed )
	{
		eDebug("couldn't read '/etc/ppp/pppoe.conf'");
		return 0;
	}
	int flags=0;
	char *p = getOptionString(buf, "REJECT_WWW=");
	if ( p && !strncasecmp(p,"yes",3) )
		flags|=1;
	p = getOptionString(buf, "REJECT_TELNET=");
	if ( p && !strncasecmp(p,"yes",3) )
		flags|=2;
	p = getOptionString(buf, "REJECT_SAMBA=");
	if ( p && !strncasecmp(p,"yes",3) )
		flags|=4;
	p = getOptionString(buf, "REJECT_FTP=");
	if ( p && !strncasecmp(p,"yes",3) )
		flags|=8;
	fclose(f);
	return flags;
}

static void updatePPPConfig( const eString &secrets, int flags )
{
	char sourceA[8192];  // source buffer
	char destA[8192]; // dest buffer
	char *source = sourceA;
	char *dest = destA;

	FILE *f = fopen("/etc/ppp/pppoe.conf", "r" );
	if ( !f )
	{
		eDebug("couldn't open '/etc/ppp/pppoe.conf' for read");
		return;
	}
	size_t readed = fread(source, 1, sizeof(sourceA), f );
	if ( !readed )
	{
		eDebug("couldn't read '/etc/ppp/pppoe.conf'");
		return;
	}

	uint spos = 0;
	uint dpos = 0;
	uint ppos = secrets.find('*');

	if ( ppos != eString::npos )
	{
		eString strUser = '\'' + secrets.left(ppos) + "\'\n";
		helper( source, dest, spos, dpos, "USER=", strUser.c_str() );
	}
	int webifport=80;
	eConfig::getInstance()->getKey("/elitedvb/network/webifport", webifport);
	eString s;
	s.sprintf("%d\n", webifport);
	helper( source, dest, spos, dpos, "ENIGMA_WEB_IF_PORT=", s.c_str() );
	helper( source, dest, spos, dpos, "REJECT_WWW=", flags&1?"yes\n":"no\n" );
	helper( source, dest, spos, dpos, "REJECT_TELNET=", flags&2?"yes\n":"no\n" );
	helper( source, dest, spos, dpos, "REJECT_SAMBA=", flags&4?"yes\n":"no\n" );
	helper( source, dest, spos, dpos, "REJECT_FTP=", flags&8?"yes\n":"no\n" );

	memcpy( dest+dpos, source+spos, readed - spos );
	dpos += readed-spos;

	fclose(f);
	f = fopen("/etc/ppp/pppoe.conf", "w");
	if ( !f )
	{
		eDebug("couldn't open '/etc/ppp/pppoe.conf' for write");
		return;
	}
	unsigned int written;
	if ( (written = fwrite( dest, 1, dpos, f )) != dpos )
		eDebug("couldn't write correct count of bytes...\n%d bytes written %d should be written", written, dpos );
	fclose(f);
}

#endif

void eZapNetworkSetup::getNameserver( __u32 &ip )
{
	char buf[256];
	__u32 tmp=UINT_MAX;

	FILE *file=fopen("/etc/resolv.conf","r");
	if (!file)
		return;

	int tmp1,tmp2,tmp3,tmp4;

	while (fgets(buf,sizeof(buf),file))
	{
		if (sscanf(buf,"nameserver %d.%d.%d.%d", &tmp1, &tmp2, &tmp3, &tmp4) == 4)
		{
			ip=tmp=tmp1<<24|tmp2<<16|tmp3<<8|tmp4;
//			sprintf(ip, "0x%02x%02x%02x%02x", tmp1, tmp2, tmp3, tmp4);
			break;
		}
	}
	fclose(file);
	if ( tmp == UINT_MAX )
		eDebug("parse resolv.conf failed");
}

void eZapNetworkSetup::getDefaultGateway(__u32 &ip)
{
	char iface[9];
	unsigned int dest=0, gw=UINT_MAX;
	char buf[256];

	FILE *file=fopen("/proc/net/route","r");
	if (!file)
		return;
	fgets(buf,sizeof(buf),file);
	while(fgets(buf,sizeof(buf),file))
	{
		if (sscanf(buf,"%8s %x %x", iface, &dest, &gw) == 3)
		{
			if (!dest)
			{
				ip = gw;
				break;
			}
		}
	}
	if (gw == UINT_MAX)
		eDebug("get route failed");
	fclose(file);
}

void eZapNetworkSetup::getIP( char *dev, __u32 &ip, __u32 &mask)
{
	int fd;
	struct ifreq req;
	struct sockaddr_in *saddr;
	unsigned char *addr;

	fd=socket(AF_INET,SOCK_DGRAM,0);
	if ( !fd )
		return;

	memset(&req, 0, sizeof(req));
	strcpy(req.ifr_name,dev);
	saddr = (struct sockaddr_in *) &req.ifr_addr;
	addr = (unsigned char*) &saddr->sin_addr.s_addr;

	if( ::ioctl(fd,SIOCGIFADDR,&req) < 0 )
		eDebug("SIOCGIFADDR failed(%m)");
	else
		ip = addr[0]<<24|addr[1]<<16|addr[2]<<8|addr[3];	

	if( ::ioctl(fd,SIOCGIFNETMASK,&req) < 0 )
		eDebug("SIOCGIFNETMASK failed(%m)");
	else
		mask = addr[0]<<24|addr[1]<<16|addr[2]<<8|addr[3];

	::close(fd);
}

eZapNetworkSetup::eZapNetworkSetup()
	:ePLiWindow(_(MENUNAME), 340)
{
	init_eZapNetworkSetup();
}

void eZapNetworkSetup::init_eZapNetworkSetup()
{
	__u32 sip=ntohl(0xC0A80000),
				snetmask=ntohl(0xFFFFFF00),
				sdns=ntohl(0xC0A80000),
				sgateway=ntohl(0xC0A80000);

	int de[4];
	int sdosetup=1;
	int connectionType=0;
	int useDHCP=1;

#ifdef USE_IFUPDOWN
	bool automatic_start;
	std::string Ip, Netmask, Broadcast, Gateway;
	useDHCP=!getInetAttributes("eth0", automatic_start, Ip, Netmask, Broadcast, Gateway);
	sdosetup=automatic_start;
#else
	eConfig::getInstance()->getKey("/elitedvb/network/usedhcp", useDHCP);
	eConfig::getInstance()->getKey("/elitedvb/network/dosetup", sdosetup);
#endif
	if (useDHCP)
	{
		getIP("eth0", sip, snetmask);
		getDefaultGateway(sgateway);
		getNameserver(sdns);
	}
	else
	{
#ifdef USE_IFUPDOWN
		int tmp[4];
		if ( sscanf(Ip.c_str(), "%d.%d.%d.%d", tmp, tmp+1, tmp+2, tmp+3) == 4 )
			sip = tmp[0]<<24 | tmp[1] << 16 | tmp[2] << 8 | tmp[3];
		else
			eDebug("couldn't parse ip(%s)", Ip.length()?Ip.c_str():"");

		if ( sscanf(Netmask.c_str(), "%d.%d.%d.%d", tmp, tmp+1, tmp+2, tmp+3) == 4 )
			snetmask = tmp[0]<<24 | tmp[1] << 16 | tmp[2] << 8 | tmp[3];
		else
			eDebug("couldn't parse netmask(%s)", Netmask.length()?Netmask.c_str():"");

		if ( sscanf(Gateway.c_str(), "%d.%d.%d.%d", tmp, tmp+1, tmp+2, tmp+3) == 4 )
			sgateway = tmp[0]<<24 | tmp[1] << 16 | tmp[2] << 8 | tmp[3];
		else
			eDebug("couldn't parse gateway(%s)", Gateway.length()?Gateway.c_str():"");

		getNameserver(sdns);  // read always from /etc/resolv.conf
#else
		eConfig::getInstance()->getKey("/elitedvb/network/ip", sip);
		eConfig::getInstance()->getKey("/elitedvb/network/netmask", snetmask);
		eConfig::getInstance()->getKey("/elitedvb/network/gateway", sgateway);
		eConfig::getInstance()->getKey("/elitedvb/network/dns", sdns);
#endif
	}
	eConfig::getInstance()->getKey("/elitedvb/network/connectionType", connectionType);

	dosetup=new eCheckbox(this, sdosetup, 1);
	dosetup->setText(_("Enable network"));
	dosetup->move(ePoint(10, yPos()));
	dosetup->resize(eSize(320, widgetHeight()));
	dosetup->setHelpText(_("Enable or disable the ethernet connection"));

	nextYPos();
	dhcp = new eCheckbox(this, useDHCP, 1);
	dhcp->setText(_("DHCP"));
	dhcp->move(ePoint(10, yPos()));
	dhcp->resize(eSize(320, widgetHeight()));
	dhcp->setHelpText(_("Get network settings from a DHCP server in the local network"));
	CONNECT(dhcp->checked, eZapNetworkSetup::dhcpStateChanged);

	nextYPos(35);
	eLabel *l=new eLabel(this);
	l->setText(_("IP:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(150, widgetHeight()));

	eNumber::unpack(sip, de);
	ip=new eNumber(this, 4, 0, 255, 3, de, 0, l, !useDHCP);
	ip->move(ePoint(160, yPos()));
	ip->resize(eSize(170, widgetHeight()));
	ip->setFlags(eNumber::flagDrawPoints);
	ip->setHelpText(_("Enter network IP address (0..9, left, right)"));
	ip->loadDeco();
	CONNECT(ip->selected, eZapNetworkSetup::fieldSelected);

	nextYPos(35);
	l=new eLabel(this);
	l->setText(_("Netmask:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(150, widgetHeight()));

	eNumber::unpack(snetmask, de);
	netmask=new eNumber(this, 4, 0, 255, 3, de, 0, l, !useDHCP);
	netmask->move(ePoint(160, yPos()));
	netmask->resize(eSize(170, widgetHeight()));
	netmask->setFlags(eNumber::flagDrawPoints);
	netmask->setHelpText(_("Enter netmask of your network (0..9, left, right)"));
	netmask->loadDeco();
	CONNECT(netmask->selected, eZapNetworkSetup::fieldSelected);

#if 0 // Issue 689: Remove the whole LAN combobox as this is the only choice
	nextYPos(35);
	l=new eLabel(this);
	l->setText(_("Type:"));
	l->move(ePoint(10, yPos()));
	l->resize(eSize(140, widgetHeight()));

	eListBoxEntryText *sel=0;
	combo_type=new eComboBox(this, 3, l);
	combo_type->move(ePoint(160, yPos()));
	combo_type->resize(eSize(170, widgetHeight()));
	combo_type->loadDeco();
	combo_type->setHelpText(_("Select network type"));
	((eZapNetworkSetup*)combo_type)->setProperty("showEntryHelp", "");
#ifdef ENABLE_PPPOE
	if ( !connectionType )
#endif
	{
		sel = new eListBoxEntryText( *combo_type, _("LAN"), (void*)0, 0, _("communicate to Local Area Network"));
#ifdef ENABLE_PPPOE
		new eListBoxEntryText( *combo_type, _("WAN(PPPoE)"), (void*)1, 0, _("communicate to the Internet via DSL"));
#endif
	}
#ifdef ENABLE_PPPOE
	else
	{
		new eListBoxEntryText( *combo_type, _("LAN"), (void*)0, 0, _("communicate to Local Area Network"));
		sel = new eListBoxEntryText( *combo_type, _("WAN(PPPoE)"), (void*)1, 0, _("communicate to the Internet via DSL"));
	}
	CONNECT(combo_type->selchanged, eZapNetworkSetup::typeChanged);
	tdsl = new eButton(this);
	tdsl->move(ePoint(340, yPos()));
	tdsl->resize(eSize(100, widgetHeight()));
	tdsl->setText("T-DSL");
	tdsl->loadDeco();
	tdsl->hide();
	tdsl->setHelpText(_("T-Online User press ok here"));
	CONNECT( tdsl->selected, eZapNetworkSetup::tdslPressed );
#endif
#endif // End issue 689

	nextYPos(35);
	lNameserver=new eLabel(this, 0);
	lNameserver->setText(_("Nameserver:"));
	lNameserver->move(ePoint(10, yPos()));
	lNameserver->resize(eSize(140, widgetHeight()));

	eNumber::unpack(sdns, de);
	dns=new eNumber(this, 4, 0, 255, 3, de, 0, lNameserver, !useDHCP);
	dns->move(ePoint(160, yPos()));
	dns->resize(eSize(170, widgetHeight()));
	dns->setFlags(eNumber::flagDrawPoints);
	dns->setHelpText(_("Enter your domain name server (0..9, left, right)"));
	dns->loadDeco();
	CONNECT(dns->selected, eZapNetworkSetup::fieldSelected);

#ifdef ENABLE_PPPOE
	nextYPos();
	lLogin=new eLabel(this);
	lLogin->setText(_("Login:"));
	lLogin->move(ePoint(10, yPos()));
	lLogin->resize(eSize(140, widgetHeight()));
	lLogin->hide();

	char *strLogin=0;
	eConfig::getInstance()->getKey("/elitedvb/network/login", strLogin);
	login=new eTextInputField(this,lLogin);
	login->move(ePoint(160, yPos()));
	login->resize(eSize(280, widgetHeight()));
	login->setMaxChars(100);
	login->loadDeco();
	login->setHelpText(_("press ok to edit your provider login name"));
	if ( strLogin )
		login->setText(strLogin);
	login->hide();
	CONNECT(login->selected, eZapNetworkSetup::loginSelected );
#endif

	nextYPos(35);
	lGateway=new eLabel(this);
	lGateway->setText(_("Gateway:"));
	lGateway->move(ePoint(10, yPos()));
	lGateway->resize(eSize(140, widgetHeight()));

	eNumber::unpack(sgateway, de);
	gateway=new eNumber(this, 4, 0, 255, 3, de, 0, l, !useDHCP);
	gateway->move(ePoint(160, yPos()));
	gateway->resize(eSize(170, widgetHeight()));
	gateway->setFlags(eNumber::flagDrawPoints);
	gateway->setHelpText(_("Enter your gateway IP address (0..9, left, right)"));
	gateway->loadDeco();
	CONNECT(gateway->selected, eZapNetworkSetup::fieldSelected);

#ifdef ENABLE_PPPOE
	nextYPos();
	lPassword=new eLabel(this);
	lPassword->setText(_("Password:"));
	lPassword->move(ePoint(10, yPos()));
	lPassword->resize(eSize(150, widgetHeight()));

	password=new eTextInputField(this,lPassword);
	password->move(ePoint(160, yPos()));
	password->resize(eSize(280, widgetHeight()));
	password->setMaxChars(30);
	password->loadDeco();
	password->setHelpText(_("press ok to edit your provider password"));
	password->hide();
	CONNECT(password->selected, eZapNetworkSetup::passwordSelected);

	nextYPos();
	int flags = getRejectFlags();
	rejectWWW=new eCheckbox(this, flags&1, 1);
	rejectWWW->setText("WWW");
	rejectWWW->move(ePoint(20, yPos()));
	rejectWWW->resize(eSize(90, widgetHeight()));
	eString t = _("reject incoming connections on port 80");
	unsigned int pos = t.find("80");
	if (pos != eString::npos )
	{
		t.erase(pos,2);
		t.insert(pos,eString().sprintf("%d", webifport));
	}
	rejectWWW->setHelpText(t);
	rejectWWW->hide();

	rejectTelnet=new eCheckbox(this, flags&2, 1);
	rejectTelnet->setText("Telnet");
	rejectTelnet->move(ePoint(130, yPos()));
	rejectTelnet->resize(eSize(90, widgetHeight()));
	rejectTelnet->setHelpText(_("reject incoming connections on port 23"));
	rejectTelnet->hide();

	rejectSamba=new eCheckbox(this, flags&4, 1);
	rejectSamba->setText("Samba");
	rejectSamba->move(ePoint(240, yPos()));
	rejectSamba->resize(eSize(100, widgetHeight()));
	rejectSamba->setHelpText(_("reject incoming connections on ports 137,138,139"));
	rejectSamba->hide();

	rejectFTP=new eCheckbox(this, flags&8, 1);
	rejectFTP->setText("FTP");
	rejectFTP->move(ePoint(360, yPos()));
	rejectFTP->resize(eSize(70, widgetHeight()));
	rejectFTP->setHelpText(_("reject incoming connections on ports 21"));
	rejectFTP->hide();
#endif

	buildWindow();
	CONNECT(bOK->selected, eZapNetworkSetup::okPressed);

	/* help text for network setup screen */
	setHelpText(_("\tCommunication Setup\n\n>>> [MENU] >>> [6] Setup >>> [6] Expert Setup\n>>> [1] Communication Setup\n. . . . . . . . . .\n\n" \
		"You will need to setup your network settings in order to be able to share A/V Recordings, FTP/HTTP/Telnet access, Streaming A/V to," \
		" and from your Dreambox.\n. . . . . . . . . .\n\nUsage:\n\n[UP]/[DOWN]\tSelect Inputfield or Button\n\n" \
		"[NUMBERS]\tEnter numbers/text\n\n[LEFT]/[RIGHT]\tPrevious/next number field\n\nIP: [NUMBERS]\tIP-Address of your Dreambox\n\n" \
		"Netmask: [NUMBERS]\nNetmask of your network (usually 255.255.255.0)\n\nNameserver: [NUMBERS]\n" \
		"The IP-Address of a Nameservers (or Gateway)\n\nGateway: [NUMBERS]\nIP-Address of your Default Gateway\n\n" \
		"Enable Network: [OK]\nActivate the Network\n\n[GREEN]\tSave Settings and Close Window\n\n" \
		"[MOUNTS]\tGoto Mount menu\n\n[EXIT]\tClose window without saving changes"));

	// Issue 689 combo_type->setCurrent(sel,true);

#ifdef ENABLE_PPPOE
	if ( readSecretString( secrets ) && secrets )
	{
		unsigned int pos = secrets.find("*");
		if ( pos != eString::npos )
		{
			login->setText( secrets.left(pos) );
			password->setText("******");
		}
	}
#endif
	setFocus(dosetup);
}

eZapNetworkSetup::~eZapNetworkSetup()
{
}

void eZapNetworkSetup::fieldSelected(int *number)
{
	focusNext(eWidget::focusDirNext);
}

void eZapNetworkSetup::dhcpStateChanged(int state)
{
	// Issue 689 netmask->setActive(!state, combo_type);
	ip->setActive(!state, netmask);
	gateway->setActive(!state, bOK);
	dns->setActive(!state, gateway);
}

void eZapNetworkSetup::okPressed()
{
	int eIP[4], eMask[4], eDNS[4], eGateway[4];
	for (int i=0; i<4; i++)
	{
		eIP[i]=ip->getNumber(i);
		eMask[i]=netmask->getNumber(i);
		eGateway[i]=gateway->getNumber(i);
		eDNS[i]=dns->getNumber(i);
	}

	int useDHCP=dhcp->isChecked();
	int sdosetup=dosetup->isChecked();
#if 0 // Issue 689 
	int type = (int) combo_type->getCurrent()->getKey();

	eConfig::getInstance()->setKey("/elitedvb/network/connectionType", type );
#endif

#ifdef USE_IFUPDOWN
	bool automatic_start = sdosetup;
	if (useDHCP)
	{
		addLoopbackDevice("lo", true);
		setDhcpAttributes("eth0", automatic_start);
	}
	else
	{
		eString Ip, Netmask, Broadcast, Gateway;
		Ip.sprintf("%d.%d.%d.%d", eIP[0], eIP[1], eIP[2], eIP[3]);
		Netmask.sprintf("%d.%d.%d.%d", eMask[0], eMask[1], eMask[2], eMask[3]);
		int bcast = ip->getNumber() | (~netmask->getNumber());
		__u8 tmp[4];
		tmp[0] = ~((__u8)eMask[0]);
		tmp[1] = ~((__u8)eMask[1]);
		tmp[2] = ~((__u8)eMask[2]);
		tmp[3] = ~((__u8)eMask[3]);
		Broadcast.sprintf("%d.%d.%d.%d", eIP[0] | tmp[0], eIP[1] | tmp[1], eIP[2] | tmp[2], eIP[3] | tmp[3]);
		Gateway.sprintf("%d.%d.%d.%d", eGateway[0], eGateway[1], eGateway[2], eGateway[3]);
		addLoopbackDevice("lo", true);
		setStaticAttributes("eth0", automatic_start, Ip, Netmask, Broadcast, Gateway);

		FILE *f=fopen("/etc/resolv.conf", "wt");
		if (!f)
			eDebug("couldn't write resolv.conf");
		else
		{
			fprintf(f, "# Generated by enigma\nnameserver %d.%d.%d.%d\n", eDNS[0], eDNS[1], eDNS[2], eDNS[3]);
			fclose(f);
		}
	}
#else
	eConfig::getInstance()->setKey("/elitedvb/network/dosetup", sdosetup);
	eConfig::getInstance()->setKey("/elitedvb/network/usedhcp", useDHCP);
	if ( !useDHCP )
	{
		__u32 sip, snetmask, sdns, sgateway;
		eNumber::pack(sip, eIP);
		eNumber::pack(snetmask, eMask);
		eNumber::pack(sdns, eDNS);
		eNumber::pack(sgateway, eGateway);
		eConfig::getInstance()->setKey("/elitedvb/network/ip", sip);
		eConfig::getInstance()->setKey("/elitedvb/network/netmask", snetmask);
		eConfig::getInstance()->setKey("/elitedvb/network/dns", sdns);
		eConfig::getInstance()->setKey("/elitedvb/network/gateway", sgateway);
	}
#endif
	eConfig::getInstance()->flush();

#ifdef ENABLE_PPPOE
	if (type)
	{
		int flags=0;
		if ( rejectWWW->isChecked() )
			flags |= 1;
		if ( rejectTelnet->isChecked() )
			flags |= 2;
		if ( rejectSamba->isChecked() )
			flags |= 4;
		if ( rejectFTP->isChecked() )
			flags |= 8;
		writeSecretString( secrets );
		updatePPPConfig( secrets,flags );
	}
#endif

#ifdef USE_IFUPDOWN
	system("ifdown eth0");
	system("killall -9 udhcpc");
	if ( automatic_start )
	{
		pid_t pid;
		switch(pid=fork())
		{
			case -1:
				eDebug("error fork ifup (%m)");
				break;
			case 0:
				for (unsigned int i=3; i < 90; ++i )
					::close(i);
				system("ifup eth0");
				_exit(0);
				break;
			default:
				waitpid(pid,0,0);
				break;
		}
	}
#endif
	// when USE_IFUPDOWN is defined this only do try to mount the automounts
	eDVB::getInstance()->configureNetwork();

	close(0);
}

#ifdef ENABLE_PPPOE
void eZapNetworkSetup::typeChanged(eListBoxEntryText *le)
{
	if ( le )
	{
		if ( le->getKey() )
		{
			rejectWWW->show();
			rejectFTP->show();
			rejectTelnet->show();
			rejectSamba->show();
			tdsl->show();
			lNameserver->hide();
			lGateway->hide();
			dns->hide();
			gateway->hide();
			lLogin->show();
			lPassword->show();
			login->show();
			password->show();
		}
		else
		{
			rejectWWW->hide();
			rejectFTP->hide();
			rejectTelnet->hide();
			rejectSamba->hide();
			tdsl->hide();
			lLogin->hide();
			lPassword->hide();
			login->hide();
			password->hide();
			lNameserver->show();
			lGateway->show();
			dns->show();
			gateway->show();
		}
	}
}

void eZapNetworkSetup::loginSelected()
{
	if ( !login->inEditMode() )
	{
		unsigned int pos =
			secrets.find("*");
		eString pw;
		if ( pos )
			pw = secrets.mid(pos+1, secrets.length() - pos - 1 );
		secrets = login->getText()+'*'+pw;
	}
}

void eZapNetworkSetup::passwordSelected()
{
	if ( password->inEditMode() )
	{
		unsigned int pos = secrets.find("*");
		if ( pos != eString::npos )
			password->setText( secrets.mid( pos+1, secrets.length() - pos - 1 ) );
	}
	else
	{
		secrets=login->getText()+'*'+password->getText();
		password->setText("******");
	}
}

void eZapNetworkSetup::tdslPressed()
{
	hide();
	eTOnlineDialog dlg(login->getText());
	dlg.show();
	if ( !dlg.exec() )
	{
		login->setText(dlg.getLogin());
		unsigned int pos =
			secrets.find("*");
		eString pw;
		if ( pos )
			pw = secrets.mid(pos+1, secrets.length() - pos - 1 );
		secrets = login->getText()+'*'+pw;
	}
	dlg.hide();
	show();
	setFocus(tdsl);
}
#endif // ENABLE_PPPOE

void eZapNetworkSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

#endif // DISABLE_NETWORK
