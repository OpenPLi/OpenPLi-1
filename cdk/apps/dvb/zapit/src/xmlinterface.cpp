/*
 * $Header: /cvs/tuxbox/apps/dvb/zapit/src/xmlinterface.cpp,v 1.11 2002/10/12 23:14:20 obi Exp $
 *
 * xmlinterface for zapit - d-box2 linux project
 *
 * (C) 2002 by thegoodguy <thegoodguy@berlios.de>
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

#include <stdio.h>
#include <xmltok.h>

#include <zapit/xmlinterface.h>

std::string Unicode_Character_to_UTF8(const int character)
{
	char buf[XML_UTF8_ENCODE_MAX];
	int length = XmlUtf8Encode(character, buf);
	return std::string(buf, length);
}

std::string convert_UTF8_To_UTF8_XML(const std::string s)
{
	std::string r;
	unsigned int i;
	for (i = 0; i < s.length(); i++)
	{
		switch (s[i])          // cf. http://www.w3.org/TR/xhtml1/dtds.html
		{
		case '<':           
			r += "&lt;";
			break;
		case '>':
			r += "&gt;";
			break;
		case '&':
			r += "&amp;";
			break;
		case '\"':
			r += "&quot;";
			break;
		case '\'':
			r += "&apos;";
			break;
		default:
			r += s[i];     // all UTF8 chars with more than one byte are >= 0x80 !
/*
  default:
  // skip characters which are not part of ISO-8859-1
  // 0x00 - 0x1F & 0x80 - 0x9F
  // cf. http://czyborra.com/charsets/iso8859.html
  //
  // reason: sender name contain 0x86, 0x87 and characters below 0x20
  if ((((unsigned char)s[i]) & 0x60) != 0)
  r += s[i];
*/
		}
	}
	return r;
}

std::string convert_to_UTF8(std::string s)
{
	std::string r;
	
	for (std::string::iterator it = s.begin(); it != s.end(); it++)
		r += Unicode_Character_to_UTF8((const unsigned char)*it);
		
	return r;
}

std::string Utf8_to_Latin1(const std::string s)
{
	std::string r;
	unsigned int i;
	for (i = 0; i < s.length(); i++)
	{
		if ((i < s.length() - 3) && ((s[i] & 0xf0) == 0xf0))      // skip (can't be encoded in Latin1)
			i += 3;
		else if ((i < s.length() - 2) && ((s[i] & 0xe0) == 0xe0)) // skip (can't be encoded in Latin1)
			i += 2;
		else if ((i < s.length() - 1) && ((s[i] & 0xc0) == 0xc0))
		{
			r += ((s[i] & 3) << 6) | (s[i + 1] & 0x3f);
			i++;
		}
		else r += s[i];
	}
	return r;
}

XMLTreeParser* parseXmlFile(const std::string filename)
{
	char buffer[2048];
	XMLTreeParser* tree_parser;
	size_t done;
	size_t length;
	FILE* xml_file;

	xml_file = fopen(filename.c_str(), "r");

	if (xml_file == NULL)
	{
		perror(filename.c_str());
		return NULL;
	}

//	tree_parser = new XMLTreeParser("ISO-8859-1"); // old encoding
	tree_parser = new XMLTreeParser(NULL);
//	tree_parser = new XMLTreeParser("UTF-8");      // new encoding

	do
	{
		length = fread(buffer, 1, sizeof(buffer), xml_file);
		done = length < sizeof(buffer);

		if (!tree_parser->Parse(buffer, length, done))
		{
			printf("[xmlinterface.cpp] Error parsing \"%s\": %s at line %d\n",
			       filename.c_str(),
			       tree_parser->ErrorString(tree_parser->GetErrorCode()),
			       tree_parser->GetCurrentLineNumber());

			fclose(xml_file);
			delete tree_parser;
			return NULL;
		}
	}
	while (!done);

	fclose(xml_file);

	if (!tree_parser->RootNode())
	{
		delete tree_parser;
		return NULL;
	}
	return tree_parser;
}
