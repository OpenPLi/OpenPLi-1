#include <lib/system/http_file.h>
#include <lib/base/estring.h>
#include <src/enigma_dyn_utils.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <shadow.h>
#include <pwd.h>

eHTTPFile::eHTTPFile(eHTTPConnection *c, int _fd, int method, const char *mime): eHTTPDataSource(c), method(method)
{
	init_eHTTPFile(c,_fd,mime);
}
void eHTTPFile::init_eHTTPFile(eHTTPConnection *c, int _fd, const char *mime)
{
	fd=_fd;
	if (method == methodGET)
	{
		c->local_header["Content-Type"]=eString(mime);
		size=lseek64(fd, 0, SEEK_END);
		char asize[32];
		snprintf(asize, 32, "%lld", size);
		lseek(fd, 0, SEEK_SET);
		c->local_header["Content-Length"]=asize;
	}
	connection->code_descr="OK";
	connection->code=200;
}

int eHTTPFile::doWrite(int bytes)
{
	if (method == methodGET)
	{
		char buff[bytes];
		if (!size)
			return -1;
		int len=bytes;
		if (len>size)
			len=size;
		len=read(fd, buff, len);
		if (len<=0)
			return -1;
		size-=connection->writeBlock(buff, len);
		if (!size)
			return -1;
		else
			return 1;
	} else if (method == methodPUT)
		return 1;
	else
		return -1;
}

void eHTTPFile::haveData(void *data, int len)
{
	if (method != methodPUT)
		return;
	::write(fd, data, len);
}

eHTTPFile::~eHTTPFile()
{
	close(fd);
}

eHTTPMovie::eHTTPMovie(eHTTPConnection *c, int _fd, int method, const char *mime, const eString &_filename )
	:eHTTPDataSource(c), fd(_fd), slice(0), size(0), filename(_filename), method(method)
{
	init_eHTTPMovie(c,mime);
}
void eHTTPMovie::init_eHTTPMovie(eHTTPConnection *c,const char *mime)
{
	if (method == methodGET)
	{
		int slice=0;
		struct stat64 s;
		while (!stat64((filename + (slice ? eString().sprintf(".%03d", slice) : eString(""))).c_str(), &s))
		{
			size+=s.st_size;
			++slice;
		}
		c->local_header["Content-Type"]=eString(mime);
		char asize[32];
		snprintf(asize, 32, "%lld", size);
		c->local_header["Content-Length"]=asize;
	}
	connection->code_descr="OK";
	connection->code=200;
}

int eHTTPMovie::doWrite(int bytes)
{
	if (method == methodGET)
	{
		char buff[bytes];
		if (!size)
			return -1;
reread:
		int len=bytes;
		if (len>size)
			len=size;
		len=read(fd, buff, len);
		if (len<=0)  // file end..
		{
			if ( size )
			{
				close(fd);
				++slice;
				fd=open((filename+eString().sprintf(".%03d", slice)).c_str(), O_RDONLY|O_LARGEFILE, 0644);
				if ( fd < 0 )
				{
					eDebug("file not exist.. but %d bytes left.. abort transfer!!", size);
					return -1;
				}
				goto reread;
			}
			else
				return -1;
		}
		size-=connection->writeBlock(buff, len);
		if (!size)
			return -1;
		else
			return 1;
	} else if (method == methodPUT)
		return 1;
	else
		return -1;
}

void eHTTPMovie::haveData(void *data, int len)
{
	if (method != methodPUT)
		return;
	::write(fd, data, len);
}

eHTTPMovie::~eHTTPMovie()
{
	close(fd);
}


eHTTPFilePathResolver::eHTTPFilePathResolver()
{
	translate.setAutoDelete(true);
}


static char _base64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

static int unbase64(eString &dst, const eString string)
{
	dst="";
	char c[4];
	int pos=0;
	unsigned int i=0;

	while (1)
	{
		if (i == string.size())
			break;
		char *ch=strchr(_base64, string[i++]);
		if (!ch)
		{
			i++;
			continue;
		}
		c[pos++]=ch-_base64;
		if (pos == 4)
		{
			char d[3];
			d[0]=c[0]<<2;
			d[0]|=c[1]>>4;
			d[1]=c[1]<<4;
			d[1]|=c[2]>>2;
			d[2]=c[2]<<6;
			d[2]|=c[3];

			dst+=d[0];
			if (c[2] != 64)
				dst+=d[1];
			if (c[3] != 64)
				dst+=d[2];
			pos=0;
		}
	}
	return pos;
}

int CheckUnixPassword(const char *user, const char *pass)
{
	passwd *pwd=getpwnam(user);
	if (!pwd)
		return -1;
	char *cpwd=pwd->pw_passwd;
	if (pwd && (!strcmp(pwd->pw_passwd, "x")))
	{
		spwd *sp=getspnam(user);
		if (!sp)						// no shadow password defined.
			return -1;
		cpwd=sp->sp_pwdp;
	}
	if (!cpwd)
		return -1;
	if ((*cpwd=='!')||(*cpwd=='*'))		 // disabled user
		return -2;
	char *cres=crypt(pass, cpwd);
	return !!strcmp(cres, cpwd);
}

int checkAuth(const eString cauth)
{
	eString auth;
	if (cauth.left(6) != "Basic ")
		return -1;
	if (unbase64(auth, cauth.mid(6)))
		return -1;
	eString username=auth.left(auth.find(":"));
	eString password=auth.mid(auth.find(":")+1);
	if (CheckUnixPassword(username.c_str(), password.c_str()))
		return -1;
	return 0;
}

eHTTPDataSource *eHTTPFilePathResolver::getDataSource(eString request, eString path, eHTTPConnection *conn)
{
	int method;
#ifdef DEBUG_HTTP_FILE
	eDebug("request = %s, path = %s", request.c_str(), path.c_str());
#endif
	if (request == "GET")
		method=eHTTPFile::methodGET;
	else if (request == "PUT")
		method=eHTTPFile::methodPUT;
	else
		return new eHTTPError(conn, 405); // method not allowed
	if (httpUnescape(path).find("../")!=eString::npos)		// evil hax0r
		return new eHTTPError(conn, 403);
	if (path[0] != '/')		// prepend '/'
		path.insert(0,"/");
	if (path[path.length()-1]=='/')
		path+="index.html";

	bool isMovieDownload=false;
	eHTTPDataSource *data=0;

	if ( path.left(6) == "/rootX" )  // movie download
	{
		isMovieDownload=true;
		path.erase(5,1);  // remove X
	}

	for (ePtrList<eHTTPFilePath>::iterator i(translate); i != translate.end(); ++i)
	{
		if (i->root==path.left(i->root.length()))
		{
			eString newpath=i->path+path.mid(i->root.length());
			if (newpath.find('?'))
				newpath=newpath.left(newpath.find('?'));
			newpath = httpUnescape(newpath);
			//eDebug("translated %s to %s", path.c_str(), newpath.c_str());
			if (i->authorized & ((method==eHTTPFile::methodGET)?1:2))
			{
				std::map<eString, eString>::iterator i=conn->remote_header.find("Authorization");
				if ((i == conn->remote_header.end()) || checkAuth(i->second))
				{
					conn->local_header["WWW-Authenticate"]="Basic realm=\"dreambox\"";
					return new eHTTPError(conn, 401); // auth req'ed
				}
			}
			
			if (strstr(newpath.c_str(), (TEMPLATE_DIR).c_str()) != 0)
			{
				char *pch = strrchr(newpath.c_str(), '/');
				eString newpath2 = TEMPLATE_DIR2 + eString(strdup(pch + 1));
				if (access(newpath2.c_str(), R_OK) == 0)
					newpath = newpath2;
			}
			
			if (strstr(newpath.c_str(), (HTDOCS_DIR).c_str()) != 0)
			{
				char *pch = strrchr(newpath.c_str(), '/');
				eString newpath2 = HTDOCS_DIR2 + eString(strdup(pch + 1));
				if (access(newpath2.c_str(), R_OK) == 0)
					newpath = newpath2;
			}
			
			int fd=open(newpath.c_str(), (method==eHTTPFile::methodGET)?O_RDONLY|O_LARGEFILE:(O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE), 0644);
			if (fd==-1)
			{
				switch (errno)
				{
				case ENOENT:
					data=new eHTTPError(conn, 404);
					break;
				case EACCES:
					data=new eHTTPError(conn, 403);
					break;
				default:
					data=new eHTTPError(conn, 403); // k.a.
					break;
				}
				break;
			}

			eString ext=path.mid(path.rfind('.'));
			const char *mime="text/unknown";
			if ((ext==".html") || (ext==".htm"))
				mime="text/html";
			else if ((ext==".jpeg") || (ext==".jpg"))
				mime="image/jpeg";
			else if (ext==".gif")
				mime="image/gif";
			else if (ext==".css")
				mime="text/css";
			else if (ext==".png")
				mime="image/png";
			else if (ext==".xml")
				mime="text/xml";
			else if (ext==".xsl")
				mime="text/xsl";
			else if (ext.find(".ts") != eString::npos)
				mime="binary/ts";

			if ( isMovieDownload )
				data=new eHTTPMovie(conn, fd, method, mime, newpath);
			else
				data=new eHTTPFile(conn, fd, method, mime);
			break;
		}
	}
	return data;
}

void eHTTPFilePathResolver::addTranslation(eString path, eString root, int authorized)
{
	if (path[path.length()-1]!='/')
		path+='/';
	if (root[root.length()-1]!='/')
		root+='/';
	translate.push_back(new eHTTPFilePath(path, root, authorized));
}
