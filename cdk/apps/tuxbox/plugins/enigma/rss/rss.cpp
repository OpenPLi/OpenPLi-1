/* 
Dreambox RSS reader
Copyright (C) 2004 Bjorn Hijmans (bjorn@hijmans.nl)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <string.h>
#include "rss.h"

extern "C" int plugin_exec( PluginParam *par );

int plugin_exec( PluginParam *par )
{
	rssMain dlg;
	dlg.show();
	int result=dlg.exec();
	dlg.hide();

	return result;
}

eString stringReplace( const eString &toReplace, char from, char to )
{	char buffer[8192];
	strcpy(buffer, (char *)toReplace.c_str());
	char *p = buffer;
	while ( *p != '\0' ) 
	{	if ( *p == from )
			*p = to;
		p++;
	}
	return eString( buffer );
}

eString removeTags(const eString& in)
{	
	eString out;
	// Find first <
	int startpos = 0; int length = 0;
	for(unsigned int i = 0; i < in.length(); i++)
	{
		length++;
		if(in.mid(i, 1) == "<")
		{	
			out = out + in.mid(startpos, length - 1);
			// Look for >
			for(unsigned int j = i; j < in.length(); j++)
			{	
				if(in.mid(j, 1) == ">")
				{
					i = j;
					startpos = i + 1;
					length = 0;
					break;
				}
			}
		}
	}
	
	out = out + in.mid(startpos);
	
	if(out == "")
	{	
		out = in;
	}
	
	return out;
}

eString removeTrailingSpaces(const eString& in)
{
	for(unsigned int i = 0; i < in.length(); ++i)
	{
		if(in.mid(i, 1) != " ")
		{
			return in.mid(i);
		}
	}
	
	return "";
}			

rssMain::rssMain(): eWindow(1)
{	cmove(ePoint(140, 120));
	cresize(eSize(440, 336));
	setText("Dreambox RSS reader");

	theList = new eListBox<eListBoxEntryText>(this);
	theList->move(ePoint(10, 10));
	theList->resize(eSize(clientrect.width() - 20, clientrect.height() - 20));
	theList->loadDeco();
	theList->setColumns(1);

	setFocus(theList);

	theConfigParser.parse(CONFIGDIR"/feeds.xml");

	theRssFeed = new rssFeed();
	theRssFeed->hide();

	inDownloadFlag = 0;

	printFeeds();	

	CONNECT(theList->selected, rssMain::selectedItem);
}

void rssMain::printFeeds()
{	std::list<ConfigItem>::iterator i;
	for(i = theConfigParser.configItems.begin(); i != theConfigParser.configItems.end(); ++i)
	{	new eListBoxEntryText(theList, i->name.c_str(), (void *) &(i->id));
	}
}

void rssMain::selectedItem(eListBoxEntryText *item)
{	if(item && !inDownloadFlag)
	{	inDownloadFlag = 1;

		int * iKey = (int *) (item->getKey());

		eString url;
		eString name;

		std::list<ConfigItem>::iterator i;
		for(i = theConfigParser.configItems.begin(); i != theConfigParser.configItems.end(); ++i)
		{	if(i->id == *iKey)
			{	url = i->url;
				name = i->name;
			}
		}

		CONNECT(theFetcher.downloadDone, rssMain::downloadDone);		
		downloadDoneFlag = 0;
		currentName = name;
		theFetcher.fetch(url);
	}
}

void rssMain::downloadDone(int err)
{	if(!downloadDoneFlag)
	{	downloadDoneFlag = 1;
		this->hide();
		theRssFeed->printRSSFeed(currentName);
		theRssFeed->show();
		theRssFeed->exec();
		theRssFeed->hide();
		this->show();
		setFocus(theList);
		inDownloadFlag = 0;
	}
}

rssFeed::rssFeed(): eWindow(1)
{	
	//720x576
	//560x376

	move(ePoint(80, 100));
	resize(eSize(560, 376));
	setText("Feed");

	theList = new eListBox<eListBoxEntryText>(this);
	theList->move(ePoint(10, 10));
	theList->resize(eSize(clientrect.width() - 20, clientrect.height() - 20));
	theList->loadDeco();
	theList->setColumns(1);

	setFocus(theList);

	CONNECT(theList->selected, rssFeed::selectedItem);
}

void rssFeed::printRSSFeed(eString title)
{	eString dest = "/var/tmp/rss.tmp";
	theRSSParser.parse(dest);
	
	setFocus(theList);
	setText(title);
	
	theList->beginAtomic();
	theList->clearList();
	
	if(theList)
	{	// Iterate through list
		std::list<NewsItem>::iterator i;
		for(i = theRSSParser.newsItems.begin(); i != theRSSParser.newsItems.end(); ++i)
		{	new eListBoxEntryText(theList, i->title.c_str(), (void *) (i->id));
		}
	}
	theList->endAtomic();

}

void rssFeed::selectedItem(eListBoxEntryText *item)
{	if(item)
	{	int iKey = (int) (item->getKey());

		eString description;
		eString title;

		std::list<NewsItem>::iterator i;
		for(i = theRSSParser.newsItems.begin(); i != theRSSParser.newsItems.end(); ++i)
		{	if(i->id == iKey)
			{	description = i->description;
				title = i->title;
			}
		}

		//eMessageBox msg(description.c_str(), title.c_str(), eMessageBox::btOK);
		//msg.show();     msg.exec();     msg.hide();
		
		rssDetail dlg(title.c_str(), description.c_str());
		dlg.show();
		dlg.exec();
		dlg.hide();
	}
	else
	{	reject();
	}
}

rssMain::~rssMain()
{
	// we have to do almost nothing here. all widgets are automatically removed
	// since they are children of the main dialog. the eWidget-destructor will do to this.
}

void Fetcher::fetch(eString url)
{	tempPath = "/var/tmp/rss.tmp";
	int error = 0;
	connectionP = eHTTPConnection::doRequest(url.c_str(), eApp, &error);
	
	if(!connectionP || error)
	{	eMessageBox msg("Error downloading " + url + "(" + eString().sprintf("%d", error) + ")", _("Details"), eMessageBox::btOK);
		msg.show();     msg.exec();     msg.hide();
	}
	else
	{	CONNECT(connectionP->transferDone, Fetcher::transferDone);
		CONNECT(connectionP->createDataSource, Fetcher::createDownloadSink);
		connectionP->local_header["User-Agent"] = "RSS";
		connectionP->start();
	}
}

void Fetcher::transferDone(int err)
{	
	if(!err)
	{	connectionP = NULL;

		// Tell caller download is ready
		/*emit*/ downloadDone(err);
	}
	else
	{	eString sMsg = "Error " + eString().sprintf("%d", err);
		eMessageBox msg(sMsg, _("User Abort"), eMessageBox::iconWarning|eMessageBox::btOK);
		msg.show();     msg.exec();     msg.hide();
	}
}

eHTTPDataSource * Fetcher::createDownloadSink(eHTTPConnection *conn)
{	dataSinkP = new eHTTPDownload(connectionP, (char *)tempPath.c_str());

	return(dataSinkP);
}

void ConfigParser::save(ConfigItem i)
{	configItems.push_back(i);
}

// Text ersetzen
void replace(char * pointer, char * alt, char * neu){
char *  dummy = 0;
char *  start = 0;
char *  ziel = 0;
int     laenge;

    if (strstr(pointer, alt)==NULL) return;
    dummy=pointer;
    ziel=pointer;
    while (dummy!=NULL){
        start=dummy;
        dummy=strstr(dummy, alt);
        if (dummy!=NULL){
            laenge=dummy-start;
            if (laenge>=0){
                memcpy(ziel, start, laenge);
                ziel+=laenge;
                strcpy(ziel, neu);
                ziel+=strlen(neu);
            }
            dummy+=strlen(alt);
        }
    }
    strcpy(ziel, start);
}


// HTML in Textdateien wandeln
void replace_sonderzeichen(char * pointer){

// Sonderzeichen ersetzen
    replace(pointer, "&nbsp;", " ");
    replace(pointer, "&quot;", "\"");
    replace(pointer, "&apos;", "'");
    replace(pointer, "&gt;", ">");
    replace(pointer, "&lt;", "<");
    replace(pointer, "&amp;", "&");
// Sonderzeichen ersetzen
    replace(pointer, "&auml;", "ä");
    replace(pointer, "&ouml;", "ö");
    replace(pointer, "&uuml;", "ü");
    replace(pointer, "&Auml;", "Ä");
    replace(pointer, "&Ouml;", "Ö");
    replace(pointer, "&Uuml;", "Ü");
    replace(pointer, "&szlig;", "ß");
    replace(pointer, "&copy;", "\169");
    replace(pointer, "&shy;", "\173");
    replace(pointer, "&Agrave;", "\192");
    replace(pointer, "&Aacute;", "\193");
    replace(pointer, "&Acirc;", "\194");
    replace(pointer, "&Atilde;", "\195");
    replace(pointer, "&Aring;", "\197");
    replace(pointer, "&AElig;", "\198");
    replace(pointer, "&Ccedil;", "\199");
    replace(pointer, "&Egrave;", "\200");
    replace(pointer, "&Eacute;", "\201");
    replace(pointer, "&Ecirc;", "\202");
    replace(pointer, "&Euml;", "\203");
    replace(pointer, "&Igrave;", "\204");
    replace(pointer, "&Iacute;", "\205");
    replace(pointer, "&Icirc;", "\206");
    replace(pointer, "&Iuml;", "\207");
    replace(pointer, "&ETH;", "\208");
    replace(pointer, "&Ntilde;", "\209");
    replace(pointer, "&Ograve;", "\210");
    replace(pointer, "&Oacute;", "\211");
    replace(pointer, "&Ocirc;", "\212");
    replace(pointer, "&Otilde;", "\213");
    replace(pointer, "&Oslash;", "\216");
    replace(pointer, "&Ugrave;", "\217");
    replace(pointer, "&Uacute;", "\218");
    replace(pointer, "&Ucirc;", "\219");
    replace(pointer, "&Yacute;", "\221");
    replace(pointer, "&THORN;", "\222");
    replace(pointer, "&agrave;", "\224");
    replace(pointer, "&aacute;", "\225");
    replace(pointer, "&acirc;", "\226");
    replace(pointer, "&atilde;", "\227");
    replace(pointer, "&aring;", "\229");
    replace(pointer, "&aelig;", "\230");
    replace(pointer, "&ccedil;", "\231");
    replace(pointer, "&egrave;", "\232");
    replace(pointer, "&eacute;", "\233");
    replace(pointer, "&ecirc;", "\234");
    replace(pointer, "&euml;", "\235");
    replace(pointer, "&igrave;", "\236");
    replace(pointer, "&iacute;", "\237");
    replace(pointer, "&icirc;", "\238");
    replace(pointer, "&iuml;", "\239");
    replace(pointer, "&eth;", "\240");
    replace(pointer, "&ntilde;", "\241");
    replace(pointer, "&ograve;", "\242");
    replace(pointer, "&oacute;", "\243");
    replace(pointer, "&ocirc;", "\244");
    replace(pointer, "&otilde;", "\245");
    replace(pointer, "&oslash;", "\248");
    replace(pointer, "&ugrave;", "\249");
    replace(pointer, "&uacute;", "\250");
    replace(pointer, "&ucirc;", "\251");
    replace(pointer, "&yacute;", "\253");
    replace(pointer, "&thorn;", "\254");
    replace(pointer, "&yuml;", "\255");
    replace(pointer, "&#169;", "©");
    replace(pointer, "&middot;", "·");
}

int ConfigParser::parse(eString file)
{	XMLTreeParser * parser;
	FILE *in = fopen(file.c_str(), "rt");

        if(in) 
	{	char buf[2048];
 		char encoding[256];

		int done;
		sprintf(encoding,"ISO-8859-1");
		unsigned int len=fread(buf, 1, sizeof(buf), in);
		char * pointer=strstr(buf, "encoding=");
		if (pointer!=NULL) pointer=strstr(pointer, "\"");
		if (pointer!=NULL){
		    pointer++;
		    int i=0;
		    while ((*pointer)!='\"')
		    {
			encoding[i++]=*pointer++;
		    }
	    	    encoding[i]=0;	
		}
		parser = new XMLTreeParser(encoding);
		do 
		{	done = ( len < sizeof(buf) );
			if ( ! parser->Parse( buf, len, done ) ) 
			{	eMessageBox msg(_("Configfile parse error"), _("User Abort"), eMessageBox::iconWarning|eMessageBox::btOK);
		                msg.show();     msg.exec();     msg.hide();
				delete parser;
				parser = NULL;
				return 0;
			}
			len=fread(buf, 1, sizeof(buf), in);
		} 
		while (!done);

                fclose(in);

		XMLTreeNode * root = parser->RootNode();
		if(!root)
		{	eMessageBox msg(_("Configfile parse error"), _("User Abort"), eMessageBox::iconWarning|eMessageBox::btOK);
		        msg.show();     msg.exec();     msg.hide();	
			return 0;
		}
		if ( strcmp( root->GetType(), "feeds") ) 
		{	eMessageBox msg(_("Invalid configfile"), _("User Abort"), eMessageBox::iconWarning|eMessageBox::btOK);
		        msg.show();     msg.exec();     msg.hide();
	                return 0;
	        }
	
		ConfigItem thisItem;
		int idcnt = 0;

		for(XMLTreeNode * node = root->GetChild(); node; node = node->GetNext())
		{	if(!strcmp(node->GetType(), "feed"))
			{	for(XMLTreeNode * i = node->GetChild(); i; i = i->GetNext())
				{	if(!strcmp(i->GetType(), "name"))
					{	thisItem.name = i->GetData();
						replace_sonderzeichen((char *)thisItem.name.c_str());
					}
					if(!strcmp(i->GetType(), "url"))
					{	thisItem.url = i->GetData();
					}
				}
				thisItem.id = idcnt++;
				save(thisItem);
			}
		}


		delete parser;
		return 1;
	}
	else
	{	eMessageBox msg(_("Config file not found"), _("User Abort"), eMessageBox::iconWarning|eMessageBox::btOK);
		msg.show();     msg.exec();     msg.hide();
		return 0;
	}
}

rssDetail::rssDetail(const char *title, const char *desc) : eWindow(0)
{
	setText(title);

	move(ePoint(80, 123));
	resize(eSize(560, 353));	

	scrollbar = new eProgress(this);
        scrollbar->setStart(0);
        scrollbar->setPerc(100);
        scrollbar->setDirection(1);
        scrollbar->move(ePoint(clientrect.width() - 20, 0));
        scrollbar->resize(eSize(20, clientrect.height() - 30));
        
	descrWidget = new eLabel(this);
	descrWidget->move(ePoint(20, 20));
	descrWidget->resize(eSize(clientrect.width() - 50, clientrect.height() - 50 - 5));
	
	descrLabel = new eLabel(descrWidget);
	descrLabel->setText(desc);
	descrLabel->move(ePoint(0, 0));
	descrLabel->resize(eSize(10, 10));
	descrLabel->setFlags(RS_WRAP);

	float lineheight=fontRenderClass::getInstance()->getLineHeight( descrLabel->getFont() );
	int lines = descrWidget->getSize().height() / (int)lineheight;
	//pageHeight = (int)(lines * lineheight);
	int newheight = lines * (int)lineheight + (int)(round(lineheight + 0.5) - (int)lineheight);
	descrWidget->resize( eSize( descrWidget->getSize().width(), newheight + (int)lineheight/6 ));
	descrLabel->resize( 
		eSize(
			descrWidget->getSize().width(), 
			descrWidget->getSize().height() * 4
		 )
	);

	const eRect &cr = getClientRect();
	eButton * ok = new eButton(this);
	ok->setText("OK");
	ok->move(ePoint((cr.width() - 78)/2, cr.height() - 30));
	ok->resize(eSize(78, 20));
	ok->setShortcut("green");
	ok->setShortcutPixmap("green");

        CONNECT(ok->selected, eWidget::accept);

	updateScrollbar();
}

void rssDetail::updateScrollbar()
{
	total = descrWidget->getSize().height();
	int pages=1;
	while( total < descrLabel->getExtend().height() ) {
		total += descrWidget->getSize().height();
		pages++;
	}

	int start =- ( descrLabel->getPosition().y() * 100 ) /total;
	int vis = descrWidget->getSize().height() *100/total;

	scrollbar->setParams(start, vis);
	scrollbar->show();
	if (pages == 1)
		total = 0;
}

int rssDetail::eventHandler(const eWidgetEvent &event)
{
	switch (event.type) {
        	case eWidgetEvent::evtKey:
			if ( (event.key)->flags == KEY_STATE_DOWN ) {
				switch ( (event.key)->code  ) {
					case RC_UP:
						handleUp();
						break;
					case RC_DN:
						handleDown();
						break;
				}
			}
			break;
		default:
			break;
	}

	return eWindow::eventHandler(event);
}

void rssDetail::handleDown( void )
{
	ePoint curPos = descrLabel->getPosition();
	if ( 
		total &&
		(total - descrWidget->getSize().height() ) >= abs( curPos.y() - descrWidget->getSize().height() ) 
	) {
		descrLabel->move( 
			ePoint( 
				curPos.x(), 
				curPos.y() - descrWidget->getSize().height() 
			)
		);
		updateScrollbar();
	}
}

void rssDetail::handleUp( void )
{
	ePoint curPos = descrLabel->getPosition();
	if ( 
		total && 
		( curPos.y() < 0 )
	) {
		descrLabel->move( 
			ePoint( 
				curPos.x(), 
				curPos.y() + descrWidget->getSize().height() 
			) 
		);
		updateScrollbar();
	}
}

void rssDetail::redrawWidget(gPainter *d, const eRect &area)
{	eWindow::redrawWidget( d, area );
}


void RSSParser::save(NewsItem i)
{	newsItems.push_back(i);
}

void RSSParser::parse(eString file)
{	XMLTreeParser * parser;
	FILE *in = fopen(file.c_str(), "rt");
	newsItems.clear();

        if(in) 
	{	char buf[2048];
 		char encoding[256];

		int done;
		sprintf(encoding,"ISO-8859-1");
		unsigned int len=fread(buf, 1, sizeof(buf), in);
		char * pointer=strstr(buf, "encoding=");
		if (pointer!=NULL) pointer=strstr(pointer, "\"");
		if (pointer!=NULL){
		    pointer++;
		    int i=0;
		    while ((*pointer)!='\"')
		    {
			encoding[i++]=*pointer++;
		    }
	    	    encoding[i]=0;	
		}
		if (strcmp(encoding,"windows-1254")==0){
		    sprintf(encoding,"ISO-8859-9");
		}
		for (unsigned int i=0;i<strlen(encoding);i++){
		    encoding[i]=toupper(encoding[i]);
		}
		if ((strcmp(encoding,"ISO-8859-1")!=0)&&(strcmp(encoding,"UTF-8")!=0)){
		    sprintf(encoding,"ISO-8859-1");
		}
		parser = new XMLTreeParser(encoding);
		if (parser==NULL){
		    parser = new XMLTreeParser("ISO-8859-1");
		}
		do 
		{	done = ( len < sizeof(buf) );
			if ( ! parser->Parse( buf, len, done ) ) 
			{	eMessageBox msg(_("XML parse error (general xml)"), _("Error"), eMessageBox::iconWarning|eMessageBox::btOK);
		                msg.show();     msg.exec();     msg.hide();
				delete parser;
				parser = NULL;
				return;
			}
			len=fread(buf, 1, sizeof(buf), in);
		} 
		while (!done);

                fclose(in);

		XMLTreeNode * root = parser->RootNode();
		if(!root)
		{	eMessageBox msg(_("XML parse error (rootnode)"), _("Error"), eMessageBox::iconWarning|eMessageBox::btOK);
		        msg.show();     msg.exec();     msg.hide();	
			return;
		}
		if (!strcmp( root->GetType(), "rss")) 
		{	NewsItem thisItem;
			int idcnt = 0;

			for(XMLTreeNode * node = root->GetChild(); node; node = node->GetNext())
			{	if(!strcmp(node->GetType(), "channel"))
				{	for(XMLTreeNode * itemNode = node->GetChild(); itemNode; itemNode = itemNode->GetNext())
					{	if(!strcmp(itemNode->GetType(), "item"))
						{	for(XMLTreeNode * i = itemNode->GetChild(); i; i = i->GetNext())
							{	if(!strcmp(i->GetType(), "title"))
								{	thisItem.title = i->GetData();
									replace_sonderzeichen((char *)thisItem.title.c_str());
									thisItem.title.removeChars('\n');
									thisItem.title.removeChars('\r');
								}
								if(!strcmp(i->GetType(), "description"))
								{	eString desc = i->GetData();
									replace_sonderzeichen((char *)desc.c_str());
									desc.removeChars('\n');
									desc.removeChars('\r');
									desc.strReplace("\t", " ");
									desc = removeTags(desc);
									desc = removeTrailingSpaces(desc);
									thisItem.description = desc;
								}
							}
							thisItem.id = idcnt++;
							save(thisItem);
						}
					}
				}
			}
		}
		else if(!strcmp(root->GetType(), "rdf:RDF"))
		{	NewsItem thisItem;
			int idcnt = 0;

			for(XMLTreeNode * node = root->GetChild(); node; node = node->GetNext())
			{	if(!strcmp(node->GetType(), "item"))
				{	for(XMLTreeNode * i = node->GetChild(); i; i = i->GetNext())
					{	if(!strcmp(i->GetType(), "title"))
						{	thisItem.title = i->GetData();
							replace_sonderzeichen((char *)thisItem.title.c_str());
							thisItem.title.removeChars('\n');
							thisItem.title.removeChars('\r');
						}
						if(!strcmp(i->GetType(), "description"))
						{	eString desc = i->GetData();
							replace_sonderzeichen((char *)desc.c_str());
							desc.removeChars('\n');
							desc.removeChars('\r');
							desc.strReplace("\t", " ");
							desc = removeTags(desc);
							desc = removeTrailingSpaces(desc);
							thisItem.description = desc;
						}
					}
					thisItem.id = idcnt++;
					save(thisItem);
				}
			}
		}
		else
		{	eString sMsg = eString().sprintf("Unknown file format (%s)", root->GetType());
			eMessageBox msg(sMsg, _("Error"), eMessageBox::iconWarning|eMessageBox::btOK);
			msg.show();     msg.exec();     msg.hide();
		}

		delete parser;
	}
	else
	{	eMessageBox msg(_("RSS file not found"), _("Error"), eMessageBox::iconWarning|eMessageBox::btOK);
		msg.show();     msg.exec();     msg.hide();
	}
}
