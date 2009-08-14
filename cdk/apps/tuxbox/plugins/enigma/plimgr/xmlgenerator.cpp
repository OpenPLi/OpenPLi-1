/***************************************************************************
                          xmlgenerator.cpp  -  description
                             -------------------
    begin                : Nov 18th 2005
    copyright            : (C) 2005 by pieterg
 ***************************************************************************/

#include <iostream>
using namespace ::std;

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "xmlgenerator.h"
#include "wrappers.h"

CXMLGenerator::CXMLGenerator(const char *pcFileName, bool bEscapeData)
{
	m_bError = true;
	m_pFile = fopen(pcFileName, "w+");
	m_bStartTagClosed = true;
	m_iIndentation = 0;
	m_bElementData = false;
	m_bLineFed = false;
	m_bEscapeData = bEscapeData;
	if (m_pFile)
	{
		m_bError = false;
		fprintf(m_pFile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
	}
}

CXMLGenerator::~CXMLGenerator()
{
	if (m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

int CXMLGenerator::StartElement(const char *pcName)
{
	if (m_bError) return -1;
	if (!m_bStartTagClosed)
	{
		if (fwrite(">", 1, 1, m_pFile) < 1) goto error;
		m_bStartTagClosed = true;

		/* auto indent */
		m_iIndentation++;
	}
	if (!m_bLineFed)
	{
		if (fwrite("\n", 1, 1, m_pFile) < 1) goto error;
	}
	m_bLineFed = false;
	for (int i = 0; i < m_iIndentation; i++)
	{
		if (fwrite("\t", 1, 1, m_pFile) < 1) goto error;
	}
	m_bStartTagClosed = false;
	if (fprintf(m_pFile, "<%s", pcName) < 0) goto error;

	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::StopElement(const char *pcName)
{
	if (m_bError) return -1;
	if (!m_bStartTagClosed)
	{
		if (fwrite(">", 1, 1, m_pFile) < 1) goto error;
		m_bStartTagClosed = true;
	}

	if (m_bLineFed)
	{
		/* auto unindent */
		m_iIndentation--;
		if (m_iIndentation < 0) m_iIndentation = 0;

		for (int i = 0; i < m_iIndentation; i++)
		{
			if (fwrite("\t", 1, 1, m_pFile) < 1) goto error;
		}
	}

	if (fprintf(m_pFile, "</%s>", pcName) < 0) goto error;

	if (fwrite("\n", 1, 1, m_pFile) < 1) goto error;
	m_bLineFed = true;
	m_bElementData = false;
	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::StartElementData()
{
	if (m_bError) return -1;
	if (!m_bStartTagClosed)
	{
		if (fwrite(">", 1, 1, m_pFile) < 1) goto error;
		m_bStartTagClosed = true;
	}
	m_bElementData = true;
	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::ElementData(const char *pcData)
{
	if (StartElementData() < 0) goto error;
	if (fprintf(m_pFile, m_bEscapeData ? "<![CDATA[%s]]>" : "%s", pcData) < 0) goto error;
	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::ElementData(int iData)
{
	if (StartElementData() < 0) return -1;
	if (fprintf(m_pFile, "%i", iData) < 0) goto error;
	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::ElementData(long long llData)
{
	if (StartElementData() < 0) return -1;
	if (fprintf(m_pFile, "%lli", llData) < 0) goto error;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::ElementData(unsigned int iData)
{
	if (StartElementData() < 0) return -1;
	if (fprintf(m_pFile, "%u", iData) < 0) goto error;
	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::ElementData(double dData)
{
	if (StartElementData() < 0) return -1;
	if (fprintf(m_pFile, "%.04lf", dData) < 0) goto error;
	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::Attribute(const char *pcName, const char *pcValue)
{
	if (m_bError) return -1;
	if (!m_bStartTagClosed)
	{
		if (fprintf(m_pFile, " %s=\"%s\"", pcName, pcValue) < 0) goto error;
	}
	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::Attribute(const char *pcName, unsigned int iValue)
{
	if (m_bError) return -1;
	if (!m_bStartTagClosed)
	{
		if (fprintf(m_pFile, " %s=\"%u\"", pcName, iValue) < 0) goto error;
	}
	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::Attribute(const char *pcName, int iValue)
{
	if (m_bError) return -1;
	if (!m_bStartTagClosed)
	{
		if (fprintf(m_pFile, " %s=\"%i\"", pcName, iValue) < 0) goto error;
	}
	return 0;
error:
	m_bError = true;
	return -1;
}

int CXMLGenerator::Attribute(const char *pcName, long long llValue)
{
	if (m_bError) return -1;
	if (!m_bStartTagClosed)
	{
		if (fprintf(m_pFile, " %s=\"%lli\"", pcName, llValue) < 0) goto error;
	}
	return 0;
error:
	m_bError = true;
	return -1;
}
