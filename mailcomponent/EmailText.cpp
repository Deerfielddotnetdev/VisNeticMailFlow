// EmailText.cpp: implementation of the CEmailText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EmailText.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEmailText::CEmailText()
:	m_sText(""),
	m_sCharset("ISO-8859-1"),
	m_sMediaType("text"),
	m_sMediaSubType("plain")
{
}

CEmailText::~CEmailText()
{

}

CEmailText::CEmailText(const CEmailText& aOther)
:	m_sText(aOther.m_sText),
	m_sCharset(aOther.m_sCharset),
	m_sMediaType(aOther.m_sMediaType),
	m_sMediaSubType(aOther.m_sMediaSubType)
{
}

CEmailText::CEmailText(const string& aText, const string& aCharset)
:	m_sText(aText),
	m_sCharset(aCharset),
	m_sMediaType(m_sMediaType),
	m_sMediaSubType(m_sMediaSubType)
{
}

CEmailText& CEmailText::operator = (const CEmailText& aOther)
{
    if (this != &aOther) 
	{
        m_sText = aOther.m_sText;
        m_sCharset = aOther.m_sCharset;
		m_sMediaType = aOther.m_sMediaType;
		m_sMediaSubType = aOther.m_sMediaSubType;
    }
	
    return *this;
}