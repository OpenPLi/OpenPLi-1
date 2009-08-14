/***************************************************************************
													xmlgenerator.h  -  description
														-------------------
		begin                : Nov 18th 2005
		copyright            : (C) 2005 by pieterg
***************************************************************************/

#ifndef _xmlgenerator_h
#define _xmlgenerator_h

#include <stdio.h>

class CXMLGenerator
{
	bool m_bStartTagClosed;
	FILE *m_pFile;
	int m_iIndentation;
	bool m_bElementData;
	bool m_bLineFed;
	bool m_bError;
	bool m_bEscapeData;

	int StartElementData();

	public:
	CXMLGenerator(const char *pcFileName, bool bEscapeData = false);
	~CXMLGenerator();

	int StartElement(const char *pcName);
	int StopElement(const char *pcName);
	int ElementData(const char *pcData);
	int ElementData(int iData);
	int ElementData(long long llData);
	int ElementData(unsigned int iData);
	int ElementData(double dData);
	int Attribute(const char *pcName, const char *pcValue);
	int Attribute(const char *pcName, int iValue);
	int Attribute(const char *pcName, unsigned int iValue);
	int Attribute(const char *pcName, long long llValue);

	bool IsOK() { return !m_bError; }
};

#endif
