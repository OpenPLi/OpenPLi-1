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
$Log: xmlrpc.cpp,v $
Revision 1.5.6.1  2008/08/09 16:40:33  fergy
xmlrpc  is again into Makefile as We will need it in the future

Revision 1.5  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.4  2002/05/18 04:31:02  TheDOC
Warningelimination

Revision 1.3  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.2  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.3  2001/12/20 00:31:38  tux
Plugins korrigiert

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.1  2001/12/11 13:34:59  TheDOC
initial release

*/

#include "xmlrpc.h"

xmlrpc_value::xmlrpc_value()
{
	type = NOTYPE;
}

xmlrpc_value::xmlrpc_value(int type, void* value)
{
	setValue(type, value);
}

void xmlrpc_value::setValue(int t, void* value)
{
	type = t;
	switch(type)
	{
	case INT:
		int_value = (int&)value;
		break;
	case BOOLEAN:
		boolean_value = (bool&)value;
		break;
	case DOUBLE:
		double_value = (double&)value;
		break;
	case DATETIME:
		datetime_value = (time_t&)value;
		break;
	case BASE64:
		base64_value = (std::string&)value;
		break;
	case STRING:
		string_value = (std::string&)value;
		break;
	case ARRAY:
		array_value = *(xmlrpc_array *)value;
		break;
	case STRUCT:
		struct_value = *(xmlrpc_struct *)value;
		break;
	}
}

// Einfache In-Order Traversierung (jedenfalls ne Abwandlung davon ;)
void xmlrpc_value::getXML(std::stringstream *ostr)
{
	//std::cout << "Value getXML: " << type << std::endl;

	*ostr << "<value>" << std::endl;
	//std::cout << "MARKME" << std::endl;
	switch(type)
	{
	case INT:
		//std::cout << "Int" << std::endl;
		*ostr << "<i4>" << int_value << "</i4>" << std::endl;
		break;
	case BOOLEAN:
		//std::cout << "Boolean" << std::endl;
		*ostr << "<boolean>" << (boolean_value?1:0) << "</boolean>" << std::endl;
		break;
	case DOUBLE:
		//std::cout << "Double" << std::endl;
		*ostr << "<double>" << double_value << "</double>" << std::endl;
		break;
	case STRING:
		//std::cout << "String" << std::endl;
		*ostr << "<string>" << string_value << "</string>" << std::endl;
		break;
	case DATETIME:
		//std::cout << "Datetime" << std::endl;
		*ostr << "<dateTime.iso8601>" << date_to_ISO8601(datetime_value) << "</dateTime.iso8601>" << std::endl;
	case ARRAY:
		//std::cout << "Array" << std::endl;
		*ostr << "<array><data>" << std::endl;
		for (int i = 0; (unsigned int) i < array_value.size(); i++)
		{
			array_value[i]->getXML(ostr);
		}
		*ostr << "</data></array>" << std::endl;
		break;
	case STRUCT:
		//std::cout << "Struct" << std::endl;
		sleep(3);
		*ostr << "<struct>" << std::endl;
		for (xmlrpc_struct::iterator it = struct_value.begin(); it != struct_value.end(); ++it)
		{
			*ostr << "<member>" << std::endl << "<name>" << (*it).first << "</name>" << std::endl;
			(*it).second->getXML(ostr);
			*ostr << "</member>" << std::endl;
		}
		*ostr << "</struct>" << std::endl;
		break;
	}
	*ostr << "</value>" << std::endl;
	//return ostr.str();
}


// Stolen from XMLRPC-EPI
time_t xmlrpc_value::date_from_ISO8601 (const char *text)
{
	struct tm tm;
	int n;
	int i;
	char buf[18];

	if (strchr (text, '-')) {
		char *p = (char *) text, *p2 = buf;
		while (p && *p) {
			if (*p != '-') {
				*p2 = *p;
				p2++;
			}
			p++;
		}
		text = buf;
	}


	tm.tm_isdst = -1;

	if (strlen (text) < 17) {
		return -1;
	}

	n = 1000;
	tm.tm_year = 0;
	for (i = 0; i < 4; i++) {
		tm.tm_year += (text[i] - '0') * n;
		n /= 10;
	}
	n = 10;
	tm.tm_mon = 0;
	for (i = 0; i < 2; i++) {
		tm.tm_mon += (text[i + 4] - '0') * n;
		n /= 10;
	}
	tm.tm_mon--;

	n = 10;
	tm.tm_mday = 0;
	for (i = 0; i < 2; i++) {
		tm.tm_mday += (text[i + 6] - '0') * n;
		n /= 10;
	}

	n = 10;
	tm.tm_hour = 0;
	for (i = 0; i < 2; i++) {
		tm.tm_hour += (text[i + 9] - '0') * n;
		n /= 10;
	}

	n = 10;
	tm.tm_min = 0;
	for (i = 0; i < 2; i++) {
		tm.tm_min += (text[i + 12] - '0') * n;
		n /= 10;
	}

	n = 10;
	tm.tm_sec = 0;
	for (i = 0; i < 2; i++) {
		tm.tm_sec += (text[i + 15] - '0') * n;
		n /= 10;
	}

	tm.tm_year -= 1900;

	return mktime (&tm);
}

std::string xmlrpc_value::date_to_ISO8601 (time_t value)
{
	struct tm *tm;
	tm = localtime (&value);
	char buf[20];

	strftime (buf, 20, "%Y%m%dT%H:%M:%S", tm);
	std::string tmp_string(buf);
	return tmp_string;
}

int xmlrpc_value::parseXML(std::vector<struct command> *command_list, int *counter)
{
	////std::cout << "Parse XML " << ((*command_list)[*counter].cmd) << " - " << counter << " - " << (*counter) << std::endl;

	if ((*command_list)[*counter].cmd == "value")
	{
		////std::cout << "value" << std::endl;
		(*counter)++;

		if ((*command_list)[*counter].cmd == "int" || (*command_list)[*counter].cmd == "i4")
		{
			(*counter)++;
			type = INT;
			int_value = atoi((*command_list)[*counter].parm.c_str());
			////std::cout << "Int-Value: " << int_value << std::endl;

			if ((*command_list)[*counter].cmd != "/int" && (*command_list)[*counter].cmd != "/i4")
			{
				//std::cout << "</int> or </i4> missing" << std::endl;
				return -1;
			}
		}
		else if ((*command_list)[*counter].cmd == "double") // BUG BUG BUG geht irgendwie nicht mit atof... who needs double anyway? ;)
		{
			(*counter)++;
			type = DOUBLE;
			double_value = atof((*command_list)[*counter].parm.c_str());
			////std::cout << "Double-Value: " << double_value << std::endl;

			if ((*command_list)[*counter].cmd != "/double")
			{
				//std::cout << "</double> missing" << std::endl;
				return -1;
			}
		}
		else if ((*command_list)[*counter].cmd == "string")
		{
			(*counter)++;
			type = STRING;
			string_value = (*command_list)[*counter].parm.c_str();
			////std::cout << "String-Value: " << string_value << std::endl;

			if ((*command_list)[*counter].cmd != "/string")
			{
				//std::cout << "</string> missing" << std::endl;
				return -1;
			}
		}
		else if ((*command_list)[*counter].cmd == "boolean")
		{
			(*counter)++;
			type = BOOLEAN;
			if ((*command_list)[*counter].parm == "0")
				boolean_value = false;
			else if ((*command_list)[*counter].parm == "1")
				boolean_value = true;
			else
			{
				//std::cout << "Wrong Boolean value: " << (*command_list)[*counter].parm << std::endl;
				return -1;
			}

			////std::cout << "Boolean-Value: " << boolean_value << std::endl;

			if ((*command_list)[*counter].cmd != "/boolean")
			{
				//std::cout << "</boolean> missing" << std::endl;
				return -1;
			}
		}
		else if ((*command_list)[*counter].cmd == "dateTime.iso8601")
		{
			(*counter)++;
			type = DATETIME;
			datetime_value = date_from_ISO8601((*command_list)[*counter].parm.c_str());
			////std::cout << "Datetime-Value: " << datetime_value << std::endl;

			if ((*command_list)[*counter].cmd != "/dateTime.iso8601")
			{
				//std::cout << "</dateTime.iso8601> missing" << std::endl;
				return -1;
			}
		}
		else if ((*command_list)[*counter].cmd == "array")
		{
			(*counter)++;

			////std::cout << "array" << std::endl;
			if ((*command_list)[*counter].cmd != "data")
			{
				//std::cout << "<data> missing" << std::endl;
			}
			(*counter)++;

			type = ARRAY;
			array_value.clear();
			while((*command_list)[*counter].cmd != "/data")
			{
				xmlrpc_value *tmp_value = new xmlrpc_value();
				if (tmp_value->parseXML(command_list, counter) == -1)
					return -1;
				array_value.push_back(tmp_value);
				(*counter)++;
			}
			(*counter)++;

			////std::cout << "Aktuelles Kommando: " << (*command_list)[*counter].cmd << std::endl;

			if ((*command_list)[*counter].cmd != "/array")
			{
				//std::cout << "</array> missing" << std::endl;
				return -1;
			}

		}
		else if ((*command_list)[*counter].cmd == "struct")
		{
			(*counter)++;
			////std::cout << "struct" << std::endl;
			type = STRUCT;

			struct_value.clear();

			while((*command_list)[*counter].cmd == "member")
			{
				////std::cout << "member" << std::endl;
				(*counter)++;
				std::string structName;

				if ((*command_list)[*counter].cmd == "name")
				{
					////std::cout << "name" << std::endl;
					(*counter)++;
					structName = (*command_list)[*counter].parm;
					if ((*command_list)[*counter].cmd != "/name")
					{
						//std::cout << "</name> missing" << std::endl;
						return -1;
					}
					(*counter)++;
				}
				else
				{
					//std::cout << "Struct-Name missing" << std::endl;
					return -1;
				}
				xmlrpc_value *tmp_value = new xmlrpc_value();
				if (tmp_value->parseXML(command_list, counter) == -1)
					return -1;
				struct_value.insert(xmlrpc_value::xmlrpc_struct_pair(structName, tmp_value));
				(*counter)++;
				////std::cout << "COMMAND: " << (*command_list)[*counter].cmd << std::endl;
				if ((*command_list)[*counter].cmd != "/member")
				{
					//std::cout << "</member> missing" << std::endl;
					return -1;
				}
				(*counter)++;
			}


			if ((*command_list)[*counter].cmd != "/struct")
			{
				//std::cout << "</struct> missing" << std::endl;
				return -1;
			}

		}

		(*counter)++;
		if ((*command_list)[*counter].cmd != "/value")
		{
			//std::cout << "</value> missing" << std::endl;
			return -1;
		}

	}

	else
	{
		//std::cout << "<value> missing" << std::endl;
		return -1;
	}

	return 0;
}

xmlrpc_params::xmlrpc_params()
{
	params.clear();
}

void xmlrpc_params::addParam(xmlrpc_value* value)
{
	params.insert(params.end(), value);
}

void xmlrpc_params::getXML(std::stringstream *ostr)
{
	if (params.size() != 0)
	{
		*ostr << "<params>" << std::endl;

		for (int i = 0; (unsigned int) i < params.size(); i++)
		{
			*ostr << "<param>" << std::endl;
			params[i]->getXML(ostr);
			*ostr << "</param>" << std::endl;
		}
		*ostr << "</params>" << std::endl;
	}
}

void xmlrpc_fault::getXML(std::stringstream *ostr)
{
	xmlrpc_value::xmlrpc_struct fault_struct;
	xmlrpc_value *code = new xmlrpc_value(INT, (void*) faultCode);
	xmlrpc_value *string = new xmlrpc_value(STRING, (void*) faultString.c_str());
	fault_struct.insert(xmlrpc_value::xmlrpc_struct_pair("faultCode", code));
	fault_struct.insert(xmlrpc_value::xmlrpc_struct_pair("faultString", string));
	xmlrpc_value *fault = new xmlrpc_value(STRUCT, &fault_struct);

	fault->getXML(ostr);
}

std::string xmlrpc_response::getXML()
{
	std::stringstream ostr;
	ostr.clear();

	ostr << "<?xml version=\"1.0\"?>" << std::endl;
	ostr << "<methodResponse>" << std::endl;
	if (type == RESPONSE)
		params->getXML(&ostr);
	else
		fault->getXML(&ostr);
	ostr << "</methodResponse>" << std::endl << std::ends;

	return ostr.str();
}

int xmlrpc_response::parseXML(std::vector<struct command> *command_list, int *counter)
{
	params = new xmlrpc_params();

	if ((*command_list)[*counter].cmd == "params")
	{
		//std::cout << "params" << std::endl;
		(*counter)++;

		while ((*command_list)[*counter].cmd == "param")
		{
			//std::cout << "param" << std::endl;

			(*counter)++;
			xmlrpc_value *value = new xmlrpc_value();
			value->parseXML(command_list, counter);
			params->addParam(value);

			(*counter)++;
			if ((*command_list)[*counter].cmd != "/param")
			{
				//std::cout << "</param> missing" << std::endl;
				return -1;
			}
			(*counter)++;
		}
	}
	else if ((*command_list)[*counter].cmd == "fault")
	{
		//std::cout << "fault" << std::endl;
	}
	else
	{
		//std::cout << "params-Fehler" << std::endl;
		return -1;
	}

	return 0;
}

int xmlrpc_request::parseXML(std::vector<struct command> *command_list, int *counter)
{
	params = new xmlrpc_params();

	if ((*command_list)[*counter].cmd == "params")
	{
		////std::cout << "params" << std::endl;
		(*counter)++;

		while ((*command_list)[*counter].cmd == "param")
		{
			////std::cout << "param" << std::endl;

			(*counter)++;
			xmlrpc_value *value = new xmlrpc_value();
			value->parseXML(command_list, counter);
			params->addParam(value);

			(*counter)++;
			if ((*command_list)[*counter].cmd != "/param")
			{
				//std::cout << "</param> missing" << std::endl;
				return -1;
			}
			(*counter)++;
		}
	}
	else
	{
		//std::cout << "params-Fehler" << std::endl;
		return -1;
	}

	return 0;
}

std::string xmlrpc_request::getXML()
{
	std::stringstream ostr;
	ostr.clear();

	ostr << "<?xml version=\"1.0\"?>" << std::endl;
	ostr << "<methodCall>" << std::endl;
	ostr << "<methodName>" << methodName << "</methodName>" << std::endl;
	params->getXML(&ostr);
	ostr << "</methodCall>" << std::endl << std::ends;

	return ostr.str();
}

void xmlrpc_parse::readFile(std::string filename)
{
	std::ifstream input(filename.c_str());
	std::string line;

	xml.clear();

	while (std::getline(input, line))
	{
		xml.append(line);
	}
	//std::cout << xml << std::endl;

	input.close();

}

int xmlrpc_parse::parseXML()
{
	std::istringstream iss(xml);
	std::string tmp_string;
	int parmcount = 0;
	while(std::getline(iss, tmp_string, '>'))
	{
		parmcount++;
		std::string parm;
		std::string cmd;

		std::istringstream iss2(tmp_string);

		std::getline(iss2, parm, '<');
		std::getline(iss2, cmd, '<');
		////std::cout << parm << " - " << cmd << std::endl;

		struct command tmp_command;
		tmp_command.parm = parm;
		tmp_command.cmd = cmd;

		command_list.push_back(tmp_command);
	}
	if (command_list.size() < 2)
	{
		type = FAILED;
		return -1;
	}

	/*if (command_list[0].cmd.substr(4) != "?xml")
	{
		//std::cout << "Kein XML-Dokument" << std::endl;
		return -1;
	}*/

	int counter = 0;
	if (command_list[1].cmd == "methodResponse")
	{
		response = new xmlrpc_response();

		type = RESPONSE;
		////std::cout << "methodResponse" << std::endl;

		counter = 2;

		response->parseXML(&command_list, &counter);
		//std::cout << response->getXML() << std::endl;
	}
	else if (command_list[1].cmd == "methodCall")
	{
		request = new xmlrpc_request();

		type = REQUEST;
		////std::cout << "methodCall" << std::endl;
		if (command_list[2].cmd == "methodName" && command_list[3].cmd == "/methodName")
		{
			request->setMethodName(command_list[3].parm);
		}
		else
		{
			//std::cout << "methodName-Fehler" << std::endl;
			return -1;
		}

		counter = 4;
		request->parseXML(&command_list, &counter);
		////std::cout << request->getXML() << std::endl;
	}
	else
	{
		//std::cout << "Parse error on method" << std::endl;
		type = FAILED;
		return -1;
	}
	return 0;
}

std::string handle::makeHandle(int type, int count,  ...)
{
	std::stringstream ostr;

	if (type == SERVICE)
	{
		va_list arguments;
		va_start(arguments, count);
		ostr << "SERVICE no=" << va_arg(arguments, int) << std::ends;
		va_end(arguments);
	}

	return ostr.str();
}

void handle::parseHandle(std::string tmp_handle)
{
	std::istringstream iss(tmp_handle);
	std::string command;

	std::getline(iss, command, ' ');

	valid = false;

	if (command == "SERVICE")
	{
		std::string tmp_string;
		type = SERVICE;
		while(std::getline(iss, tmp_string, ' '))
		{
			std::istringstream iss(tmp_string);
			std::string cmd;
			std::string parm;
			std::getline(iss, cmd, '=');
			std::getline(iss, parm, '=');

			if (cmd == "no")
			{
				channelnumber = atoi(parm.c_str());
				valid = true;
			}
		}
	}
}

void xmlrpc::setInput(std::string xml)
{
	xmlin = xml;
}

void xmlrpc::parse()
{
	xmlrpc_parse parser;
	xmlrpc_request *request;
	xmlrpc_response response;
	handle h;

	parser.setXML(xmlin);
	parser.parseXML();

	if (parser.getType() == REQUEST)
	{
		request = parser.getRequest();
		std::string methodName = request->getMethodName();
		std::string tmp_handle;

		if (methodName == "getList")
		{
			xmlrpc_value::xmlrpc_array list_array;

			tmp_handle = request->getParams()->getParam(0)->getStringValue();

			if (tmp_handle == "")
			{
				for (int count = 0; count < container_obj->channels_obj->numberChannels(); count++)
				{
					std::string tmp_string;
					xmlrpc_value::xmlrpc_struct channel_struct;
					xmlrpc_value *servicename = new xmlrpc_value(STRING, (void*) container_obj->channels_obj->getServiceName(count).c_str());
					tmp_string = "Service";
					xmlrpc_value *servicetype = new xmlrpc_value(STRING, (void*) tmp_string.c_str());
					xmlrpc_value *servicehandle = new xmlrpc_value(STRING, (void*) h.makeHandle(SERVICE, 1, count).c_str());

					channel_struct.insert(xmlrpc_value::xmlrpc_struct_pair("caption", servicename));
					channel_struct.insert(xmlrpc_value::xmlrpc_struct_pair("type", servicetype));
					channel_struct.insert(xmlrpc_value::xmlrpc_struct_pair("handle", servicehandle));

					xmlrpc_value *new_channel = new xmlrpc_value(STRUCT, &channel_struct);
					list_array.push_back(new_channel);
				}
			}
			xmlrpc_params params;
			xmlrpc_value *array_value = new xmlrpc_value(ARRAY, &list_array);
			params.addParam(array_value);
			response.setParams(&params);

			xmlout = response.getXML();
		}
		else if (methodName == "zapTo")
		{
			tmp_handle = request->getParams()->getParam(0)->getStringValue();

			h.parseHandle(tmp_handle);
			if (h.handleIsValid())
			{
				container_obj->channels_obj->setCurrentChannel(h.getChannelNumber());

				container_obj->channels_obj->zapCurrentChannel();
				container_obj->channels_obj->setCurrentOSDProgramInfo();

				container_obj->channels_obj->receiveCurrentEIT();
				container_obj->channels_obj->setCurrentOSDProgramEIT();
				container_obj->channels_obj->updateCurrentOSDProgramAPIDDescr();
				xmlrpc_params params;
				response.setParams(&params);
			}
			else
			{
				xmlrpc_fault fault;
				fault.setFaultCode(3);
				fault.setFaultString("invalid handle");

				response.setType(FAULT);
				response.setFault(&fault);
			}


			xmlout = response.getXML();
		}
		else if (methodName == "getInfo")
		{
			//std::cout << "getInfo" << std::endl;
			tmp_handle = request->getParams()->getParam(0)->getStringValue();
			//std::cout << "Mark1" << std::endl;
			xmlrpc_value::xmlrpc_struct tmp_struct;
			//std::cout << "Mark2" << std::endl;
			if (tmp_handle == "")
			{
				xmlrpc_value *value = new xmlrpc_value(STRING, (void*) h.makeHandle(SERVICE, 1, container_obj->channels_obj->getCurrentChannelNumber()).c_str());
				tmp_struct.insert(xmlrpc_value::xmlrpc_struct_pair("handle", value));
			}
			//std::cout << "Mark3" << std::endl;
			h.parseHandle(tmp_handle);
			//std::cout << "Mark4" << std::endl;
			if (h.handleIsValid() || tmp_handle == "")
			{
				std::string tmp_string;



				if (h.getType() == SERVICE)
				{
					//std::cout << "Mark5" << std::endl;
					int channelnumber = h.getChannelNumber();
					//std::cout << "Channelnumber: " << channelnumber << std::endl;

					if (channelnumber != container_obj->channels_obj->getCurrentChannelNumber())
					{
						container_obj->channels_obj->setCurrentChannel(h.getChannelNumber());

						container_obj->channels_obj->zapCurrentChannel();
						/*container_obj->channels_obj->setCurrentOSDProgramInfo(container_obj->osd_obj);

						container_obj->channels_obj->receiveCurrentEIT();
						container_obj->channels_obj->setCurrentOSDProgramEIT(container_obj->osd_obj);
						container_obj->channels_obj->updateCurrentOSDProgramAPIDDescr(container_obj->osd_obj);*/
						//std::cout << "Zapping complete" << std::endl;
					}

					{
						tmp_string = "";
						xmlrpc_value *value = new xmlrpc_value(STRING, (void*) tmp_string.c_str());
						tmp_struct.insert(xmlrpc_value::xmlrpc_struct_pair("parentHandle", value));
						//std::cout << "ParentHandle" << std::endl;
					}

					{
						xmlrpc_value *value = new xmlrpc_value(STRING, (void*) container_obj->channels_obj->getCurrentServiceName().c_str());
						tmp_struct.insert(xmlrpc_value::xmlrpc_struct_pair("caption", value));
						//std::cout << container_obj->channels_obj->getCurrentServiceName().c_str() << std::endl;
					}

					{
						tmp_string = "Service";
						xmlrpc_value *value = new xmlrpc_value(STRING, (void*) tmp_string.c_str());
						tmp_struct.insert(xmlrpc_value::xmlrpc_struct_pair("type", value));
						//std::cout << tmp_string.c_str() << std::endl;

					}

					if (container_obj->channels_obj->getCurrentType() == 0x1)
					{
						xmlrpc_value *value = new xmlrpc_value(INT, (void*) container_obj->channels_obj->getCurrentVPID());
						tmp_struct.insert(xmlrpc_value::xmlrpc_struct_pair("videoPid", value));
						//std::cout << "VPID: " << container_obj->channels_obj->getCurrentVPID() << std::endl;
					}
					if(false)
					{
						xmlrpc_value::xmlrpc_array apid_array;

						//std::cout << "APIDCount: " << container_obj->channels_obj->getCurrentAPIDcount() << std::endl;

						for (int i = 0; i < container_obj->channels_obj->getCurrentAPIDcount(); i++)
						{
							xmlrpc_value::xmlrpc_struct apid_struct;
							{
								xmlrpc_value *value = new xmlrpc_value(INT, (void*) container_obj->channels_obj->getCurrentAPID(i));
								apid_struct.insert(xmlrpc_value::xmlrpc_struct_pair("audioPid", value));
								//std::cout << "APID: " << container_obj->channels_obj->getCurrentAPID(i) << std::endl;
							}
							{
								tmp_string = (container_obj->channels_obj->getCurrentDD(i)?"ac3":"mpeg");
								xmlrpc_value *value = new xmlrpc_value(INT, (void*) tmp_string.c_str());
								apid_struct.insert(xmlrpc_value::xmlrpc_struct_pair("type", value));
								//std::cout << tmp_string.c_str() << std::endl;
							}

							xmlrpc_value *value = new xmlrpc_value(STRUCT, &apid_struct);
							apid_array.push_back(value);
						}
						//std::cout << "Adding APID to array" << std::endl;
						xmlrpc_value *value = new xmlrpc_value(ARRAY, &apid_array);
						tmp_struct.insert(xmlrpc_value::xmlrpc_struct_pair("audioPids", value));

						/*std::stringstream *ostr;
						value->getXML(ostr);
						(*ostr) << std::endl << std::ends;
						//std::cout << ostr->str() << std::endl;*/

						//std::cout << "Added" << std::endl;
					}


				}
				//std::cout << "Adding Struct" << std::endl;
				xmlrpc_params params;
				xmlrpc_value *struct_value = new xmlrpc_value(STRUCT, &tmp_struct);
				//std::cout << "Struct added" << std::endl;

				/*std::stringstream *ostr;
				struct_value->getXML(ostr);
				(*ostr) << std::endl << std::ends;
				//std::cout << ostr->str() << std::endl;*/
				params.addParam(struct_value);
				response.setParams(&params);
			}
			else
			{
				xmlrpc_fault fault;
				fault.setFaultCode(3);
				fault.setFaultString("invalid handle");

				response.setType(FAULT);
				response.setFault(&fault);
			}
			//std::cout << "Mark6" << std::endl;

			xmlout = response.getXML();
			//std::cout << xmlout << std::endl;
			//std::cout << "Mark7" << std::endl;
		}
		else if (methodName == "beginRecordMode")
		{
			tmp_handle = request->getParams()->getParam(0)->getStringValue();

			h.parseHandle(tmp_handle);
			if (h.handleIsValid())
			{
				int channelnumber = h.getChannelNumber();

				if (channelnumber != container_obj->channels_obj->getCurrentChannelNumber())
				{
					container_obj->channels_obj->setCurrentChannel(h.getChannelNumber());

					container_obj->channels_obj->zapCurrentChannel();
					container_obj->channels_obj->setCurrentOSDProgramInfo();

					container_obj->channels_obj->receiveCurrentEIT();
					container_obj->channels_obj->setCurrentOSDProgramEIT();
					container_obj->channels_obj->updateCurrentOSDProgramAPIDDescr();
				}
				container_obj->zap_obj->dmx_stop();
				xmlrpc_params params;
				response.setParams(&params);
			}
			else
			{
				xmlrpc_fault fault;
				fault.setFaultCode(3);
				fault.setFaultString("invalid handle");

				response.setType(FAULT);
				response.setFault(&fault);
			}


			xmlout = response.getXML();
		}
		else if (methodName == "endRecordMode")
		{
			tmp_handle = request->getParams()->getParam(0)->getStringValue();

			container_obj->zap_obj->dmx_start();

			xmlrpc_params params;
			response.setParams(&params);

			xmlout = response.getXML();
		}
	}
	else
	{
		xmlrpc_fault fault;
		fault.setFaultCode(0);
		fault.setFaultString("Syntax Error");

		response.setFault(&fault);

		xmlout = response.getXML();
	}

}

std::string xmlrpc::getOutput()
{
	return xmlout;
}

