#ifndef __http_file_h
#define __http_file_h

#include "httpd.h"

class eHTTPFile: public eHTTPDataSource
{
	int fd;
	off64_t size;
	const char *mime;
	int method;
	void init_eHTTPFile(eHTTPConnection *c, int _fd, const char *mime);
public:
	enum { methodGET, methodPUT };
	eHTTPFile(eHTTPConnection *c, int fd, int method, const char *mime);
	~eHTTPFile();
	int doWrite(int);
	void haveData(void *data, int len);
};

class eHTTPMovie: public eHTTPDataSource
{
	int fd, slice;
	off64_t size;
	const char *mime;
	eString filename;
	int method;
	void init_eHTTPMovie(eHTTPConnection *c,const char *mime);
public:
	enum { methodGET, methodPUT };
	eHTTPMovie(eHTTPConnection *c, int fd, int method, const char *mime, const eString &filename);
	~eHTTPMovie();
	int doWrite(int);
	void haveData(void *data, int len);
};

class eHTTPFilePathResolver: public eHTTPPathResolver
{
	struct eHTTPFilePath
	{
		eString path;
		eString root;
		int authorized; // must be authorized (1 means read, 2 write)
		eHTTPFilePath(eString path, eString root, int authorized): path(path), root(root), authorized(authorized)
		{
		}
	};
	ePtrList<eHTTPFilePath> translate;
public:
	eHTTPFilePathResolver();
	eHTTPDataSource *getDataSource(eString request, eString path, eHTTPConnection *conn);
	void addTranslation(eString path, eString root, int auth);
};

#endif
