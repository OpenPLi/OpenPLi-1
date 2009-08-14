/*
	webserver  -   DBoxII-Project

	Copyright (C) 2001/2002 Dirk Szymanski 'Dirch'

	$Id: request.cpp,v 1.32 2002/10/15 20:39:47 woglinde Exp $

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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



*/


#include <arpa/inet.h> 
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "request.h"
#include "webdbox.h"
#include "debug.h"

#define OUTBUFSIZE 2048


//-------------------------------------------------------------------------
CWebserverRequest::CWebserverRequest(CWebserver *server) 
{
	Parent = server;

	Method = M_UNKNOWN; 
	Host = "";
	URL = "";
	Filename = "";
	FileExt = "";
	Path = "";
	Param_String="";
	ContentType = "";
	HttpStatus = 0;
	RequestCanceled = false;

	outbuf = new char[OUTBUFSIZE];

}

//-------------------------------------------------------------------------
CWebserverRequest::~CWebserverRequest() 
{
	if(outbuf)
		delete[] outbuf;

	EndRequest();
}

//-------------------------------------------------------------------------
bool CWebserverRequest::Authenticate()			// check if authentication is required
{
	if(Parent->MustAuthenticate)
	{
		if(!CheckAuth())
		{
//			dprintf("Authenticate\n");
			SocketWriteLn("HTTP/1.0 401 Unauthorized");
			SocketWriteLn("WWW-Authenticate: Basic realm=\"dbox\"\r\n");
			if (Method != M_HEAD) {
				SocketWriteLn("Access denied.");
			}
			return false;
		}
		else
		{
//			dprintf("Zugriff ok\n");
			return true;
		}
	}
	else
		return true;
}

//-------------------------------------------------------------------------
bool CWebserverRequest::CheckAuth()			// check if given username an pssword are valid
{
	if(HeaderList["Authorization"] == "")
		return false;
	string encodet = HeaderList["Authorization"].substr(6,HeaderList["Authorization"].length() - 6);
	string decodet = b64decode((char *)encodet.c_str());
	int pos = decodet.find_first_of(':');
	string user = decodet.substr(0,pos);
	string passwd = decodet.substr(pos + 1, decodet.length() - pos - 1);
//	dprintf("user: '%s' passwd: '%s'\n",user.c_str(),passwd.c_str());

	if(user.compare(Parent->AuthUser) == 0 && passwd.compare(Parent->AuthPassword) == 0)
	{
//		dprintf("passwort ok\n");
		return true;
	}
	else
	{
//		dprintf("passwort nicht ok\n");
		return false;
	}
}

//-------------------------------------------------------------------------
bool CWebserverRequest::GetRawRequest()
{
#define bufferlen 1024
	char *buffer;
	buffer = new char[bufferlen+1];
	if((rawbuffer_len = read(Socket,buffer,bufferlen)) != -1)
	{
		rawbuffer = string(buffer,rawbuffer_len);
		delete[] buffer;
		return true;
	}
	delete[] buffer;
	return false;
}
//-------------------------------------------------------------------------
void CWebserverRequest::SplitParameter(string param_str)
{
string nummer = "1";
	if(param_str.length() > 0)
	{
		int pos = param_str.find('=');
		if(pos != -1)
		{
			if (ParameterList[param_str.substr(0,pos)] == "")
				ParameterList[param_str.substr(0,pos)] = param_str.substr(pos+1,param_str.length() - (pos+1));
			else {
				ParameterList[param_str.substr(0,pos)] += ",";
				ParameterList[param_str.substr(0,pos)] += param_str.substr(pos+1,param_str.length() - (pos+1));
			}
		}
		else
		{
			ParameterList[nummer] = param_str;
			nummer[0]++;
		}
	}
}
//-------------------------------------------------------------------------

bool CWebserverRequest::ParseParams(string param_string)			// parse parameter string
{
string name,value,param;
string param_str;
int pos;
bool ende = false;

	if(param_string.length() <= 0)
		return false;

	param_str = param_string;
	while(!ende)
	{
		pos = param_str.find_first_of("&");
		if(pos > 0)
		{
			param = param_str.substr(0,pos);	
			param_str = param_str.substr(pos+1,param_str.length() - (pos+1));
		}
		else
		{
			param = param_str;
			ende = true;
		}
//		dprintf("param: '%s' param_str: '%s'\n",param.c_str(),param_str.c_str());
		SplitParameter(param);
	}
	return true;
};
//-------------------------------------------------------------------------


bool CWebserverRequest::ParseFirstLine(string zeile)				// parse first line of request
{
int ende, anfang, t;

	anfang = zeile.find(' ');				// GET /images/elist.gif HTTP/1.1 
	ende = zeile.rfind(' ');				// nach leerzeichen splitten

	if (anfang > 0 && ende > 0 && anfang != ende)
	{
		string method,url,http;
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


//-------------------------------------------------------------------------
bool CWebserverRequest::ParseHeader(string header)					// parse the header of the request
{
bool ende = false;
int pos;
string sheader;
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

//-------------------------------------------------------------------------
bool CWebserverRequest::ParseBoundaries(string bounds)			// parse boundaries of post method
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
			boundaries[i] = string(anfang + Boundary.length() +2,e_ende - (anfang + Boundary.length()+2) -2);
			aprintf("boundary[%d]='%s'\n",i,boundaries[i].c_str());
			anfang = e_ende;
			i++;
		}
	}while(anfang > 0 && anfang < (ende - 2));

	return true;
}

//-------------------------------------------------------------------------
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
		string zeile1 = rawbuffer.substr(0,ende-1);

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
			string header = rawbuffer.substr(ende+1,headerende - ende - 2);
			ParseHeader(header);
			Host = HeaderList["Host"];
			if(Method == M_POST) // TODO: Und testen ob content = formdata
			{				

				string t = "multipart/form-data; boundary=";
				if(HeaderList["Content-Type"].compare(0,t.length(),t) == 0)
				{
					SocketWriteLn("Sorry, momentan broken\n");
					/*Boundary = "--" + HeaderList["Content-Type"].substr(t.length(),HeaderList["Content-Type"].length() - t.length());
					dprintf("Boundary: '%s'\n",Boundary.c_str());
					if((headerende + 3) < rawbuffer_len)
						ParseBoundaries(rawbuffer.substr(headerende + 3,rawbuffer_len - (headerende + 3)));
					HandleUpload();*/
				}			
				else if(HeaderList["Content-Type"].compare("application/x-www-form-urlencoded") == 0)
				{
					dprintf("Form Daten in Parameter String\n");
					if((headerende + 3) < rawbuffer_len)
					{
						string params = rawbuffer.substr(headerende + 3,rawbuffer_len - (headerende + 3));
						if(params[params.length()-1] == '\n')
							params.substr(0,params.length() -2);
						ParseParams(params);
					}
				}
				
				dprintf("Method Post !\n");
			}

/*
			if(Method == M_POST) // TODO: Und testen ob content = formdata
			{
				if( (ende + 3) < rawbuffer + rawbuffer_len)
				{
//					Parent->Debug("Post Parameter vorhanden\n");
					anfang = ende + 3;
					Param_String = string(anfang,rawbuffer + rawbuffer_len - anfang);
					dprintf("Post Param_String: %s\n",Param_String.c_str());
					ParseParams(Param_String);
				}
				if(HeaderList->GetIndex("Content-Type") != -1)
				{
					dprintf("Content-Type: %s\n",HeaderList->GetValue(HeaderList->GetIndex("Content-Type")));
					if(strcasecmp("application/x-www-form-urlencoded",HeaderList->GetValue(HeaderList->GetIndex("Content-Type"))) == 0)
						dprintf("Form Daten in Parameter String\n");
					if(strstr(HeaderList->GetValue(HeaderList->GetIndex("Content-Type")),"multipart/form-data") != 0)
					{
						char * boundary;
						boundary = strstr(HeaderList->GetValue(HeaderList->GetIndex("Content-Type")),"boundary=");
						if(boundary)
						{
							boundary += strlen("boundary=");

							dprintf("boundary : %s\n",boundary);
							Upload = new TUpload(this);
							Upload->Boundary = new TString(boundary);
							Boundary = new TString(boundary);
							dprintf("Form Daten in Parameter String und Datei upload\nBoundary: %ld\n",Boundary);
						}
					}					
				}
			}
*/
			return true;
		}
		else {
			SocketWrite("HTTP/1.0 501 Not implemented\r\n");
			SocketWrite("Content-Type: text/plain\r\n\r\n");
			SocketWrite("501 : Request-Method not implemented.\n");
			HttpStatus = 501;
//			dprintf("501 : Request-Method not implemented.\n");
			return false;
		}
	}
	return false;
}
//-------------------------------------------------------------------------
bool CWebserverRequest::HandleUpload()				// momentan broken 
{
	int t = 0;
//	FILE *output;
//	int count = 0;

	SocketWrite("HTTP/1.1 100 Continue \r\n\r\n");		// Erstmal weitere Daten anfordern

	if(HeaderList["Content-Length"] != "")
	{
		dprintf("Contenlaenge gefunden\n");
		long contentsize = atol(HeaderList["Content-Length"].c_str());
		dprintf("Contenlaenge :%ld\n",contentsize);
		char *buffer2 =(char *) malloc(contentsize);
		if(!buffer2)
		{
			dprintf("Kein Speicher für upload\n");
			return false;
		}
		long long gelesen = 0;
		dprintf("Buffer ok Groesse:%ld\n",contentsize);
		while(gelesen < contentsize)
		{
			t = read(Socket,&buffer2[gelesen],contentsize-gelesen);
			if(t <= 0)
				dprintf("nix mehr\n");
			gelesen += t;
			dprintf("gelesen %lld\n",gelesen);
		}
		dprintf("fertig\n");
		FILE *out = fopen("/var/tmp/test.ausgabe","w");
		if(out != NULL)
		{
			fwrite(buffer2,gelesen,1,out);
			fclose(out);
		}
		else
			dprintf("nicht geschreiben\n");
		free(buffer2);
		
		if(gelesen == contentsize)
		{
			dprintf("Upload komplett gelesen: %ld bytes\n",contentsize);
			return true;
		} 
		else
		{
			dprintf("Upload konnte nicht komplett gelesen werden  %ld bytes\n",contentsize);
			return false;
		}
	}
	else
	{
		dprintf("Content-Length ist nicht in der HeaderListe\n");
		return false;
	}
}
//-------------------------------------------------------------------------
void CWebserverRequest::PrintRequest()					// for debugging and verbose output
{
	CDEBUG::getInstance()->LogRequest(this);
}

//-------------------------------------------------------------------------
void CWebserverRequest::SendHTMLHeader(string Titel)
{
	SocketWriteLn("<html>\n<head><title>" + Titel + "</title><link rel=\"stylesheet\" type=\"text/css\" href=\"../global.css\">");
	SocketWriteLn("<meta http-equiv=\"cache-control\" content=\"no-cache\">");
	SocketWriteLn("<meta http-equiv=\"expires\" content=\"0\"></head>\n<body>");
}

//-------------------------------------------------------------------------
void CWebserverRequest::SendHTMLFooter()
{
	SocketWriteLn("</body></html>");
}
//-------------------------------------------------------------------------
void CWebserverRequest::Send302(char *URI)
{
	SocketWrite("HTTP/1.0 302 Moved Permanently\r\nLocation: ");
	SocketWrite(URI);
	SocketWrite("\r\nContent-Type: text/html\r\n\r\n");
	if (Method != M_HEAD) {
		SocketWrite("<html><head><title>Object moved</title></head><body>");
		SocketWrite("302 : Object moved.<brk>If you dont get redirected click <a href=\"");
		SocketWrite(URI);
		SocketWrite("\">here</a></body></html>\n");
	}
	HttpStatus = 302;
}
//-------------------------------------------------------------------------
void CWebserverRequest::Send404Error()
{
	SocketWrite("HTTP/1.0 404 Not Found\r\n");		//404 - file not found
	SocketWrite("Content-Type: text/plain\r\n\r\n");
	if (Method != M_HEAD) {
		SocketWrite("404 : File not found\n\nThe requested file was not found on this dbox ;)\n");
	}
	HttpStatus = 404;
}
//-------------------------------------------------------------------------
void CWebserverRequest::Send500Error()
{
	SocketWrite("HTTP/1.0 500 InternalError\r\n");		//500 - internal error
	SocketWrite("Content-Type: text/plain\r\n\r\n");
	if (Method != M_HEAD) {
		SocketWrite("500 : InternalError\n\nPerhaps some parameters missing ? ;)");
	}
	HttpStatus = 500;
}
//-------------------------------------------------------------------------

void CWebserverRequest::SendPlainHeader(string contenttype)
{
	SocketWrite("HTTP/1.0 200 OK\r\nContent-Type: " + contenttype + "\r\n\r\n");
	HttpStatus = 200;
}

//-------------------------------------------------------------------------
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
	// Nur umschreiben wenn nicht mit /fb/ oder /control/ anfängt
	if( (strncmp(Path.c_str(),"/fb",3) != 0) && (strncmp(Path.c_str(),"/control",8) != 0) && (strncmp(Path.c_str(),"/bouquetedit",12) != 0))	
//	if( (Path.substr(0,3).compare("/fb") > 0) && 
//	(Path.substr(0,8).compare("/control") > 0) && 
//	(Path.substr(0,12).compare("/bouquetedit") > 0))	
	{
		if(strncmp(Path.c_str(),"/public",7) == 0)							// mit /public gelangt man in den inhalt von PublicDocumentRoor
			Path = Parent->PublicDocumentRoot + Path.substr(7,Path.length() - 7);	
		else
			Path = Parent->PrivateDocumentRoot + Path;

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

//-------------------------------------------------------------------------
bool CWebserverRequest::SendResponse()
{
	RewriteURL();		// Erst mal die URL umschreiben

	if(Path.compare("/control/") == 0)						// api for external programs
	{
		return Parent->WebDbox->ControlAPI->Execute(this);
	}
	else if(Path.compare("/bouquetedit/") == 0)				// bouquetedit api
	{
		return Parent->WebDbox->BouqueteditAPI->Execute(this);
	}
	else if(Path.compare("/fb/") == 0)						// webbrowser api
	{
		return Parent->WebDbox->WebAPI->Execute(this);
	}
	else
	{
	// Normale Datei										//normal file
		if( (tmpint = OpenFile(Path,Filename) ) != -1 )		// Testen ob Datei auf Platte geöffnet werden kann
		{											// Wenn Datei geöffnet werden konnte
			if (!SocketWrite("HTTP/1.0 200 OK\r\n"))
			{
				close(tmpint);
				return false;
			}
			HttpStatus = 200;
			if( FileExt == "" )		// Anhand der Dateiendung den Content bestimmen
				ContentType = "text/html";
			else
			{
				if(  (FileExt.compare("html") == 0) || (FileExt.compare("htm") == 0) )
					ContentType = "text/html";
				else if(FileExt.compare("gif") == 0)
					ContentType = "image/gif";
				else if((FileExt.compare("png") == 0) || (FileExt.compare("PNG") == 0) )
					ContentType = "image/png";
				else if( (FileExt.compare("jpg") == 0) || (FileExt.compare("JPG") == 0) )
					ContentType = "image/jpeg";
				else if( (FileExt.compare("css") == 0) || (FileExt.compare("CSS") == 0) )
					ContentType = "text/css";
				else if(FileExt.compare("xml") == 0)
					ContentType = "text/xml";
				else
					ContentType = "text/plain";

			}
			if (!SocketWrite("Content-Type: " + ContentType + "\r\n\r\n"))
			{
				close(tmpint);
				return false;
			}
			if (Method != M_HEAD) {
				SendOpenFile(tmpint);
			}
			else {
				close(tmpint);
			}
		}
		else
		{											// Wenn Datei nicht geöffnet werden konnte
			Send404Error();							// 404 Error senden
		}
		return true;
	}
}
//-------------------------------------------------------------------------
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
//-------------------------------------------------------------------------
void CWebserverRequest::SendOk()
{
	SocketWrite("ok");
}
//-------------------------------------------------------------------------
void CWebserverRequest::SendError()
{
	SocketWrite("error");
}
//-------------------------------------------------------------------------
void CWebserverRequest::printf ( const char *fmt, ... )
{
#define OUTBUFSIZE 2048

char *buffer = new char[OUTBUFSIZE];
	buffer[0] = 0;
	va_list arglist;
	va_start( arglist, fmt );
//	if(arglist)
		vsnprintf( buffer,OUTBUFSIZE, fmt, arglist );
	va_end(arglist);
//	::printf(buffer);
	SocketWriteData(buffer,strlen(buffer));
	delete[] buffer;
}


bool CWebserverRequest::SocketWrite(char *text)
{
	return SocketWriteData(text, strlen(text));
}
//-------------------------------------------------------------------------
bool CWebserverRequest::SocketWriteLn(char *text)
{
	if(!SocketWriteData(text, strlen(text)))
		return false;
	return SocketWriteData("\r\n",2);
}
//-------------------------------------------------------------------------
bool CWebserverRequest::SocketWriteData( char* data, long length )
{
//	write(Socket, data, length );
	if(RequestCanceled)
		return false;
	if((write(Socket, data, length) == -1) )
	{
		dprintf("request canceled\n");
		RequestCanceled = true;
		return false;
	}
	return true;
}
//-------------------------------------------------------------------------

bool CWebserverRequest::SendFile(string path,string filename)
{
	if( (tmpint = OpenFile(path,filename) ) != -1 )	
	{											
		SendOpenFile(tmpint);
		return true;
	}
	else
		return false;
}
//-------------------------------------------------------------------------
void CWebserverRequest::SendOpenFile(int file)
{
	if(!RequestCanceled)
	{
		off_t start = 0;
		off_t end = lseek(file,0,SEEK_END);
		sendfile(Socket,file,&start,end);
	}
	close(file);
}
//-------------------------------------------------------------------------
int CWebserverRequest::OpenFile(string path, string filename)
{
	struct stat statbuf;
//tmpint als file und
//tmpstring als pathfilename missbraucht
	tmpint = -1;
	if(path[path.length()-1] != '/')
		tmpstring = path + "/" + filename;
	else
		tmpstring = path + filename;
	if(tmpstring.length() > 0)
	{
		tmpint = open( tmpstring.c_str(), O_RDONLY );
		if (tmpint<=0)
		{
			aprintf("cannot open file %s\n", tmpstring.c_str());
			dperror("");
		}
		fstat(tmpint,&statbuf);
		if (!S_ISREG(statbuf.st_mode)) {
			close(tmpint);
			tmpint = -1;
		}
	}
	return tmpint;
}


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

bool CWebserverRequest::ParseFile(string file,CStringList params)		// replace all parameters if file
{
	FILE * f;
	char zeile[1024];
	if(RequestCanceled)
		return false;
	if((f = fopen(file.c_str(),"r")) == NULL)
	{
		aprintf("Parse file open error: '%s'\n",file.c_str());
		return false;
	}
	while(!feof(f))
	{
		if(fgets(zeile,sizeof(zeile),f))
		{
			SocketWrite(ParseLine(zeile,params));
		}
	};
	fclose(f);
	return true;
}
//-------------------------------------------------------------------------

string CWebserverRequest::ParseLine(string line,CStringList params)		// replaces %%xx%% with string in params["xx"]
{					
	int a = 0,e = 0,anfang = 0, ende = 0;
	if((a = line.find_first_of('%')) >= 0)
	{
		if(line[a] == '%')
		{
			anfang = a;
			string rest = line.substr(a+2,line.length() - (a+2));
			if((e = rest.find_first_of('%')) > 0)
				if(rest[e] == '%')
				{
					ende = a + 2 + e + 2;					
					return line.substr(0,anfang) + ((params[rest.substr(0,e)].length() > 0)?params[rest.substr(0,e)]:"param '"+rest.substr(0,e)+"' not found") + line.substr(ende, line.length() - ende);
				}
		}
	}
	return line;
}
//-------------------------------------------------------------------------
// Decode URLEncoded string
void CWebserverRequest::URLDecode(string &encodedString) 
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
