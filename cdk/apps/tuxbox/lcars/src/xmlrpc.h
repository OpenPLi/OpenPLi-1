/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
	homepage			 : www.chatville.de
	modified by			 : -
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
$Log: xmlrpc.h,v $
Revision 1.4.6.1  2008/08/09 16:40:33  fergy
xmlrpc  is again into Makefile as We will need it in the future

Revision 1.4  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.3  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.2  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.1  2001/12/11 13:34:59  TheDOC
initial release

*/
#ifndef XMLRPC_H
#define XMLRPC_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <sstream>
#include <stdarg.h>
#include <asm/types.h>
#include "container.h"

enum
{
    NOTYPE, INT, BOOLEAN, STRING, DOUBLE, DATETIME, BASE64, STRUCT, ARRAY
};

enum
{
    RESPONSE, REQUEST, FAULT, FAILED
};

enum
{
    SERVICE, EMPTY
};
struct command
{
	std::string parm;
	std::string cmd;
};

class xmlrpc_value
{
	int type;
	union
	{
		int int_value;
		bool boolean_value;
		double double_value;
		time_t datetime_value;
	};
	std::string string_value;
	std::string base64_value;
	std::vector<xmlrpc_value*> array_value;
	std::map<std::string, xmlrpc_value*> struct_value;
public:
	xmlrpc_value();
	xmlrpc_value(int type, void* value);

	typedef std::vector<xmlrpc_value*> xmlrpc_array;
	typedef std::map<std::string, xmlrpc_value*> xmlrpc_struct;
	typedef std::pair<std::string, xmlrpc_value*> xmlrpc_struct_pair;

	void setValue(int type, void* value);

	int getType() { return type; }

	time_t date_from_ISO8601 (const char *text);
	std::string date_to_ISO8601 (time_t value);

	int getIntValue() { return int_value; }
	bool getBooleanValue() { return boolean_value; }
	double getDoubleValue() { return double_value; }
	time_t getDatetimeValue() { return datetime_value; }
	std::string getStringValue() { return string_value; }
	std::string getBase64Value() { return base64_value; }
	xmlrpc_array getArrayValue() { return array_value; }
	xmlrpc_struct getStructValue() { return struct_value; }

	void getXML(std::stringstream *ostr);
	int parseXML(std::vector<struct command> *command_list, int *counter);
};

class xmlrpc_params
{
	std::vector<xmlrpc_value*> params;
public:
	xmlrpc_params();

	void addParam(xmlrpc_value* value);
	int paramCount() { return params.size(); }
	xmlrpc_value* getParam(int number) { return params[number]; }

	void getXML(std::stringstream *ostr);
};

class xmlrpc_fault
{
	int faultCode;
	std::string faultString;
public:
	void setFaultCode(int number) { faultCode = number; }
	int getFaultCode() { return faultCode; }

	void setFaultString(std::string name) { faultString = name; }
	std::string getFaultString() { return faultString; }

	void getXML(std::stringstream *ostr);
};

class xmlrpc_response
{
	xmlrpc_params *params;
	xmlrpc_fault *fault;
	int type;
public:
	void setParams(xmlrpc_params *p) { params = p; type = RESPONSE; }
	xmlrpc_params* getParams() { return params; }

	void setFault(xmlrpc_fault *f) { fault = f; type = FAULT; }
	xmlrpc_fault* getFault() { return fault; }

	void setType(int t) { type = t; }
	int getType() { return type; }

	std::string getXML();

	int parseXML(std::vector<struct command> *command_list, int *counter);
};

class xmlrpc_request
{
	xmlrpc_params *params;
	std::string methodName;
public:
	void setMethodName(std::string name) { methodName = name; }
	std::string getMethodName() { return methodName; }
	void setParams(xmlrpc_params *p) { params = p; }
	xmlrpc_params* getParams() { return params; }

	std::string getXML();
	int parseXML(std::vector<struct command> *command_list, int *counter);
};

class xmlrpc_parse
{


	std::vector<struct command> command_list;

	std::string xml;
	int type;

	xmlrpc_request *request;
	xmlrpc_response *response;
public:
	void readFile(std::string filename);
	void setXML(std::string x) { xml = x; }

	int parseXML();

	int getType() { return type; }
	xmlrpc_response* getResponse() { return response; }
	xmlrpc_request* getRequest() { return request; }


};

class handle
{
	bool valid;
	int type;
	union
	{
		int channelnumber;
	};
public:
	std::string makeHandle(int type, int count, ...);

	void parseHandle(std::string tmp_handle);
	bool handleIsValid() { return valid; }
	int getType() { return type; }
	int getChannelNumber() { return channelnumber; }
};


class xmlrpc
{
	std::string xmlin;
	std::string xmlout;
	container *container_obj;
public:
	void setObjects(container *c) { container_obj = c; }
	void setInput(std::string xml);
	void parse();
	std::string getOutput();
};


#endif
