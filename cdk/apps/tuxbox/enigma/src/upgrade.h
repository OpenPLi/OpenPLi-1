#ifndef __src_upgrade_h
#define __src_upgrade_h

#include <lib/gui/ewindow.h>
#include <lib/system/httpd.h>
#include <lib/gui/listbox.h>

class XMLTreeParser;
class eLabel;
class eProgress;
class eButton;

class eListBoxEntryImage: public eListBoxEntryText
{
	friend class eListBox<eListBoxEntryImage>;
public:
	eString name, target, url, version, creator;
	unsigned char md5[16];
	eListBoxEntryImage(eListBox<eListBoxEntryImage> *listbox, eString name, eString target, eString url, eString version, eString creator, const unsigned char *md5);
};

class eHTTPDownload: public eHTTPDataSource
{
	int received;
	int total;
	int fd;
	eString filename;
	void init_eHTTPDownload(eHTTPConnection *c,const char *filename);
public:
	eHTTPDownload(eHTTPConnection *c, const char *filename);
	Signal2<void,int,int> progress; // received, total (-1 for unknown)
	~eHTTPDownload();
	void haveData(void *data, int len);
};

class eHTTPDownloadXML: public eHTTPDataSource
{
	XMLTreeParser &parser;
public:
	int error;
	eString errorstring;
	eHTTPDownloadXML(eHTTPConnection *c, XMLTreeParser &parser);
	void haveData(void *data, int len);
};

class eUpgrade: public eWindow
{
	struct changelogEntry
	{
		eString date;
		eString machines;
		int priority;
		eString text;
	};
	eString mIDStr;
	std::list<changelogEntry> changelogentries;
	eHTTPConnection *http, *changelog;
	int lasttime;
	unsigned char expected_md5[16];
	eString current_url;
	eString ourversion, selectedversion;
	void catalogTransferDone(int err);
	void changelogTransferDone(int err);
	void imageTransferDone(int err);
	eHTTPDataSource *createCatalogDataSink(eHTTPConnection *conn);
	eHTTPDataSource *createImageDataSink(eHTTPConnection *conn);
	eHTTPDataSource *createChangelogDataSink(eHTTPConnection *conn);
	XMLTreeParser *catalog;
	eHTTPDownloadXML *datacatalog;
	eHTTPDownload *image, *changelogdownload;
	
	void imageSelected(eListBoxEntryImage *image);
	void imageSelchanged(eListBoxEntryImage *image);

	eListBox<eListBoxEntryImage> *images;

	eStatusBar *status;
	eProgress *progress;
	eLabel *progresstext;
	eLabel *imagehelp;
	eButton *abort;
	eLabel *changes;
	
	void loadCatalog(const char *url);
	void loadChangelog(const char *url);
	void loadImage(const char *url);
	
	void setStatus(const eString &string);
	void setError(int error);
	
	void abortDownload();
	
	void downloadProgress(int received, int total);

	void flashImage(int checkmd5);
	void displayChangelog(eString oldversion, eString newversion, eString mid);
	void init_eUpgrade(bool manual);
public:
	eUpgrade(bool manual);
	~eUpgrade();
};

#endif
