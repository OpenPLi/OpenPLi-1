#include "upgrade.h"
#include <unistd.h>
#include <xmltree.h>
#include <enigma.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/emessage.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/eskin.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/edvb.h>
#include <lib/system/info.h>
#include <lib/gdi/font.h>
#include <lib/gdi/epng.h>
#include <lib/gdi/gfbdc.h>
#include <lib/gdi/fb.h>
#include <libmd5sum.h>

#include <sys/mman.h>

#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7)) && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
#include <linux/compiler.h>
#endif
#include <mtd/mtd-user.h>
#include <sys/ioctl.h>
#include <sys/mount.h>

#define TMP_IMAGE "/tmp/root.cramfs"
#define TMP_IMAGE_ALT "/tmp/cdk.cramfs"
#define TMP_CHANGELOG "/tmp/changelog"

class ProgressWindow: public eWindow
{
	void init_ProgressWindow( const char *wtext );
public:
	eProgress progress;
	ProgressWindow( const char * );
};

ProgressWindow::ProgressWindow( const char *wtext )
	:eWindow(0), progress(this)
{
	init_ProgressWindow(wtext);
}
void ProgressWindow::init_ProgressWindow( const char *wtext )
{
	cresize(eSize(470,50));
	valign();
	setText(wtext);
	progress.move(ePoint(10,10));
	progress.resize(eSize(450,30));
}

static eString getVersionInfo(const char *info)
{
	FILE *f=fopen("/.version", "rt");
	if (!f)
		return "";
	eString result;
	while (1)
	{
		char buffer[128];
		if (!fgets(buffer, 128, f))
			break;
		if (strlen(buffer))
			buffer[strlen(buffer)-1]=0;
		if ((!strncmp(buffer, info, strlen(info)) && (buffer[strlen(info)]=='=')))
		{
			int i = strlen(info)+1;
			result = eString(buffer).mid(i, strlen(buffer)-i);
			break;
		}
	}	
	fclose(f);
	return result;
}

eListBoxEntryImage::eListBoxEntryImage
	(eListBox<eListBoxEntryImage> *listbox, eString name, eString target, eString url, eString version, eString creator, const unsigned char md5[16])
	: eListBoxEntryText((eListBox<eListBoxEntryText> *)listbox, name),
	name(name), target(target), url(url), version(version), creator(creator)
{
	if (md5)
		memcpy(this->md5, md5, 16);
	else
		memset(this->md5, 0, 16);
}

eHTTPDownload::eHTTPDownload(eHTTPConnection *c, const char *filename): eHTTPDataSource(c), filename(filename)
{
	init_eHTTPDownload(c,filename);
}
void eHTTPDownload::init_eHTTPDownload(eHTTPConnection *c,const char *filename)
{
	if (c->remote_header.count("Content-Length"))
		total=atoi(c->remote_header["Content-Length"].c_str());
	else
		total=-1;
	received=0;
	fd=::creat(filename, 0777);
	progress(received, total);
}

eHTTPDownload::~eHTTPDownload()
{
	if (fd >= 0)
		::close(fd);
	if ((total != -1) && (total != received))
		::unlink(filename.c_str());
}

void eHTTPDownload::haveData(void *data, int len)
{
	if (len)
	{
		if (fd >= 0)
			::write(fd, data, len);
	}
	received+=len;
	progress(received, total);
}

eHTTPDownloadXML::eHTTPDownloadXML(eHTTPConnection *c, XMLTreeParser &parser): eHTTPDataSource(c), parser(parser)
{
	error=0;
	errorstring="";
}

void eHTTPDownloadXML::haveData(void *data, int len)
{
	if ((!error) && (!parser.Parse((char*)data, len, !data)))
	{
		errorstring.sprintf("XML parse error: %s at line %d",
			parser.ErrorString(parser.GetErrorCode()),
			parser.GetCurrentLineNumber());
		error=1;
	}
}

eUpgrade::eUpgrade(bool manual)
:http(0), changelog(0)
{
	init_eUpgrade(manual);
}
void eUpgrade::init_eUpgrade(bool manual)
{
	status = new eStatusBar(this);
	status->setFlags(eStatusBar::flagOwnerDraw);
	status->loadDeco();
	status->setName("status");

	images=new eListBox<eListBoxEntryImage>(this);
	images->setName("images");
	CONNECT(images->selected, eUpgrade::imageSelected);
	CONNECT(images->selchanged, eUpgrade::imageSelchanged);
	
	imagehelp=new eLabel(this);
	imagehelp->setName("imagehelp");
	imagehelp->setText(_("Please select the software version to upgrade to:"));

	progress=new eProgress(this);
	progress->setName("progress");
	progress->hide();
	
	progresstext=new eLabel(this);
	progresstext->setName("progresstext");
	progresstext->hide();
	
	changes=new eLabel(this, RS_WRAP);
	changes->setName("changes");
	
	abort=new eButton(this);
	abort->setName("abort");
	CONNECT(abort->selected, eUpgrade::abortDownload);
	abort->hide();

	if (eSkin::getActive()->build(this, "eUpgrade"))
		eFatal("skin load of \"eUpgrade\" failed");

	catalog=0;
	changelog=0;

	mIDStr=eSystemInfo::getInstance()->getmidStr();

	eString caturl=getVersionInfo("catalog");

	if (caturl.length() && !manual )
		loadCatalog(caturl.c_str());

	ourversion=getVersionInfo("version");
	
	struct stat s;
	if (!stat(TMP_IMAGE_ALT, &s))
		rename(TMP_IMAGE_ALT, TMP_IMAGE);
	if (!stat(TMP_IMAGE, &s))
		new eListBoxEntryImage(images, _("manual upload"), "", "", "", "", 0);

	/* help text for software update screen */
	setHelpText(_("\tSoftware Update\n\n-> [MENU] ->(6)Setup ->Update\n. . . . . . . . . .\n\nENIGMA is always under development\n\n" \
								"Here you can install an updated version of ENIGMA\n. . . . . . . . . .\n\nUsage:\n\n" \
								"Version:[UP]/[DOWN],[OK]\tSelect software version\n\n[OK]\tStart update\n\n[EXIT]\tClose window without saving changes"));
}

void eUpgrade::loadCatalog(const char *url)
{
	current_url=url;
	int error;
	if (catalog)
		delete catalog;
	catalog=new XMLTreeParser("ISO-8859-1");
	http=eHTTPConnection::doRequest(url, eApp, &error);
	if (!http)
	{
		catalogTransferDone(error);
	} else
	{
		setStatus(_("downloading catalog..."));
		CONNECT(http->transferDone, eUpgrade::catalogTransferDone);
		CONNECT(http->createDataSource, eUpgrade::createCatalogDataSink);
		http->local_header["User-Agent"]="enigma-upgrade/1.0.0";
		http->start();
	}
}

void eUpgrade::loadChangelog(const char *url)
{
	current_url=url;
	int error;
	if (changelog)
		delete changelog;
	changelog=eHTTPConnection::doRequest(url, eApp, &error);
	if (!changelog)
	{
		changelogTransferDone(error);
	} else
	{
		setStatus(_("downloading changelog..."));
		CONNECT(changelog->transferDone, eUpgrade::changelogTransferDone);
		CONNECT(changelog->createDataSource, eUpgrade::createChangelogDataSink);
		changelog->local_header["User-Agent"]="enigma-upgrade/1.0.0";
		changelog->start();
	}
}

void eUpgrade::loadImage(const char *url)
{
	images->hide();
	imagehelp->hide();
	current_url=url;
	int error;
	if (http)
		delete http;
	progress->show();
	progresstext->show();
	abort->show();
	http=eHTTPConnection::doRequest(url, eApp, &error);
	if (!http)
	{
		imageTransferDone(error);
	} else
	{
		setStatus(_("downloading image..."));
		CONNECT(http->transferDone, eUpgrade::imageTransferDone);
		CONNECT(http->createDataSource, eUpgrade::createImageDataSink);
		http->local_header["User-Agent"]="enigma-upgrade/1.0.0";
		http->start();
	}
}

void eUpgrade::catalogTransferDone(int err)
{
	if ((!err) && http && (http->code == 200) && datacatalog && !datacatalog->error)
	{
		XMLTreeNode *root=catalog->RootNode();
		eString mytarget=mIDStr;
		images->beginAtomic();
		for (XMLTreeNode *r=root->GetChild(); r; r=r->GetNext())
		{
			if (!strcmp(r->GetType(), "image"))
			{
				const char *name=r->GetAttributeValue("name");
				const char *url=r->GetAttributeValue("url");
				const char *version=r->GetAttributeValue("version");
				const char *target=r->GetAttributeValue("target");
				const char *creator=r->GetAttributeValue("creator");
				const char *amd5=r->GetAttributeValue("md5");
				unsigned char md5[16];
				if (!creator)
					creator=_("unknown");
				if (!amd5)
					continue;
				for (int i=0; i<32; i+=2)
				{
					char x[3];
					x[0]=amd5[i];
					if (!x[0])
						break;
					x[1]=amd5[i+1];
					if (!x[1])
						break;
					int v=0;
					if (sscanf(x, "%02x", &v) != 1)
						break;
					md5[i/2]=v;
				}
				if (!(name && url && version && target))
					continue;
				if (!strstr(target, mytarget.c_str()))
					continue;
				new eListBoxEntryImage(images, name, target, url, version, creator, md5);
			} else if (!strcmp(r->GetType(), "changelog"))
			{
				const char *changelog=r->GetAttributeValue("url");
				if (changelog)
					loadChangelog(changelog);
			}
		}
		setFocus(images);
		images->endAtomic();
		setStatus(_("Please select version to upgrade or LAME! to abort"));
		if (images->getCurrent())
			imageSelchanged(images->getCurrent());
	} else
	{
		if (err || http->code !=200)
			setError(err);
		else if (datacatalog)
		{
			eDebug("data error.");
			// setStatus(datacatalog->errorstring);
			setStatus("XML parse error.");
		}
	}
	if (catalog)
		delete catalog;
	http=0;
}

void flashImage(int checkmd5);

void eUpgrade::imageTransferDone(int err)
{
	progress->hide();
	progresstext->hide();
	abort->hide();
	if (err || !http || http->code != 200)
		setError(err);
	else
		flashImage(1);
	http=0;
	images->show();
	imagehelp->show();
}

void eUpgrade::changelogTransferDone(int err)
{
	if (err || !changelog || changelog->code != 200)
	{
		setError(err);
	} else
	{
		FILE *f=fopen(TMP_CHANGELOG, "rt");
		if (f)
		{
			char temp[1024];
			while (fgets(temp, 1024, f))
			{
				if (*temp)	
					temp[strlen(temp)-1]=0; // remove trailng \n
				eString str(temp);
				changelogEntry entry;
				
				entry.date=str.left(12);
				entry.priority=str[13]-'0';
				entry.text=str.mid(15);
				unsigned int in=entry.text.find(' ');
				entry.machines="*";
				if (in != eString::npos)
				{
					entry.machines=entry.text.left(in);
					entry.text=entry.text.mid(in+1);
				}
				changelogentries.push_back(entry);
			}
			fclose(f);
		}
		displayChangelog(ourversion.mid(4), selectedversion.mid(4), mIDStr);
	}
	changelog=0;
}

void eUpgrade::imageSelected(eListBoxEntryImage *img)
{
	if (img)
	{
		if (img->url.length())
		{
			memcpy(expected_md5, img->md5, 16);
			setStatus(img->url);
			loadImage(img->url.c_str());
		} else
			flashImage(0);
	} else
		close(0); // aborted
}

void eUpgrade::imageSelchanged(eListBoxEntryImage *img)
{
	selectedversion=img->version;
	displayChangelog(ourversion.mid(4), selectedversion.mid(4), mIDStr);
}

void eUpgrade::setStatus(const eString &string)
{
	status->setText(string);
}

void eUpgrade::setError(int err)
{
	eString errmsg;
	switch (err)
	{
	case 0:
		if (http && http->code != 200)
			errmsg="error: server replied " + eString().setNum(http->code) + " " + http->code_descr;
		break;
	case -2:
		errmsg="Can't resolve hostname!";
		break;
	case -3:
		errmsg="Can't connect! (check network settings)";
		break;
	default:
		errmsg.sprintf("unknown error %d", err);
	}
	setStatus(errmsg);
	if (errmsg.length())
	{
		if (current_url.length())
			errmsg+="\n(URL: " + current_url + ")";
		eMessageBox::ShowBox(errmsg, _("Error!"), eMessageBox::btOK|eMessageBox::iconError);
	}
}

eHTTPDataSource *eUpgrade::createCatalogDataSink(eHTTPConnection *conn)
{
	return datacatalog=new eHTTPDownloadXML(conn, *catalog);
}

eHTTPDataSource *eUpgrade::createImageDataSink(eHTTPConnection *conn)
{
	image=new eHTTPDownload(conn, TMP_IMAGE);
	lasttime=0;
	CONNECT(image->progress, eUpgrade::downloadProgress);
	return image;
}

eHTTPDataSource *eUpgrade::createChangelogDataSink(eHTTPConnection *conn)
{
	changelogdownload=new eHTTPDownload(conn, TMP_CHANGELOG);
	lasttime=0;
	CONNECT(changelogdownload->progress, eUpgrade::downloadProgress);
	return changelogdownload;
}

void eUpgrade::downloadProgress(int received, int total)
{
	if ((time(0) == lasttime) && (received != total))
		return;
	lasttime=time(0);
	if (total > 0)
	{
		eString pt;
		int perc=received*100/total;
		pt.sprintf("%d/%d kb (%d%%)", received/1024, total/1024, perc);
		progress->setPerc(perc);
		progresstext->setText(pt);
	} else
	{
		eString pt;
		pt.sprintf("%d kb", received/1024);
		progress->setPerc(0);
		progresstext->setText(pt);
	}
}

void eUpgrade::abortDownload()
{
	if (http)
	{
		delete http;
		http=0;
	}
	setStatus(_("Download aborted."));
	progress->hide();
	progresstext->hide();
	abort->hide();
	images->show();
	imagehelp->show();
}

bool erase(char mtd[30], const char *titleText);

void eUpgrade::flashImage(int checkmd5)
{
	setStatus(_("checking consistency of file..."));
	unsigned char md5[16];
	if (checkmd5 && md5_file (TMP_IMAGE, 1, (unsigned char*) &md5))
	{
		setStatus(_("write error while downloading..."));
		hide();
		eMessageBox::ShowBox(
			_("write error while downloading..."),
			_("Error!"),
			eMessageBox::btOK|eMessageBox::iconError);
		show();
	} else
	{
		if (checkmd5 && memcmp(md5, expected_md5, 16))
		{
			setStatus(_("Data error. The checksum didn't match."));
			hide();
			eMessageBox::ShowBox(
				_("Data error. The checksum didn't match."),
				_("Error!"),
				eMessageBox::btOK|eMessageBox::iconError);
			show();
		} else
		{
			setStatus(_("Checksum OK. Ready to upgrade."));
			hide();
			int res = eMessageBox::ShowBox(
				_("Are you sure you want to upgrade to this new version?"),
				_("Ready to upgrade"),
				eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconQuestion);
			int mtdsize;
			if (res == eMessageBox::btYes)
			{
				::sync();
				Decoder::Flush();
				char mtd[20];
				int mID = atoi(mIDStr.c_str());
				switch (mID)
				{
				case 1:   // d-box2
				case 2:
				case 3:
					strcpy(mtd,"/dev/mtd/2");
					mtdsize=0x6e0000;
					break;
				case 5:   // dm7000
				case 6:   // dm56xx
				case 7:   // dm500
				case 8:   // tr272S
					strcpy(mtd,"/dev/mtd/0");
					mtdsize=0x600000;
					break;
				default:
					strcpy(mtd,"/dev/null");
					mtdsize=0;
				}
				int fd1=0,fd2=0;

				if( (fd1 = open( TMP_IMAGE, O_RDONLY )) < 0 )
				{
					eMessageBox::ShowBox(_("Can't read flashimage.img!"), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK );
					close(0);
					return;
				}

				int filesize = lseek( fd1, 0, SEEK_END);
				lseek( fd1, 0, SEEK_SET);

				if(filesize==0)
				{
					eMessageBox::ShowBox(_("flashimage.img has filesize of 0byte!"), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK );
					return;
				}

#ifdef USE_EXTERNAL_FLASHTOOL
				{
					::close(fd1);
					gPixmap pixmap;
					pixmap.x=720;
					pixmap.y=576;
					pixmap.bpp=gFBDC::getInstance()->getPixmap().bpp;
					pixmap.bypp=gFBDC::getInstance()->getPixmap().bypp;
					pixmap.stride=gFBDC::getInstance()->getPixmap().stride;
					__u8 data[720*576*pixmap.bypp];
					pixmap.data=data;
					pixmap.clut.colors=256;
					pixmap.clut.data=gFBDC::getInstance()->getPixmap().clut.data;
					gPixmapDC outputDC(&pixmap);
					eWidget virtualRoot;
					virtualRoot.move(ePoint(0, 0));
					virtualRoot.resize(eSize(720, 576));
					virtualRoot.setTarget(&outputDC);
					virtualRoot.makeRoot();
					virtualRoot.setBackgroundColor(gColor(0));
					virtualRoot.show();
					int ybegin;
					int yend;
					{
						int fillColor = eSkin::getActive()->queryScheme("eProgress.left").color;
						if (pixmap.bpp == 32)
						{
                        				fillColor =   (((pixmap.clut.data[fillColor].a == 0xFF ? 0xFF : gFBDC::getInstance()->getRampAlpha(254)) ^ 0xFF) << 24) |
                                					(gFBDC::getInstance()->getRamp(pixmap.clut.data[fillColor].r)<<16) |
                                					(gFBDC::getInstance()->getRamp(pixmap.clut.data[fillColor].g)<<8) |
                                					(gFBDC::getInstance()->getRamp(pixmap.clut.data[fillColor].b));
						}
						eDebug("Writing upgradescreen1 - erasing...\n");
						eMessageBox mb(
							_("Please wait... do NOT switch off the receiver!"),
							_("upgrade in progress"), eMessageBox::iconInfo);
						mb.show();
						ePoint posmb = mb.getAbsolutePosition();
						ybegin = posmb.y();
						{
							ProgressWindow wnd(_("Erasing Flash..."));
							wnd.show();
							while(gRC::getInstance().mustDraw())
								usleep(1000);
//							if ( !savePNG("/tmp/update1.png", &pixmap) )
//								eDebug("saved update pic 1");
							int fd = open("/tmp/update1.raw", O_CREAT|O_WRONLY|O_TRUNC);
							if ( fd >= 0 )
							{
								ePoint poswnd = wnd.getAbsolutePosition();
								eSize sizewnd = wnd.getSize();
								ePoint pos = wnd.progress.getAbsolutePosition();
								int x = 2+pos.x(); // borderwidth+xpos..
								int y = 2+pos.y(); // borderwidth+ypos..
								int width = wnd.progress.getSize().width()-4;
								int height = wnd.progress.getSize().height()-4;
								yend = poswnd.y()+sizewnd.height();
								write(fd, &ybegin, sizeof(ybegin));
								write(fd, &yend, sizeof(yend));
								write(fd, &x, sizeof(x));
								write(fd, &y, sizeof(y));
								write(fd, &width, sizeof(width));
								write(fd, &height, sizeof(height));
								write(fd, &fillColor, sizeof(fillColor));
								//write(fd, pixmap.data, 720*576*pixmap.bypp);
								write(fd, (__u8*)pixmap.data+ybegin*pixmap.stride, (yend-ybegin)*pixmap.stride);
								::close(fd);
							}
						}
						{
							eDebug("Writing upgradescreen1 - erasing...\n");
							ProgressWindow wnd(_("Writing Software to Flash..."));
							wnd.show();
							while(gRC::getInstance().mustDraw())
								usleep(1000);
//							if ( !savePNG("/tmp/update2.png", &pixmap) )
//								eDebug("saved update pic 2");
							int fd = open("/tmp/update2.raw", O_CREAT|O_WRONLY|O_TRUNC);
							if ( fd >= 0 )
							{
								ePoint poswnd = wnd.getAbsolutePosition();
								eSize sizewnd = wnd.getSize();
								ePoint pos = wnd.progress.getAbsolutePosition();
								int x = 2+pos.x(); // borderwidth+xpos..
								int y = 2+pos.y(); // borderwidth+ypos..
								int width = wnd.progress.getSize().width()-4;
								int height = wnd.progress.getSize().height()-4;
								if (poswnd.y()+sizewnd.height() > yend);
									yend = poswnd.y()+sizewnd.height();
								write(fd, &ybegin, sizeof(ybegin));
								write(fd, &yend, sizeof(yend));
								write(fd, &x, sizeof(x));
								write(fd, &y, sizeof(y));
								write(fd, &width, sizeof(width));
								write(fd, &height, sizeof(height));
								write(fd, &fillColor, sizeof(fillColor));
								//write(fd, pixmap.data, 720*576*pixmap.bypp);
								write(fd, (__u8*)pixmap.data+ybegin*pixmap.stride, (yend-ybegin)*pixmap.stride);
								::close(fd);
							}
						}
					}
					int fd = open("/tmp/fbinfo", O_CREAT|O_WRONLY|O_TRUNC);
					if ( fd >= 0 )
					{
						int bytes = pixmap.bypp;
						int bits = pixmap.bpp;
						write(fd, &bits, sizeof(bits));
						write(fd, &bytes, sizeof(bytes));
						::close(fd);
					}
					pixmap.clut.data=0;
					fd = open("/tmp/mtd.txt", O_WRONLY|O_CREAT|O_TRUNC);
					if ( fd >= 0 )
					{
						write(fd, mtd, strlen(mtd));
						::close(fd);
					}
#if 0
					// recreate the framebuffer cmap. 
					struct fb_cmap mycmap;
					struct fb_cmap* cmap = &mycmap;
        				__u16 red[256], green[256], blue[256], trans[256];
        				mycmap.start  = 0;
        				mycmap.len    = pixmap.clut.colors;
        				mycmap.red    = red;
        				mycmap.green  = green;
        				mycmap.blue   = blue;
        				mycmap.transp = trans;
        				for (int i=0; i< pixmap.clut.colors; ++i)
        				{
                				mycmap.red[i]    = gFBDC::getInstance()->getRamp(pixmap.clut.data[i].r)<<8;
                				mycmap.green[i]  = gFBDC::getInstance()->getRamp(pixmap.clut.data[i].g)<<8;
                				mycmap.blue[i]   = gFBDC::getInstance()->getRamp(pixmap.clut.data[i].b)<<8;
                				mycmap.transp[i] = (pixmap.clut.data[i].a == 0xFF ? 0xFF : gFBDC::getInstance()->getRampAlpha(254)) << 8;
                				if (!mycmap.red[i])
                        				mycmap.red[i]=0x100;
        				}
#else
					struct fb_cmap* cmap = fbClass::getInstance()->CMAP();
#endif
					fd = open("/tmp/cmap", O_WRONLY|O_CREAT|O_TRUNC);
					if ( fd >= 0 )
					{
						write(fd, &cmap->start, sizeof(cmap->start));
						write(fd, &cmap->len, sizeof(cmap->len));
						write(fd, cmap->red, cmap->len*sizeof(__u16));
						write(fd, cmap->green, cmap->len*sizeof(__u16));
						write(fd, cmap->blue, cmap->len*sizeof(__u16));
						write(fd, cmap->transp, cmap->len*sizeof(__u16));
						::close(fd);
					}
					eZap::getInstance()->getDesktop(eZap::desktopFB)->makeRoot();
					fbClass::getInstance()->lock();
				}
#else /* !USE_EXTERNAL_FLASHTOOL */
				{
					eMessageBox mb(
						_("Please wait... do NOT switch off the receiver!"),
						_("upgrade in progress"), eMessageBox::iconInfo);
					mb.show();

					// without this nice we have not enough priority for
					// file operations... then the update ist very slow on the
					// dreambox
					nice(-10);
					eEPGCache::getInstance()->messages.send(eEPGCache::Message(eEPGCache::Message::pause));

					system("cp /sbin/rebootSE /tmp/reboot");

					if(!erase(mtd,_("Erasing Flash...")))
					{
						mb.hide();
						eMessageBox::ShowBox(_("Erase error!"), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK );
						close(0);
						return;
					}

					if( (fd2 = open(mtd, O_RDWR )) < 0 )
					{
						mb.hide();
						eMessageBox::ShowBox(_("Can't open mtd!"), _("Flash"), eMessageBox::iconInfo|eMessageBox::btOK );
						close(0);
						return;
					}

					mtd_info_t meminfo;
					if( ioctl( fd2, MEMGETINFO, &meminfo ) != 0 )
					{
						close(0);
						return;
					}

					ProgressWindow wnd(_("Writing Software to Flash..."));
					wnd.show();

					char buf[meminfo.erasesize];

					int fsize=filesize;

					eDebug("flashing now...");

					int rbytes=0;
					while( ( rbytes = read( fd1, buf, sizeof(buf) ) ) )
					{
						fsize -= write( fd2, buf, rbytes );
						wnd.progress.setPerc( ((filesize-fsize)*100)/filesize );
					}
					::close(fd1);
					::close(fd2);

					nice(0);

					mb.hide();
					wnd.hide();

					eMessageBox::ShowBox(
						_("upgrade successful!\nrestarting..."),
						_("upgrade ok"),
					eMessageBox::btOK|eMessageBox::iconInfo);
				}
#endif
				eZap::getInstance()->quit(3);
//				system("/sbin/reboot");
//				system("/bin/reboot");
//				exit(0);
			} else
				close(0);
		}
	}
}

bool erase(char mtd[30], const char *titleText)
{
	int fd;
	mtd_info_t meminfo;
	erase_info_t erase;

	if( (fd = open( mtd, O_RDWR )) < 0 )
		return false;

	if( ioctl( fd, MEMGETINFO, &meminfo ) != 0 )
		return false;

	ProgressWindow wnd(titleText);
	wnd.show();

	erase.length = meminfo.erasesize;
	for (erase.start = 0; erase.start < meminfo.size; erase.start += meminfo.erasesize )
	{
		eDebugNoNewLine("\rErasing %u Kibyte @ %x -- %2u %% complete",
			meminfo.erasesize/1024, erase.start,
			erase.start*100/meminfo.size );

		wnd.progress.setPerc( erase.start*100/meminfo.size );

		if(ioctl( fd, MEMERASE, &erase) != 0)
		{
			::close(fd);
			wnd.hide();
			return false;
		}
	}
	wnd.hide();

	::close(fd);

	return true;
}

void eUpgrade::displayChangelog(eString oldversion, eString newversion, eString mid)
{
	eString changetext;
	if (newversion.empty())
	{
		changetext="";
	} else if (oldversion == newversion)
	{
		changetext=_("This is the currently installed release.");
	} else if (oldversion > newversion)
	{
		changetext=_("This is an older release.");
	} else if (changelogentries.empty())
	{
		changetext=_("No changelog data available.");
	} else
	{
		// build list of changetext
		
		std::multimap<int,eString> pchanges;
		for (std::list<changelogEntry>::const_iterator i(changelogentries.begin());
				i != changelogentries.end(); ++i)
		{
					// check if change is new for this version
			if ((i->date > oldversion) && (i->date <= newversion) && ((i->machines=="*") || (i->machines.find(mid) != eString::npos)))
				pchanges.insert(std::pair<int,eString>(i->priority, i->text));
		}
		if (pchanges.empty())
			changetext=_("No new features were added in this release.");
		else
		{
			changetext=_("The following new features were added in this release:\n");
			for (std::map<int,eString>::const_iterator i(pchanges.begin());
					i != pchanges.end(); ++i)
				changetext+=" * " + i->second + "\n";
		}
	}
	changes->setText(changetext);
}

eUpgrade::~eUpgrade()
{
	if (http)
		delete http;
	if (changelog)
		delete changelog;
}
