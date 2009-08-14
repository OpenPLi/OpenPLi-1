/*
 * $Id: request.cpp,v 1.4 2009/03/11 20:42:11 rhabarber1848 Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
  * based on nhttpd (C) 2001/2002 Dirk Szymanski
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
#include <cstdarg>
#include <cstdio>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>
#include "debug.h"
#include "helper.h"
#include <chttpd/chttpdconfig.h>

#define OUTBUFSIZE 10240
#define IADDR_LOCAL "127.0.0.1"
#define UPLOAD_TMP_FILE "/tmp/upload.tmp"

extern chttpdConfig cfg;

CWebserverRequest::CWebserverRequest(CWebserver *server)
{
	Parent = server;

	Method = M_UNKNOWN;
	Host = "";
	URL = "";
	Filename = "";
	FileExt = "";
	Path = "";
	Param_String = "";
	HttpStatus = 0;
	RequestCanceled = false;

	outbuf = new char[OUTBUFSIZE + 1];
}

CWebserverRequest::~CWebserverRequest(void)
{
	if (outbuf)
		delete[] outbuf;

	EndRequest();
}

// check if authentication is required
bool CWebserverRequest::Authenticate(void)
{
	if (!cfg.MustAuthenticate)
		return true;

	if (CheckAuth())
		return true;

	SocketWriteLn("HTTP/1.0 401 Unauthorized");
	SocketWriteLn("WWW-Authenticate: Basic realm=\"dbox\"\r\n");

	if (Method != M_HEAD)
		SocketWriteLn("Access denied.");

	return false;
}

// check if given username an pssword are valid
bool CWebserverRequest::CheckAuth()
{
	if (HeaderList["Authorization"] == "")
		return false;

	std::string encodet = HeaderList["Authorization"].substr(6,HeaderList["Authorization"].length() - 6);
	std::string decodet = b64decode((char *)encodet.c_str());
	int pos = decodet.find_first_of(':');
	std::string user = decodet.substr(0,pos);
	std::string passwd = decodet.substr(pos + 1, decodet.length() - pos - 1);

	return (user.compare(cfg.AuthUser) == 0 &&
	        passwd.compare(cfg.AuthPassword) == 0);
}

bool CWebserverRequest::GetRawRequest()
{
#define bufferlen 16384
	char *buffer = new char[bufferlen+1];

	if ((rawbuffer_len = read(Socket,buffer,bufferlen)) < 1)
	{
		delete[] buffer;
		return false;
	}

	rawbuffer = std::string(buffer, rawbuffer_len);

	delete[] buffer;
	return true;
}

void CWebserverRequest::SplitParameter(char *param_copy)
{
        if (param_copy == NULL)
		return;

	char *p = strchr(param_copy, '=');

	if (p == NULL) {
		char number_buf[20];
 		sprintf(number_buf, "%d", ParameterList.size()+1);
		ParameterList[number_buf] = param_copy;
		return;
        }

	*p = '\0';
	if (ParameterList[param_copy].empty())
		ParameterList[param_copy] = p + 1;

	else
	{
		std::string key = param_copy;
		*p  = ',';
		ParameterList[key] += p;
	}
}

bool CWebserverRequest::ParseParams(std::string param_string)			// parse parameter string
{
	if(param_string.length() <= 0)
		return false;
	char *param_copy = new char[param_string.length()+1];
	memcpy(param_copy, param_string.c_str(), param_string.length()+1);

	// instead of copying and allocating strings again and again,
	// we just copy once, move pointers
	// around and set '\0' chars. is faster and less memory is used.
        char *param; // this is the param
	char *ptr1;  // points to the begin of param
        char *ptr2;  // points to the '&' sign

        ptr1 = param_copy;
        ptr2 = ptr1;
	while(ptr2 && *ptr1) {
                param  = ptr1;
		ptr2 = strchr(ptr1, '&');
		if(ptr2 != NULL)
		{
			*ptr2 = '\0';
			ptr1  = ptr2+1;
		}
		SplitParameter(param);
	}
	delete[] param_copy;
	return true;
};

bool CWebserverRequest::ParseFirstLine(std::string zeile)	// parse first line of request
{
	int ende, anfang, t;

	anfang = zeile.find(' ');				// GET /images/elist.gif HTTP/1.1
	ende = zeile.rfind(' ');				// nach leerzeichen splitten

	if (anfang > 0 && ende > 0 && anfang != ende)
	{
		std::string method,url,http;
		method= zeile.substr(0,anfang);
		url = zeile.substr(anfang+1,ende - (anfang+1));
		http = zeile.substr(ende+1,zeile.length() - ende+1);
//		dprintf("m: '%s' u: '%s' h:'%s'\n",method.c_str(),url.c_str(),http.c_str());

		if(method.compare("POST") == 0)
			Method = M_POST;
		else if(method.compare("GET") == 0)
			Method = M_GET;
		else if(method.compare("PUT") == 0)
			Method = M_PUT;
		else if(method.compare("HEAD") == 0)
			Method = M_HEAD;
		else
		{
			aprintf("Unknown method or invalid request");
			dprintf("Request: '%s'\n",rawbuffer.c_str());
			return false;
		}

		if((t = url.find('?')) > 0)			// eventuellen Parameter inner URL finden
		{
			URL = url.substr(0,t);
			Param_String = url.substr(t+1,url.length() - (t+1));
			URLDecode(Param_String);

			return ParseParams(Param_String);
		}
		else
			URL = url;
	}
	return true;
}

bool CWebserverRequest::ParseHeader(std::string header)		// parse the header of the request
{
	bool ende = false;
	int pos;
	std::string sheader;

	while(!ende)
	{
		if((pos = header.find_first_of("\n")) > 0)
		{
			sheader = header.substr(0,pos-1);
			header = header.substr(pos+1,header.length() - (pos+1));
		}
		else
		{
			sheader = header;
			ende = true;
		}

		if((pos = sheader.find_first_of(':')) > 0)
		{
			HeaderList[sheader.substr(0,pos)] = sheader.substr(pos+2,sheader.length() - pos - 2);
//			dprintf("%s: %s\n",sheader.substr(0,pos).c_str(),HeaderList[sheader.substr(0,pos)].c_str());
		}
	}

	return true;
}

bool CWebserverRequest::ParseBoundaries(std::string bounds)	// parse boundaries of post method
{
	aprintf("formdata: '%s'\n",bounds.c_str());
	int i=0;
	char * e_ende;
	char * anfang = (char *) bounds.c_str();
	char * ende = (char *) bounds.c_str() + bounds.length();
	do
	{
		anfang = strstr(anfang,Boundary.c_str());
//		dprintf("anfang: %s\n",anfang);
		if(anfang != 0)
		{
			e_ende = strstr(anfang +1,Boundary.c_str());
			if(e_ende == 0)
				e_ende = ende - 4;
//			dprintf("ende: %s\n",e_ende);
			boundaries[i] = std::string(anfang + Boundary.length() +2,e_ende - (anfang + Boundary.length()+2) -2);
			aprintf("boundary[%d]='%s'\n",i,boundaries[i].c_str());
			anfang = e_ende;
			i++;
		}
	}while(anfang > 0 && anfang < (ende - 2));

	return true;
}

bool CWebserverRequest::ParseRequest()
{
	int ende;

	if(rawbuffer_len > 0 )
	{
		if((ende = rawbuffer.find_first_of('\n')) == 0)
		{
			aprintf("ParseRequest: End of line not found\n");
			Send500Error();
			return false;
		}
		std::string zeile1 = rawbuffer.substr(0,ende-1);

		if(ParseFirstLine(zeile1))
		{
			unsigned int i;
			for(i = 0; ((rawbuffer[i] != '\n') || (rawbuffer[i+2] != '\n')) && (i < rawbuffer.length());i++);
			int headerende = i;
//			dprintf("headerende: %d buffer_len: %d\n",headerende,rawbuffer_len);
			if(headerende == 0)
			{
				aprintf("ParseRequest: no headers found\n");
				Send500Error();
				return false;
			}
			std::string header = rawbuffer.substr(ende+1,headerende - ende - 2);
			ParseHeader(header);
			Host = HeaderList["Host"];
			if(Method == M_POST) // TODO: Und testen ob content = formdata
			{
				std::string t = "multipart/form-data; boundary=";
				if(HeaderList["Content-Type"].compare(0,t.length(),t) == 0)
				{
//					SocketWriteLn("Sorry, momentan broken\n");
					Boundary = "--" + HeaderList["Content-Type"].substr(t.length(),HeaderList["Content-Type"].length() - t.length());
					aprintf("Boundary: '%s'\n",Boundary.c_str());
//					if((headerende + 3) < rawbuffer_len)
//						ParseBoundaries(rawbuffer.substr(headerende + 3,rawbuffer_len - (headerende + 3)));
					HandleUpload();
				}
				else if(HeaderList["Content-Type"].compare("application/x-www-form-urlencoded") == 0)
				{
//					dprintf("Form Daten in Parameter String\n");
					if((headerende + 3) < rawbuffer_len)
					{
						std::string params = rawbuffer.substr(headerende + 3,rawbuffer_len - (headerende + 3));
						if(params[params.length()-1] == '\n')
							params.substr(0,params.length() -2);
						ParseParams(params);
					}
				}

				dprintf("Method Post !\n");
			}


			return true;
		}
		else {
			SocketWrite("HTTP/1.0 501 Not implemented\r\n");
			SocketWrite("Content-Type: text/plain\r\n\r\n");
			SocketWrite("501 : Request-Method not implemented.\n");
			HttpStatus = 501;
			aprintf("501 : Request-Method not implemented.\n");
			return false;
		}
	}
	else
	{
		aprintf("rawbuffer_len = %ld\n",rawbuffer_len);
		return false;
	}
}

bool CWebserverRequest::HandleUpload()	
{
	int t,y = 0;

	SocketWrite("HTTP/1.1 100 Continue \r\n\r\n");		// Erstmal weitere Daten anfordern

	if(HeaderList["Content-Length"] != "")
	{
		remove(UPLOAD_TMP_FILE);
		// Get Multipart Upload
		//--------------------------------
		aprintf("Contenlaenge gefunden\n");
		long contentsize = atol(HeaderList["Content-Length"].c_str());
		aprintf("Contenlaenge :%ld\n",contentsize);
		char *buffer2 =(char *) malloc(contentsize);
		if(!buffer2)
		{
			aprintf("Kein Speicher für upload\n");
			return false;
		}
		long long gelesen = 0;
		aprintf("Buffer ok Groesse:%ld\n",contentsize);
		fcntl(Socket, F_SETFL, fcntl(Socket,F_GETFL)|O_NONBLOCK); //Non blocking
		while(gelesen < contentsize)
		{
			if(y > 500)
			{
				aprintf("y read Abbruch\n");
				break;
			}
			aprintf("vor %lld noch %lld",gelesen,contentsize-gelesen);
			t = read(Socket,&buffer2[gelesen],contentsize-gelesen);//Test -1
			aprintf("nach read t%d\n",t);
			if(t!=-1)//EAGAIN =-1
			{
				if(t <= 0)
				{
					aprintf("nix mehr\n");
					break;
				}
				gelesen += t;
				y=0; 
			}
			else
				y++;
		}
		aprintf("fertig\n");
		if(gelesen == contentsize)
		{
			// Extract Upload File
			//--------------------------------
			std::string marker;
			std::string buff;
			buff = std::string(buffer2, gelesen);
			free(buffer2);

			int pos = 0;
			if((pos = buff.find("\r\n")) > 0)	// find marker
			{
				marker = buff.substr(0,pos-1);
				buff = buff.substr(pos+1,buff.length() - (pos+1)); // snip
			}		
			if((pos = buff.find("Content-Type")) > 0) // Multipart File
			{
				buff = buff.substr(pos+1,buff.length() - (pos+1)); // snip
			}		
			if((pos = buff.find("\r\n\r\n")) > 0) // Multipart File - Start offset
			{
				buff = buff.substr(pos+4,buff.length() - (pos+4)); // snip
			}		
			if((pos = buff.find(marker)) > 0)// find marker after file
			{
				buff= buff.substr(0,pos-2); // snip "\r\n"+marker
			}		
			aprintf("write");
			
			// Write Upload Extract file
			//--------------------------------
			FILE *out = fopen(UPLOAD_TMP_FILE,"w"); // save tmp & mem - open here => File=0 tmp=Socket-space
			if(out != NULL)
			{
				fwrite(buff.c_str(),buff.length(),1,out);
				fclose(out);
			}
			else
				aprintf("nicht geschrieben\n");
		}
		if(gelesen == contentsize)
		{
			aprintf("Upload komplett gelesen: %ld bytes\n",contentsize);
			return true;
		}
		else
		{
			aprintf("Upload konnte nicht komplett gelesen werden  %ld bytes\n",contentsize);
			return false;
		}
	}
	else
	{
		aprintf("Content-Length ist nicht in der HeaderListe\n");
		return false;
	}
}

void CWebserverRequest::PrintRequest(void)					// for debugging and verbose output
{
	CDEBUG::getInstance()->LogRequest(this);
}

void CWebserverRequest::SendHTMLHeader(std::string Titel)
{
	SocketWriteLn("<html>\n<head><title>" + Titel + "</title><link rel=\"stylesheet\" type=\"text/css\" href=\"../global.css\">");
	SocketWriteLn("<meta http-equiv=\"cache-control\" content=\"no-cache\">");
	SocketWriteLn("<meta http-equiv=\"expires\" content=\"0\"></head>\n<body>");
}

void CWebserverRequest::SendHTMLFooter(void)
{
	SocketWriteLn("</body></html>");
}

void CWebserverRequest::Send302(char const *URI)
{
	printf("HTTP/1.0 302 Moved Permanently\r\nLocation: %s\r\nContent-Type: text/html\r\n\r\n",URI);
	if (Method != M_HEAD) {
		SocketWrite("<html><head><title>Object moved</title></head><body>");
		printf("302 : Object moved.<brk>If you dont get redirected click <a href=\"%s\">here</a></body></html>\n",URI);
	}
	HttpStatus = 302;
}

void CWebserverRequest::Send404Error(void)
{
	SocketWrite("HTTP/1.0 404 Not Found\r\n");		//404 - file not found
	SocketWrite("Content-Type: text/plain\r\n\r\n");
	if (Method != M_HEAD) {
		SocketWrite("404 : File not found\n\nThe requested file was not found on this dbox ;)\n");
	}
	HttpStatus = 404;
}

void CWebserverRequest::Send500Error(void)
{
	SocketWrite("HTTP/1.0 500 InternalError\r\n");		//500 - internal error
	SocketWrite("Content-Type: text/plain\r\n\r\n");
	if (Method != M_HEAD) {
		SocketWrite("500 : InternalError\n\nPerhaps some parameters missing ? ;)");
	}
	HttpStatus = 500;
}

void CWebserverRequest::SendPlainHeader(std::string contenttype)
{
	SocketWrite("HTTP/1.0 200 OK\r\nContent-Type: " + contenttype + "\r\n\r\n");
	HttpStatus = 200;
}

void CWebserverRequest::RewriteURL()
{

	if(( URL.length() == 1) && (URL[URL.length()-1] == '/' ))		// Wenn letztes Zeichen ein / ist dann index.html anhängen
	{
		Path = URL;
		Filename = "index.html";
	}
	else
	{		// Sonst aufsplitten
		unsigned int split = URL.rfind('/') + 1;

		if(split > 0)
			Path = URL.substr(0,split);
		else
			Path = "/";


		if(split < URL.length())
			Filename= URL.substr(split,URL.length()- split);
		else
			dprintf("Kein Dateiname !\n");
	}

	if(Filename.find('%') > 0)	// Wenn Sonderzeichen im Dateinamen sind
	{
		char filename[255]={0};
		char * str = (char *) Filename.c_str();
		for (unsigned int i = 0,n = 0; i < strlen(str) ;i++ )
		{
			if(str[i] == '%')
			{
				switch (*((short *)(&str[i+1])))
				{
					case 0x3230 : filename[n++] = ' '; break;
					default: filename[n++] = ' '; break;

				}
				i += 2;
			}
			else
				filename[n++] = str[i];
		}
		Filename = filename;
	}

	FileExt = "";
	if(Filename.length() > 0)
	{
		int fileext = Filename.rfind('.');

		if(fileext > 0)		// Dateiendung
		{
			FileExt = Filename.substr(fileext+1,Filename.length()-(fileext+1));
		}
	}
}

bool CWebserverRequest::SendResponse()
{
	RewriteURL();		// Erst mal die URL umschreiben

	if( Client_Addr.find(IADDR_LOCAL)>0 ) // != local
	{
		if(!Authenticate()) // Jeder Aufruf muss geprueft werden
        		return false;
	}

	return SendFile(Path,Filename);
}

bool CWebserverRequest::EndRequest()
{
	if(Socket)
	{
		close(Socket);
		RequestCanceled = true;
		Socket = 0;
	}
	return true;
}

void CWebserverRequest::SendOk()
{
	SocketWrite("ok");
}

void CWebserverRequest::SendError()
{
	SocketWrite("error");
}

void CWebserverRequest::printf ( const char *fmt, ... )
{
	memset(outbuf, 0, OUTBUFSIZE);
	va_list arglist;
	va_start( arglist, fmt );
	vsnprintf( outbuf,OUTBUFSIZE, fmt, arglist );
	va_end(arglist);
	SocketWriteData(outbuf,strlen(outbuf));
}

bool CWebserverRequest::SocketWrite(char const *text)
{
	return SocketWriteData(text, strlen(text));
}

bool CWebserverRequest::SocketWriteLn(char const *text)
{
	if(!SocketWriteData(text, strlen(text)))
		return false;
	return SocketWriteData("\r\n",2);
}

//-------------------------------------------------------------------------

bool CWebserverRequest::SocketWriteData( char const * data, long length )
{
	if(RequestCanceled)
		return false;
	if((send(Socket, data, length, MSG_NOSIGNAL) == -1) )
	{
		perror("request canceled\n");
		RequestCanceled = true;
		return false;
	}
	return true;
}

std::string CWebserverRequest::GetContentType(std::string ext)
{
	std::string ctype;
		// Anhand der Dateiendung den Content bestimmen
	if(  (ext.compare("html") == 0) || (ext.compare("htm") == 0) )
		ctype = "text/html";
	else if(ext.compare("gif") == 0)
		ctype = "image/gif";
	else if((ext.compare("png") == 0) || (ext.compare("PNG") == 0) )
		ctype = "image/png";
	else if( (ext.compare("jpg") == 0) || (ext.compare("JPG") == 0) )
		ctype = "image/jpeg";
	else if( (ext.compare("css") == 0) || (ext.compare("CSS") == 0) )
		ctype = "text/css";
	else if(ext.compare("xml") == 0)
		ctype = "text/xml";
	else
		ctype = "text/plain";
	return ctype;
}

bool CWebserverRequest::SendFile(const std::string path,const std::string filename)
{
	if( (tmpint = OpenFile(path, filename) ) != -1 )
	{							// Wenn Datei geöffnet werden konnte
		if (!SocketWrite("HTTP/1.0 200 OK\r\n"))
		{
			close(tmpint);
			return false;
		}
		HttpStatus = 200;

		if (!SocketWrite("Content-Type: " + GetContentType(FileExt) + "\r\n\r\n"))
		{
			close(tmpint);
			return false;
		}
		if (Method == M_HEAD) {
			close(tmpint);
			return true;
		}
		off_t start = 0;
		off_t end = lseek(tmpint,0,SEEK_END);
		int written = 0;
		if((written = sendfile(Socket,tmpint,&start,end)) == -1)
			perror("sendfile failed");
		close(tmpint);
		return true;
	}
	else
	{
		Send404Error();
		return false;
	}
}

std::string CWebserverRequest::GetFileName(std::string path, std::string filename)
{
	std::string tmpfilename;
	if(path[path.length()-1] != '/')
		tmpfilename = path + "/" + filename;
	else
		tmpfilename = path + filename;

	if( access(std::string(cfg.PublicDocumentRoot + tmpfilename).c_str(),4) == 0)
			tmpfilename = cfg.PublicDocumentRoot + tmpfilename;
	else if(access(std::string(cfg.PrivateDocumentRoot + tmpfilename).c_str(),4) == 0)
			tmpfilename = cfg.PrivateDocumentRoot + tmpfilename;
	else if(access(tmpfilename.c_str(),4) == 0)
			;
	else
	{
		return "";
	}
	return tmpfilename;
}

int CWebserverRequest::OpenFile(std::string path, std::string filename)
{
	struct stat statbuf;
	int  fd= -1;

	tmpstring = GetFileName(path, filename);

	if(tmpstring.length() > 0)
	{
		fd = open( tmpstring.c_str(), O_RDONLY );
		if (fd<=0)
		{
			aprintf("cannot open file %s: ", filename.c_str());
			dperror("");
		}
		fstat(fd,&statbuf);
		if (!S_ISREG(statbuf.st_mode)) {
			close(fd);
			fd = -1;
		}
	}
	return fd;
}

bool CWebserverRequest::ParseFile(const std::string filename,CStringList &params)
{
	char *file_buffer, *out_buffer;
	long file_length= 0,out_buffer_size = 0;
	int out_len = 0;
	FILE * fd;

	tmpstring = GetFileName("/",filename);
	if(tmpstring.length() > 0)
	{
		if((fd = fopen(tmpstring.c_str(),"r")) == NULL)
		{
			perror("Parse file open error");
			return false;
		}

		// get filesize
		fseek(fd, 0, SEEK_END);
		file_length = ftell(fd);
		rewind(fd);

		file_buffer = new char[file_length];		// allocate buffer for file

		out_buffer_size = file_length + 2048;
		out_buffer = new char[out_buffer_size];		// allocate output buffer

		fread(file_buffer, file_length, 1, fd);		// read file

		if((out_len = ParseBuffer(file_buffer, file_length, out_buffer, out_buffer_size, params)) > 0)
			SocketWriteData(out_buffer,out_len);

		fclose(fd);

		delete[] out_buffer;
		delete[] file_buffer;
	}
	return true;
}

long CWebserverRequest::ParseBuffer(char *file_buffer, long file_length, char *out_buffer, long out_buffer_size, CStringList &params)
{
	long pos = 0, outpos = 0, endpos = 0;
	std::string parameter = "";

	while(pos < file_length && outpos < out_buffer_size)
	{
		if(file_buffer[pos] == '%' && file_buffer[pos+1] == '%')	// begin of parameter
		{
			endpos = pos + 2;		// skip start seperators
			parameter = "";
			while((endpos < (file_length -2)) && !((file_buffer[endpos] == '%') && (file_buffer[endpos+1] == '%')))
			{	// search for end of parameter
				parameter += file_buffer[endpos++];
			}
			if(params[parameter].length() > 0)
			{	// if parameter found in param array
				strcpy(&out_buffer[outpos],params[parameter].c_str());
				outpos += params[parameter].length();
			}
			else
			{
				strcpy(&out_buffer[outpos],"%%NOT_FOUND%%");
				outpos += 13;
			}
			pos = endpos + 2;	// skip end seperators
		}
		else
			out_buffer[outpos++] = file_buffer[pos++];
	}
	out_buffer[outpos] = 0;

	return outpos;
}

// Decode URLEncoded std::string
void CWebserverRequest::URLDecode(std::string &encodedString)
{
	char *newString=NULL;
	const char *string = encodedString.c_str();
	int count=0;
	char hex[3]={'\0'};
	unsigned long iStr;

	count = 0;
	if((newString = (char *)malloc(sizeof(char) * strlen(string) + 1) ) != NULL)
	{

	/* copy the new sring with the values decoded */
		while(string[count]) /* use the null character as a loop terminator */
		{
			if (string[count] == '%')
			{
				hex[0]=string[count+1];
				hex[1]=string[count+2];
				hex[2]='\0';
				iStr = strtoul(hex,NULL,16); /* convert to Hex char */
				newString[count]=(char)iStr;
				count++;
				string = string + 2; /* need to reset the pointer so that we don't write hex out */
			}
			else
			{
				if (string[count] == '+')
					newString[count] = ' ';
				else
					newString[count] = string[count];
				count++;
			}
		} /* end of while loop */

		newString[count]='\0'; /* when done copying the string,need to terminate w/ null char */
	}
	else
	{
		return;
	}
	encodedString = newString;
	free(newString);
}
#endif

