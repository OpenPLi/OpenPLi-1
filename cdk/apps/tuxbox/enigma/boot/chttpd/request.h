/*
 * $Id: request.h,v 1.2 2005/10/18 11:30:19 digi_casi Exp $
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

#ifndef __chttpd_request_h__
#define __chttpd_request_h__

#include <string>
#include <map>
#include "webserver.h"

typedef std::map<std::string, std::string> CStringList;

enum Method_Typ
{
	M_UNKNOWN = 0,
	M_POST = 1,
	M_GET = 2,
	M_PUT = 3,
	M_HEAD = 4
};

class CWebserverRequest
{
protected:
	bool RequestCanceled;
	std::string rawbuffer;
	int rawbuffer_len;
	char *outbuf;
	std::string Boundary;
	
	long tmplong;
	int tmpint;
	std::string tmpstring;

	bool CheckAuth(void);
	std::string GetContentType(std::string ext);
	std::string GetFileName(std::string path, std::string filename);
	void SplitParameter(char *param_str);
	void RewriteURL(void);
	int OpenFile(std::string path, std::string filename);

public:
	std::string Client_Addr;
	int Socket;
	unsigned long RequestNumber;

	void printf(const char *fmt, ...);

	bool SocketWrite(char const *text);
	bool SocketWriteLn(char const *text);
	bool SocketWriteData(char const *data, long length);
	bool SocketWrite(const std::string text) { return SocketWrite(text.c_str()); }
	bool SocketWriteLn(const std::string text) { return SocketWriteLn(text.c_str()); }
	bool SendFile(const std::string path, const std::string filename);

	void SendHTMLFooter(void);
	void SendHTMLHeader(std::string Titel);
	void SendPlainHeader(std::string contenttype = "text/plain");
	void Send302(char const *URI);
	void Send404Error(void);
	void Send500Error(void);

	bool Authenticate(void);

	long ParseBuffer(char *file_buffer, long file_length, char *out_buffer, long out_buffer_size, CStringList &params);
	bool ParseFile(const std::string filename, CStringList &params);

	int Method;
	int HttpStatus;

	std::string Host;
	std::string URL;
	std::string Path;
	std::string Filename;
	std::string FileExt;
	std::string Param_String;

	CStringList ParameterList;
	CStringList HeaderList;
	
	std::map<int, std::string> boundaries;

	class CWebserver *Parent;

	CWebserverRequest(CWebserver *server);
	~CWebserverRequest(void);

	bool GetRawRequest(void);
	bool ParseRequest(void);
	bool ParseParams(std::string param_string);
	bool ParseFirstLine(std::string zeile);
	bool ParseHeader(std::string header);
	bool ParseBoundaries(std::string bounds);
	static void URLDecode(std::string &encodedString);
	bool HandleUpload(void);
	bool HandleUpload(char *Name);
	void PrintRequest(void);
	bool SendResponse(void);
	bool EndRequest(void);
	void SendOk(void);
	void SendError(void);
};

#endif /* __chttpd_request_h__ */
